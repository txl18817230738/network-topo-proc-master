# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Fmt_LIB NAMES libfmt.so DOC "fmt library")
if(NOT Fmt_LIB)
        message(FATAL_ERROR "libfmt not found")
endif()

add_library(Fmt SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Fmt PATH ${Fmt_LIB})
