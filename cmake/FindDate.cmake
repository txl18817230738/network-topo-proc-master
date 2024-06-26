# Copyright (c) 2024 vesoft inc. All rights reserved.

find_library(Date_LIB NAMES libdate-tz.so DOC "date library")
if(NOT Date_LIB)
        message(FATAL_ERROR "libdate-tz not found")
endif()

add_library(Date SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Date PATH ${Date_LIB} LIBRARIES Curl)
