# Copyright (c) 2022 vesoft inc. All rights reserved.

# - Try to find duckdb includes dirs and libraries
#
# Usage of this module as follows:
#
#     find_package(Duckdb)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
# Variables defined by this module:
#
#  Duckdb_FOUND            System has duckdb, include and lib dirs found
#  Duckdb_INCLUDE_DIR      The duckdb includes directories.
#  Duckdb_LIBRARY          The duckdb library.

find_path(Duckdb_INCLUDE_DIR NAMES duckdb)
find_library(Duckdb_LIBRARY NAMES libduckdb_static.a)

message(STATUS "Duckdb: ${Duckdb_INCLUDE_DIR}")

if(Duckdb_INCLUDE_DIR AND Duckdb_LIBRARY)
    set(Duckdb_FOUND TRUE)
    mark_as_advanced(
        Duckdb_INCLUDE_DIR
        Duckdb_LIBRARY
    )
endif()

if(NOT Duckdb_FOUND)
    message(FATAL_ERROR "Duckdb doesn't exist")
endif()

