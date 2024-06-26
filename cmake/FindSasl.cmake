# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Sasl_LIB NAMES libsasl2.so DOC "Sasl library")
if(NOT Sasl_LIB)
        message(FATAL_ERROR "libsasl not found")
endif()

add_library(Sasl SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Sasl PATH ${Sasl_LIB} LIBRARIES dl)
