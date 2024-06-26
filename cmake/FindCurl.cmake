find_library(Curl_LIB NAMES libcurl.so)
if(NOT Curl_LIB)
    message(FATAL_ERROR "libcurl not found")
endif()

add_library(Curl SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Curl PATH ${Curl_LIB} LIBRARIES SSL Zlib)
