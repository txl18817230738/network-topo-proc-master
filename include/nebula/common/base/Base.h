// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <cassert>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/any.hpp>
#include <boost/variant.hpp>
#include <folly/Hash.h>
#include <folly/Range.h>
#include <folly/String.h>
#include <folly/init/Init.h>
#include <gflags/gflags.h>

#if defined(__clang__) && defined(__aarch64__)
#if FOLLY_HAVE_EXTRANDOM_SFMT19937
#undef FOLLY_HAVE_EXTRANDOM_SFMT19937
#endif
#endif  // __clang__ && __aarch64__
#include <folly/Conv.h>
#include <folly/RWSpinLock.h>
#include <folly/Random.h>
#include <folly/ThreadLocal.h>
#include <folly/Unicode.h>
#include <folly/Varint.h>
#include <folly/dynamic.h>
#include <folly/json.h>

#include "nebula/common/base/Logging.h"

#define NG_MUST_USE_RESULT __attribute__((warn_unused_result))
#define DONT_OPTIMIZE __attribute__((optimize("O0")))

#define ALWAYS_INLINE __attribute__((always_inline))
#define ALWAYS_NO_INLINE __attribute__((noinline))

#define BEGIN_NO_OPTIMIZATION _Pragma("GCC push_options") _Pragma("GCC optimize(\"O0\")")
#define END_NO_OPTIMIZATION _Pragma("GCC pop_options")

#define NEBULA_STRINGIFY(STR) NEBULA_STRINGIFY_X(STR)
#define NEBULA_STRINGIFY_X(STR) #STR

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif  // UNUSED

#ifndef MAYBE_UNUSED
#if (__cplusplus >= 201703L)  // c++17
#include <folly/CppAttributes.h>
#define MAYBE_UNUSED FOLLY_MAYBE_UNUSED
#else
#define MAYBE_UNUSED __attribute__((unused))
#endif
#endif

#ifndef COMPILER_BARRIER
#define COMPILER_BARRIER() asm volatile("" ::: "memory")
#endif  // COMPILER_BARRIER

#if defined(__GNUC__) || defined(__clang__)
#define INLINE_LAMBDA __attribute__((__always_inline__))
#else
#define INLINE_LAMBDA
#endif
