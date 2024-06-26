# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Glog_LIB NAMES libglog.so DOC "Glog library")
if(NOT Glog_LIB)
        message(FATAL_ERROR "libglog not found")
endif()

add_library(Glog SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Glog PATH ${Glog_LIB} LIBRARIES Gflags Unwind pthread)
