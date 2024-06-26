// Copyright (c) 2022 vesoft inc. All rights reserved.
#pragma once
#include <cstdint>
#include <string>

namespace nebula::catalog {

using NodeTypeID = int16_t;
using EdgeTypeID = int32_t;
using IndexID = int32_t;
using SchemaID = int64_t;
using GraphTypeID = int64_t;
using GraphID = int32_t;
using PropertyID = int64_t;
using CatalogVersion = int64_t;
using Label = std::string;
using CatalogJobID = int32_t;
using Edition = int64_t;
extern const int16_t kInvalidID;
static constexpr int64_t kMinReservedTypeID{0};
static constexpr int64_t kMaxReservedTypeID{1023};
extern const int64_t kInitID;
extern const int64_t kInvalidVersion;

/**
 * @brief kUnique: Parallel edges are not allowed. Rank is always zero.
 *        kAuto: Rank is generated automatically.
 *        kSimpleProp: Rank is represented by a single prop, and less than 8 bytes.
 *        kMultiProps: Rank is represented by multiple props.
 */
enum class RankType : uint8_t {
    kUnique = 0,
    kAuto = 1,
    kSimpleProp = 2,
    kMultiProps = 3,
};

enum class IndexType : uint8_t {
    kNormal = 0,
    kFullText = 1,
    kVector = 2,
    kSpatial = 3,
};

// forward declaration
class Graph;
class GraphType;
class Schema;
class ElementType;
class NodeType;
class EdgeType;
class PatternMacro;
class Procedure;
class Constant;
class Library;
class Property;
class PropertyType;
class IndexProperty;
class Index;
class NodeIndex;
class EdgeIndex;
}  // namespace nebula::catalog
