// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <ostream>
#include <type_traits>
#include <variant>

#include "nebula/common/base/Base.h"
#include "nebula/common/base/Status.h"

namespace nebula {

template <typename V>
struct ValueTypeTrait {
    using type = V;
};

template <>
struct ValueTypeTrait<void>;

template <typename Value>
class StatusOr {
public:
    using ValueType = typename ValueTypeTrait<Value>::type;

    static_assert(!std::is_same_v<ValueType, Status>, "Value type cannot be Status");
    static_assert(!std::is_same_v<Value, void>,
                  "StatusOr<void> is not allowed. Use Status directly");

public:
    StatusOr() = default;

    StatusOr(const ValueType &data) : data_(data) {}        // NOLINT
    StatusOr(ValueType &&data) : data_(std::move(data)) {}  // NOLINT

    template <typename U>
    struct is_status_or {
        static auto sfinae(...) -> uint8_t {
            return 0;
        }
        template <typename V>
        static auto sfinae(const StatusOr<V> &) -> uint16_t {
            return 0;
        }
        static constexpr auto value = (sizeof(sfinae(std::declval<U>())) == sizeof(uint16_t));
    };

    template <typename T>
    static constexpr bool is_initializable_v =
            !is_status_or<T>::value && std::is_constructible_v<ValueType, T> &&
            std::is_convertible_v<T, ValueType>;

    template <typename T, typename = std::enable_if_t<is_initializable_v<T>>>
    StatusOr(T &&v) : data_(ValueType(std::forward<T>(v))) {}  // NOLINT

    StatusOr(const Status &err) : data_(err) {}        // NOLINT
    StatusOr(Status &&err) : data_(std::move(err)) {}  // NOLINT

    StatusOr(ErrorCode code, const std::string &message) : data_(Status(code, message)) {}
    StatusOr(ErrorCode code, std::string &&message) : data_(Status(code, std::move(message))) {}

    template <typename... Args>
    StatusOr(ErrorCode code, fmt::format_string<Args...> fmt, Args &&... args)
            : StatusOr(code, fmt::format(fmt, std::forward<Args>(args)...)) {}

    StatusOr(const StatusOr &r) = default;
    StatusOr(StatusOr &&r) = default;

    explicit operator bool() const {
        return ok();
    }

    StatusOr &operator=(const ValueType &data) {
        data_ = data;
        return *this;
    }

    StatusOr &operator=(const StatusOr &r) = default;

    StatusOr &operator=(StatusOr &&r) = default;

    bool operator==(const StatusOr &rhs) const {
        return data_ == rhs.data_;
    }

    bool operator<(const StatusOr &rhs) const {
        if constexpr (std::is_same_v<ValueType, folly::Unit>) {
            if (ok() || rhs.ok()) {
                return false;
            } else {
                return status() < rhs.status();
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

    Status &status() & {
        CHECK(!ok());
        return std::get<Status>(data_);
    }

    const Status &status() const & {
        CHECK(!ok());
        return std::get<Status>(data_);
    }

    Status status() && {
        CHECK(!ok());
        return std::move(std::get<Status>(data_));
    }

    std::string toString() const {
        if (ok()) {
            // No unify way to print ValueType
            return "Ok";
        } else {
            return status().toString();
        }
    }

    constexpr bool ok() const {
        return std::holds_alternative<ValueType>(data_);
    }

    void clear() {
        data_ = ValueType();
    }

    ErrorCode code() const {
        // FIXME(yee): remove it after Status is removed
        return status().code();
    }

    std::string message() const {
        // FIXME(yee): remove it after Status is removed
        return status().message();
    }

private:
    std::variant<ValueType, Status> data_;
};

template <typename T>
static inline std::ostream &operator<<(std::ostream &os, const StatusOr<T> &statusOr) {
    os << statusOr.toString();
    return os;
}

}  // namespace nebula
