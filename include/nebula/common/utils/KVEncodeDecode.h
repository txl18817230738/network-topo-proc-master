// Copyright (c) 2024 vesoft inc. All rights reserved.

#include <cstdint>
#include <utility>

#include <folly/String.h>

#include "nebula/common/datatype/ZonedDatetime.h"
#include "nebula/common/service/Service.h"

#pragma once

namespace nebula {


class KVEncoder {
private:
    std::string resultStr_;

public:
    template <typename T,
              typename IntegralType =
                      std::enable_if_t<std::is_enum_v<T>, std::underlying_type_t<T>>>
    KVEncoder& encode(T t);

    template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
    KVEncoder& encode(T t);

    KVEncoder& encode(bool t);
    KVEncoder& encode(const std::string& str);
    KVEncoder& encode(const HostAddress& addr);
    KVEncoder& encode(const ZonedDatetime& dt);

    template <typename T>
    KVEncoder& encode(const std::vector<T>& vec);

    template <typename K, typename V>
    KVEncoder& encode(const std::pair<K, V>& pair);

    template <typename K, typename V>
    KVEncoder& encode(const std::map<K, V>& map);

    template <typename K, typename V>
    KVEncoder& encode(const std::unordered_map<K, V>& map);

    std::string result() {
        return resultStr_;
    }
};

class KVDecoder {
public:
    explicit KVDecoder(folly::StringPiece piece) : piece_(piece) {}

    template <typename T,
              typename = std::enable_if_t<std::is_enum_v<T>, std::underlying_type_t<T>>>
    KVDecoder& decode(T& t);

    template <typename T, typename std::enable_if_t<std::is_integral_v<T>, int> = 0>
    KVDecoder& decode(T& t);

    KVDecoder& decode(std::string& str);
    KVDecoder& decode(bool& t);
    KVDecoder& decode(HostAddress& addr);
    KVDecoder& decode(ZonedDatetime& dt);


    template <typename T>
    KVDecoder& decode(std::vector<T>& vec);

    template <typename K, typename V>
    KVDecoder& decode(std::pair<K, V>& pair);

    template <typename K, typename V>
    KVDecoder& decode(std::map<K, V>& map);

    template <typename K, typename V>
    KVDecoder& decode(std::unordered_map<K, V>& map);

private:
    folly::StringPiece piece_;
};


// handle enum type
template <typename T, typename IntegralType>
KVEncoder& KVEncoder::encode(T t) {
    auto big = folly::Endian::big(static_cast<IntegralType>(t));
    resultStr_ += {reinterpret_cast<const char*>(&big), sizeof(IntegralType)};
    return *this;
}

template <typename T, typename IntegralType>
KVDecoder& KVDecoder::decode(T& t) {
    IntegralType tt = *reinterpret_cast<const IntegralType*>(piece_.data());
    t = static_cast<T>(folly::Endian::big(tt));
    piece_.advance(sizeof(T));
    return *this;
}


// handle integral type
template <typename T, std::enable_if_t<std::is_integral_v<T>, int>>
KVEncoder& KVEncoder::encode(T t) {
    T big = folly::Endian::big(t);
    resultStr_ += {reinterpret_cast<const char*>(&big), sizeof(T)};
    return *this;
}

template <typename T, typename std::enable_if_t<std::is_integral_v<T>, int>>
KVDecoder& KVDecoder::decode(T& t) {
    T tt = *reinterpret_cast<const T*>(piece_.data());
    t = folly::Endian::big(tt);
    piece_.advance(sizeof(T));
    return *this;
}


// handle bool type
inline KVEncoder& KVEncoder::encode(bool t) {
    int8_t big = folly::Endian::big(static_cast<int8_t>(t));
    resultStr_ += {reinterpret_cast<const char*>(&big), sizeof(int8_t)};
    return *this;
}

inline KVDecoder& KVDecoder::decode(bool& t) {
    int8_t tt = *reinterpret_cast<const int8_t*>(piece_.data());
    t = static_cast<bool>(folly::Endian::big(tt));
    piece_.advance(sizeof(int8_t));
    return *this;
}


// handle string type
inline KVEncoder& KVEncoder::encode(const std::string& str) {
    encode(static_cast<uint32_t>(str.size()));
    resultStr_ += str;
    return *this;
}

inline KVDecoder& KVDecoder::decode(std::string& str) {
    uint32_t size;
    decode(size);
    str = piece_.subpiece(0, size).toString();
    piece_.advance(size);
    return *this;
}


// handle HostAddress type
inline KVEncoder& KVEncoder::encode(const HostAddress& addr) {
    encode(addr.host);
    encode(addr.port);
    return *this;
}


inline KVDecoder& KVDecoder::decode(HostAddress& addr) {
    decode(addr.host);
    decode(addr.port);
    return *this;
}


// handle ZonedDatetime type
inline KVEncoder& KVEncoder::encode(const ZonedDatetime& dt) {
    auto big = folly::Endian::big(dt.datetime.qword);
    resultStr_ += {reinterpret_cast<const char*>(&big), sizeof(dt.datetime.qword)};
    return *this;
}


inline KVDecoder& KVDecoder::decode(ZonedDatetime& dt) {
    int64_t qword = *reinterpret_cast<const int64_t*>(piece_.data());
    piece_.advance(sizeof(int64_t));
    dt.datetime.qword = folly::Endian::big(qword);
    return *this;
}


// handle vector type
template <typename T>
KVEncoder& KVEncoder::encode(const std::vector<T>& vec) {
    encode(static_cast<uint32_t>(vec.size()));
    for (auto& v : vec) {
        encode(v);
    }
    return *this;
}

template <typename T>
KVDecoder& KVDecoder::decode(std::vector<T>& vec) {
    uint32_t size;
    decode(size);
    vec.reserve(size);
    for (size_t i = 0; i < size; i++) {
        T t;
        decode(t);
        vec.emplace_back(std::move(t));
    }
    return *this;
}

// handle pair type
template <typename K, typename V>
KVEncoder& KVEncoder::encode(const std::pair<K, V>& pair) {
    encode(pair.first);
    encode(pair.second);

    return *this;
}

template <typename K, typename V>
KVDecoder& KVDecoder::decode(std::pair<K, V>& pair) {
    decode(pair.first);
    decode(pair.second);
    return *this;
}


// handle map type
template <typename K, typename V>
KVEncoder& KVEncoder::encode(const std::map<K, V>& map) {
    encode(static_cast<uint32_t>(map.size()));
    for (auto& [k, v] : map) {
        encode(k);
        encode(v);
    }
    return *this;
}


template <typename K, typename V>
KVDecoder& KVDecoder::decode(std::map<K, V>& map) {
    uint32_t size;
    decode(size);
    for (size_t i = 0; i < size; i++) {
        K k;
        V v;
        decode(k);
        decode(v);
        map.emplace(std::move(k), std::move(v));
    }
    return *this;
}


// handle unordered_map type
template <typename K, typename V>
KVEncoder& KVEncoder::encode(const std::unordered_map<K, V>& map) {
    encode(static_cast<uint32_t>(map.size()));
    for (auto& [k, v] : map) {
        encode(k);
        encode(v);
    }
    return *this;
}

template <typename K, typename V>
KVDecoder& KVDecoder::decode(std::unordered_map<K, V>& map) {
    uint32_t size;
    decode(size);
    for (size_t i = 0; i < size; i++) {
        K k;
        V v;
        decode(k);
        decode(v);
        map.emplace(std::move(k), std::move(v));
    }
    return *this;
}


}  // namespace nebula
