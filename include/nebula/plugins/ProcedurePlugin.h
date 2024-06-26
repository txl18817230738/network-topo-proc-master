// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/plugins/Plugin.h"
#include "nebula/plugins/PluginCommon.h"

namespace nebula {
namespace plugin {

/**
 * Base class for Plugins that provides customizable procedure implementations.
 *
 * A ProcedurePlugin bundles a collections of procedures, those procedure share
 * the same PluginInfo and have the same lifecycle managed by the Plugin framework.
 */
class ProcedurePlugin : public Plugin {
public:
    explicit ProcedurePlugin(const PluginInfo& info) : Plugin(info) {
        CHECK(info.type == PluginType::PROCEDURE);
    }

    Status init(const std::unordered_map<std::string, std::string>& config) override {
        if (info_.type != PluginType::PROCEDURE) {
            return V_STATUS(PLUGIN_INVALID_TYPE, info_.toString(), "Procedure Plugin");
        }
        return Plugin::init(config);
    }

    Status destroy() override {
        return Plugin::destroy();
    }

    /**
     * @brief Add a procedure to this plugin
     */
    Status addProcedure(Procedure proc) {
        auto name = proc.name;

        procedures_.emplace(std::move(name), std::move(proc));
        return Status::OK();
    }

    /**
     * @brief Retrieve a procedure
     */
    const Procedure* getProcedure(const std::string& name) const {
        auto iter = procedures_.find(name);
        if (iter == procedures_.end()) {
            return nullptr;
        }
        return &iter->second;
    }

    /**
     * @brief Get all functions
     */
    std::unordered_map<std::string, Procedure>& getAllProcedures() const {
        return procedures_;
    }

protected:
    mutable std::unordered_map<std::string, Procedure> procedures_;
};


}  // namespace plugin
}  // namespace nebula
