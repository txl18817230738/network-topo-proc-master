// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>

#include "arrow/type_fwd.h"
#include "nebula/catalog/element/type.h"
#include "nebula/common/memory/StlAllocator.h"
#include "nebula/common/utils/Types.h"

namespace nebula {

using String = std::basic_string<char, std::char_traits<char>, memory::StlAllocator<char>>;

template <typename T, typename Alloc = memory::StlAllocator<T>>
using NVector = std::vector<T, Alloc>;

template <typename T, typename Alloc = memory::StlAllocator<T>>
using NSet = std::unordered_set<T, std::hash<T>, std::equal_to<>, Alloc>;

template <typename K, typename V, typename Alloc = memory::StlAllocator<std::pair<const K, V>>>
using NMap = std::unordered_map<K, V, std::hash<K>, std::equal_to<>, Alloc>;

class NullValue;
class Node;
class Edge;
class List;
struct LocalTime;
struct ZonedTime;
struct LocalDatetime;
struct ZonedDatetime;
struct Duration;
struct Date;
class Value;
class BindingTable;
class Ref;
class Path;
class Record;
struct EdgeID;
struct Nothing;
class NodeView;
class EdgeView;

enum ValueTypeKind : uint8_t {
    // predefined types
    kNode = 0x1,  // may refer to multiple NodeTypes
    kEdge = 0x2,  // may refer to multiple EdgeTypes
    kNull = 0x3,
    kBool = 0x4,
    // encode order of numeric type, don't change them
    kInt8 = 0x5,
    kUint8 = 0x6,
    kInt16 = 0x7,
    kUint16 = 0x8,
    kInt32 = 0x9,
    kUint32 = 0xA,
    kInt64 = 0xB,
    kUint64 = 0xC,
    kFloat32 = 0xD,
    kFloat64 = 0xE,
    kBytes = 0xF,
    kString = 0x10,  // TODO support real character string type

    // constructed types
    kList = 0x11,
    kPath = 0x12,
    kRecord = 0x13,

    // Temporal types
    // TODO(Aiee) add kTime, kDatetime which contains timezone
    // kTime = 1UL << 20,
    kLocalTime = 0x15,  // Time without timezone
    kDuration = 0x16,
    kDate = 0x17,
    // kDateTime = 1UL << 24,
    kLocalDatetime = 0x18,  // Datetime without timezone
    kZonedTime = 0x19,      // time with timezone
    kZonedDatetime = 0x20,  // Datetime with timezone

    kRef = 0x21,  // TODO(Aiee) Used as graph reference now

    // types won't instance a real value ever
    kFunction = 0xFD,
    kAny = 0xFE,
    // 0xFF is reserved to indicate a byte in the memory as initialized but not used to
    // represent any valid data type.
    // do not chagne it.
    kInvalid = 0xFF,
};

enum RefKind : uint8_t {
    // a node stored in vector::FlatVector<kNode>
    refNode = 0x01,

    // an edge stored in vector::FlatVector<kEdge>
    refEdge = 0x02,

    // the native type Graph and other data structures that could store a graph
    refGraph = 0x10,
    refMemGraph = 0x11,
    refParallelMemGraph = 0x12,

    // the native type BindingTable and other data structures that could store a
    // BindingTable
    refTable = 0x20,
    refRowTable = 0x21,
    refVectorTable = 0x22,

    refInVec = 0xFE,
    refInvalid = 0xFF
};
// refKind to string
inline std::string enum2String(RefKind kind) {
    switch (kind) {
        case RefKind::refNode:
            return "refNode";
        case RefKind::refEdge:
            return "refEdge";
        case RefKind::refGraph:
            return "refGraph";
        case RefKind::refMemGraph:
            return "refMemGraph";
        case RefKind::refParallelMemGraph:
            return "refParallelMemGraph";
        case RefKind::refTable:
            return "refTable";
        case RefKind::refRowTable:
            return "refRowTable";
        case RefKind::refVectorTable:
            return "refVectorTable";
        case RefKind::refInVec:
            return "refInVec";
        case RefKind::refInvalid:
            return "refInvalid";
        default:
            return "unknown";
    }
}

inline std::ostream& operator<<(std::ostream& os, RefKind kind) {
    return os << enum2String(kind);
}

// types used in Vector
// Views
class StringView;
namespace vector {
// ElementViews for complex types
class ListView;
class RecordView;
class PathView;
class DynamicView;
class NullView;

// ElementWriters for complex types
class StringWriter;
class ListWriter;
class RecordWriter;
class ScalarWriter;
class NodeWriter;
class EdgeWriter;
class PathWriter;
class NullWriter;
template <ValueTypeKind T, typename enable = void>
class ParallelWriter;

// Headers for complex types
struct ListHeader;
union NodeHeader;
union EdgeHeader;
struct PathHeader;
union ScalarData;
struct MutexNodeHeader;
struct MutexEdgeHeader;
}  // namespace vector

// SortOrder is a little heavy, internally use this simple Compare Flags describes value
// collation in comparison.
struct CompareFlags {
    bool nullsFirst = true;
    bool ascending = true;
};

template <ValueTypeKind T>
struct TypeTraits {};

template <typename T, T v, T... args>
constexpr bool value_in = ((v == args) || ...);

#define DEFINE_TYPE_PREDICATE(type, typePred, ...)                      \
    template <type v>                                                   \
    struct predicate##_struct {                                         \
        static constexpr bool value = value_in<type, v, ##__VA_ARGS__>; \
    };                                                                  \
    template <type v>                                                   \
    constexpr bool typePred = predicate##_struct<v>::value;

