// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

#include "nebula/common/datatype/Date.h"
#include "nebula/common/datatype/Datetime.h"
#include "nebula/common/datatype/Null.h"
#include "nebula/common/datatype/Time.h"
#include "nebula/common/datatype/ValueTypeKind.h"
#include "nebula/common/datatype/ZonedDatetime.h"
#include "nebula/common/datatype/ZonedTime.h"
#include "nebula/common/memory/StlAllocator.h"
#include "nebula/common/nrpc/BufferReaderWriter.h"
#include "nebula/common/nrpc/Context.h"

namespace nebula {

class Ref;

using properties_type = NMap<String, Value>;

struct HashKeyAppender {
    std::string key;

    template <class T>
    FOLLY_ALWAYS_INLINE void append(const T& t) {
        key.append(reinterpret_cast<const char*>(&t), sizeof(T));
    }

    FOLLY_ALWAYS_INLINE void reset() {
        key.clear();
    }
};

class Value final {
public:
    using allocator_type = nebula::memory::StlAllocator<std::byte>;

    union Data {
        // Fixed length data types less or equal than 8 bytes
        NullValue* null_;
        bool bool_;
        int8_t int8_;
        int16_t int16_;
        int32_t int32_;
        int64_t int64_;
        uint8_t uint8_;
        uint16_t uint16_;
        uint32_t uint32_;
        uint64_t uint64_;
        float float_;
        double double_;
        // Variable length data types or fixed length data types greater than 8 bytes
        String* string_;
        String* bytes_;
        List* list_;
        Path* path_;
        Record* record_;
        Node* node_;
        Edge* edge_;
        // Temporal types
        // TODO(Aiee) Add timezone support
        Date date_;
        LocalTime localTime_;
        ZonedTime zonedTime_;
        LocalDatetime localDatetime_;
        ZonedDatetime zonedDatetime_;
        Ref* ref_;
        Duration* duration_;

        Data() {}
        ~Data() {}
    };

    static_assert(sizeof(Data) == sizeof(void*), "Data should be the same size as pointer.");

    Value(allocator_type = allocator_type());  // NOLINT
    Value(const NullValue& val);               // NOLINT
    Value(NullValue&& val);                    // NOLINT
    Value(bool val);                           // NOLINT
    Value(int8_t val);                         // NOLINT
    Value(int16_t val);                        // NOLINT
    Value(int32_t val);                        // NOLINT
    Value(int64_t val);                        // NOLINT
    Value(uint8_t val);                        // NOLINT
    Value(uint16_t val);                       // NOLINT
    Value(uint32_t val);                       // NOLINT
    Value(uint64_t val);                       // NOLINT
    Value(float val);                          // NOLINT
    Value(double val);                         // NOLINT
    Value(const char* val);                    // NOLINT
    Value(std::string_view val);               // NOLINT
    Value(const String& val);                  // NOLINT
    Value(String&& val) noexcept;              // NOLINT
    Value(const std::string& val);             // NOLINT
    Value(std::string&& val);                  // NOLINT
    Value(const List& val);                    // NOLINT
    Value(List&& val) noexcept;                // NOLINT
    Value(const Record& val);                  // NOLINT
    Value(Record&& val) noexcept;              // NOLINT
    Value(const Node& val);                    // NOLINT
    Value(Node&& val) noexcept;                // NOLINT
    Value(const Edge& val);                    // NOLINT
    Value(Edge&& val) noexcept;                // NOLINT
    Value(const Path& val);                    // NOLINT
    Value(Path&& val) noexcept;                // NOLINT
    Value(LocalTime val);                      // NOLINT
    Value(ZonedTime val);                      // NOLINT
    Value(Date val);                           // NOLINT
    Value(const Duration& val);                // NOLINT
    Value(Duration&& val) noexcept;            // NOLINT
    Value(LocalDatetime val);                  // NOLINT
    Value(ZonedDatetime val);                  // NOLINT
    Value(const Ref& val);                     // NOLINT
    Value(Ref&& val) noexcept;                 // NOLINT

    Value(const Value& other);
    Value(Value&& other) noexcept;

