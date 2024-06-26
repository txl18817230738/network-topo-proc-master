// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <string>
#include <unordered_map>

#include <folly/RWSpinLock.h>
#include <folly/dynamic.h>

#include "nebula/common/base/StatusOr.h"
#include "nebula/common/module/SharedLibraryModule.h"

/**
 * ModuleManager is a singleton class to manage all dynamic loaded modules,
 * which may be implemented in various programming languages, C++, Python, Java, etc.
 *
 * A module consists of procedures and functions, which only differ in their return types.
 *
 * A fully qualified procedure or function name is a dot-delimited string, with each part
 * is a word following the C token rules, and the last part is the procedure or function name.
 * The same rules apply to the fully qualified module name in which the last part is
 * the extension which indicates the implementing language.
 */

namespace nebula::module {

class ModuleManager final {
public:
    /**
     * @brief   Load all modules in the module directory. This method is meant to
     *      be invoked upon the process startup.
     *      The module directory resides in the parent directory of the executable.
     *      All modules with illegal name are ignored to be loaded.
     *      All modules with the same base name are ignored to be loaded.
     *
     * @return  For now, this method always returns OK, since the failure condition
     *      can be corrected and the modules can be loaded later at runtime.
     */
    [[nodiscard]] static Status loadAll() {
        return get().loadAllInternal();
    }

    /**
     * @brief   Load a specific module.
     * @param `name', module name without file extension
     * @return  Status
     */
    [[nodiscard]] static Status load(const std::string &name) {
        return get().loadInternal(name);
    }

    /**
     * @brief   Unload a specific module, in an optimistic way,
     *          will fail if the module is being in use.
     * @param   `name', module name without file extension
     * @return  Status
     */
    [[nodiscard]] static Status unload(const std::string &name) {
        return get().unloadInternal(name);
    }

    /**
     * @brief   Atomically reload a specific module, in an optimistic way,
     *          will fail if the module is being in use.
     * @param   `name', module name without file extension
     * @return  Status
     */
    [[nodiscard]] static Status reload(const std::string &name) {
        return get().reloadInternal(name);
    }


    /**
     * @brief   Check if a module is loaded.The method `loaded` must be called before `load` and
     * `unload` because these two methods are not reentrant
     * @param   `name', module name without file extension
     * @return  true if the module is loaded, false otherwise
     */
    static bool loaded(const std::string &name);

    /**
     * @brief   Locate and return a procedure
     * @param   `name', fully qualified procedure name
     * @return  An object holding the target procedure, which is null if not exists, and
     *          a reference to the underlying module to protect the module from being unloaded.
     *          Thus the caller must destruct the holder when the procedure is no longer needed.
     */
    static ProcHolder getProc(const std::string &name) {
        return get().getProcInternal(name);
    }

    /**
     * @brief   Locate and return a function
     * @param   `name', fully qualified function name
     * @return  An object holding the target function, which is null if not exists, and
     *          a reference to the underlying module to protect the module from being unloaded.
     *          Thus the caller must destruct the holder when the function is no longer needed.
     */
    static FuncHolder getFunc(const std::string &name) {
        return get().getFuncInternal(name);
    }

    /**
     * @brief   Locate and return a plugin
     * @param   `name', fully qualified plugin name
     * @return  An object holding the target function, which is null if not exists, and
     *          a reference to the underlying module to protect the module from being unloaded.
     *          Thus the caller must destruct the holder when the function is no longer needed.
     */
    static PluginHolder getPlugin(const std::string &name) {
        return get().getPluginInternal(name);
    }

    static std::vector<PluginHolder> getPluginByType(const plugin::PluginType type) {
        return get().getPluginByTypeInternal(type);
    }

    /**
     * @brief   Get information about all loaded modules.
     * @return  A json represented as a folly::dynamic, whose structure is as follows:
     *  {
     *      "modules": [
     *          {
     *              "name": "my.algo",
     *              "type": "so",
     *              "functions": [                              # optional
     *                  {
     *                      "name": "sort",
     *                      "return_type": "LIST",
     *                      "comment": "sort a list"            # optional
     *                      "parameters": [                     # optional
     *                          {
     *                              "name": "list",
     *                              "type": "LIST",
     *                              "comment": "list to sort"   # optional
     *                          }
     *                      ]   # parameters
     *                  },
     *                  ...
     *              ]   # functions
     *              "procedures": [                             # optional
     *                  {
     *                      "name": "pagerank",
     *                      "return_columns": [
     *                          {
     *                              "name": "col1"
     *                              "type": "TYPE1"
     *                          },
     *                          {
     *                              "name": "col2"
     *                              "type": "TYPE2"
     *                          },
     *                          ...
     *                      ]   # return columns
     *                      "parameters": [                     # optional
     *                          {
     *                              "name": "graph",
     *                              "type": "GRAPH"
     *                              "comment": "..."            # optional
     *                          },
     *                          ...
     *                      ]   # parameters
     *                  },
     *                  ...
     *              ]   # procedures
     *          },
     *          ...
     *      ]   # modules
     *  }
     */
    static folly::dynamic getAllModuleInfo() {
        return get().getAllModuleInfoInternal();
    }

private:
    friend class ModuleManager_RemoveDuplicates_Test;
    ModuleManager();
    ~ModuleManager();
    ModuleManager(const ModuleManager &) = delete;
    ModuleManager(ModuleManager &&) = delete;
    ModuleManager &operator=(const ModuleManager &) = delete;
    ModuleManager &operator=(ModuleManager &&) = delete;

    static ModuleManager &get();
    Status loadAllInternal();
    Status loadInternal(const std::string &name);
    Status loadInternalLocked(const std::string &name);
    Status unloadInternal(const std::string &name);
    Status unloadInternalLocked(const std::string &name);
    Status reloadInternal(const std::string &name);

    FuncHolder getFuncInternal(const std::string &name) const;
    ProcHolder getProcInternal(const std::string &name) const;
    PluginHolder getPluginInternal(const std::string &name) const;
    std::vector<PluginHolder> getPluginByTypeInternal(const plugin::PluginType type) const;

    folly::dynamic getAllModuleInfoInternal() const;

    static std::vector<std::string> removeDuplicates(std::vector<std::string> &list);

private:
    std::string moduleDir_;
    std::unordered_map<std::string, std::unique_ptr<Module>> modules_;
    mutable folly::RWSpinLock lock_;
};

}  // namespace nebula::module

#define REGISTER_PLUGIN(PluginClass)                                                    \
    extern "C" nebula::Status nebula_module_load(                                       \
            nebula::module::SharedLibraryModule *module) {                              \
        {                                                                               \
            auto p = std::make_shared<PluginClass>();                                   \
            nebula::module::Plugin plugin{p, module};                                   \
            NG_RETURN_IF_ERROR(module->addPlugin(plugin));                              \
        }                                                                               \
        return nebula::Status::OK();                                                    \
    }                                                                                   \
    extern "C" void nebula_module_unload(nebula::module::SharedLibraryModule *module) { \
        (void)module;                                                                   \
    }
