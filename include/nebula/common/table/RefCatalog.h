// Copyright (c) 2023 vesoft inc. All rights reserved.

#include "folly/Synchronized.h"
#include "nebula/common/datatype/Ref.h"
#include "nebula/common/table/vector/VectorTypeDefs.h"

#pragma once

namespace nebula {

class BindingTable;
using BindingTablePtr = std::unique_ptr<BindingTable>;
class MemGraph;
using GraphPtr = std::shared_ptr<MemGraph>;

namespace vector {
class BaseVector;
}  // namespace vector


/**
 * @brief RefCatalog is a list of all live vectors. A vector is registered here in the
 * catalog upon its creation, and deregistered here upon its destruction. Each vector in this
 * catalog has an unique ID that others can use to reference this vector indirecly to avoid
 * doing so using raw pointers which may be too large to store or become invalid.
 *
 * TODO(Xuntao): For the time being, restrict it to the scope of a VectorTable only. Make it
 * global with specif requirement later.
 *
 */
class RefCatalog final {
public:
    RefCatalog() = default;
    size_t numVectors() {
        return vectorRegistry_.rlock()->size();
    }

    void registerVector(vector::BaseVector* vector);
    void deregisterVector(vector::VectorUID vectorID);
    vector::BaseVector* getVector(vector::VectorUID vectorID);

    void registerTable(BindingTablePtr table);
    void deregisterTable(RefEntryID tableID);
    BindingTable* getTable(RefEntryID tableID);

    void registerGraph(GraphPtr graph);
    void deregisterGraph(RefEntryID graphID);
    GraphPtr getGraph(RefEntryID graphID);

private:
    folly::Synchronized<std::unordered_map<vector::VectorUID, vector::BaseVector*>>
            vectorRegistry_;
    folly::Synchronized<std::unordered_map<RefEntryID, BindingTablePtr>> tableRegistry_;
    folly::Synchronized<std::unordered_map<RefEntryID, std::shared_ptr<MemGraph>>>
            graphRegistry_;
};

}  // namespace nebula
