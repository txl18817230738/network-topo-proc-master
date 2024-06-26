// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <deque>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>

#include <folly/concurrency/ConcurrentHashMap.h>
#include <folly/io/IOBuf.h>

#include "nebula/common/base/Base.h"
#include "nebula/common/base/Status.h"

namespace nebula::nrpc {

#define DEBUG_MARK_THIS_RW_IN_MEMORY(x, y)     \
    DLOG(INFO) << "Manipulate object (" << (x) \
               << ") at address: " << reinterpret_cast<uint64_t>(y);

/**
 * Specialize this template class to customize your (de)serialization methods
 */
template <typename T, typename Enable = void>
struct BufferReaderWriter {
    /**
     * Serialize an object to tail of the target buffer.
     */
    static void write(folly::IOBuf *buf, const T &obj) = delete;

    /**
     * Deserialize an object from the head of the target buffer
     */
    static Status read(folly::IOBuf *buf, T *obj) = delete;

    /**
     * Calculate the size of an object when serialized
     */
    static size_t encodedSize(const T &obj) = delete;
};


/**
 * ValueTypeKind trait to tell if the encoded size is constant when serialized.
 *
 * All scalar types are constant sized, while all user-defined types are not by default.
 *
 * This type trait is for detecting whether we could speed up (de)serialization (via `memcpy')
 * and calculation of encoded size.
 *
 */
template <typename T, typename enable = void>
struct IsConstantEncodedSize : public std::false_type {
    static constexpr size_t size = ~0UL;
};

/**
 * All scalar types are constant encoded size. AND the encoded size is the same to natual size,
 * thus they are trivially encoding (via `memcpy').
 *
 * NOTE For those types that are constant encoded size, that cannot be speeded up for
 * serialization (structs with padding, and for space saving purpose), one could specialize
 * `IsConstantEncodedSize' with a different `size' with its natual, and provide your own
 * methods.
 */
template <typename T>
struct IsConstantEncodedSize<T, std::enable_if_t<std::is_scalar_v<T>>> : public std::true_type {
    static constexpr size_t size = sizeof(T);
};

template <typename T>
struct IsTriviallyEncoding {
    static constexpr bool value =
            IsConstantEncodedSize<T>::value && IsConstantEncodedSize<T>::size == sizeof(T);
};


template <typename T>
struct BufferReaderWriter<T, std::enable_if_t<std::is_scalar_v<T> && !std::is_pointer_v<T>>> {
    static void write(folly::IOBuf *buf, T obj) {
        buf->reserve(0, sizeof(T));
        *reinterpret_cast<T *>(buf->writableTail()) = obj;
        buf->append(sizeof(T));
    }

    static Status read(folly::IOBuf *buf, T *obj) {
        if (buf->length() < sizeof(T)) {
            return Status(ErrorCode::RPC_FAILED,
                          "Data not enough, %lu required, %lu left",
                          sizeof(T),
                          buf->length());
        }
        *obj = *reinterpret_cast<const T *>(buf->data());
        buf->trimStart(sizeof(T));
        return Status::OK();
    }

    static size_t encodedSize(const T &) {
        return sizeof(T);
    }
};


template <typename T>
struct BufferReaderWriter<
        T,
        std::enable_if_t<!std::is_scalar_v<T> && IsTriviallyEncoding<T>::value>> {
    static void write(folly::IOBuf *buf, const T &obj) {
        buf->reserve(0, sizeof(T));
        ::memcpy((void *)buf->writableTail(), (void *)&obj, sizeof(T));  // NOLINT
        buf->append(sizeof(T));
    }

    static Status read(folly::IOBuf *buf, T *obj) {
        if (buf->length() < sizeof(T)) {
            return Status(ErrorCode::RPC_DESERIALIZE_ERROR,
                          "Data not enough, %lu required, %lu left",
                          sizeof(T),
                          buf->length());
        }

        ::memcpy((void *)obj, (const void *)buf->data(), sizeof(T));  // NOLINT
        buf->trimStart(sizeof(T));
        return Status::OK();
    }

    static size_t encodedSize(const T &) {
        return sizeof(T);
    }
};

template <typename T>
struct BufferReaderWriter<T, std::enable_if_t<std::is_empty<T>::value>> {
    static void write(folly::IOBuf *buf, const T &obj) {
        UNUSED(buf);
        UNUSED(obj);
        // do nothing
    }

