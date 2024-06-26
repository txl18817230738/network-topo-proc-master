// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <optional>

#include "nebula/common/base/Status.h"
#include "nebula/common/datatype/ResultTable.h"

namespace nebula {

// An execution outcome is a component of an execution context representing the outcome of an
// execution and comprises:
//  1) A GQL-status object.
//  2) An optional result that is always a value. Every result is required to be a supported
//  result, which is a valid result of a successful outcome. Supported results are direct values
//  or indirect values that possibly reference GQL-objects.
// TODO(jie): Remove it. It's should be replaced by ExecuteResponse
struct ExecutionOutcome {
    ExecutionOutcome() = default;

    void clear() {
        status.clear();
        result.reset();
    }

    bool operator==(const ExecutionOutcome &rhs) const {
        return status == rhs.status && result == rhs.result;
    }

    bool operator!=(const ExecutionOutcome &rhs) const {
        return !(*this == rhs);
    }
    // TODO(Aiee) add default value
    Status status;
    std::optional<ResultTable> result;
};


}  // namespace nebula
