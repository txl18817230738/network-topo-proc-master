// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <functional>

#include "nebula/common/base/Base.h"
#include "nebula/common/datatype/Value.h"
#include "nebula/common/service/Service.h"


namespace nebula {
class Utils final {
public:
    // Calculate the raft service address based on the storage service address
    static HostAddress getRaftAddrFromStoreAddr(const HostAddress& srvcAddr) {
        if (srvcAddr == HostAddress::nullAddr()) {
            return srvcAddr;
        }
        return {srvcAddr.host, srvcAddr.port + 1};
    }

    /**
     * @brief Increment the string lexcically.
     * For example, "abc" -> "abd", "ab\xff" -> "ac", "\xff\xff" -> "" (overflow)
     * @param str
     * @return true if the string overflowed
     */
    static bool incr(std::string& str) {
        int i = str.size() - 1;
        for (; i >= 0; --i) {
            str[i] += 1;
            if (str[i] != '\0') {
                break;
            }
        }
        // earse all of the `\0` after i
        str.erase(i + 1);
        return i < 0;
    }
};

/**
 * @brief NodeID related methods
 *
 * NodeID is a unique id within a graph. It contains three components:
 * NodeTypeID(2 bytes) + BucketID(2 bytes) + SequenceNumber(4 bytes)
 */
class NodeIDUtils final {
public:
    /**
     * @brief Extract node type from node id
     */
    static NodeTypeID nodeType(NodeID id) {
        return (id & kNodeTypeMask) >> 48;
    }
    /**
     * @brief Substitute node type in node id
     */
    static NodeID substituteNodeType(NodeID id, NodeTypeID type) {
        return (id & ~kNodeTypeMask) | (static_cast<NodeID>(type) << 48);
    }

    /**
     * @brief Extract bucket id from node id
     */
    static BucketID bucket(NodeID id) {
        return (id & kBucketMask) >> 32;
    }

    /**
     * @brief Substitute bucket id in node id
     */
    static NodeID substituteBucket(NodeID id, BucketID bucket) {
        return (id & ~kBucketMask) | (static_cast<NodeID>(bucket) << 32);
    }

    /**
     * @brief Extract node sequence id from node id
     */
    static NodeSeqID nodeSeqID(NodeID id) {
        return (id & kNodeSeqIDMask);
    }

    /**
     * @brief Substitute node sequence id in node id
     */
    static NodeID substituteNodeSeqID(NodeID id, NodeSeqID seq) {
        return (id & ~kNodeSeqIDMask) | seq;
    }


    /**
     * @brief Build nodeId based on its components: node type, bucket id, node seq no
     */
    static NodeID toNodeID(NodeTypeID type, BucketID bucket, NodeSeqID seq) {
        return (static_cast<NodeID>(type) << 48) | (static_cast<NodeID>(bucket) << 32) | seq;
    }

private:
    static_assert(sizeof(NodeID) == sizeof(uint64_t));
    static constexpr uint64_t kNodeTypeMask = 0xFFFF000000000000;
    static constexpr uint64_t kBucketMask = 0x0000FFFF00000000;
    static constexpr uint64_t kNodeSeqIDMask = 0x00000000FFFFFFFF;
};

class BucketUtils final {
public:
    template <typename Alloc = std::allocator<Value>>
    static BucketID genBucketId(const std::vector<Value, Alloc>& pkValues) {
        size_t seed = hashContainer(pkValues);
        seed = ((seed >> 16) ^ seed) * 0x119de1f3;
        seed = ((seed >> 16) ^ seed) * 0x119de1f3;
        seed = (seed >> 16) ^ seed;
        return static_cast<BucketID>(seed);
    }
};

}  // namespace nebula