    static Status read(folly::IOBuf *buf, T *obj) {
        UNUSED(buf);
        UNUSED(obj);
        return Status::OK();
    }

    static size_t encodedSize(const T &) {
        return 0;
    }
};


template <typename T>
struct BufferReaderWriter<T *, void> {
    using pointer_type = T *;
    static void write(folly::IOBuf *buf, const pointer_type &obj) {
        BufferReaderWriter<T>::write(buf, *obj);
    }

    static Status read(folly::IOBuf *buf, pointer_type *obj) {
        // *obj = std::make_shared<T>();
        return BufferReaderWriter<T>::read(buf, *obj);
    }

    static size_t encodedSize(const pointer_type &obj) {
        return BufferReaderWriter<T>::encodedSize(*obj);
    }

    static const bool enabled = true;
};


template <typename T>
struct BufferReaderWriter<std::shared_ptr<T>, std::enable_if_t<std::is_final_v<T>>> {
    using SharedPtr = std::shared_ptr<T>;
    static void write(folly::IOBuf *buf, const SharedPtr &obj) {
        BufferReaderWriter<bool>::write(buf, obj != nullptr);
        if (obj) {
            BufferReaderWriter<T>::write(buf, *obj);
        }
    }

    static Status read(folly::IOBuf *buf, SharedPtr *obj) {
        *obj = std::make_shared<T>();
        bool b = false;
        NG_RETURN_IF_ERROR(BufferReaderWriter<bool>::read(buf, &b));
        if (b) {
            return BufferReaderWriter<T>::read(buf, obj->get());
        }

        return Status::OK();
    }

    static Status read(folly::IOBuf *buf, SharedPtr *obj, bool flag) {
        *obj = std::make_shared<T>(flag);
        bool b = false;
        NG_RETURN_IF_ERROR(BufferReaderWriter<bool>::read(buf, &b));
        if (b) {
            return BufferReaderWriter<T>::read(buf, obj->get());
        }

        return Status::OK();
    }

    static size_t encodedSize(const SharedPtr &obj) {
        size_t sz = sizeof(bool);
        if (obj) {
            sz += BufferReaderWriter<T>::encodedSize(*obj);
        }
        return sz;
    }
};


template <typename T, typename Dp>
struct BufferReaderWriter<std::unique_ptr<T, Dp>, std::enable_if_t<std::is_final_v<T>>> {
    using UniquePtr = std::unique_ptr<T, Dp>;
    static void write(folly::IOBuf *buf, const UniquePtr &obj) {
        BufferReaderWriter<bool>::write(buf, obj != nullptr);
        if (obj) {
            BufferReaderWriter<T>::write(buf, *obj);
        }
    }

    static Status read(folly::IOBuf *buf, UniquePtr *obj) {
        *obj = std::make_unique<T>();
        bool b = false;
        NG_RETURN_IF_ERROR(BufferReaderWriter<bool>::read(buf, &b));
        if (b) {
            return BufferReaderWriter<T>::read(buf, obj->get());
        }

        return Status::OK();
    }

    static size_t encodedSize(const UniquePtr &obj) {
        size_t sz = sizeof(bool);
        if (obj) {
            sz += BufferReaderWriter<T>::encodedSize(*obj);
        }
        return sz;
    }
};


// NOTE: T should not be a base class, and it must have a default constructor
template <typename T>
struct BufferReaderWriter<std::optional<T>> {
    using Optional = std::optional<T>;
    static void write(folly::IOBuf *buf, const Optional &obj) {
        BufferReaderWriter<bool>::write(buf, obj.has_value());
        if (obj.has_value()) {
            BufferReaderWriter<T>::write(buf, obj.value());
        }
    }

    static Status read(folly::IOBuf *buf, Optional *obj) {
        bool b = false;
        NG_RETURN_IF_ERROR(BufferReaderWriter<bool>::read(buf, &b));
        if (b) {
            *obj = std::make_optional<T>();
            return BufferReaderWriter<T>::read(buf, &obj->value());
        } else {
            obj->reset();
        }

        return Status::OK();
    }

