# Copyright (c) 2022 vesoft inc. All rights reserved.

# CMake version check
cmake_minimum_required(VERSION 3.15.0)

# Set the project name
project("Nebula Graph C++ SDK" C CXX)

set(CMAKE_DEPENDS_IN_PROJECT_ONLY TRUE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_INCLUDE_WHAT_YOU_USE "Enable include-what-you-use find not used include files" OFF)

add_definitions(-DNEBULA_HOME=${CMAKE_SOURCE_DIR})

if ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "Only out-of-source build is permitted, Please run `cmake' in a separate directory.")
endif ()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/nebula)

include(PlatformCheck)
include(NebulaCMakeMacros)
include(GeneralCMakeOptions)
include(GeneralCMakeConfig)
include(GeneralCompilerConfig)
include(LinkerConfig)
include(ThirdPartyConfig)
include(NebulaCustomTargets)
include(IncludeWhatYouUse)

add_custom_target(
        clang-format
        COMMAND "find" "src/" "tests/" "-type" "f" "\\(" "-iname" "\\*.h" "-o" "-iname" "\\*.cpp" "\\)" "|" "xargs" "clang-format" "-i"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# alias
add_custom_target(
        format
        DEPENDS clang-format
)

include_directories(AFTER ${CMAKE_SOURCE_DIR}/include)
include_directories(AFTER ${CMAKE_SOURCE_DIR}/src)

list(INSERT CMAKE_LIBRARY_PATH 0 ${CMAKE_SOURCE_DIR}/lib)
list(INSERT CMAKE_LIBRARY_PATH 0 ${CMAKE_SOURCE_DIR}/3rd)

print_config(CMAKE_INCLUDE_PATH)
print_config(CMAKE_LIBRARY_PATH)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L ${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -L ${CMAKE_SOURCE_DIR}/3rd")

# For simplicity, we make all ordinary libraries depend on the compile-time generated files,
# including the precompiled header, a.k.a Base.h.gch, and thrift headers.
macro(nebula_add_library name type)
    add_library(${name} ${type} ${ARGN})
    # gflags defined in shared library triggers bug since clang-12
    # -fPIC is a workaround
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options(${name} PRIVATE -fPIC)
    endif ()
endmacro()

# A wrapper for target_link_libraries()
macro(nebula_link_libraries target)
    target_link_libraries(
            ${target}
            ${ARGN}
            ${JEMALLOC_LIB}
            pthread
            ${COVERAGES}
    )
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_link_libraries(${target} atomic)
    endif ()
endmacro(nebula_link_libraries)

function(nebula_add_subdirectory dir_name)
    if (NOT ENABLE_TESTING)
        set(IGNORE_DIRS "test" "benchmark" "example" "duckdb")
        if (${dir_name} IN_LIST IGNORE_DIRS)
            add_subdirectory(${dir_name} EXCLUDE_FROM_ALL)
            return()
        endif ()
    endif ()
    add_subdirectory(${dir_name})
endfunction()

macro(nebula_add_solib)
    cmake_parse_arguments(
            solib
            "NO_TOPO;NO_INSTALL"
            "NAME"
            "SOURCES;OBJECTS;LIBRARIES"
            ${ARGN}
    )

    if(${solib_NAME} MATCHES ".*[A-Z_].*")
        message(FATAL_ERROR "${solib_NAME}: not a qualified library name, please follow '[a-z0-9-]+'")
    endif()

    add_library(${solib_NAME} SHARED ${solib_SOURCES} ${solib_OBJECTS})
    nebula_link_libraries(${solib_NAME} ${solib_LIBRARIES})
    set_target_properties(
            ${solib_NAME}
            PROPERTIES
            PREFIX ""
            LINK_FLAGS "-Wl,-rpath=\$ORIGIN:\$ORIGIN/../3rd"
    )
    if(ENABLE_ASAN)
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            # target_link_libraries(${solib_NAME} asan)
            target_link_options(${solib_NAME} PRIVATE -shared-libasan)
        endif()
    endif()
    target_link_options(${solib_NAME} PRIVATE -Wl,--no-undefined)
    # Install the target on `make install'
    if(NOT solib_NO_INSTALL)
        install(
                TARGETS ${solib_NAME}
                PERMISSIONS
                OWNER_EXECUTE OWNER_WRITE OWNER_READ
                GROUP_EXECUTE GROUP_READ
                WORLD_EXECUTE WORLD_READ
                DESTINATION lib
        )
    endif()
    # Add the target to the dependency topology
    if(NOT solib_NO_TOPO)
        set_property(GLOBAL APPEND PROPERTY NEBULA_TARGET_LIST ${solib_NAME})
    endif()
endmacro()

nebula_add_subdirectory(src)
