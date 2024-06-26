// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <optional>

#include "nebula/common/base/Status.h"

namespace nebula {

// The response of the authentication of a principal.
struct AuthResponse {
    Status status;
    std::optional<int64_t> identifier;
    // May be not required in v5.0
    // std::optional<int32_t> timeZoneOffsetSeconds{};
    // std::optional<std::string> timeZoneName{};

    void clear() {
        status.clear();
        identifier = {};
    }

    bool operator==(const AuthResponse& rhs) const {
        return status == rhs.status && identifier == rhs.identifier;
    }

    bool operator!=(const AuthResponse& rhs) const {
        return !(*this == rhs);
    }
};

}  // namespace nebula