    static size_t encodedSize(const Optional &obj) {
        size_t sz = sizeof(bool);
        if (obj) {
            sz += BufferReaderWriter<T>::encodedSize(*obj);
        }
        return sz;
    }
};

template <typename T, typename Alloc>
struct BufferReaderWriter<std::basic_string<T, std::char_traits<T>, Alloc>, void> {
    using String = std::basic_string<T, std::char_traits<T>, Alloc>;
    static void write(folly::IOBuf *buf, const String &str) {
        write(buf, str.data(), str.length());
    }

    static void write(folly::IOBuf *buf, folly::StringPiece str) {
        write(buf, str.data(), str.size());
    }

    static void write(folly::IOBuf *buf, const char *str, size_t len) {
        buf->reserve(0, len + 4);
        *reinterpret_cast<uint32_t *>(buf->writableTail()) = len;
        buf->append(4);
        ::memcpy((void *)buf->writableTail(), str, len);  // NOLINT
        buf->append(len);
    }

    static Status read(folly::IOBuf *buf, String *str) {
        if (buf->length() < 4) {
            return Status(ErrorCode::RPC_DESERIALIZE_ERROR,
                          "Data not enough, 4B required, %lu left",
                          buf->length());
        }

        auto len = *reinterpret_cast<const uint32_t *>(buf->data());
        buf->trimStart(4);

        if (buf->length() < len) {
            return Status(ErrorCode::RPC_DESERIALIZE_ERROR,
                          "Data not enough, %u required, %lu left",
                          len,
                          buf->length());
        }

        str->resize(len);
        ::memcpy((void *)&(*str)[0], (void *)buf->data(), len);  // NOLINT
        buf->trimStart(len);

        return Status::OK();
    }

    static size_t encodedSize(const String &str) {
        return str.length() + 4;
    }
};


template <typename T1, typename T2>
struct BufferReaderWriter<std::pair<T1, T2>, void> {
    using Pair = std::pair<T1, T2>;
    static void write(folly::IOBuf *buf, const Pair &pair) {
        auto len = encodedSize(pair);
        buf->reserve(0, len);

        BufferReaderWriter<std::remove_cv_t<T1>>::write(buf, pair.first);
        BufferReaderWriter<std::remove_cv_t<T2>>::write(buf, pair.second);
    }

    static Status read(folly::IOBuf *buf, Pair *pair) {
        NG_RETURN_IF_ERROR(BufferReaderWriter<T1>::read(buf, &pair->first));
        return BufferReaderWriter<T2>::read(buf, &pair->second);
    }

    static size_t encodedSize(const Pair &pair) {
        return BufferReaderWriter<std::remove_cv_t<T1>>::encodedSize(pair.first) +
               BufferReaderWriter<std::remove_cv_t<T2>>::encodedSize(pair.second);
    }
};

template <typename... Types>
struct BufferReaderWriter<std::tuple<Types...>, void> {
    using Tuple = std::tuple<Types...>;
    template <size_t I>
    using ElementType = std::remove_cv_t<std::tuple_element_t<I, Tuple>>;

    template <size_t... Is>
    static void writeImpl(folly::IOBuf *buf, const Tuple &tuple, std::index_sequence<Is...>) {
        (BufferReaderWriter<ElementType<Is>>::write(buf, std::get<Is>(tuple)), ...);
    }

