// Copyright (c) 2023 vesoft inc. All rights reserved.

#include "nebula/common/datatype/Edge.h"
#include "nebula/common/datatype/Value.h"
#include "nebula/common/datatype/ValueTypeKind.h"
#include "nebula/common/table/vector/VectorTypeDefs.h"

#pragma once

namespace nebula {

namespace vector {

template <ValueTypeKind T, typename enable>
struct VectorReader;

template <ValueTypeKind T, typename>
class FlatVectorBuilder;

}  // namespace vector

/****************************************************************
 * EdgeView
 ****************************************************************/
class EdgeView {
public:
    using vreader_t = vector::VectorReader<ValueTypeKind::kEdge, void>;

    explicit EdgeView(const Edge& edge) : edge_(&edge) {}
    explicit EdgeView(const Edge* edge) : edge_(edge) {}
    EdgeView(const vreader_t* vreader, vector::vector_size_t offset);

    Edge toNative() const;

    EdgeID getEdgeID() const;

    NodeID getSrcID() const {
        return getEdgeID().srcID;
    }

    NodeID getDstID() const {
        return getEdgeID().dstID;
    }

    GraphID getGraphID() const {
        return getEdgeID().graphID;
    }

    EdgeTypeID getEdgeTypeID() const {
        return getEdgeID().edgeTypeID;
    }
    EdgeRank getEdgeRank() const {
        return getEdgeID().rank;
    }

    bool hasProperty(StringView propName) const;

    std::unordered_map<StringView, vector::DynamicView> properties() const;

    vector::DynamicView getProperty(StringView propName) const;

    bool operator==(const EdgeView& other) const;
    bool operator!=(const EdgeView& other) const {
        return !(*this == other);
    }
    bool operator<(const EdgeView& other) const;
    // TODO(wuu): re-implement `toString` and `hashKey`
    std::string toString() const {
        return toNative().toString();
    }
    void hashKey(HashKeyAppender& appender) const {
        toNative().hashKey(appender);
    }
    void deref();

private:
    friend class vector::EdgeWriter;
    friend class vector::FlatVectorBuilder<ValueTypeKind::kEdge, void>;

    vector::DynamicView getProperty(size_t propIdx) const;

    // EdgeID edgeID_;
    vector::EdgeHeader header_;
    vreader_t* vreader_{nullptr};
    vector::vector_size_t offset_{vector::kInvalidVectorIndex};
    const Edge* edge_{nullptr};
};

}  // namespace nebula

namespace std {
template <>
struct hash<nebula::EdgeView> {
    std::size_t operator()(const nebula::EdgeView& edge) const noexcept;
};

}  // namespace std
