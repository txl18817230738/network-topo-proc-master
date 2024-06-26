# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Jemalloc_LIB NAMES libjemalloc.so DOC "Jemalloc library")
if(NOT Jemalloc_LIB)
        message(FATAL_ERROR "libjemalloc not found")
endif()

add_library(Jemalloc SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Jemalloc PATH ${Jemalloc_LIB} LIBRARIES pthread dl)
