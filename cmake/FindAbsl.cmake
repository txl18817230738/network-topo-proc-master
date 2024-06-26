# Absl::bad_optional_access
find_library(bad_optional_access_LIB NAMES libabsl_bad_optional_access.so)
if(NOT bad_optional_access_LIB)
    message(FATAL_ERROR "libabsl_bad_optional_access.so not found")
endif()
add_library(Absl::bad_optional_access SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::bad_optional_access
    PATH ${bad_optional_access_LIB}
    LIBRARIES
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::bad_variant_access
find_library(bad_variant_access_LIB NAMES libabsl_bad_variant_access.so)
if(NOT bad_variant_access_LIB)
    message(FATAL_ERROR "libabsl_bad_variant_access.so not found")
endif()
add_library(Absl::bad_variant_access SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::bad_variant_access
    PATH ${bad_variant_access_LIB}
    LIBRARIES
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::base
find_library(base_LIB NAMES libabsl_base.so)
if(NOT base_LIB)
    message(FATAL_ERROR "libabsl_base.so not found")
endif()
add_library(Absl::base SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::base
    PATH ${base_LIB}
    LIBRARIES
        Absl::raw_logging_internal
        Absl::spinlock_wait
        Absl::log_severity
)

# Absl::city
find_library(city_LIB NAMES libabsl_city.so)
if(NOT city_LIB)
    message(FATAL_ERROR "libabsl_city.so not found")
endif()
add_library(Absl::city SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::city
    PATH ${city_LIB}
    LIBRARIES
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::civil_time
find_library(civil_time_LIB NAMES libabsl_civil_time.so)
if(NOT civil_time_LIB)
    message(FATAL_ERROR "libabsl_civil_time.so not found")
endif()
add_library(Absl::civil_time SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::civil_time PATH ${civil_time_LIB})

# Absl::cord_internal
find_library(cord_internal_LIB NAMES libabsl_cord_internal.so)
if(NOT cord_internal_LIB)
    message(FATAL_ERROR "libabsl_cord_internal.so not found")
endif()
add_library(Absl::cord_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::cord_internal
    PATH ${cord_internal_LIB}
    LIBRARIES
        Absl::crc_cord_state
        Absl::crc32c
        Absl::crc_internal
        Absl::crc_cpu_detect
        Absl::str_format_internal
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::cord
find_library(cord_LIB NAMES libabsl_cord.so)
if(NOT cord_LIB)
    message(FATAL_ERROR "libabsl_cord.so not found")
endif()
add_library(Absl::cord SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::cord
    PATH ${cord_LIB}
    LIBRARIES
        Absl::cordz_info
        Absl::cord_internal
        Absl::crc_cord_state
        Absl::crc32c
        Absl::crc_internal
        Absl::crc_cpu_detect
        Absl::str_format_internal
        Absl::cordz_functions
        Absl::exponential_biased
        Absl::cordz_handle
        Absl::synchronization
        Absl::stacktrace
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::symbolize
        Absl::debugging_internal
        Absl::malloc_internal
        Absl::demangle_internal
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::civil_time
        Absl::time_zone
        Absl::throw_delegate
        Absl::bad_optional_access
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::cordz_functions
find_library(cordz_functions_LIB NAMES libabsl_cordz_functions.so)
if(NOT cordz_functions_LIB)
    message(FATAL_ERROR "libabsl_cordz_functions.so not found")
endif()
add_library(Absl::cordz_functions SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::cordz_functions
    PATH ${cordz_functions_LIB}
    LIBRARIES
        Absl::exponential_biased
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::cordz_handle
find_library(cordz_handle_LIB NAMES libabsl_cordz_handle.so)
if(NOT cordz_handle_LIB)
    message(FATAL_ERROR "libabsl_cordz_handle.so not found")
endif()
add_library(Absl::cordz_handle SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::cordz_handle
    PATH ${cordz_handle_LIB}
    LIBRARIES
        Absl::synchronization
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::stacktrace
        Absl::symbolize
        Absl::malloc_internal
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::civil_time
        Absl::int128
        Absl::time_zone
)

# Absl::cordz_info
find_library(cordz_info_LIB NAMES libabsl_cordz_info.so)
if(NOT cordz_info_LIB)
    message(FATAL_ERROR "libabsl_cordz_info.so not found")
endif()
add_library(Absl::cordz_info SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::cordz_info
    PATH ${cordz_info_LIB}
    LIBRARIES
        Absl::cord_internal
        Absl::cordz_functions
        Absl::cordz_handle
        Absl::synchronization
        Absl::crc_cord_state
        Absl::crc32c
        Absl::crc_internal
        Absl::crc_cpu_detect
        Absl::str_format_internal
        Absl::exponential_biased
        Absl::stacktrace
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::time
        Absl::civil_time
        Absl::time_zone
        Absl::symbolize
        Absl::strings
        Absl::int128
        Absl::string_view
        Absl::throw_delegate
        Absl::strings_internal
        Absl::debugging_internal
        Absl::malloc_internal
        Absl::demangle_internal
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::crc32c
find_library(crc32c_LIB NAMES libabsl_crc32c.so)
if(NOT crc32c_LIB)
    message(FATAL_ERROR "libabsl_crc32c.so not found")
endif()
add_library(Absl::crc32c SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::crc32c
    PATH ${crc32c_LIB}
    LIBRARIES
        Absl::crc_internal
        Absl::crc_cpu_detect
        Absl::str_format_internal
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::crc_cord_state
find_library(crc_cord_state_LIB NAMES libabsl_crc_cord_state.so)
if(NOT crc_cord_state_LIB)
    message(FATAL_ERROR "libabsl_crc_cord_state.so not found")
endif()
add_library(Absl::crc_cord_state SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::crc_cord_state
    PATH ${crc_cord_state_LIB}
    LIBRARIES
        Absl::crc32c
        Absl::crc_internal
        Absl::crc_cpu_detect
        Absl::str_format_internal
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::crc_cpu_detect
find_library(crc_cpu_detect_LIB NAMES libabsl_crc_cpu_detect.so)
if(NOT crc_cpu_detect_LIB)
    message(FATAL_ERROR "libabsl_crc_cpu_detect.so not found")
endif()
add_library(Absl::crc_cpu_detect SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::crc_cpu_detect
    PATH ${crc_cpu_detect_LIB}
    LIBRARIES
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::crc_internal
find_library(crc_internal_LIB NAMES libabsl_crc_internal.so)
if(NOT crc_internal_LIB)
    message(FATAL_ERROR "libabsl_crc_internal.so not found")
endif()
add_library(Absl::crc_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::crc_internal
    PATH ${crc_internal_LIB}
    LIBRARIES
        Absl::crc_cpu_detect
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::debugging_internal
find_library(debugging_internal_LIB NAMES libabsl_debugging_internal.so)
if(NOT debugging_internal_LIB)
    message(FATAL_ERROR "libabsl_debugging_internal.so not found")
endif()
add_library(Absl::debugging_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::debugging_internal
    PATH ${debugging_internal_LIB}
    LIBRARIES
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::demangle_internal
find_library(demangle_internal_LIB NAMES libabsl_demangle_internal.so)
if(NOT demangle_internal_LIB)
    message(FATAL_ERROR "libabsl_demangle_internal.so not found")
endif()
add_library(Absl::demangle_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::demangle_internal
    PATH ${demangle_internal_LIB}
    LIBRARIES
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::die_if_null
find_library(die_if_null_LIB NAMES libabsl_die_if_null.so)
if(NOT die_if_null_LIB)
    message(FATAL_ERROR "libabsl_die_if_null.so not found")
endif()
add_library(Absl::die_if_null SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::die_if_null
    PATH ${die_if_null_LIB}
    LIBRARIES
        Absl::log_internal_conditions
        Absl::log_internal_message
        Absl::examine_stack
        Absl::log_internal_format
        Absl::str_format_internal
        Absl::log_internal_proto
        Absl::log_internal_nullguard
        Absl::log_internal_log_sink_set
        Absl::log_internal_globals
        Absl::log_globals
        Absl::hash
        Absl::city
        Absl::bad_optional_access
        Absl::bad_variant_access
        Absl::low_level_hash
        Absl::synchronization
        Absl::stacktrace
        Absl::symbolize
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::graphcycles_internal
        Absl::malloc_internal
        Absl::kernel_timeout_internal
        Absl::log_sink
        Absl::log_entry
        Absl::strerror
        Absl::time
        Absl::civil_time
        Absl::time_zone
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::examine_stack
find_library(examine_stack_LIB NAMES libabsl_examine_stack.so)
if(NOT examine_stack_LIB)
    message(FATAL_ERROR "libabsl_examine_stack.so not found")
endif()
add_library(Absl::examine_stack SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::examine_stack
    PATH ${examine_stack_LIB}
    LIBRARIES
        Absl::stacktrace
        Absl::symbolize
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::malloc_internal
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::exponential_biased
find_library(exponential_biased_LIB NAMES libabsl_exponential_biased.so)
if(NOT exponential_biased_LIB)
    message(FATAL_ERROR "libabsl_exponential_biased.so not found")
endif()
add_library(Absl::exponential_biased SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::exponential_biased PATH ${exponential_biased_LIB})

# Absl::flags_commandlineflag_internal
find_library(flags_commandlineflag_internal_LIB NAMES libabsl_flags_commandlineflag_internal.so)
if(NOT flags_commandlineflag_internal_LIB)
    message(FATAL_ERROR "libabsl_flags_commandlineflag_internal.so not found")
endif()
add_library(Absl::flags_commandlineflag_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::flags_commandlineflag_internal PATH ${flags_commandlineflag_internal_LIB})

# Absl::flags_commandlineflag
find_library(flags_commandlineflag_LIB NAMES libabsl_flags_commandlineflag.so)
if(NOT flags_commandlineflag_LIB)
    message(FATAL_ERROR "libabsl_flags_commandlineflag.so not found")
endif()
add_library(Absl::flags_commandlineflag SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::flags_commandlineflag
    PATH ${flags_commandlineflag_LIB}
    LIBRARIES
        Absl::flags_commandlineflag_internal
        Absl::strings
        Absl::bad_optional_access
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::flags_config
find_library(flags_config_LIB NAMES libabsl_flags_config.so)
if(NOT flags_config_LIB)
    message(FATAL_ERROR "libabsl_flags_config.so not found")
endif()
add_library(Absl::flags_config SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::flags_config
    PATH ${flags_config_LIB}
    LIBRARIES
        Absl::flags_program_name
        Absl::synchronization
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::stacktrace
        Absl::symbolize
        Absl::malloc_internal
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::throw_delegate
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::civil_time
        Absl::time_zone
)

# Absl::flags_internal
find_library(flags_internal_LIB NAMES libabsl_flags_internal.so)
if(NOT flags_internal_LIB)
    message(FATAL_ERROR "libabsl_flags_internal.so not found")
endif()
add_library(Absl::flags_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::flags_internal
    PATH ${flags_internal_LIB}
    LIBRARIES
        Absl::flags_commandlineflag
        Absl::flags_commandlineflag_internal
        Absl::flags_config
        Absl::flags_marshalling
        Absl::flags_program_name
        Absl::synchronization
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::stacktrace
        Absl::symbolize
        Absl::malloc_internal
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::time
        Absl::civil_time
        Absl::time_zone
        Absl::bad_optional_access
        Absl::str_format_internal
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::flags_marshalling
find_library(flags_marshalling_LIB NAMES libabsl_flags_marshalling.so)
if(NOT flags_marshalling_LIB)
    message(FATAL_ERROR "libabsl_flags_marshalling.so not found")
endif()
add_library(Absl::flags_marshalling SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::flags_marshalling
    PATH ${flags_marshalling_LIB}
    LIBRARIES
        Absl::bad_optional_access
        Absl::str_format_internal
        Absl::strings
        Absl::int128
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::flags_private_handle_accessor
find_library(flags_private_handle_accessor_LIB NAMES libabsl_flags_private_handle_accessor.so)
if(NOT flags_private_handle_accessor_LIB)
    message(FATAL_ERROR "libabsl_flags_private_handle_accessor.so not found")
endif()
add_library(Absl::flags_private_handle_accessor SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::flags_private_handle_accessor
    PATH ${flags_private_handle_accessor_LIB}
    LIBRARIES
        Absl::flags_commandlineflag
        Absl::flags_commandlineflag_internal
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::bad_optional_access
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::flags_program_name
find_library(flags_program_name_LIB NAMES libabsl_flags_program_name.so)
if(NOT flags_program_name_LIB)
    message(FATAL_ERROR "libabsl_flags_program_name.so not found")
endif()
add_library(Absl::flags_program_name SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::flags_program_name
    PATH ${flags_program_name_LIB}
    LIBRARIES
        Absl::synchronization
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::stacktrace
        Absl::symbolize
        Absl::malloc_internal
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::throw_delegate
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::civil_time
        Absl::time_zone
)

# Absl::flags_reflection
find_library(flags_reflection_LIB NAMES libabsl_flags_reflection.so)
if(NOT flags_reflection_LIB)
    message(FATAL_ERROR "libabsl_flags_reflection.so not found")
endif()
add_library(Absl::flags_reflection SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::flags_reflection
    PATH ${flags_reflection_LIB}
    LIBRARIES
        Absl::flags_private_handle_accessor
        Absl::flags_config
        Absl::flags_commandlineflag
        Absl::flags_commandlineflag_internal
        Absl::flags_program_name
        Absl::cord
        Absl::cordz_info
        Absl::cord_internal
        Absl::cordz_functions
        Absl::cordz_handle
        Absl::crc_cord_state
        Absl::crc32c
        Absl::crc_internal
        Absl::crc_cpu_detect
        Absl::str_format_internal
        Absl::raw_hash_set
        Absl::hash
        Absl::bad_optional_access
        Absl::city
        Absl::bad_variant_access
        Absl::low_level_hash
        Absl::hashtablez_sampler
        Absl::exponential_biased
        Absl::synchronization
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::stacktrace
        Absl::symbolize
        Absl::malloc_internal
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::throw_delegate
        Absl::int128
        Absl::civil_time
        Absl::time_zone
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::flags
find_library(flags_LIB NAMES libabsl_flags.so)
if(NOT flags_LIB)
    message(FATAL_ERROR "libabsl_flags.so not found")
endif()
add_library(Absl::flags SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::flags
    PATH ${flags_LIB}
    LIBRARIES
        Absl::flags_internal
        Absl::flags_reflection
        Absl::flags_marshalling
        Absl::flags_config
        Absl::flags_program_name
        Absl::flags_private_handle_accessor
        Absl::flags_commandlineflag
        Absl::flags_commandlineflag_internal
        Absl::cord
        Absl::cordz_info
        Absl::cord_internal
        Absl::cordz_functions
        Absl::cordz_handle
        Absl::crc_cord_state
        Absl::crc32c
        Absl::str_format_internal
        Absl::crc_internal
        Absl::crc_cpu_detect
        Absl::raw_hash_set
        Absl::hash
        Absl::bad_optional_access
        Absl::city
        Absl::bad_variant_access
        Absl::low_level_hash
        Absl::hashtablez_sampler
        Absl::exponential_biased
        Absl::synchronization
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::stacktrace
        Absl::symbolize
        Absl::malloc_internal
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::int128
        Absl::civil_time
        Absl::time_zone
)

# Absl::graphcycles_internal
find_library(graphcycles_internal_LIB NAMES libabsl_graphcycles_internal.so)
if(NOT graphcycles_internal_LIB)
    message(FATAL_ERROR "libabsl_graphcycles_internal.so not found")
endif()
add_library(Absl::graphcycles_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::graphcycles_internal
    PATH ${graphcycles_internal_LIB}
    LIBRARIES
        Absl::malloc_internal
        Absl::base
        Absl::spinlock_wait
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::hash
find_library(hash_LIB NAMES libabsl_hash.so)
if(NOT hash_LIB)
    message(FATAL_ERROR "libabsl_hash.so not found")
endif()
add_library(Absl::hash SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::hash
    PATH ${hash_LIB}
    LIBRARIES
        Absl::city
        Absl::strings
        Absl::low_level_hash
        Absl::string_view
        Absl::throw_delegate
        Absl::strings_internal
        Absl::bad_optional_access
        Absl::bad_variant_access
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
        Absl::int128
)

# Absl::hashtablez_sampler
find_library(hashtablez_sampler_LIB NAMES libabsl_hashtablez_sampler.so)
if(NOT hashtablez_sampler_LIB)
    message(FATAL_ERROR "libabsl_hashtablez_sampler.so not found")
endif()
add_library(Absl::hashtablez_sampler SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::hashtablez_sampler
    PATH ${hashtablez_sampler_LIB}
    LIBRARIES
        Absl::exponential_biased
        Absl::synchronization
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::time
        Absl::civil_time
        Absl::time_zone
        Absl::stacktrace
        Absl::symbolize
        Absl::malloc_internal
        Absl::strings
        Absl::int128
        Absl::string_view
        Absl::strings_internal
        Absl::throw_delegate
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::int128
find_library(int128_LIB NAMES libabsl_int128.so)
if(NOT int128_LIB)
    message(FATAL_ERROR "libabsl_int128.so not found")
endif()
add_library(Absl::int128 SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::int128 PATH ${int128_LIB})

# Absl::kernel_timeout_internal
find_library(kernel_timeout_internal_LIB NAMES libabsl_kernel_timeout_internal.so)
if(NOT kernel_timeout_internal_LIB)
    message(FATAL_ERROR "libabsl_kernel_timeout_internal.so not found")
endif()
add_library(Absl::kernel_timeout_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::kernel_timeout_internal
    PATH ${kernel_timeout_internal_LIB}
    LIBRARIES
        Absl::time
        Absl::civil_time
        Absl::strings
        Absl::int128
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::time_zone
)

# Absl::leak_check
find_library(leak_check_LIB NAMES libabsl_leak_check.so)
if(NOT leak_check_LIB)
    message(FATAL_ERROR "libabsl_leak_check.so not found")
endif()
add_library(Absl::leak_check SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::leak_check PATH ${leak_check_LIB})

# Absl::log_entry
find_library(log_entry_LIB NAMES libabsl_log_entry.so)
if(NOT log_entry_LIB)
    message(FATAL_ERROR "libabsl_log_entry.so not found")
endif()
add_library(Absl::log_entry SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_entry
    PATH ${log_entry_LIB}
    LIBRARIES
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::throw_delegate
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::civil_time
        Absl::time_zone
)

# Absl::log_globals
find_library(log_globals_LIB NAMES libabsl_log_globals.so)
if(NOT log_globals_LIB)
    message(FATAL_ERROR "libabsl_log_globals.so not found")
endif()
add_library(Absl::log_globals SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_globals
    PATH ${log_globals_LIB}
    LIBRARIES
        Absl::hash
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::city
        Absl::throw_delegate
        Absl::bad_optional_access
        Absl::bad_variant_access
        Absl::low_level_hash
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
        Absl::int128
)

# Absl::log_initialize
find_library(log_initialize_LIB NAMES libabsl_log_initialize.so)
if(NOT log_initialize_LIB)
    message(FATAL_ERROR "libabsl_log_initialize.so not found")
endif()
add_library(Absl::log_initialize SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_initialize
    PATH ${log_initialize_LIB}
    LIBRARIES
        Absl::log_globals
        Absl::log_internal_globals
        Absl::time
        Absl::hash
        Absl::city
        Absl::bad_optional_access
        Absl::bad_variant_access
        Absl::low_level_hash
        Absl::civil_time
        Absl::time_zone
        Absl::strings
        Absl::int128
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::log_internal_check_op
find_library(log_internal_check_op_LIB NAMES libabsl_log_internal_check_op.so)
if(NOT log_internal_check_op_LIB)
    message(FATAL_ERROR "libabsl_log_internal_check_op.so not found")
endif()
add_library(Absl::log_internal_check_op SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_internal_check_op
    PATH ${log_internal_check_op_LIB}
    LIBRARIES
        Absl::log_internal_message
        Absl::log_internal_nullguard
        Absl::examine_stack
        Absl::log_internal_format
        Absl::str_format_internal
        Absl::log_internal_proto
        Absl::log_internal_log_sink_set
        Absl::log_internal_globals
        Absl::log_globals
        Absl::hash
        Absl::city
        Absl::bad_optional_access
        Absl::bad_variant_access
        Absl::low_level_hash
        Absl::synchronization
        Absl::stacktrace
        Absl::symbolize
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::graphcycles_internal
        Absl::malloc_internal
        Absl::kernel_timeout_internal
        Absl::log_sink
        Absl::log_entry
        Absl::strerror
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::civil_time
        Absl::time_zone
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::log_internal_conditions
find_library(log_internal_conditions_LIB NAMES libabsl_log_internal_conditions.so)
if(NOT log_internal_conditions_LIB)
    message(FATAL_ERROR "libabsl_log_internal_conditions.so not found")
endif()
add_library(Absl::log_internal_conditions SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_internal_conditions
    PATH ${log_internal_conditions_LIB}
    LIBRARIES
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::log_internal_format
find_library(log_internal_format_LIB NAMES libabsl_log_internal_format.so)
if(NOT log_internal_format_LIB)
    message(FATAL_ERROR "libabsl_log_internal_format.so not found")
endif()
add_library(Absl::log_internal_format SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_internal_format
    PATH ${log_internal_format_LIB}
    LIBRARIES
        Absl::log_internal_globals
        Absl::time
        Absl::civil_time
        Absl::time_zone
        Absl::str_format_internal
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::log_internal_globals
find_library(log_internal_globals_LIB NAMES libabsl_log_internal_globals.so)
if(NOT log_internal_globals_LIB)
    message(FATAL_ERROR "libabsl_log_internal_globals.so not found")
endif()
add_library(Absl::log_internal_globals SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_internal_globals
    PATH ${log_internal_globals_LIB}
    LIBRARIES
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::throw_delegate
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
        Absl::int128
        Absl::civil_time
        Absl::time_zone
)

# Absl::log_internal_log_sink_set
find_library(log_internal_log_sink_set_LIB NAMES libabsl_log_internal_log_sink_set.so)
if(NOT log_internal_log_sink_set_LIB)
    message(FATAL_ERROR "libabsl_log_internal_log_sink_set.so not found")
endif()
add_library(Absl::log_internal_log_sink_set SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_internal_log_sink_set
    PATH ${log_internal_log_sink_set_LIB}
    LIBRARIES
        Absl::log_internal_globals
        Absl::log_globals
        Absl::log_sink
        Absl::synchronization
        Absl::hash
        Absl::city
        Absl::bad_optional_access
        Absl::bad_variant_access
        Absl::low_level_hash
        Absl::log_entry
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::time
        Absl::civil_time
        Absl::time_zone
        Absl::stacktrace
        Absl::symbolize
        Absl::strings
        Absl::string_view
        Absl::throw_delegate
        Absl::strings_internal
        Absl::int128
        Absl::malloc_internal
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::log_internal_message
find_library(log_internal_message_LIB NAMES libabsl_log_internal_message.so)
if(NOT log_internal_message_LIB)
    message(FATAL_ERROR "libabsl_log_internal_message.so not found")
endif()
add_library(Absl::log_internal_message SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_internal_message
    PATH ${log_internal_message_LIB}
    LIBRARIES
        Absl::examine_stack
        Absl::log_internal_format
        Absl::log_internal_proto
        Absl::log_internal_nullguard
        Absl::strerror
        Absl::str_format_internal
        Absl::log_internal_log_sink_set
        Absl::log_internal_globals
        Absl::log_globals
        Absl::hash
        Absl::city
        Absl::bad_optional_access
        Absl::bad_variant_access
        Absl::low_level_hash
        Absl::synchronization
        Absl::stacktrace
        Absl::symbolize
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::graphcycles_internal
        Absl::malloc_internal
        Absl::kernel_timeout_internal
        Absl::log_sink
        Absl::log_entry
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::civil_time
        Absl::time_zone
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::log_internal_nullguard
find_library(log_internal_nullguard_LIB NAMES libabsl_log_internal_nullguard.so)
if(NOT log_internal_nullguard_LIB)
    message(FATAL_ERROR "libabsl_log_internal_nullguard.so not found")
endif()
add_library(Absl::log_internal_nullguard SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::log_internal_nullguard PATH ${log_internal_nullguard_LIB})

# Absl::log_internal_proto
find_library(log_internal_proto_LIB NAMES libabsl_log_internal_proto.so)
if(NOT log_internal_proto_LIB)
    message(FATAL_ERROR "libabsl_log_internal_proto.so not found")
endif()
add_library(Absl::log_internal_proto SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_internal_proto
    PATH ${log_internal_proto_LIB}
    LIBRARIES
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::log_severity
find_library(log_severity_LIB NAMES libabsl_log_severity.so)
if(NOT log_severity_LIB)
    message(FATAL_ERROR "libabsl_log_severity.so not found")
endif()
add_library(Absl::log_severity SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::log_severity PATH ${log_severity_LIB})

# Absl::log_sink
find_library(log_sink_LIB NAMES libabsl_log_sink.so)
if(NOT log_sink_LIB)
    message(FATAL_ERROR "libabsl_log_sink.so not found")
endif()
add_library(Absl::log_sink SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::log_sink
    PATH ${log_sink_LIB}
    LIBRARIES
        Absl::log_entry
        Absl::time
        Absl::strings
        Absl::string_view
        Absl::throw_delegate
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::civil_time
        Absl::time_zone
)

# Absl::low_level_hash
find_library(low_level_hash_LIB NAMES libabsl_low_level_hash.so)
if(NOT low_level_hash_LIB)
    message(FATAL_ERROR "libabsl_low_level_hash.so not found")
endif()
add_library(Absl::low_level_hash SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::low_level_hash
    PATH ${low_level_hash_LIB}
    LIBRARIES
        Absl::int128
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::malloc_internal
find_library(malloc_internal_LIB NAMES libabsl_malloc_internal.so)
if(NOT malloc_internal_LIB)
    message(FATAL_ERROR "libabsl_malloc_internal.so not found")
endif()
add_library(Absl::malloc_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::malloc_internal
    PATH ${malloc_internal_LIB}
    LIBRARIES
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::random_distributions
find_library(random_distributions_LIB NAMES libabsl_random_distributions.so)
if(NOT random_distributions_LIB)
    message(FATAL_ERROR "libabsl_random_distributions.so not found")
endif()
add_library(Absl::random_distributions SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::random_distributions
    PATH ${random_distributions_LIB}
    LIBRARIES
        Absl::strings
        Absl::int128
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::random_internal_platform
find_library(random_internal_platform_LIB NAMES libabsl_random_internal_platform.so)
if(NOT random_internal_platform_LIB)
    message(FATAL_ERROR "libabsl_random_internal_platform.so not found")
endif()
add_library(Absl::random_internal_platform SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::random_internal_platform PATH ${random_internal_platform_LIB})

# Absl::random_internal_pool_urbg
find_library(random_internal_pool_urbg_LIB NAMES libabsl_random_internal_pool_urbg.so)
if(NOT random_internal_pool_urbg_LIB)
    message(FATAL_ERROR "libabsl_random_internal_pool_urbg.so not found")
endif()
add_library(Absl::random_internal_pool_urbg SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::random_internal_pool_urbg
    PATH ${random_internal_pool_urbg_LIB}
    LIBRARIES
        Absl::random_internal_randen
        Absl::random_internal_seed_material
        Absl::random_seed_gen_exception
        Absl::random_internal_randen_hwaes
        Absl::random_internal_randen_hwaes_impl
        Absl::random_internal_randen_slow
        Absl::random_internal_platform
        Absl::bad_optional_access
        Absl::strings
        Absl::string_view
        Absl::throw_delegate
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::int128
)

# Absl::random_internal_randen_hwaes_impl
find_library(random_internal_randen_hwaes_impl_LIB NAMES libabsl_random_internal_randen_hwaes_impl.so)
if(NOT random_internal_randen_hwaes_impl_LIB)
    message(FATAL_ERROR "libabsl_random_internal_randen_hwaes_impl.so not found")
endif()
add_library(Absl::random_internal_randen_hwaes_impl SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::random_internal_randen_hwaes_impl
    PATH ${random_internal_randen_hwaes_impl_LIB}
    LIBRARIES
        Absl::random_internal_platform
)

# Absl::random_internal_randen_hwaes
find_library(random_internal_randen_hwaes_LIB NAMES libabsl_random_internal_randen_hwaes.so)
if(NOT random_internal_randen_hwaes_LIB)
    message(FATAL_ERROR "libabsl_random_internal_randen_hwaes.so not found")
endif()
add_library(Absl::random_internal_randen_hwaes SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::random_internal_randen_hwaes
    PATH ${random_internal_randen_hwaes_LIB}
    LIBRARIES
        Absl::random_internal_randen_hwaes_impl
        Absl::random_internal_platform
)

# Absl::random_internal_randen_slow
find_library(random_internal_randen_slow_LIB NAMES libabsl_random_internal_randen_slow.so)
if(NOT random_internal_randen_slow_LIB)
    message(FATAL_ERROR "libabsl_random_internal_randen_slow.so not found")
endif()
add_library(Absl::random_internal_randen_slow SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::random_internal_randen_slow
    PATH ${random_internal_randen_slow_LIB}
    LIBRARIES
        Absl::random_internal_platform
)

# Absl::random_internal_randen
find_library(random_internal_randen_LIB NAMES libabsl_random_internal_randen.so)
if(NOT random_internal_randen_LIB)
    message(FATAL_ERROR "libabsl_random_internal_randen.so not found")
endif()
add_library(Absl::random_internal_randen SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::random_internal_randen
    PATH ${random_internal_randen_LIB}
    LIBRARIES
        Absl::random_internal_randen_hwaes
        Absl::random_internal_randen_slow
        Absl::random_internal_randen_hwaes_impl
        Absl::random_internal_platform
)

# Absl::random_internal_seed_material
find_library(random_internal_seed_material_LIB NAMES libabsl_random_internal_seed_material.so)
if(NOT random_internal_seed_material_LIB)
    message(FATAL_ERROR "libabsl_random_internal_seed_material.so not found")
endif()
add_library(Absl::random_internal_seed_material SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::random_internal_seed_material
    PATH ${random_internal_seed_material_LIB}
    LIBRARIES
        Absl::strings
        Absl::bad_optional_access
        Absl::string_view
        Absl::throw_delegate
        Absl::strings_internal
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
        Absl::int128
)

# Absl::random_seed_gen_exception
find_library(random_seed_gen_exception_LIB NAMES libabsl_random_seed_gen_exception.so)
if(NOT random_seed_gen_exception_LIB)
    message(FATAL_ERROR "libabsl_random_seed_gen_exception.so not found")
endif()
add_library(Absl::random_seed_gen_exception SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::random_seed_gen_exception PATH ${random_seed_gen_exception_LIB})

# Absl::random_seed_sequences
find_library(random_seed_sequences_LIB NAMES libabsl_random_seed_sequences.so)
if(NOT random_seed_sequences_LIB)
    message(FATAL_ERROR "libabsl_random_seed_sequences.so not found")
endif()
add_library(Absl::random_seed_sequences SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::random_seed_sequences
    PATH ${random_seed_sequences_LIB}
    LIBRARIES
        Absl::random_internal_pool_urbg
        Absl::random_internal_seed_material
        Absl::random_seed_gen_exception
        Absl::random_internal_randen
        Absl::random_internal_randen_hwaes
        Absl::random_internal_randen_hwaes_impl
        Absl::random_internal_randen_slow
        Absl::random_internal_platform
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::bad_optional_access
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::raw_hash_set
find_library(raw_hash_set_LIB NAMES libabsl_raw_hash_set.so)
if(NOT raw_hash_set_LIB)
    message(FATAL_ERROR "libabsl_raw_hash_set.so not found")
endif()
add_library(Absl::raw_hash_set SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::raw_hash_set
    PATH ${raw_hash_set_LIB}
    LIBRARIES
        Absl::hash
        Absl::hashtablez_sampler
        Absl::bad_optional_access
        Absl::city
        Absl::bad_variant_access
        Absl::low_level_hash
        Absl::exponential_biased
        Absl::synchronization
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::stacktrace
        Absl::symbolize
        Absl::malloc_internal
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::time
        Absl::strings
        Absl::int128
        Absl::string_view
        Absl::throw_delegate
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::civil_time
        Absl::time_zone
)

# Absl::raw_logging_internal
find_library(raw_logging_internal_LIB NAMES libabsl_raw_logging_internal.so)
if(NOT raw_logging_internal_LIB)
    message(FATAL_ERROR "libabsl_raw_logging_internal.so not found")
endif()
add_library(Absl::raw_logging_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::raw_logging_internal
    PATH ${raw_logging_internal_LIB}
    LIBRARIES
        Absl::log_severity
)

# Absl::spinlock_wait
find_library(spinlock_wait_LIB NAMES libabsl_spinlock_wait.so)
if(NOT spinlock_wait_LIB)
    message(FATAL_ERROR "libabsl_spinlock_wait.so not found")
endif()
add_library(Absl::spinlock_wait SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::spinlock_wait PATH ${spinlock_wait_LIB})

# Absl::stacktrace
find_library(stacktrace_LIB NAMES libabsl_stacktrace.so)
if(NOT stacktrace_LIB)
    message(FATAL_ERROR "libabsl_stacktrace.so not found")
endif()
add_library(Absl::stacktrace SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::stacktrace
    PATH ${stacktrace_LIB}
    LIBRARIES
        Absl::debugging_internal
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::statusor
find_library(statusor_LIB NAMES libabsl_statusor.so)
if(NOT statusor_LIB)
    message(FATAL_ERROR "libabsl_statusor.so not found")
endif()
add_library(Absl::statusor SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::statusor
    PATH ${statusor_LIB}
    LIBRARIES
        Absl::status
        Absl::cord
        Absl::cordz_info
        Absl::cord_internal
        Absl::cordz_functions
        Absl::exponential_biased
        Absl::cordz_handle
        Absl::synchronization
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::time
        Absl::civil_time
        Absl::time_zone
        Absl::crc_cord_state
        Absl::crc32c
        Absl::crc_internal
        Absl::crc_cpu_detect
        Absl::bad_optional_access
        Absl::stacktrace
        Absl::str_format_internal
        Absl::strerror
        Absl::symbolize
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::int128
        Absl::throw_delegate
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::malloc_internal
        Absl::base
        Absl::spinlock_wait
        Absl::bad_variant_access
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::status
find_library(status_LIB NAMES libabsl_status.so)
if(NOT status_LIB)
    message(FATAL_ERROR "libabsl_status.so not found")
endif()
add_library(Absl::status SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::status
    PATH ${status_LIB}
    LIBRARIES
        Absl::cord
        Absl::strerror
        Absl::bad_optional_access
        Absl::cordz_info
        Absl::cord_internal
        Absl::cordz_functions
        Absl::exponential_biased
        Absl::cordz_handle
        Absl::synchronization
        Absl::stacktrace
        Absl::symbolize
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::graphcycles_internal
        Absl::malloc_internal
        Absl::kernel_timeout_internal
        Absl::time
        Absl::civil_time
        Absl::time_zone
        Absl::crc_cord_state
        Absl::crc32c
        Absl::str_format_internal
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::int128
        Absl::crc_internal
        Absl::crc_cpu_detect
        Absl::base
        Absl::spinlock_wait
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::strerror
find_library(strerror_LIB NAMES libabsl_strerror.so)
if(NOT strerror_LIB)
    message(FATAL_ERROR "libabsl_strerror.so not found")
endif()
add_library(Absl::strerror SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::strerror PATH ${strerror_LIB})

# Absl::str_format_internal
find_library(str_format_internal_LIB NAMES libabsl_str_format_internal.so)
if(NOT str_format_internal_LIB)
    message(FATAL_ERROR "libabsl_str_format_internal.so not found")
endif()
add_library(Absl::str_format_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::str_format_internal
    PATH ${str_format_internal_LIB}
    LIBRARIES
        Absl::strings
        Absl::int128
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::strings_internal
find_library(strings_internal_LIB NAMES libabsl_strings_internal.so)
if(NOT strings_internal_LIB)
    message(FATAL_ERROR "libabsl_strings_internal.so not found")
endif()
add_library(Absl::strings_internal SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::strings_internal
    PATH ${strings_internal_LIB}
    LIBRARIES
        Absl::base
        Absl::raw_logging_internal
        Absl::log_severity
        Absl::spinlock_wait
)

# Absl::strings
find_library(strings_LIB NAMES libabsl_strings.so)
if(NOT strings_LIB)
    message(FATAL_ERROR "libabsl_strings.so not found")
endif()
add_library(Absl::strings SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::strings
    PATH ${strings_LIB}
    LIBRARIES
        Absl::string_view
        Absl::strings_internal
        Absl::int128
        Absl::throw_delegate
        Absl::base
        Absl::spinlock_wait
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::string_view
find_library(string_view_LIB NAMES libabsl_string_view.so)
if(NOT string_view_LIB)
    message(FATAL_ERROR "libabsl_string_view.so not found")
endif()
add_library(Absl::string_view SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::string_view
    PATH ${string_view_LIB}
    LIBRARIES
        Absl::base
        Absl::throw_delegate
        Absl::spinlock_wait
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::symbolize
find_library(symbolize_LIB NAMES libabsl_symbolize.so)
if(NOT symbolize_LIB)
    message(FATAL_ERROR "libabsl_symbolize.so not found")
endif()
add_library(Absl::symbolize SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::symbolize
    PATH ${symbolize_LIB}
    LIBRARIES
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::malloc_internal
        Absl::strings
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::int128
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::synchronization
find_library(synchronization_LIB NAMES libabsl_synchronization.so)
if(NOT synchronization_LIB)
    message(FATAL_ERROR "libabsl_synchronization.so not found")
endif()
add_library(Absl::synchronization SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::synchronization
    PATH ${synchronization_LIB}
    LIBRARIES
        Absl::graphcycles_internal
        Absl::kernel_timeout_internal
        Absl::stacktrace
        Absl::symbolize
        Absl::time
        Absl::civil_time
        Absl::time_zone
        Absl::malloc_internal
        Absl::debugging_internal
        Absl::demangle_internal
        Absl::strings
        Absl::int128
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::throw_delegate
find_library(throw_delegate_LIB NAMES libabsl_throw_delegate.so)
if(NOT throw_delegate_LIB)
    message(FATAL_ERROR "libabsl_throw_delegate.so not found")
endif()
add_library(Absl::throw_delegate SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::throw_delegate
    PATH ${throw_delegate_LIB}
    LIBRARIES
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::time
find_library(time_LIB NAMES libabsl_time.so)
if(NOT time_LIB)
    message(FATAL_ERROR "libabsl_time.so not found")
endif()
add_library(Absl::time SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(
    NAME Absl::time
    PATH ${time_LIB}
    LIBRARIES
        Absl::civil_time
        Absl::strings
        Absl::time_zone
        Absl::int128
        Absl::string_view
        Absl::strings_internal
        Absl::base
        Absl::spinlock_wait
        Absl::throw_delegate
        Absl::raw_logging_internal
        Absl::log_severity
)

# Absl::time_zone
find_library(time_zone_LIB NAMES libabsl_time_zone.so)
if(NOT time_zone_LIB)
    message(FATAL_ERROR "libabsl_time_zone.so not found")
endif()
add_library(Absl::time_zone SHARED IMPORTED GLOBAL)
nebula_setup_imported_library(NAME Absl::time_zone PATH ${time_zone_LIB})

