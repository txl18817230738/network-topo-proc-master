// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/base/Status.h"
#include "nebula/common/datatype/ResultTable.h"
#include "nebula/common/response/Summary.h"

namespace nebula {

struct ExecuteResponse {
    Status status;
    std::unique_ptr<ResultTable> result;
    Summary summary;
    std::string cursor;
    ExecuteResponse() = default;
    ExecuteResponse(ExecuteResponse&& resp) {
        status = std::move(resp.status);
        result = std::move(resp.result);
        summary = std::move(resp.summary);
        cursor = std::move(resp.cursor);
    }

    bool ok() const {
        return status.ok();
    }
};

}  // namespace nebula
