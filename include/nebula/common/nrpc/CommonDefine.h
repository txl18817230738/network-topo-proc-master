// Copyright (c) 2023 vesoft inc. All rights reserved.
#pragma once

#include <type_traits>

#include "nebula/common/nrpc/BufferReaderWriter.h"

#define MacroArgCount(...)      \
    _MacroArgCount(__VA_ARGS__, \
                   30,          \
                   29,          \
                   28,          \
                   27,          \
                   26,          \
                   25,          \
                   24,          \
                   23,          \
                   22,          \
                   21,          \
                   20,          \
                   19,          \
                   18,          \
                   17,          \
                   16,          \
                   15,          \
                   14,          \
                   13,          \
                   12,          \
                   11,          \
                   10,          \
                   9,           \
                   8,           \
                   7,           \
                   6,           \
                   5,           \
                   4,           \
                   3,           \
                   2,           \
                   1,           \
                   0)
#define _MacroArgCount(_0,    \
                       _1,    \
                       _2,    \
                       _3,    \
                       _4,    \
                       _5,    \
                       _6,    \
                       _7,    \
                       _8,    \
                       _9,    \
                       _10,   \
                       _11,   \
                       _12,   \
                       _13,   \
                       _14,   \
                       _15,   \
                       _16,   \
                       _17,   \
                       _18,   \
                       _19,   \
                       _20,   \
                       _21,   \
                       _22,   \
                       _23,   \
                       _24,   \
                       _25,   \
                       _26,   \
                       _27,   \
                       _28,   \
                       _29,   \
                       COUNT, \
                       ...)   \
    COUNT

#define CONCAT(A, B) _CONCAT(A, B)
#define _CONCAT(A, B) A##B

#define SERIALIZE_AS_SHALLOW_COPY(classname)                                        \
    template <>                                                                     \
    struct nebula::nrpc::IsConstantEncodedSize<classname> : public std::true_type { \
        static constexpr size_t size = sizeof(classname);                           \
    }

#define _MEMBER(membername) msg.membername

#define MEMBERS1(membername) _MEMBER(membername)

#define MEMBERS2(membername, ...) _MEMBER(membername), MEMBERS1(__VA_ARGS__)

#define MEMBERS3(membername, ...) _MEMBER(membername), MEMBERS2(__VA_ARGS__)

#define MEMBERS4(membername, ...) _MEMBER(membername), MEMBERS3(__VA_ARGS__)

#define MEMBERS5(membername, ...) _MEMBER(membername), MEMBERS4(__VA_ARGS__)

#define MEMBERS6(membername, ...) _MEMBER(membername), MEMBERS5(__VA_ARGS__)

#define MEMBERS7(membername, ...) _MEMBER(membername), MEMBERS6(__VA_ARGS__)

#define MEMBERS8(membername, ...) _MEMBER(membername), MEMBERS7(__VA_ARGS__)

#define MEMBERS9(membername, ...) _MEMBER(membername), MEMBERS8(__VA_ARGS__)

#define MEMBERS10(membername, ...) _MEMBER(membername), MEMBERS9(__VA_ARGS__)

#define MEMBERS11(membername, ...) _MEMBER(membername), MEMBERS10(__VA_ARGS__)

#define MEMBERS12(membername, ...) _MEMBER(membername), MEMBERS11(__VA_ARGS__)

#define MEMBERS13(membername, ...) _MEMBER(membername), MEMBERS12(__VA_ARGS__)

#define MEMBERS14(membername, ...) _MEMBER(membername), MEMBERS13(__VA_ARGS__)

#define MEMBERS15(membername, ...) _MEMBER(membername), MEMBERS14(__VA_ARGS__)

#define MEMBERS16(membername, ...) _MEMBER(membername), MEMBERS15(__VA_ARGS__)

#define MEMBERS17(membername, ...) _MEMBER(membername), MEMBERS16(__VA_ARGS__)

#define MEMBERS18(membername, ...) _MEMBER(membername), MEMBERS17(__VA_ARGS__)

#define MEMBERS19(membername, ...) _MEMBER(membername), MEMBERS18(__VA_ARGS__)

#define MEMBERS20(membername, ...) _MEMBER(membername), MEMBERS19(__VA_ARGS__)

#define MEMBERS21(membername, ...) _MEMBER(membername), MEMBERS20(__VA_ARGS__)

