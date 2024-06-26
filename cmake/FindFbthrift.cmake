# Copyright (c) 2023 vesoft inc. All rights reserved.

find_program(Thrift_BIN NAMES thrift1)
if(NOT Thrift_BIN)
    message(FATAL_ERROR "thrift1 not found")
endif()

set(thrift_libs
    thriftcpp2
    async
    thriftanyrep
    thriftprotocol
    thriftannotation
    transport
    concurrency
    thriftmetadata
    thriftfrozen2
    thrifttype
    thrifttyperep
    thrift-core
    rpcmetadata
)

# Find individual thrift libraries
foreach(lib ${thrift_libs})
    find_library(${lib}_LIB NAMES lib${lib}.so)
    if(NOT ${lib}_LIB)
        message(FATAL_ERROR "lib${lib} not found")
    endif()
    add_library(Thrift::${lib} SHARED IMPORTED GLOBAL)
    list(APPEND submodules Thrift::${lib})
endforeach()

# Adjust the dependencies accordingly if the topology changes
nebula_setup_imported_library(
    NAME Thrift::thriftcpp2
    PATH ${thriftcpp2_LIB}
    LIBRARIES
        Thrift::transport
        Thrift::thriftanyrep
        Thrift::thriftprotocol
        Thrift::thriftmetadata
        Thrift::async
        Fizz
)
nebula_setup_imported_library(NAME Thrift::async PATH ${async_LIB} LIBRARIES Thrift::transport)
nebula_setup_imported_library(NAME Thrift::thriftanyrep PATH ${thriftanyrep_LIB} LIBRARIES Thrift::thrifttype)
nebula_setup_imported_library(
    NAME Thrift::thriftprotocol
    PATH ${thriftprotocol_LIB}
    LIBRARIES
        Thrift::thrift-core
        Thrift::rpcmetadata
)
nebula_setup_imported_library(
    NAME Thrift::transport
    PATH ${transport_LIB}
    LIBRARIES
        Thrift::concurrency
        Thrift::thrift-core
        Thrift::rpcmetadata
)
nebula_setup_imported_library(NAME Thrift::concurrency PATH ${concurrency_LIB} LIBRARIES Folly)
nebula_setup_imported_library(
    NAME Thrift::thriftfrozen2
    PATH ${thriftfrozen2_LIB}
    LIBRARIES
        Thrift::thriftmetadata
        Thrift::thrift-core
)
nebula_setup_imported_library(NAME Thrift::thriftmetadata PATH ${thriftmetadata_LIB} LIBRARIES Folly)
nebula_setup_imported_library(NAME Thrift::thrifttype PATH ${thrifttype_LIB} LIBRARIES Thrift::thrifttyperep)
nebula_setup_imported_library(NAME Thrift::thrifttyperep PATH ${thrifttyperep_LIB} LIBRARIES Folly)
nebula_setup_imported_library(NAME Thrift::thrift-core PATH ${thrift-core_LIB} LIBRARIES Folly)
nebula_setup_imported_library(NAME Thrift::rpcmetadata PATH ${rpcmetadata_LIB} LIBRARIES Folly)
nebula_setup_imported_library(NAME Thrift::thriftannotation PATH ${thriftannotation_LIB} LIBRARIES Folly)

# Create an interface library to represent the whole thrift libraries
add_library(Thrift INTERFACE)
set_target_properties(
    Thrift
    PROPERTIES
        INTERFACE_LINK_LIBRARIES "${submodules}"
)

set_property(GLOBAL APPEND PROPERTY NEBULA_TARGET_LIST Thrift)