    ~Value();

    Value& operator=(const Value& other);
    Value& operator=(Value&& other) noexcept;

    ValueTypeKind getType() const {
        return type_;
    }

    ValueTypeKind setType(ValueTypeKind type) {
        type_ = type;
        return type_;
    }

    // Returns true if the value equals to the minimal value of its type can hold.
    bool isMin() const;
    // Returns true if the value equals to the maximal value of its type can hold.
    bool isMax() const;
    // Gets the next value if it exists.
    std::optional<Value> next() const;
    // Gets the previous value if it exists.
    std::optional<Value> prev() const;

    bool isAny() const {
        return true;
    }

    bool isNumeric() const {
        // return type_ & kNumericMask == Numeric;
        return isInteger() || isFloat() || isDouble();
    }

    bool isNull() const {
        return type_ == ValueTypeKind::kNull;
    }

    bool isBool() const {
        return type_ == ValueTypeKind::kBool;
    }

    bool isInt8() const {
        return type_ == ValueTypeKind::kInt8;
    }

    bool isInt16() const {
        return type_ == ValueTypeKind::kInt16;
    }

    bool isInt32() const {
        return type_ == ValueTypeKind::kInt32;
    }

    bool isInt64() const {
        return type_ == ValueTypeKind::kInt64;
    }

    bool isInteger() const {
        switch (type_) {
            case ValueTypeKind::kInt8:
            case ValueTypeKind::kInt16:
            case ValueTypeKind::kInt32:
            case ValueTypeKind::kInt64:
            case ValueTypeKind::kUint8:
            case ValueTypeKind::kUint16:
            case ValueTypeKind::kUint32:
            case ValueTypeKind::kUint64:
                return true;
            default:
                return false;
        }
    }

    bool isFloat() const {
        return type_ == ValueTypeKind::kFloat32;
    }

    bool isDouble() const {
        return type_ == ValueTypeKind::kFloat64;
    }

    bool isString() const {
        return type_ == ValueTypeKind::kString;
    }

    bool isList() const {
        return type_ == ValueTypeKind::kList;
    }

    bool isPath() const {
        return type_ == ValueTypeKind::kPath;
    }

    bool isRecord() const {
        return type_ == ValueTypeKind::kRecord;
    }

    bool isNode() const {
        return type_ == ValueTypeKind::kNode;
    }

    bool isEdge() const {
        return type_ == ValueTypeKind::kEdge;
    }

    bool isLocalTime() const {
        return type_ == ValueTypeKind::kLocalTime;
    }

    bool isZonedTime() const {
        return type_ == ValueTypeKind::kZonedTime;
    }

    bool isRef() const {
        return type_ == ValueTypeKind::kRef;
    }

    bool isDuration() const {
        return type_ == ValueTypeKind::kDuration;
    }

    bool isDate() const {
        return type_ == ValueTypeKind::kDate;
    }

    bool isLocalDatetime() const {
        return type_ == ValueTypeKind::kLocalDatetime;
    }

    bool isZonedDatetime() const {
        return type_ == ValueTypeKind::kZonedDatetime;
    }

    bool isNaN() const {
        return (type_ == ValueTypeKind::kFloat32 && std::isnan(data_.float_)) ||
               (type_ == ValueTypeKind::kFloat64 && std::isnan(data_.double_));
    }

    bool getBool() const {
        DCHECK_EQ(type_, kBool);
        return data_.bool_;
    }

    int8_t getInt8() const {
        DCHECK_EQ(type_, kInt8);
        return data_.int8_;
    }

    int16_t getInt16() const {
        DCHECK_EQ(type_, kInt16);
        return data_.int16_;
    }

    int32_t getInt32() const {
        DCHECK_EQ(type_, kInt32);
        return data_.int32_;
    }

    int64_t getInt64() const {
        DCHECK_EQ(type_, kInt64);
        return data_.int64_;
    }

    uint8_t getUint8() const {
        DCHECK_EQ(type_, kUint8);
        return data_.uint8_;
    }

    uint16_t getUint16() const {
        DCHECK_EQ(type_, kUint16);
        return data_.uint16_;
    }

