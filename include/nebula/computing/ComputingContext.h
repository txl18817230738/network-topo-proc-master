// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <memory>

namespace nebula {
namespace gql {
class RequestContext;
}  // namespace gql

class MemGraph;

namespace computing {


class ComputingEngine;

/**
 * @brief ComputingContext is the execution context of computing algorithms.
 *  It contains the information of the computing environment.
 */
class ComputingContext final {
public:
    ComputingContext(ComputingEngine* engine,
                     const MemGraph* graph,
                     std::shared_ptr<gql::RequestContext> rctx = nullptr)
            : engine_(engine), graph_(graph), rctx_(rctx) {}

    /**
     * @brief engine returns the computing engine used for computing algorithms.
     * @return the computing engine used for computing algorithms.
     */
    ComputingEngine* engine() const {
        return engine_;
    }

    const MemGraph* graph() const {
        return graph_;
    }

    std::shared_ptr<gql::RequestContext> rctx() const {
        return rctx_;
    }

private:
    ComputingEngine* engine_{nullptr};
    const MemGraph* graph_{nullptr};
    std::shared_ptr<gql::RequestContext> rctx_;
};


}  // namespace computing
}  // namespace nebula
