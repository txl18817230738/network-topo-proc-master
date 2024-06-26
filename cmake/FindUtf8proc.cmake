# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Utf8proc_LIB NAMES libutf8proc.so DOC "Utf8proc library")
if(NOT Utf8proc_LIB)
        message(FATAL_ERROR "libutf8proc not found")
endif()

add_library(Utf8proc SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Utf8proc PATH ${Utf8proc_LIB})
