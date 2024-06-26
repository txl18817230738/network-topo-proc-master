// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <limits>

#include <folly/io/IOBuf.h>

#include "nebula/common/datatype/Row.h"
#include "nebula/common/memory/StlAllocator.h"

namespace nebula {

class ResultTable final {
public:
    using deque_type = std::deque<Row, memory::StlAllocator<Row>>;
    using colname_type = std::vector<std::string>;

    ResultTable() : records_(memory::StlAllocator<deque_type>()) {}

    Row& append(const Row& record) {
        return records_.emplace_back(record);
    }
    const Row& at(size_t index) const {
        DCHECK_LT(index, records_.size());
        return records_.at(index);
    }
    Row& at(size_t index) {
        DCHECK_LT(index, records_.size());
        return records_.at(index);
    }

    const colname_type& getColumnNames() const {
        return columnNames_;
    }
    void setColumnNames(const std::vector<std::string>& colNames) {
        columnNames_.resize(colNames.size());
        for (std::size_t i = 0; i < colNames.size(); ++i) {
            columnNames_[i] = colNames[i];
        }
    }
    const deque_type& getRecords() const {
        return records_;
    }
    deque_type& getRecords() {
        return records_;
    }
    size_t getNumRecords() const {
        return records_.size();
    }
    void merge(const ResultTable& other) {
        records_.insert(records_.end(), other.records_.begin(), other.records_.end());
    }
    void merge(ResultTable&& other) {
        records_.insert(records_.end(),
                        std::make_move_iterator(other.records_.begin()),
                        std::make_move_iterator(other.records_.end()));
    }

    template <ValueTypeKind TYPE>
    static inline int compareAsc(const Value& lhs, const Value& rhs);

    void clear() {
        records_.clear();
    }

    std::string toString(size_t num = std::numeric_limits<size_t>::max()) const;

private:
    colname_type columnNames_;
    // TODO: Rethink how to avoid the data copy from binding table to result table
    deque_type records_;
};

using ResultTablePtr = std::shared_ptr<ResultTable>;

bool operator==(const ResultTable& lhs, const ResultTable& rhs);
bool operator!=(const ResultTable& lhs, const ResultTable& rhs);

// Base implement for primitive types
template <ValueTypeKind TYPE>
inline int ResultTable::compareAsc(const Value& lhs, const Value& rhs) {
    using T = typename TypeTraits<TYPE>::NativeType;
    if constexpr (TypeTraits<TYPE>::isPrimitiveType) {
        return comparePrimitiveAsc(lhs.get<T>(), rhs.get<T>());
    }
    return false;
}

template <>
inline int ResultTable::compareAsc<ValueTypeKind::kList>(const Value& lhs, const Value& rhs) {
    UNUSED(lhs);
    UNUSED(rhs);
    return 0;
}

template <>
inline int ResultTable::compareAsc<ValueTypeKind::kNode>(const Value& lhs, const Value& rhs) {
    UNUSED(lhs);
    UNUSED(rhs);
    return 0;
}

template <>
inline int ResultTable::compareAsc<ValueTypeKind::kEdge>(const Value& lhs, const Value& rhs) {
    UNUSED(lhs);
    UNUSED(rhs);
    return 0;
}

}  // namespace nebula
