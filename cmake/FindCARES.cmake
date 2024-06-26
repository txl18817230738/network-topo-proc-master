# Copyright (c) 2024 vesoft inc. All rights reserved.

find_library(cares_LIB NAMES libcares.so DOC "libcares.so library")
if(NOT cares_LIB)
    message(FATAL_ERROR "libcares.so not found")
endif()

add_library(CARES SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME CARES PATH ${cares_LIB})