    static void write(folly::IOBuf *buf, const Tuple &tuple) {
        auto len = encodedSize(tuple);
        buf->reserve(0, len);
        writeImpl(buf, tuple, std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }

    template <size_t... Is>
    static Status readImpl(folly::IOBuf *buf, Tuple &tuple, std::index_sequence<Is...>) {
        Status status;
        ((status = BufferReaderWriter<ElementType<Is>>::read(buf, &std::get<Is>(tuple)),
          status.ok()) &&
         ...);
        return status;
    }

    static Status read(folly::IOBuf *buf, Tuple *tuple) {
        return readImpl(buf, *tuple, std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }

    template <size_t... Is>
    static size_t encodedSizeImpl(const Tuple &tuple, std::index_sequence<Is...>) {
        return (BufferReaderWriter<ElementType<Is>>::encodedSize(std::get<Is>(tuple)) + ...);
    }

    static size_t encodedSize(const Tuple &tuple) {
        return encodedSizeImpl(tuple, std::make_index_sequence<std::tuple_size_v<Tuple>>());
    }
};

template <typename T, typename Alloc>
struct BufferReaderWriter<std::vector<T, Alloc>, void> {
    using Vector = std::vector<T, Alloc>;
    static void write(folly::IOBuf *buf, const Vector &vec) {
        auto len = encodedSize(vec);
        buf->reserve(0, len);
        *reinterpret_cast<uint32_t *>(buf->writableTail()) = vec.size();
        buf->append(4);

        if constexpr (IsTriviallyEncoding<T>::value) {
            ::memcpy((void *)buf->writableTail(), (void *)&vec[0], len - 4);  // NOLINT
            buf->append(len - 4);
        } else {
            for (auto &v : vec) {
                BufferReaderWriter<std::remove_cv_t<T>>::write(buf, v);
            }
        }
    }

    static Status read(folly::IOBuf *buf, Vector *vec) {
        if (buf->length() < 4) {
            return Status(ErrorCode::RPC_DESERIALIZE_ERROR,
                          "Data not enough, 4B required, %lu left",
                          buf->length());
        }

        auto n = *reinterpret_cast<const uint32_t *>(buf->data());
        buf->trimStart(4);
        vec->clear();
        vec->resize(n);

        if constexpr (IsTriviallyEncoding<T>::value) {
            if (buf->length() < n * sizeof(T)) {
                return Status(ErrorCode::RPC_DESERIALIZE_ERROR,
                              "Data not enough, %lu required, %lu left",
                              n * sizeof(T),
                              buf->length());
            }
            ::memcpy((void *)&(*vec)[0], (void *)buf->data(), n * sizeof(T));  // NOLINT
            buf->trimStart(n * sizeof(T));
        } else {
            for (auto i = 0u; i < n; i++) {
                auto status = BufferReaderWriter<T>::read(buf, &(*vec)[i]);
                if (!status.ok()) {
                    return status;
                }
            }
        }

        return Status::OK();
    }

    static size_t encodedSize(const Vector &vec) {
        if constexpr (IsConstantEncodedSize<T>::value) {
            return 4 + vec.size() * IsConstantEncodedSize<T>::size;
        } else {
            size_t len = 4;
            for (auto &v : vec) {
                len += BufferReaderWriter<std::remove_cv_t<T>>::encodedSize(v);
            }
            return len;
        }
    }
};


template <typename T, typename Alloc>
struct BufferReaderWriter<std::deque<T, Alloc>, void> {
    using Deque = std::deque<T, Alloc>;
    static void write(folly::IOBuf *buf, const Deque &deq) {
        auto len = encodedSize(deq);
        buf->reserve(0, len);
        *reinterpret_cast<uint32_t *>(buf->writableTail()) = deq.size();
        buf->append(4);

        for (auto &v : deq) {
            BufferReaderWriter<std::remove_cv_t<T>>::write(buf, v);
        }
    }

    static Status read(folly::IOBuf *buf, Deque *deq) {
        if (buf->length() < 4) {
            return Status(ErrorCode::RPC_DESERIALIZE_ERROR,
                          "Data not enough, 4B required, %lu left",
                          buf->length());
        }

        auto n = *reinterpret_cast<const uint32_t *>(buf->data());
        buf->trimStart(4);
        deq->clear();
        deq->resize(n);

        for (auto i = 0u; i < n; i++) {
            NG_RETURN_IF_ERROR(BufferReaderWriter<T>::read(buf, &(*deq)[i]));
        }

        return Status::OK();
    }

    static size_t encodedSize(const Deque &deq) {
        if constexpr (IsConstantEncodedSize<T>::value) {
            return 4 + deq.size() * IsConstantEncodedSize<T>::size;
        } else {
            size_t len = 4;
            for (auto &v : deq) {
                len += BufferReaderWriter<std::remove_cv_t<T>>::encodedSize(v);
            }
            return len;
        }
    }
};


template <typename Key, typename Value, typename Hash, typename Equal, typename Alloc>
struct BufferReaderWriter<std::unordered_map<Key, Value, Hash, Equal, Alloc>, void> {
    using Record = std::unordered_map<Key, Value, Hash, Equal, Alloc>;
    static void write(folly::IOBuf *buf, const Record &map) {
        auto len = encodedSize(map);
        buf->reserve(0, len);
        *reinterpret_cast<uint32_t *>(buf->writableTail()) = map.size();
        buf->append(4);

        for (auto &pair : map) {
            BufferReaderWriter<Key>::write(buf, pair.first);
            BufferReaderWriter<Value>::write(buf, pair.second);
        }
    }

    static Status read(folly::IOBuf *buf, Record *map) {
        if (buf->length() < 4) {
            return Status(ErrorCode::RPC_DESERIALIZE_ERROR,
                          "Data not enough, 4B required, %lu left",
                          buf->length());
        }

        auto n = *reinterpret_cast<const uint32_t *>(buf->data());
        buf->trimStart(4);
        map->clear();
        map->reserve(n);

        for (auto i = 0u; i < n; i++) {
            Key key;
            Value value;
            if constexpr (std::uses_allocator_v<Key, Alloc>) {
                ::new (&key) Key();
            }
            if constexpr (std::uses_allocator_v<Value, Alloc>) {
                ::new (&value) Value();
            }
            auto status = BufferReaderWriter<Key>::read(buf, &key);
            if (!status.ok()) {
                return status;
            }
            status = BufferReaderWriter<Value>::read(buf, &value);
            if (!status.ok()) {
                return status;
            }
            map->emplace(std::move(key), std::move(value));
        }

        return Status::OK();
    }

    static size_t encodedSize(const Record &map) {
        if constexpr (IsTriviallyEncoding<Key>::value && IsTriviallyEncoding<Value>::value) {
            return 4 + (IsConstantEncodedSize<Key>::size + IsConstantEncodedSize<Value>::size) *
                               map.size();
        } else {
            size_t len = 4;
            for (auto &pair : map) {
                len += BufferReaderWriter<Key>::encodedSize(pair.first);
                len += BufferReaderWriter<Value>::encodedSize(pair.second);
            }
            return len;
        }
    }
};


template <typename Key, typename Value, typename Compare, typename Alloc>
struct BufferReaderWriter<std::map<Key, Value, Compare, Alloc>, void> {
    using Record = std::map<Key, Value, Compare, Alloc>;
    static void write(folly::IOBuf *buf, const Record &map) {
        auto len = encodedSize(map);
        buf->reserve(0, len);
        *reinterpret_cast<uint32_t *>(buf->writableTail()) = map.size();
        buf->append(4);

        for (auto &pair : map) {
            BufferReaderWriter<Key>::write(buf, pair.first);
            BufferReaderWriter<Value>::write(buf, pair.second);
        }
    }

    static Status read(folly::IOBuf *buf, Record *map) {
        if (buf->length() < 4) {
            return Status(ErrorCode::RPC_DESERIALIZE_ERROR,
                          "Data not enough, 4B required, %lu left",
                          buf->length());
        }

        auto n = *reinterpret_cast<const uint32_t *>(buf->data());
        buf->trimStart(4);
        map->clear();

        Key key;
        Value value;
        for (auto i = 0u; i < n; i++) {
            auto status = BufferReaderWriter<Key>::read(buf, &key);
            if (!status.ok()) {
                return status;
            }
            status = BufferReaderWriter<Value>::read(buf, &value);
            if (!status.ok()) {
                return status;
            }
            map->emplace(std::move(key), std::move(value));
        }

        return Status::OK();
    }

    static size_t encodedSize(const Record &map) {
        if constexpr (IsTriviallyEncoding<Key>::value && IsTriviallyEncoding<Value>::value) {
            return 4 + (IsConstantEncodedSize<Key>::size + IsConstantEncodedSize<Value>::size) *
                               map.size();
        } else {
            size_t len = 4;
            for (auto &pair : map) {
                len += BufferReaderWriter<Key>::encodedSize(pair.first);
                len += BufferReaderWriter<Value>::encodedSize(pair.second);
            }
            return len;
        }
    }
};

template <typename Key, typename Compare, typename Alloc>
struct BufferReaderWriter<std::unordered_set<Key, Compare, Alloc>, void> {
    using Set = std::unordered_set<Key, Compare, Alloc>;
    static void write(folly::IOBuf *buf, const Set &set) {
        auto len = encodedSize(set);
        buf->reserve(0, len);
        *reinterpret_cast<uint32_t *>(buf->writableTail()) = set.size();
        buf->append(4);

        for (auto &key : set) {
            BufferReaderWriter<Key>::write(buf, key);
        }
    }

    static Status read(folly::IOBuf *buf, Set *set) {
        if (buf->length() < 4) {
            return Status(ErrorCode::RPC_DESERIALIZE_ERROR,
                          "Data not enough, 4B required, %lu left",
                          buf->length());
        }

        auto n = *reinterpret_cast<const uint32_t *>(buf->data());
        buf->trimStart(4);
        set->clear();

        Key key;
        for (auto i = 0u; i < n; i++) {
            auto status = BufferReaderWriter<Key>::read(buf, &key);
            if (!status.ok()) {
                return status;
            }
            set->emplace(std::move(key));
        }

        return Status::OK();
    }

    static size_t encodedSize(const Set &set) {
        if constexpr (IsTriviallyEncoding<Key>::value) {
            return 4 + (IsConstantEncodedSize<Key>::size) * set.size();
        } else {
            size_t len = 4;
            for (auto &key : set) {
                len += BufferReaderWriter<Key>::encodedSize(key);
            }
            return len;
        }
    }
};

template <typename Key, typename Compare, typename Alloc>
struct BufferReaderWriter<std::set<Key, Compare, Alloc>, void> {
    using Record = std::set<Key, Compare, Alloc>;
    static void write(folly::IOBuf *buf, const Record &set) {
        auto len = encodedSize(set);
        buf->reserve(0, len);
        *reinterpret_cast<uint32_t *>(buf->writableTail()) = set.size();
        buf->append(4);

        for (auto &e : set) {
            BufferReaderWriter<Key>::write(buf, e);
        }
    }

    static Status read(folly::IOBuf *buf, Record *set) {
        if (buf->length() < 4) {
            return Status(ErrorCode::RPC_DESERIALIZE_ERROR,
                          "Data not enough, 4B required, %lu left",
                          buf->length());
        }

        auto n = *reinterpret_cast<const uint32_t *>(buf->data());
        buf->trimStart(4);
        set->clear();

        Key key;
        for (auto i = 0u; i < n; i++) {
            auto status = BufferReaderWriter<Key>::read(buf, &key);
            if (!status.ok()) {
                return status;
            }
            set->emplace(std::move(key));
        }

        return Status::OK();
    }

    static size_t encodedSize(const Record &set) {
        if constexpr (IsTriviallyEncoding<Key>::value) {
            return 4 + IsConstantEncodedSize<Key>::size * set.size();
        } else {
            size_t len = 4;
            for (auto &e : set) {
                len += BufferReaderWriter<Key>::encodedSize(e);
            }
            return len;
        }
    }
};

// clang-format off
struct FoldWrapper {
    static void write(folly::IOBuf *) {}

    template <typename Arg, typename... Args>
    static void write(folly::IOBuf *buf, const Arg &arg, const Args &... args) {
        BufferReaderWriter<Arg>::write(buf, arg);
        write(buf, args...);
    }

    static Status read(folly::IOBuf *) {
        return Status::OK();
    }

    template <typename Arg, typename... Args>
    static Status read(folly::IOBuf *buf, const Arg &arg, const Args &... args) {
        auto status = BufferReaderWriter<Arg>::read(buf, const_cast<Arg *>(&arg));
        if (!status.ok()) {
            return status;
        }
        return read(buf, args...);
    }

    template <typename... Args>
    static size_t encodedSize(const Args &... args) {
        return (BufferReaderWriter<Args>::encodedSize(args) + ...);
    }

    template <typename... Args>
    static constexpr size_t constEncodedSize() {
        static_assert(((IsConstantEncodedSize<Args>::size != ~0UL) && ...));
        return (IsConstantEncodedSize<Args>::size + ...);
    }
};

}  // namespace nebula::nrpc


