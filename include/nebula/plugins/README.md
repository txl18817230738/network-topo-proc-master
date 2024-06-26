# Nebula Graph C++ Plugin Development Guide

This document provides step-by-step instructions on how to develop a C++ plugin
for Nebula Graph. We will use the BFS algorithm as an example throughout this
guide.

## 1. Understanding the Procedure Structure

Before we start, it's essential to understand the `Procedure` structure in
Nebula Graph:

```c++
struct Procedure final {
    using AsyncExecutionOutcome = folly::Future<ExecutionOutcome>;

    std::string name;
    std::vector<Parameter> params;
    std::vector<Field> fields;
    std::function<AsyncExecutionOutcome(ProcContextPtr ctx, std::vector<Value> args)> func;
    std::string comment;
};
```

The `Procedure` structure consists of:

- `name`: The name of the procedure. This name is used in GQL's `CALL` statement
  to call the procedure, similar to `CALL algo.bfs(g, src) RETURN *`, where
  `bfs` is the name of the procedure.
- `params`: The input parameters of the procedure. The `Parameter` structure is
  defined as follows:

    ```c++
    struct Parameter final {
        ValueTypeKind type; // Parameter type
        std::string name; // Parameter name
        std::string comment; // Parameter comment
    };
    ```

- `fields`: The return result types of the procedure. Each column can have a
  `Field` structure as follows:

    ```c++
    struct Field final {
        ValueTypeKind type;  // Column return type
        std::string name;  // Column name, can be directly referenced in subsequent YIELD clause in CALL statement
    };
    ```

- `func`: The real procedure function definition, which saves the corresponding
  function pointer. When the procedure is called, this function will be
  executed. It is an asynchronous function, returning a
  `folly::Future<ExecutionOutcome>` result. All outputs of the procedure must be
  returned through the `ExecutionOutcome` type's `result` table:

    ```c++
    struct ExecutionOutcome {
        Status status;
        std::optional<ResultTable> result;  // optional
        std::optional<std::string> planDesc;
    };
    ```

- `comment`: A brief description of the procedure.

In addition, the `ProcContextPtr` parameter is used to pass some runtime
information from the Nebula core to the procedure:

```c++
class ProcContext final {
public:
    computing::ComputingEngine *computingEngine() const;
    RefCatalog *refCatalog() const;
    std::shared_ptr<gql::RequestContext> rctx() const;private:
};

using ProcContextPtr = std::shared_ptr<ProcContext>;
```

TODO: For more information about the `ValueTypeKind` types, refer to the Nebula
data types documentation.

## 2. Implementing the Procedure

Here is an example of a procedure implementation using the BFS algorithm:

```c++
static constexpr const char *kColumnNames[] = {
        "node_id",
        "distance",
};

// The bfs procedure implementation
static folly::Future<ExecutionOutcome> bfs(std::shared_ptr<ProcContext> pctx,
                                           std::vector<Value> args) {
    ExecutionOutcome outcome;
    outcome.status = Status::OK();

    if (args.size() < 2u || !args[0].isRef() || !args[1].isInt64()) {
        return outcome;
    }

    auto engine = pctx->computingEngine();
    const nebula::Ref &ref = args[0].getRef();
    auto memGraph = pctx->refCatalog()->getGraph(ref.entryID());
    auto ctx = std::make_unique<ComputingContext>(engine, memGraph.get(), pctx->rctx());

    // Call the BFS computing algorithm
    auto bfs = std::make_unique<BFS>(ctx.get(), args[1].getInt64());
    bfs->run();

    ResultTable table;
    std::vector<std::string> colNames;
    for (auto &col : kColumnNames) {
        colNames.emplace_back(col);
    }
    table.setColumnNames(std::move(colNames));

    // Collect the output result of BFS algorithm
    bfs->getResult(&table);

    outcome.result.emplace(std::move(table));
    return outcome;
}

// The BFS procedure declaration
Procedure declareBfsProcedure() {
    Procedure proc;
    proc.name = "bfs";
    proc.comment = "bfs";
    // Point to the above bfs implementation
    proc.func = &bfs;
    // The parameters declaration of BFS procedure
    proc.params = {
            Parameter{
                    ValueTypeKind::kRef,
                    "graphName",
                    "graph name",
            },
            Parameter{
                    ValueTypeKind::kInt64,
                    "srcId",
                    "source node id",
            },
    };
    // The return types declaration of BFS procedure
    proc.fields = {
            Field{ValueTypeKind::kInt64, kColumnNames[0]},
            Field{ValueTypeKind::kInt64, kColumnNames[1]},
    };
    return proc;
}
```

For more information about the graph computing interface, please refer to the
`include/computing/README.md` documentation.

## 3. Implementing the Procedure Plugin

All procedure plugins must inherit from the `nebula::plugin::ProcedurePlugin`
base type, as shown in the following example:

```c++
extern Procedure declareBFSProcedure();

class BFSProcedurePlugin : public nebula::plugin::ProcedurePlugin {
public:
    static const constexpr char* kName = "BFS algorithm plugin";
    static const constexpr char* kAuthor = "me";
    static const constexpr char* kDescription = "my plugin";
    static const constexpr char* kLicense = PLUGIN_LICENSE_PROPRIETARY;
    static const constexpr char* kVersion = "5.0";

public:
    MyProcedurePlugin(): ProcedurePlugin(PluginInfo{
        PluginType::PROCEDURE,
        kName,
        kAuthor,
        kDescription,
        kLicense,
        kVersion,
        NEBULA_PLUGIN_API_VERSION
    }) {
    addProcedure(declareBFSProcedure());
  }
};
```

In `BFSProcedurePlugin`, you need to register the type declaration of
`BFSProcedure` using `addProcedure`.

By following the above steps, you should be able to successfully develop your
own Nebula Graph C++ plugin. Once your plugin source code is ready, place it
under the SDK's development environment. This will successfully compile the
corresponding shared library for the Nebula processes to load and call.
