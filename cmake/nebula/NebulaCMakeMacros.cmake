macro(nebula_add_executable)
    cmake_parse_arguments(
        nebula_exec                 # prefix
        "DEPENDS_ON_PLUGIN"         # <options>
        "NAME;OUTPUT"               # <one_value_args>
        "SOURCES;OBJECTS;LIBRARIES" # <multi_value_args>
        ${ARGN}
    )
    add_executable(
        ${nebula_exec_NAME}
        ${nebula_exec_SOURCES}
        ${nebula_exec_OBJECTS}
    )
    # Add dependencies to all plugins
    if(nebula_exec_DEPENDS_ON_PLUGIN)
        add_dependencies(${nebula_exec_NAME} nebula-plugins)
    endif()
    # Append the ASan options object to the source list
    if(ENABLE_ASAN)
        target_sources(
            ${nebula_exec_NAME}
            PRIVATE $<TARGET_OBJECTS:asan-options-object>
        )
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            target_link_options(${nebula_exec_NAME} PRIVATE -shared-libasan)
        endif()
    endif()
    # gflags defined in shared library triggers bug in clang since versin 12 and fixed in 14.
    # -fPIC is a workaround
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_compile_options(${nebula_exec_NAME} PRIVATE -fPIC)
    endif()

    # The version library contains some version info that
    # our extension scripts for the GDB debugger rely on.
    # So we link every executable with this library explicitly.
    # Note that here we assume that the `--as-needed` linker option is set by default.
    target_link_libraries(${nebula_exec_NAME} -Wl,--no-as-needed version -Wl,--as-needed)

    nebula_link_libraries(
        ${nebula_exec_NAME}
        ${nebula_exec_LIBRARIES}
    )

    if(${nebula_exec_NAME} MATCHES "_test$")
        set_target_properties(
            ${nebula_exec_NAME}
            PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/test
        )
    elseif(${nebula_exec_NAME} MATCHES "_bm$")
        set_target_properties(
            ${nebula_exec_NAME}
            PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/bench
        )
    endif()
    set_target_properties(
        ${nebula_exec_NAME}
        PROPERTIES LINK_FLAGS "-Wl,-rpath=\$ORIGIN/../lib:\$ORIGIN/../3rd")
    if(NOT ${nebula_exec_OUTPUT} STREQUAL "")
        set_target_properties(
            ${nebula_exec_NAME}
            PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${nebula_exec_OUTPUT}
        )
    endif()
    add_custom_command(
        TARGET ${nebula_exec_NAME} POST_BUILD
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/check-dependencies.sh ${CMAKE_BINARY_DIR} $<TARGET_FILE:${nebula_exec_NAME}>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
endmacro()

macro(nebula_add_plugin)
    cmake_parse_arguments(
        plugin
        ""
        "NAME"
        "SOURCES;OBJECTS;LIBRARIES"
        ${ARGN}
    )
    add_library(${plugin_NAME} SHARED ${plugin_SOURCES} ${plugin_OBJECTS})

    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        # disable STB_GNU_UNIQUE option to avoid un-closeable shared library(can not unload and reload will not take effect(the first load version will be remained))
        target_compile_options (${plugin_NAME} PRIVATE --no-gnu-unique)
    endif()

    nebula_link_libraries(${plugin_NAME} plugin module ${plugin_LIBRARIES})
    set_target_properties(
        ${plugin_NAME}
        PROPERTIES
            PREFIX ""
            LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
            LINK_FLAGS "-Wl,-rpath=\$ORIGIN/../lib:\$ORIGIN/../3rd"
    )
    target_link_options(${plugin_NAME} PRIVATE -Wl,--no-undefined)

    if(ENABLE_ASAN)
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            target_link_options(${plugin_NAME} PRIVATE -shared-libasan)
        endif()
    endif()

    # Some executables depend on plugins implicitly
    # So we add an INTERFACE library to allow those executables
    # to declare the dependency explicitly.
    if(NOT TARGET nebula-plugins)
        add_library(nebula-plugins INTERFACE)
    endif()
    add_dependencies(nebula-plugins ${plugin_NAME})

    install(
        TARGETS
            ${plugin_NAME}
        PERMISSIONS
            OWNER_EXECUTE OWNER_WRITE OWNER_READ
            GROUP_EXECUTE GROUP_READ
            WORLD_EXECUTE WORLD_READ
        DESTINATION
            plugins
    )
endmacro()

macro(nebula_add_test)
    cmake_parse_arguments(
        nebula_test                             # prefix
        "DISABLED;FUZZER;DEPENDS_ON_PLUGIN"     # <options>
        "NAME"                                  # <one_value_args>
        "SOURCES;OBJECTS;LIBRARIES;LABELS" # <multi_value_args>
        ${ARGN}
    )

    if(nebula_test_DEPENDS_ON_PLUGIN)
        nebula_add_executable(
            NAME ${nebula_test_NAME}
            SOURCES ${nebula_test_SOURCES}
            OBJECTS ${nebula_test_OBJECTS}
            LIBRARIES ${nebula_test_LIBRARIES}
            DEPENDS_ON_PLUGIN
        )
    else()
        nebula_add_executable(
            NAME ${nebula_test_NAME}
            SOURCES ${nebula_test_SOURCES}
            OBJECTS ${nebula_test_OBJECTS}
            LIBRARIES ${nebula_test_LIBRARIES}
        )
    endif()

    if (${nebula_test_FUZZER})
        #Currently only Clang supports fuzz test
        if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
            set_target_properties(${nebula_test_NAME} PROPERTIES COMPILE_FLAGS "-g -fsanitize=fuzzer")
            set_target_properties(${nebula_test_NAME} PROPERTIES LINK_FLAGS "-fsanitize=fuzzer")
        endif()
    elseif (NOT ${nebula_test_DISABLED})
        string(REGEX REPLACE "${CMAKE_SOURCE_DIR}/src/(.*)/test" "\\1" test_group ${CMAKE_CURRENT_SOURCE_DIR})
        add_test(NAME ${nebula_test_NAME} COMMAND ${nebula_test_NAME})
        if (NOT "${nebula_test_LABELS}" STREQUAL "")
            set(test_group "${test_group} ${nebula_test_LABELS}")
        endif()
        set_tests_properties(${nebula_test_NAME} PROPERTIES LABELS ${test_group})
        # e.g. cmake -DNEBULA_ASAN_PRELOAD=/path/to/libasan.so
        # or,  cmake -DNEBULA_ASAN_PRELOAD=`/path/to/gcc --print-file-name=libasan.so`
        if (NEBULA_ASAN_PRELOAD)
            set_property(
                TEST ${nebula_test_NAME}
                PROPERTY ENVIRONMENT LD_PRELOAD=${NEBULA_ASAN_PRELOAD}
            )
        endif()
    endif()
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
    if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_link_libraries(${target} atomic)
    endif()
endmacro(nebula_link_libraries)

function(nebula_add_subdirectory dir_name)
    if (NOT ENABLE_TESTING)
      set(IGNORE_DIRS "test" "benchmark" "example" "duckdb")
      if (${dir_name} IN_LIST IGNORE_DIRS)
        add_subdirectory(${dir_name} EXCLUDE_FROM_ALL)
        return()
      endif()
    endif()
    add_subdirectory(${dir_name})
endfunction()

macro(nebula_add_exe_linker_flag flag)
    string(FIND "${CMAKE_EXE_LINKER_FLAGS}" "${flag}" position)
    if(${position} EQUAL -1)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flag}")
    endif()
