# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Gflags_LIB NAMES libgflags.so DOC "Gflags library")
if(NOT Gflags_LIB)
        message(FATAL_ERROR "libgflags not found")
endif()

add_library(Gflags SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Gflags PATH ${Gflags_LIB} LIBRARIES pthread)