// FIXME(czp): Unify TypeTraits and CppTypeTraits after type system ready
template <typename T>
struct CppTypeTraits {
    static constexpr bool isDataType = false;
    static constexpr bool isPrimitiveType = false;
};

template <typename T>
struct TemporalTypeTraits {
    static constexpr bool value = false;
};

template <typename T>
constexpr auto is_temporal_v = TemporalTypeTraits<T>::value;

/*
 * NativeType: Stand alone type that takes ownership (owns the memory). It supports full
 *             operations of the type. e.g. String
 * MemoryType: Fixed-width header type in Vector. It contains some summary information about the
 *             type, which may be useful when we don't need the full content. e.g. 16B string
 *             header (StringView)
 * ViewType:   Light-weight, read-only view type. It uses pointer to point to the memory. The
 *             real object may be stored in any place (Vector in most cases). e.g. StringView
 * WriterType: Light-weight, writing handler type. It also uses pointer, but it provides writing
 *             methods to the underlying object. e.g. StringWriter
 */

#define CHECK_IF_TYPE_IS_TRIVIAL                                                    \
    std::is_same_v<NativeType, ViewType>&& std::is_same_v<NativeType, WriterType>&& \
            std::is_same_v<NativeType, MemoryType>

#define CHECK_IF_TYPE_IS_PLAIN std::is_same_v<ViewType, MemoryType>

#define DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(T)                              \
    template <>                                                              \
    struct CppTypeTraits<typename TypeTraits<ValueTypeKind::k##T>::ViewType> \
            : public TypeTraits<ValueTypeKind::k##T> {}

#define DEFINE_CPP_TYPE_TRAITS_FOR_WRITER_TYPE(T)                              \
    template <>                                                                \
    struct CppTypeTraits<typename TypeTraits<ValueTypeKind::k##T>::WriterType> \
            : public TypeTraits<ValueTypeKind::k##T> {}

// TODO(wuu): remove `ArrowViewType`
// TODO(wuu): remove CppTypeTraits for `NativeType`, e.g. CppTypeTraits<Node>

template <>
struct TypeTraits<ValueTypeKind::kNode> {
    using NativeType = Node;
    using ViewType = NodeView;
    using WriterType = vector::NodeWriter;
    using ParallelWriterType = vector::ParallelWriter<ValueTypeKind::kNode>;
    using MemoryType = vector::NodeHeader;
    using MutexMemoryType = vector::MutexNodeHeader;
    using UniqueID = NodeID;
    using TypeID = catalog::NodeTypeID;
    using ElementType = catalog::NodeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kNode;
    static constexpr RefKind refTypeKind = RefKind::refNode;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = false;
    static constexpr bool isFixedWidth = false;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "Node";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = Node;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Node);
DEFINE_CPP_TYPE_TRAITS_FOR_WRITER_TYPE(Node);

template <>
struct CppTypeTraits<Node> : public TypeTraits<ValueTypeKind::kNode> {};

template <>
struct TypeTraits<ValueTypeKind::kRef> {
    using NativeType = Ref;
    using ViewType = NativeType;
    using WriterType = NativeType;
    // using ParallelWriterType = vector::ParallelWriter<ValueTypeKind::kNode>;
    using MemoryType = NativeType;
    // using MutexMemoryType = vector::MutexNodeHeader;
    // using UniqueID = NodeID;
    // using TypeID = catalog::NodeTypeID;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kRef;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = false;
    static constexpr bool isFixedWidth = false;
    // static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "Ref";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = NativeType;
};

template <>
struct CppTypeTraits<Ref> : public TypeTraits<ValueTypeKind::kRef> {};

template <>
struct TypeTraits<ValueTypeKind::kEdge> {
    using NativeType = Edge;
    using ViewType = EdgeView;
    using WriterType = vector::EdgeWriter;
    using ParallelWriterType = vector::ParallelWriter<ValueTypeKind::kEdge>;
    using MemoryType = vector::EdgeHeader;
    using MutexMemoryType = vector::MutexEdgeHeader;
    using UniqueID = EdgeID;
    using TypeID = catalog::EdgeTypeID;
    using ElementType = catalog::EdgeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kEdge;
    static constexpr RefKind refTypeKind = RefKind::refEdge;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = false;
    static constexpr bool isFixedWidth = false;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "Edge";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = Edge;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Edge);
DEFINE_CPP_TYPE_TRAITS_FOR_WRITER_TYPE(Edge);

template <>
struct CppTypeTraits<Edge> : public TypeTraits<ValueTypeKind::kEdge> {};

template <>
struct TypeTraits<ValueTypeKind::kBool> {
    using NativeType = bool;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kBool;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "BOOL";
    static constexpr NativeType min = std::numeric_limits<bool>::min();
    static constexpr NativeType max = std::numeric_limits<bool>::max();
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::BooleanBuilder;
    using ArrowArray = arrow::BooleanArray;
    using ArrowViewType = bool;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Bool);

