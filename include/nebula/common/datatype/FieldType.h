// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <memory>
#include <string>

#include "nebula/common/base/Logging.h"
#include "nebula/common/nrpc/CommonDefine.h"
#include "nebula/common/valuetype/ValueType.h"

namespace nebula {

class FieldType final {
public:
    FieldType() = default;
    FieldType(const std::string& name, ValueTypePtr type) : name_(name), type_(type) {}

    bool hasType() const {
        return type_ != nullptr;
    }

    const std::string& name() const {
        return name_;
    }

    const ValueTypePtr& type() const {
        return type_;
    }

    bool isNumeric() const {
        return type_ && type_->isA<NumericValueType>();
    }

    void setType(ValueTypePtr type) {
        type_ = type;
    }

    void setName(const std::string& name) {
        name_ = name;
    }

    size_t hash() const {
        return folly::hash::hash_combine(name_);
    }

    std::string toString(bool verbose = false) const {
        return name_ + (verbose ? ":" + type_->toString() : "");
    }

    bool operator==(const FieldType& rhs) const {
        return name_ == rhs.name_ && *type_ == *rhs.type_;
    }
    bool operator!=(const FieldType& rhs) const {
        return !(*this == rhs);
    }

private:
    friend struct nrpc::BufferReaderWriter<FieldType>;

    std::string name_;
    ValueTypePtr type_;
};


inline std::ostream& operator<<(std::ostream& os, const FieldType& type) {
    return os << type.toString();
}

SERIALIZE_EACH_MEMBER(FieldType, name_, type_)

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::FieldType> {
    std::size_t operator()(const nebula::FieldType& v) const noexcept {
        return v.hash();
    }
};

}  // namespace std
