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

#include <matter/tracing/build_config.h>
#include <tracing/metric_macros.h>
#include <lib/core/CHIPError.h>
#include <system/SystemClock.h>
#include <tracing/metric_keys.h>

namespace chip {
namespace Tracing {

/**
 * Define a metric that can be logged. A metric consists of a key-value pair. The value is
 * currently limited to simple scalar values. The value is interpreted based on the key type.
 *
 * Additionally a metric is tagged as either an instant type or marked with a begin and end
 * for the event. When the latter is used, a duration can be associated between the two events.
 */
struct MetricEvent
{
    // This specifies the different categories of metric events that can created. In addition to
    // emitting an event, events paired with a Begin and End are used to track duration for the
    // event. An instant event just represents a one shot event.
    enum class Tag
    {
        Begin,  // Marks the begin of an event. This is typically tied with an End event.
        End,    // Marks the end of an event. This is typically preceeded with a Begin event.
        Instant // No duration
    };

    // This defines the different types of values that can stored when a metric is emitted
    struct Value
    {
        enum class Type : uint8_t
        {
            Signed32Type,   // int32_t
            Unsigned32Type, // uint32_t
            ChipErrorType   // chip::ChipError
        };

        union Store
        {
            int32_t int32_value;
            uint32_t uint32_value;

            Store(int32_t v) : int32_value(v) {}

            Store(uint32_t v) : uint32_value(v) {}
        };

        Store store;
        Type type;

        Value(uint32_t value) : store(value), type(Type::Unsigned32Type) {}

        Value(int32_t value) : store(value), type(Type::Signed32Type) {}

        Value(const ChipError & err) : store(err.AsInteger()), type(Type::ChipErrorType) {}
    };

    Tag tag;
    MetricKey key;
    Value value;
    System::Clock::Microseconds64 timePoint;

    MetricEvent(Tag tg, MetricKey k, int32_t val = 0) :
        tag(tg), key(k), value(val), timePoint(System::SystemClock().GetMonotonicMicroseconds64())
    {}

    MetricEvent(Tag tg, MetricKey k, uint32_t val) :
        tag(tg), key(k), value(val), timePoint(System::SystemClock().GetMonotonicMicroseconds64())
    {}

    MetricEvent(Tag tg, MetricKey k, int8_t val) : MetricEvent(tg, k, int32_t(val)) {}

    MetricEvent(Tag tg, MetricKey k, uint8_t val) : MetricEvent(tg, k, uint32_t(val)) {}

    MetricEvent(Tag tg, MetricKey k, int16_t val) : MetricEvent(tg, k, int32_t(val)) {}

    MetricEvent(Tag tg, MetricKey k, uint16_t val) : MetricEvent(tg, k, uint32_t(val)) {}

    MetricEvent(Tag tg, MetricKey k, const ChipError & err) :
        tag(tg), key(k), value(err.AsInteger()), timePoint(System::SystemClock().GetMonotonicMicroseconds64())
    {}
};

namespace Internal {

void LogMetricEvent(::chip::Tracing::MetricEvent & event);

} // namespace Internal

namespace utils {

/**
 * Utility to emit an instant metric if the error is not a success. Used in SuccessOrExit macro
 * to allow emitting the event before jumping to the exit label.
 */
inline bool logMetricIfError(const ::chip::ChipError & err, MetricKey metricKey)
{
    bool success = ::chip::ChipError::IsSuccess(err);
    if (!success)
    {
        using Tag = chip::Tracing::MetricEvent::Tag;
        ::chip::Tracing::MetricEvent _metric_event(Tag::Instant, metricKey, err);
        ::chip::Tracing::Internal::LogMetricEvent(_metric_event);
    }
    return success;
}

} // namespace utils

} // namespace Tracing
} // namespace chip