template <>
struct TypeTraits<ValueTypeKind::kInt8> {
    using NativeType = int8_t;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kInt8;
    static constexpr bool isNumericType = true;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "INT8";
    static constexpr NativeType min = std::numeric_limits<int8_t>::min();
    static constexpr NativeType max = std::numeric_limits<int8_t>::max();
    static constexpr std::size_t precision = 7;
    static constexpr std::size_t scale = 0;
    static constexpr bool kSigned = true;
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::Int8Builder;
    using ArrowArray = arrow::Int8Array;
    using ArrowViewType = int8_t;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Int8);

template <>
struct TypeTraits<ValueTypeKind::kInt16> {
    using NativeType = int16_t;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kInt16;
    static constexpr bool isNumericType = true;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "INT16";
    static constexpr NativeType min = std::numeric_limits<int16_t>::min();
    static constexpr NativeType max = std::numeric_limits<int16_t>::max();
    static constexpr std::size_t precision = 15;
    static constexpr std::size_t scale = 0;
    static constexpr bool kSigned = true;
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::Int16Builder;
    using ArrowArray = arrow::Int16Array;
    using ArrowViewType = int16_t;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Int16);

template <>
struct TypeTraits<ValueTypeKind::kInt32> {
    using NativeType = int32_t;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kInt32;
    static constexpr bool isNumericType = true;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "INT32";
    static constexpr NativeType min = std::numeric_limits<int32_t>::min();
    static constexpr NativeType max = std::numeric_limits<int32_t>::max();
    static constexpr std::size_t precision = 31;
    static constexpr std::size_t scale = 0;
    static constexpr bool kSigned = true;
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::Int32Builder;
    using ArrowArray = arrow::Int32Array;
    using ArrowViewType = int32_t;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Int32);

template <>
struct TypeTraits<ValueTypeKind::kInt64> {
    using NativeType = int64_t;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kInt64;
    static constexpr bool isNumericType = true;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "INT64";
    static constexpr NativeType min = std::numeric_limits<int64_t>::min();
    static constexpr NativeType max = std::numeric_limits<int64_t>::max();
    static constexpr std::size_t precision = 63;
    static constexpr std::size_t scale = 0;
    static constexpr bool kSigned = true;
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::Int64Builder;
    using ArrowArray = arrow::Int64Array;
    using ArrowViewType = int64_t;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Int64);

template <>
struct TypeTraits<ValueTypeKind::kUint8> {
    using NativeType = uint8_t;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kUint8;
    static constexpr bool isNumericType = true;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "UINT8";
    static constexpr NativeType min = std::numeric_limits<uint8_t>::min();
    static constexpr NativeType max = std::numeric_limits<uint8_t>::max();
    static constexpr std::size_t precision = 7;
    static constexpr std::size_t scale = 0;
    static constexpr bool kSigned = false;
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::UInt8Builder;
    using ArrowArray = arrow::UInt8Array;
    using ArrowViewType = uint8_t;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Uint8);

template <>
struct TypeTraits<ValueTypeKind::kUint16> {
    using NativeType = uint16_t;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kUint16;
    static constexpr bool isNumericType = true;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "UINT16";
    static constexpr NativeType min = std::numeric_limits<uint16_t>::min();
    static constexpr NativeType max = std::numeric_limits<uint16_t>::max();
    static constexpr std::size_t precision = 15;
    static constexpr std::size_t scale = 0;
    static constexpr bool kSigned = false;
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::UInt16Builder;
    using ArrowArray = arrow::UInt16Array;
    using ArrowViewType = uint16_t;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Uint16);

template <>
struct TypeTraits<ValueTypeKind::kUint32> {
    using NativeType = uint32_t;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kUint32;
    static constexpr bool isNumericType = true;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "UINT32";
    static constexpr NativeType min = std::numeric_limits<uint32_t>::min();
    static constexpr NativeType max = std::numeric_limits<uint32_t>::max();
    static constexpr std::size_t precision = 31;
    static constexpr std::size_t scale = 0;
    static constexpr bool kSigned = false;
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::UInt32Builder;
    using ArrowArray = arrow::UInt32Array;
    using ArrowViewType = uint32_t;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Uint32);

template <>
struct TypeTraits<ValueTypeKind::kUint64> {
    using NativeType = uint64_t;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kUint64;
    static constexpr bool isNumericType = true;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "UINT64";
    static constexpr NativeType min = std::numeric_limits<uint64_t>::min();
    static constexpr NativeType max = std::numeric_limits<uint64_t>::max();
    static constexpr std::size_t precision = 63;
    static constexpr std::size_t scale = 0;
    static constexpr bool kSigned = false;
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::UInt64Builder;
    using ArrowArray = arrow::UInt64Array;
    using ArrowViewType = uint64_t;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Uint64);


template <>
struct TypeTraits<ValueTypeKind::kFloat32> {
    using NativeType = float;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kFloat32;
    static constexpr bool isNumericType = true;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "FLOAT";
    static constexpr NativeType min = std::numeric_limits<float>::min();
    static constexpr NativeType max = std::numeric_limits<float>::max();
    static constexpr std::size_t precision = 23;
    static constexpr std::size_t scale = 8;
    static constexpr bool kSigned = true;
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FloatBuilder;
    using ArrowArray = arrow::FloatArray;
    using ArrowViewType = float;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Float32);

