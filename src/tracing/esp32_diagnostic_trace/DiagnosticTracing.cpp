/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
#include <esp_log.h>
#include <lib/support/CHIPMemString.h>
#include <tracing/esp32_diagnostic_trace/Counter.h>
#include <tracing/esp32_diagnostic_trace/DiagnosticTracing.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {

namespace {
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
        ChipLogError(DeviceLayer, "MurmurHash resulted in a hash value of 0");
    }

    return hash;
}
} // anonymous namespace

constexpr size_t kPermitListMaxSize = CONFIG_MAX_PERMIT_LIST_SIZE;
using HashValue                     = uint32_t;

/*
 * gPermitList will allow the following traces to be stored in the storage instance while other traces are skipped.
 * Only traces with scope in gPermitList are allowed.
 * Used for MATTER_TRACE_SCOPE()
 */
HashValue gPermitList[kPermitListMaxSize] = { MurmurHash("PASESession"),
                                              MurmurHash("CASESession"),
                                              MurmurHash("NetworkCommissioning"),
                                              MurmurHash("GeneralCommissioning"),
                                              MurmurHash("OperationalCredentials"),
                                              MurmurHash("CASEServer"),
                                              MurmurHash("Fabric") };

/*
 * gSkipList will skip the following traces from being stored in the storage instance while other traces are stored.
 * Used for MATTER_TRACE_INSTANT()
 */
HashValue gSkipList[kPermitListMaxSize] = {
    MurmurHash("Resolver"),
};

bool IsPresent(const char * str, HashValue * list)
{
    for (int i = 0; i < kPermitListMaxSize; i++)
    {
        if (list[i] == 0)
        {
            break;
        }
        if (MurmurHash(str) == list[i])
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
    VerifyOrReturn(mStorageInstance != nullptr, ChipLogError(DeviceLayer, "Diagnostic Storage Instance cannot be NULL"));
    DiagnosticEntry entry;
    switch (event.ValueType())
    {
    case ValueType::kInt32:
        ChipLogDetail(DeviceLayer, "The value of %s is %" PRId32, event.key(), event.ValueInt32());
        Platform::CopyString(entry.label, event.key());
        entry.intValue                 = event.ValueInt32();
        entry.type                     = Diagnostics::ValueType::kSignedInteger;
        entry.timestamps_ms_since_boot = esp_log_timestamp();
        ReturnOnFailure(mStorageInstance->Store(entry));
        break;
    case ValueType::kUInt32:
        ChipLogDetail(DeviceLayer, "The value of %s is %" PRIu32, event.key(), event.ValueUInt32());
        Platform::CopyString(entry.label, event.key());
        entry.uintValue                = event.ValueUInt32();
        entry.type                     = Diagnostics::ValueType::kUnsignedInteger;
        entry.timestamps_ms_since_boot = esp_log_timestamp();
        ReturnOnFailure(mStorageInstance->Store(entry));
        break;
    case ValueType::kChipErrorCode:
        ChipLogDetail(DeviceLayer, "The value of %s is error with code %lu ", event.key(), event.ValueErrorCode());
        break;
    case ValueType::kUndefined:
        ChipLogDetail(DeviceLayer, "The value of %s is undefined", event.key());
        break;
    default:
        ChipLogDetail(DeviceLayer, "The value of %s is of an UNKNOWN TYPE", event.key());
        break;
    }
}

void ESP32Diagnostics::TraceCounter(const char * label)
{
    ESPDiagnosticCounter & counter = ESPDiagnosticCounter::GetInstance();
    counter.IncreaseCount(label);
    ReturnOnFailure(counter.ReportMetrics(label, mStorageInstance));
}

void ESP32Diagnostics::TraceBegin(const char * label, const char * group)
{
    VerifyOrReturn(IsPresent(group, gPermitList));
    ReturnOnFailure(StoreDiagnostics(label, group));
}

void ESP32Diagnostics::TraceEnd(const char * label, const char * group) {}

void ESP32Diagnostics::TraceInstant(const char * label, const char * value)
{
    VerifyOrReturn(!IsPresent(value, gSkipList));
    ReturnOnFailure(StoreDiagnostics(label, value));
}

CHIP_ERROR ESP32Diagnostics::StoreDiagnostics(const char * label, const char * group)
{
    VerifyOrReturnError(mStorageInstance != nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(DeviceLayer, "Diagnostic Storage Instance cannot be NULL"));

    // Create diagnostic entry
    DiagnosticEntry entry;
    Platform::CopyString(entry.label, label);
    Platform::CopyString(entry.stringValue, group);
    entry.type                     = Diagnostics::ValueType::kCharString;
    entry.timestamps_ms_since_boot = esp_log_timestamp();

    return mStorageInstance->Store(entry);
}

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
