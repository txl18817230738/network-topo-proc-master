# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(DoubleConversion_LIB NAMES libdouble-conversion.so DOC "double-version library")
if(NOT DoubleConversion_LIB)
        message(FATAL_ERROR "libdouble-conversion not found")
endif()

add_library(DoubleConversion SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME DoubleConversion PATH ${DoubleConversion_LIB})