template <>
struct TypeTraits<ValueTypeKind::kFloat64> {
    using NativeType = double;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kFloat64;
    static constexpr bool isNumericType = true;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "DOUBLE";
    static constexpr NativeType min = std::numeric_limits<double>::min();
    static constexpr NativeType max = std::numeric_limits<double>::max();
    static constexpr std::size_t precision = 52;
    static constexpr std::size_t scale = 11;
    static constexpr bool kSigned = true;
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::DoubleBuilder;
    using ArrowArray = arrow::DoubleArray;
    using ArrowViewType = double;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Float64);

template <>
struct TypeTraits<ValueTypeKind::kBytes> {
    using NativeType = String;
    using ViewType = StringView;
    using WriterType = vector::StringWriter;
    using MemoryType = StringView;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kBytes;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = false;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "BYTES";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::BinaryBuilder;
    using ArrowArray = arrow::BinaryArray;
    using ArrowViewType = std::string_view;
};

template <>
struct TypeTraits<ValueTypeKind::kString> {
    using NativeType = String;
    using ViewType = StringView;
    using WriterType = vector::StringWriter;
    using MemoryType = StringView;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kString;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = false;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "STRING";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::StringBuilder;
    using ArrowArray = arrow::StringArray;
    using ArrowViewType = std::string_view;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(String);
DEFINE_CPP_TYPE_TRAITS_FOR_WRITER_TYPE(String);

template <>
struct CppTypeTraits<std::string_view> : public TypeTraits<ValueTypeKind::kString> {};

template <>
struct CppTypeTraits<std::string> : public TypeTraits<ValueTypeKind::kString> {};

template <>
struct CppTypeTraits<String> : public TypeTraits<ValueTypeKind::kString> {};

template <>
struct TypeTraits<ValueTypeKind::kList> {
    using NativeType = List;
    using ViewType = vector::ListView;
    using WriterType = vector::ListWriter;
    using MemoryType = vector::ListHeader;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kList;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = false;
    static constexpr bool isFixedWidth = false;
    static constexpr bool hasRefGetter = true;
    // for duckdb we only test Int list for now
    static constexpr const char* name = "LIST";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = List;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(List);
DEFINE_CPP_TYPE_TRAITS_FOR_WRITER_TYPE(List);

template <>
struct CppTypeTraits<List> : public TypeTraits<ValueTypeKind::kList> {};

template <>
struct TypeTraits<ValueTypeKind::kPath> {
    using NativeType = Path;
    using ViewType = vector::PathView;
    using WriterType = vector::PathWriter;
    using MemoryType = vector::PathHeader;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kPath;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = false;
    static constexpr bool isFixedWidth = false;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "Path";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = Path;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Path);
DEFINE_CPP_TYPE_TRAITS_FOR_WRITER_TYPE(Path);

template <>
struct CppTypeTraits<Path> : public TypeTraits<ValueTypeKind::kPath> {};

template <>
struct TypeTraits<ValueTypeKind::kRecord> {
    using NativeType = Record;
    using ViewType = vector::RecordView;
    using WriterType = vector::RecordWriter;
    using MemoryType = folly::Unit;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kRecord;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = false;
    static constexpr bool isFixedWidth = false;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "RECORD";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = Record;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Record);
DEFINE_CPP_TYPE_TRAITS_FOR_WRITER_TYPE(Record);

template <>
struct CppTypeTraits<Record> : public TypeTraits<ValueTypeKind::kRecord> {};

template <>
struct TypeTraits<ValueTypeKind::kLocalTime> {
    using NativeType = LocalTime;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kLocalTime;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "LocalTime";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = LocalTime;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(LocalTime);

template <>
struct TemporalTypeTraits<LocalTime> {
    static constexpr bool value = true;
};

template <>
struct TypeTraits<ValueTypeKind::kZonedTime> {
    using NativeType = ZonedTime;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kZonedTime;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "ZonedTime";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = ZonedTime;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(ZonedTime);

template <>
struct TemporalTypeTraits<ZonedTime> {
    static constexpr bool value = true;
};

template <>
struct TypeTraits<ValueTypeKind::kDuration> {
    using NativeType = Duration;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kDuration;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "Duration";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = Duration;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Duration);

template <>
struct TemporalTypeTraits<Duration> {
    static constexpr bool value = true;
};

template <>
struct TypeTraits<ValueTypeKind::kLocalDatetime> {
    using NativeType = LocalDatetime;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kLocalDatetime;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "LocalDatetime";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = LocalDatetime;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(LocalDatetime);

template <>
struct TemporalTypeTraits<LocalDatetime> {
    static constexpr bool value = true;
};

template <>
struct TypeTraits<ValueTypeKind::kZonedDatetime> {
    using NativeType = ZonedDatetime;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kZonedDatetime;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "ZonedDatetime";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = ZonedDatetime;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(ZonedDatetime);

template <>
struct TemporalTypeTraits<ZonedDatetime> {
    static constexpr bool value = true;
};


template <>
struct TypeTraits<ValueTypeKind::kDate> {
    using NativeType = Date;
    using ViewType = NativeType;
    using WriterType = NativeType;
    using MemoryType = NativeType;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kDate;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = true;
    static constexpr bool isFixedWidth = true;
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "Date";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = Date;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Date);

template <>
struct TemporalTypeTraits<Date> {
    static constexpr bool value = true;
};

template <>
struct TypeTraits<ValueTypeKind::kAny> {
    using NativeType = Value;
    using ViewType = vector::DynamicView;
    using WriterType = vector::ScalarWriter;
    using MemoryType = vector::ScalarData;
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kAny;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = false;
    static constexpr bool isFixedWidth = false;
    // FIXME(czp): How to handle get<Value>?
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "Any";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = Value;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Any);
DEFINE_CPP_TYPE_TRAITS_FOR_WRITER_TYPE(Any);

template <>
struct TypeTraits<ValueTypeKind::kNull> {
    using NativeType = NullValue;
    using ViewType = vector::NullView;
    using WriterType = vector::NullWriter;
    using MemoryType = folly::Unit;
    // Treat null as trivial type for now
    static constexpr bool isTrivial = CHECK_IF_TYPE_IS_TRIVIAL;
    static constexpr bool isPlain = CHECK_IF_TYPE_IS_PLAIN;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kNull;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = false;
    static constexpr bool isFixedWidth = true;
    // FIXME(czp): How to handle get<Value>?
    static constexpr bool hasRefGetter = true;
    static constexpr const char* name = "Null";
    static constexpr bool isDataType = true;
    using ArrowBuilder = arrow::FixedSizeBinaryBuilder;
    using ArrowArray = arrow::FixedSizeBinaryArray;
    using ArrowViewType = Value;
};
DEFINE_CPP_TYPE_TRAITS_FOR_VIEW_TYPE(Null);
DEFINE_CPP_TYPE_TRAITS_FOR_WRITER_TYPE(Null);

template <>
struct CppTypeTraits<NullValue> : public TypeTraits<ValueTypeKind::kNull> {};

template <>
struct TypeTraits<ValueTypeKind::kInvalid> {
    using NativeType = bool;
    static constexpr ValueTypeKind typeKind = ValueTypeKind::kInvalid;
    static constexpr bool isNumericType = false;
    static constexpr bool isPrimitiveType = false;
    static constexpr bool isFixedWidth = false;
    // FIXME(czp): How to handle get<Value>?
    static constexpr bool hasRefGetter = false;
    static constexpr const char* name = "Invalid";
    using ArrowBuilder = arrow::BooleanBuilder;
    using ArrowArray = arrow::BooleanArray;
    using ArrowViewType = bool;
};


inline std::string enum2String(ValueTypeKind type);

template <>
struct CppTypeTraits<nebula::Value> : public TypeTraits<ValueTypeKind::kAny> {};

template <typename T, typename = void>
class VariadicView;

// TODO(wuu): remove this
template <typename T>
struct CppTypeTraits<VariadicView<T>> {
    static constexpr ValueTypeKind typeKind = CppTypeTraits<T>::typeKind;
};

template <typename>
struct isVariadicType : public std::false_type {};

template <typename T>
struct isVariadicType<VariadicView<T>> : public std::true_type {};

#define NEBULA_DYNAMIC_TYPE_DISPATCH_IMPL(PREFIX, SUFFIX, typeKind, PARAM)            \
    [&]() {                                                                           \
        switch (typeKind) {                                                           \
            case ::nebula::ValueTypeKind::kNull: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kNull> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kNode: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kNode> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kEdge: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kEdge> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kBool: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kBool> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt8: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kInt8> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt16: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kInt16> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt32: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kInt32> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt64: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kInt64> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint8: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kUint8> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint16: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kUint16> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint32: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kUint32> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint64: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kUint64> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kFloat32: {                                 \
                return PREFIX<::nebula::ValueTypeKind::kFloat32> SUFFIX##PARAM;       \
            }                                                                         \
            case ::nebula::ValueTypeKind::kFloat64: {                                 \
                return PREFIX<::nebula::ValueTypeKind::kFloat64> SUFFIX##PARAM;       \
            }                                                                         \
            case ::nebula::ValueTypeKind::kBytes: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kBytes> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kString: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kString> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kList: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kList> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kPath: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kPath> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kRecord: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kRecord> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kLocalTime: {                               \
                return PREFIX<::nebula::ValueTypeKind::kLocalTime> SUFFIX##PARAM;     \
            }                                                                         \
            case ::nebula::ValueTypeKind::kZonedTime: {                               \
                return PREFIX<::nebula::ValueTypeKind::kZonedTime> SUFFIX##PARAM;     \
            }                                                                         \
            case ::nebula::ValueTypeKind::kDuration: {                                \
                return PREFIX<::nebula::ValueTypeKind::kDuration> SUFFIX##PARAM;      \
            }                                                                         \
            case ::nebula::ValueTypeKind::kDate: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kDate> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kLocalDatetime: {                           \
                return PREFIX<::nebula::ValueTypeKind::kLocalDatetime> SUFFIX##PARAM; \
            }                                                                         \
            case ::nebula::ValueTypeKind::kZonedDatetime: {                           \
                return PREFIX<::nebula::ValueTypeKind::kZonedDatetime> SUFFIX##PARAM; \
            }                                                                         \
            case ::nebula::ValueTypeKind::kRef: {                                     \
                return PREFIX<::nebula::ValueTypeKind::kRef> SUFFIX##PARAM;           \
            }                                                                         \
            case ::nebula::ValueTypeKind::kAny: {                                     \
                return PREFIX<::nebula::ValueTypeKind::kAny> SUFFIX##PARAM;           \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInvalid: {                                 \
                return PREFIX<::nebula::ValueTypeKind::kInvalid> SUFFIX##PARAM;       \
            }                                                                         \
            case ::nebula::ValueTypeKind::kFunction: {                                \
                DLOG(FATAL) << static_cast<int>(typeKind);                            \
                throw std::runtime_error("These types won't instance value ever.");   \
            }                                                                         \
        }                                                                             \
        DCHECK(false);                                                                \
        throw std::runtime_error("not a known type kind: {}");                        \
    }()

// TODO(Aiee) Delete after nebula vector is embedded
#define NEBULA_DYNAMIC_TYPE_DISPATCH(TEMPLATE_FUNC, typeKind, ...) \
    NEBULA_DYNAMIC_TYPE_DISPATCH_IMPL(TEMPLATE_FUNC, , typeKind, (__VA_ARGS__))

constexpr bool isPrimitiveType(ValueTypeKind type) {
    return NEBULA_DYNAMIC_TYPE_DISPATCH_IMPL(TypeTraits, ::isPrimitiveType, type, /*empty*/);
}

constexpr bool isNumericType(ValueTypeKind type) {
    return NEBULA_DYNAMIC_TYPE_DISPATCH_IMPL(TypeTraits, ::isNumericType, type, /*empty*/);
}

constexpr bool isFixedWidth(ValueTypeKind type) {
    return NEBULA_DYNAMIC_TYPE_DISPATCH_IMPL(TypeTraits, ::isFixedWidth, type, /*empty*/);
}

inline std::string enum2String(ValueTypeKind type) {
    return NEBULA_DYNAMIC_TYPE_DISPATCH_IMPL(TypeTraits, ::name, type, /*empty*/);
}

inline std::ostream& operator<<(std::ostream& os, ValueTypeKind type) {
    return os << enum2String(type);
}

#define NEBULA_DYNAMIC_PROPERTY_TYPE_DISPATCH_IMPL(PREFIX, SUFFIX, typeKind, PARAM)   \
    [&]() {                                                                           \
        switch (typeKind) {                                                           \
            case ::nebula::ValueTypeKind::kNull: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kNull> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kBool: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kBool> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt8: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kInt8> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt16: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kInt16> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt32: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kInt32> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt64: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kInt64> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint8: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kUint8> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint16: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kUint16> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint32: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kUint32> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint64: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kUint64> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kFloat32: {                                 \
                return PREFIX<::nebula::ValueTypeKind::kFloat32> SUFFIX##PARAM;       \
            }                                                                         \
            case ::nebula::ValueTypeKind::kFloat64: {                                 \
                return PREFIX<::nebula::ValueTypeKind::kFloat64> SUFFIX##PARAM;       \
            }                                                                         \
            case ::nebula::ValueTypeKind::kString: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kString> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kList: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kList> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kLocalTime: {                               \
                return PREFIX<::nebula::ValueTypeKind::kLocalTime> SUFFIX##PARAM;     \
            }                                                                         \
            case ::nebula::ValueTypeKind::kZonedTime: {                               \
                return PREFIX<::nebula::ValueTypeKind::kZonedTime> SUFFIX##PARAM;     \
            }                                                                         \
            case ::nebula::ValueTypeKind::kDuration: {                                \
                return PREFIX<::nebula::ValueTypeKind::kDuration> SUFFIX##PARAM;      \
            }                                                                         \
            case ::nebula::ValueTypeKind::kDate: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kDate> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kLocalDatetime: {                           \
                return PREFIX<::nebula::ValueTypeKind::kLocalDatetime> SUFFIX##PARAM; \
            }                                                                         \
            case ::nebula::ValueTypeKind::kZonedDatetime: {                           \
                return PREFIX<::nebula::ValueTypeKind::kZonedDatetime> SUFFIX##PARAM; \
            }                                                                         \
            case ::nebula::ValueTypeKind::kRecord: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kRecord> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kAny: {                                     \
                return PREFIX<::nebula::ValueTypeKind::kAny> SUFFIX##PARAM;           \
            }                                                                         \
            default: {                                                                \
                DCHECK(false) << "Type dispatch not implemented: "                    \
                              << std::to_string(typeKind);                            \
                throw std::runtime_error("Type dispatch not implemented: " +          \
                                         std::to_string(typeKind));                   \
            }                                                                         \
        }                                                                             \
    }()

#define NEBULA_DYNAMIC_PHYSICAL_TYPE_DISPATCH_IMPL(PREFIX, SUFFIX, typeKind, PARAM)   \
    [&]() {                                                                           \
        switch (typeKind) {                                                           \
            case ::nebula::ValueTypeKind::kBool: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kBool> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt8: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kInt8> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt16: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kInt16> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt32: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kInt32> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kInt64: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kInt64> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint8: {                                   \
                return PREFIX<::nebula::ValueTypeKind::kUint8> SUFFIX##PARAM;         \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint16: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kUint16> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint32: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kUint32> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kUint64: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kUint64> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kFloat32: {                                 \
                return PREFIX<::nebula::ValueTypeKind::kFloat32> SUFFIX##PARAM;       \
            }                                                                         \
            case ::nebula::ValueTypeKind::kFloat64: {                                 \
                return PREFIX<::nebula::ValueTypeKind::kFloat64> SUFFIX##PARAM;       \
            }                                                                         \
            case ::nebula::ValueTypeKind::kString: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kString> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kList: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kList> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kLocalTime: {                               \
                return PREFIX<::nebula::ValueTypeKind::kLocalTime> SUFFIX##PARAM;     \
            }                                                                         \
            case ::nebula::ValueTypeKind::kZonedTime: {                               \
                return PREFIX<::nebula::ValueTypeKind::kZonedTime> SUFFIX##PARAM;     \
            }                                                                         \
            case ::nebula::ValueTypeKind::kDuration: {                                \
                return PREFIX<::nebula::ValueTypeKind::kDuration> SUFFIX##PARAM;      \
            }                                                                         \
            case ::nebula::ValueTypeKind::kDate: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kDate> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kLocalDatetime: {                           \
                return PREFIX<::nebula::ValueTypeKind::kLocalDatetime> SUFFIX##PARAM; \
            }                                                                         \
            case ::nebula::ValueTypeKind::kZonedDatetime: {                           \
                return PREFIX<::nebula::ValueTypeKind::kZonedDatetime> SUFFIX##PARAM; \
            }                                                                         \
            case ::nebula::ValueTypeKind::kRecord: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kRecord> SUFFIX##PARAM;        \
            }                                                                         \
            case ::nebula::ValueTypeKind::kNode: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kNode> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kEdge: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kEdge> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kPath: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kPath> SUFFIX##PARAM;          \
            }                                                                         \
            case ::nebula::ValueTypeKind::kRef: {                                     \
                return PREFIX<::nebula::ValueTypeKind::kRef> SUFFIX##PARAM;           \
            }                                                                         \
            case ::nebula::ValueTypeKind::kNull: {                                    \
                return PREFIX<::nebula::ValueTypeKind::kNull> SUFFIX##PARAM;          \
            }                                                                         \
            default: {                                                                \
                DCHECK(false) << "Invalid type: " << std::to_string(typeKind);        \
                throw std::runtime_error("ValueTypeKind dispatch not implemented: " + \
                                         std::to_string(typeKind));                   \
            }                                                                         \
        }                                                                             \
    }()

#define NEBULA_DYNAMIC_PHYSICAL_TYPE_WITH_ANY_DISPATCH_IMPL(PREFIX, SUFFIX, typeKind, PARAM) \
    [&]() {                                                                                  \
        switch (typeKind) {                                                                  \
            case ::nebula::ValueTypeKind::kBool: {                                           \
                return PREFIX<::nebula::ValueTypeKind::kBool> SUFFIX##PARAM;                 \
            }                                                                                \
            case ::nebula::ValueTypeKind::kInt8: {                                           \
                return PREFIX<::nebula::ValueTypeKind::kInt8> SUFFIX##PARAM;                 \
            }                                                                                \
            case ::nebula::ValueTypeKind::kInt16: {                                          \
                return PREFIX<::nebula::ValueTypeKind::kInt16> SUFFIX##PARAM;                \
            }                                                                                \
            case ::nebula::ValueTypeKind::kInt32: {                                          \
                return PREFIX<::nebula::ValueTypeKind::kInt32> SUFFIX##PARAM;                \
            }                                                                                \
            case ::nebula::ValueTypeKind::kInt64: {                                          \
                return PREFIX<::nebula::ValueTypeKind::kInt64> SUFFIX##PARAM;                \
            }                                                                                \
            case ::nebula::ValueTypeKind::kUint8: {                                          \
                return PREFIX<::nebula::ValueTypeKind::kUint8> SUFFIX##PARAM;                \
            }                                                                                \
            case ::nebula::ValueTypeKind::kUint16: {                                         \
                return PREFIX<::nebula::ValueTypeKind::kUint16> SUFFIX##PARAM;               \
            }                                                                                \
            case ::nebula::ValueTypeKind::kUint32: {                                         \
                return PREFIX<::nebula::ValueTypeKind::kUint32> SUFFIX##PARAM;               \
            }                                                                                \
            case ::nebula::ValueTypeKind::kUint64: {                                         \
                return PREFIX<::nebula::ValueTypeKind::kUint64> SUFFIX##PARAM;               \
            }                                                                                \
            case ::nebula::ValueTypeKind::kFloat32: {                                        \
                return PREFIX<::nebula::ValueTypeKind::kFloat32> SUFFIX##PARAM;              \
            }                                                                                \
            case ::nebula::ValueTypeKind::kFloat64: {                                        \
                return PREFIX<::nebula::ValueTypeKind::kFloat64> SUFFIX##PARAM;              \
            }                                                                                \
            case ::nebula::ValueTypeKind::kString: {                                         \
                return PREFIX<::nebula::ValueTypeKind::kString> SUFFIX##PARAM;               \
            }                                                                                \
            case ::nebula::ValueTypeKind::kList: {                                           \
                return PREFIX<::nebula::ValueTypeKind::kList> SUFFIX##PARAM;                 \
            }                                                                                \
            case ::nebula::ValueTypeKind::kLocalTime: {                                      \
                return PREFIX<::nebula::ValueTypeKind::kLocalTime> SUFFIX##PARAM;            \
            }                                                                                \
            case ::nebula::ValueTypeKind::kZonedTime: {                                      \
                return PREFIX<::nebula::ValueTypeKind::kZonedTime> SUFFIX##PARAM;            \
            }                                                                                \
            case ::nebula::ValueTypeKind::kDuration: {                                       \
                return PREFIX<::nebula::ValueTypeKind::kDuration> SUFFIX##PARAM;             \
            }                                                                                \
            case ::nebula::ValueTypeKind::kDate: {                                           \
                return PREFIX<::nebula::ValueTypeKind::kDate> SUFFIX##PARAM;                 \
            }                                                                                \
            case ::nebula::ValueTypeKind::kLocalDatetime: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kLocalDatetime> SUFFIX##PARAM;        \
            }                                                                                \
            case ::nebula::ValueTypeKind::kZonedDatetime: {                                  \
                return PREFIX<::nebula::ValueTypeKind::kZonedDatetime> SUFFIX##PARAM;        \
            }                                                                                \
            case ::nebula::ValueTypeKind::kRecord: {                                         \
                return PREFIX<::nebula::ValueTypeKind::kRecord> SUFFIX##PARAM;               \
            }                                                                                \
            case ::nebula::ValueTypeKind::kNode: {                                           \
                return PREFIX<::nebula::ValueTypeKind::kNode> SUFFIX##PARAM;                 \
            }                                                                                \
            case ::nebula::ValueTypeKind::kEdge: {                                           \
                return PREFIX<::nebula::ValueTypeKind::kEdge> SUFFIX##PARAM;                 \
            }                                                                                \
            case ::nebula::ValueTypeKind::kPath: {                                           \
                return PREFIX<::nebula::ValueTypeKind::kPath> SUFFIX##PARAM;                 \
            }                                                                                \
            case ::nebula::ValueTypeKind::kAny: {                                            \
                return PREFIX<::nebula::ValueTypeKind::kAny> SUFFIX##PARAM;                  \
            }                                                                                \
            case ::nebula::ValueTypeKind::kRef: {                                            \
                return PREFIX<::nebula::ValueTypeKind::kRef> SUFFIX##PARAM;                  \
            }                                                                                \
            case ::nebula::ValueTypeKind::kNull: {                                           \
                return PREFIX<::nebula::ValueTypeKind::kNull> SUFFIX##PARAM;                 \
            }                                                                                \
            default: {                                                                       \
                DCHECK(false);                                                               \
                throw std::runtime_error("Type dispatch not implemented: " +                 \
                                         std::to_string(typeKind));                          \
            }                                                                                \
        }                                                                                    \
    }()

// property type can be used as property of node/edge/list/record
#define NEBULA_DYNAMIC_PROPERTY_TYPE_DISPATCH(TEMPLATE_FUNC, typeKind, ...) \
    NEBULA_DYNAMIC_PROPERTY_TYPE_DISPATCH_IMPL(TEMPLATE_FUNC, , typeKind, (__VA_ARGS__))

#define NEBULA_DYNAMIC_PHYSICAL_TYPE_DISPATCH(TEMPLATE_FUNC, typeKind, ...) \
    NEBULA_DYNAMIC_PHYSICAL_TYPE_DISPATCH_IMPL(TEMPLATE_FUNC, , typeKind, (__VA_ARGS__))

// physical type + any
// in many cases, we need to treat `kAny` separately
#define NEBULA_DYNAMIC_PHYSICAL_TYPE_WITH_ANY_DISPATCH(TEMPLATE_FUNC, typeKind, ...) \
    NEBULA_DYNAMIC_PHYSICAL_TYPE_WITH_ANY_DISPATCH_IMPL(                             \
            TEMPLATE_FUNC, , typeKind, (__VA_ARGS__))

inline constexpr bool isTrivialType(ValueTypeKind type) {
    return NEBULA_DYNAMIC_PHYSICAL_TYPE_WITH_ANY_DISPATCH_IMPL(TypeTraits, ::isTrivial, type,
                                                               /*empty*/);
}
inline constexpr bool unableInline(ValueTypeKind type) {
    switch (type) {
        case ValueTypeKind::kString:
        case ValueTypeKind::kList:
        case ValueTypeKind::kDuration:
        case ValueTypeKind::kRecord:
        case ValueTypeKind::kNode:
        case ValueTypeKind::kEdge:
        case ValueTypeKind::kPath:
            return true;
        default:
            return false;
    }
}

// Combine two hash values
inline size_t hashCombine(size_t seed, size_t hash) {
    return seed ^ (hash + 0x9e3779b9 + (seed << 6) + (seed >> 2));
}

template <typename T>
inline size_t hashContainer(const T& container) {
    size_t seed = 0;
    std::hash<typename T::value_type> hasher;
    for (const auto& v : container) {
        seed = hashCombine(seed, hasher(v));
    }
    return seed;
}

template <typename K, typename V>
inline size_t hashContainer(const NMap<K, V>& nmap) {
    size_t seed = 0;
    std::hash<typename NMap<K, V>::value_type> hasher;
    for (const auto& v : nmap) {
        seed ^= hasher(v);
    }
    return seed;
}

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::String> {
    size_t operator()(const nebula::String& s) const noexcept {
        return hash<string_view>()({s.data(), s.size()});
    }
};

}  // namespace std
