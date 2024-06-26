// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <regex>
#include <string_view>

#include <folly/dynamic.h>

#include "nebula/common/base/Status.h"
#include "nebula/common/datatype/Value.h"
#include "nebula/common/response/ExecutionOutcome.h"
#include "nebula/plugins/Plugin.h"
#include "nebula/plugins/PluginCommon.h"

namespace nebula {

namespace exec {
class ExecutionContext;
}  // namespace exec

namespace module {

class Module;

/**
 * Prototype of Plugin
 */
struct Plugin final {
    std::shared_ptr<plugin::Plugin> ptr;
    Module *module;
};

struct Function final {
    plugin::Function func;
    Module *module;
};

struct Procedure final {
    plugin::Procedure proc;
    Module *module;
};

/**
 * Object to hold a pointer to procedure or function and a reference to the underlying module
 */
template <typename T>
class Holder final {
public:
    explicit Holder(const T *target) {
        target_ = target;
        if (target_ != nullptr) {
            target_->module->ref();
        }
    }

    ~Holder() {
        if (target_ != nullptr) {
            target_->module->unref();
            target_ = nullptr;
        }
    }

    Holder(Holder &&rhs) noexcept {
        target_ = rhs.target_;
        rhs.target_ = nullptr;
    }

    Holder &operator=(Holder &&rhs) noexcept {
        if (this != &rhs) {
            if (target_ != nullptr) {
                target_->module->unref();
            }
            target_ = rhs.target_;
            rhs.target_ = nullptr;
        }
        return *this;
    }

    Holder(const Holder &rhs) {
        target_ = rhs.target_;
        if (target_ != nullptr) {
            target_->module->ref();
        }
    }

    Holder &operator=(const Holder &rhs) {
        if (this != &rhs) {
            if (target_ != nullptr) {
                target_->module->unref();
            }
            target_ = rhs.target_;
            if (target_ != nullptr) {
                target_->module->ref();
            }
        }
        return *this;
    }

    void reset() {
        if (target_ != nullptr) {
            target_->module->unref();
            target_ = nullptr;
        }
    }

    bool operator==(const Holder &rhs) const {
        return target_ == rhs.target_;
    }

    bool operator!=(const Holder &rhs) const {
        return target_ != rhs.target_;
    }

    const T *operator->() const {
        return target_;
    }

    const T &operator*() const {
        return *target_;
    }

    operator bool() const {
        return target_ != nullptr;
    }

    const T *get() const {
        return target_;
    }

private:
    const T *target_{nullptr};
};

using ProcHolder = Holder<Procedure>;
using FuncHolder = Holder<Function>;
using PluginHolder = Holder<Plugin>;

/**
 * Virtual base class to represent a dynamic loaded module.
 * Derive from this class to implement a module for a specific programming language.
 */
class Module {
public:
    Module() = default;
    virtual ~Module() {}
    Module(const Module &) = delete;
    Module(Module &&) = delete;
    Module &operator=(const Module &) = delete;
    Module &operator=(Module &&) = delete;


    /**
     * @brief   Check if the specified module is loaded
     */
    virtual bool loaded(const std::string &name) const = 0;

    /**
     * @brief   Load a module
     * @param   `name', module name with the extension and without the module directory
     */
    virtual Status load(const std::string &name) = 0;

    /**
     * @brief   Unload this module
     */
    virtual Status unload() = 0;

    /**
     * @brief   Add a procedure to this module, invoked by the module writer upon loading
     */
    virtual Status addProc(Procedure proc) = 0;

    /**
     * @brief   Retrieve a procedure
     */
    virtual ProcHolder getProc(const std::string &name) const = 0;

    /**
     * @brief   Add a function to this module, invoked by the module writer upon loading
     */
    virtual Status addFunc(Function func) = 0;

    /**
     * @brief   Retrieve a procedure
     */
    virtual FuncHolder getFunc(const std::string &name) const = 0;

    /**
     * @brief   Add a plugin to this module, invoked by the module writer upon loading
     */
    virtual Status addPlugin(Plugin plugin) = 0;

    /**
     * @brief   Retrieve a plugin
     */
    virtual PluginHolder getPlugin(const std::string &name) const = 0;

    /**
     * @brief   Retrieve plugins by type
     */
    virtual std::vector<PluginHolder> getPluginByType(const plugin::PluginType type) const = 0;

    virtual const std::string &name() const = 0;

    /**
     * @brief   Get infos about this module. See ModuleManager::getAllModuleInfo for the spec.
     */
    virtual folly::dynamic getModuleInfo() const = 0;

    /**
     * Reference counting on the module instance.
     * ModuleManager and Holder are responsible to manage this, so the module writer does
     * not have to care.
     */
    auto refcnt() const {
        return refcnt_.load();
    }

    void ref() {
        ++refcnt_;
    }

    void unref() {
        --refcnt_;
    }

    static std::pair<std::string, std::string> splitFromTail(const std::string &name);
    static bool isValidFullModuleName(const std::string &name);
    static bool isValidModuleName(const std::string &name);
    static bool isValidFuncName(const std::string &name);
    static bool isValidPluginName(const std::string &name);
    static bool isValidPluginApiVersion(uint32_t pluginApiVersion, bool strict);

private:
    std::atomic<size_t> refcnt_{1};
};

}  // namespace module
}  // namespace nebula
