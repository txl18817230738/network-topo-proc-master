// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <folly/dynamic.h>

#include "nebula/common/memory/StlAllocator.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"
#include "nebula/common/time/Constants.h"
namespace nebula {

// TODO(Aiee) refactor this class according ISO 8601-1:2019, Date and time
//
// Duration equals to months + seconds + microseconds
// The base between months and days is not fixed, so we store years and months
// separately.
//
// The valid duration type size is 16 bytes: 8bytes for days and 8 bytes for secs, 4 bytes for
// ms, 4 bytes for months
struct Duration {
    using allocator_type = nebula::memory::StlAllocator<std::byte>;

    // day + hours + minutes + seconds + microseconds
    int64_t seconds{0};
    int32_t microseconds{0};
    // years + months
    int32_t months{0};

    Duration() = default;
    Duration(int32_t m, int64_t s, int32_t us) : seconds(s), microseconds(us), months(m) {}

    int64_t years() const {
        return months / 12;
    }

    int64_t monthsInYear() const {
        return months % 12;
    }

    int64_t days() const {
        return seconds / time::kSecondsOfDay;
    }

    int64_t hours() const {
        return seconds % time::kSecondsOfDay / time::kSecondsOfHour;
    }

    int64_t minutes() const {
        return seconds % time::kSecondsOfHour / time::kSecondsOfMinute;
    }

    int64_t secondsInMinute() const {
        return seconds % time::kSecondsOfMinute;
    }

    int64_t microsecondsInSecond() const {
        return microseconds;
    }

    Duration operator-() const {
        return {-months, -seconds, -microseconds};
    }

    Duration operator+(const Duration& rhs) const {
        return {months + rhs.months, seconds + rhs.seconds, microseconds + rhs.microseconds};
    }

    Duration operator-(const Duration& rhs) const {
        return {months - rhs.months, seconds - rhs.seconds, microseconds - rhs.microseconds};
    }

    Duration& addYears(int32_t y) {
        months += y * 12;
        return *this;
    }

    Duration& addQuarters(int32_t q) {
        months += q * 3;
        return *this;
    }

    Duration& addMonths(int32_t m) {
        months += m;
        return *this;
    }

    Duration& addWeeks(int32_t w) {
        seconds += (w * 7 * time::kSecondsOfDay);
        return *this;
    }

    Duration& addDays(int64_t d) {
        seconds += d * time::kSecondsOfDay;
        return *this;
    }

    Duration& addHours(int64_t h) {
        seconds += h * time::kSecondsOfHour;
        return *this;
    }

    Duration& addMinutes(int64_t minutes) {
        seconds += minutes * time::kSecondsOfMinute;
        return *this;
    }

    Duration& addSeconds(int64_t s) {
        seconds += s;
        return *this;
    }

    Duration& addMilliseconds(int64_t ms) {
        seconds += ms / 1000;
        microseconds += ((ms % 1000) * 1000);
        return *this;
    }

    Duration& addMicroseconds(int32_t us) {
        microseconds += us;
        return *this;
    }

    // can't compare
    bool operator<(const Duration& rhs) const {
        return months < rhs.months || (months == rhs.months && seconds < rhs.seconds) ||
               (months == rhs.months && seconds == rhs.seconds &&
                microseconds < rhs.microseconds);
    }

    bool operator==(const Duration& rhs) const {
        return months == rhs.months && seconds == rhs.seconds &&
               microseconds == rhs.microseconds;
    }

    void addDuration(const Duration& duration);
    void subDuration(const Duration& duration);

    std::string toString() const {
        return folly::sformat("P{}MT{}.{:0>6}000S",
                              months,
                              seconds + microseconds / 1000000,
                              microseconds % 1000000);
    }

    folly::dynamic toJson() const {
        return toString();
    }

private:
    friend struct nrpc::BufferReaderWriter<Duration>;
};

static_assert(sizeof(Duration) == 16, "Duration should be 16 bytes");

inline std::ostream& operator<<(std::ostream& os, const Duration& d) {
    os << d.toString();
    return os;
}

template <>
struct nrpc::BufferReaderWriter<nebula::Duration> {
    static void write(folly::IOBuf* buf, const nebula::Duration& duration) {
        BufferReaderWriter<int64_t>::write(buf, duration.seconds);
        BufferReaderWriter<int32_t>::write(buf, duration.microseconds);
        BufferReaderWriter<int32_t>::write(buf, duration.months);
    }

    static Status read(folly::IOBuf* buf, nebula::Duration* duration) {
        NG_RETURN_IF_ERROR(BufferReaderWriter<int64_t>::read(buf, &duration->seconds));
        NG_RETURN_IF_ERROR(BufferReaderWriter<int32_t>::read(buf, &duration->microseconds));
        return BufferReaderWriter<int32_t>::read(buf, &duration->months);
    }

    static size_t encodedSize(const nebula::Duration& duration) {
        return BufferReaderWriter<int64_t>::encodedSize(duration.seconds) +
               BufferReaderWriter<int32_t>::encodedSize(duration.microseconds) +
               BufferReaderWriter<int32_t>::encodedSize(duration.months);
    }
};

}  // namespace nebula

namespace std {

// Inject a customized hash function
template <>
struct hash<nebula::Duration> {
    std::size_t operator()(const nebula::Duration& d) const noexcept {
        return folly::hash::hash_combine(d.months, d.seconds, d.microseconds);
    }
};

}  // namespace std
