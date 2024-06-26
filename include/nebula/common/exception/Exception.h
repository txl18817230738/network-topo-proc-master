// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <exception>

#include "nebula/common/base/Base.h"
#include "nebula/common/base/Error.h"
#include "nebula/common/base/ErrorCode.h"

namespace nebula {

class Exception : public std::exception {
public:
    template <typename... Args>
    Exception(ErrorCode code, fmt::format_string<Args...> fmt, Args&&... args)
            : Exception(code, fmt::format(fmt, std::forward<Args>(args)...)) {}

    Exception(ErrorCode code, const std::string& message);

    explicit Exception(Error error) : Exception(error.code(), error.message()) {}

    const char* what() const noexcept override;

    ErrorCode errorCode() const {
        return errorCode_;
    }

    const Error error() const;

    const std::string& message() const;

    const std::string toString() const;

    bool operator==(const Exception& rhs) const {
        return errorCode_ == rhs.errorCode_ && message_ == rhs.message_;
    }

private:
    ErrorCode errorCode_;
    std::string message_;
};

class MemoryExceededException : public Exception {
public:
    template <typename... Args>
    explicit MemoryExceededException(fmt::format_string<Args...> fmt, Args&&... args)
            : MemoryExceededException(fmt::format(fmt, std::forward<Args>(args)...)) {}

    explicit MemoryExceededException(const std::string& message)
            : Exception(ErrorCode::MEMORY_EXCEEDED, message) {}
};

}  // namespace nebula
