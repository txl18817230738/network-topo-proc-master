// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/module/SharedLibraryModule.h"
#include "nebula/plugins/Plugin.h"
#include "nebula/plugins/PluginCommon.h"

namespace nebula {
namespace plugin {

/**
 * Base class for Plugins that provides customizable function implementations.
 *
 * A FunctionPlugin bundles a collections of functions, those functions share
 * the same PluginInfo and have the same lifecycle managed by the Plugin framework.
 */
class FunctionPlugin : public Plugin {
public:
    explicit FunctionPlugin(const PluginInfo& info) : plugin::Plugin(info) {}

    Status init(const std::unordered_map<std::string, std::string>& config) override {
        if (info_.type != PluginType::FUNCTION) {
            return V_STATUS(PLUGIN_INVALID_TYPE, info_.toString(), "Function Plugin");
        }
        return Plugin::init(config);
    }

    Status destroy() override {
        return Plugin::destroy();
    }

    /**
     * @brief Add a function to this plugin
     */
    Status addFunction(Function func) {
        auto name = func.name;
        functions_.emplace(std::move(name), std::move(func));
        return Status::OK();
    }

    /**
     * @brief Retrieve a function
     */
    const Function* getFunction(const std::string& name) const {
        auto iter = functions_.find(name);
        if (iter == functions_.end()) {
            return nullptr;
        }
        return &iter->second;
    }

    /**
     * @brief Get all functions
     */
    std::unordered_map<std::string, Function>& getAllFunctions() const {
        return functions_;
    }

protected:
    mutable std::unordered_map<std::string, Function> functions_;
};


}  // namespace plugin
}  // namespace nebula
