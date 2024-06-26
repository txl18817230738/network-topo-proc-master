# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Ldap_LIB NAMES libldap.so DOC "Ldap library")
find_library(Lber_LIB NAMES liblber.so DOC "Lber library")
if(NOT Ldap_LIB)
        message(FATAL_ERROR "libldap not found")
endif()
if(NOT Lber_LIB)
        message(FATAL_ERROR "liblber not found")
endif()

add_library(Ldap SHARED IMPORTED GLOBAL)
add_library(Lber SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Ldap PATH ${Ldap_LIB} LIBRARIES Lber Sasl SSL resolv)
nebula_setup_imported_library(NAME Lber PATH ${Lber_LIB} LIBRARIES pthread)
