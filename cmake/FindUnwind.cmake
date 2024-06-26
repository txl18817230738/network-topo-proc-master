# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Unwind_LIB NAMES libunwind.so DOC "Libunwind library")
if(NOT Unwind_LIB)
        message(FATAL_ERROR "libunwind not found")
endif()

add_library(Unwind SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Unwind PATH ${Unwind_LIB})
