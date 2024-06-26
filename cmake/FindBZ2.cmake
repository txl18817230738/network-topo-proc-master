# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(BZ2_LIB NAMES libbz2.so DOC "BZ2 library")
if(NOT BZ2_LIB)
        message(FATAL_ERROR "libbz2 not found")
endif()

add_library(BZ2 SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME BZ2 PATH ${BZ2_LIB})
