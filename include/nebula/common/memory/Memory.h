// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#if ENABLE_JEMALLOC
#include <jemalloc/jemalloc.h>
#endif
#include <new>

#include "nebula/common/base/Base.h"

namespace nebula {
namespace memory {

constexpr int64_t KiB = 1024;
constexpr int64_t MiB = 1024 * KiB;
constexpr int64_t GiB = 1024 * MiB;

enum Layer {
    Global,
    Query,
    Operator,
    Invalid,
};

/**
 * Traits for memory layers, enable compile time polymorphism for layered memory tracker.
 */
template <Layer L>
struct LayerTraits {
    static constexpr Layer parent = Layer::Invalid;
    static constexpr Layer child = Layer::Invalid;
    static constexpr int maxReserveBytes = 0;
    static constexpr bool hasParent = false;
    static constexpr bool hasChild = false;
    static constexpr bool hasLimit = false;
    static constexpr const char* name = "Invalid";
};

template <>
struct LayerTraits<Global> {
    static constexpr Layer parent = Layer::Invalid;
    static constexpr Layer child = Layer::Query;
    static constexpr int maxReserveBytes = 0;
    static constexpr bool hasParent = false;
    static constexpr bool hasChild = true;
    static constexpr bool hasLimit = true;
    static constexpr const char* name = "Global";
};

template <>
struct LayerTraits<Query> {
    static constexpr Layer parent = Layer::Global;
    static constexpr Layer child = Layer::Operator;
    static constexpr int maxReserveBytes = 64 * KiB;
    static constexpr bool hasParent = true;
    static constexpr bool hasChild = true;
    static constexpr bool hasLimit = true;
    static constexpr const char* name = "Query";
};

template <>
struct LayerTraits<Operator> {
    static constexpr Layer parent = Layer::Query;
    static constexpr Layer child = Layer::Invalid;
    static constexpr int maxReserveBytes = 4 * KiB;
    static constexpr bool hasParent = true;
    static constexpr bool hasChild = false;
    static constexpr bool hasLimit = false;
    static constexpr const char* name = "Operator";
};

inline ALWAYS_INLINE void* allocate(std::size_t size) {
    void* ptr = malloc(size);
    if (LIKELY(ptr != nullptr)) return ptr;
    throw std::bad_alloc{};
}

inline ALWAYS_INLINE void* alignedAlloc(std::size_t size, std::size_t align) {
    void* ptr = aligned_alloc(align, size);
    if (LIKELY(ptr != nullptr)) return ptr;
    throw std::bad_alloc{};
}

inline ALWAYS_INLINE void dealloc(void* ptr) noexcept {
    free(ptr);
}

}  // namespace memory
}  // namespace nebula
