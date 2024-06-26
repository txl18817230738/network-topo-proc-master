// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/base/Base.h"
#include "nebula/common/datatype/ValueTypeKind.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"

namespace nebula {

using RefEntryID = uint32_t;

class Ref {
public:
    Ref() = default;
    Ref(RefKind kind, RefEntryID entryID, uint32_t offset = 0) {
        code_ = (kind << 24) | (entryID & 0x00FFFFFF);
        offset_ = offset;
    }

    RefKind kind() const {
        return static_cast<RefKind>(code_ >> 24);
    }
    RefEntryID entryID() const {
        return static_cast<uint32_t>(code_ & 0x00FFFFFF);
    }
    uint32_t code() const {
        return code_;
    }
    uint32_t offset() const {
        return offset_;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << "Ref code: " << code_ << " offset: " << offset_;
        return ss.str();
    }

    void reset(uint32_t code, uint32_t offset) {
        code_ = code;
        offset_ = offset;
    }

    void setCode(uint32_t code) {
        code_ = code;
    }

    void setOffset(uint32_t offset) {
        offset_ = offset;
    }

private:
    friend struct nrpc::BufferReaderWriter<Ref>;

    uint32_t code_{0xFF000000};
    uint32_t offset_{0};
};

static_assert(sizeof(Ref) == 8, "Ref shall be 8 bytes.");

using RefPtr = Ref*;

inline std::ostream& operator<<(std::ostream& os, const Ref& path) {
    return os << path.toString();
}

bool operator==(const Ref& left, const Ref& right);
bool operator!=(const Ref& lhs, const Ref& rhs);
bool operator<(const Ref& lhs, const Ref& rhs);

template <>
struct nrpc::BufferReaderWriter<nebula::Ref> {
    static void write(folly::IOBuf* buf, const nebula::Ref& ref) {
        BufferReaderWriter<uint32_t>::write(buf, ref.code());
        BufferReaderWriter<uint32_t>::write(buf, ref.offset());
    }

    static Status read(folly::IOBuf* buf, nebula::Ref* ref) {
        NG_RETURN_IF_ERROR(BufferReaderWriter<uint32_t>::read(buf, &ref->code_));
        NG_RETURN_IF_ERROR(BufferReaderWriter<uint32_t>::read(buf, &ref->offset_));
        return Status::OK();
    }

    static size_t encodedSize(const nebula::Ref&) {
        return sizeof(uint32_t) * 2;
    }
};

}  // namespace nebula


namespace std {

template <>
struct hash<nebula::Ref> {
    std::size_t operator()(const nebula::Ref& r) const noexcept {
        return folly::hash::fnv64_buf(reinterpret_cast<const void*>(&r), sizeof(r));
    }
};

}  // namespace std
