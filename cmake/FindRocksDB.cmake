# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(RocksDB_LIB NAMES librocksdb.so DOC "RocksDB library")
if(NOT RocksDB_LIB)
        message(FATAL_ERROR "librocksdb not found")
endif()

add_library(RocksDB SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(
    NAME RocksDB
    PATH ${RocksDB_LIB}
    LIBRARIES
        Snappy
        BZ2
        LZ4
        Zlib
        Zstd
        pthread
        m
)
