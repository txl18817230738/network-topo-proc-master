// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <initializer_list>

#include "nebula/common/base/Status.h"
#include "nebula/common/datatype/Value.h"
#include "nebula/common/memory/StlAllocator.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"
#include "nebula/common/nrpc/Context.h"

namespace nebula {

class Row final {
public:
    using allocator_type = memory::StlAllocator<Value>;
    using vector_type = std::vector<Value, allocator_type>;

    allocator_type get_allocator() const noexcept {
        return values_.get_allocator();
    }

    Row(std::initializer_list<Value> values);

    explicit Row(const allocator_type& alloc = allocator_type()) : values_(alloc) {}

    explicit Row(const std::vector<Value>& values,
                 const allocator_type& alloc = allocator_type())
            : values_(values.begin(), values.end(), alloc) {}

    explicit Row(vector_type&& values) : values_(std::move(values)) {}

    Row(const Row& other, const allocator_type& alloc = allocator_type())
            : values_(other.values_, alloc) {}

    Row& operator=(const Row& s) = default;

    Row(Row&& other, const allocator_type& alloc = allocator_type()) noexcept
            : values_(std::move(other.values_), alloc) {}

    ~Row() {}

    // Append the value to the end of the record.
    void append(const Value& value) {
        values_.emplace_back(value);
    }
    void append(Value&& value) {
        values_.emplace_back(std::move(value));
    }

    void append(const Value& value, std::size_t count) {
        values_.insert(values_.end(), count, value);
    }

    void append(const Row& r) {
        values_.insert(values_.end(), r.values_.begin(), r.values_.end());
    }

    void append(Row&& r) {
        values_.insert(values_.end(),
                       std::make_move_iterator(r.values_.begin()),
                       std::make_move_iterator(r.values_.end()));
    }

    size_t size() const {
        return values_.size();
    }

    bool empty() const {
        return values_.empty();
    }

    // Get the field value at the given index with bounding check.
    const Value& at(size_t fieldIndex) const {
        DCHECK_LT(fieldIndex, values_.size());
        return values_[fieldIndex];
    }

    // Get the field value at the given index.
    const Value& operator[](size_t fieldIndex) const {
        CHECK_LT(fieldIndex, values_.size());
        return values_[fieldIndex];
    }

    Value& operator[](size_t fieldIndex) {
        CHECK_LT(fieldIndex, values_.size());
        return values_[fieldIndex];
    }

    const vector_type& values() const {
        return values_;
    }

    // Reserve capacity for the number of fields.
    void reserve(size_t size) {
        values_.reserve(size);
    }

    void resize(size_t size) {
        values_.resize(size);
    }

    void maxLen(std::vector<size_t>& maxLen) const {
        CHECK_LE(values_.size(), maxLen.size());
        for (size_t i = 0; i < values_.size(); ++i) {
            size_t len = values_[i].toString().length();
            if (len > maxLen[i]) maxLen[i] = len;
        }
    }

    // Just for debug
    std::string toString(const std::vector<size_t>& maxLen = {}) const;

private:
    friend struct nrpc::BufferReaderWriter<Row>;

    // TODO(jie): Replace `values_` with:
    // Value* values_;
    // size_t size_;   // The size_ may also be omitted.
    vector_type values_;
};

bool operator==(const Row& lhs, const Row& rhs);
bool operator!=(const Row& lhs, const Row& rhs);

template <>
struct nrpc::BufferReaderWriter<Row> {
    static void write(folly::IOBuf* buf, const Row& record) {
        BufferReaderWriter<Row::vector_type>::write(buf, record.values_);
    }

    static Status read(folly::IOBuf* buf, Row* record) {
        return BufferReaderWriter<Row::vector_type>::read(buf, &record->values_);
    }

    static size_t encodedSize(const Row& record) {
        return BufferReaderWriter<Row::vector_type>::encodedSize(record.values_);
    }
};

using RowIdMap = NMap<Row, NodeID>;
using RowNodeMap = NMap<Row, Node>;
using RowSet = NSet<Row>;
using RowList = NVector<Row>;

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::Row> {
    size_t operator()(const nebula::Row& r) const {
        return nebula::hashContainer(r.values());
    }
};

}  // namespace std
