// Copyright (c) 2024 vesoft inc. All rights reserved.

#pragma once

#include <string>

#include "nebula/common/response/PlanInfo.h"
#include "nebula/common/response/QueryStats.h"

namespace nebula {

struct Summary {
    int64_t buildTimeUs{0};
    int64_t optimizeTimeUs{0};
    int64_t totalServerTimeUs{0};
    std::string explainType;
    PlanInfo planInfo;
    QueryStats queryStats;
};

}  // namespace nebula
