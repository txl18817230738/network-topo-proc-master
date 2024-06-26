# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(SSL_LIB NAMES libssl.so DOC "SSL library")
find_library(Crypto_LIB NAMES libcrypto.so DOC "Crypto library")
if(NOT SSL_LIB)
        message(FATAL_ERROR "libssl not found")
endif()
if(NOT Crypto_LIB)
        message(FATAL_ERROR "libcrypto not found")
endif()

add_library(SSL SHARED IMPORTED GLOBAL)
add_library(Crypto SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME SSL PATH ${SSL_LIB} LIBRARIES Crypto)
nebula_setup_imported_library(NAME Crypto PATH ${Crypto_LIB} LIBRARIES pthread)
