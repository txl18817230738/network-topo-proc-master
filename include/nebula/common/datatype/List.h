// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <cstddef>

#include "nebula/common/datatype/Value.h"

/****************************************************************
 * List: std::vector<Value>
 ****************************************************************/
namespace nebula {
class List final {
public:
    using allocator_type = nebula::memory::StlAllocator<Value>;
    using vector_type = std::vector<Value, allocator_type>;

    allocator_type get_allocator() const noexcept {
        return values_.get_allocator();
    }

    explicit List(const allocator_type& alloc = allocator_type()) : values_(alloc) {}

    explicit List(const vector_type& values, const allocator_type& alloc = allocator_type())
            : values_(values.begin(), values.end(), alloc) {}

    List(const List& other, const allocator_type& alloc = allocator_type())
            : values_(other.values_, alloc) {}

    List(List&& other, const allocator_type& alloc = allocator_type()) noexcept
            : values_(std::move(other.values_), alloc) {}

    List& operator=(const List& other) {
        if (&other != this) {
            values_ = other.values_;
        }
        return *this;
    }

    List& operator=(List&& other) {
        if (&other != this) {
            values_ = std::move(other.values_);
        }
        return *this;
    }

    // TODO(wuu): add `type_` data member
    ValueTypeKind type() const {
        return empty() ? ValueTypeKind::kNull : values_.front().getType();
    }

    const auto& values() const {
        return values_;
    }

    auto& values() {
        return values_;
    }

    size_t size() const {
        return values_.size();
    }

    const auto& front() const {
        return values_.front();
    }
    auto& front() {
        return values_.front();
    }

    const auto& back() const {
        return values_.back();
    }
    auto& back() {
        return values_.back();
    }

    auto begin() const {
        return values_.begin();
    }
    auto end() const {
        return values_.end();
    }

    bool contains(const Value& val) const {
        return std::find(values_.begin(), values_.end(), val) != values_.end();
    }

    const Value& operator[](size_t i) const {
        return values_[i];
    }

    void reserve(std::size_t s) {
        values_.reserve(s);
    }

    void resize(std::size_t s) {
        values_.resize(s);
    }

    template <typename... Args>
    void emplaceBack(Args&&... args) {
        values_.emplace_back(std::forward<Args>(args)...);
    }

    void merge(List&& other) {
        values_.reserve(values_.size() + other.size());
        values_.insert(values_.end(), other.values_.begin(), other.values_.end());
    }

    bool operator<(const List& rhs) const {
        return values_ < rhs.values_;
    }

    std::string toString() const;

    void hashKey(HashKeyAppender& appender) const {
        for (auto& value : values_) {
            value.hashKey(appender);
        }
    }

    bool empty() const {
        return values_.empty();
    }

    void clear() {
        values_.clear();
    }

private:
    friend struct nrpc::BufferReaderWriter<List>;
    vector_type values_;
};

inline std::ostream& operator<<(std::ostream& os, const List& list) {
    return os << list.toString();
}

bool operator==(const List& lhs, const List& rhs);
bool operator!=(const List& lhs, const List& rhs);


template <>
struct nrpc::BufferReaderWriter<nebula::List> {
    static void write(folly::IOBuf* buf, const nebula::List& list) {
        BufferReaderWriter<std::vector<Value, nebula::memory::StlAllocator<Value>>>::write(
                buf, list.values_);
    }

    static Status read(folly::IOBuf* buf, nebula::List* list) {
        return BufferReaderWriter<
                std::vector<Value, nebula::memory::StlAllocator<Value>>>::read(buf,
                                                                               &list->values_);
    }

    static size_t encodedSize(const nebula::List& list) {
        return BufferReaderWriter<std::vector<Value, nebula::memory::StlAllocator<Value>>>::
                encodedSize(list.values_);
    }
};

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::List> {
    std::size_t operator()(const nebula::List& list) const noexcept {
        return nebula::hashContainer(list.values());
    }
};

}  // namespace std
