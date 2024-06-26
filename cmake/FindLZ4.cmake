# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(LZ4_LIB NAMES liblz4.so DOC "LZ4 library")
if(NOT LZ4_LIB)
        message(FATAL_ERROR "liblz4 not found")
endif()

add_library(LZ4 SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME LZ4 PATH ${LZ4_LIB})
