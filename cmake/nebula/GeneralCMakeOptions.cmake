############ Nebula defined options immutable during configure stage #############
option(ENABLE_TESTING                   "Build unit tests" ON)
option(ENABLE_CCACHE                    "Use ccache to speed up compiling" ON)
option(ENABLE_WERROR                    "Regard warnings as errors" ON)
option(ENABLE_JEMALLOC                  "Use jemalloc as memory allocator" ON)
option(ENABLE_ASAN                      "Build with AddressSanitizer" OFF)
option(ENABLE_FUZZY_TESTING             "Enable Fuzzy tests" OFF)
option(ENABLE_FRAME_POINTER             "Build with frame pointer" OFF)
option(ENABLE_STRICT_ALIASING           "Build with -fstrict-aliasing" OFF)
option(ENABLE_COVERAGE                  "Build with coverage report" OFF)
option(ENABLE_COMPRESSED_DEBUG_INFO     "Compress debug info to reduce binary size" ON)
option(ENABLE_CLANG_TIDY                "Enable clang-tidy if present" OFF)
option(DISABLE_CXX11_ABI                "Whether to disable cxx11 abi" OFF)

# Define a `NEBULA_DEBUG_PREFIX' option,
# to make the file path of the source code in debug information relative.
# There are several benefits:
#   1. Hide verbose filesystem infos from the end users when packaging.
#   2. Make the source file path short and clean while debugging
#   3. Easier to add the corresponding source diretory to the debug session
#
# This option has three available options:
#   1. NONE, the default one, to use the absolute path as usual
#   2. ROOT, relative to the root of the source tree, suitable for packaging
#   3. BUILD, relative to the build directory, suitable for developing environment
#
# This option is initialized from the environment variable `NEBULA_DEBUG_PREFIX'
# and could be overriden at cmake command line.
set(default_prefix "$ENV{NEBULA_DEBUG_PREFIX}")
if("${default_prefix}" STREQUAL "")
    set(default_prefix "NONE")
endif()
set(NEBULA_DEBUG_PREFIX_DOC "Make the source file prefix relative, options: NONE ROOT BUILD")
set(NEBULA_DEBUG_PREFIX "${default_prefix}" CACHE STRING "${NEBULA_DEBUG_PREFIX_DOC}")
if(NOT "${NEBULA_DEBUG_PREFIX}" STREQUAL "NONE" AND
        NOT "${NEBULA_DEBUG_PREFIX}" STREQUAL "ROOT" AND
        NOT "${NEBULA_DEBUG_PREFIX}" STREQUAL "BUILD")
    message(FATAL_ERROR "Unknown option for NEBULA_DEBUG_PREFIX: '${NEBULA_DEBUG_PREFIX}'")
endif()

get_cmake_property(variable_list VARIABLES)
foreach(_varname ${variable_list})
    string(REGEX MATCH "^ENABLE" matched ${_varname})
    if(matched)
        print_option(${_varname})
    endif()
endforeach()

print_config(NEBULA_DEBUG_PREFIX)
