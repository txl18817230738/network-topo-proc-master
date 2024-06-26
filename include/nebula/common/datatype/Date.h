// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <cstdint>
#include <ostream>

#include "nebula/common/datatype/Duration.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"
namespace nebula {

// https://en.wikipedia.org/wiki/Leap_year#Leap_day
static inline bool isLeapYear(int16_t year) {
    if (year % 4 != 0) {
        return false;
    } else if (year % 100 != 0) {
        return true;
    } else if (year % 400 != 0) {
        return false;
    } else {
        return true;
    }
}

// In nebula only store UTC time, and the interpretation of time value based on
// the timezone configuration in current system.

const int64_t kDaysSoFar[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
const int64_t kLeapDaysSoFar[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366};

// An instant capturing the date, but not the time, nor the time zone.
struct Date {
    int16_t year;  // Any integer
    int8_t month;  // 1 - 12
    int8_t day;    // 1 - 31

    Date() : year{0}, month{1}, day{1} {}
    Date(int16_t y, int8_t m, int8_t d) : year{y}, month{m}, day{d} {}
    // Tak the number of days since -32768/1/1, and convert to the real date
    explicit Date(uint64_t days);

    void clear() {
        year = 0;
        month = 1;
        day = 1;
    }

    void reset(int16_t y, int8_t m, int8_t d) {
        year = y;
        month = m;
        day = d;
    }

    Date date() const {
        return {year, month, day};
    }

    bool operator==(const Date& rhs) const {
        return year == rhs.year && month == rhs.month && day == rhs.day;
    }

    bool operator<(const Date& rhs) const {
        if (!(year == rhs.year)) {
            return year < rhs.year;
        }
        if (!(month == rhs.month)) {
            return month < rhs.month;
        }
        if (!(day == rhs.day)) {
            return day < rhs.day;
        }
        return false;
    }

    Date operator+(int64_t days) const;
    Date operator-(int64_t days) const;

    void addDuration(const Duration& duration);
    void subDuration(const Duration& duration);

    std::string toString() const;
    folly::dynamic toJson() const {
        return toString();
    }

    // Return the number of days since -32768/1/1
    int64_t toInt() const;
    // Convert the number of days since -32768/1/1 to the real date
    void fromInt(int64_t days);

private:
    friend struct nrpc::BufferReaderWriter<Date>;
};

static_assert(sizeof(Date) == 4, "Date should be 4 bytes");

inline Date operator+(const Date& l, const Duration& r) {
    Date d = l;
    d.addDuration(r);
    return d;
}

inline Date operator-(const Date& l, const Duration& r) {
    Date d = l;
    d.subDuration(r);
    return d;
}

inline std::ostream& operator<<(std::ostream& os, const Date& d) {
    os << d.toString();
    return os;
}

template <>
struct nrpc::BufferReaderWriter<nebula::Date> {
    static void write(folly::IOBuf* buf, const nebula::Date& date) {
        BufferReaderWriter<int16_t>::write(buf, date.year);
        BufferReaderWriter<int8_t>::write(buf, date.month);
        BufferReaderWriter<int8_t>::write(buf, date.day);
    }

    static Status read(folly::IOBuf* buf, nebula::Date* date) {
        NG_RETURN_IF_ERROR(BufferReaderWriter<int16_t>::read(buf, &date->year));
        NG_RETURN_IF_ERROR(BufferReaderWriter<int8_t>::read(buf, &date->month));
        return BufferReaderWriter<int8_t>::read(buf, &date->day);
    }

    static size_t encodedSize(const nebula::Date& date) {
        return BufferReaderWriter<int16_t>::encodedSize(date.year) +
               BufferReaderWriter<int8_t>::encodedSize(date.month) +
               BufferReaderWriter<int8_t>::encodedSize(date.day);
    }
};

}  // namespace nebula

namespace std {

// Inject a customized hash function
template <>
struct hash<nebula::Date> {
    std::size_t operator()(const nebula::Date& h) const noexcept {
        return folly::hash::hash_combine(h.year, h.month, h.day);
    }
};

}  // namespace std
