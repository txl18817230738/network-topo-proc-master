// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <algorithm>

#include <fmt/core.h>

#include "nebula/common/datatype/ValueTypeKind.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"
#include "nebula/common/utils/EdgeUtils.h"
#include "nebula/common/utils/Types.h"
#include "nebula/common/valuetype/ObjectType.h"

namespace nebula {
class ValueType;
}

namespace std {

template <>
struct equal_to<std::shared_ptr<nebula::ValueType>> {
    bool operator()(const std::shared_ptr<nebula::ValueType>& lhs,
                    const std::shared_ptr<nebula::ValueType>& rhs) const;
};

}  // namespace std

namespace nebula {

#define SUPERTYPE_OF_ANY(rhs)                                                             \
    if (rhs.isA<ValueTypeKind::kAny>()) {                                                 \
        const auto* rhsAny = rhs.as<AnyValueType>();                                      \
        if (rhsAny->open()) {                                                             \
            return false;                                                                 \
        } else {                                                                          \
            auto all = std::all_of(rhsAny->variants().begin(),                            \
                                   rhsAny->variants().end(),                              \
                                   [&](const auto& v) { return this->supertypeOf(*v); }); \
            return all;                                                                   \
        }                                                                                 \
    }


#define DEFINE_TYPE_INFO(T, F)                                         \
    class T##ValueType : public F {                                    \
    public:                                                            \
        T##ValueType() : F(ValueTypeKind::k##T) {}                     \
        ValueTypePtr clone() const override {                          \
            auto p = std::make_shared<T##ValueType>();                 \
            p->doClone(*this);                                         \
            return p;                                                  \
        }                                                              \
                                                                       \
        bool supertypeOf(const ValueType& rhs) const override {        \
            SUPERTYPE_OF_ANY(rhs);                                     \
            return type_ == rhs.type();                                \
        }                                                              \
                                                                       \
        static constexpr ValueTypeKind typeKind = ValueTypeKind::k##T; \
    }

#define DEFINE_NUMERIC_TYPE_INFO(T)                                    \
    class T##ValueType : public NumericValueType {                     \
    public:                                                            \
        T##ValueType() : NumericValueType(ValueTypeKind::k##T) {}      \
        ValueTypePtr clone() const override {                          \
            auto p = std::make_shared<T##ValueType>();                 \
            p->doClone(*this);                                         \
            return p;                                                  \
        }                                                              \
        static constexpr ValueTypeKind typeKind = ValueTypeKind::k##T; \
    }

class ValueType;
using ValueTypePtr = std::shared_ptr<ValueType>;
using ValueTypeConstPtr = const std::shared_ptr<ValueType>;

struct ValueTypeLess;
using ValueTypeSet = std::set<ValueTypePtr, ValueTypeLess>;

class ValueType {
public:
    virtual ~ValueType() = default;

    ValueTypeKind type() const {
        return type_;
    }

    // NOTE: `ValueType::makeByType` may create incomplete valueType of nested types.
    // You should only use this for basic types.
    static ValueTypePtr makeByType(ValueTypeKind type);
    static ValueTypePtr makeByValue(const Value& value);

    template <typename T, typename = std::enable_if_t<std::is_base_of_v<ValueType, T>>>
    const T* as() const {
        return dynamic_cast<const T*>(this);
    }

    template <typename T, typename = std::enable_if_t<std::is_base_of_v<ValueType, T>>>
    T* as() {
        return dynamic_cast<T*>(this);
    }

    template <typename T, typename = std::enable_if_t<std::is_base_of_v<ValueType, T>>>
    bool isA() const {
        return as<T>() != nullptr;
    }

    template <ValueTypeKind T>
    bool isA() const {
        return type_ == T;
    }

    virtual bool isGraphElementType() const {
        return false;
    }

    virtual ValueTypePtr clone() const = 0;

    virtual std::string toString() const {
        return enum2String(type_);
    }

    // T1 contains all elements of T2, then T1 is supertype of T2
    // GQL [3.10.3]
    virtual bool supertypeOf(const ValueType& rhs) const = 0;

    bool subtypeOf(const ValueType& rhs) const {
        return rhs.supertypeOf(*this);
    }

    virtual bool eq(const ValueType& rhs) const {
        return type_ == rhs.type_;
    }

    // TODO(czp): Support compatibleWith for numeric/temporal types
    virtual bool compatibleWith(const ValueType& rhs) const {
        return this->eq(rhs);
    }

