# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Zstd_LIB NAMES libzstd.so DOC "Zstd library")
if(NOT Zstd_LIB)
        message(FATAL_ERROR "libzstd not found")
endif()

add_library(Zstd SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Zstd PATH ${Zstd_LIB})
