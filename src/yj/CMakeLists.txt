# Copyright (c) 2023 vesoft inc. All rights reserved.

nebula_add_solib(
    NAME yj
    SOURCES
        NetworkTopoProcedure.cpp
        YJProcedurePlugin.cpp
    LIBRARIES
        nb-base
        nb-module
        nb-response
        nb-datatype
        nb-mem-graph
        nb-memory-table
        nb-computing
        nb-memory
        nb-plugin
        nb-graph-context
        nb-computing-algo
        nb-value-type
        fmt
        glog
        folly
        boost_context
)


# nebula_add_solib(
#     NAME yj2
#     SOURCES
#         NetworkTopoProcedure2.cpp
#         YJProcedurePlugin.cpp
#     LIBRARIES
#         nb-base
#         nb-module
#         nb-response
#         nb-datatype
#         nb-mem-graph
#         nb-memory-table
#         nb-computing
#         nb-memory
#         nb-plugin
#         nb-graph-context
#         nb-computing-algo
#         nb-value-type
#         fmt
#         glog
#         folly
#         boost_context
# )
