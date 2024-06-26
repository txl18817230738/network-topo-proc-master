// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <functional>

#include "nebula/common/graph/GraphTraits.h"
#include "nebula/common/table/BindingTableCommon.h"

namespace nebula {

class RefCatalog;
struct EdgeID;
enum class ConflictAction;
class GraphView;

namespace catalog {
class Graph;
}  // namespace catalog

// MemGraph is a in-memory data structure for graph. It is built on the boost graph library
// (BGL). It is thread-safe.
//
// There are two issues that need to be considered:
//
// 1. MemGraph is a directed graph but nebula graph has both directed and undirected edges.
// MemGraph supports it by storing two directed edges for one undirected edge. It will bring
// some side effects, which is unavoidable. To be specific, when calling the edge-only methods,
// including `numEdges()`, `edges()`, `edgeIDs()`, `degree()`, the undirected edge will be count
// twice.
//
// 2. Nebula graph will store the same edge twice, no matter it is directed or undirected, as
// "incoming edge" and "outgoing edge"
// MemGraph will transform the incoming edge to outgoing edge before storing it. So it will only
// stores the outgoing edge.
class MemGraph final : public GraphImpl,
                       public BindingTable,
                       public std::enable_shared_from_this<MemGraph> {
    using node_value_t = GraphTraits::node_value_t;
    using edge_value_t = GraphTraits::edge_value_t;
    static constexpr node_value_t kNodeValue = node_value_t();
    static constexpr edge_value_t kEdgeValue = edge_value_t();


    using AdjList = GraphTraits::AdjList;
    using NodeProp = GraphTraits::NodeProp;
    using EdgeProp = GraphTraits::EdgeProp;
    using NodeDesc = GraphTraits::NodeDesc;
    using EdgeDesc = GraphTraits::EdgeDesc;

    using VidMap = std::unordered_map<NodeID, NodeDesc>;
    using EidMap = std::
            unordered_map<EdgeID, EdgeDesc, GraphTraits::EdgeIDHash, GraphTraits::EdgeIDEqual>;

public:
    Status reserve(size_t) override {
        DLOG(FATAL) << "Not needed.";
        return {};
    }
    size_t getNumRecords() const override {
        return vidMap_.rlock()->size() + eidMap_.rlock()->size();
    }
    size_t bytes() const override {
        DLOG(FATAL) << "Not implemented yet.";
        return 0;
    }
    size_t numCols() const override {
        DLOG(FATAL) << "Not needed.";
        return 0;
    }
    void setColumnNames(const std::vector<std::string> &) override {
        DLOG(FATAL) << "Not needed.";
        return;
    }
    BindingTableRPCMode determineRPCMode() override {
        DLOG(FATAL) << "RPC for MemGraph has not been implemented yet.";
        return BindingTableRPCMode::kNoCopy;
    }

public:
    using NodeIterator = GraphTraits::NodeIterator;
    using EdgeIterator = GraphTraits::EdgeIterator;
    using OutEdgeIterator = GraphTraits::OutEdgeIterator;
    using InEdgeIterator = GraphTraits::InEdgeIterator;
    using AdjIterator = GraphTraits::AdjIterator;

    using NodeFilterFn = std::function<bool(const Node &)>;
    using EdgeFilterFn = std::function<bool(const Edge &)>;
    using PropFilterFn = std::function<bool(const Value &)>;

    static EdgeFilterFn kDefaultEdgeFilter;
    static NodeFilterFn kDefaultNodeFilter;

public:
    // insert node/edge, return false if the node/edge already exists
    bool insertNode(const Node &node) override;
    bool insertNode(const Node &node, ConflictAction);
    bool insertEdge(const Edge &edge) override;
    bool insertEdge(const Edge &edge, ConflictAction);
    // update node/edge, return false if the node/edge doesn't exist
    bool updateNode(const Node &node);
    bool updateEdge(const Edge &edge);
    // delete node/edge, return false if the node/edge doesn't exist
    bool deleteNode(NodeID vid);
    bool deleteEdge(const EdgeID &eid);
    // get node/edge, return std::nullopt if the node/edge doesn't exist
    std::optional<Node> getNode(NodeID vid) const;
    std::optional<Edge> getEdge(const EdgeID &eid) const;
    Node anyNode() const;
    StatusOr<NodeView> getNodeView(NodeID vid) const;
    StatusOr<EdgeView> getEdgeView(EdgeID eid) const;
    StatusOr<std::unique_ptr<GraphView>> filterByNodeType(NodeTypeID nodeTypeID) const;
    // get the number of nodes/edges
    size_t numNodes() const override;
    size_t numEdges() const override;
    // get the degree/inDegree/outDegree of the node, return 0 if the node doesn't exist
    size_t degree(NodeID vid) const;
    size_t inDegree(NodeID vid) const;
    size_t outDegree(NodeID vid) const;

    // get the corresponding iterator
    // the iterator must be used in pair, the first one (`begin`) holds the read lock of graph
    // NOTE: don't forget to check if the iterator is valid (by check begin() != end())
    auto nodes() const -> std::pair<NodeIterator, NodeIterator>;
    auto edges() const -> std::pair<EdgeIterator, EdgeIterator>;
    auto outNeighbors(NodeID vid) const -> std::pair<AdjIterator, AdjIterator>;
    auto outEdges(NodeID vid) const -> std::pair<OutEdgeIterator, OutEdgeIterator>;
    auto inEdges(NodeID vid) const -> std::pair<InEdgeIterator, InEdgeIterator>;

    // get the corresponding IDs
    // NOTE: it may occupy a lot of memory if the graph is large
    auto nodeIDs() const -> std::vector<NodeID>;
    auto edgeIDs() const -> std::vector<EdgeID>;
    auto outNeighborIDs(NodeID vid, EdgeFilterFn filter = kDefaultEdgeFilter) const
            -> std::vector<NodeID>;
    auto inNeighborIDs(NodeID vid, EdgeFilterFn filter = kDefaultEdgeFilter) const
            -> std::vector<NodeID>;
    auto neighborIDs(NodeID vid, EdgeFilterFn filter = kDefaultEdgeFilter) const
            -> std::vector<NodeID>;
    auto outEdgeIDs(NodeID vid) const -> std::vector<EdgeID>;
    auto inEdgeIDs(NodeID vid) const -> std::vector<EdgeID>;
    auto edgeIDs(NodeID src, NodeID dst) const -> std::vector<EdgeID>;

    // Return the target vertex list by node filter
    auto verticesByFilter(NodeFilterFn filter) const -> std::vector<NodeID>;

    // Check whether the property exists in the given vertex and satisfy the given condition
    bool hasProperty(NodeID vid, const std::string &propName) const;
    bool hasProperty(NodeID vid, const std::string &propName, PropFilterFn cond) const;
    Value getProperty(NodeID vid, const std::string &propName) const;
    Value getProperty(const EdgeID &e, const std::string &propName) const;

    // Randomly extract a subgraph from this graph.
    std::shared_ptr<MemGraph> extractRandomSubGraph() const;

    // Produce a view for a graph. All the nodes and edges in the view are registered in the
    // view. The view is not thread-safe.
    static StatusOr<std::unique_ptr<GraphView>> createView(const MemGraph *graph);

    // only for test
    bool checkValid() const;
    // only for debug
    std::string debugString(bool verbose = false) const;

public:
    explicit MemGraph(const catalog::Graph *graphRef = nullptr);

    uint32_t ID() const {
        return graphID_;
    }

    const catalog::Graph *getGraphRef() const {
        return graphRef_;
    }

    static Ref makeRef(RefCatalog *catalog, std::shared_ptr<MemGraph> graph);

    bool operator==(MemGraph &rhs) const;

    bool operator!=(MemGraph &rhs) const {
        return !(*this == rhs);
    }

public:
    Status loadGraphType(catalog::Schema *schema);
    Status loadGraphType();
    StatusOr<bool> validateGraphType() const;
    Status setHomeGraphID(GraphID homeGraphID) {
        homeGraphID_ = homeGraphID;
        return {};
    }
    GraphID homeGraphID() const {
        return homeGraphID_;
    }
    catalog::GraphType *graphType() const {
        return *graphType_.rlock();
    }

protected:
    friend class GraphView;
    const AdjList &graph() {
        return graph_.unsafeGetUnlocked();
    }
    StatusOr<NodeDesc> nodeDesc(NodeID vid) const;
    StatusOr<EdgeDesc> edgeDesc(EdgeID eid) const;

private:
    bool insertDirectedEdge(const Edge &edge, ConflictAction conflictAction);
    bool insertUndirectedEdge(const Edge &edge, ConflictAction conflictAction);
    bool updateNode(const Node &node, NodeDesc nodeDesc);
    void updateEdgeProperties(AdjList &graph, const EdgeDesc *edgeDesc, const Edge &edge);
    bool updateDirectedEdge(const Edge &edge, const EdgeDesc *edgeDesc = nullptr);
    bool updateUndirectedEdge(const Edge &edge,
                              const EdgeDesc *edgeDesc = nullptr,
                              const EdgeDesc *revEdgeDesc = nullptr);
    bool deleteDirectedEdge(const EdgeID &eid);
    bool deleteUndirectedEdge(const EdgeID &eid);

public:
    // Currently, there are three types of MemGraphs regarding of graph types:
    //
    // Type-I: Close-typed MemGraph with a home graph, whose graph elements come from a real
    // home graph in the storage. Thus, its graph type is the same with that of the real home
    // graph.
    //
    // Type-II: Close-typed MemGraph without a home graph, which has its own graph type.
    // All the types of its graph elements are consistent with such graph type.
    //
    // Type-III: Open-typed MemGraph, which does not have a fixed graph type. It accepts all
    // types of graph elements. However, one graph type could be constructed as a summary of all
    // the types in this graph.
    enum class GraphTypeState : uint8_t {
        kNone = 0,
        kTypeI = 1,
        kTypeII = 2,
        kTypeIII = 3,
    };
    Status setGraphTypeState(GraphTypeState state) {
        type_ = state;
        return {};
    }
    Status setGraphType(catalog::GraphType *graphType) {
        auto type = graphType_.wlock();
        *type = graphType;
        return {};
    }

private:
    // We use three big rw locks to protect the graph.
    // In every state of the graph, if the node/edge exists in the vidMap/eidMap, then it must
    // exist in the (boost) graph.
    // In the other hand, there may be some nodes/edges existing in the boost graph but not in
    // the vidMap/eidMap. But it's ok, because we will check the vidMap/eidMap first for every
    // read/write.
    folly::Synchronized<AdjList> graph_;
    folly::Synchronized<VidMap> vidMap_;
    folly::Synchronized<EidMap> eidMap_;

    GraphTypeState type_{GraphTypeState::kTypeIII};
    folly::Synchronized<catalog::GraphType *> graphType_{nullptr};

    // An unique ID for the graph, used in RefCatalog. All graphs have an unique ID.
    uint32_t graphID_;
    // Graph ID of the home graph. Type-II and Type-III graphs do not have a home graph ID.
    GraphID homeGraphID_;
    const catalog::Graph *graphRef_{nullptr};
};

using GraphPtr = std::shared_ptr<MemGraph>;

}  // namespace nebula
