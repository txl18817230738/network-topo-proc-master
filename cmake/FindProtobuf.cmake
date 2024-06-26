# Copyright (c) 2024 vesoft inc. All rights reserved.

find_program(PROTOC NAMES protoc)
if(NOT PROTOC)
    message(FATAL_ERROR "protoc: not found")
endif()

find_library(protobuf_LIB NAMES libprotobuf.so)
if(NOT protobuf_LIB)
    message(FATAL_ERROR "libprotobuf.so not found")
endif()

find_library(protobuf_lite_LIB NAMES libprotobuf-lite.so)
if(NOT protobuf_lite_LIB)
    message(FATAL_ERROR "libprotobuf-lite.so not found")
endif()

find_library(protoc_LIB NAMES libprotoc.so)
if(NOT protoc_LIB)
    message(FATAL_ERROR "libprotoc.so not found")
endif()

add_library(Protobuf SHARED IMPORTED GLOBAL)
add_library(Protobuf::lite SHARED IMPORTED GLOBAL)
add_library(Protoc SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(
    NAME Protobuf
    PATH ${protobuf_LIB}
    LIBRARIES
        Zlib
        Absl::raw_hash_set
        Absl::statusor
        Absl::die_if_null
        Absl::log_internal_conditions
        Absl::log_internal_check_op
        Absl::bad_variant_access
)
nebula_setup_imported_library(
    NAME Protobuf::lite
    PATH ${protobuf_lite_LIB}
    LIBRARIES
        Absl::log_internal_check_op
        Absl::log_internal_message
        Absl::raw_hash_set
        Absl::cord
)
nebula_setup_imported_library(
    NAME Protoc
    PATH ${protoc_LIB}
    LIBRARIES Protobuf
)

set_property(GLOBAL APPEND PROPERTY NEBULA_TARGET_LIST Protobuf)
set_property(GLOBAL APPEND PROPERTY NEBULA_TARGET_LIST Protobuf::lite)
set_property(GLOBAL APPEND PROPERTY NEBULA_TARGET_LIST Protoc)
