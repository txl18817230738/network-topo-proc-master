// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <string>

#include <folly/Likely.h>
#include <folly/Unit.h>

#include "nebula/common/base/ErrorCode.h"

namespace nebula {

class Status {
public:
    Status() = default;
    Status(ErrorCode code)  // NOLINT
            : code_(code) {}
    Status(ErrorCode code, const std::string& message) : code_(code), message_(message) {}
    Status(ErrorCode code, std::string&& message) : code_(code), message_(std::move(message)) {}
    template <typename... Args>
    Status(ErrorCode code, fmt::format_string<Args...> fmt, Args&&... args)
            : Status(code, fmt::format(fmt, std::forward<Args>(args)...)) {}

    Status(const Status&) = default;
    Status(Status&&) = default;
    Status& operator=(const Status&) = default;
    Status& operator=(Status&&) = default;

    static Status from(const std::string& codeStr, const std::string& message) {
        return {ErrorCodeUtils::parseFrom(codeStr), message};
    }

    static Status OK() {
        return {ErrorCode::SUCCESSFUL_COMPLETION};
    }

    const ErrorCode& code() const {
        return code_;
    }
    ErrorCode& code() {
        return code_;
    }

    const std::string& message() const {
        return message_;
    }
    std::string& message() {
        return message_;
    }

    bool ok() const {
        return code_ == ErrorCode::SUCCESSFUL_COMPLETION;
    }

    std::string toString() const {
        if (code_ == ErrorCode::SUCCESSFUL_COMPLETION) return SUCCESS;

        ErrorCategory category = ErrorCodeUtils::getCategory(code_);
        auto codeStr = ErrorCodeUtils::format(code_);
        if (category == ErrorCategory::T || category == ErrorCategory::U) {
            // Log the detailed error
            LOG(ERROR) << fmt::format("[{}]: {}", codeStr, message_);
            // Only return to user an error code with its corresponding description
            return fmt::format("[{}]: INTERNAL ERROR", codeStr);
        }

        if (message_.empty()) {
            return fmt::format("[{}]: {}", codeStr, "(No additional message.)");
        }
        return fmt::format("[{}]: {}", codeStr, message_);
    }

    bool operator==(const Status& rhs) const {
        return code_ == rhs.code_ && message_ == rhs.message_;
    }

    void clear() {
        code_ = ErrorCode::SUCCESSFUL_COMPLETION;
        message_.clear();
    }

    bool operator<(const Status& rhs) const {
        if (code_ < rhs.code_) {
            return true;
        } else if (code_ > rhs.code_) {
            return false;
        }
        return message_ < rhs.message_;
    }

    // This method is only used for the macros starting with `NG_` below.
    const Status& status() const {
        return *this;
    }

private:
    ErrorCode code_{ErrorCode::SUCCESSFUL_COMPLETION};
    std::string message_;
};

inline std::ostream& operator<<(std::ostream& os, const Status& status) {
    return os << status.toString();
}

}  // namespace nebula


// The macros starting with `NG_` below are applicable to both Status and Result.

#define NG_RETURN_IF_ERROR(T)      \
    do {                           \
        const auto& __res = (T);   \
        if (!__res.ok()) {         \
            return __res.status(); \
        }                          \
    } while (0)

#define NG_THROW_IF_ERROR(T)                         \
    do {                                             \
        const auto& __res = (T);                     \
        if (!__res.ok()) {                           \
            throw nebula::Exception(__res.status()); \
        }                                            \
    } while (0)

#define NG_LOG_AND_RETURN_IF_ERROR(T)       \
    do {                                    \
        const auto& __res = (T);            \
        if (!__res.ok()) {                  \
            LOG(ERROR) << __res.toString(); \
            return __res.status();          \
        }                                   \
    } while (0)

#define NAME_CONCAT_IMPL(x, y) x##y
#define NAME_CONCAT(x, y) NAME_CONCAT_IMPL(x, y)

#define NG_ASSIGN_OR_RETURN_IMPL(result_name, lhs, rexpr) \
    auto&& result_name = (rexpr); /* NOLINT */            \
    NG_RETURN_IF_ERROR(result_name);                      \
    lhs = std::move(result_name).value() /* NOLINT */

#define NG_ASSIGN_OR_THROW_IMPL(result_name, lhs, rexpr) \
    auto&& result_name = (rexpr); /* NOLINT */           \
    NG_THROW_IF_ERROR(result_name);                      \
    lhs = std::move(result_name).value() /* NOLINT */

// NOTE: it will move `rexpr`
#define NG_ASSIGN_OR_RETURN(lhs, rexpr) \
    NG_ASSIGN_OR_RETURN_IMPL(NAME_CONCAT(_res, __COUNTER__), lhs, rexpr)

#define NG_ASSIGN_OR_THROW(lhs, rexpr) \
    NG_ASSIGN_OR_THROW_IMPL(NAME_CONCAT(_res, __COUNTER__), lhs, rexpr)
