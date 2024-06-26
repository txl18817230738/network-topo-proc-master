// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <sys/socket.h>

#include <arpa/inet.h>
#include <folly/hash/Hash.h>

#include "nebula/common/base/StatusOr.h"

namespace nebula::nrpc {

struct Address {
    // clang-format off
    uint16_t        family{AF_INET};
    uint16_t        port{0};
    union {
        uint32_t    ipv4;
        uint8_t     ipv6[16];
    };
    uint64_t        id{0};
    // clang-format on

    Address() {
        ::memset(ipv6, 0, 16);
    }

    Address(const Address &rhs) {
        ::memcpy(reinterpret_cast<void *>(this),
                 reinterpret_cast<const void *>(&rhs),
                 sizeof(*this));
    }

    ~Address() = default;

    bool operator==(const Address &rhs) const {
        if (id != rhs.id) {
            return false;
        }
        if (family != rhs.family || port != rhs.port) {
            return false;
        }
        return family == AF_INET ? ipv4 == rhs.ipv4 : memcmp(ipv6, rhs.ipv6, 16) == 0;
    }

    bool operator!=(const Address &rhs) const {
        return !operator==(rhs);
    }

    Address &operator=(const Address &rhs) {
        if (this != &rhs) {
            ::memcpy(reinterpret_cast<void *>(this),
                     reinterpret_cast<const void *>(&rhs),
                     sizeof(*this));
        }
        return *this;
    }

    static StatusOr<Address> from(std::string_view host, uint16_t port);

    static StatusOr<Address> from(const struct sockaddr *a);

    static StatusOr<Address> from(const struct sockaddr_storage *a);

    bool isAny() const;

    std::string str() const;
} __attribute__((packed));

static_assert(sizeof(Address) == 28UL);

}  // namespace nebula::nrpc

namespace std {

template <>
struct hash<nebula::nrpc::Address> {
    size_t operator()(const nebula::nrpc::Address &addr) const {
        return folly::hash::fnv64_buf(reinterpret_cast<const char *>(&addr), sizeof(addr));
    }
};

}  // namespace std