#define MEMBERS22(membername, ...) _MEMBER(membername), MEMBERS21(__VA_ARGS__)

#define MEMBERS23(membername, ...) _MEMBER(membername), MEMBERS22(__VA_ARGS__)

#define MEMBERS24(membername, ...) _MEMBER(membername), MEMBERS23(__VA_ARGS__)

#define MEMBERS25(membername, ...) _MEMBER(membername), MEMBERS24(__VA_ARGS__)

#define MEMBERS26(membername, ...) _MEMBER(membername), MEMBERS25(__VA_ARGS__)

#define MEMBERS27(membername, ...) _MEMBER(membername), MEMBERS26(__VA_ARGS__)

#define MEMBERS28(membername, ...) _MEMBER(membername), MEMBERS27(__VA_ARGS__)

#define MEMBERS29(membername, ...) _MEMBER(membername), MEMBERS28(__VA_ARGS__)

#define MEMBERS30(membername, ...) _MEMBER(membername), MEMBERS29(__VA_ARGS__)

#define _STATIC_MEMBER(classname, membername) decltype(classname::membername)

#define STATIC_MEMBER1(classname, membername) _STATIC_MEMBER(classname, membername)

#define STATIC_MEMBER2(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER1(classname, __VA_ARGS__)

#define STATIC_MEMBER3(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER2(classname, __VA_ARGS__)

#define STATIC_MEMBER4(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER3(classname, __VA_ARGS__)

#define STATIC_MEMBER5(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER4(classname, __VA_ARGS__)

#define STATIC_MEMBER6(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER5(classname, __VA_ARGS__)

#define STATIC_MEMBER7(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER6(classname, __VA_ARGS__)

#define STATIC_MEMBER8(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER7(classname, __VA_ARGS__)

#define STATIC_MEMBER9(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER8(classname, __VA_ARGS__)

#define STATIC_MEMBER10(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER9(classname, __VA_ARGS__)

#define STATIC_MEMBER11(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER10(classname, __VA_ARGS__)

#define STATIC_MEMBER12(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER11(classname, __VA_ARGS__)

#define STATIC_MEMBER13(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER12(classname, __VA_ARGS__)

#define STATIC_MEMBER14(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER13(classname, __VA_ARGS__)

#define STATIC_MEMBER15(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER14(classname, __VA_ARGS__)

#define STATIC_MEMBER16(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER15(classname, __VA_ARGS__)

#define STATIC_MEMBER17(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER16(classname, __VA_ARGS__)

#define STATIC_MEMBER18(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER17(classname, __VA_ARGS__)

#define STATIC_MEMBER19(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER18(classname, __VA_ARGS__)

#define STATIC_MEMBER20(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER19(classname, __VA_ARGS__)

#define STATIC_MEMBER21(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER20(classname, __VA_ARGS__)

#define STATIC_MEMBER22(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER21(classname, __VA_ARGS__)

#define STATIC_MEMBER23(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER22(classname, __VA_ARGS__)

#define STATIC_MEMBER24(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER23(classname, __VA_ARGS__)

#define STATIC_MEMBER25(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER24(classname, __VA_ARGS__)

#define STATIC_MEMBER26(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER25(classname, __VA_ARGS__)

#define STATIC_MEMBER27(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER26(classname, __VA_ARGS__)

#define STATIC_MEMBER28(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER27(classname, __VA_ARGS__)

#define STATIC_MEMBER29(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER28(classname, __VA_ARGS__)

#define STATIC_MEMBER30(classname, membername, ...) \
    _STATIC_MEMBER(classname, membername), _STATIC_MEMBER29(classname, __VA_ARGS__)


#define SERIALIZE_NO_MEMBER(classname)                            \
    template <>                                                   \
    struct nebula::nrpc::BufferReaderWriter<classname> {          \
        static void write(folly::IOBuf *, const classname &) {}   \
        static nebula::Status read(folly::IOBuf *, classname *) { \
            return nebula::Status::OK();                          \
        }                                                         \
        static size_t encodedSize(const classname &) {            \
            return 0U;                                            \
        }                                                         \
    };

