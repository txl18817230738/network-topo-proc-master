// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/datatype/Value.h"
#include "nebula/common/datatype/ValueTypeKind.h"

namespace nebula {

/****************************************************************
 * StringView
 * - 16B = 4B size + 4B prefix + pointer (to full string)
 ****************************************************************/
class StringView {
public:
    static constexpr uint32_t kPrefixSize = 4;
    static constexpr uint32_t kInlineSize = 12;

    StringView() {
        memset(this, 0, sizeof(StringView));
    }
    StringView(const char* str, uint32_t len) : size_(len) {
        DCHECK(str || len == 0);
        if (isInline()) {
            // empty str's first 8 bytes should be 0
            memset(prefix_, 0, kPrefixSize);
            if (size_ == 0) {
                return;
            }
            value_.data = nullptr;
            memcpy(prefix_, str, size_);
        } else {
            memcpy(prefix_, str, kPrefixSize);
            value_.data = str;
        }
    }
    /* implicit */ StringView(const char* data)  // NOLINT
            : StringView(data, strlen(data)) {}
    StringView(const String& str)  // NOLINT
            : StringView{str.data(), static_cast<uint32_t>(str.size())} {}
    StringView(const std::string& str)  // NOLINT
            : StringView{str.data(), static_cast<uint32_t>(str.size())} {}
    StringView(const std::string_view& str)  // NOLINT
            : StringView{str.data(), static_cast<uint32_t>(str.size())} {}

    std::string_view toCppStringView() const {
        return std::string_view{*this};
    }
    String toNebulaString() const {
        return {data(), size()};
    }

    Value toValue() const {
        return {toString()};
    }

    String toNative() const {
        return String{data(), size()};
    }

    operator std::string_view() const {  // NOLINT
        return {data(), size()};
    }

    // should we return nullptr when `size_` == 0?
    const char* data() const {
        return isInline() ? prefix_ : value_.data;
    }

    bool isAllAscii() const {
        const char* d = data();
        for (auto i = 0u; i < size_; i++) {
            if (d[i] & 0x80) {
                return false;
            }
        }
        return true;
    }

    void unsafeUpdateData(const char* data) {
        // DCHECK(!isInline());
        value_.data = data;
    }

    uint32_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    const char* begin() && = delete;
    const char* begin() const& {
        return data();
    }

    const char* end() && = delete;
    const char* end() const& {
        return data() + size();
    }

    bool isInline() const {
        return size_ <= kInlineSize;
    }

    // operators: ==, !=, <, <=, >, >=

    // in inlined case, we can get the result by (at most) 2 int64_t comparisons
    bool operator==(const StringView& other) const {
        if (sizeAndPrefixAsInt64() != other.sizeAndPrefixAsInt64()) {
            return false;
        }
        if (isInline()) {
            return size_ <= kPrefixSize || inlinedAsInt64() == other.inlinedAsInt64();
        }
        return memcmp(value_.data + kPrefixSize,
                      other.value_.data + kPrefixSize,
                      size_ - kPrefixSize) == 0;
    }
    bool operator!=(const StringView& other) const {
        return !(*this == other);
    }

    // Returns 0, if this == other
    //       < 0, if this < other
    //       > 0, if this > other
    int32_t compare(const StringView& other) const {
        // compare prefix first
        int32_t prefixRes = memcmp(prefix_, other.prefix_, kPrefixSize);
        if (prefixRes != 0) {
            return prefixRes;
        }
        int32_t size = std::min(size_, other.size_) - kPrefixSize;
        if (size <= 0) {
            // both of them has the same prefix, and the shorter one's size <= kPrefixSize
            return size_ - other.size_;
        }
        if (isInline() && other.isInline()) {
            DCHECK_LE(size, kInlineSize - kPrefixSize);
            int32_t result = memcmp(value_.inlined, other.value_.inlined, size);
            return (result != 0) ? result : size_ - other.size_;
        }
        int32_t result = memcmp(data() + kPrefixSize, other.data() + kPrefixSize, size);
        return (result != 0) ? result : size_ - other.size_;
    }

    bool operator<(const StringView& other) const {
        return compare(other) < 0;
    }

    bool operator<=(const StringView& other) const {
        return compare(other) <= 0;
    }

    bool operator>(const StringView& other) const {
        return compare(other) > 0;
    }

    bool operator>=(const StringView& other) const {
        return compare(other) >= 0;
    }

    std::string toString() const {
        return {data(), size()};
    }

private:
    inline int64_t sizeAndPrefixAsInt64() const {
        return reinterpret_cast<const int64_t*>(this)[0];
    }

    inline int64_t inlinedAsInt64() const {
        return reinterpret_cast<const int64_t*>(this)[1];
    }

    int32_t prefixAsInt() const {
        return *reinterpret_cast<const int32_t*>(&prefix_);
    }

    uint32_t size_{0};
    char prefix_[4];
    union {
        char inlined[8];
        const char* data;
    } value_;
};

static_assert(sizeof(StringView) == 16, "StringView should be 16 bytes");

inline std::ostream& operator<<(std::ostream& os, const StringView& value) {
    return os << value.toCppStringView();
}

}  // namespace nebula

namespace std {
template <>
struct hash<nebula::StringView> {
    size_t operator()(const nebula::StringView& key) const noexcept {
        return std::hash<std::string_view>()(key.toCppStringView());
    }
};
}  // namespace std
