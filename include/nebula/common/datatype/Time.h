// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <ostream>

#include "nebula/common/datatype/Duration.h"
namespace nebula {

struct LocalTime {
    int8_t hour;
    int8_t minute;
    int8_t sec;
    // TODO(Aiee) This padding is temporary since we use arrow as the data type provider in
    // function
    int8_t padding{0};
    int32_t microsec;

    LocalTime() : hour{0}, minute{0}, sec{0}, microsec{0} {}
    LocalTime(int8_t h, int8_t min, int8_t s, int32_t us)
            : hour{h}, minute{min}, sec{s}, microsec{us} {}

    void clear() {
        hour = 0;
        minute = 0;
        sec = 0;
        microsec = 0;
    }

    bool operator==(const LocalTime& rhs) const {
        return hour == rhs.hour && minute == rhs.minute && sec == rhs.sec &&
               microsec == rhs.microsec;
    }

    bool operator<(const LocalTime& rhs) const {
        if (!(hour == rhs.hour)) {
            return hour < rhs.hour;
        }
        if (!(minute == rhs.minute)) {
            return minute < rhs.minute;
        }
        if (!(sec == rhs.sec)) {
            return sec < rhs.sec;
        }
        if (!(microsec == rhs.microsec)) {
            return microsec < rhs.microsec;
        }
        return false;
    }

    void addDuration(const Duration& duration);
    void subDuration(const Duration& duration);

    // 'Z' representing UTC timezone
    std::string toString() const;

private:
    friend struct nrpc::BufferReaderWriter<LocalTime>;
};

static_assert(sizeof(LocalTime) == 8, "LocalTime should be 8 bytes");

inline std::ostream& operator<<(std::ostream& os, const LocalTime& d) {
    os << d.toString();
    return os;
}

inline LocalTime operator+(const LocalTime& l, const Duration& r) {
    LocalTime t = l;
    t.addDuration(r);
    return t;
}

inline LocalTime operator-(const LocalTime& l, const Duration& r) {
    LocalTime t = l;
    t.subDuration(r);
    return t;
}

template <>
struct nrpc::BufferReaderWriter<nebula::LocalTime> {
    static void write(folly::IOBuf* buf, const nebula::LocalTime& time) {
        BufferReaderWriter<int8_t>::write(buf, time.hour);
        BufferReaderWriter<int8_t>::write(buf, time.minute);
        BufferReaderWriter<int8_t>::write(buf, time.sec);
        BufferReaderWriter<int32_t>::write(buf, time.microsec);
    }

    static Status read(folly::IOBuf* buf, nebula::LocalTime* time) {
        NG_RETURN_IF_ERROR(BufferReaderWriter<int8_t>::read(buf, &time->hour));
        NG_RETURN_IF_ERROR(BufferReaderWriter<int8_t>::read(buf, &time->minute));
        NG_RETURN_IF_ERROR(BufferReaderWriter<int8_t>::read(buf, &time->sec));
        return BufferReaderWriter<int32_t>::read(buf, &time->microsec);
    }

    static size_t encodedSize(const nebula::LocalTime& time) {
        return BufferReaderWriter<int8_t>::encodedSize(time.hour) +
               BufferReaderWriter<int8_t>::encodedSize(time.minute) +
               BufferReaderWriter<int8_t>::encodedSize(time.sec) +
               BufferReaderWriter<int32_t>::encodedSize(time.microsec);
    }
};

}  // namespace nebula

namespace std {

// Inject a customized hash function
template <>
struct hash<nebula::LocalTime> {
    std::size_t operator()(const nebula::LocalTime& h) const noexcept {
        return folly::hash::hash_combine(h.hour, h.minute, h.sec, h.microsec);
    }
};

}  // namespace std
