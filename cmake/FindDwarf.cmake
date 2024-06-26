# Copyright (c) 2024 vesoft inc. All rights reserved.

find_library(Dwarf_LIB NAMES libdwarf.so DOC "Dwarf library")
if(NOT Dwarf_LIB)
        message(FATAL_ERROR "libdwarf not found")
endif()

add_library(Dwarf SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Dwarf PATH ${Dwarf_LIB})
