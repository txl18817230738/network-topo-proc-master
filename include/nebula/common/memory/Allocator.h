// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/memory/MemoryTracker.h"

namespace nebula {
namespace memory {

template <Layer L>
class Allocator {
public:
    static Allocator createFrom(MemoryTracker<L>* FOLLY_NONNULL tracker) {
        if (tracker) {
            return Allocator(tracker->name(), tracker);
        } else {
            return Allocator("", tracker);
        }
    }

    Allocator(const std::string& name, MemoryTracker<L>* FOLLY_NONNULL tracker)
            : name_(name), tracker_(tracker) {}

    void* FOLLY_NONNULL allocate(size_t size, size_t alignment = 0) {
        if (tracker_) {
            tracker_->acquire(size);
        }
        if (alignment <= 8) {
            return nebula::memory::allocate(size);
        } else {
            return nebula::memory::alignedAlloc(alignment, size);
        }
    }

    void* FOLLY_NONNULL reallocate(void* FOLLY_NONNULL oldPtr,
                                   size_t oldSize,
                                   size_t newSize,
                                   size_t alignment = 0) {
        void* newPtr = nullptr;
        if (tracker_) {
            tracker_->release(oldSize);
            tracker_->acquire(newSize);
        }
        if (alignment <= 8) {
            newPtr = nebula::memory::allocate(newSize);
        } else {
            newPtr = nebula::memory::alignedAlloc(alignment, newSize);
        }
        if (newPtr != nullptr) {
            std::memcpy(newPtr, oldPtr, oldSize);
            nebula::memory::dealloc(oldPtr);
        }
        return newPtr;
    }

    void deallocate(void* FOLLY_NONNULL ptr, size_t size) {
        if (tracker_) {
            tracker_->release(size);
        }
        nebula::memory::dealloc(ptr);
    }

private:
    std::string name_;
    MemoryTracker<L>* FOLLY_NONNULL tracker_;
};

}  // namespace memory
}  // namespace nebula
