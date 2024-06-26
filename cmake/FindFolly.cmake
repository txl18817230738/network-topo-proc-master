# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Folly_LIB NAMES libfolly.so DOC "Folly library")
find_library(FollyBenchmark_LIB NAMES libfollybenchmark.so DOC "Folly benchmark library")
if(NOT Folly_LIB)
        message(FATAL_ERROR "libfolly not found")
endif()
if(NOT FollyBenchmark_LIB)
        message(FATAL_ERROR "libfollybenchmark not found")
endif()

add_library(Folly SHARED IMPORTED GLOBAL)
add_library(Folly::benchmark SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Folly::benchmark PATH ${FollyBenchmark_LIB} LIBRARIES Folly)
nebula_setup_imported_library(
    NAME Folly
    PATH ${Folly_LIB}
    LIBRARIES
        Boost::context
        Boost::filesystem
        Boost::program_options
        Fmt
        SSL
        Event
        Glog
        DoubleConversion
        Sodium
        BZ2
        LZ4
        Zlib
        LZMA
        Snappy
        Zstd
        Dwarf
        m
        dl
)
