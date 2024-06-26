// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include "nebula/plugins/ProcedurePlugin.h"

namespace yj {

class YJProcedurePlugin : public nebula::plugin::ProcedurePlugin {
public:
    static const constexpr char* kName = "yj";
    static const constexpr char* kAuthor = "yj";
    static const constexpr char* kDescription = "";
    static const constexpr char* kLicense = PLUGIN_LICENSE_PROPRIETARY;
    static const constexpr char* kVersion = "5.0";

public:
    YJProcedurePlugin();
};

}  // namespace yj
