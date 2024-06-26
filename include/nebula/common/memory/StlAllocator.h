// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <folly/Likely.h>

#include "nebula/common/memory/MemoryTracker.h"
namespace nebula {
namespace memory {

template <typename T, Layer L>
class BasicStlAllocator {
public:
    using value_type = T;
    using char_type = char;

    template <class U>
    struct rebind {
        using other = BasicStlAllocator<U, L>;
    };

    template <typename U>
    BasicStlAllocator(const BasicStlAllocator<U, L>&) noexcept {}

    BasicStlAllocator() noexcept {}

    T* allocate(size_t n) {
        if (LIKELY(currentTracker != nullptr)) {
            currentTracker->acquire(sizeof(T) * n);
        }
        return static_cast<T*>(nebula::memory::allocate(n * sizeof(T)));
    }

    void deallocate(T* p, size_t n) {
        if (LIKELY(currentTracker != nullptr)) {
            currentTracker->release(sizeof(T) * n);
        }
        return nebula::memory::dealloc(p);
    }

    template <typename T1>
    bool operator==(const BasicStlAllocator<T1, L>&) const {
        return std::is_same_v<T, T1>;
    }

    template <typename T1>
    bool operator!=(const BasicStlAllocator<T1, L>& rhs) const {
        return !(*this == rhs);
    }
};

template <typename T>
using StlAllocator = BasicStlAllocator<T, memory::Operator>;

}  // namespace memory
}  // namespace nebula