    inline bool operator==(const ValueType& rhs) const {
        return this->eq(rhs);
    }

    // The order is determined by GQL [22.18]
    // return: -1 if this < rhs, 0 if this == rhs, 1 if this > rhs
    virtual int compare(const ValueType& rhs) const {
        if (type_ != rhs.type_) {
            return type_ < rhs.type_ ? -1 : 1;
        }
        return 0;
    }

    inline bool operator<(const ValueType& rhs) const {
        return this->compare(rhs) < 0;
    }

    bool operator!=(const ValueType& rhs) const {
        return !this->eq(rhs);
    }

    // FIXME(docking): Implement override functions in derived classes
    virtual void write(folly::IOBuf* buf) const;
    virtual Status read(folly::IOBuf* buf);
    virtual size_t encodedSize() const;

protected:
    void doClone(const ValueType& input) {
        type_ = input.type_;
    }

    ValueType() noexcept = default;

    explicit ValueType(ValueTypeKind type) : type_(type) {}

    ValueTypeKind type_{ValueTypeKind::kAny};
};

struct ValueTypeLess {
    bool operator()(const ValueTypePtr& lhs, const ValueTypePtr& rhs) const {
        return lhs->compare(*rhs) < 0;
    }
};

class AnyValueType final : public ValueType {
public:
    // Open dynamic union type
    AnyValueType() : ValueType(ValueTypeKind::kAny) {}

    // Closed dynamic union type
    explicit AnyValueType(const ValueTypeSet& variants) noexcept;

    ValueTypePtr clone() const override {
        auto p = std::make_shared<AnyValueType>();
        for (const auto& variant : variants_) {
            p->variants_.emplace(variant->clone());
        }
        return p;
    }

    bool open() const {
        return variants_.empty();
    }

    bool supertypeOf(const ValueType& rhs) const override;

    bool eq(const ValueType& rhs) const override;

    int compare(const ValueType& rhs) const override;

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kAny;

    const auto& variants() const& {
        return variants_;
    }

    std::string toString() const override;

    void write(folly::IOBuf* buf) const override;
    Status read(folly::IOBuf* buf) override;
    size_t encodedSize() const override;

    static ValueTypeSet flatAny(const ValueTypeSet& variants);

private:
    ValueTypeSet variants_;
};

DEFINE_TYPE_INFO(Bool, ValueType);

class BytesValueType : public ValueType {
public:
    BytesValueType() : ValueType(ValueTypeKind::kBytes) {}
    ValueTypePtr clone() const override {
        auto p = std::make_shared<BytesValueType>();
        p->doClone(*this);
        return p;
    }

    bool supertypeOf(const ValueType& rhs) const override {
        SUPERTYPE_OF_ANY(rhs);
        return type_ == rhs.type() || rhs.type() == ValueTypeKind::kString;
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kBytes;
};

class StringValueType : public ValueType {
public:
    StringValueType() : ValueType(ValueTypeKind::kString) {}
    ValueTypePtr clone() const override {
        auto p = std::make_shared<StringValueType>();
        p->doClone(*this);
        return p;
    }

    bool supertypeOf(const ValueType& rhs) const override {
        SUPERTYPE_OF_ANY(rhs);
        return type_ == rhs.type();
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kString;
};

class NumericValueType : public ValueType {
public:
    bool isSigned() const;
    bool isSignedInt() const;
    bool isUnsignedInt() const;
    bool isFloat() const;

    std::size_t precision() const;

    bool supertypeOf(const ValueType& rhs) const override;

protected:
    explicit NumericValueType(ValueTypeKind type) : ValueType(type) {}
};

DEFINE_NUMERIC_TYPE_INFO(Int8);
DEFINE_NUMERIC_TYPE_INFO(Int16);
DEFINE_NUMERIC_TYPE_INFO(Int32);
DEFINE_NUMERIC_TYPE_INFO(Int64);
DEFINE_NUMERIC_TYPE_INFO(Uint8);
DEFINE_NUMERIC_TYPE_INFO(Uint16);
DEFINE_NUMERIC_TYPE_INFO(Uint32);
DEFINE_NUMERIC_TYPE_INFO(Uint64);

DEFINE_NUMERIC_TYPE_INFO(Float32);
DEFINE_NUMERIC_TYPE_INFO(Float64);

class DateValueType : public ValueType {
public:
    DateValueType() : ValueType(ValueTypeKind::kDate) {}
    ValueTypePtr clone() const override {
        auto p = std::make_shared<DateValueType>();
        p->doClone(*this);
        return p;
    }

