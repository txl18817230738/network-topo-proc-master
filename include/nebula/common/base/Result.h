// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <optional>
#include <ostream>
#include <type_traits>
#include <variant>

#include <folly/Unit.h>

#include "nebula/common/base/Error.h"
#include "nebula/common/base/ErrorCode.h"
#include "nebula/common/base/Logging.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"

namespace nebula {

template <typename V>
struct ResultTypeTrait {
    using type = V;
};

template <>
struct ResultTypeTrait<void> {
    using type = folly::Unit;
};


template <typename Value>
class Result {
public:
    using ValueType = typename ResultTypeTrait<Value>::type;

    static_assert(!std::is_same_v<ValueType, Error>);

public:
    Result() = default;

    Result(const ValueType &data) : data_(data) {}        // NOLINT
    Result(ValueType &&data) : data_(std::move(data)) {}  // NOLINT

    template <typename T>
    static constexpr bool is_initializable_v =
            std::is_constructible_v<ValueType, T> &&std::is_convertible_v<T, ValueType>;

    template <typename T, typename = std::enable_if_t<is_initializable_v<T>>>
    Result(T &&v) : data_(ValueType(std::forward<T>(v))) {}  // NOLINT

    Result(const Error &err) : data_(err) {}        // NOLINT
    Result(Error &&err) : data_(std::move(err)) {}  // NOLINT
    Result(ErrorCode code) : data_(Error(code)) {}  // NOLINT

    Result(ErrorCode code, const std::string &message) : data_(Error(code, message)) {}
    Result(ErrorCode code, std::string &&message) : data_(Error(code, std::move(message))) {}

    Result(const Result &r) = default;
    Result(Result &&r) = default;

    operator bool() const {
        return ok();
    }

    Result &operator=(const ValueType &data) {
        data_ = data;
        return *this;
    }

    Result &operator=(const Result &r) = default;

    Result &operator=(Result &&r) = default;

    bool operator==(const Result &rhs) const {
        return data_ == rhs.data_;
    }

    bool operator<(const Result &rhs) const {
        if constexpr (std::is_same_v<ValueType, folly::Unit>) {
            if (ok() || rhs.ok()) {
                return false;
            } else {
                return error() < rhs.error();
            }
        } else {
            return data_ < rhs.data_;
        }
    }

    ValueType &value() & {
        CHECK(ok());
        return std::get<ValueType>(data_);
    }

    const ValueType &value() const & {
        CHECK(ok());
        return std::get<ValueType>(data_);
    }

    ValueType value() && {
        CHECK(ok());
        return std::move(std::get<ValueType>(data_));
    }

    bool isError() const {
        return !ok();
    }

    Error &error() & {
        CHECK(isError());
        return std::get<Error>(data_);
    }

    const Error &error() const & {
        CHECK(isError());
        return std::get<Error>(data_);
    }

    Error error() && {
        CHECK(isError());
        return std::move(std::get<Error>(data_));
    }

    // The following methods are for compatibility with StatusOr
    Error &status() & {
        CHECK(isError());
        return error();
    }

    const Error &status() const & {
        CHECK(isError());
        return error();
    }

    Error status() && {
        CHECK(isError());
        return std::move(error());
    }

    std::string toString() const {
        if (ok()) {
            // No unify way to print ValueType
            return "Ok";
        } else {
            return "Err(" + std::get<Error>(data_).toString() + ")";
        }
    }

    constexpr bool ok() const {
        return std::holds_alternative<ValueType>(data_);
    }

    void __fbthrift_clear() {
        clear();
    }

    void clear() {
        data_ = ValueType();
    }

    ErrorCode code() const {
        // FIXME(yee): remove it after Status is removed
        return error().code();
    }

    std::string message() const {
        // FIXME(yee): remove it after Status is removed
        return error().message();
    }

private:
    std::variant<ValueType, Error> data_;
};

template <typename T>
static inline std::ostream &operator<<(std::ostream &os, const Result<T> &result) {
    os << result.toString();
    return os;
}

template <typename V>
struct nrpc::BufferReaderWriter<Result<V>> {
    static void write(folly::IOBuf *buf, const Result<V> &value) {
        // true for ok, false for error
        BufferReaderWriter<bool>::write(buf, value.ok());
        if (value.ok()) {
            BufferReaderWriter<typename Result<V>::ValueType>::write(buf, value.value());
        } else {
            BufferReaderWriter<Error>::write(buf, value.error());
        }
    }

    static Status read(folly::IOBuf *buf, Result<V> *value) {
        bool ok{false};
        auto status = nrpc::BufferReaderWriter<bool>::read(buf, &ok);
        if (!status.ok()) {
            return status;
        }
        if (ok) {
            typename Result<V>::ValueType v;
            status = BufferReaderWriter<typename Result<V>::ValueType>::read(buf, &v);
            if (status.ok()) {
                *value = std::move(v);
            }
        } else {
            Error e;
            status = BufferReaderWriter<Error>::read(buf, &e);
            if (status.ok()) {
                *value = std::move(e);
            }
        }
        return status;
    }

    static size_t encodedSize(const Result<V> &value) {
        size_t len = 0;
        len += nrpc::BufferReaderWriter<bool>::encodedSize(value.ok());

        if (value.ok()) {
            len += nrpc::BufferReaderWriter<typename Result<V>::ValueType>::encodedSize(
                    value.value());
        } else {
            len += nrpc::BufferReaderWriter<Error>::encodedSize(value.error());
        }
        return len;
    }
};

}  // namespace nebula

#define RES_RETURN_IF_ERROR(T)    \
    do {                          \
        const auto &__res = (T);  \
        if (!__res.ok()) {        \
            return __res.error(); \
        }                         \
    } while (0)

#define RES_THROW_IF_ERROR(T)                       \
    do {                                            \
        const auto &__res = (T);                    \
        if (!__res.ok()) {                          \
            throw nebula::Exception(__res.error()); \
        }                                           \
    } while (0)

#define RES_LOG_AND_RETURN_IF_ERROR(T)             \
    do {                                           \
        const auto &__res = (T);                   \
        if (!__res.ok()) {                         \
            LOG(INFO) << __res.error().toString(); \
            return __res.error();                  \
        }                                          \
    } while (0)