endmacro()

macro(nebula_remove_exe_linker_flag flag)
    string(REPLACE "${flag}" "" output "${CMAKE_EXE_LINKER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${output}")
endmacro()

macro(nebula_add_shared_linker_flag flag)
    string(FIND "${CMAKE_SHARED_LINKER_FLAGS}" "${flag}" position)
    if(${position} EQUAL -1)
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${flag}")
    endif()
endmacro()

macro(nebula_remove_shared_linker_flag flag)
    string(REPLACE "${flag}" "" output "${CMAKE_SHARED_LINKER_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${output}")
endmacro()

function(nebula_string_rpad output width fill_char value)
    string(LENGTH ${value} length)
    math(EXPR delta "${width} - ${length}")
    set(pads "")
    if(${delta} GREATER 0)
        math(EXPR delta "${delta} - 1")
        foreach(loop_var RANGE ${delta})
            set(pads "${pads}${fill_char}")
        endforeach()
    endif()
    set(${output} "${value}${pads}" PARENT_SCOPE)
endfunction()

macro(print_option name)
    get_property(helpstring CACHE ${name} PROPERTY HELPSTRING)
    nebula_string_rpad(padded 32 " " "${name}")
    string(FIND "${helpstring}" "No help" position)
    if(helpstring AND ${position} EQUAL -1)
        message(STATUS "${padded}: ${${name}} (${helpstring})")
    else()
        message(STATUS "${padded}: ${${name}}")
    endif()
endmacro()

macro(print_config name)
    print_option(${name})
endmacro()
