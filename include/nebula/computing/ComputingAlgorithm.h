// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <unordered_map>

#include "nebula/common/datatype/EdgeID.h"
#include "nebula/common/datatype/ResultTable.h"
#include "nebula/common/graph/MemGraph.h"
#include "nebula/common/utils/Types.h"
#include "nebula/computing/ComputingContext.h"
#include "nebula/computing/ComputingEngine.h"
#include "nebula/computing/VertexSubset.h"

namespace nebula::computing {

class ComputingAlgorithmBase {
public:
    template <typename T>
    using EdgeMapFn = std::function<T(NodeID, NodeID)>;
    using EdgeFilterFn = std::function<bool(NodeID, NodeID)>;
    template <typename T>
    using ReduceFn = std::function<T(const T&, NodeID)>;

    /**
     * @brief The direction of the edge to be traversed
     */
    enum class EdgeDirection {
        kOutEdge = 0,  // outgoing edge direction
        kInEdge,       // incoming edge direction
        kBothEdge,     // both outgoing and incoming edge direction
    };

    virtual ~ComputingAlgorithmBase() = default;

    /**
     * @brief utility function to identify the vertex/edge filter is always true.
     */
    static bool alwaysReturnTrue(NodeID...) {
        return true;
    }

    /**
     * @brief utility function to reduce the state of all neighbors of current vertex.
     */
    template <typename T>
    static T defaultReduce(const T& a, NodeID) {
        return a;
    }

    const MemGraph* graph() const {
        return ctx_->graph();
    }

protected:
    explicit ComputingAlgorithmBase(ComputingContext* ctx);

    /**
     * @brief following functions are used to update the nodes/edges of graph in disk referenced
     * by the memory graph.
     */
    void insertEdge(const std::string& edgeTypeName,
                    const std::vector<Value>& srcPK,
                    const std::vector<Value>& dstPK,
                    const properties_type& props = {});
    void insertNode(const std::string& nodeTypeName, const nebula::properties_type& props);
    void updateEdge(const Edge& edge);
    Status updateEdgeInternal(const Edge& edge);

    /**
     * @brief Get the vertex subset by the given vertex id and label set
     * @param vid The given vertex id.
     * @param labels The given label set, the labels are connected by AND.
     * @return The vertex subset.
     */
    VertexSubset verticesByAllLabels(const VertexSubset& vs,
                                     const std::set<std::string>& labels) const;

    /**
     * @brief Get the vertex subset by the given vertex id and label set
     * @param vid The given vertex id.
     * @param labels The given label set, the labels are connected by OR.
     * @return The vertex subset.
     */
    VertexSubset verticesByAnyLabel(const VertexSubset& vs,
                                    const std::set<std::string>& labels) const;

    const std::set<Label>& getNodeLabelSet(NodeID vid) const;
    const std::set<Label>& getEdgeLabelSet(const EdgeID& eid) const;

    /**
     * @brief Get the neighbors of the given vertex.
     * @param vid The given vertex id.
     * @param dir The direction of the edge.
     * @return The neighbors of the given vertex.
     */
    std::vector<NodeID> neighbors(NodeID vid, EdgeDirection dir) const;

    EdgeDirection reverse(EdgeDirection dir) const;

    static const size_t kThresholdParam;

    ComputingContext* ctx_{nullptr};
};

/**
 * @brief `ComputingAlgorithm<StateType>` is the base class of all algorithms.
 *  Each vertex has a state of type defined as `StateType`. This state type must define a
 *  `result` function that returns the result of the algorithm for each vertex. The
 *  implementation class of each algorithm needs to implement the `run` interface, where
 *  edgeMap/VertexSubset functions are called to complete the graph computation on the
 *  memory graph.
 */
template <typename StateType>
class ComputingAlgorithm : public ComputingAlgorithmBase {
public:
    using BaseType = ComputingAlgorithm<StateType>;

    explicit ComputingAlgorithm(ComputingContext* ctx) : ComputingAlgorithmBase(ctx) {
        states_.reserve(graph()->numNodes());
        for (auto [begin, end] = graph()->nodes(); begin != end; ++begin) {
            states_.emplace(*begin, StateType());
        }
    }

    ~ComputingAlgorithm() override = default;

    virtual std::string name() const = 0;

