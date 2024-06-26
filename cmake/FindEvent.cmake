# Copyright (c) 2023 vesoft inc. All rights reserved.

find_library(Event_LIB NAMES libevent.so DOC "Event library")
find_library(EventCore_LIB NAMES libevent_core.so DOC "Event core library")
if(NOT Event_LIB)
        message(FATAL_ERROR "libevent not found")
endif()
if(NOT EventCore_LIB)
        message(FATAL_ERROR "libevent_core not found")
endif()

add_library(Event SHARED IMPORTED GLOBAL)
add_library(Event::core SHARED IMPORTED GLOBAL)

nebula_setup_imported_library(NAME Event PATH ${Event_LIB} LIBRARIES pthread)
nebula_setup_imported_library(NAME Event::core PATH ${EventCore_LIB} LIBRARIES pthread)
