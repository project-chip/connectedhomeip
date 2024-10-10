
/*
 *
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

#include <algorithm>
#include <esp_err.h>
#include <esp_heap_caps.h>
#include <esp_log.h>
#include <memory>
#include <tracing/backend.h>
#include <tracing/esp32_diagnostic_trace/Counter.h>
#include <tracing/esp32_diagnostic_trace/DiagnosticTracing.h>
#include <tracing/metric_event.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {

#define LOG_HEAP_INFO(label, group, entry_exit)                                                                                    \
    do                                                                                                                             \
    {                                                                                                                              \
        ESP_DIAG_EVENT("MTR_TRC", "%s - %s - %s", entry_exit, label, group);                                                       \
    } while (0)

constexpr size_t kPermitListMaxSize = CONFIG_MAX_PERMIT_LIST_SIZE;
using HashValue                     = uint32_t;

// Implements a murmurhash with 0 seed.
uint32_t MurmurHash(const void * key)
{
    const uint32_t kMultiplier = 0x5bd1e995;
    const uint32_t kShift      = 24;
    const unsigned char * data = (const unsigned char *) key;
    uint32_t hash              = 0;

    while (*data)
    {
        uint32_t value = *data++;
        value *= kMultiplier;
        value ^= value >> kShift;
        value *= kMultiplier;
        hash *= kMultiplier;
        hash ^= value;
    }

    hash ^= hash >> 13;
    hash *= kMultiplier;
    hash ^= hash >> 15;

    if (hash == 0)
    {
        ESP_LOGW("Tracing", "MurmurHash resulted in a hash value of 0");
    }

    return hash;
}

HashValue gPermitList[kPermitListMaxSize] = { MurmurHash("PASESession"),
                                              MurmurHash("CASESession"),
                                              MurmurHash("NetworkCommissioning"),
                                              MurmurHash("GeneralCommissioning"),
                                              MurmurHash("OperationalCredentials"),
                                              MurmurHash("CASEServer"),
                                              MurmurHash("BLE"),
                                              MurmurHash("BLE_Error"),
                                              MurmurHash("Wifi"),
                                              MurmurHash("Wifi_Error"),
                                              MurmurHash("Fabric") }; // namespace

bool IsPermitted(HashValue hashValue)
{
    for (HashValue permitted : gPermitList)
    {
        if (permitted == 0)
        {
            break;
        }
        if (hashValue == permitted)
        {
            return true;
        }
    }
    return false;
}

void ESP32Diagnostics::LogMessageReceived(MessageReceivedInfo & info) {}

void ESP32Diagnostics::LogMessageSend(MessageSendInfo & info) {}

void ESP32Diagnostics::LogNodeLookup(NodeLookupInfo & info) {}

void ESP32Diagnostics::LogNodeDiscovered(NodeDiscoveredInfo & info) {}

void ESP32Diagnostics::LogNodeDiscoveryFailed(NodeDiscoveryFailedInfo & info) {}

void ESP32Diagnostics::LogMetricEvent(const MetricEvent & event)
{
    DiagnosticStorageImpl & diagnosticStorage = DiagnosticStorageImpl::GetInstance();
    CHIP_ERROR err = CHIP_NO_ERROR;
    switch (event.ValueType())
    {
    case ValueType::kInt32: {
        ESP_LOGI("mtr", "The value of %s is %ld ", event.key(), event.ValueInt32());
        Metric<int32_t> metric(event.key(), event.ValueInt32(), esp_log_timestamp());
        err = diagnosticStorage.Store(metric);
    }
    break;

    case ValueType::kUInt32: {
        ESP_LOGI("mtr", "The value of %s is %lu ", event.key(), event.ValueUInt32());
        Metric<uint32_t> metric(event.key(), event.ValueUInt32(), esp_log_timestamp());
        err = diagnosticStorage.Store(metric);
    }
    break;

    case ValueType::kChipErrorCode:
        ESP_LOGI("mtr", "The value of %s is error with code %lu ", event.key(), event.ValueErrorCode());
        break;

    case ValueType::kUndefined:
        ESP_LOGI("mtr", "The value of %s is undefined", event.key());
        break;

    default:
        ESP_LOGI("mtr", "The value of %s is of an UNKNOWN TYPE", event.key());
        break;
    }

    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Failed to store Metric Diagnostic data"));
}

void ESP32Diagnostics::TraceCounter(const char * label)
{
    ::Diagnostics::ESPDiagnosticCounter::GetInstance(label)->ReportMetrics();
}

void ESP32Diagnostics::TraceBegin(const char * label, const char * group) {
    StoreDiagnostics(label, group);
}

void ESP32Diagnostics::TraceEnd(const char * label, const char * group) {
    StoreDiagnostics(label, group);
}

void ESP32Diagnostics::TraceInstant(const char * label, const char * group) {
    StoreDiagnostics(label, group);
}

void ESP32Diagnostics::StoreDiagnostics(const char* label, const char* group) {
    CHIP_ERROR err = CHIP_NO_ERROR;
    HashValue hashValue                           = MurmurHash(group);
    DiagnosticStorageImpl & diagnosticStorage = DiagnosticStorageImpl::GetInstance();
    if (IsPermitted(hashValue))
    {
        Trace trace(label, group, esp_log_timestamp());
        err = diagnosticStorage.Store(trace);
        VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Failed to store Trace Diagnostic data"));
    }
}

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
