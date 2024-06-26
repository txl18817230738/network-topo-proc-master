# The precedence to decide NEBULA_THIRDPARTY_ROOT is:
#   1. The path defined with CMake argument, i.e -DNEBULA_THIRDPARTY_ROOT=path
#   2. ${CMAKE_BINARY_DIR}/third-party/install, if exists
#   3. The path specified with environment variable NEBULA_THIRDPARTY_ROOT=path
#   4. /opt/vesoft/third-party, if exists
#   5. At last, one copy will be downloaded and installed to ${CMAKE_BINARY_DIR}/third-party/install

set(NEBULA_THIRDPARTY_VERSION "5.1")

if(${DISABLE_CXX11_ABI})
    SET(NEBULA_THIRDPARTY_ROOT ${CMAKE_BINARY_DIR}/third-party-98/install)
    if(NOT EXISTS ${CMAKE_BINARY_DIR}/third-party-98/install)
        message(STATUS "Install abi 98 third-party")
        include(InstallThirdParty)
    endif()
else()
    if("${NEBULA_THIRDPARTY_ROOT}" STREQUAL "")
        if(EXISTS ${CMAKE_BINARY_DIR}/third-party/install)
            SET(NEBULA_THIRDPARTY_ROOT ${CMAKE_BINARY_DIR}/third-party/install)
        elseif(NOT $ENV{NEBULA_THIRDPARTY_ROOT} STREQUAL "")
            SET(NEBULA_THIRDPARTY_ROOT $ENV{NEBULA_THIRDPARTY_ROOT})
        elseif(EXISTS /opt/vesoft/third-party/${NEBULA_THIRDPARTY_VERSION})
            SET(NEBULA_THIRDPARTY_ROOT "/opt/vesoft/third-party/${NEBULA_THIRDPARTY_VERSION}")
        else()
            include(InstallThirdParty)
        endif()
    endif()
endif()

if(NOT ${NEBULA_THIRDPARTY_ROOT} STREQUAL "")
    print_config(NEBULA_THIRDPARTY_ROOT)
    file(READ ${NEBULA_THIRDPARTY_ROOT}/version-info version_info)
    string(REGEX REPLACE "\n" "\n   " version_info ${version_info})
    string(REGEX REPLACE ": " "                : " version_info ${version_info})
    message(STATUS "Version of nebula third party   :\n   ${version_info}")
    list(INSERT CMAKE_INCLUDE_PATH 0 ${NEBULA_THIRDPARTY_ROOT}/include)
    list(INSERT CMAKE_LIBRARY_PATH 0 ${NEBULA_THIRDPARTY_ROOT}/lib)
    list(INSERT CMAKE_LIBRARY_PATH 0 ${NEBULA_THIRDPARTY_ROOT}/lib64)
    list(INSERT CMAKE_PROGRAM_PATH 0 ${NEBULA_THIRDPARTY_ROOT}/bin)
    list(INSERT CMAKE_MODULE_PATH  0 ${NEBULA_THIRDPARTY_ROOT}/lib/cmake)
    include_directories(SYSTEM ${NEBULA_THIRDPARTY_ROOT}/include)
    link_directories(
        ${NEBULA_THIRDPARTY_ROOT}/lib
        ${NEBULA_THIRDPARTY_ROOT}/lib64
    )
    #file(CREATE_LINK ${NEBULA_THIRDPARTY_ROOT}/lib ${CMAKE_BINARY_DIR}/3rd SYMBOLIC)
endif()

if(NOT ${NEBULA_OTHER_ROOT} STREQUAL "")
    string(REPLACE ":" ";" DIR_LIST ${NEBULA_OTHER_ROOT})
    list(LENGTH DIR_LIST len)
    foreach(DIR IN LISTS DIR_LIST )
        list(INSERT CMAKE_INCLUDE_PATH 0 ${DIR}/include)
        list(INSERT CMAKE_LIBRARY_PATH 0 ${DIR}/lib)
        list(INSERT CMAKE_PROGRAM_PATH 0 ${DIR}/bin)
        include_directories(SYSTEM ${DIR}/include)
        link_directories(${DIR}/lib)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L ${DIR}/lib")
    endforeach()
endif()

print_config(CMAKE_INCLUDE_PATH)
print_config(CMAKE_LIBRARY_PATH)
print_config(CMAKE_PROGRAM_PATH)

