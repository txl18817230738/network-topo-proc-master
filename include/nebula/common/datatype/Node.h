// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <string_view>

#include "nebula/common/datatype/Graph.h"
#include "nebula/common/datatype/Value.h"
#include "nebula/common/utils/Types.h"
#include "nebula/common/utils/Utils.h"

namespace nebula {

class Node final {
public:
    using allocator_type = nebula::properties_type::allocator_type;

    allocator_type get_allocator() const noexcept {
        return properties_.get_allocator();
    }

    explicit Node(const allocator_type& alloc = allocator_type()) : properties_(alloc) {}

    explicit Node(NodeID nodeID,
                  GraphID graphID,
                  const allocator_type& alloc = allocator_type())
            : nodeID_(nodeID), graphID_(graphID), properties_(alloc) {}

    Node(NodeID nodeID, GraphID graphID, const properties_type& properties)
            : nodeID_(nodeID), graphID_(graphID), properties_(properties) {}

    Node(NodeID nodeID, GraphID graphID, properties_type&& properties)
            : nodeID_(nodeID), graphID_(graphID), properties_(std::move(properties)) {}

    Node(NodeID nodeID,
         GraphID graphID,
         NodeTypeID nodeTypeID,
         const allocator_type& alloc = allocator_type())
            : nodeID_(nodeID), graphID_(graphID), properties_(alloc) {
        setNodeTypeID(nodeTypeID);
    }
    Node(NodeID nodeID,
         GraphID graphID,
         NodeTypeID nodeTypeID,
         const properties_type& properties,
         const allocator_type& alloc = allocator_type())
            : nodeID_(nodeID), graphID_(graphID), properties_(properties, alloc) {
        setNodeTypeID(nodeTypeID);
    }

    Node(const Node& other, const allocator_type& alloc = allocator_type())
            : nodeID_(other.nodeID_),
              graphID_(other.graphID_),
              properties_(other.properties_, alloc) {}

    Node(Node&& other, const allocator_type& alloc = allocator_type()) noexcept
            : nodeID_(std::move(other.nodeID_)),
              graphID_(std::move(other.graphID_)),
              properties_(std::move(other.properties_), alloc) {}

    Node& operator=(const Node&) = default;

    Node& operator=(Node&&) = default;

    GraphID getGraphID() const {
        return graphID_;
    }

    void setGraphID(GraphID graphID) {
        graphID_ = graphID;
    }

    NodeID getNodeID() const {
        return nodeID_;
    }

    void setNodeID(NodeID nodeID) {
        nodeID_ = nodeID;
    }

    NodeTypeID getNodeTypeID() const {
        return NodeIDUtils::nodeType(nodeID_);
    }

    void setNodeTypeID(NodeTypeID nodeTypeID) {
        nodeID_ = NodeIDUtils::substituteNodeType(nodeID_, nodeTypeID);
    }

    const properties_type& properties() const {
        return properties_;
    }

    properties_type& properties() {
        return properties_;
    }

    bool hasProperty(std::string_view propName) const {
        return properties_.find(propName.data()) != properties_.end();
    }

    // Throw exception if property not found
    const Value& getProperty(std::string_view propName) const {
        DCHECK(hasProperty(propName)) << "property not found: " << propName;
        return properties_.at(propName.data());
    }

    void setProperty(std::string_view propName, const Value& value) {
        properties_[String(propName)] = value;
    }

    bool operator<(const Node& rhs) const {
        return nodeID_ < rhs.nodeID_;
    }

    std::string toString() const;

    void hashKey(HashKeyAppender& appender) const {
        appender.append(nodeID_);
    }

private:
    friend struct nrpc::BufferReaderWriter<Node>;
    // Default node id is 0, which means storage will generate it when insert.
    // If node id is not 0, which means storage will reuse it to update existed node.
    NodeID nodeID_{0};
    GraphID graphID_{-1};
    properties_type properties_;
};

inline std::ostream& operator<<(std::ostream& os, const Node& node) {
    return os << node.toString();
}

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);

template <>
struct nrpc::BufferReaderWriter<nebula::Node> {
    static void write(folly::IOBuf* buf, const nebula::Node& node) {
        BufferReaderWriter<NodeID>::write(buf, node.nodeID_);
        BufferReaderWriter<GraphID>::write(buf, node.graphID_);
        BufferReaderWriter<nebula::properties_type>::write(buf, node.properties_);
    }

    static Status read(folly::IOBuf* buf, nebula::Node* node) {
        NG_RETURN_IF_ERROR(BufferReaderWriter<NodeID>::read(buf, &node->nodeID_));
        NG_RETURN_IF_ERROR(BufferReaderWriter<GraphID>::read(buf, &node->graphID_));
        return BufferReaderWriter<nebula::properties_type>::read(buf, &node->properties_);
    }

    static size_t encodedSize(const nebula::Node& node) {
        return BufferReaderWriter<NodeID>::encodedSize(node.nodeID_) +
               BufferReaderWriter<GraphID>::encodedSize(node.graphID_) +
               BufferReaderWriter<nebula::properties_type>::encodedSize(node.properties_);
    }
};

using NodeList = NVector<Node>;
using NodeIDList = NVector<NodeID>;
using NodeIDCountMap = NMap<NodeID, size_t>;

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::Node> {
    std::size_t operator()(const nebula::Node& node) const noexcept {
        auto vid = node.getNodeID();
        return folly::hash::fnv64_buf(reinterpret_cast<const void*>(&vid), sizeof(vid));
    }
};

}  // namespace std
