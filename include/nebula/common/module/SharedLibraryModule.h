// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <string_view>
#include <unordered_map>

#include "nebula/common/base/Status.h"
#include "nebula/common/datatype/Value.h"
#include "nebula/common/module/Module.h"

namespace nebula::module {

class SharedLibraryModule final : public Module {
public:
    explicit SharedLibraryModule(std::string moduleDir);
    ~SharedLibraryModule() override = default;
    SharedLibraryModule(const SharedLibraryModule &) = delete;
    SharedLibraryModule(SharedLibraryModule &&) = delete;
    SharedLibraryModule &operator=(const SharedLibraryModule &) = delete;
    SharedLibraryModule &operator=(SharedLibraryModule &&) = delete;

    bool loaded(const std::string &name) const override;

    Status load(const std::string &name) override;

    Status unload() override;

    Status addProc(Procedure proc) override;

    ProcHolder getProc(const std::string &name) const override;

    Status addFunc(Function func) override;

    Status addPlugin(Plugin plugin) override;

    PluginHolder getPlugin(const std::string &name) const override;

    std::vector<PluginHolder> getPluginByType(const plugin::PluginType type) const override;

    FuncHolder getFunc(const std::string &name) const override;

    const std::string &name() const override {
        return name_;
    }

    folly::dynamic getModuleInfo() const override;

private:
    bool loaded() const;

private:
    using LoadHook = Status (*)(SharedLibraryModule *);
    using UnloadHook = void (*)(SharedLibraryModule *);
    static constexpr auto kLoadFuncName = "nebula_module_load";
    static constexpr auto kUnloadFuncName = "nebula_module_unload";
    std::string moduleDir_;
    std::string name_;
    std::string path_;
    // Whether the module can be unloaded or reloaded
    bool uncloseable_{false};
    void *handle_{nullptr};
    LoadHook load_{nullptr};
    UnloadHook unload_{nullptr};
    std::unordered_map<std::string, Procedure> procedures_;
    std::unordered_map<std::string, Function> functions_;
    std::unordered_map<std::string, Plugin> plugins_;
};

}  //  namespace nebula::module
