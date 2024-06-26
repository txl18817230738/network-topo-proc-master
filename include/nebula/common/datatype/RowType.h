// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/datatype/FieldType.h"

namespace nebula {

class RowType final {
private:
    std::vector<FieldType> fieldTypes_;

public:
    RowType() = default;

    // construct a single type, single column row, without letting the user to pass in a
    // std::vector
    explicit RowType(FieldType fieldType) {
        fieldTypes_.resize(1);
        fieldTypes_[0] = fieldType;
    }
    explicit RowType(std::string fieldName, ValueTypePtr valueType) {
        fieldTypes_.resize(1);
        fieldTypes_[0] = FieldType(std::move(fieldName), std::move(valueType));
    }
    // construct via std::vector
    explicit RowType(std::vector<FieldType> fields) : fieldTypes_(std::move(fields)) {}
    RowType(std::vector<std::string> names, const std::vector<ValueTypePtr>& valueTypes) {
        DCHECK(names.size() == valueTypes.size());
        for (size_t i = 0; i < names.size(); ++i) {
            fieldTypes_.emplace_back(std::move(names[i]), valueTypes[i]);
        }
    }

    // use this with caution!
    static RowType makeAny(std::vector<std::string> names) {
        std::vector<ValueTypePtr> valueTypes(names.size(), std::make_shared<AnyValueType>());
        return {std::move(names), std::move(valueTypes)};
    }

    const std::vector<FieldType>& fieldTypes() const {
        return fieldTypes_;
    }

    size_t size() const {
        return fieldTypes_.size();
    }

    bool empty() const {
        return fieldTypes_.empty();
    }

    std::vector<std::string> names() const {
        std::vector<std::string> names;
        names.reserve(fieldTypes_.size());
        for (auto& field : fieldTypes_) {
            names.emplace_back(field.name());
        }
        return names;
    }

    std::unordered_set<std::string> nameSet() const {
        std::unordered_set<std::string> names;
        names.reserve(fieldTypes_.size());
        for (auto& field : fieldTypes_) {
            names.emplace(field.name());
        }
        return names;
    }

    std::vector<ValueTypePtr> types() const {
        std::vector<ValueTypePtr> types;
        types.reserve(fieldTypes_.size());
        for (auto& field : fieldTypes_) {
            types.emplace_back(field.type());
        }
        return types;
    }

    std::unordered_set<FieldType> fieldSet() const {
        std::unordered_set<FieldType> fields;
        fields.reserve(fieldTypes_.size());
        for (auto& field : fieldTypes_) {
            fields.emplace(field);
        }
        return fields;
    }

    size_t indexOf(const std::string& name) const {
        auto ret = maybeIndexOf(name);
        CHECK(ret.has_value());
        return ret.value();
    }
    std::optional<size_t> maybeIndexOf(const std::string& name) const {
        auto iter = std::find_if(fieldTypes_.begin(),
                                 fieldTypes_.end(),
                                 [&name](const FieldType& ft) { return ft.name() == name; });
        if (iter == fieldTypes_.end()) {
            return std::nullopt;
        }
        return iter - fieldTypes_.begin();
    }

    const std::string& nameOf(size_t index) const {
        CHECK_LT(index, fieldTypes_.size());
        return fieldTypes_[index].name();
    }
    std::optional<std::string> maybeNameOf(size_t index) const {
        if (index >= fieldTypes_.size()) {
            return std::nullopt;
        }
        return fieldTypes_[index].name();
    }

    const FieldType& operator[](size_t index) const {
        DCHECK_LT(index, fieldTypes_.size());
        return fieldTypes_[index];
    }

    FieldType& operator[](size_t index) {
        DCHECK_LT(index, fieldTypes_.size());
        return fieldTypes_[index];
    }

    bool contains(const std::string name) const {
        return std::find_if(
                       fieldTypes_.begin(), fieldTypes_.end(), [&name](const FieldType& ft) {
                           return ft.name() == name;
                       }) != fieldTypes_.end();
    }

    auto begin() {
        return fieldTypes_.begin();
    }
    auto end() {
        return fieldTypes_.end();
    }
    auto begin() const {
        return fieldTypes_.begin();
    }
    auto end() const {
        return fieldTypes_.end();
    }

    auto& front() {
        return fieldTypes_.front();
    }

    auto& back() {
        return fieldTypes_.back();
    }

    const auto& front() const {
        return fieldTypes_.front();
    }

    const auto& back() const {
        return fieldTypes_.back();
    }

    // Returns the index of the field with the given name. If the field does not exist, returns
    // -1.
    ssize_t getIndex(const std::string& name, bool forward = true) const {
        auto n = fieldTypes_.size();
        for (size_t i = 0u; i < n; ++i) {
            auto idx = forward ? i : n - i - 1;
            if (fieldTypes_[idx].name() == name) {
                return idx;
            }
        }
        DLOG(ERROR) << "Field `" << name << "' not found";
        return -1;
    }


    size_t hash() const {
        size_t hash = 0;
        for (auto& fieldType : fieldTypes_) {
            hash = folly::hash::hash_combine(hash, fieldType.hash());
        }
        return hash;
    }

    std::string toString(bool verbose = false) const {
        std::vector<std::string> fieldsStrVec;
        fieldsStrVec.reserve(fieldTypes_.size());
        for (auto& field : fieldTypes_) {
            fieldsStrVec.emplace_back(field.toString(verbose));
        }
        return fmt::format("{}", fmt::join(fieldsStrVec, ", "));
    }

    // merge two RowType, the result is the union of the two RowType
    void merge(const RowType& other) {
        // Only append the field if it does not exist in the current RowType
        for (auto& fieldType : other.fieldTypes()) {
            if (std::find_if(fieldTypes_.begin(),
                             fieldTypes_.end(),
                             [&fieldType](const FieldType& ft) {
                                 return ft.name() == fieldType.name();
                             }) == fieldTypes_.end()) {
                fieldTypes_.emplace_back(fieldType);
            }
        }
    }

    template <typename... Args>
    void append(Args&&... args) {
        fieldTypes_.emplace_back(std::forward<Args>(args)...);
    }

    void reserve(size_t n) {
        fieldTypes_.reserve(n);
    }

    void clear() {
        fieldTypes_.clear();
    }

    bool operator==(const RowType& rhs) const {
        return fieldTypes_ == rhs.fieldTypes_;
    }
    bool operator!=(const RowType& rhs) const {
        return !(*this == rhs);
    }
    friend struct nrpc::BufferReaderWriter<RowType>;
};

inline std::ostream& operator<<(std::ostream& os, const RowType& type) {
    return os << type.toString();
}

SERIALIZE_EACH_MEMBER(RowType, fieldTypes_)

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::RowType> {
    std::size_t operator()(const nebula::RowType& v) const noexcept {
        return v.hash();
    }
};

}  // namespace std
