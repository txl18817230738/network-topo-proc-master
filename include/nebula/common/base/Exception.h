// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <exception>
#include <string>

#include <fmt/format.h>

#include "nebula/common/base/Status.h"

namespace nebula {

class Exception : public std::exception {
public:
    explicit Exception(Status status) : status_(status) {}

    const char* what() const noexcept override {
        return status_.message().c_str();
    }

    const Status status() const {
        return status_;
    }

    const std::string toString() const {
        return status_.toString();
    }

    bool operator==(const Exception& rhs) const {
        return status_ == rhs.status();
    }

private:
    Status status_;
};

class MemoryExceededException : public Exception {
public:
    template <typename... Args>
    explicit MemoryExceededException(fmt::format_string<Args...> fmt, Args&&... args)
            : MemoryExceededException(fmt::format(fmt, std::forward<Args>(args)...)) {}

    explicit MemoryExceededException(const std::string& message)
            : Exception(Status(ErrorCode::MEMORY_EXCEEDED, message)) {}
};

}  // namespace nebula
