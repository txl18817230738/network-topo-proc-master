// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <memory>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "nebula/common/datatype/ValueTypeKind.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"
#include "nebula/common/utils/Types.h"

namespace nebula {

class ValueType;
using ValueTypePtr = std::shared_ptr<ValueType>;

class GraphElementObjectType;
using ObjectTypePtr = std::shared_ptr<GraphElementObjectType>;

namespace catalog {
class NodeType;
class EdgeType;
class GraphType;
}  // namespace catalog

ValueTypePtr propertyToValueType(const catalog::Property* p);

class GraphElementObjectType {
public:
    explicit GraphElementObjectType(ValueTypeKind t) : type_(t) {}

    GraphElementObjectType(ValueTypeKind t,
                           std::unordered_map<std::string, ValueTypePtr>&& fields)
            : type_(t), fieldTypes_(std::move(fields)) {}

    GraphElementObjectType(ValueTypeKind t,
                           std::string name,
                           std::unordered_map<std::string, ValueTypePtr>&& fields,
                           const std::unordered_set<std::string>& labels)
            : type_(t),
              name_(std::move(name)),
              fieldTypes_(std::move(fields)),
              labels_(labels) {}
    GraphElementObjectType(ValueTypeKind t,
                           const catalog::ElementType* elemType,
                           std::string name,
                           std::unordered_map<std::string, ValueTypePtr>&& fields,
                           const std::unordered_set<std::string>& labels)
            : type_(t),
              elemType_(elemType),
              name_(std::move(name)),
              fieldTypes_(std::move(fields)),
              labels_(labels) {}

    virtual ~GraphElementObjectType() = default;

    template <typename T,
              typename = std::enable_if_t<std::is_base_of_v<GraphElementObjectType, T>>>
    const T* as() const {
        return dynamic_cast<const T*>(this);
    }
    template <typename T,
              typename = std::enable_if_t<std::is_base_of_v<GraphElementObjectType, T>>>
    T* as() {
        return dynamic_cast<T*>(this);
    }

    ObjectTypePtr clone() const;
    virtual int compare(const GraphElementObjectType& rhs) const;
    virtual bool supertypeOf(const GraphElementObjectType& rhs) const;
    virtual bool eq(const GraphElementObjectType& rhs) const;
    virtual std::string toString() const = 0;
    virtual void write(folly::IOBuf* buf) const = 0;
    virtual Status read(folly::IOBuf* buf) = 0;
    virtual size_t encodedSize() const = 0;

    bool operator==(const GraphElementObjectType& rhs) const {
        return this->eq(rhs);
    }
    bool operator!=(const GraphElementObjectType& rhs) const {
        return !(*this == rhs);
    }
    bool operator<(const GraphElementObjectType& rhs) const {
        return this->compare(rhs) < 0;
    }

    ValueTypeKind type() const {
        return type_;
    }
    const auto& name() const {
        return name_;
    }
    const auto& fieldTypes() const {
        return fieldTypes_;
    }
    std::vector<std::string> propNames() const {
        std::vector<std::string> props;
        props.reserve(fieldTypes_.size());
        for (const auto& f : fieldTypes_) {
            props.emplace_back(f.first);
        }
        return props;
    }

    const auto& labels() const {
        return labels_;
    }

    std::vector<std::string> fieldNames() const;
    std::vector<StringView> fieldNamesView() const;
    const ValueTypePtr typeOfField(const std::string& name) const;
    // if the property doesn't exist, it will return true (nullable)
    bool isNullable(const std::string& name) const;
    void dropFields(const std::vector<std::string>& fields);

protected:
    virtual void doClone(const GraphElementObjectType& input);

    ValueTypeKind type_;
    const catalog::ElementType* elemType_{nullptr};
    std::string name_;
    std::unordered_map<std::string, ValueTypePtr> fieldTypes_;
    std::unordered_set<std::string> labels_;
};


class EdgeObjectType;

class NodeObjectType : public GraphElementObjectType {
public:
    NodeObjectType() : GraphElementObjectType(ValueTypeKind::kNode) {}

    explicit NodeObjectType(const catalog::NodeType* nodeType);
    explicit NodeObjectType(const Node& node);

    explicit NodeObjectType(NodeTypeID id)
            : GraphElementObjectType(ValueTypeKind::kNode), id_(id) {}
    NodeObjectType(NodeTypeID id,
                   std::unordered_map<std::string, ValueTypePtr>&& fields) noexcept
            : GraphElementObjectType(ValueTypeKind::kNode, std::move(fields)), id_(id) {}

    NodeObjectType(std::string name,
                   NodeTypeID id,
                   std::unordered_map<std::string, ValueTypePtr>&& fields,
                   const std::unordered_set<std::string>& labels) noexcept
            : GraphElementObjectType(
                      ValueTypeKind::kNode, std::move(name), std::move(fields), labels),
              id_(id) {}

