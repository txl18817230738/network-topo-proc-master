# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Benchmark_LIB NAMES libbenchmark.so DOC "Google Benchmark library")
find_library(BenchmarkMain_LIB NAMES libbenchmark_main.so DOC "Google Benchmark main library")
if(NOT Benchmark_LIB)
        message(FATAL_ERROR "libbenchmark not found")
endif()
if(NOT BenchmarkMain_LIB)
        message(FATAL_ERROR "libbenchmark_main not found")
endif()

add_library(Benchmark SHARED IMPORTED GLOBAL)
add_library(Benchmark::main SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Benchmark PATH ${Benchmark_LIB} LIBRARIES m)
nebula_setup_imported_library(NAME Benchmark::main PATH ${BenchmarkMain_LIB} LIBRARIES Benchmark)
