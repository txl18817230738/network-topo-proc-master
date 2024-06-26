// Copyright (c) 2023 vesoft inc. All rights reserved.
#pragma once

#include <cstdint>
#include <string>

#include <boost/uuid/uuid.hpp>

namespace nebula {

struct IDGen final {
    static uint32_t uniqueID_3B();
};

struct TokenGen final {
    static std::string genSalt(int min, int max);
};


struct UUIDV4 final {
    static std::string getIdStr();
};

}  // namespace nebula
