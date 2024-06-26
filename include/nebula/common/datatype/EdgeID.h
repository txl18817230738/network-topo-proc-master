// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/nrpc/BufferReaderWriter.h"
#include "nebula/common/utils/EdgeUtils.h"
#include "nebula/common/utils/Types.h"

namespace nebula {

// The quadruple will identify a unique Edge in the graph
struct EdgeID {
    NodeID srcID{-1};
    NodeID dstID{-1};
    EdgeRank rank{-1};
    GraphID graphID{-1};
    EdgeTypeID edgeTypeID{-1};

    void toUndirected() {
        edgeTypeID = EdgeUtils::undirected0(EdgeUtils::removeDirection(edgeTypeID));
    }

    void reverse() {
        std::swap(srcID, dstID);
        edgeTypeID = EdgeUtils::reverse(edgeTypeID);
    }

    // After standardization, the edge id could compare byte by byte to represent comparision of
    // Edge
    void standardization() {
        if (EdgeUtils::isOutgoing(edgeTypeID)) {
            edgeTypeID = EdgeUtils::removeDirection(edgeTypeID);
        } else if (EdgeUtils::isIncoming(edgeTypeID)) {
            edgeTypeID = EdgeUtils::removeDirection(edgeTypeID);
            // reverse src and dst
            std::swap(srcID, dstID);
        } else {
            // undirected
            edgeTypeID = EdgeUtils::undirected0(EdgeUtils::removeDirection(edgeTypeID));
            if (srcID >= dstID) {
                std::swap(srcID, dstID);
            }
        }
    }

    std::string toString() const {
        std::stringstream os;
        std::string edgeDirection;
        if (EdgeUtils::isOutgoing(edgeTypeID)) {
            edgeDirection = "->";
        } else if (EdgeUtils::isIncoming(edgeTypeID)) {
            edgeDirection = "<-";
        } else {
            edgeDirection = "~";
        }
        os << "(" << srcID << edgeDirection << dstID << " " << rank << "@" << edgeTypeID << ")";
        return os.str();
    }

    bool operator==(const EdgeID& rhs) const {
        if (graphID != rhs.graphID) {
            return false;
        }
        // TODO(yuxuan.wang) maybe compare two temprary standardized EdgeID is more faster.
        bool isLhsDirected = EdgeUtils::isDirected(edgeTypeID);
        bool isRhsDirected = EdgeUtils::isDirected(rhs.edgeTypeID);
        // If one edge is directed and the other is not, they cannot be equal
        if (isLhsDirected != isRhsDirected) {
            return false;
        }
        // Check if the edge type IDs are the same, ignoring the direction
        if (EdgeUtils::removeDirection(edgeTypeID) !=
            EdgeUtils::removeDirection(rhs.edgeTypeID)) {
            return false;
        }
        // Check if the edge ranks are the same
        if (rank != rhs.rank) {
            return false;
        }
        // Checks if two directed edges are equal
        if (isLhsDirected) {
            if (EdgeUtils::isOutgoing(edgeTypeID) == EdgeUtils::isOutgoing(rhs.edgeTypeID)) {
                return srcID == rhs.srcID && dstID == rhs.dstID;
            } else {
                return srcID == rhs.dstID && dstID == rhs.srcID;
            }
        }
        // Checks if two undirected edges are equal
        return (srcID == rhs.srcID && dstID == rhs.dstID) ||
               (srcID == rhs.dstID && dstID == rhs.srcID);
    }

    bool operator!=(const EdgeID& rhs) const {
        return !(*this == rhs);
    }

    bool operator<(const EdgeID& rhs) const {
        if (graphID != rhs.graphID) {
            return graphID < rhs.graphID;
        }
        if (srcID != rhs.srcID) {
            return srcID < rhs.srcID;
        }
        if (dstID != rhs.dstID) {
            return dstID < rhs.dstID;
        }
        if (edgeTypeID != rhs.edgeTypeID) {
            return edgeTypeID < rhs.edgeTypeID;
        }
        if (rank != rhs.rank) {
            return rank < rhs.rank;
        }
        return false;
    }

    bool operator>(const EdgeID& rhs) const {
        return !(*this < rhs || *this == rhs);
    }

    void clear() {
        srcID = -1;
        dstID = -1;
        rank = -1;
        graphID = -1;
        edgeTypeID = -1;
    }
};
static_assert(sizeof(EdgeID) == 32);

inline std::ostream& operator<<(std::ostream& os, const EdgeID& edgeID) {
    return os << edgeID.toString();
}

template <>
struct nrpc::BufferReaderWriter<nebula::EdgeID> {
    static void write(folly::IOBuf* buf, const nebula::EdgeID& edge) {
        BufferReaderWriter<NodeID>::write(buf, edge.srcID);
        BufferReaderWriter<NodeID>::write(buf, edge.dstID);
        BufferReaderWriter<EdgeRank>::write(buf, edge.rank);
        BufferReaderWriter<GraphID>::write(buf, edge.graphID);
        BufferReaderWriter<EdgeTypeID>::write(buf, edge.edgeTypeID);
    }

    static Status read(folly::IOBuf* buf, nebula::EdgeID* edge) {
        NG_RETURN_IF_ERROR(BufferReaderWriter<NodeID>::read(buf, &edge->srcID));
        NG_RETURN_IF_ERROR(BufferReaderWriter<NodeID>::read(buf, &edge->dstID));
        NG_RETURN_IF_ERROR(BufferReaderWriter<EdgeRank>::read(buf, &edge->rank));
        NG_RETURN_IF_ERROR(BufferReaderWriter<GraphID>::read(buf, &edge->graphID));
        return BufferReaderWriter<EdgeTypeID>::read(buf, &edge->edgeTypeID);
    }

    static size_t encodedSize(const nebula::EdgeID& edge) {
        return BufferReaderWriter<NodeID>::encodedSize(edge.srcID) +
               BufferReaderWriter<NodeID>::encodedSize(edge.dstID) +
               BufferReaderWriter<EdgeRank>::encodedSize(edge.rank) +
               BufferReaderWriter<GraphID>::encodedSize(edge.graphID) +
               BufferReaderWriter<EdgeTypeID>::encodedSize(edge.edgeTypeID);
    }
};

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::EdgeID> {
    size_t operator()(const nebula::EdgeID& e) const {
        auto edgeID = e;
        edgeID.standardization();
        return folly::hash::hash_combine(
                edgeID.graphID, edgeID.edgeTypeID, edgeID.srcID, edgeID.dstID, edgeID.rank);
    }
};

}  // namespace std
