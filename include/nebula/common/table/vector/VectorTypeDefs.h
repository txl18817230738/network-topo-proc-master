// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <cstdint>
#include <limits>

#include "nebula/common/datatype/Date.h"
#include "nebula/common/datatype/Datetime.h"
#include "nebula/common/datatype/EdgeID.h"
#include "nebula/common/datatype/Ref.h"
#include "nebula/common/datatype/Time.h"
#include "nebula/common/datatype/ZonedDatetime.h"
#include "nebula/common/datatype/ZonedTime.h"
#include "nebula/common/utils/Types.h"
#include "nebula/common/utils/Utils.h"

namespace nebula {
namespace vector {

using VectorUID = uint32_t;
using vector_size_t = uint32_t;
static constexpr vector_size_t kMaxMemoryTypeSize = 32;
static constexpr vector_size_t kMaximumVectorBytes = std::numeric_limits<vector_size_t>::max();
static constexpr vector_size_t kMaximumVectorSize = kMaximumVectorBytes / kMaxMemoryTypeSize;
static constexpr vector_size_t kInvalidVectorIndex = std::numeric_limits<vector_size_t>::max();
static constexpr vector_size_t kPreferredBatchSize = 1024;


/****************************************************************
 * Fixed-Width Headers
 ****************************************************************/


// list
struct ListHeader {
    vector_size_t offset;
    vector_size_t size;
};
static_assert(sizeof(ListHeader) == 8, "ListHeader should be 8 bytes");

struct NodeKey {
    NodeID nodeID;
    GraphID graphID;
};

// node
union NodeHeader {
    explicit NodeHeader(NodeID nodeID = 0, GraphID graphID = -1) {
        nodeKey.nodeID = nodeID;
        nodeKey.graphID = graphID;
    }
    NodeKey nodeKey;
    Ref nodeRef;
};
static_assert(sizeof(NodeHeader) == 16, "NodeHeader should be 16 bytes");


// edge
union EdgeHeader {
    EdgeHeader() {
        edgeID = EdgeID();
    }
    explicit EdgeHeader(EdgeID id) {
        edgeID = id;
    }
    EdgeTypeID typeID() const {
        return EdgeUtils::removeDirection(edgeID.edgeTypeID);
    }
    EdgeID edgeID;
    Ref edgeRef;
};
static_assert(sizeof(EdgeHeader) == 32, "EdgeHeader should be 32 bytes");


// path
struct PathHeader {
    NodeID headNodeID;
    NodeID tailNodeID;
    // TODO(wuu): change to uint16_t
    uint32_t size{0};          // numNodes + numEdges
    uint32_t length{0};        // numEdges
    vector_size_t headOffset;  // head node offset in the vector
    vector_size_t tailOffset;  // tail node offset in the vector
};
static_assert(sizeof(PathHeader) == 32, "PathHeader should be 32 bytes");


// fixed-width header of ScalarVector, 8B
// basic types <= 8B will be inlined
union ScalarData {
    bool bool_;
    int8_t int8_;
    int16_t int16_;
    int32_t int32_;
    int64_t int64_;
    uint8_t uint8_;
    uint16_t uint16_;
    uint32_t uint32_;
    uint64_t uint64_;
    float float_;
    double double_;
    LocalTime localTime_;          // 8B
    ZonedTime zonedTime_;          // 8B
    Date date_;                    // 4B
    LocalDatetime localDatetime_;  // 8B
    ZonedDatetime zonedDatetime_;  // 8B
    Ref ref_;                      // 8B
    // complex types
    void* ptr_;

    bool isEmpty() const {
        return ptr_ == nullptr;
    }

    ScalarData() : ptr_(nullptr) {}                     // NOLINT
    ScalarData(bool v) : bool_(v) {}                    // NOLINT
    ScalarData(int8_t v) : int8_(v) {}                  // NOLINT
    ScalarData(int16_t v) : int16_(v) {}                // NOLINT
    ScalarData(int32_t v) : int32_(v) {}                // NOLINT
    ScalarData(int64_t v) : int64_(v) {}                // NOLINT
    ScalarData(uint8_t v) : uint8_(v) {}                // NOLINT
    ScalarData(uint16_t v) : uint16_(v) {}              // NOLINT
    ScalarData(uint32_t v) : uint32_(v) {}              // NOLINT
    ScalarData(uint64_t v) : uint64_(v) {}              // NOLINT
    ScalarData(float v) : float_(v) {}                  // NOLINT
    ScalarData(double v) : double_(v) {}                // NOLINT
    ScalarData(LocalTime v) : localTime_(v) {}          // NOLINT
    ScalarData(ZonedTime v) : zonedTime_(v) {}          // NOLINT
    ScalarData(Date v) : date_(v) {}                    // NOLINT
    ScalarData(LocalDatetime v) : localDatetime_(v) {}  // NOLINT
    ScalarData(ZonedDatetime v) : zonedDatetime_(v) {}  // NOLINT
    ScalarData(Ref v) : ref_(v) {}                      // NOLINT
    ScalarData(void* v) : ptr_(v) {}                    // NOLINT
};
static_assert(sizeof(ScalarData) == 8, "ScalarData should be 8 Bytes.");

static const ScalarData kEmptyScalarData{};

// TODO(wuu): optimize it
static constexpr size_t kMaxViewTypeSize = 56;

}  // namespace vector
}  // namespace nebula
