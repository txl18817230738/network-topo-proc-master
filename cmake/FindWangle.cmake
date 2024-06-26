# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Wangle_LIB NAMES libwangle.so DOC "Wangle library")
if(NOT Wangle_LIB)
        message(FATAL_ERROR "libwangle not found")
endif()

add_library(Wangle SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Wangle PATH ${Wangle_LIB} LIBRARIES Fizz)
