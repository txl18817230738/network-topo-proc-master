# Copyright (c) 2024 vesoft inc. All rights reserved.

set(grpc_libs
    grpc
    grpc_unsecure
    grpcpp_channelz
    grpc_plugin_support
    grpc_authorization_provider
    grpc++
    grpc++_alts
    grpc++_error_details
    grpc++_reflection
    grpc++_unsecure
    address_sorting
    gpr
    upb
    upb_collections_lib
    upb_json_lib
    upb_textformat_lib
    utf8_range_lib
)

foreach(lib ${grpc_libs})
    find_library(${lib}_LIB NAMES lib${lib}.so)
    if(NOT ${lib}_LIB)
        message(FATAL_ERROR "lib${lib} not found")
    endif()
    add_library(GRPC::${lib} SHARED IMPORTED GLOBAL)
endforeach()

nebula_setup_imported_library(
    NAME GRPC::grpc
    PATH ${grpc_LIB}
    LIBRARIES
        RE2
        CARES
        Zlib
        SSL
        GRPC::gpr
        GRPC::address_sorting
        GRPC::upb_collections_lib
        GRPC::upb_json_lib
        Absl::raw_hash_set
        Absl::bad_variant_access
        Absl::statusor
)

nebula_setup_imported_library(
    NAME GRPC::grpc_unsecure
    PATH ${grpc_unsecure_LIB}
    LIBRARIES
        CARES
        Zlib
        GRPC::gpr
        GRPC::address_sorting
        GRPC::upb_collections_lib
        GRPC::utf8_range_lib
        Absl::raw_hash_set
        Absl::bad_variant_access
        Absl::statusor
        Absl::random_internal_pool_urbg
)

nebula_setup_imported_library(
    NAME GRPC::grpcpp_channelz
    PATH ${grpcpp_channelz_LIB}
    LIBRARIES
        GRPC::grpc++
        GRPC::upb
        Protobuf
)

nebula_setup_imported_library(
    NAME GRPC::grpc_plugin_support
    PATH ${grpc_plugin_support_LIB}
    LIBRARIES
        Protoc
        Absl::raw_hash_set
)

nebula_setup_imported_library(
    NAME GRPC::grpc_authorization_provider
    PATH ${grpc_authorization_provider_LIB}
    LIBRARIES
        GRPC::upb
        GRPC::utf8_range_lib
        GRPC::gpr
        Zlib
        CARES
        RE2
)

nebula_setup_imported_library(
    NAME GRPC::grpc++
    PATH ${grpc++_LIB}
    LIBRARIES
        GRPC::grpc
)

nebula_setup_imported_library(
    NAME GRPC::grpc++_alts
    PATH ${grpc++_alts_LIB}
    LIBRARIES
        GRPC::grpc++
)
nebula_setup_imported_library(
    NAME GRPC::grpc++_error_details
    PATH ${grpc++_error_details_LIB}
)
nebula_setup_imported_library(
    NAME GRPC::grpc++_reflection
    PATH ${grpc++_reflection_LIB}
    LIBRARIES
        GRPC::grpc++
        GRPC::upb
        Protobuf
)
nebula_setup_imported_library(
    NAME GRPC::grpc++_unsecure
    PATH ${grpc++_unsecure_LIB}
    LIBRARIES
        GRPC::grpc_unsecure
)
nebula_setup_imported_library(
    NAME GRPC::address_sorting
    PATH ${address_sorting_LIB}
)
nebula_setup_imported_library(
    NAME GRPC::gpr
    PATH ${gpr_LIB}
    LIBRARIES
        Absl::status
        Absl::flags_internal
        Absl::flags_reflection
        Absl::flags_marshalling
        Absl::str_format_internal
)
nebula_setup_imported_library(
    NAME GRPC::upb
    PATH ${upb_LIB}
)
nebula_setup_imported_library(
    NAME GRPC::upb_collections_lib
    PATH ${upb_collections_lib_LIB}
)
nebula_setup_imported_library(
    NAME GRPC::upb_json_lib
    PATH ${upb_json_lib_LIB}
    LIBRARIES
        GRPC::utf8_range_lib
)
nebula_setup_imported_library(
    NAME GRPC::upb_textformat_lib
    PATH ${upb_textformat_lib_LIB}
    LIBRARIES
        GRPC::utf8_range_lib
)
nebula_setup_imported_library(
    NAME GRPC::utf8_range_lib
    PATH ${utf8_range_lib_LIB}
)

