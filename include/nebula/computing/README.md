# Nebula Graph Computing Algorithm Developer Guide

This guide provides a comprehensive overview of developing graph computing
algorithms in Nebula. The implementation of our algorithms is inspired by the
concepts presented in the [Ligra](https://people.csail.mit.edu/jshun/ligra.shtml) and
[FLASH](https://graphscope.io/docs/v0.25.0/analytical_engine/flash) papers. We
provide similar types and interfaces such as `VertexSubset` and `edgeMap` that
help users quickly implement their graph algorithms.

## Fundamentals

In Nebula, a `VertexSubset` is defined as a set of node IDs. The `edgeMap` and
other interfaces will be explained in detail later in this guide.

## Implementing Algorithms

All algorithms must inherit from the `nebula::ComputingAlgorithm<T>` base class,
which is a template class. The type in template represents the data structure of
the state on each vertex. For instance, consider the implementation of the
Breadth-First Search (BFS) algorithm:

```c++
class BFS final : public ComputingAlgorithm<BFSState> {
public:
    BFS(ComputingContext* ctx, NodeID srcId);

    void update(NodeID src, NodeID dst);
    bool cond(NodeID dst);
    void run() override;

    std::string name() const override {
        return "bfs";
    }

private:
    // the parameter of BFS algorithm
    NodeID srcId_;
};
```

In the BFS algorithm, the `BFSState` is defined as follows:

```c++
struct BFSState {
    int64_t distance{-1};

    // The interface is used to get the state of each vertex of the algorithm
    void getResult(Row& row) const {
        row.append(distance);
    }
};
```

During the iteration process of the graph computation, the state on each vertex
is saved using this type. For instance, in BFS, the distance from each vertex to
the source is stored. Each state type must implement a `getResult` interface to
return the results after the graph algorithm is complete.

All subclasses of the `ComputingAlgorithm<T>` must override the
`ComputingAlgorithm<T>::run()` interface, which is used to execute the specific
algorithm logic. The `ComputingAlgorithm::ctx_` defines a `ComputingContext`
that stores the `MemGraph` field. All graph computing algorithms run on
`MemGraph`. The `ComputingContext` is defined as follows:

```c++
class ComputingContext final {
public:
    ComputingEngine* engine() const;
    const MemGraph* graph() const ;
    std::shared_ptr<gql::RequestContext> rctx() const;
};
```

## API for Algorithm Implementation

The `ComputingAlgorithm` provides the following functions to help quickly implement algorithms:

### edgeMap

```
VertexSubset edgeMap(
  VertexSubset U,
  EdgeSet H,
  F(NodeID s, NodeID d) -> bool,
  M(NodeID s, NodeID d) -> T,
  C(NodeID v) -> bool,
  R(T t, NodeID d) -> NodeID
);
```

This function performs the following operations: for each vertex in the given
VertexSubset `U`, it finds some endpoints that meet the following conditions:
the adjacent edge is in set `H`; the source/destination meets the `F(s,d)`
filter, and the destination meets the `C(d)` filter; for each destination `d`,
it uses `R` to aggregate each `M(s,d)`.

### vertexMap

```
VertexSubset VertexMap(
  VertexSubset U,
  F(NodeID v) -> bool,
  M(NodeID v) -> NodeID
);
```

This function performs the following operations: for each `VertexSubset` in `U`,
it returns the set of vertices that meet the `F` filter and executes `M`.

### vSize

```
size_t VSize(VertexSubset U)
```

This function returns the number of elements in the `VertexSubset`.

## Example: BFS Algorithm Implementation

Here is an example of how to implement the BFS algorithm:

```c++
void BFS::update(NodeID src, NodeID dst) {
    auto& s = state(src);
    casOp<int64_t>(&state(dst).distance, -1, s.distance + 1);
}

bool BFS::cond(NodeID dst) {
    // return target vertex not initialized
    return state(dst).distance == -1;
}

void BFS::run() {
    VertexSubset frontier(ctx_);
    frontier.add(srcId_);
    state(srcId_).distance = 0;

    auto m = [this](auto src, auto dst) { update(src, dst); };
    auto c = [this](auto dst) { return cond(dst); };

    while (!frontier.empty()) {
        frontier = edgeMap(frontier, m, c);
    }
}
```

Note that the `m` and `c` functions in the algorithm must be thread-safe. The
`ComputingAlgorithm` provides some atomic operation utility functions that can
be used to simplify coding operations. The `state(s)` function retrieves the
state defined on the vertex `s` in the graph, similar to the `BFSState`
structure.

The BFS algorithm iterates over the `frontier` `VertexSubset` until it is empty.
Once this process is complete, the distance from each vertex to the source has
been calculated.

That's it! You can implement your graph computing algorithms in Nebula following
the above process.
