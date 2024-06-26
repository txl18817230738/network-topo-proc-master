// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <string>

#include <folly/Unit.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>

#include "nebula/common/base/ErrorCode.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"

namespace nebula {

class Error {
public:
    Error() = default;
    Error(const Error&) = default;
    Error(Error&&) = default;
    Error& operator=(const Error&) = default;
    Error& operator=(Error&&) = default;
    /* implicit */ Error(ErrorCode code);  // NOLINT
    Error(ErrorCode code, const std::string& message);
    Error(ErrorCode code, std::string&& message);

    const ErrorCode& code() const;
    ErrorCode& code() {
        return code_;
    }
    const std::string& message() const;
    std::string toString() const;

    bool operator==(const Error& rhs) const {
        return code_ == rhs.code_ && message_ == rhs.message_;
    }

    void __fbthrift_clear() {
        clear();
    }

    void clear() {
        code_ = ErrorCode::UNKNOWN;
        message_.clear();
    }

    bool operator<(const Error& rhs) const {
        if (code_ < rhs.code_) {
            return true;
        } else if (code_ > rhs.code_) {
            return false;
        }
        return message_ < rhs.message_;
    }

private:
    friend nrpc::BufferReaderWriter<Error>;
    friend class apache::thrift::Cpp2Ops<Error>;

    ErrorCode code_ = ErrorCode::UNKNOWN;
    std::string message_;
};

inline std::ostream& operator<<(std::ostream& os, const Error& err) {
    return os << err.toString();
}

template <>
struct nrpc::BufferReaderWriter<Error> {
    static void write(folly::IOBuf* buf, const Error& value) {
        // true for ok, false for error
        BufferReaderWriter<ErrorCode>::write(buf, value.code_);
        BufferReaderWriter<std::string>::write(buf, value.message_);
    }

    static Status read(folly::IOBuf* buf, Error* value) {
        auto status = nrpc::BufferReaderWriter<ErrorCode>::read(buf, &value->code_);
        if (!status.ok()) {
            return status;
        }
        status = nrpc::BufferReaderWriter<std::string>::read(buf, &value->message_);
        return status;
    }

    static size_t encodedSize(const Error& value) {
        size_t len = nrpc::BufferReaderWriter<ErrorCode>::encodedSize(value.code());
        len += nrpc::BufferReaderWriter<std::string>::encodedSize(value.message());
        return len;
    }
};

}  // namespace nebula
