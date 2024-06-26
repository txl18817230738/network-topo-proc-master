// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <string>

#include "nebula/common/base/Base.h"
#include "nebula/common/base/Error.h"

namespace nebula {

struct Diagnostics {
    // COMMAND_FUNCTION : CF;
    // COMMAND_FUNCTION_CODE : CFC;
    // NUMBER : N;
    // CURRENT_SCHEMA : CS;
    // HOME_GRAPH : HG;
    // CURRENT_GRAPH : CG;
};

// GQLStatus comprises a condition code and additional diagnostic information.
//
// Every GQL-program returns some diagnostic information to the GQL-client that originated the
// GQL-request of which the GQL-program was a part.
struct GQLStatus {
    // TODO(Aiee): This is a temp constructor used to quickly mock a GQLStatus.
    GQLStatus() = default;
    explicit GQLStatus(const Error& err) : GQLStatus(err.code(), err.message()) {}
    GQLStatus(ErrorCode code, std::string message)
            : status(ErrorCodeUtils::toString(code, message)), error(code, message) {}

    static const GQLStatus& success();

    bool ok() const {
        return *this == success();
    }

    void clear() {
        status.clear();
        error.clear();
    }

    void __fbthrift_clear() {
        return clear();
    }

    bool operator==(const GQLStatus& rhs) const {
        return status == rhs.status;
    }
    bool operator!=(const GQLStatus& rhs) const {
        return !(*this == rhs);
    }

    // TODO(Aiee) fix generated code compilation
    bool operator<(const GQLStatus& rhs) const {
        UNUSED(rhs);
        return false;
    }
    bool operator>(const GQLStatus& rhs) const {
        UNUSED(rhs);
        return false;
    }

    std::string toString() const {
        return status;
    }

    // TODO(codesigner) remove status when thrift removed
    std::string status;
    Error error;
};

}  // namespace nebula
