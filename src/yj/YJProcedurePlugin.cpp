// Copyright (c) 2023 vesoft inc. All rights reserved.

#include "yj/YJProcedurePlugin.h"

#include "nebula/common/module/ModuleManager.h"

using nebula::Status;
using nebula::plugin::PluginInfo;
using nebula::plugin::PluginType;
using nebula::plugin::Procedure;

// TODO: declare more DBMS procedures here
extern Procedure declareNetworkTopoProcedure();

namespace yj {

YJProcedurePlugin::YJProcedurePlugin()
        : ProcedurePlugin(PluginInfo{PluginType::PROCEDURE,
                                     kName,
                                     kAuthor,
                                     kDescription,
                                     kLicense,
                                     kVersion,
                                     NEBULA_PLUGIN_API_VERSION}) {
    addProcedure(declareNetworkTopoProcedure());
}

}  // namespace yj

REGISTER_PLUGIN(yj::YJProcedurePlugin)