    uint32_t getUint32() const {
        DCHECK_EQ(type_, kUint32);
        return data_.uint32_;
    }

    uint64_t getUint64() const {
        DCHECK_EQ(type_, kUint64);
        return data_.uint64_;
    }

    bool isSignedInt() const {
        return type_ == ValueTypeKind::kInt8 || type_ == ValueTypeKind::kInt16 ||
               type_ == ValueTypeKind::kInt32 || type_ == ValueTypeKind::kInt64;
    }

    int64_t getInteger() const {
        switch (type_) {
            case ValueTypeKind::kInt8:
                return data_.int8_;
            case ValueTypeKind::kInt16:
                return data_.int16_;
            case ValueTypeKind::kInt32:
                return data_.int32_;
            case ValueTypeKind::kInt64:
                return data_.int64_;
            case ValueTypeKind::kUint8:
                return data_.uint8_;
            case ValueTypeKind::kUint16:
                return data_.uint16_;
            case ValueTypeKind::kUint32:
                return data_.uint32_;
            case ValueTypeKind::kUint64:
                return data_.uint64_;
            default:
                LOG(FATAL) << "Malformed value type: " << static_cast<int>(type_);
                return 0;
        }
    }

    // Transfrom a "raw" user input numeric literal(stored as uint64_t or double).
    // Value of unsigned int type will be cast to int64_t if possible.  If `negate`
    // is true, return the negated value. `std::nullopt` will be returned if range
    // overflow is detected.
    template <bool negate>
    std::optional<Value> transformNumeric() const;

    uint64_t getUnsignedInteger() const {
        switch (type_) {
            case ValueTypeKind::kUint8:
                return data_.uint8_;
            case ValueTypeKind::kUint16:
                return data_.uint16_;
            case ValueTypeKind::kUint32:
                return data_.uint32_;
            case ValueTypeKind::kUint64:
                return data_.uint64_;
            default:
                LOG(FATAL) << "Malformed value type: " << static_cast<int>(type_);
                return 0;
        }
    }

    float getFloat() const {
        DCHECK_EQ(type_, kFloat32);
        return data_.float_;
    }

    double getDouble() const {
        DCHECK_EQ(type_, kFloat64);
        return data_.double_;
    }

    double getFloatingVal() const {
        switch (type_) {
            case ValueTypeKind::kFloat64:
                return getDouble();
            case ValueTypeKind::kFloat32:
                return getFloat();
            default:
                LOG(FATAL) << "Malformed value type: " << static_cast<int>(type_);
        }
    }

    template <typename T>
    const T& get() const {
        throw std::runtime_error("Unimplemented template type for Value::get() const");
    }
    template <typename T>
    T& get() {
        throw std::runtime_error("Unimplemented template type for Value::get()");
    }

    template <typename T>
    const Value& getProp() const {
        throw std::runtime_error("Unimplemented template type for Value::getProp()");
    }

    const NullValue& getNull() const {
        return *data_.null_;
    }

    const String& getBytes() const {
        return *data_.bytes_;
    }

    const String& getString() const {
        DCHECK_EQ(type_, kString);
        return *data_.string_;
    }

    std::string_view getBytesView() const {
        return {data_.bytes_->data(), data_.bytes_->size()};
    }

    std::string_view getStringView() const {
        DCHECK_EQ(type_, kString);
        return {data_.string_->data(), data_.string_->size()};
    }

    const List& getList() const {
        DCHECK_EQ(type_, kList);
        return *data_.list_;
    }

    const Path& getPath() const {
        DCHECK_EQ(type_, kPath);
        return *data_.path_;
    }

    const Record& getRecord() const {
        DCHECK_EQ(type_, kRecord);
        return *data_.record_;
    }

    const Node& getNode() const {
        DCHECK_EQ(type_, kNode);
        return *data_.node_;
    }

    const Edge& getEdge() const {
        DCHECK_EQ(type_, kEdge);
        return *data_.edge_;
    }

    const LocalTime& getLocalTime() const {
        DCHECK_EQ(type_, kLocalTime);
        return data_.localTime_;
    }

