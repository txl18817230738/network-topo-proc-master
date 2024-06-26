// Copyright (c) 2023 vesoft inc. All rights reserved.

#pragma once

#include <bitset>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>

#include <folly/io/IOBuf.h>

#include "nebula/common/base/Status.h"
#include "nebula/common/datatype/Row.h"
#include "nebula/common/datatype/RowType.h"
#include "nebula/common/datatype/Value.h"
#include "nebula/common/memory/Allocator.h"
#include "nebula/common/memory/MemoryTracker.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"
#include "nebula/common/valuetype/ValueType.h"

namespace nebula {

class RowTable;
class MemGraph;

namespace vector {
class Chunk;
using ChunkPtr = std::shared_ptr<Chunk>;
class ChunkList;
using ChunkListPtr = std::shared_ptr<ChunkList>;
class VectorTable;
}  // namespace vector

enum ScenarioHint : uint8_t {
    kAnalytical = 0x0,
    kTransactional = 0x1,
    kTopological = 0x2,
    // ... more to be added
};

class BindingTableFactory;

template <typename Table, typename ChunkA, typename ChunkB = void>
class TableChecker {
public:
    bool checkTable(Table* a, Table* b);
    bool checkChunk(ChunkA* a, ChunkA* b);
    bool checkChunk(ChunkB* a, ChunkB* b);
};

// Set the following value to 16KiB, which is equal to the size of a page in InnoDB.
// It is the base chunk size. A BindingTable may choose to use a multiple of it in practice.
#define MINIMUM_MT_CHUNK_SIZE 4096
#define BASE_MT_CHUNK_SIZE 16384
// set the maximum chunk size to 4 MiB
#define MAXIMUM_MT_CHUNK_SIZE 4194304
// the default size for a BitSet within a given chunk
#define DEFAULT_MT_CHUNK_BITSET_SIZE (BASE_MT_CHUNK_SIZE / 4)
// the default type for PK and indexes to the table (e.g., the 100-th row in the RowTable)
// this replaces the usages of size_t in many interface methods
using DEFAULT_PK_TYPE = std::uint64_t;
// use MT_SIZE for the length for the following structures:
// (1) the number of characters in a string, without counting the \0 character
// (2) the number of elements in a List
// (3) the number of properties in a Node, Edge, Path, Record, etc.
using MT_SIZE = std::uint16_t;
// thus, their maximum size is:
#define MAXIMUM_MT_SIZE 65535
#define ALIGN_64(x) (((x) + 63) & 0xFFFFFFFFFFFFFFC0)
static constexpr uint16_t kRowTableChunkContextCode = 0x4352;  // 'RC'
static constexpr uint16_t kRowTableChunkContextCodeCopy = 0x4353;
static constexpr uint16_t kVectorBatchContextCode = 0x8486;  // 'VT'

/**
 * @brief MemoryTableStatus
 * NO_SPACE means there is no spaces for the an allocation called
 * by the corresponding method. Measures shall be taken to address it.
 *
 */

// the length of a single bitmap (as std::bitset) that forms the
// BitSet class below.
#define MaxSingleBitMapLength 512

// The data type for the constructed internal keys in keyColumn_
// TODO(xuntao): replace it with a customized key class for arbitrary sizes
// and possibly other functions.
#define COLUMNAR_KEY_TYPE std::uint_fast64_t
using KEY_OFFSET_TYPE = uint64_t;
using InternalOffset = KEY_OFFSET_TYPE;
/**
 * @brief Convert the type of a Value to the ValueTypeKind format.
 *        Although I have listed all the types here, I'm only implementing for basic
 *        types for now. TODO(xuntao): support all types.
 *
 * @param e
 * @return ValueTypeKind
 */
ValueTypeKind getValueType(Value e);

// std::string typeToString(ValueTypeKind e);

class BindingTableUtil final {
public:
    static size_t calculateChunkSize(size_t chunkID,
                                     ScenarioHint hint = ScenarioHint::kTransactional) {
        size_t size = 0;
        switch (hint) {
            case ScenarioHint::kTransactional:
                if (chunkID < 8) {
                    size = MINIMUM_MT_CHUNK_SIZE;
                } else if (chunkID < 64) {
                    size = BASE_MT_CHUNK_SIZE;
                } else {
                    size = 64 * BASE_MT_CHUNK_SIZE;
                }
                break;
            case ScenarioHint::kAnalytical:
                if (chunkID < 64) {
                    size = BASE_MT_CHUNK_SIZE;
                } else {
                    size = 64 * BASE_MT_CHUNK_SIZE;
                }
                break;
            case ScenarioHint::kTopological:
                size = 64 * BASE_MT_CHUNK_SIZE;
                break;
            default:
                size = BASE_MT_CHUNK_SIZE;
        }
        return size;
    }
};

class BitSet final {
public:
    BitSet() = default;
    explicit BitSet(size_t maximumNumBits) {
        init(maximumNumBits);
    }
    ~BitSet() {
        auto allocator = nebula::memory::Allocator<nebula::memory::Operator>(
                "BindingTable::BitSet", memory::currentTracker);
        allocator.deallocate(bitmaps_, bytes_);
    }
    void init(size_t maximumNumBits) {
        bytes_ = (maximumNumBits + 7) >> 3;
        auto allocator = nebula::memory::Allocator<nebula::memory::Operator>(
                "BindingTable::BitSet", memory::currentTracker);
        bitmaps_ = static_cast<uint8_t*>(allocator.allocate(bytes_));
        std::memset(bitmaps_, 0x00, bytes_);
    }
    void extend(size_t byteSize);
    // set the index-th bit to true
    void set(size_t index);
    // set the index-th bit to false
    void reset(size_t index);
    // count the number of bits that have been set to true
    size_t count() const;
    size_t size() const {
        return size_;
    }
    size_t bytes() const;

