// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

namespace nebula {

// TODO(shylock) help compile, remove when expr2 ready
struct Nothing {
public:
    // Nothing never instance value
    // explicit Nothing(...) = delete;

    std::string toString() const {
        return "";
    }
};

}  // namespace nebula

namespace std {

// Inject a customized hash function
template <>
struct hash<nebula::Nothing> {
    std::size_t operator()(const nebula::Nothing&) const noexcept {
        return 0;
    }
};

}  // namespace std