    /**
     * @brief The entry of the algorithm.
     */
    virtual void run() = 0;

    /**
     * @brief edgeMap is the basic operation of all algorithms.
     * @param u The vertex subset to be operated on.
     * @param f The filter function of edge.
     * @param m The function to be applied to each edge.
     * @param c The filter function to each target vertex of adjacent edges of vertex in u.
     * @param r The reduce function to merge the results of each target vertex.
     * @param dir The direction of the edge.
     * @return The vertex subset after the operation.
     */
    template <typename T>
    VertexSubset edgeMap(VertexSubset& u,
                         EdgeFilterFn f,
                         EdgeMapFn<T> m,
                         VertexFilterFn c,
                         ReduceFn<T> r,
                         EdgeDirection dir = EdgeDirection::kOutEdge) const;

    /**
     * @brief vertexMap is used to perform the vertex action when the vertex in u meets the
     * filter f
     * @param u The vertex subset to be operated on.
     * @param f The filter function of vertex.
     * @param m The vertex operation function.
     */
    VertexSubset vertexMap(VertexSubset& u, VertexFilterFn f, VertexActionFn m);

    /**
     * @brief vSize return the number of vertices in VertexSubset
     */
    size_t vSize(VertexSubset& u) const {
        return u.size();
    }

    /**
     * @brief this functions are used to get the state of the given vertex.
     */
    StateType& state(NodeID vid) {
        DCHECK(states_.find(vid) != states_.end()) << "Invalid vid: " << vid;
        return states_[vid];
    }
    const StateType& state(NodeID vid) const {
        return const_cast<ComputingAlgorithm*>(this)->state(vid);
    }

    /**
     * @brief Get the result of the algorithm.
     * @param result The result table to be filled with the states of all vertices.
     */
    void getResult(ResultTable* result) const;

protected:
    template <typename T>
    VertexSubset edgeMapSparse(VertexSubset& u,
                               EdgeFilterFn f,
                               EdgeMapFn<T> m,
                               VertexFilterFn c,
                               ReduceFn<T> r,
                               EdgeDirection dir = EdgeDirection::kOutEdge) const;
    template <typename T>
    VertexSubset edgeMapDense(VertexSubset& u,
                              EdgeFilterFn f,
                              EdgeMapFn<T> m,
                              VertexFilterFn c,
                              ReduceFn<T> r,
                              EdgeDirection dir = EdgeDirection::kOutEdge) const;

    template <typename T>
    static bool casOp(T* ptr, T oldVal, T newVal) {
        return __sync_bool_compare_and_swap(ptr, oldVal, newVal);
    }

    template <typename T>
    static void write(T* ptr, T val) {
        volatile T oldV;
        do {
            oldV = *ptr;
        } while (!casOp(ptr, oldV, val));
    }

    template <typename T>
    static void writeAdd(T* ptr, T val) {
        volatile T newV, oldV;
        do {
            oldV = *ptr;
            newV = oldV + val;
        } while (!casOp(ptr, oldV, newV));
    }

