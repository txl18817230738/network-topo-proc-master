// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <folly/hash/Hash.h>

#include "folly/String.h"
#include "nebula/common/nrpc/Address.h"
#include "nebula/common/nrpc/CommonDefine.h"
#include "nebula/common/utils/Types.h"

namespace nebula {

enum class ServiceType : int8_t {
    UNKNOWN = 0,
    STORAGE = 1,
    GRAPH = 2,
    META = 3,
    ALL = 4,
};


// Only for deploy and manage.
struct HostInfo {
    std::string host;
    Port agentPort;
    int32_t cpuCores;
};


struct ServiceState {
    bool isReported = false;
    bool licenseValid = true;
    bool markRemoved = false;

    bool operator==(const ServiceState& right) const {
        return isReported == right.isReported && licenseValid == right.licenseValid &&
               markRemoved == right.markRemoved;
    }
};

/******************************************************
 *
 * HostAddress: hostname + hostport
 * Used to identify a host's serve address. Hostname can be ip or domain name.
 * Most of the time, we use ip address to identify a host when the host supply a service by
 * fixed port.
 * As for graph&storage role in cluster, we use a service (servicetype + id) to identify them.
 * And every service report it's serve address to meta server and sync to every node by
 * partmanager heartbeat.
 ******************************************************/
struct HostAddress {
    std::string host;
    Port port{};

    static HostAddress& nullAddr() {
        static HostAddress addr{"", 0};
        return addr;
    }

    HostAddress() = default;
    HostAddress(const std::string& h, Port p) : host(h), port(p) {}

    bool operator==(const HostAddress& rhs) const {
        return host == rhs.host && port == rhs.port;
    }

    bool operator!=(const HostAddress& rhs) const {
        return !(*this == rhs);
    }

    bool operator<(const HostAddress& rhs) const {
        if (host == rhs.host) {
            return port < rhs.port;
        }
        return host < rhs.host;
    }

    static StatusOr<HostAddress> fromString(folly::StringPiece str) {
        std::vector<folly::StringPiece> out;
        folly::split(":", str, out);
        if (out.size() != 2) {
            return V_STATUS(NETWORK_INVALID_HOST, str);
        }
        HostAddress addr;
        addr.host = out[0];
        try {
            addr.port = folly::to<decltype(addr.port)>(out[1]);
        } catch (const std::exception& ex) {
            return V_STATUS(NETWORK_INVALID_HOST, str);
        }
        return addr;
    }

    std::string toString() const {
        return folly::sformat("{}:{}", host, port);
    }
};

enum class AddressType : int8_t {
    SERVICE = 0,
    RAFT = 1,
};

using ServiceAddressMap = std::map<ServiceID, std::map<AddressType, HostAddress>>;

std::ostream& operator<<(std::ostream& os, const ServiceType& type);

inline std::ostream& operator<<(std::ostream& os, const HostAddress& addr) {
    return os << addr.toString();
}

SERIALIZE_EACH_MEMBER(nebula::HostAddress, host, port)

}  // namespace nebula


namespace std {

template <>
struct hash<nebula::HostAddress> {
    std::size_t operator()(const nebula::HostAddress& addr) const noexcept {
        int64_t h = folly::hash::fnv32_buf(addr.host.data(), addr.host.size());
        return folly::hash::hash_combine(h, addr.port);
    }
};

}  // namespace std
