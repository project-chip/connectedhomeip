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
#include <stdint.h>

namespace chip {
namespace Tracing {

typedef const char* MetricKey;

constexpr MetricKey kMetricDiscoveryOverBLE = "discovery-over-ble";
constexpr MetricKey kMetricDiscoveryOnNetwork = "discovery-on-nw";
constexpr MetricKey kMetricPASEConnectionEstablished = "pase-conn-est";
constexpr MetricKey kMetricPASEConnectionFailed = "pase-conn-failed";
constexpr MetricKey kMetricAttestationResult = "attestation-result";
constexpr MetricKey kMetricAttestationOverridden = "attestation-overridden";
constexpr MetricKey kMetricCASEConnectionEstablished = "case-conn-est";
constexpr MetricKey kMetricCASEConnectionFailed = "case-conn-failed";
constexpr MetricKey kMetricWiFiRSSI = "wifi-rssi";

struct MetricEvent
{
    MetricKey key;
    union Value {
        uint32_t uvalue;
        int32_t svalue;
        Value(uint32_t v) : uvalue(v) {}
        Value(int32_t v) : svalue(v) {}
    } value;
    bool isSigned;

    MetricEvent(MetricKey k, uint32_t val = 0) : key(k), value(val), isSigned(false) {}
    MetricEvent(MetricKey k, int32_t val = 0) : key(k), value(val), isSigned(true) {}
    MetricEvent(MetricKey k, ChipError err) : key(k), value(err.AsInteger()), isSigned(false) {}
};

} // namespace Tracing
} // namespace chip
