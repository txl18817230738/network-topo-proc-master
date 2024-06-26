// Copyright (c) 2024 vesoft inc. All rights reserved.

#pragma once

#include <string>
#include <vector>

namespace nebula {

struct PlanInfo {
    std::string id;
    std::string name;
    std::string details;
    std::vector<std::string> columns;
    double timeMs{0.0};
    int64_t rows{0};
    double memoryKib{0.0};
    double blockedMs{0.0};
    double queuedMs{0.0};
    double consumeMs{0.0};
    double produceMs{0.0};
    double finishMs{0.0};
    int64_t batches{0};
    int64_t concurrency{0};
    std::string otherStatsJson;
    std::vector<PlanInfo> children;
};

}  // namespace nebula