# Codegen from .proto file of gRPC
# @PROTO  path to the .proto file
# @OUTDIR output directory, the generated code will be
#               grouped into subdirectories according to different languages.
# @DEPS   dependent proto package, if any
# @NO_EXPORT   only generate C++ code if set
macro(grpc_generate)
    cmake_parse_arguments(
        pb
        "NO_EXPORT"
        "PROTO;OUTDIR"
        "DEPS"
        ${ARGN}
    )
    get_filename_component(bin_dir ${PROTOC} DIRECTORY)
    get_filename_component(proto_dir ${pb_PROTO} DIRECTORY)
    get_filename_component(proto_filename ${pb_PROTO} NAME)
    get_filename_component(basename ${proto_filename} NAME_WLE)
    file(MAKE_DIRECTORY ${pb_OUTDIR}/cpp)
    set(output_headers
        ${pb_OUTDIR}/cpp/${basename}.pb.h
        ${pb_OUTDIR}/cpp/${basename}.grpc.pb.h
    )
    set(output_sources
        ${pb_OUTDIR}/cpp/${basename}.pb.cc
        ${pb_OUTDIR}/cpp/${basename}.grpc.pb.cc
    )
    if (pb_NO_EXPORT)
        add_custom_command(
            COMMENT "Generating gRPC files for '${basename}'"
            OUTPUT ${output_headers} ${output_sources}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${pb_PROTO}
            COMMAND
                env LD_LIBRARY_PATH=${LIB_STDCXX_DIR}:${LIB_GCC_DIR} PATH=${bin_dir}:$ENV{PATH}
                ${PROTOC}
                    -I ${CMAKE_CURRENT_SOURCE_DIR}/${proto_dir}
                    --plugin=protoc-gen-grpc=${bin_dir}/grpc_cpp_plugin
                    --cpp_out=${pb_OUTDIR}/cpp
                    --grpc_out=${pb_OUTDIR}/cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/${pb_PROTO}
        )
    else()
        file(MAKE_DIRECTORY
            ${pb_OUTDIR}/java
            ${pb_OUTDIR}/go/${basename}
            ${pb_OUTDIR}/python
            ${pb_OUTDIR}/node      # TODO(dutor) `protoc' is buggy on the `--js_out' option
        )
        add_custom_command(
            COMMENT "Generating gRPC files for '${basename}'"
            OUTPUT ${output_headers} ${output_sources}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${pb_PROTO}
            COMMAND
                env LD_LIBRARY_PATH=${LIB_STDCXX_DIR}:${LIB_GCC_DIR} PATH=${bin_dir}:$ENV{PATH}
                ${PROTOC}
                    -I ${CMAKE_CURRENT_SOURCE_DIR}/${proto_dir}
                    --plugin=protoc-gen-grpc=${bin_dir}/grpc_cpp_plugin
                    --plugin=protoc-gen-grpc-python=${bin_dir}/grpc_python_plugin
                    --plugin=protoc-gen-grpc-node=${bin_dir}/grpc_node_plugin
                    --cpp_out=${pb_OUTDIR}/cpp
                    --grpc_out=${pb_OUTDIR}/cpp
                    --java_out=${pb_OUTDIR}/java
                    --grpc-java_out=${pb_OUTDIR}/java
                    --go_out=${pb_OUTDIR}/go/${basename}
                    --go_opt=paths=source_relative
                    --go-grpc_out=${pb_OUTDIR}/go/${basename}
                    --go-grpc_opt=paths=source_relative
                    --python_out=${pb_OUTDIR}/python
                    --grpc-python_out=${pb_OUTDIR}/python
                    #--js_out=${pb_OUTDIR}/node
                    --grpc-node_out=${pb_OUTDIR}/node
                    ${CMAKE_CURRENT_SOURCE_DIR}/${pb_PROTO}
        )
    endif()
    nebula_add_solib(
        NAME ${basename}-grpc
        SOURCES ${output_sources}
        LIBRARIES
            GRPC::grpc++
            Protobuf
            ${pb_DEPS}
    )
    message(STATUS "Added target ${basename}-grpc")
endmacro()
