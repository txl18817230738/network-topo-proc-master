// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <string>

namespace nebula {

// NullValue is a singleton class that represents a null value.
// It is used in the implementation of NullVector.
class NullValue final {
public:
    static const NullValue kNullValue;

    std::string toString() const {
        return "NULL";
    }

private:
    NullValue() = default;
};

inline std::ostream& operator<<(std::ostream& os, const NullValue& nVal) {
    return os << nVal.toString();
}

// TODO(Aiee) Double check the comparison logic
bool operator==(const NullValue& lhs, const NullValue& rhs);
bool operator!=(const NullValue& lhs, const NullValue& rhs);

}  // namespace nebula
namespace std {

// Inject a customized hash function
template <>
struct hash<nebula::NullValue> {
    std::size_t operator()(const nebula::NullValue&) const noexcept {
        return 0;
    }
};

}  // namespace std