#define SERIALIZE_NO_MEMBER_BASE(classname, base)                             \
    template <>                                                               \
    struct nebula::nrpc::BufferReaderWriter<classname> {                      \
        static_assert(std::is_base_of_v<base, classname>);                    \
        static void write(folly::IOBuf *buf, const classname &msg) {          \
            nebula::nrpc::BufferReaderWriter<base>::write(buf, msg);          \
        }                                                                     \
                                                                              \
        static nebula::Status read(folly::IOBuf *buf, classname *msgPtr) {    \
            return nebula::nrpc::BufferReaderWriter<base>::read(buf, msgPtr); \
        }                                                                     \
                                                                              \
        static size_t encodedSize(const classname &msg) {                     \
            return BufferReaderWriter<base>::encodedSize(msg);                \
        }                                                                     \
    };


#define _SERIALIZE_EACH_MEMBER_BASE(classname, base, ...)                  \
    template <>                                                            \
    struct nebula::nrpc::BufferReaderWriter<classname> {                   \
        static_assert(std::is_base_of_v<base, classname>);                 \
        static void write(folly::IOBuf *buf, const classname &msg) {       \
            nebula::nrpc::BufferReaderWriter<base>::write(buf, msg);       \
            FoldWrapper::write(buf, __VA_ARGS__);                          \
        }                                                                  \
                                                                           \
        static nebula::Status read(folly::IOBuf *buf, classname *msgPtr) { \
            nebula::nrpc::BufferReaderWriter<base>::read(buf, msgPtr);     \
            classname &msg = *msgPtr;                                      \
            return FoldWrapper::read(buf, __VA_ARGS__);                    \
        }                                                                  \
                                                                           \
        static size_t encodedSize(const classname &msg) {                  \
            return BufferReaderWriter<base>::encodedSize(msg) +            \
                   FoldWrapper::encodedSize(__VA_ARGS__);                  \
        }                                                                  \
    };

#define SERIALIZE_EACH_MEMBER_BASE(classname, base, ...) \
    _SERIALIZE_EACH_MEMBER_BASE(                         \
            classname, base, CONCAT(MEMBERS, MacroArgCount(__VA_ARGS__))(__VA_ARGS__))

#define _SERIALIZE_EACH_MEMBER(classname, ...)                             \
    template <>                                                            \
    struct nebula::nrpc::BufferReaderWriter<classname> {                   \
        static void write(folly::IOBuf *buf, const classname &msg) {       \
            FoldWrapper::write(buf, __VA_ARGS__);                          \
        }                                                                  \
                                                                           \
        static nebula::Status read(folly::IOBuf *buf, classname *msgPtr) { \
            classname &msg = *msgPtr;                                      \
            return FoldWrapper::read(buf, __VA_ARGS__);                    \
        }                                                                  \
                                                                           \
        static size_t encodedSize(const classname &msg) {                  \
            return FoldWrapper::encodedSize(__VA_ARGS__);                  \
        }                                                                  \
    };

#define SERIALIZE_EACH_MEMBER(classname, ...) \
    _SERIALIZE_EACH_MEMBER(classname, CONCAT(MEMBERS, MacroArgCount(__VA_ARGS__))(__VA_ARGS__))

#define SERIALIZE_DECLARE(classname)                                      \
    template <>                                                           \
    struct nebula::nrpc::BufferReaderWriter<classname> {                  \
        static void write(folly::IOBuf *buf, const classname &msg);       \
        static nebula::Status read(folly::IOBuf *buf, classname *msgPtr); \
        static size_t encodedSize(const classname &msg);                  \
    }

#define _CONSTANT_LENGTH(classname, ...)                                             \
    template <>                                                                      \
    struct nebula::nrpc::IsConstantEncodedSize<classname> : public std::true_type {  \
        static constexpr size_t size = FoldWrapper::constEncodedSize<__VA_ARGS__>(); \
    };

#define CONSTANT_LENGTH(classname, ...) \
    _CONSTANT_LENGTH(                   \
            classname,                  \
            CONCAT(STATIC_MEMBER, MacroArgCount(__VA_ARGS__))(classname, __VA_ARGS__))

#define SERIALIZE_CONST_LENGTH_MEMBER(classname, ...) \
    CONSTANT_LENGTH(classname, __VA_ARGS__)           \
    SERIALIZE_EACH_MEMBER(classname, __VA_ARGS__)