    bool supertypeOf(const ValueType& rhs) const override {
        SUPERTYPE_OF_ANY(rhs);
        return type_ == rhs.type();
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kDate;
};

class LocalTimeValueType : public ValueType {
public:
    LocalTimeValueType() : ValueType(ValueTypeKind::kLocalTime) {}
    ValueTypePtr clone() const override {
        auto p = std::make_shared<LocalTimeValueType>();
        p->doClone(*this);
        return p;
    }

    bool supertypeOf(const ValueType& rhs) const override {
        SUPERTYPE_OF_ANY(rhs);
        return type_ == rhs.type();
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kLocalTime;
};

class ZonedTimeValueType : public ValueType {
public:
    ZonedTimeValueType() : ValueType(ValueTypeKind::kZonedTime) {}
    ValueTypePtr clone() const override {
        auto p = std::make_shared<ZonedTimeValueType>();
        p->doClone(*this);
        return p;
    }

    bool supertypeOf(const ValueType& rhs) const override {
        SUPERTYPE_OF_ANY(rhs);
        return type_ == rhs.type();
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kZonedTime;
};

class LocalDatetimeValueType : public ValueType {
public:
    LocalDatetimeValueType() : ValueType(ValueTypeKind::kLocalDatetime) {}
    ValueTypePtr clone() const override {
        auto p = std::make_shared<LocalDatetimeValueType>();
        p->doClone(*this);
        return p;
    }

    bool supertypeOf(const ValueType& rhs) const override {
        SUPERTYPE_OF_ANY(rhs);
        return type_ == rhs.type() || rhs.isA<ValueTypeKind::kDate>() ||
               rhs.isA<ValueTypeKind::kLocalTime>();
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kLocalDatetime;
};

class ZonedDatetimeValueType : public ValueType {
public:
    ZonedDatetimeValueType() : ValueType(ValueTypeKind::kZonedDatetime) {}
    ValueTypePtr clone() const override {
        auto p = std::make_shared<ZonedDatetimeValueType>();
        p->doClone(*this);
        return p;
    }

    bool supertypeOf(const ValueType& rhs) const override {
        SUPERTYPE_OF_ANY(rhs);
        return type_ == rhs.type() || rhs.isA<ValueTypeKind::kDate>() ||
               rhs.isA<ValueTypeKind::kZonedTime>();
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kZonedDatetime;
};

DEFINE_TYPE_INFO(Duration, ValueType);


class ConstructedValueType : public ValueType {
protected:
    explicit ConstructedValueType(ValueTypeKind t) noexcept : ValueType(t) {}
};

class ListValueType : public ConstructedValueType {
public:
    // TODO(wuu): The default constructor of nested types will result in invalid valueType,
    // which is dangerous because we're going to use valueType to create Vector at runtime.
    // It should be only used in RPC serialization and decleared as private.
    ListValueType() : ConstructedValueType(ValueTypeKind::kList) {}

    explicit ListValueType(ValueTypePtr child, bool group = false) noexcept
            : ConstructedValueType(ValueTypeKind::kList),
              child_(std::move(DCHECK_NOTNULL(child))),
              group_(group) {}

    const ValueType* child() const {
        return child_.get();
    }

    bool group() const {
        return group_;
    }

    ValueTypePtr clone() const override {
        auto p = std::make_shared<ListValueType>();
        p->doClone(*this);
        return p;
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kList;

    int compare(const ValueType& rhs) const override;
    bool supertypeOf(const ValueType& rhs) const override;
    bool eq(const ValueType& rhs) const override;
    bool compatibleWith(const ValueType& rhs) const override;

    std::string toString() const override {
        return fmt::format(
                "{}<{}>", ConstructedValueType::toString(), DCHECK_NOTNULL(child_)->toString());
    }

    void write(folly::IOBuf* buf) const override;
    Status read(folly::IOBuf* buf) override;
    size_t encodedSize() const override;

protected:
    void doClone(const ListValueType& input) {
        ConstructedValueType::doClone(input);
        if (input.child_) {
            child_ = input.child_->clone();
        }
        group_ = input.group_;
    }

private:
    ValueTypePtr child_{std::make_shared<AnyValueType>()};
    // Is group list to store group variable
    bool group_{false};
};

class PathValueType : public ConstructedValueType {
public:
    PathValueType() : ConstructedValueType(ValueTypeKind::kPath) {}

