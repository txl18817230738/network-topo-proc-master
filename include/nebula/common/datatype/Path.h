// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <cstddef>

#include "nebula/common/datatype/Edge.h"
#include "nebula/common/datatype/Node.h"
#include "nebula/common/datatype/Value.h"

namespace nebula {

// Path form: [v1, e1, v2, e2, e3, e4, v3]
// - `front()` and `back()` must be nodes
// - it may have no nodes between two edges (e.g. [v1, e1, e2, v2])
// - it must have edges between two nodes
// - `size()` returns the actual number of members in the path
// - `length()` returns the number of edges in the path
class Path final {
public:
    using allocator_type = memory::StlAllocator<Value>;
    using vector_type = std::vector<Value, allocator_type>;

    allocator_type get_allocator() const noexcept {
        return values_.get_allocator();
    }

    explicit Path(const allocator_type& alloc = allocator_type()) : values_(alloc) {}

    explicit Path(const vector_type& values, const allocator_type& alloc = allocator_type())
            : values_(values.begin(), values.end(), alloc) {}

    Path(const Path& other, const allocator_type& alloc = allocator_type())
            : values_(other.values_, alloc) {}

    Path(Path&& other, const allocator_type& alloc = allocator_type()) noexcept
            : values_(std::move(other.values_), alloc) {}

    Path& operator=(const Path& other) = default;

    Path& operator=(Path&& other) {
        if (&other != this) {
            values_ = std::move(other.values_);
        }
        return *this;
    }

    const auto& values() const {
        return values_;
    }

    auto& values() {
        return values_;
    }

    // FIXME(Aiee) Update when refactor Path
    // Returns the number of members in the path
    // Path size != Path length
    size_t size() const {
        return values_.size();
    }
    // Returns the length of the path, which is also the number of edges.
    // The path may contain variable-length pattern, i.g. [(v1), (e1), (v2), [e2, e3, e4], (v3)]
    //
    // GQL 4.15.2.2 Path value types section specifies the length of Path containing 0 node is
    // the null value, but in our implementation we return 0.
    size_t length() const {
        return std::count_if(
                values_.begin(), values_.end(), [](const auto& v) { return v.isEdge(); });
    }

    const Node& front() const {
        if (values_.empty()) {
            LOG(FATAL) << "Path is empty";
        }
        CHECK(values_.front().isNode());
        return values_.front().getNode();
    }

    const auto& back() const {
        if (values_.empty()) {
            LOG(FATAL) << "Path is empty";
        }
        CHECK(values_.back().isNode());
        return values_.back().getNode();
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

    bool operator<(const Path& rhs) const {
        return values_ < rhs.values_;
    }

    // FIXME(Aiee) the path string format is not defined yet
    std::string toString() const;

    void hashKey(HashKeyAppender& appender) const {
        for (auto& value : values_) {
            value.hashKey(appender);
        }
    }

    bool empty() const {
        return values_.empty();
    }

    // for debug
    bool isValid() const;

private:
    friend struct nrpc::BufferReaderWriter<Path>;
    vector_type values_;
};

inline std::ostream& operator<<(std::ostream& os, const Path& path) {
    return os << path.toString();
}

bool operator==(const Path& left, const Path& right);

template <>
struct nrpc::BufferReaderWriter<nebula::Path> {
    static void write(folly::IOBuf* buf, const nebula::Path& path) {
        BufferReaderWriter<std::vector<Value, nebula::memory::StlAllocator<Value>>>::write(
                buf, path.values_);
    }

    static Status read(folly::IOBuf* buf, nebula::Path* path) {
        return BufferReaderWriter<
                std::vector<Value, nebula::memory::StlAllocator<Value>>>::read(buf,
                                                                               &path->values_);
    }

    static size_t encodedSize(const nebula::Path& path) {
        return BufferReaderWriter<std::vector<Value, nebula::memory::StlAllocator<Value>>>::
                encodedSize(path.values_);
    }
};

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::Path> {
    size_t operator()(const nebula::Path& p) const noexcept {
        return nebula::hashContainer(p.values());
    }
};

}  // namespace std