    NodeObjectType(const NodeObjectType& rhs) : GraphElementObjectType(ValueTypeKind::kNode) {
        doClone(rhs);
    }
    NodeObjectType& operator=(const NodeObjectType& rhs) {
        if (this != &rhs) {
            doClone(rhs);
        }
        return *this;
    }

    int compare(const GraphElementObjectType& rhs) const override;
    bool eq(const GraphElementObjectType& rhs) const override;
    std::string toString() const override;
    void write(folly::IOBuf* buf) const override;
    Status read(folly::IOBuf* buf) override;
    size_t encodedSize() const override;

protected:
    void doClone(const GraphElementObjectType& input) override;

public:
    auto id() const {
        return id_;
    }

private:
    NodeTypeID id_{-1};
};

class EdgeObjectType : public GraphElementObjectType {
public:
    EdgeObjectType() : GraphElementObjectType(ValueTypeKind::kEdge) {}

    explicit EdgeObjectType(const catalog::EdgeType* edgeType, const catalog::GraphType* graph);
    explicit EdgeObjectType(const catalog::EdgeType* edgeType);
    explicit EdgeObjectType(const Edge& edge);

    explicit EdgeObjectType(EdgeTypeID id)
            : GraphElementObjectType(ValueTypeKind::kEdge), id_(id) {}
    EdgeObjectType(EdgeTypeID id,
                   std::unordered_map<std::string, ValueTypePtr>&& fields) noexcept
            : GraphElementObjectType(ValueTypeKind::kEdge, std::move(fields)), id_(id) {}

    EdgeObjectType(std::string name,
                   EdgeTypeID id,
                   std::unordered_map<std::string, ValueTypePtr>&& fields,
                   const std::unordered_set<std::string>& labels,
                   bool directed,
                   const NodeObjectType& src,
                   const NodeObjectType& dst) noexcept
            : GraphElementObjectType(
                      ValueTypeKind::kEdge, std::move(name), std::move(fields), labels),
              id_(id),
              directed_(directed),
              src_(src),
              dst_(dst) {}

    EdgeObjectType(const EdgeObjectType& rhs) : GraphElementObjectType(ValueTypeKind::kEdge) {
        doClone(rhs);
    }
    EdgeObjectType& operator=(const EdgeObjectType& rhs) {
        if (this != &rhs) {
            doClone(rhs);
        }
        return *this;
    }

    int compare(const GraphElementObjectType& rhs) const override;
    bool supertypeOf(const GraphElementObjectType& rhs) const override;
    bool eq(const GraphElementObjectType& rhs) const override;
    std::string toString() const override;
    void write(folly::IOBuf* buf) const override;
    Status read(folly::IOBuf* buf) override;
    size_t encodedSize() const override;

protected:
    void doClone(const GraphElementObjectType& input) override;

public:
    auto id() const {
        return id_;
    }
    auto directed() const {
        return directed_;
    }
    const auto& src() const {
        return src_;
    }
    const auto& dst() const {
        return dst_;
    }

private:
    EdgeTypeID id_;
    bool directed_{false};
    NodeObjectType src_;
    NodeObjectType dst_;
};

struct ObjectTypeLess {
    bool operator()(const ObjectTypePtr& lhs, const ObjectTypePtr& rhs) const {
        return lhs->compare(*rhs) < 0;
    }
};
using ObjectTypeSet = std::set<ObjectTypePtr, ObjectTypeLess>;

template <>
struct nrpc::BufferReaderWriter<ObjectTypePtr> {
    static void write(folly::IOBuf* buf, const ObjectTypePtr& obj) {
        if (obj) {
            nrpc::BufferReaderWriter<bool>::write(buf, true);
            nrpc::BufferReaderWriter<ValueTypeKind>::write(buf, obj->type());
            obj->write(buf);
        } else {
            nrpc::BufferReaderWriter<bool>::write(buf, false);
        }
    }

    static Status read(folly::IOBuf* buf, ObjectTypePtr* obj) {
        bool hasVal{false};
        NG_RETURN_IF_ERROR(nrpc::BufferReaderWriter<bool>::read(buf, &hasVal));
        if (hasVal) {
            ValueTypeKind kind{};
            NG_RETURN_IF_ERROR(nrpc::BufferReaderWriter<ValueTypeKind>::read(buf, &kind));
            switch (kind) {
                case ValueTypeKind::kNode: {
                    *obj = std::make_shared<NodeObjectType>();
                    break;
                }
                case ValueTypeKind::kEdge: {
                    *obj = std::make_shared<EdgeObjectType>();
                    break;
                }
                default: {
                    return V_STATUS(RPC_DESERIALIZE_OBJECT_TYPE_ERROR, enum2String(kind));
                }
            }
            if ((*obj)) {
                return (*obj)->read(buf);
            }
        } else {
            *obj = nullptr;
        }
        return Status::OK();
    }

    static size_t encodedSize(const ObjectTypePtr& obj) {
        return sizeof(bool) + (obj ? sizeof(ValueTypeKind) + obj->encodedSize() : 0);
    }
};

}  // namespace nebula
