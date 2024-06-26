# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Sodium_LIB NAMES libsodium.so DOC "Sodium library")
if(NOT Sodium_LIB)
        message(FATAL_ERROR "libsodium not found")
endif()

add_library(Sodium SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Sodium PATH ${Sodium_LIB} LIBRARIES pthread)
