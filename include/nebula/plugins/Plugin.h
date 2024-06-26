// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <string>

#include <folly/dynamic.h>
#include <folly/json.h>

#include "nebula/common/base/Logging.h"
#include "nebula/common/base/Status.h"

namespace nebula {
namespace plugin {

/**
 * NEBULA_PLUGIN_API_VERSION encoded in 4 bytes (uint32_t),
 * two bytes for major version, and two bytes minor version.
 *
 * `NEBULA_PLUGIN_API_VERSION` syncs with nebula server version.
 *
 *  Plugin api version check:
 *  - FLAGS_plugin_api_version_check_strict = true (default)
 *    plugin api version and nebula server must be the same.
 *
 *  - FLAGS_plugin_api_version_check_strict = false:
 *    plugin api version support backward compatibility between
 *    minor versions. newer nebula server could load older plugin
 *    dynamic library
 */
#define NEBULA_PLUGIN_API_VERSION (5 << 16 | 0)

#define PLUGIN_LICENSE_PROPRIETARY "PROPRIETARY"
#define PLUGIN_LICENSE_GPL "GPL"
#define PLUGIN_LICENSE_BSD "BSD"
#define PLUGIN_LICENSE_APACHE_2 "Apache2"
#define PLUGIN_LICENSE_MIT "MIT"

enum class PluginType {
    AUDIT,
    FUNCTION,
    PROCEDURE,
    LOG_ROTATE,
    LICENSE_MANAGER,
    STORAGE_ENGINE,
    GENERIC,
    AUTHENTICATOR,
};

/**
 * Plugin info describe plugin's uniq identity when registering into Plugin system.
 */
class PluginInfo {
public:
    PluginType type;           /* type of plugin,                */
    std::string name;          /* uniq name of plugin            */
    std::string author;        /* author of plugin               */
    std::string description;   /* a descriptive string of plugin */
    std::string license;       /* license of plugin              */
    std::string pluginVersion; /* version of plugin              */
    uint32_t apiVersion;       /* version of nebula plugin api   */

    static std::string pluginTypeString(PluginType pluginType) {
        switch (pluginType) {
            case PluginType::AUDIT:
                return "AUDIT";
            case PluginType::FUNCTION:
                return "FUNCTION";
            case PluginType::PROCEDURE:
                return "PROCEDURE";
            case PluginType::LOG_ROTATE:
                return "LOG_ROTATE";
            case PluginType::LICENSE_MANAGER:
                return "LICENSE_MANAGER";
            case PluginType::STORAGE_ENGINE:
                return "STORAGE_ENGINE";
            case PluginType::GENERIC:
                return "GENERIC";
            case PluginType::AUTHENTICATOR:
                return "AUTHENTICATOR";
            default:
                return "";
        }
    }

    folly::dynamic toJson() const {
        folly::dynamic info = folly::dynamic::object;
        info["type"] = pluginTypeString(type);
        info["author"] = author;
        info["description"] = description;
        info["license"] = license;
        info["pluginVersion"] = pluginVersion;
        info["apiVersion"] = apiVersion;
        return info;
    }

    std::string toString() const {
        return folly::toJson(toJson());
    }

    std::string getApiVersion() const {
        return fmt::format("{}.{}", apiVersion >> 16, apiVersion & 0xff);
    }
};

/**
 * A plugin is an API enables creation of customizable server components.
 * Plugins can be loaded at server startup, or reloaded and unloaded at runtime
 * without restarting the server.
 *
 * The plugins supported by this interface include, but are not limited to,
 * audit, udf, procedures, storage engine.
 */
class Plugin {
public:
    explicit Plugin(PluginInfo info) : info_(info) {}

    virtual ~Plugin() = default;

    /**
     * Get plugin init, PluginInfo specify the meta data of a Plugin.
     * @return PluginInfo
     */
    const PluginInfo& info() {
        return info_;
    }

    /**
     * Initialize this plugin, this function should be invoked, before this Plugin put into use.
     *
     * @return Status indicate init success of not, a plugin is functional only if init success.
     */
    virtual Status init(const std::unordered_map<std::string, std::string>&) {
        return Status::OK();
    }

    /**
     * Destroy this plugin, release any resources.
     *
     * @return Status indicate release success of not.
     */
    virtual Status destroy() {
        return Status::OK();
    }

    /**
     * Return a descriptive json object of this plugin.
     *
     * @return json object
     */
    virtual folly::dynamic toJson() const {
        return info_.toJson();
    }

protected:
    PluginInfo info_;
};

}  // namespace plugin
}  // namespace nebula
