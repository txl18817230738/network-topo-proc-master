# Copyright (c) 2023 vesoft inc. All rights reserved.

set(LLVM_DIR ${NEBULA_THIRDPARTY_ROOT}/lib/cmake/llvm )
find_package(LLVM 15.0.7 REQUIRED)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

llvm_map_components_to_libnames(llvm_libs
                                  codegen
                                  core
                                  mcjit
                                  native
                                  ipo
                                  bitreader
                                  target
                                  linker
                                  analysis
                                  debuginfodwarf
        )
