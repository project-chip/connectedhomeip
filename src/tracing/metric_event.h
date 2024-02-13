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

#if MATTER_TRACING_ENABLED

#include <lib/core/CHIPError.h>
#include <stdint.h>
#include <chrono>

namespace chip {
namespace Tracing {

/**
 * Defines the key type use to identity a specific metric
 */
typedef const char* MetricKey;

/**
 * List of supported metric keys
 */
constexpr MetricKey kMetricDiscoveryOverBLE = "disc-over-ble";
constexpr MetricKey kMetricDiscoveryOnNetwork = "disc-on-nw";
constexpr MetricKey kMetricPASESession = "pase-session";
constexpr MetricKey kMetricPASESessionEstState = "pase-conn-est";
constexpr MetricKey kMetricAttestationResult = "attestation-result";
constexpr MetricKey kMetricAttestationOverridden = "attestation-overridden";
constexpr MetricKey kMetricCASESession = "case-session";
constexpr MetricKey kMetricCASESessionEstState = "case-conn-est";
constexpr MetricKey kMetricWiFiRSSI = "wifi-rssi";

/**
 * Define a metric that can be logged. A metric consists of a key-value pair. The value is
 * currently limited to simple scalar values. The value is interpreted based on the key type.
 *
 * Additionally a metric is tagged as either an instant type or marked with a begin and end 
 * for the event. When the latter is used, a duration can be associated between the two events.
 */
struct MetricEvent
{
    // Metric tag type
    enum class Tag {
        Begin,    // Implies tracking a duration
        End,      // Implies tracking a duration
        Instant   // No duration
    };

    // Value for the metric
    struct Value {
        enum class ValueType {
            SignedValue,
            UnsignedValue,
            ErrorValue
        };

        union ValueStore {
            uint32_t uvalue;
            int32_t svalue;
            ValueStore(uint32_t v) : uvalue(v) {}
            ValueStore(int32_t v) : svalue(v) {}
        } store;
        ValueType type;

        Value(uint32_t value) : store(value), type(ValueType::UnsignedValue) {}

        Value(int32_t value) : store(value), type(ValueType::SignedValue) {}

        Value(const ChipError & err) : store(err.AsInteger()), type(ValueType::ErrorValue) {}
    };

    Tag tag;
    MetricKey key;
    Value value;
    std::chrono::steady_clock::time_point timePoint;

    MetricEvent(Tag tg, MetricKey k, uint32_t val = 0)
        : tag(tg), key(k), value(val), timePoint(std::chrono::steady_clock::now())
    {}

    MetricEvent(Tag tg, MetricKey k, int32_t val)
        : tag(tg), key(k), value(val), timePoint(std::chrono::steady_clock::now())
    {}

    MetricEvent(Tag tg, MetricKey k, const ChipError & err)
        : tag(tg), key(k), value(err.AsInteger()), timePoint(std::chrono::steady_clock::now())
    {}
};

} // namespace Tracing
} // namespace chip

#endif // MATTER_TRACING_ENABLED
