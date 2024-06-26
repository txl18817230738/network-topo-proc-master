# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Zlib_LIB NAMES libz.so DOC "Zlib library")
if(NOT Zlib_LIB)
        message(FATAL_ERROR "libz not found")
endif()

add_library(Zlib SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Zlib PATH ${Zlib_LIB})
