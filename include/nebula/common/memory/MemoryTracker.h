// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/memory/MemoryUsage.h"

namespace nebula {
namespace memory {

constexpr static int64_t UNLIMITED = std::numeric_limits<int64_t>::max();

/**
 * A memory consumption tracker with templated layer. It throws an exception
 * if amount of consumed memory become greater than certain limit.
 */
template <Layer L>
class MemoryTracker {
public:
    typedef MemoryUsage<L> Usage;

    MemoryTracker(const std::string& name, int64_t limit)
            : MemoryTracker(nullptr, name, limit) {}

    MemoryTracker(MemoryTracker<LayerTraits<L>::parent>* parent,
                  const std::string& name,
                  int64_t limit)
            : parent_(parent), name_(name) {
        if (limit != UNLIMITED) {
            usage_.hardLimit = limit + LayerTraits<L>::maxReserveBytes;
        } else {
            usage_.hardLimit = limit;
        }
    }

    ~MemoryTracker() {
        if constexpr (LayerTraits<L>::hasParent) {
            CHECK(parent_ != nullptr);
            // For none leaf Trackers, what memory should return to parent:
            //   reservation:   should return to parent,
            //                  reservation is memory occupied by this tracker, but not acquired
            //                  by children;
            //   usage_.amount: shoud NOT return to parent,
            //                  amount is summed up memory already acquired by children, child
            //                  should return themselves;
            if (reservation_ != 0) {
                parent_->release(reservation_);
            }
        }
        /*
         * Amount of consumed memory should always equal zero on destruction.
         */
        if (usage_.amount != 0) {
            LOG(WARNING) << toJsonString() << " does not free all memory";
        }
        VLOG(3) << toPrettyJsonString();
    }

    /**
     * Create child of this tracker, a child should has the next layer defined in LayerTraits.
     */
    std::unique_ptr<MemoryTracker<LayerTraits<L>::child>> createChild(
            const std::string& name, int64_t limit = UNLIMITED) {
        if constexpr (LayerTraits<L>::hasChild) {
            return std::make_unique<MemoryTracker<LayerTraits<L>::child>>(this, name, limit);
        }
    }

    const std::string& name() {
        return name_;
    }

    /**
     * Inform size of memory allocation
     */
    void acquire(int64_t size) {
        usage_.alloc(size);

        if constexpr (LayerTraits<L>::hasParent) {
            int64_t willReserve = reservation_.load(std::memory_order_relaxed) - size;

            if (UNLIKELY(willReserve < 0)) {
                int64_t getFromParent = LayerTraits<L>::maxReserveBytes;
                while (willReserve + getFromParent <= 0) {
                    getFromParent += LayerTraits<L>::maxReserveBytes;
                }
                parent_->acquire(getFromParent);
                reservation_.fetch_add(getFromParent, std::memory_order_relaxed);
            }

            reservation_.fetch_sub(size, std::memory_order_relaxed);
        }
    }

    /**
     * Inform size of memory deallocation
     */
    void release(int64_t size) {
        if constexpr (LayerTraits<L>::hasParent) {
            int64_t willReserve =
                    reservation_.fetch_add(size, std::memory_order_relaxed) + size;
            int64_t releaseParent = 0;
            while (willReserve > LayerTraits<L>::maxReserveBytes) {
                releaseParent += LayerTraits<L>::maxReserveBytes;
                willReserve -= LayerTraits<L>::maxReserveBytes;
            }
            if (releaseParent > 0) {
                parent_->release(releaseParent);
                reservation_.fetch_sub(releaseParent, std::memory_order_relaxed);
            }
        }

        usage_.free(size);
    }

    const Usage& getUsage() const {
        return usage_;
    }

    void setHardLimit(int64_t limit) {
        usage_.hardLimit = limit;
    }

    void setSoftLimit(int64_t limit) {
        usage_.softLimit = limit;
    }

    folly::dynamic toJson() const {
        folly::dynamic r = folly::dynamic::object;
        r["name"] = name_;
        r["reserve"] = reservation_.load();
        r["usage"] = usage_.toJson();
        return r;
    }