    const ZonedTime& getZonedTime() const {
        DCHECK_EQ(type_, kZonedTime);
        return data_.zonedTime_;
    }

    const Date& getDate() const {
        DCHECK_EQ(type_, kDate);
        return data_.date_;
    }

    const LocalDatetime& getLocalDatetime() const {
        DCHECK_EQ(type_, kLocalDatetime);
        return data_.localDatetime_;
    }

    const ZonedDatetime& getZonedDatetime() const {
        DCHECK_EQ(type_, kZonedDatetime);
        return data_.zonedDatetime_;
    }

    const Ref& getRef() const {
        DCHECK_EQ(type_, kRef);
        return *data_.ref_;
    }

    const Duration& getDuration() const {
        DCHECK_EQ(type_, kDuration);
        return *data_.duration_;
    }

    bool& mutableBool() {
        return data_.bool_;
    }

    int8_t& mutableInt8() {
        return data_.int8_;
    }

    int16_t& mutableInt16() {
        return data_.int16_;
    }

    int32_t& mutableInt32() {
        return data_.int32_;
    }

    int64_t& mutableInt64() {
        return data_.int64_;
    }

    float& mutableFloat() {
        return data_.float_;
    }

    double& mutableDouble() {
        return data_.double_;
    }

    String& mutableString() {
        return *data_.string_;
    }

    String& mutableBytes() {
        return *data_.bytes_;
    }

    List& mutableList() {
        return *data_.list_;
    }

    Record& mutableRecord() {
        return *data_.record_;
    }

    Ref& mutableRef() {
        return *data_.ref_;
    }

    Path& mutablePath() {
        return *data_.path_;
    }

    Node& mutableNode() {
        return *data_.node_;
    }

    Edge& mutableEdge() {
        return *data_.edge_;
    }

    LocalTime& mutableLocalTime() {
        return data_.localTime_;
    }

    ZonedTime& mutableZonedTime() {
        return data_.zonedTime_;
    }

    Date& mutableDate() {
        return data_.date_;
    }

    LocalDatetime& mutableLocalDatetime() {
        return data_.localDatetime_;
    }

    ZonedDatetime& mutableZonedDatetime() {
        return data_.zonedDatetime_;
    }

    Duration& mutableDuration() {
        return *data_.duration_;
    }

    std::string toString() const;

    void hashKey(HashKeyAppender& appender) const;

    Value toFloat() const;

    // Explicitly convert data to a integer value.
    // See 19.22 <cast specification>
    Value toInt() const;

    // Cast integer to another integer type.
    Value castInt(ValueTypeKind type) const;

    Value castNumeric(ValueTypeKind to) const;
    // Compare the value with another value. Returns 0 if they are equal, 1 if this value is
    // greater than the other, -1 if this value is less than the other.
    int compare(const Value& other,
                bool areNullsLargerThanNonNulls = true,
                bool areNullsEqual = true) const;

    void clear();

    template <typename T>
    static Value doCastNumeric(T val, ValueTypeKind to) {
        switch (to) {
            case ValueTypeKind::kInt8:
                return Value(static_cast<int8_t>(val));
            case ValueTypeKind::kInt16:
                return Value(static_cast<int16_t>(val));
            case ValueTypeKind::kInt32:
                return Value(static_cast<int32_t>(val));
            case ValueTypeKind::kInt64:
                return Value(static_cast<int64_t>(val));
            case ValueTypeKind::kUint8:
                return Value(static_cast<uint8_t>(val));
            case ValueTypeKind::kUint16:
                return Value(static_cast<uint16_t>(val));
            case ValueTypeKind::kUint32:
                return Value(static_cast<uint32_t>(val));
            case ValueTypeKind::kUint64:
                return Value(static_cast<uint64_t>(val));
            case ValueTypeKind::kFloat32:
                return Value(static_cast<float>(val));
            case ValueTypeKind::kFloat64:
                return Value(static_cast<double>(val));
            default:
                LOG(FATAL) << "support only numeric value: " << to;
                return NullValue::kNullValue;
        }
    }

private:
    void copyValue(const Value& other);
    void moveValue(Value&& other) noexcept;

private:
    template <typename T>
    T* allocate(allocator_type& alloc) {
        return reinterpret_cast<T*>(alloc.allocate(sizeof(T)));
    }

