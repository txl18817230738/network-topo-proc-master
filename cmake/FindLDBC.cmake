# Copyright (c) 2022 vesoft inc. All rights reserved.

# download ldbc official data set
include(ExternalProject)

set(SF01_PARAM_URL $ENV{SF01_PARAM_URL})
if (NOT SF01_PARAM_URL)
    set(SF01_PARAM_URL https://oss-cdn.nebula-graph.com.cn/ldbc/substitution_parameters-sf0.1.tar.gz)
endif()

set(SF01_DATA_URL $ENV{SF01_DATA_URL})
if (NOT SF01_DATA_URL)
    set(SF01_DATA_URL https://oss-cdn.nebula-graph.com.cn/ldbc/social_network-csv_merge_foreign-sf0.1.tar.gz)
endif()

ExternalProject_Add(
    ldbc_parameters
    PREFIX ${CMAKE_BINARY_DIR}/ldbc
    SOURCE_DIR ${CMAKE_BINARY_DIR}/ldbc/substitution_parameters-sf0.1
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/ldbc/tarballs
    DOWNLOAD_NO_PROGRESS TRUE
    STAMP_DIR ${CMAKE_BINARY_DIR}/ldbc/stamp
    TLS_VERIFY FALSE
    URL ${SF01_PARAM_URL}
    URL_MD5 80ac52b62831393e72707066d70afc1a
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD TRUE
)

ExternalProject_Add(
    ldbc_data
    PREFIX ${CMAKE_BINARY_DIR}/ldbc
    SOURCE_DIR ${CMAKE_BINARY_DIR}/ldbc/social_network-csv_merge_foreign-sf0.1
    DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/ldbc/tarballs
    DOWNLOAD_NO_PROGRESS TRUE
    STAMP_DIR ${CMAKE_BINARY_DIR}/ldbc/stamp
    TLS_VERIFY FALSE
    URL ${SF01_DATA_URL}
    URL_MD5 381bc21a219b2e07767f8c58f43ea528
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    LOG_DOWNLOAD TRUE
)