    explicit PathValueType(std::vector<ValueTypePtr>&& children) noexcept
            : ConstructedValueType(ValueTypeKind::kPath),
              children_(mergeProps(std::move(children))) {
        DCHECK(isValid());
    }

    const auto& children() const {
        return children_;
    }

    ValueTypePtr nodeValueTypeList() const;

    ValueTypePtr edgeValueTypeList() const;

    ValueTypePtr clone() const override {
        auto p = std::make_shared<PathValueType>();
        p->doClone(*this);
        return p;
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kPath;

    int compare(const ValueType& rhs) const override;
    bool supertypeOf(const ValueType& rhs) const override;
    bool eq(const ValueType& rhs) const override;

    void write(folly::IOBuf* buf) const override;
    Status read(folly::IOBuf* buf) override;
    size_t encodedSize() const override;

    std::string toString() const override;

    bool isValid() const;

protected:
    void doClone(const PathValueType& input) {
        ConstructedValueType::doClone(input);
        for (const auto& child : input.children_) {
            children_.emplace_back(child->clone());
        }
    }

    // It will merge the same NodeType and EdgeType inside the PathType
    // For example, it will change
    // PATH<
    //   NodeRef<(Person)>{"a": INT32},
    //   EdgeRef<(Person)-[PERSON_KNOWS_PERSON]->(Person)>{},
    //   NodeRef<(Person)>{"b": INT64}>
    // to
    // PATH<
    //   NodeRef<(Person)>{"a": INT32, "b": INT64},
    //   EdgeRef<(Person)-[PERSON_KNOWS_PERSON]->(Person)>{}>
    //   NodeRef<(Person)>{"a": INT32, "b": INT64}>
    //
    // TODO(wuu): We need to do this because of current implementation of PathVector, which may
    // need to refactor later.
    static std::vector<ValueTypePtr> mergeProps(std::vector<ValueTypePtr>&& children);

private:
    // node, edge, node, list<edge>, node, ...
    std::vector<ValueTypePtr> children_;
};

class RecordValueType : public ConstructedValueType {
public:
    RecordValueType() : ConstructedValueType(ValueTypeKind::kRecord) {}

    explicit RecordValueType(std::unordered_map<std::string, ValueTypePtr>&& fields) noexcept
            : ConstructedValueType(ValueTypeKind::kRecord), fieldTypes_(std::move(fields)) {}
    explicit RecordValueType(
            std::vector<std::pair<std::string, ValueTypePtr>>&& fields) noexcept
            : ConstructedValueType(ValueTypeKind::kRecord) {
        for (auto& [fieldName, fieldType] : fields) {
            fieldTypes_.emplace(std::move(fieldName), std::move(fieldType));
        }
    }

    RecordValueType(std::vector<std::string>&& fieldNames,
                    std::vector<ValueTypePtr>&& valueTypes) noexcept
            : ConstructedValueType(ValueTypeKind::kRecord) {
        DCHECK(fieldNames.size() == valueTypes.size());
        for (size_t i = 0; i < fieldNames.size(); i++) {
            fieldTypes_.emplace(std::move(fieldNames[i]), std::move(valueTypes[i]));
        }
    }

    const auto& fieldTypes() const {
        return fieldTypes_;
    }

    std::vector<std::string> fieldNames() const {
        std::vector<std::string> fieldNames;
        for (auto& [name, _] : fieldTypes_) {
            fieldNames.emplace_back(name);
        }
        return fieldNames;
    }

    ValueTypePtr clone() const override {
        auto p = std::make_shared<RecordValueType>();
        p->doClone(*this);
        return p;
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kRecord;

    int compare(const ValueType& rhs) const override;

    bool supertypeOf(const ValueType& rhs) const override;

    bool eq(const ValueType& rhs) const override;
    bool compatibleWith(const ValueType& rhs) const override;

    std::string toString() const override;

    const ValueTypePtr typeOfField(const std::string& name) const;

    void write(folly::IOBuf* buf) const override;
    Status read(folly::IOBuf* buf) override;
    size_t encodedSize() const override;

protected:
    void doClone(const RecordValueType& input) {
        ConstructedValueType::doClone(input);
        for (const auto& field : input.fieldTypes_) {
            fieldTypes_.emplace(field.first, field.second->clone());
        }
    }

private:
    std::unordered_map<std::string, ValueTypePtr> fieldTypes_;
};

class GraphReferenceValueType : public ValueType {
public:
    GraphReferenceValueType() : ValueType(ValueTypeKind::kRef) {}
    GraphReferenceValueType(ValueTypeSet&& nodeTypeSet, ValueTypeSet&& edgeTypeSet)
            : ValueType(ValueTypeKind::kRef) {
        nodeTypes_ = std::move(nodeTypeSet);
        edgeTypes_ = std::move(edgeTypeSet);
    }
    ~GraphReferenceValueType() override = default;

