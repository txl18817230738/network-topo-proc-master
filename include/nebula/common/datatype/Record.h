// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <memory>
#include <string_view>

#include "nebula/common/datatype/Value.h"

namespace nebula {
class Record final {
public:
    using properties_type = nebula::properties_type;
    using allocator_type = nebula::properties_type::allocator_type;

    Record() = default;

    explicit Record(const properties_type& values) : fields_(values) {}
    explicit Record(properties_type&& values) : fields_(std::move(values)) {}

    Record(const Record& other) : fields_(other.fields()) {}
    Record(Record&& other) noexcept : fields_(std::move(other.fields_)) {}

    Record& operator=(const Record&) = default;
    Record& operator=(Record&&) = default;

    const properties_type& properties() const {
        return fields_;
    }
    const properties_type& fields() const {
        return fields_;
    }

    properties_type& fields() {
        return fields_;
    }

    size_t size() const {
        return fields_.size();
    }

    const Value& operator[](std::string_view key) const {
        return fields_.at(key.data());
    }

    bool hasField(std::string_view field) const {
        return fields_.find(field.data()) != fields_.end();
    }

    const Value& getValue(std::string_view field) const {
        DCHECK(hasField(field)) << "Field not found: " << field;
        return fields_.at(field.data());
    }

    void clear() {
        fields_.clear();
    }

    template <typename... Args>
    auto emplace(Args&&... args) {
        fields_.emplace(std::forward<Args>(args)...);
    }

    std::string toString() const;

    void hashKey(HashKeyAppender& appender) const {
        for (auto& value : fields_) {
            appender.append(value.first);
            value.second.hashKey(appender);
        }
    }

private:
    friend struct nrpc::BufferReaderWriter<Record>;
    // The key of gql's record can be other types except string.
    properties_type fields_;
};

inline std::ostream& operator<<(std::ostream& os, const Record& record) {
    return os << record.toString();
}

bool operator==(const Record& lhs, const Record& rhs);
bool operator!=(const Record& lhs, const Record& rhs);

template <>
struct nrpc::BufferReaderWriter<nebula::Record> {
    static void write(folly::IOBuf* buf, const nebula::Record& record) {
        BufferReaderWriter<Record::properties_type>::write(buf, record.fields_);
    }

    static Status read(folly::IOBuf* buf, nebula::Record* record) {
        return BufferReaderWriter<Record::properties_type>::read(buf, &record->fields_);
    }

    static size_t encodedSize(const nebula::Record& record) {
        return BufferReaderWriter<Record::properties_type>::encodedSize(record.fields_);
    }
};

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::Record> {
    std::size_t operator()(const nebula::Record& record) const noexcept {
        return nebula::hashContainer(record.fields());
    }
};

}  // namespace std
