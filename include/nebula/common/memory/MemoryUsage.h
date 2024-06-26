// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/base/Exception.h"
#include "nebula/common/memory/Memory.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"

namespace nebula {
namespace memory {

template <Layer L>
struct MemoryUsage {
    std::atomic<int64_t> amount{0};
    int64_t peak{0};
    // soft limit can be exceeded, a query exceeded more memory compare to soft limit is more
    // likely to be killed when system decide to kill queries to release memory
    int64_t softLimit;
    // hard limit can not be exceeded, a query exceed hard limit will be killed instantly
    int64_t hardLimit;

    folly::dynamic toJson() const {
        folly::dynamic r = folly::dynamic::object;
        r["used"] = amount.load();
        r["peek"] = peak;
        r["soft_limit"] = softLimit;
        r["hard_limit"] = hardLimit;
        return r;
    }

    void alloc(int64_t size) {
        int64_t newAmount = amount.fetch_add(size, std::memory_order_relaxed) + size;
        if (newAmount > hardLimit) {
            amount.fetch_sub(size, std::memory_order_relaxed);
            throw MemoryExceededException(
                    "Memory usage for {} exceeded hard limit {} when try to allocate {}",
                    LayerTraits<L>::name,
                    hardLimit,
                    size);
        }
        if (newAmount > peak) {
            peak = newAmount;
        }
    }

    void free(int64_t size) {
        amount.fetch_sub(size, std::memory_order_relaxed);
    }
};

/**
 * Special memory usage for operator for following reasons:
 *   1. each operator runs in single thread, no need to support multi-thread;
 *   2. memory may move between operator, alloc in one while freed by another,
 *      no reason to record peak.
 */
template <>
struct MemoryUsage<Operator> {
    int64_t amount{0};
    int64_t peak{0};
#ifndef NDEBUG
    int64_t allocated{0};
    int64_t freed{0};
    int64_t hist[8] = {0, 0, 0, 0, 0, 0, 0, 0};

    std::string histString() const {
        return fmt::format("{}|{}|{}|{}|{}|{}|{}|{}",
                           hist[0],
                           hist[1],
                           hist[2],
                           hist[3],
                           hist[4],
                           hist[5],
                           hist[6],
                           hist[7]);
    }
#endif

    folly::dynamic toJson() const {
        folly::dynamic r = folly::dynamic::object;
        r["amount"] = amount;
        r["peak"] = peak;
#ifndef NDEBUG
        r["allocated"] = allocated;
        r["freed"] = freed;
        r["hist"] = histString();
#endif
        return r;
    }

    void alloc(int64_t size) {
        amount += size;
        if (amount > peak) {
            peak = amount;
        }
#ifndef NDEBUG
        allocated += size;
        if (size <= 32)
            hist[0]++;
        else if (size <= 128)
            hist[1]++;
        else if (size <= 512)
            hist[2]++;
        else if (size <= 1024)
            hist[3]++;
        else if (size <= 4096)
            hist[4]++;
        else if (size <= 32768)
            hist[5]++;
        else if (size <= 65536)
            hist[6]++;
        else
            hist[7]++;
#endif
    }

    void free(int64_t size) {
        amount -= size;
#ifndef NDEBUG
        freed += size;
#endif
    }

    void add(const MemoryUsage<Operator>& other) {
        amount += other.amount;
        peak = peak > other.peak ? peak : other.peak;
#ifndef NDEBUG
        allocated += other.allocated;
        freed += other.freed;
        for (size_t i = 0; i < 8; ++i) {
            hist[i] += other.hist[i];
        }
#endif
    }
};


typedef MemoryUsage<Operator> OperatorMemoryUsage;

}  // namespace memory

namespace nrpc {

template <>
struct BufferReaderWriter<memory::OperatorMemoryUsage> {
    static void write(folly::IOBuf* buf, const memory::OperatorMemoryUsage& msg) {
        BufferReaderWriter<int64_t>::write(buf, msg.amount);
        BufferReaderWriter<int64_t>::write(buf, msg.peak);
#ifndef NDEBUG
        BufferReaderWriter<int64_t>::write(buf, msg.allocated);
        BufferReaderWriter<int64_t>::write(buf, msg.freed);
        for (size_t i = 0; i < 8; ++i) {
            BufferReaderWriter<int64_t>::write(buf, msg.hist[i]);
        }
#endif
    }

    static Status read(folly::IOBuf* buf, memory::OperatorMemoryUsage* msgPtr) {
        NG_RETURN_IF_ERROR(BufferReaderWriter<int64_t>::read(buf, &msgPtr->amount));
        NG_RETURN_IF_ERROR(BufferReaderWriter<int64_t>::read(buf, &msgPtr->peak));
#ifndef NDEBUG
        NG_RETURN_IF_ERROR(BufferReaderWriter<int64_t>::read(buf, &msgPtr->allocated));
        NG_RETURN_IF_ERROR(BufferReaderWriter<int64_t>::read(buf, &msgPtr->freed));
        for (size_t i = 0; i < 8; ++i) {
            NG_RETURN_IF_ERROR(BufferReaderWriter<int64_t>::read(buf, &msgPtr->hist[i]));
        }
#endif
        return Status::OK();
    }

    static size_t encodedSize(const memory::OperatorMemoryUsage& msg) {
        size_t sz = BufferReaderWriter<int64_t>::encodedSize(msg.amount);
        sz += BufferReaderWriter<int64_t>::encodedSize(msg.peak);
#ifndef NDEBUG
        sz += BufferReaderWriter<int64_t>::encodedSize(msg.allocated);
        sz += BufferReaderWriter<int64_t>::encodedSize(msg.freed);
        for (size_t i = 0; i < 8; ++i) {
            sz += BufferReaderWriter<int64_t>::encodedSize(msg.hist[i]);
        }
#endif
        return sz;
    }
};

}  // namespace nrpc

}  // namespace nebula
