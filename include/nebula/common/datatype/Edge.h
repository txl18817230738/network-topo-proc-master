// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/datatype/EdgeID.h"
#include "nebula/common/datatype/Value.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"
#include "nebula/common/utils/Types.h"

namespace nebula {

class Edge final {
public:
    using allocator_type = nebula::properties_type::allocator_type;

    allocator_type get_allocator() const noexcept {
        return properties_.get_allocator();
    }

    explicit Edge(const allocator_type& alloc = allocator_type()) : properties_(alloc) {}
    Edge(NodeID srcID,
         NodeID dstID,
         GraphID graphID,
         EdgeTypeID edgeTypeID,
         EdgeRank rank,
         const allocator_type& alloc = allocator_type())
            : id_{srcID, dstID, rank, graphID, edgeTypeID}, properties_(alloc) {}

    Edge(NodeID srcID,
         NodeID dstID,
         GraphID graphID,
         EdgeTypeID edgeTypeID,
         EdgeRank rank,
         properties_type&& properties,
         const allocator_type& alloc = allocator_type())
            : id_{srcID, dstID, rank, graphID, edgeTypeID},
              properties_(std::move(properties), alloc) {}

    Edge(NodeID srcID,
         NodeID dstID,
         GraphID graphID,
         EdgeTypeID edgeTypeID,
         EdgeRank rank,
         const properties_type& properties,
         const allocator_type& alloc = allocator_type())
            : id_{srcID, dstID, rank, graphID, edgeTypeID}, properties_(properties, alloc) {}

    explicit Edge(EdgeID id, const allocator_type& alloc = allocator_type())
            : id_{id}, properties_(alloc) {}

    Edge(EdgeID id,
         const properties_type& properties,
         const allocator_type& alloc = allocator_type())
            : id_{id}, properties_(properties, alloc) {}

    Edge(const Edge& other, const allocator_type& alloc)
            : id_{other.id_}, properties_(other.properties(), alloc) {}

    Edge(Edge&& other, const allocator_type& alloc) noexcept
            : id_{std::move(other.id_)}, properties_(std::move(other.properties_), alloc) {}

    const EdgeID& getEdgeID() const {
        return id_;
    }
    EdgeID& getEdgeID() {
        return id_;
    }
    void setEdgeID(EdgeID id) {
        id_ = id;
    }

    NodeID getSrcID() const {
        return id_.srcID;
    }

    void setSrcID(NodeID srcID) {
        id_.srcID = srcID;
    }

    NodeID getDstID() const {
        return id_.dstID;
    }

    void setDstID(NodeID dstID) {
        id_.dstID = dstID;
    }

    void swapSrcAndDst() {
        std::swap(id_.srcID, id_.dstID);
    }

    GraphID getGraphID() const {
        return id_.graphID;
    }

    void setGraphID(GraphID graphID) {
        id_.graphID = graphID;
    }

    EdgeTypeID getEdgeTypeID() const {
        return id_.edgeTypeID;
    }

    void setEdgeTypeID(EdgeTypeID edgeTypeID) {
        id_.edgeTypeID = edgeTypeID;
    }

    EdgeRank getEdgeRank() const {
        return id_.rank;
    }

    void setEdgeRank(EdgeRank rank) {
        id_.rank = rank;
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

    const Value& getProperty(std::string_view propName) const {
        DCHECK(hasProperty(propName)) << "property not found: " << propName;
        return properties_.at(propName.data());
    }

    void setProperty(std::string_view propName, const Value& value) {
        properties_[String(propName)] = value;
    }

    bool operator<(const Edge& rhs) const {
        return id_ < rhs.id_;
    }

    std::string toString() const;

    void hashKey(HashKeyAppender& appender) const {
        appender.append(id_.srcID);
        appender.append(id_.dstID);
        appender.append(id_.graphID);
        appender.append(id_.edgeTypeID);
        appender.append(id_.rank);
    }

private:
    friend struct nrpc::BufferReaderWriter<Edge>;

    EdgeID id_;
    properties_type properties_;
};

using EdgeList = std::vector<Edge, nebula::memory::StlAllocator<Edge>>;

using EdgeIDList = std::vector<EdgeID, nebula::memory::StlAllocator<EdgeID>>;

using EdgeIDSet = std::unordered_set<EdgeID,
                                     std::hash<nebula::EdgeID>,
                                     std::equal_to<>,
                                     nebula::memory::StlAllocator<EdgeID>>;
inline std::ostream& operator<<(std::ostream& os, const Edge& edge) {
    return os << edge.toString();
}

bool operator==(const Edge& lhs, const Edge& rhs);
bool operator!=(const Edge& lhs, const Edge& rhs);

template <>
struct nrpc::BufferReaderWriter<nebula::Edge> {
    static void write(folly::IOBuf* buf, const nebula::Edge& edge) {
        BufferReaderWriter<nebula::EdgeID>::write(buf, edge.id_);
        BufferReaderWriter<nebula::properties_type>::write(buf, edge.properties_);
    }

    static Status read(folly::IOBuf* buf, nebula::Edge* edge) {
        NG_RETURN_IF_ERROR(BufferReaderWriter<nebula::EdgeID>::read(buf, &edge->id_));
        return BufferReaderWriter<nebula::properties_type>::read(buf, &edge->properties_);
    }

    static size_t encodedSize(const nebula::Edge& edge) {
        return BufferReaderWriter<nebula::EdgeID>::encodedSize(edge.id_) +
               BufferReaderWriter<nebula::properties_type>::encodedSize(edge.properties_);
    }
};

}  // namespace nebula

namespace std {
template <>
struct hash<nebula::Edge> {
    std::size_t operator()(const nebula::Edge& edge) const noexcept {
        return std::hash<nebula::EdgeID>()(edge.getEdgeID());
    }
};

}  // namespace std
