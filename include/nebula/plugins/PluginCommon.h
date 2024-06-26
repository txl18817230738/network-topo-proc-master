// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <memory>

#include <folly/futures/Future.h>

#include "nebula/common/base/Status.h"
#include "nebula/common/datatype/Value.h"
#include "nebula/common/response/ExecutionOutcome.h"
#include "nebula/common/valuetype/ValueType.h"

using nebula::Status;

namespace nebula {
class RefCatalog;

namespace computing {
class ComputingEngine;
}  // namespace computing

namespace exec {
class ExecutionContext;
}

namespace gql {
class RequestContext;
}

namespace plugin {

/**
 * struct used to specify the prototype of a parameter of a procedure or function
 */
struct Parameter final {
    ValueTypePtr type;
    std::string name;
    std::string comment;

    std::string toString() const {
        return fmt::format("{}:{}", name, type->toString());
    }
};

/**
 * struct used to specify the prototype of a field of the result of a procedure or function
 */
struct Field final {
    ValueTypePtr type;
    std::string name;

    std::string toString(bool verbose = true) const {
        if (!verbose) return colName();
        return fmt::format("{}:{}", colName(), type->toString());
    }

    std::string colName() const {
        if (name.find(' ') == std::string::npos) {
            return name;
        }
        return fmt::format("`{}`", name);
    }
};


/**
 * Container of some context infos used during the execution of a procedure or function
 */
class ProcContext final {
public:
    explicit ProcContext(exec::ExecutionContext *ectx) : ectx_(CHECK_NOTNULL(ectx)) {}

    computing::ComputingEngine *computingEngine() const;

    RefCatalog *refCatalog() const;

    std::shared_ptr<gql::RequestContext> rctx() const;

private:
    exec::ExecutionContext *ectx_;
};

using ProcContextPtr = std::shared_ptr<ProcContext>;

/**
 * Prototype of an user-defined procedure
 */
struct Procedure final {
    using AsyncExecutionOutcome = folly::Future<ExecutionOutcome>;

    std::string name;
    std::vector<Parameter> params;
    std::vector<Field> fields;
    std::function<AsyncExecutionOutcome(ProcContextPtr ctx, std::vector<Value> args)> func;
    std::string comment;

    std::string toString() const {
        std::vector<std::string> paramStrList, returnStrList;
        for (const auto &param : params) {
            paramStrList.emplace_back(param.toString());
        }
        for (const auto &ret : fields) {
            returnStrList.emplace_back(ret.toString());
        }
        return fmt::format("PROCEDURE {}({}) RETURN ({})",
                           name,
                           folly::join(", ", paramStrList),
                           folly::join(", ", returnStrList));
    }
};

/**
 * Prototype of an user-defined function
 */
struct Function final {
    std::string name;
    std::vector<Parameter> params;
    ValueTypeKind returnType;
    std::function<Value(const ProcContext *, std::vector<Value> &args)> func;
    std::string comment;
};

}  // namespace plugin
}  // namespace nebula
