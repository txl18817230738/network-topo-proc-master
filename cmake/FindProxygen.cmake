# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Proxygen_LIB NAMES libproxygen.so DOC "Proxygen library")
find_library(ProxygenHttpServer_LIB NAMES libproxygenhttpserver.so DOC "Proxygen http server library")
if(NOT Proxygen_LIB)
        message(FATAL_ERROR "libproxygen not found")
endif()
if(NOT ProxygenHttpServer_LIB)
        message(FATAL_ERROR "libproxygenhttpserver not found")
endif()

add_library(Proxygen SHARED IMPORTED GLOBAL)
add_library(Proxygen::httpserver SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Proxygen PATH ${Proxygen_LIB} LIBRARIES Wangle Boost::iostreams)
nebula_setup_imported_library(NAME Proxygen::httpserver PATH ${ProxygenHttpServer_LIB} LIBRARIES Proxygen)