    int compare(const ValueType& rhs) const override;
    bool supertypeOf(const ValueType& rhs) const override;
    bool eq(const ValueType& rhs) const override;
    bool compatibleWith(const ValueType& rhs) const override;  // Used for graph procedure
    ValueTypePtr clone() const override {
        auto p = std::make_shared<GraphReferenceValueType>();
        p->doClone(*this);
        return p;
    }
    std::string toString() const override;

    void pushNodeType(const ValueTypePtr nodeType) {
        nodeTypes_.emplace(nodeType);
    }

    void pushEdgeType(const ValueTypePtr edgeType) {
        edgeTypes_.emplace(edgeType);
    }

    const ValueTypeSet& nodeTypes() const {
        return nodeTypes_;
    }

    const ValueTypeSet& edgeTypes() const {
        return edgeTypes_;
    }

    std::size_t numNodeTypes() const {
        return nodeTypes_.size();
    }

    std::size_t numEdgeTypes() const {
        return edgeTypes_.size();
    }

protected:
    void doClone(const GraphReferenceValueType& rhs) {
        ValueType::doClone(rhs);
        nodeTypes_ = rhs.nodeTypes_;
        edgeTypes_ = rhs.edgeTypes_;
    }

private:
    ValueTypeSet nodeTypes_;
    ValueTypeSet edgeTypes_;
};


// 4.3.6 Binding tables
// A binding table is a primary object comprising a collection of zero or more (possibly
// duplicate) records of the same record type.
class BindingTableReferenceValueType : public ValueType {
public:
    BindingTableReferenceValueType() : ValueType(ValueTypeKind::kRef) {}

    explicit BindingTableReferenceValueType(std::shared_ptr<RecordValueType> recordType)
            : ValueType(ValueTypeKind::kRef), recordType_(recordType) {}

    ~BindingTableReferenceValueType() override = default;

    ValueTypePtr clone() const override {
        return std::make_shared<BindingTableReferenceValueType>(
                std::static_pointer_cast<RecordValueType>(recordType_->clone()));
    }

    const auto& recordType() const {
        return recordType_;
    }

    bool supertypeOf(const ValueType& rhs) const override;
    bool eq(const ValueType& rhs) const override;

    const auto& fieldTypes() const {
        return recordType_->fieldTypes();
    }

    std::vector<std::string> fieldNames() const {
        return recordType_->fieldNames();
    }

    std::string toString() const override;

private:
    std::shared_ptr<RecordValueType> recordType_;
};


class GraphElementReferenceValueType : public ValueType {
public:
    explicit GraphElementReferenceValueType(ValueTypeKind t) : ValueType(t) {}
    GraphElementReferenceValueType(ValueTypeKind t, const ObjectTypeSet& variants)
            : ValueType(t), variants_(variants) {
        DCHECK(isValid());
    }

    const auto& variants() const {
        return variants_;
    }

    bool isGraphElementType() const override {
        return true;
    }

    int compare(const ValueType& rhs) const override;
    bool supertypeOf(const ValueType& rhs) const override;
    bool eq(const ValueType& rhs) const override;

    void write(folly::IOBuf* buf) const override;
    Status read(folly::IOBuf* buf) override;
    size_t encodedSize() const override;

    // Returns the type of the graph object field if the field exists, otherwise returns Null
    ValueTypePtr typeOfField(const std::string& name) const;
    std::vector<StringView> sortedFieldNames() const;
    std::unordered_map<std::string, ValueTypePtr> fieldTypes() const;
    std::unordered_set<std::string> labels() const;
    void dropFields(const std::vector<std::string>& fields);

    ObjectTypeSet intersect(const GraphElementReferenceValueType* o);

    // for debug
    bool isValid() const;

protected:
    void doClone(const ValueType& input);