    template <typename T>
    static void writeMax(T* ptr, T val) {
        volatile T newV, oldV;
        do {
            oldV = *ptr;
            newV = oldV > val ? oldV : val;
        } while (!casOp(ptr, oldV, newV));
    }


private:
    /**
     * @brief The state of each vertex.
     */
    std::unordered_map<NodeID, StateType> states_;
};

//---------- implementation --------------

template <typename StateType>
template <typename T>
VertexSubset ComputingAlgorithm<StateType>::edgeMap(VertexSubset& u,
                                                    EdgeFilterFn f,
                                                    EdgeMapFn<T> m,
                                                    VertexFilterFn c,
                                                    ReduceFn<T> r,
                                                    EdgeDirection dir) const {
    if (u.size() > graph()->numNodes() / kThresholdParam) {
        return edgeMapDense(u, f, m, c, r, dir);
    }
    return edgeMapSparse(u, f, m, c, r, dir);
}

template <typename StateType>
template <typename T>
VertexSubset ComputingAlgorithm<StateType>::edgeMapSparse(VertexSubset& u,
                                                          EdgeFilterFn f,
                                                          EdgeMapFn<T> m,
                                                          VertexFilterFn c,
                                                          ReduceFn<T> r,
                                                          EdgeDirection dir) const {
    using NodeIDList = std::vector<NodeID>;
    auto getTargets = [this, f, m, c, r, dir](NodeID srcId) -> folly::SemiFuture<NodeIDList> {
        auto vFilter = [srcId, f, m, c](NodeID dstId,
                                        std::unordered_map<NodeID, std::vector<T>>& tmp,
                                        NodeIDList& res) {
            if (f(srcId, dstId) && c(dstId)) {
                tmp[dstId].emplace_back(m(srcId, dstId));
                res.push_back(dstId);
            }
        };

        auto nbrs = neighbors(srcId, dir);
        if (nbrs.size() > ComputingEngine::kParallelThreshold) {
            // Use parallel filter if there are too many out edges
            using UT = decltype(nbrs);
            using FT = decltype(vFilter);
            return ctx_->engine()->parallelReduce<UT, FT, T>(nbrs, vFilter, r);
        }

        std::vector<NodeID> res;
        std::unordered_map<NodeID, std::vector<T>> tmp;
        for (auto tid : nbrs) {
            vFilter(tid, tmp, res);
        }
        for (auto nbr : res) {
            // FIXME(yee): init the value of T in reduce function?
            T t;
            DCHECK_GT(tmp.count(nbr), 0);
            for (const auto& v : tmp[nbr]) {
                t = r(t, v);
            }
        }

        return res;
    };

    auto flatten = [](std::vector<NodeIDList>&& res) {
        std::vector<NodeID> vids;
        vids.reserve(std::accumulate(
                res.begin(), res.end(), 0, [](auto sum, auto& v) { return sum + v.size(); }));
        for (auto& v : res) {
            vids.insert(vids.end(), v.begin(), v.end());
        }
        return vids;
    };

    auto engine = ctx_->engine();
    auto future = engine->parallelFor(u.vids(), getTargets).deferValue(flatten);
    auto vids = engine->runOnCurrentThread(std::move(future));

    return VertexSubset(ctx_, std::move(vids));
}

template <typename StateType>
template <typename T>
VertexSubset ComputingAlgorithm<StateType>::edgeMapDense(VertexSubset& u,
                                                         EdgeFilterFn f,
                                                         EdgeMapFn<T> m,
                                                         VertexFilterFn c,
                                                         ReduceFn<T> r,
                                                         EdgeDirection dir) const {
    // FIXME(yee): here get new all vertices each time
    auto allNodes = graph()->nodeIDs();
    auto filter = [this, &u, f, m, c, dir](NodeID vid,
                                           std::unordered_map<NodeID, std::vector<T>>& tmp,
                                           std::vector<NodeID>& res) {
        if (!c(vid)) return;
        // TODO(yee): handle in parallel when there are too many in edges
        auto nbrs = neighbors(vid, reverse(dir));
        for (auto tid : nbrs) {
            if (u.isIn(tid) && f(tid, vid)) {
                tmp[vid].emplace_back(m(tid, vid));
                res.push_back(vid);
                break;
            }
        }
    };
    using UT = decltype(allNodes);
    using FT = decltype(filter);
    auto future = ctx_->engine()->parallelReduce<UT, FT, T>(allNodes, filter, r);
    auto vids = ctx_->engine()->runOnCurrentThread(std::move(future));
    return VertexSubset(ctx_, std::move(vids));
}


template <typename StateType>
VertexSubset ComputingAlgorithm<StateType>::vertexMap(VertexSubset& u,
                                                      VertexFilterFn f,
                                                      VertexActionFn m) {
    auto engine = ctx_->engine();
    auto future = engine->parallelFilter(u.vids(), [f, m](NodeID id, std::vector<NodeID>& res) {
        if (f(id)) {
            m(id);
            res.push_back(id);
        }
    });
    auto vids = engine->runOnCurrentThread(std::move(future));
    return VertexSubset(ctx_, std::move(vids));
}

template <typename StateType>
void ComputingAlgorithm<StateType>::getResult(ResultTable* result) const {
    for (auto [begin, end] = graph()->nodes(); begin != end; ++begin) {
        NodeID vid = *begin;
        Row row;
        row.append(vid);
        state(vid).getResult(row);
        result->append(std::move(row));
    }
}

}  // namespace nebula::computing
