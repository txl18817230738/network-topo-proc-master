// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/datatype/Datetime.h"
#include "nebula/common/datatype/ZonedTime.h"
namespace nebula {

// In nebula only store UTC time, and the interpretation of time value based on
// the timezone configuration in current system.

// An instant capturing the date and the time, contain the time zone.
//
// ISO 8601-1:2019 is the standard for date and time representation specified in the doc.
// https://en.wikipedia.org/wiki/ISO_8601
struct ZonedDatetime {
    LocalDatetime datetime;
    ZonedDatetime() : datetime(1970, 1, 1, 0, 0, 0, 0) {}
    ZonedDatetime(int16_t y, int8_t m, int8_t d, int8_t h, int8_t min, int8_t s, int32_t us)
            : datetime(y, m, d, h, min, s, us) {}

    explicit ZonedDatetime(const Date& date) : datetime(date) {}

    ZonedDatetime(const Date& date, const LocalTime& time) : datetime(date, time) {}

    ZonedDatetime(const Date& date, const ZonedTime& zonedTime) {
        datetime.year = date.year;
        datetime.month = date.month;
        datetime.day = date.day;
        datetime.hour = zonedTime.time.hour;
        datetime.minute = zonedTime.time.minute;
        datetime.sec = zonedTime.time.sec;
        datetime.microsec = zonedTime.time.microsec;
    }

    explicit ZonedDatetime(int64_t _qword) {
        datetime.qword = _qword;
    }

    static ZonedDatetime microsToZonedDatetime(int64_t micros) {
        auto us = std::chrono::microseconds(micros);
        auto tp = std::chrono::time_point<std::chrono::system_clock>(us);
        auto tt = std::chrono::system_clock::to_time_t(tp);
        auto tm = std::gmtime(&tt);
        return ZonedDatetime(tm->tm_year + 1900,
                             tm->tm_mon + 1,
                             tm->tm_mday,
                             tm->tm_hour,
                             tm->tm_min,
                             tm->tm_sec,
                             micros % 1000000);
    }

    Date date() const {
        return Date(datetime.year, datetime.month, datetime.day);
    }

    ZonedTime zonedTime() const {
        return ZonedTime(datetime.hour, datetime.minute, datetime.sec, datetime.microsec);
    }

    bool operator==(const ZonedDatetime& rhs) const {
        return datetime == rhs.datetime;
    }

    bool operator<(const ZonedDatetime& rhs) const {
        return datetime < rhs.datetime;
    }

    void clear() {
        datetime.clear();
    }

    void addDuration(const Duration& duration) {
        datetime.addDuration(duration);
    }

    void subDuration(const Duration& duration) {
        datetime.subDuration(duration);
    }

    std::string toString() const {
        return fmt::format("ZONED {}", datetime.toString());
    }

    // 'Z' representing UTC timezone
    folly::dynamic toJson() const {
        return toString() + "Z";
    }
};

static_assert(sizeof(ZonedDatetime) == 8, "ZonedDatetime should be 8 bytes");

inline std::ostream& operator<<(std::ostream& os, const ZonedDatetime& d) {
    os << d.toString();
    return os;
}

inline ZonedDatetime operator+(const ZonedDatetime& l, const Duration& r) {
    ZonedDatetime dt = l;
    dt.addDuration(r);
    return dt;
}

inline ZonedDatetime operator-(const ZonedDatetime& l, const Duration& r) {
    ZonedDatetime dt = l;
    dt.subDuration(r);
    return dt;
}


template <>
struct nrpc::BufferReaderWriter<nebula::ZonedDatetime> {
    static void write(folly::IOBuf* buf, const nebula::ZonedDatetime& dateTime) {
        BufferReaderWriter<int64_t>::write(buf, dateTime.datetime.qword);
    }

    static Status read(folly::IOBuf* buf, nebula::ZonedDatetime* dateTime) {
        return BufferReaderWriter<int64_t>::read(buf, &dateTime->datetime.qword);
    }

    static size_t encodedSize(const nebula::ZonedDatetime& dateTime) {
        return BufferReaderWriter<int64_t>::encodedSize(dateTime.datetime.qword);
    }
};

}  // namespace nebula

namespace std {

// Inject a customized hash function
template <>
struct hash<nebula::ZonedDatetime> {
    std::size_t operator()(const nebula::ZonedDatetime& h) const noexcept {
        return h.datetime.qword;
    }
};

}  // namespace std