    std::string toJsonString() const {
        return folly::toJson(toJson());
    }

    std::string toPrettyJsonString() const {
        return folly::toPrettyJson(toJson());
    }

private:
    MemoryTracker<LayerTraits<L>::parent>* parent_{nullptr};
    std::string name_;
    std::atomic<int64_t> reservation_{0};
    Usage usage_;
};

template <>
class MemoryTracker<Operator> {
public:
    typedef MemoryUsage<Operator> Usage;

    MemoryTracker(const std::string& name, int64_t limit)
            : MemoryTracker(nullptr, name, limit) {}

    MemoryTracker(MemoryTracker<LayerTraits<Operator>::parent>* parent,
                  const std::string& name,
                  int64_t limit)
            : parent_(parent), name_(name) {
        UNUSED(limit);
    }

    ~MemoryTracker() {
        // release any released reservation
        if (reservation_ != 0) {
            parent_->release(reservation_);
        }
        if (usage_.amount != 0) {
            parent_->release(usage_.amount);
        }
        VLOG(3) << toPrettyJsonString();
    }

    /**
     * Create child of this tracker, a child should has the next layer defined in LayerTraits.
     */
    std::unique_ptr<MemoryTracker<LayerTraits<Operator>::child>> createChild(
            const std::string& name, int64_t limit = UNLIMITED) {
        UNUSED(name);
        UNUSED(limit);
        return nullptr;
    }

    const std::string& name() {
        return name_;
    }

    /**
     * Inform size of memory allocation
     */
    void acquire(int64_t size) {
        int64_t willBe = reservation_ - size;
        if (UNLIKELY(willBe < 0)) {
            int64_t getFromParent = LayerTraits<Operator>::maxReserveBytes;
            while (willBe + getFromParent <= 0) {
                getFromParent += LayerTraits<Operator>::maxReserveBytes;
            }
            parent_->acquire(getFromParent);
            willBe += getFromParent;
        }
        reservation_ = willBe;
        usage_.alloc(size);
    }

    /**
     * Inform size of memory deallocation
     */
    void release(int64_t size) {
        reservation_ += size;
        while (reservation_ > LayerTraits<Operator>::maxReserveBytes) {
            parent_->release(LayerTraits<Operator>::maxReserveBytes);
            reservation_ -= LayerTraits<Operator>::maxReserveBytes;
        }
        usage_.free(size);
    }

    const Usage& getUsage() const {
        return usage_;
    }


    folly::dynamic toJson() const {
        folly::dynamic r = folly::dynamic::object;
        r["name"] = name_;
        r["usage"] = usage_.toJson();
        r["reserve"] = reservation_;
        return r;
    }

    std::string toJsonString() const {
        return folly::toJson(toJson());
    }

    std::string toPrettyJsonString() const {
        return folly::toPrettyJson(toJson());
    }

private:
    MemoryTracker<LayerTraits<Operator>::parent>* parent_{nullptr};
    std::string name_;
    int64_t reservation_{0};
    Usage usage_;
};

typedef memory::MemoryTracker<memory::Layer::Global> GlobalMemoryTracker;
typedef memory::MemoryTracker<memory::Layer::Query> QueryMemoryTracker;
typedef memory::MemoryTracker<memory::Layer::Operator> OperatorMemoryTracker;

extern thread_local MemoryTracker<Operator>* currentTracker;

/**
 * Get the global memory tracker
 */
GlobalMemoryTracker& GetGlobalMemoryTracker();

/**
 * A guard to set current thread's memory tracker
 */
struct MemoryTrackerGuard {
    MemoryTracker<Operator>* prev_;

    explicit MemoryTrackerGuard(MemoryTracker<Operator>* tracker) {
        prev_ = currentTracker;
        currentTracker = tracker;
    }

    ~MemoryTrackerGuard() {
        currentTracker = prev_;
    }
};

}  // namespace memory
}  // namespace nebula
