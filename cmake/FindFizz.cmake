# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Fizz_LIB NAMES libfizz.so DOC "Fizz library")
if(NOT Fizz_LIB)
        message(FATAL_ERROR "libfizz not found")
endif()

add_library(Fizz SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Fizz PATH ${Fizz_LIB} LIBRARIES Folly)
