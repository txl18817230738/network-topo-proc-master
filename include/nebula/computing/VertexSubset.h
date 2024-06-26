// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <functional>
#include <vector>

#include "nebula/common/utils/Utils.h"
#include "nebula/computing/BitSet.h"

namespace nebula::computing {

class ComputingContext;

using VertexFilterFn = std::function<bool(NodeID)>;
using VertexActionFn = std::function<void(NodeID)>;
using VertexMapFn = std::function<std::vector<NodeID>(NodeID)>;

/**
 * FIXME(yee): refine the comments
 * @brief VertexSubset is a set of vertices represented by their IDs.
 *  It can be dense or sparse. If it is dense, it is represented by a bitset.
 *  If it is sparse, it is represented by a vector.
 */
class VertexSubset final {
public:
    explicit VertexSubset(const ComputingContext* ctx, size_t vSize = 1u, bool isDense = false);

    VertexSubset(const ComputingContext* ctx, std::vector<NodeID> vids);

    // vSize function
    size_t size() const {
        return vids_.size();
    }

    bool empty() const {
        return vids_.empty();
    }

    const std::vector<NodeID>& vids() const {
        return vids_;
    }

    /**
     * @brief Filter the vertices in the subset in parallel.
     * @param f The vid filter function, must be thread-safe.
     * @return A new VertexSubset.
     */
    VertexSubset filter(VertexFilterFn f) const;

    /**
     * @brief Apply a function to each vertex in the subset in parallel.
     * @param action The vid map function, must be thread-safe.
     * @return The current VertexSubset.
     */
    VertexSubset& forEach(VertexActionFn&& action);

    /**
     * @brief Apply a map function to each vertex in the subset in parallel and return a new
     *        subset.
     * @param m The vid map function, must be thread-safe.
     * @return The new VertexSubset.
     */
    VertexSubset map(VertexMapFn&& m) const;

    bool isIn(NodeID vid) const {
        return vid2idx_.count(vid);
    }

    void add(NodeID vid) {
        if (vid2idx_.count(vid)) return;
        vids_.push_back(vid);
        vid2idx_.emplace(vid, vids_.size() - 1);
    }

    bool isDense() const {
        return isDense_;
    }

    void toDense();

    std::string toString() const;

    VertexSubset merge(const VertexSubset& rhs) const;

private:
    const ComputingContext* ctx_{nullptr};

    bool isDense_{false};
    BitSet bitset_;

    std::vector<NodeID> vids_;
    std::unordered_map<NodeID, size_t> vid2idx_;
};

}  // namespace nebula::computing
