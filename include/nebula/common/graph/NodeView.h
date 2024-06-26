// Copyright (c) 2023 vesoft inc. All rights reserved.

#include "nebula/common/datatype/Node.h"
#include "nebula/common/datatype/ValueTypeKind.h"
#include "nebula/common/table/vector/VectorTypeDefs.h"

#pragma once

namespace nebula {

namespace vector {

template <ValueTypeKind T, typename>
struct VectorReader;

template <ValueTypeKind T, typename>
class FlatVectorBuilder;

}  // namespace vector

/****************************************************************
 * NodeView
 ****************************************************************/
class NodeView {
public:
    using vreader_t = vector::VectorReader<ValueTypeKind::kNode, void>;

    explicit NodeView(const Node& node) : node_(&node) {
        header_.nodeKey.nodeID = node_->getNodeID();
        header_.nodeKey.graphID = node.getGraphID();
    }
    explicit NodeView(Node& node) : node_(&node) {
        header_.nodeKey.nodeID = node_->getNodeID();
        header_.nodeKey.graphID = node_->getGraphID();
    }
    explicit NodeView(Node* node) : node_(node) {
        header_.nodeKey.nodeID = node_->getNodeID();
        header_.nodeKey.graphID = node->getGraphID();
    }
    // NodeView(vreader_t* vreader, vector::vector_size_t offset);
    NodeView(const vreader_t* vreader, vector::vector_size_t offset);


    Node toNative() const;
    Node toNative(std::vector<std::string>& selectedPropNames) const;

    NodeID getNodeID() const {
        return header_.nodeKey.nodeID;
    }

    GraphID getGraphID() const {
        return header_.nodeKey.graphID;
    }

    NodeTypeID getNodeTypeID() const {
        return NodeIDUtils::nodeType(header_.nodeKey.nodeID);
    }

    bool hasProperty(StringView propName) const;

    std::unordered_map<StringView, vector::DynamicView> properties() const;

    vector::DynamicView getProperty(StringView propName) const;

    bool operator==(const NodeView& other) const;
    bool operator!=(const NodeView& other) const {
        return !(*this == other);
    }
    bool operator<(const NodeView& other) const;
    // TODO(wuu): re-implement `toString` and `hashKey`
    std::string toString() const {
        return toNative().toString();
    }
    void hashKey(HashKeyAppender& appender) const {
        toNative().hashKey(appender);
    }

    NodeID nodeID() const {
        return header_.nodeKey.nodeID;
    }

    void deref();

private:
    friend class vector::NodeWriter;
    friend class vector::FlatVectorBuilder<ValueTypeKind::kNode, void>;

    vector::DynamicView getProperty(size_t propIdx) const;

    // NodeID nodeID_;
    vector::NodeHeader header_;
    vreader_t* vreader_{nullptr};
    vector::vector_size_t offset_{vector::kInvalidVectorIndex};
    const Node* node_{nullptr};
};
}  // namespace nebula

namespace std {

template <>
struct hash<nebula::NodeView> {
    std::size_t operator()(const nebula::NodeView& node) const noexcept;
};

}  // namespace std
