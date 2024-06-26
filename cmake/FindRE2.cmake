# Copyright (c) 2024 vesoft inc. All rights reserved.

find_library(RE2_LIB NAMES libre2.so DOC "lbre2.so library")
if(NOT RE2_LIB)
    message(FATAL_ERROR "libre2.so not found")
endif()

add_library(RE2 SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(
    NAME RE2
    PATH ${RE2_LIB}
    LIBRARIES
        Absl::str_format_internal
        Absl::synchronization
        Absl::raw_hash_set
)
