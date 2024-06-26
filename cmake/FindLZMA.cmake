# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(LZMA_LIB NAMES liblzma.so DOC "LZMA library")
if(NOT LZMA_LIB)
        message(FATAL_ERROR "liblzma not found")
endif()

add_library(LZMA SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME LZMA PATH ${LZMA_LIB} LIBRARIES pthread)
