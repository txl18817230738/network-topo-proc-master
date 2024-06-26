// Copyright (c) 2022 vesoft inc. All rights reserved.

#pragma once

namespace nebula {

#define VALUE_GET_DEFINE(ValueTypeKind, value)       \
    template <>                                      \
    inline const ValueTypeKind& Value::get() const { \
        return value;                                \
    }                                                \
    template <>                                      \
    inline ValueTypeKind& Value::get() {             \
        return value;                                \
    }

VALUE_GET_DEFINE(Value, *this)
VALUE_GET_DEFINE(bool, data_.bool_)
VALUE_GET_DEFINE(int8_t, data_.int8_)
VALUE_GET_DEFINE(int16_t, data_.int16_)
VALUE_GET_DEFINE(int32_t, data_.int32_)
VALUE_GET_DEFINE(int64_t, data_.int64_)
VALUE_GET_DEFINE(uint8_t, data_.uint8_)
VALUE_GET_DEFINE(uint16_t, data_.uint16_)
VALUE_GET_DEFINE(uint32_t, data_.uint32_)
VALUE_GET_DEFINE(uint64_t, data_.uint64_)
VALUE_GET_DEFINE(float, data_.float_)
VALUE_GET_DEFINE(double, data_.double_)
VALUE_GET_DEFINE(String, *data_.string_)
VALUE_GET_DEFINE(List, *data_.list_)
VALUE_GET_DEFINE(Record, *data_.record_)
VALUE_GET_DEFINE(Path, *data_.path_)
VALUE_GET_DEFINE(Node, *data_.node_)
VALUE_GET_DEFINE(Edge, *data_.edge_)
VALUE_GET_DEFINE(LocalTime, data_.localTime_)
VALUE_GET_DEFINE(ZonedTime, data_.zonedTime_)
VALUE_GET_DEFINE(Date, data_.date_)
VALUE_GET_DEFINE(LocalDatetime, data_.localDatetime_)
VALUE_GET_DEFINE(ZonedDatetime, data_.zonedDatetime_)
VALUE_GET_DEFINE(Duration, *data_.duration_)
VALUE_GET_DEFINE(Ref, *data_.ref_)
VALUE_GET_DEFINE(NullValue, *data_.null_)
}  // namespace nebula