    bool operator[](size_t index) const {
        uint8_t byte = bitmaps_[index / 8];
        uint8_t pos = index % 8;
        return (byte & (0x80 >> pos)) >> (7 - pos);
    }
    bool get(size_t recordIndex, size_t columnIndex) const;

    BitSet& operator<<=(size_t pos) noexcept;
    bool operator==(const BitSet& other) const noexcept {
        if (count() != other.count()) return false;
        if (size() != other.size()) return false;
        if (memcmp(bitmaps_, other.bitmaps_, bytes_) != 0) return false;
        return true;
    }
    bool operator!=(BitSet& other) const noexcept {
        return !(*this == other);
    }
    void append(BitSet& other);
    uint8_t* bitmaps() const {
        return bitmaps_;
    }
    void append(BitSet& other, size_t recordIndex);

private:
    friend struct nrpc::BufferReaderWriter<BitSet>;
    size_t size_{0};
    size_t bytes_{0};
    uint8_t* bitmaps_{nullptr};
};

template <>
struct nrpc::BufferReaderWriter<BitSet> {
    static void write(folly::IOBuf* buf, const char* str, size_t len) {
        buf->reserve(0, len);
        ::memcpy((void*)buf->writableTail(), str, len);  // NOLINT
        buf->append(len);
    }

    static Status read(folly::IOBuf* buf, char* str, size_t len) {
        if (buf->length() < len) {
            return {ErrorCode::RPC_DESERIALIZE_ERROR,
                    "Data not enough, 4B required, %lu left",
                    buf->length()};
        }
        ::memcpy(str, (void*)buf->data(), len);  // NOLINT
        buf->trimStart(len);
        return Status::OK();
    }

    static void write(folly::IOBuf* buf, const BitSet& bitSet) {
        BufferReaderWriter<size_t>::write(buf, bitSet.size_);
        BufferReaderWriter<size_t>::write(buf, bitSet.bytes_);
        write(buf, reinterpret_cast<const char*>(bitSet.bitmaps_), bitSet.bytes_);
    }

    static Status read(folly::IOBuf* buf, BitSet& bitSet) {
        BufferReaderWriter<size_t>::read(buf, &bitSet.size_);
        size_t neededBytes = 0;
        BufferReaderWriter<size_t>::read(buf, &neededBytes);
        if (neededBytes > bitSet.bytes()) {
            bitSet.extend(neededBytes);
        }
        read(buf, reinterpret_cast<char*>(bitSet.bitmaps_), neededBytes);
        return Status::OK();
    }

    static size_t encodedSize(const BitSet& bitSet) {
        return sizeof(size_t) * 2 + bitSet.bytes_;
    }
};

template <typename T>
class BasicChunk {
public:
    BasicChunk() = default;
    ~BasicChunk() = default;

    T* prev() const {
        return prevChunkPtr_;
    }
    T* next() const {
        return nextChunkPtr_;
    }
    T* prevChunkPtr_{nullptr};
    T* nextChunkPtr_{nullptr};
};

template <typename T>
class LinkedChunks {
public:
    explicit LinkedChunks(bool isSparse = false) : isSparse_(isSparse) {}
    ~LinkedChunks() {
        T* ptr = firstChunk_;
        while (ptr != nullptr) {
            auto temp = ptr;
            ptr = ptr->nextChunkPtr_;
            delete temp;
        }
    }