execute_process(
    COMMAND ldd --version
    COMMAND head -1
    COMMAND cut -d ")" -f 2
    COMMAND cut -d " " -f 2
    OUTPUT_VARIABLE GLIBC_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
print_config(GLIBC_VERSION)

macro(nebula_setup_imported_library)
    cmake_parse_arguments(
        imported
        "NO_TOPO"
        "NAME;PATH"
        "LIBRARIES"
        ${ARGN}
    )
    execute_process(
        COMMAND readelf -d ${imported_PATH}
        COMMAND grep SONAME
        COMMAND sed "s/.*\\[\\(.*\\)\\]/\\1/"
        OUTPUT_VARIABLE soname
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    # Setup location, SONAME and dependencies
    set_target_properties(
        ${imported_NAME}
        PROPERTIES
            IMPORTED_LOCATION ${imported_PATH}
            IMPORTED_SONAME ${soname}
            INTERFACE_LINK_LIBRARIES "${imported_LIBRARIES}"
    )
    # Copy the shared library and symlinks to the build directory
    # NOTE: FOLLOW_SYMLINK_CHAIN requires CMake 3.15+
    get_filename_component(dir ${imported_PATH} DIRECTORY)
    file(COPY ${dir}/${soname} DESTINATION ${CMAKE_BINARY_DIR}/3rd FOLLOW_SYMLINK_CHAIN)
    if(NOT imported_NO_TOPO)
        # Collect this library to the global target list
        set_property(GLOBAL APPEND PROPERTY NEBULA_TARGET_LIST ${imported_NAME})
    endif()
endmacro()

message("")

find_package(Boost)
find_package(Fmt)
find_package(SSL)
find_package(Event)
find_package(Ev)
find_package(Glog)
find_package(Gflags)
find_package(Unwind)
find_package(DoubleConversion)
find_package(Dwarf)
find_package(Sodium)
find_package(BZ2)
find_package(LZ4)
find_package(Zlib)
find_package(LZMA)
find_package(Snappy)
find_package(Zstd)
find_package(Date)
find_package(Folly)
find_package(RocksDB)
find_package(Proxygen)
find_package(Wangle)
find_package(Fizz)
find_package(Jemalloc)
find_package(Gtest)
# find_package(Ldap)
# find_package(Sasl)
find_package(Utf8proc)
find_package(Arrow)
find_package(Absl)
find_package(Curl)
find_package(Benchmark)
find_package(LLVMALT REQUIRED)
find_package(FLEX REQUIRED)
find_package(BISON 3.0.5 REQUIRED)
include(MakeBisonRelocatable)
find_package(Protobuf)
find_package(RE2)
find_package(CARES)
find_package(GRPC)

install(DIRECTORY ${CMAKE_BINARY_DIR}/3rd/ DESTINATION 3rd
    PATTERN "libgtest*" EXCLUDE
    PATTERN "libgmock*" EXCLUDE
    PATTERN "libbenchmark*" EXCLUDE
    PATTERN "libarrow*" EXCLUDE
    PATTERN "libgandiva*" EXCLUDE
)

# Copy std libs
file(COPY ${LIB_STDCXX_DIR}/${LIB_STDCXX_SONAME} DESTINATION ${CMAKE_BINARY_DIR}/3rd FOLLOW_SYMLINK_CHAIN)
file(COPY ${LIB_GCC_DIR}/${LIB_GCC_SONAME} DESTINATION ${CMAKE_BINARY_DIR}/3rd FOLLOW_SYMLINK_CHAIN)
file(COPY ${LIB_ATOMIC_DIR}/${LIB_ATOMIC_SONAME} DESTINATION ${CMAKE_BINARY_DIR}/3rd FOLLOW_SYMLINK_CHAIN)
if(ENABLE_ASAN)
    file(COPY ${LIB_ASAN_DIR}/${LIB_ASAN_SONAME} DESTINATION ${CMAKE_BINARY_DIR}/3rd FOLLOW_SYMLINK_CHAIN)
endif()

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L ${NEBULA_THIRDPARTY_ROOT}/lib")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L ${NEBULA_THIRDPARTY_ROOT}/lib64")

# All duckdb libraries
set(DUCKDB_LIBRARIES
    duckdb_static
    duckdb_utf8proc
    duckdb_fastpforlib
    duckdb_re2
    duckdb_miniz
    duckdb_fmt
    duckdb_hyperloglog
    duckdb_pg_query
    duckdb_fsst
    duckdb_mbedtls
    parquet_extension
    jemalloc_extension
)

if (NOT ENABLE_JEMALLOC OR ENABLE_ASAN)
    set(JEMALLOC_LIB )
else()
    set(JEMALLOC_LIB Jemalloc)
    add_definitions(-DENABLE_JEMALLOC)
endif()
