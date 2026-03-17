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
#include <cctype>
#include <lib/support/CHIPMemString.h>
#include <system/SystemClock.h>
#include <tracing/esp32_diagnostics/Counter.h>
#include <tracing/esp32_diagnostics/DiagnosticTracing.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {

namespace {
// Implements a murmurhash with 0 seed.
uint32_t MurmurHash(const void * key)
{
    const uint32_t kMultiplier = 0x5bd1e995;
    const uint32_t kShift      = 24;
    const unsigned char * data = static_cast<const unsigned char *>(key);
    uint32_t hash              = 0;

    while (*data)
    {
        uint32_t value = static_cast<uint32_t>(tolower(static_cast<unsigned char>(*data++)));
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

ESP32Diagnostics::ESP32Diagnostics(CircularDiagnosticBuffer * storageInstance) : mStorageInstance(storageInstance)
{
    ClearFilters();
    InitializeDefaultFilters();
}

void ESP32Diagnostics::InitializeDefaultFilters()
{
    static constexpr const char * kDefaultFilters[] = {
        "PASESession", "CASESession", "NetworkCommissioning", "GeneralCommissioning", "OperationalCredentials",
        "CASEServer",  "Fabric",
    };
    for (const auto & filter : kDefaultFilters)
    {
        mEnabledFilters.insert(MurmurHash(filter));
    }
}

CHIP_ERROR ESP32Diagnostics::AddFilter(const char * scope)
{
    VerifyOrReturnError(scope != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(strlen(scope) > 0, CHIP_ERROR_INVALID_ARGUMENT);
    mEnabledFilters.insert(MurmurHash(scope));
    return CHIP_NO_ERROR;
}

CHIP_ERROR ESP32Diagnostics::RemoveFilter(const char * scope)
{
    VerifyOrReturnError(scope != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(strlen(scope) > 0, CHIP_ERROR_INVALID_ARGUMENT);
    auto it = mEnabledFilters.find(MurmurHash(scope));
    VerifyOrReturnError(it != mEnabledFilters.end(), CHIP_ERROR_INCORRECT_STATE);
    mEnabledFilters.erase(it);
    return CHIP_NO_ERROR;
}

void ESP32Diagnostics::ClearFilters()
{
    mEnabledFilters.clear();
}

bool ESP32Diagnostics::IsEnabled(const char * scope)
{
    // If no filters are set, all scopes are enabled
    if (mEnabledFilters.empty())
    {
        return true;
    }
    return mEnabledFilters.count(MurmurHash(scope)) > 0;
}

#ifdef CONFIG_ENABLE_ESP_DIAGNOSTIC_METRICS
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
        entry.timestamps_ms_since_boot = static_cast<uint32_t>(chip::System::SystemClock().GetMonotonicMilliseconds64().count());
        ReturnOnFailure(mStorageInstance->Store(entry));
        break;
    case ValueType::kUInt32:
        ChipLogDetail(DeviceLayer, "The value of %s is %" PRIu32, event.key(), event.ValueUInt32());
        Platform::CopyString(entry.label, event.key());
        entry.uintValue                = event.ValueUInt32();
        entry.type                     = Diagnostics::ValueType::kUnsignedInteger;
        entry.timestamps_ms_since_boot = static_cast<uint32_t>(chip::System::SystemClock().GetMonotonicMilliseconds64().count());
        ReturnOnFailure(mStorageInstance->Store(entry));
        break;
    case ValueType::kChipErrorCode:
        ChipLogDetail(DeviceLayer, "The value of %s is error with code %" PRIu32, event.key(), event.ValueErrorCode());
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
#endif // CONFIG_ENABLE_ESP_DIAGNOSTIC_METRICS

#ifdef CONFIG_ENABLE_ESP_DIAGNOSTIC_TRACES
void ESP32Diagnostics::TraceBegin(const char * label, const char * group)
{
    VerifyOrReturn(IsEnabled(group));
    ReturnOnFailure(StoreDiagnostics(label, group));
}

void ESP32Diagnostics::TraceEnd(const char * label, const char * group) {}

void ESP32Diagnostics::TraceInstant(const char * label, const char * value)
{
    VerifyOrReturn(IsEnabled(value));
    ReturnOnFailure(StoreDiagnostics(label, value));
}
#endif // CONFIG_ENABLE_ESP_DIAGNOSTIC_TRACES

CHIP_ERROR ESP32Diagnostics::StoreDiagnostics(const char * label, const char * group)
{
    VerifyOrReturnError(label != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(group != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mStorageInstance != nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(DeviceLayer, "Diagnostic Storage Instance cannot be NULL"));
    DiagnosticEntry entry;
    Platform::CopyString(entry.label, label);
    Platform::CopyString(entry.stringValue, group);
    entry.type                     = Diagnostics::ValueType::kCharString;
    entry.timestamps_ms_since_boot = static_cast<uint32_t>(chip::System::SystemClock().GetMonotonicMilliseconds64().count());

    return mStorageInstance->Store(entry);
}

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
