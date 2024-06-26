// Copyright (c) 2023 vesoft inc. All rights reserved.

#include <cstdint>

#pragma once

namespace nebula {

class IDGen final {
public:
    static uint32_t uniqueID_3B();
};

}  // namespace nebula
