// Copyright (c) 2022 vesoft inc. All rights reserved.
#pragma once

namespace nebula {

class NonCopyable {
protected:
    NonCopyable() {}
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
};

static_assert(sizeof(NonCopyable) == 1UL, "Unexpected sizeof(NonCopyable)!");

class NonMovable {
protected:
    NonMovable() {}
    NonMovable(NonMovable&&) = delete;
    NonMovable& operator=(NonMovable&&) = delete;
};

static_assert(sizeof(NonMovable) == 1UL, "Unexpected sizeof(NonMovable)!");

}  // namespace nebula