    ObjectTypeSet variants_;
};

class EdgeReferenceValueType;

using NodeTypeToPropNameMap = std::unordered_map<NodeTypeID, std::vector<std::string>>;
class NodeReferenceValueType : public GraphElementReferenceValueType {
public:
    static constexpr ValueTypeKind typeKind = ValueTypeKind::kNode;

    explicit NodeReferenceValueType(const NodeObjectType* nodeType)
            : NodeReferenceValueType{ObjectTypeSet{nodeType->clone()}} {}
    explicit NodeReferenceValueType(const NodeObjectType& nodeType)
            : NodeReferenceValueType{ObjectTypeSet{nodeType.clone()}} {}

    // nodeTypes should only contain NodeObjectType
    explicit NodeReferenceValueType(const ObjectTypeSet& nodeTypes) noexcept
            : GraphElementReferenceValueType(ValueTypeKind::kNode, nodeTypes) {}
    // nodeTypes should only contain NodeReferenceValueType
    explicit NodeReferenceValueType(const ValueTypeSet& nodeTypes) noexcept
            : GraphElementReferenceValueType(ValueTypeKind::kNode, flatRef(nodeTypes)) {}

    // same ctor as NodeObjectType for convenience, mainly used in ut
    explicit NodeReferenceValueType(NodeTypeID id)
            : NodeReferenceValueType{ObjectTypeSet{std::make_shared<NodeObjectType>(id)}} {}
    NodeReferenceValueType(NodeTypeID id,
                           std::unordered_map<std::string, ValueTypePtr>&& fields)
            : NodeReferenceValueType{
                      ObjectTypeSet{std::make_shared<NodeObjectType>(id, std::move(fields))}} {}

    // This method is only for convenience
    // It should be only used when this node reference type contains exactly one node type
    const NodeObjectType* oneNodeType() const;
    std::vector<const NodeObjectType*> nodeTypes() const;
    std::vector<NodeTypeID> nodeTypeIDs() const;
    NodeTypeToPropNameMap nodeTypeToPropNames() const;
    std::shared_ptr<NodeReferenceValueType> intersect(const NodeReferenceValueType* o);

    ValueTypePtr clone() const override;
    std::string toString() const override;

    static ObjectTypeSet flatRef(const ValueTypeSet& variants);

    // TODO(Xuntao): To be removed by https://github.com/vesoft-inc/nebula-ng/issues/3151
    NodeReferenceValueType() : GraphElementReferenceValueType(ValueTypeKind::kNode) {}
};

using EdgeTypeToPropNameMap = std::unordered_map<EdgeTypeID, std::vector<std::string>>;
class EdgeReferenceValueType : public GraphElementReferenceValueType {
public:
    static constexpr ValueTypeKind typeKind = ValueTypeKind::kEdge;

    explicit EdgeReferenceValueType(const EdgeObjectType* edgeType)
            : EdgeReferenceValueType{ObjectTypeSet{edgeType->clone()}} {}
    explicit EdgeReferenceValueType(const EdgeObjectType& edgeType)
            : EdgeReferenceValueType{ObjectTypeSet{edgeType.clone()}} {}

    // edgeTypes should only contain EdgeObjectType
    explicit EdgeReferenceValueType(const ObjectTypeSet& edgeTypes) noexcept
            : GraphElementReferenceValueType(ValueTypeKind::kEdge, edgeTypes) {}
    // edgeTypes should only contain EdgeReferenceValueType
    explicit EdgeReferenceValueType(const ValueTypeSet& edgeTypes) noexcept
            : GraphElementReferenceValueType(ValueTypeKind::kEdge, flatRef(edgeTypes)) {}

    // same ctor as EdgeObjectType for convenience, mainly used in ut
    explicit EdgeReferenceValueType(EdgeTypeID id)
            : EdgeReferenceValueType{ObjectTypeSet{std::make_shared<EdgeObjectType>(id)}} {}
    EdgeReferenceValueType(EdgeTypeID id,
                           std::unordered_map<std::string, ValueTypePtr>&& fields)
            : EdgeReferenceValueType{
                      ObjectTypeSet{std::make_shared<EdgeObjectType>(id, std::move(fields))}} {}

    // This method is only for convenience
    // It should be only used when this edge reference type contains exactly one edge type
    const EdgeObjectType* oneEdgeType() const;

    const std::vector<const EdgeObjectType*> edgeTypes() const;

    std::vector<EdgeTypeID> edgeTypeIDs() const;
    EdgeTypeToPropNameMap edgeTypeToPropNames() const;

