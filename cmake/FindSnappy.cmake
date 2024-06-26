# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Snappy_LIB NAMES libsnappy.so DOC "Snappy library")
if(NOT Snappy_LIB)
        message(FATAL_ERROR "libsnappy not found")
endif()

add_library(Snappy SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Snappy PATH ${Snappy_LIB})
