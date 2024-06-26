// Copyright (c) 2023 vesoft inc. All rights reserved.
#pragma once

#include <sys/types.h>

#include <cstdint>
#include <vector>

#include "nebula/catalog/element/type.h"
#include "nebula/common/base/ErrorCode.h"

namespace nebula {

using ClusterID = int64_t;
using ServiceClusterID = ClusterID;
using ServiceID = int64_t;

constexpr ServiceID kInvalidServiceID = 0;
constexpr ClusterID kInvalidClusterID = -1;

/**
 * @brief
 *
 * GraphID: It is **NOT** same as spaceId. Multiple graph with different graphId could be saved
 *          in same space. Besides, cloned graph, subgraph and view which origins from a graph
 *          will be saved in the same graph space with different GraphID as well.
 * PartID: partition id when a graph is distributed to multiple shard
 * NodeTypeID/EdgeTypeID: schema id from catalog
 * NodeID: the primary key will be mapped to a unique integer id. It consists of 3
 * components: NodeTypeID(2 bytes) + BucketID(2 bytes) + Sequence number(4 bytes) EdgeRank: edge
 * rank in big endian to support multi graph, encoding in big endian Version: version number to
 * support mvcc, encoding in big endian, and the order of version is same as lexicographical
 * order
 */


using SchemaID = ::nebula::catalog::SchemaID;
using GraphTypeID = ::nebula::catalog::GraphTypeID;
using GraphID = ::nebula::catalog::GraphID;  // Only least three significant bytes will be used
using QueryID = std::string;
using PartID = uint32_t;
using NodeID = int64_t;  // NodeID consists of NodeTypeID + BucketID + NodeSeqID
using BucketID = uint16_t;
using NodeSeqID = uint32_t;
using NodeTypeID = ::nebula::catalog::NodeTypeID;
using EdgeTypeID = ::nebula::catalog::EdgeTypeID;
// Used to be compatible with two types of IDs
using ElementTypeID =
        std::conditional_t<sizeof(NodeTypeID) >= sizeof(EdgeTypeID), NodeTypeID, EdgeTypeID>;
using PropertyID = ::nebula::catalog::PropertyID;
using IndexID = ::nebula::catalog::IndexID;
using Label = ::nebula::catalog::Label;
using EdgeRank = int64_t;
using Version = uint64_t;
using Port = uint32_t;
using CatalogVersion = ::nebula::catalog::CatalogVersion;
using AuthIdentifier = uint64_t;  // sth like session id, from session.h
using UserID = std::string;
using RoleID = int32_t;
using Label = ::nebula::catalog::Label;
using CatalogJobID = ::nebula::catalog::CatalogJobID;
using JobID = int64_t;
static constexpr JobID kFirstJobId = 1;
using TaskID = int32_t;
using RaftKey = uint64_t;
using MemoryLimit = int64_t;

// raft related
using TermID = int64_t;
using LogID = int64_t;
using ClusterID = int64_t;
using Epoch = int64_t;

// reserved property names
namespace reserved {
constexpr const char kNodeTypeId[] = "_ntId";
constexpr const char kId[] = "_id";
constexpr const char kEdgeTypeId[] = "_etId";
constexpr const char kSrc[] = "_src";
constexpr const char kDst[] = "_dst";
constexpr const char kRank[] = "_rank";
// kEid is composed of edge type id, src, dst and rank
constexpr const char kEid[] = "_eid";
}  // namespace reserved

// default datetime format
static const char kDefaultDatetimeFormat[] = "%Y-%m-%dT%H:%M:%S";
// the following formats must be consistent with the above one (e.g., no reordering of elements)
static const char kDefaultZonedDtFormat[] = "%Y-%m-%dT%H:%M:%S %z";
static const char kDefaultLocalTimeFormat[] = "%H:%M:%S";
static const char kDefaultZonedTimeFormat[] = "%H:%M:%S %z";
static const char kDefaultDateFormat[] = "%Y-%m-%d";

// column family name of RocksDB
static const char kDefaultCf[] = "default";      // used for meta, or some system info
static const char kNodeCf[] = "node";            // node data
static const char kEdgeCf[] = "edge";            // edge data
static const char kPrimaryCf[] = "primary";      // primary key index
static const char kSecondaryCf[] = "secondary";  // secondary key index

// for raft and restore
static const char kRaftApplyIndex[] = "__raft_apply_index__";
static const char kRaftConfig[] = "__raft_config__";

// authentication
static const char kRootUsername[] = "root";
static const char kRootDefaultPassword[] = "nebula";

// authorization
static constexpr RoleID kFirstRoleID = 1;
static constexpr int64_t kFirstRoleVersion = 1;
static constexpr int64_t kFirstUserVersion = 1;

// The enum value is also the column family handle index when open rocksdb.
// Noticed that the 4 bits high is reserved for data version.
enum class NebulaKeyType : uint8_t {
    kSystem = 0x00,     // For system use
    kNode = 0x01,       // For node
    kEdge = 0x02,       // For edge
    kPrimary = 0x03,    // For mapping from primary key to node id
    kSecondary = 0x04,  // For mapping from node type to node id
    kKeyValue = 0x05,   // For test only, kv will be saved in same cf of kSystem
};

// All keys of NebulaKeyType::kSystem contains SystemKeySubType as the second byte
enum class SystemKeySubType : uint8_t {
    kSystemPart = 0x01,             // Partition id in storage
    kSystemPeers = 0x02,            // Raft peers of each partitions
    kSystemSeqID = 0x03,            // Next node seq id of each (graph + bucket)
    kSystemRaftConfirmedId = 0x04,  // For raft's confirmed log id
    kSystemRaftAppliedId = 0x05,    // For raft's applied log id
    kSystemRaftCurrTerm = 0x06,     // For raft's current term
    kSystemRaftVotedFor = 0x07,     // For raft's voted for
};

struct PartitionList {
    std::vector<PartID> partList;
};

}  // namespace nebula
