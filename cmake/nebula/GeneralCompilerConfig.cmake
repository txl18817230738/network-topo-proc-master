set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

print_config(CMAKE_CXX_STANDARD)
print_config(CMAKE_CXX_COMPILER)
print_config(CMAKE_CXX_COMPILER_ID)

if (!CMAKE_CXX_COMPILER)
    message(FATAL_ERROR "No C++ compiler found")
endif()

include(CheckCXXCompilerFlag)

add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Wpedantic)
add_compile_options(-Wunused-parameter)
add_compile_options(-Wshadow)
add_compile_options(-Wnon-virtual-dtor)
add_compile_options(-Woverloaded-virtual)
add_compile_options(-Wignored-qualifiers)
add_compile_options(-fvisibility-inlines-hidden)
add_compile_options(-fno-semantic-interposition)

if("${NEBULA_DEBUG_PREFIX}" STREQUAL "ROOT")
    add_compile_options(-fdebug-prefix-map=${CMAKE_SOURCE_DIR}/=)
elseif("${NEBULA_DEBUG_PREFIX}" STREQUAL "BUILD")
    execute_process(
        COMMAND realpath --relative-to=${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE relative_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    add_compile_options(-fdebug-prefix-map=${CMAKE_SOURCE_DIR}/=${relative_path}/)
endif()

# libev
add_definitions(-DEV_MULTIPLICITY=1)

include_directories(AFTER ${CMAKE_SOURCE_DIR}/src)
include_directories(AFTER ${CMAKE_CURRENT_BINARY_DIR}/src)


if(ENABLE_WERROR)
    add_compile_options(-Werror)
    add_compile_options(-Wno-attributes)
endif()

if(NOT ENABLE_STRICT_ALIASING)
    add_compile_options(-fno-strict-aliasing)
else()
    add_compile_options(-fstrict-aliasing)
endif()

if(ENABLE_FRAME_POINTER)
    add_compile_options(-fno-omit-frame-pointer)
else()
    add_compile_options(-fomit-frame-pointer)
endif()

if(ENABLE_COVERAGE)
    add_compile_options(--coverage)
    add_compile_options(-g)
    add_compile_options(-O0)
    add_compile_options(-fno-inline -fno-inline-small-functions -fno-default-inline -fprofile-arcs -ftest-coverage -fno-elide-constructors)
    nebula_add_exe_linker_flag(--coverage)
    nebula_add_shared_linker_flag(--coverage)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-ggdb3)
endif()

# TODO(doodle) Add option suggest-override for gnu
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Here we need to specify the path of libstdc++ used by Clang
    if(NOT ${NEBULA_CLANG_USE_GCC_TOOLCHAIN} STREQUAL "")
        print_config(NEBULA_CLANG_USE_GCC_TOOLCHAIN)
        execute_process(
            COMMAND ${NEBULA_CLANG_USE_GCC_TOOLCHAIN}/bin/gcc -dumpmachine
            OUTPUT_VARIABLE gcc_target_triplet
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        add_compile_options("--gcc-toolchain=${NEBULA_CLANG_USE_GCC_TOOLCHAIN}")
        nebula_add_exe_linker_flag(--gcc-toolchain=${NEBULA_CLANG_USE_GCC_TOOLCHAIN})
        nebula_add_shared_linker_flag(--gcc-toolchain=${NEBULA_CLANG_USE_GCC_TOOLCHAIN})
        if(NOT "${gcc_target_triplet}" STREQUAL "")
            add_compile_options(--target=${gcc_target_triplet})
            nebula_add_exe_linker_flag(--target=${gcc_target_triplet})
            nebula_add_shared_linker_flag(--target=${gcc_target_triplet})
        endif()
    endif()
#add_compile_options(-Wno-mismatched-tags)
    add_compile_options(-Wno-self-assign-overloaded)
    add_compile_options(-Wno-self-move)
    add_compile_options(-Wno-format-pedantic)
    add_compile_options(-Wno-gnu-zero-variadic-macro-arguments)
    add_compile_options(-Wno-return-type-c-linkage)
endif()

# Retrieve path and soname of libstdc++
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=libstdc++.so
    OUTPUT_VARIABLE libstdcxx_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND readelf -d ${libstdcxx_path}
    COMMAND grep SONAME
    COMMAND sed "s/.*\\[\\(.*\\)\\]/\\1/"
    OUTPUT_VARIABLE LIB_STDCXX_SONAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=${LIB_STDCXX_SONAME}
    OUTPUT_VARIABLE libstdcxx_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
get_filename_component(LIB_STDCXX_DIR ${libstdcxx_path} DIRECTORY CACHE)

# Retrieve path and soname of libgcc
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=libgcc_s.so.1
    OUTPUT_VARIABLE libgcc_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND readelf -d ${libgcc_path}
    COMMAND grep SONAME
    COMMAND sed "s/.*\\[\\(.*\\)\\]/\\1/"
    OUTPUT_VARIABLE LIB_GCC_SONAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=${LIB_GCC_SONAME}
    OUTPUT_VARIABLE libgcc_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
get_filename_component(LIB_GCC_DIR ${libgcc_path} DIRECTORY CACHE)

# Retrieve path and soname of libatomic
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=libatomic.so
    OUTPUT_VARIABLE libatomic_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND readelf -d ${libatomic_path}
    COMMAND grep SONAME
    COMMAND sed "s/.*\\[\\(.*\\)\\]/\\1/"
    OUTPUT_VARIABLE LIB_ATOMIC_SONAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=${LIB_ATOMIC_SONAME}
    OUTPUT_VARIABLE libatomic_path
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
get_filename_component(LIB_ATOMIC_DIR ${libatomic_path} DIRECTORY CACHE)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # Retrieve path and soname of libasan
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=libasan.so
        OUTPUT_VARIABLE libasan_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND readelf -d ${libasan_path}
        COMMAND grep SONAME
        COMMAND sed "s/.*\\[\\(.*\\)\\]/\\1/"
        OUTPUT_VARIABLE LIB_ASAN_SONAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=${LIB_ASAN_SONAME}
        OUTPUT_VARIABLE libasan_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    get_filename_component(LIB_ASAN_DIR ${libasan_path} DIRECTORY CACHE)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    # Retrieve path and soname of libclang_rt.asan
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=libclang_rt.asan-x86_64.so
        OUTPUT_VARIABLE libasan_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND readelf -d ${libasan_path}
        COMMAND grep SONAME
        COMMAND sed "s/.*\\[\\(.*\\)\\]/\\1/"
        OUTPUT_VARIABLE LIB_ASAN_SONAME
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=${LIB_ASAN_SONAME}
        OUTPUT_VARIABLE libasan_path
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    get_filename_component(LIB_ASAN_DIR ${libasan_path} DIRECTORY CACHE)
endif()
