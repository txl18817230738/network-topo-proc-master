# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(SimdJson_LIB NAMES libsimdjson.so DOC "SimdJson library")
if(NOT SimdJson_LIB)
        message(FATAL_ERROR "libsimdjson not found")
endif()

add_library(SimdJson SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME SimdJson PATH ${SimdJson_LIB})
