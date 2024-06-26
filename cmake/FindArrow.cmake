# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Arrow_LIB NAMES libarrow.so DOC "Apache Arrow library")
find_library(ArrowTesting_LIB NAMES libarrow_testing.so DOC "Apache Arrow testing library")
find_library(ArrowGandiva_LIB NAMES libgandiva.so DOC "Apache Gandiva library")
if(NOT Arrow_LIB)
    message(FATAL_ERROR "libarrow not found")
endif()
if(NOT ArrowTesting_LIB)
    message(FATAL_ERROR "libarrow_testing not found")
endif()
if(NOT ArrowGandiva_LIB)
    message(FATAL_ERROR "libgandiva not found")
endif()

add_library(Arrow SHARED IMPORTED GLOBAL)
add_library(Arrow::testing SHARED IMPORTED GLOBAL)
add_library(Arrow::gandiva SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(
    NAME Arrow
    PATH ${Arrow_LIB}
    LIBRARIES RE2 Utf8proc pthread dl m
    NO_TOPO
)
nebula_setup_imported_library(
    NAME Arrow::testing
    PATH ${ArrowTesting_LIB}
    LIBRARIES Arrow Gtest
    NO_TOPO
)
nebula_setup_imported_library(
    NAME Arrow::gandiva
    PATH ${ArrowGandiva_LIB}
    LIBRARIES Arrow Crypto Zlib Zstd
    NO_TOPO
)