    T* addNewChunk(size_t chunkSize = 0) {
        // create a new chunk first
        if (chunkSize == 0) {
            chunkSize = BindingTableUtil::calculateChunkSize(numChunks_);
        }
        DCHECK_LE(chunkSize, MAXIMUM_MT_CHUNK_SIZE);
        if (chunkSize > MAXIMUM_MT_CHUNK_SIZE) {
            DLOG(ERROR) << "Attempting to allocate a chunk that is too large and not supported "
                           "yet. Requested size: "
                        << chunkSize << " bytes.";
        }
        T* newChunk = new T(chunkSize);
        if (newChunk == nullptr) {
            LOG(ERROR) << "Failed to allocate a new chunk for BindingTable.";
            return nullptr;
        }
        newChunk->nextChunkPtr_ = nullptr;
        if (firstChunk_ == nullptr) {
            firstChunk_ = newChunk;
        }
        if (lastChunk_ != nullptr) {
            lastChunk_->nextChunkPtr_ = newChunk;
        }
        newChunk->prevChunkPtr_ = lastChunk_;
        lastChunk_ = newChunk;
        totalBytes_ += chunkSize;
        numChunks_++;
        return newChunk;
    }

    T* emplaceNewChunk() {
        T* newChunk = new T();
        newChunk->nextChunkPtr_ = nullptr;
        if (firstChunk_ == nullptr) {
            firstChunk_ = newChunk;
            newChunk->prevChunkPtr_ = nullptr;
        } else if (lastChunk_ != nullptr) {
            lastChunk_->nextChunkPtr_ = newChunk;
            newChunk->prevChunkPtr_ = lastChunk_;
        }
        lastChunk_ = newChunk;
        numChunks_++;
        return newChunk;
    }

    inline T* getFirstChunk() const {
        return firstChunk_;
    }

    void setFirstChunk(T* chunk) {
        firstChunk_ = chunk;
    }
    void setLastChunk(T* chunk) {
        lastChunk_ = chunk;
    }

    T* getLastChunk() const {
        return lastChunk_;
    }

    T* getNextChunk(T* chunk) const {
        return chunk->nextChunkPtr_;
    }

    size_t numChunks() const {
        return numChunks_;
    }

    void setNumChunks(size_t num) {
        numChunks_ = num;
    }

    size_t bytes() const {
        return totalBytes_;
    }

protected:
    friend class BindingTableFactory;

    // need to be passed via nRPC
    size_t numChunks_{0};
    DEFAULT_PK_TYPE numRecords_{0};

    // does not need to pass via nRPC
    // they would be constructed dynamically while deserializing a nRPC buffer
    size_t totalBytes_{0};
    T* firstChunk_{nullptr};
    T* lastChunk_{nullptr};

    // unused for now
    bool isSparse_{false};
};

enum BindingTableKind : uint8_t {
    kRowTable = 0,
    kVectorTable = 1,
    kMemGraph = 2,
    kNullTable = 0xFF,
};

enum BindingTableRPCMode : uint8_t {
    kCopy = 0,
    kNoCopy = 1,
    kFull = 2,
};

class BindingTableDescriptor final {
public:
    BindingTableDescriptor() = default;

    explicit BindingTableDescriptor(const RowType& rowType,
                                    bool ordered = false,
                                    bool duplicateFree = false)
            : recordType_(rowType), ordered_(ordered), duplicateFree_(duplicateFree) {}


    const RowType& getRowType() const {
        return recordType_;
    }

    FOLLY_ALWAYS_INLINE bool empty() const {
        return recordType_.empty();
    }

    // FIXME(jie): The binding table desc and record type are too complicated and too hard to
    // use. need to simplify them.
    std::vector<std::string> getColumnNames() const {
        return recordType_.names();
    }
    size_t getNumColumns() const {
        return recordType_.size();
    }

    void setColumnNames(const std::vector<std::string>& colNames);

    void setRecordType(const RowType& rowType) {
        recordType_ = rowType;
    }

    ValueTypePtr getValueTypeByName(const std::string& name) const {
        if (recordType_.names().empty()) {
            return nullptr;
        }
        auto rowIdx = recordType_.getIndex(name);
        if (rowIdx == -1) {
            return nullptr;
        }
        return recordType_.types()[rowIdx];
    }

    void appendCol(FieldType field) {
        recordType_.append(std::move(field));
    }

    bool isOrdered() const {
        return ordered_;
    }

    void setOrdered(bool ordered) {
        ordered_ = ordered;
    }

    bool isDuplicateFree() const {
        return duplicateFree_;
    }
    void setDuplicateFree(bool duplicateFree) {
        duplicateFree_ = duplicateFree;
    }

    // Merge Descriptor of other table
    void merge(const BindingTableDescriptor& other) {
        ordered_ = ordered_ && other.ordered_;
        duplicateFree_ = duplicateFree_ && other.duplicateFree_;
        recordType_.merge(other.recordType_);
    }

