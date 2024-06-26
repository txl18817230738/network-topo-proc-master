// Copyright (c) 2024 vesoft inc. All rights reserved.

#pragma once
#include "nebula/common/base/StatusOr.h"
#include "nebula/common/thread/GenericThreadPool.h"
#include "nebula/plugins/Plugin.h"

namespace nebula {
namespace meta {
class MetaStore;
}
namespace plugin {

/**
 * Base class for Plugins that provides customizable authenticator implementations.
 */
class AuthenticatorPlugin : public Plugin {
public:
    explicit AuthenticatorPlugin(const PluginInfo& info) : plugin::Plugin(info) {}

    Status destroy() override {
        return Plugin::destroy();
    }

    virtual Status stop() {
        return Status::OK();
    }

    // TODO(yee): introduce the plugin context to init the plugin
    Status init(const std::unordered_map<std::string, std::string>& config) override {
        return Plugin::init(config);
    }

    // reuse authenticatorManager's worker thread
    virtual Status init(thread::GenericWorker* worker) {
        UNUSED(worker);
        return Status::OK();
    }

    virtual std::string name() = 0;

    /**
     * check authentication information, return checked authenticator information to be stored
     */
    virtual StatusOr<std::string> check(const std::string& username,
                                        const std::string& authInfo) = 0;


    /**
     * authInfo: original authenticator information
     * updateAuthInfo : authenticator information to be updated
     * every plugin needs to decode authInfo, then update the authenticate information
     * and return the updated authenticate information
     */
    virtual StatusOr<std::string> update(const std::string& username,
                                         const std::string& encodedInfo,
                                         const std::string& authInfo) = 0;

    /**
     * return desensitized authenticator information, invoke by listUser
     */
    virtual StatusOr<std::string> list(const std::string& authInfo) = 0;


    /**
     * expected : encoded authenticate information
     * authInfo : authenticate information to be vertify
     * return the verification result
     */
    virtual Status authenticate(const std::string& clientHost,
                                const std::string& username,
                                const std::string& expectedAuthInfo,
                                const std::string& authInfo) = 0;
};

}  // namespace plugin
}  // namespace nebula