    template <typename T>
    void deallocate(allocator_type& alloc, T* p) noexcept {
        alloc.deallocate(reinterpret_cast<std::byte*>(p), sizeof(T));
    }

    template <typename T, typename... Args>
    T* newObject(Args&&... args) {
        return new T(std::forward<Args>(args)...);
    }

    template <typename T>
    void deleteObject(T* p) {
        delete p;
    }

private:
    friend nrpc::BufferReaderWriter<Value>;
    ValueTypeKind type_;
    Data data_;
};

constexpr auto kEpsilon = 1e-12;

inline std::ostream& operator<<(std::ostream& os, const Value& value) {
    return os << value.toString();
}


inline uint64_t operator|(const ValueTypeKind& lhs, const ValueTypeKind& rhs) {
    return static_cast<uint64_t>(lhs) | static_cast<uint64_t>(rhs);
}
inline uint64_t operator&(const ValueTypeKind& lhs, const ValueTypeKind& rhs) {
    return static_cast<uint64_t>(lhs) & static_cast<uint64_t>(rhs);
}

// FIXME: Fix these operator overloading functions to support basic comparison and calculation,
// which is important for test.
// Arithmetic operations
Value operator+(const Value& lhs, const Value& rhs);
Value operator-(const Value& lhs, const Value& rhs);
Value operator*(const Value& lhs, const Value& rhs);
Value operator/(const Value& lhs, const Value& rhs);
Value operator%(const Value& lhs, const Value& rhs);
// Comparison operations
bool operator==(const Value& lhs, const Value& rhs);
bool operator!=(const Value& lhs, const Value& rhs);
bool operator<(const Value& lhs, const Value& rhs);
bool operator>(const Value& lhs, const Value& rhs);
bool operator<=(const Value& lhs, const Value& rhs);
bool operator>=(const Value& lhs, const Value& rhs);
// unary operations
Value operator-(const Value& rhs);


template <>
struct nrpc::BufferReaderWriter<Value> {
    static void write(folly::IOBuf* buf, const Value& value);

    static Status read(folly::IOBuf* buf, Value* value);

    static size_t encodedSize(const Value& value);
};


template <ValueTypeKind type>
inline const auto& get(const Value& value) {
    DCHECK_EQ(type, type == ValueTypeKind::kAny ? ValueTypeKind::kAny : value.getType());
    using NativeType = typename TypeTraits<type>::NativeType;
    return value.get<NativeType>();
}

template <>
inline void HashKeyAppender::append<String>(const String& t) {
    key.append(t.data(), t.size());
}

template <bool negate>
std::optional<Value> Value::transformNumeric() const {
    DCHECK(isNumeric());
    const Value& val = *this;
    if (val.isDouble() || val.isFloat()) {
        if constexpr (negate) {
            return -val;
        }
        return val;
    }

    if (val.isSignedInt()) {
        if constexpr (!negate) {
            return val;
        }

        auto sval = -val;
        if (!sval.isNull()) {
            return sval;
        }
        if (val.getInteger() > std::numeric_limits<int64_t>::min()) {
            return -(val.getInteger());
        }
        return std::nullopt;
    }

    // convert unsigned int
    auto uval = val.getUnsignedInteger();
    if constexpr (!negate) {
        auto res = folly::tryTo<int64_t>(uval);
        if (res.hasValue()) {
            return res.value();
        }
        return std::nullopt;
    }

    if (uval <= static_cast<uint64_t>(std::numeric_limits<int64_t>::min())) {
        return static_cast<int64_t>(-uval);
    }
    return std::nullopt;
}

}  // namespace nebula

namespace std {

template <>
struct hash<nebula::Value> {
    std::size_t operator()(const nebula::Value& v) const noexcept;
};

}  // namespace std

#include "nebula/common/datatype/Value-inl.h"