    void clear() {
        recordType_.clear();
        ordered_ = false;
        duplicateFree_ = false;
    }

    void setHint(ScenarioHint hint) {
        hint_ = hint;
    }

    ScenarioHint hint() const {
        return hint_;
    }

    std::string toString() const {
        return recordType_.toString();
    }

private:
    friend struct nrpc::BufferReaderWriter<BindingTableDescriptor>;

    // Record type of the binding table.
    // It could be explicitly defined by the user or inferred from the schema,
    // and maybe changed during the building of the binding table.
    RowType recordType_;
    // An indication whether the binding table is declared as ordered or as unordered.
    // e.g. The binding table generated by a sort executor should be declared as ordered.
    bool ordered_{false};
    // An indication whether the binding table is declared as duplicate-free or as allowing
    // duplicates. A duplicate-free binding table shall not contain duplicates of the same
    // record, that is it either contains some arbitrary record R of its declared record type
    // exactly once or it does not contain R at all.
    // e.g. The binding table generated by a distinct executor should be declared as
    // duplicate-free.
    bool duplicateFree_{false};
    // An optional preferred column sequence that is a permutation of the column names of the
    // binding table.
    // std::optional<PreferredColumnSequence> preferredColumnSequence_;
    ScenarioHint hint_{kAnalytical};
};

class BindingTable {
public:
    BindingTable();
    virtual ~BindingTable() = default;
    // BindingTable class has no read/write methods
    // metadata and utility methods
    virtual Status reserve(size_t size) = 0;
    virtual size_t getNumRecords() const = 0;
    virtual size_t bytes() const = 0;
    virtual size_t numCols() const = 0;
    // Print the table to a human readable string.
    // Mainly for debugging purpose.
    const std::string toString(size_t num = std::numeric_limits<size_t>::max()) const;
    void setDescriptor(BindingTableDescriptor&& desc) {
        desc_ = std::move(desc);
    }
    void setDescriptor(const BindingTableDescriptor& desc) {
        desc_ = desc;
    }
    const BindingTableDescriptor& getDescriptor() const {
        return desc_;
    }
    BindingTableDescriptor& getDescriptor() {
        return desc_;
    }
    virtual void setColumnNames(const std::vector<std::string>& colNames) = 0;
    void setKind(BindingTableKind kind) {
        kind_ = kind;
    }
    BindingTableKind kind() const {
        return kind_;
    }
    BindingTableRPCMode rpcMode() const {
        return rpcMode_;
    }
    void setRPCMode(BindingTableRPCMode mode) {
        rpcMode_ = mode;
    }
    virtual BindingTableRPCMode determineRPCMode() = 0;
    uint32_t ID() const {
        return tableID_;
    }
    vector::VectorTable* asVectorTable();
    const vector::VectorTable* asVectorTable() const;
    RowTable* asRowTable();
    const RowTable* asRowTable() const;
    MemGraph* asMemGraph();
    const MemGraph* asMemGraph() const;

protected:
    friend struct nrpc::BufferReaderWriter<BindingTable>;

    BindingTableKind kind_;
    BindingTableRPCMode rpcMode_{BindingTableRPCMode::kCopy};
    BindingTableDescriptor desc_;

private:
    uint32_t tableID_;
};

using BindingTablePtr = std::unique_ptr<BindingTable>;

bool operator==(const BindingTableDescriptor& lhs, const BindingTableDescriptor& rhs);
bool operator!=(const BindingTableDescriptor& lhs, const BindingTableDescriptor& rhs);
std::ostream& operator<<(std::ostream& os, const BindingTableDescriptor& table);

template <>
struct nrpc::BufferReaderWriter<BindingTableDescriptor> {
    static void write(folly::IOBuf* buf, const BindingTableDescriptor& table) {
        BufferReaderWriter<RowType>::write(buf, table.recordType_);
        BufferReaderWriter<bool>::write(buf, table.ordered_);
        BufferReaderWriter<bool>::write(buf, table.duplicateFree_);
    }

    static Status read(folly::IOBuf* buf, BindingTableDescriptor* desc) {
        auto status = BufferReaderWriter<RowType>::read(buf, &desc->recordType_);
        if (!status.ok()) {
            return status;
        }
        status = BufferReaderWriter<bool>::read(buf, &desc->ordered_);
        if (!status.ok()) {
            return status;
        }
        return BufferReaderWriter<bool>::read(buf, &desc->duplicateFree_);
    }

    static size_t encodedSize(const BindingTableDescriptor& desc) {
        return BufferReaderWriter<RowType>::encodedSize(desc.recordType_) +
               BufferReaderWriter<bool>::encodedSize(desc.ordered_) +
               BufferReaderWriter<bool>::encodedSize(desc.duplicateFree_);
    }
};

}  // namespace nebula
