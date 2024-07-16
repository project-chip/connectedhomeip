/*
 *    Copyright (c) 2024 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
#pragma once

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <tracing/metric_keys.h>
#include <tracing/metric_macros.h>
#include <tracing/registry.h>

namespace chip {
namespace Tracing {

/**
 * Define a metric that can be logged. A metric consists of a key and an optional value pair.
 * The value is currently limited to simple scalar values.
 *
 * Additionally a metric is tagged as either an instant event or marked with a begin/end
 * for the event. When the latter is used, a duration can be associated between the two events.
 */
class MetricEvent
{
public:
    MetricEvent(const MetricEvent &)             = default;
    MetricEvent(MetricEvent &&)                  = default;
    MetricEvent & operator=(const MetricEvent &) = default;
    MetricEvent & operator=(MetricEvent &&)      = default;

    // This specifies the different categories of metric events that can created. In addition to
    // emitting an event, events paired with a kBeginEvent and kEndEvent can be used to track
    // duration for the event. A kInstantEvent represents a one shot event.
    enum class Type
    {
        // This specifies an event marked to track the Begin of an operation
        kBeginEvent,

        // This specifies an event marked to track the End of an operation
        kEndEvent,

        // This specifies a one shot event
        kInstantEvent
    };

    // This defines the different types of values that can stored when a metric is emitted
    struct Value
    {
        Value(const Value &)             = default;
        Value(Value &&)                  = default;
        Value & operator=(const Value &) = default;
        Value & operator=(Value &&)      = default;

        enum class Type : uint8_t
        {
            kUndefined,    // Value is not valid
            kInt32,        // int32_t
            kUInt32,       // uint32_t
            kChipErrorCode // chip::ChipError
        };

        union Store
        {
            int32_t int32_value;
            uint32_t uint32_value;

            Store() {}

            Store(int32_t v) : int32_value(v) {}

            Store(uint32_t v) : uint32_value(v) {}
        };

        Store store;
        Type type;

        Value() : type(Type::kUndefined) {}

        Value(uint32_t value) : store(value), type(Type::kUInt32) {}

        Value(int32_t value) : store(value), type(Type::kInt32) {}

        Value(const ChipError & err) : store(err.AsInteger()), type(Type::kChipErrorCode) {}
    };

    MetricEvent(Type type, MetricKey key) : mType(type), mKey(key) {}

    MetricEvent(Type type, MetricKey key, int32_t value) : mType(type), mKey(key), mValue(value) {}

    MetricEvent(Type type, MetricKey key, uint32_t value) : mType(type), mKey(key), mValue(value) {}

    MetricEvent(Type type, MetricKey key, const ChipError & error) : mType(type), mKey(key), mValue(error) {}

    MetricEvent(Type type, MetricKey key, Value value) : mType(type), mKey(key), mValue(value) {}

    MetricEvent(Type type, MetricKey key, int8_t value) : MetricEvent(type, key, int32_t(value)) {}

    MetricEvent(Type type, MetricKey key, uint8_t value) : MetricEvent(type, key, uint32_t(value)) {}

    MetricEvent(Type type, MetricKey key, int16_t value) : MetricEvent(type, key, int32_t(value)) {}

    MetricEvent(Type type, MetricKey key, uint16_t value) : MetricEvent(type, key, uint32_t(value)) {}

    Type type() const { return mType; }

    MetricKey key() const { return mKey; }

    Value::Type ValueType() const { return mValue.type; }

    uint32_t ValueUInt32() const
    {
        VerifyOrDie(mValue.type == Value::Type::kUInt32);
        return mValue.store.uint32_value;
    }

    int32_t ValueInt32() const
    {
        VerifyOrDie(mValue.type == Value::Type::kInt32);
        return mValue.store.int32_value;
    }

    uint32_t ValueErrorCode() const
    {
        VerifyOrDie(mValue.type == Value::Type::kChipErrorCode);
        return mValue.store.uint32_value;
    }

private:
    Type mType;
    MetricKey mKey;
    Value mValue;
};

namespace ErrorHandling {

/**
 * Utility to emit an instant metric if the error is not a success.
 */
inline bool LogMetricIfError(MetricKey metricKey, const ::chip::ChipError & err)
{
    bool success = ::chip::ChipError::IsSuccess(err);
    if (!success)
    {
        MATTER_LOG_METRIC(metricKey, err);
    }
    return success;
}

} // namespace ErrorHandling

/**
 * This utility class helps generate a Begin and End metric event within the scope of a block using RAII.
 */
class ScopedMetricEvent
{
public:
    ScopedMetricEvent(const ScopedMetricEvent &)             = delete;
    ScopedMetricEvent(ScopedMetricEvent &&)                  = delete;
    ScopedMetricEvent & operator=(const ScopedMetricEvent &) = delete;
    ScopedMetricEvent & operator=(ScopedMetricEvent &&)      = delete;

    ScopedMetricEvent(MetricKey key, ChipError & error) : mKey(key), mError(error)
    {
        MATTER_LOG_METRIC_BEGIN(mKey);
        IgnoreUnusedVariable(mKey);
        IgnoreUnusedVariable(mError);
    }

    ~ScopedMetricEvent() { MATTER_LOG_METRIC_END(mKey, mError); }

private:
    MetricKey mKey;
    ChipError & mError;
};

} // namespace Tracing
} // namespace chip
