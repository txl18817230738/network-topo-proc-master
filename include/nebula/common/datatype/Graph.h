// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/base/Base.h"
#include "nebula/common/base/Status.h"
#include "nebula/common/datatype/Edge.h"
#include "nebula/common/datatype/Node.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"

namespace nebula {

// This is a temporaray impl. for Graph. Graph is to be replaced with GraphImpl later. We use
// GraphImpl for the time being to avoid changing too many non-graph codes, which may cause
// conflicts with other engineering tasks going on.
class GraphImpl {
public:
    GraphImpl() = default;
    virtual ~GraphImpl() = default;

    virtual bool insertNode(const Node& node) = 0;
    virtual bool insertEdge(const Edge& edge) = 0;
    virtual size_t numNodes() const = 0;
    virtual size_t numEdges() const = 0;
};

class Graph {
public:
    Graph() = default;

    std::string toString() const {
        return "";
    }
};

inline std::ostream& operator<<(std::ostream& os, const Graph& graph) {
    return os << graph.toString();
}

bool operator==(const Graph& lhs, const Graph& rhs);
bool operator!=(const Graph& lhs, const Graph& rhs);
bool operator<(const Graph& lhs, const Graph& rhs);

template <>
struct nrpc::BufferReaderWriter<nebula::Graph> {
    static void write(folly::IOBuf* buf, const nebula::Graph& graph) {
        UNUSED(buf);
        UNUSED(graph);
    }

    static Status read(folly::IOBuf* buf, nebula::Graph* graph) {
        UNUSED(buf);
        UNUSED(graph);
        return Status(ErrorCode::UNSUPPORTED, "Not implemented");
    }

    static size_t encodedSize(const nebula::Graph& graph) {
        UNUSED(graph);
        return 0;
    }
};

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::Graph> {
    std::size_t operator()(const nebula::Graph& graph) const noexcept {
        UNUSED(graph);
        DLOG(FATAL) << "Not implemented";
        return 0;
    }
};

}  // namespace std
