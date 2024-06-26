// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/datatype/Time.h"
namespace nebula {

struct ZonedTime {
    LocalTime time;
    ZonedTime() = default;
    ZonedTime(int8_t h, int8_t min, int8_t s, int32_t us) : time(h, min, s, us) {}
    explicit ZonedTime(LocalTime t) : time(t) {}

    bool operator==(const ZonedTime& rhs) const {
        return time == rhs.time;
    }

    bool operator<(const ZonedTime& rhs) const {
        return time < rhs.time;
    }

    void clear() {
        time.clear();
    }

    void addDuration(const Duration& duration) {
        time.addDuration(duration);
    }

    void subDuration(const Duration& duration) {
        time.subDuration(duration);
    }

    std::string toString() const {
        return fmt::format("ZONED {}", time.toString());
    }

private:
    friend struct nrpc::BufferReaderWriter<ZonedTime>;
};

static_assert(sizeof(ZonedTime) == 8, "ZonedTime should be 8 bytes");

inline std::ostream& operator<<(std::ostream& os, const ZonedTime& d) {
    os << d.toString();
    return os;
}

inline ZonedTime operator+(const ZonedTime& l, const Duration& r) {
    ZonedTime t = l;
    t.addDuration(r);
    return t;
}

inline ZonedTime operator-(const ZonedTime& l, const Duration& r) {
    ZonedTime t = l;
    t.subDuration(r);
    return t;
}

template <>
struct nrpc::BufferReaderWriter<nebula::ZonedTime> {
    static void write(folly::IOBuf* buf, const nebula::ZonedTime& zonedTime) {
        BufferReaderWriter<nebula::LocalTime>::write(buf, zonedTime.time);
    }

    static Status read(folly::IOBuf* buf, nebula::ZonedTime* zonedTime) {
        return BufferReaderWriter<nebula::LocalTime>::read(buf, &zonedTime->time);
    }

    static size_t encodedSize(const nebula::ZonedTime& zonedTime) {
        return BufferReaderWriter<nebula::LocalTime>::encodedSize(zonedTime.time);
    }
};

}  // namespace nebula

namespace std {

// Inject a customized hash function
template <>
struct hash<nebula::ZonedTime> {
    std::size_t operator()(const nebula::ZonedTime& t) const noexcept {
        return std::hash<nebula::LocalTime>()(t.time);
    }
};
}  // namespace std
