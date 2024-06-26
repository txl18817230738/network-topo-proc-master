// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <boost/graph/adjacency_iterator.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <folly/Synchronized.h>

#include "nebula/common/datatype/Edge.h"
#include "nebula/common/datatype/Graph.h"
#include "nebula/common/datatype/Node.h"

namespace boost {
struct MemTrackerListS {};
template <class ValueType>
struct container_gen<MemTrackerListS, ValueType> {
    using type = std::list<ValueType, nebula::memory::StlAllocator<ValueType>>;
};
template <>
struct parallel_edge_traits<MemTrackerListS> {
    using type = allow_parallel_edge_tag;
};
}  // namespace boost

namespace nebula {

struct GraphTraits {
    struct node_value_t {
        using kind = boost::vertex_property_tag;
    };
    struct edge_value_t {
        using kind = boost::edge_property_tag;
    };
    using NodeProp = boost::property<node_value_t, Node>;
    using EdgeProp = boost::property<edge_value_t, Edge>;
    // For boost adjacency_list, we use listS for vertex and edge container. We store Node and
    // Edge inside it as the property of vertex and edge.
    // It's bidirectional, and allows parallel edges.
    using AdjList = boost::adjacency_list<boost::MemTrackerListS,
                                          boost::MemTrackerListS,
                                          boost::bidirectionalS,
                                          NodeProp,
                                          EdgeProp>;

    // The NodeDesc and EdgeDesc are used to access the boost graph.
    using NodeDesc = boost::graph_traits<AdjList>::vertex_descriptor;
    using EdgeDesc = boost::graph_traits<AdjList>::edge_descriptor;

    // This is our iterator, which is a wrapper of boost iterator.
    // - `operator*` returns the ID of the node or edge (NodeID or EdgeID), which can be
    // used to query the MemGraph
    template <typename BoostIterator>
    class NebulaNodeIterator {
        using RLockPtr = folly::Synchronized<AdjList>::ConstLockedPtr;
        static constexpr node_value_t node_value{};

    public:
        NebulaNodeIterator() = default;
        NebulaNodeIterator(BoostIterator iter, RLockPtr rlock, const AdjList &graph)
                : iter_{std::move(iter)}, rlock_{std::move(rlock)}, graph_{&graph} {}
        NebulaNodeIterator(BoostIterator iter, const AdjList &graph)
                : iter_{std::move(iter)}, graph_{&graph} {}

        NebulaNodeIterator &operator++() {
            ++iter_;
            return *this;
        }
        NebulaNodeIterator &operator--() {
            --iter_;
            return *this;
        }
        NodeID operator*() const {
            return getNodeID();
        }
        NodeID getNodeID() const {
            return boost::get(node_value, *graph_, *iter_).getNodeID();
        }
        Node getNode() const {
            return boost::get(node_value, *graph_, *iter_);
        }
        bool operator==(const NebulaNodeIterator &rhs) const {
            return iter_ == rhs.iter_;
        }
        bool operator!=(const NebulaNodeIterator &rhs) const {
            return iter_ != rhs.iter_;
        }

    private:
        BoostIterator iter_;
        RLockPtr rlock_;
        const AdjList *graph_{nullptr};
    };

    template <typename BoostIterator>
    class NebulaEdgeIterator {
        using RLockPtr = folly::Synchronized<AdjList>::ConstLockedPtr;
        static constexpr edge_value_t edge_value{};

    public:
        NebulaEdgeIterator() = default;
        NebulaEdgeIterator(BoostIterator iter, RLockPtr rlock, const AdjList &graph)
                : iter_{std::move(iter)}, rlock_{std::move(rlock)}, graph_{&graph} {}
        NebulaEdgeIterator(BoostIterator iter, const AdjList &graph)
                : iter_{std::move(iter)}, graph_{&graph} {}

        NebulaEdgeIterator &operator++() {
            ++iter_;
            return *this;
        }
        NebulaEdgeIterator &operator--() {
            --iter_;
            return *this;
        }
        EdgeID operator*() const {
            return getEdgeID();
        }
        EdgeID getEdgeID() const {
            return boost::get(edge_value, *graph_, *iter_).getEdgeID();
        }
        NodeID getSrcID() const {
            return getEdgeID().srcID;
        }
        NodeID getDstID() const {
            return getEdgeID().dstID;
        }
        EdgeRank getEdgeRank() const {
            return getEdgeID().rank;
        }
        Edge getEdge() const {
            return boost::get(edge_value, *graph_, *iter_);
        }
        bool operator==(const NebulaEdgeIterator &rhs) const {
            return iter_ == rhs.iter_;
        }
        bool operator!=(const NebulaEdgeIterator &rhs) const {
            return iter_ != rhs.iter_;
        }

    private:
        BoostIterator iter_;
        RLockPtr rlock_;
        const AdjList *graph_{nullptr};
    };

    // There are 5 kinds of iterators:
    // - NodeIterator: iterate all nodes
    // - EdgeIterator: iterate all edges
    // - OutEdgeIterator: iterate all out edges of a node
    // - InEdgeIterator: iterate all in edges of a node
    // - AdjIterator: iterate all adjacent nodes of a node (the destination of out edges)
    using NodeIterator = NebulaNodeIterator<boost::graph_traits<AdjList>::vertex_iterator>;
    using EdgeIterator = NebulaEdgeIterator<boost::graph_traits<AdjList>::edge_iterator>;
    using OutEdgeIterator = NebulaEdgeIterator<boost::graph_traits<AdjList>::out_edge_iterator>;
    using InEdgeIterator = NebulaEdgeIterator<boost::graph_traits<AdjList>::in_edge_iterator>;
    using AdjIterator = NebulaNodeIterator<boost::graph_traits<AdjList>::adjacency_iterator>;

    // We need to add hash and equal for EdgeID, because the default std::hash implementation
    // for EdgeID will do "standardization()" first
    struct EdgeIDHash {
        size_t operator()(const EdgeID &edgeID) const {
            return folly::hash::hash_combine(
                    edgeID.edgeTypeID, edgeID.srcID, edgeID.dstID, edgeID.rank);
        }
    };
    struct EdgeIDEqual {
        bool operator()(const EdgeID &l, const EdgeID &r) const {
            return l.edgeTypeID == r.edgeTypeID && l.srcID == r.srcID && l.dstID == r.dstID &&
                   l.rank == r.rank;
        }
    };
};

}  // namespace nebula
