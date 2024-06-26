// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <cstdint>

namespace nebula::computing {

class ComputingContext;

using std::size_t;

/**
 * @brief The BitSet class is used to store the state of whether each vertex is visited in the
 *  algorithm. The bit is set to 1 when the vertex is visited, otherwise it is 0.
 */
class BitSet final {
public:
    explicit BitSet(const ComputingContext *ctx, size_t size = 0u);

    // move
    BitSet(BitSet &&rhs) noexcept;
    BitSet &operator=(BitSet &&rhs) noexcept;

    // copy
    BitSet(const BitSet &rhs);
    BitSet &operator=(const BitSet &rhs);

    ~BitSet();

    void realloc(size_t size);

    size_t size() const {
        return size_;
    }

    size_t count() const;

    void set(size_t idx);
    bool get(size_t idx) const;

protected:
    void init();

    const ComputingContext *ctx_{nullptr};
    char *bits_{nullptr};
    size_t size_{0u};
};

}  // namespace nebula::computing