    std::string toString() const override;
    ValueTypePtr clone() const override;

    static ObjectTypeSet flatRef(const ValueTypeSet& variants);

    std::shared_ptr<EdgeReferenceValueType> intersect(const EdgeReferenceValueType* o);

    // TODO(Xuntao): To be removed by https://github.com/vesoft-inc/nebula-ng/issues/3151
    EdgeReferenceValueType() : GraphElementReferenceValueType(ValueTypeKind::kEdge) {}
};


class FunctionValueType : public ValueType {
public:
    FunctionValueType() : ValueType(ValueTypeKind::kFunction) {}
    FunctionValueType(std::string name, std::vector<ValueTypePtr>&& args, ValueTypePtr&& ret)
            : ValueType(ValueTypeKind::kFunction),
              name_(std::move(name)),
              args_(std::move(args)),
              ret_(std::move(ret)) {}

    const auto& name() const {
        return name_;
    }

    const auto& args() const {
        return args_;
    }

    const auto& ret() const {
        return ret_;
    }

    bool isValid() const;

    ValueTypePtr clone() const override {
        auto p = std::make_shared<FunctionValueType>();
        p->doClone(*this);
        return p;
    }

    static constexpr ValueTypeKind typeKind = ValueTypeKind::kFunction;

    bool supertypeOf(const ValueType& rhs) const override;

    bool eq(const ValueType& rhs) const override;

protected:
    void doClone(const FunctionValueType& input) {
        ValueType::doClone(input);
        name_ = input.name_;
        for (const auto& arg : input.args_) {
            args_.emplace_back(arg->clone());
        }
        ret_ = input.ret_->clone();
    }

private:
    std::string name_;
    std::vector<ValueTypePtr> args_;
    ValueTypePtr ret_;
};

class NullValueType : public ValueType {
public:
    NullValueType() : ValueType(ValueTypeKind::kNull) {}

    ValueTypePtr clone() const override {
        auto p = std::make_shared<NullValueType>();
        p->doClone(*this);
        return p;
    }

    bool supertypeOf(const ValueType& rhs) const override {
        SUPERTYPE_OF_ANY(rhs);
        return rhs.isA<NullValueType>();
    }

    bool eq(const ValueType& rhs) const override {
        return rhs.isA<NullValueType>();
    }

    bool compatibleWith(const ValueType&) const override {
        return true;
    }

protected:
    explicit NullValueType(ValueTypeKind t) noexcept : ValueType(t) {}
};

class InvalidValueType : public ValueType {
public:
    InvalidValueType() : ValueType(ValueTypeKind::kInvalid) {}

    ValueTypePtr clone() const override {
        auto p = std::make_shared<InvalidValueType>();
        p->doClone(*this);
        return p;
    }

    bool supertypeOf(const ValueType& rhs) const override {
        SUPERTYPE_OF_ANY(rhs);
        return rhs.isA<NullValueType>();
    }

protected:
    explicit InvalidValueType(ValueTypeKind t) noexcept : ValueType(t) {}
};

#undef DEFINE_TYPE_INFO

template <>
struct nrpc::BufferReaderWriter<ValueTypePtr> {
    static void write(folly::IOBuf* buf, const ValueTypePtr& obj) {
        if (obj) {
            nrpc::BufferReaderWriter<bool>::write(buf, true);
            nrpc::BufferReaderWriter<ValueTypeKind>::write(buf, obj->type());
            obj->write(buf);
        } else {
            nrpc::BufferReaderWriter<bool>::write(buf, false);
        }
    }

    static Status read(folly::IOBuf* buf, ValueTypePtr* obj) {
        bool hasVal{false};
        NG_RETURN_IF_ERROR(nrpc::BufferReaderWriter<bool>::read(buf, &hasVal));
        if (hasVal) {
            ValueTypeKind kind{};
            NG_RETURN_IF_ERROR(nrpc::BufferReaderWriter<ValueTypeKind>::read(buf, &kind));
            *obj = ValueType::makeByType(kind);
            if ((*obj)) {
                return (*obj)->read(buf);
            }
        } else {
            *obj = nullptr;
        }
        return Status::OK();
    }

    static size_t encodedSize(const ValueTypePtr& obj) {
        return sizeof(bool) + (obj ? sizeof(ValueTypeKind) + obj->encodedSize() : 0);
    }
};

}  // namespace nebula
