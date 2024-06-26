# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Ev_LIB NAMES libev.so DOC "ev library")
if(NOT Ev_LIB)
        message(FATAL_ERROR "libev not found")
endif()

add_library(Ev SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Ev PATH ${Ev_LIB})
