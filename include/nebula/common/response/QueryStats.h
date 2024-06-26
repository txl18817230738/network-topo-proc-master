// Copyright (c) 2024 vesoft inc. All rights reserved.

#pragma once

#include <atomic>

namespace nebula {

// query level statistics
struct QueryStats {
    std::atomic<int64_t> numAffectedNodes{0};
    std::atomic<int64_t> numAffectedEdges{0};

    QueryStats() = default;
    QueryStats& operator=(const QueryStats& rhs) {
        numAffectedNodes.store(rhs.numAffectedNodes.load());
        numAffectedEdges.store(rhs.numAffectedEdges.load());
        return *this;
    }
};

}  // namespace nebula
