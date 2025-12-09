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

#include <lib/support/CHIPMemString.h>
#include <system/SystemClock.h>
#include <tracing/esp32_diagnostics/Counter.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {

std::map<const char *, uint32_t, ESPDiagnosticCounter::StringCompare> ESPDiagnosticCounter::mCounterList;

void ESPDiagnosticCounter::IncreaseCount(const char * label)
{
    mCounterList[label]++;
}

uint32_t ESPDiagnosticCounter::GetInstanceCount(const char * label) const
{
    auto it = mCounterList.find(label);
    return (it != mCounterList.end()) ? it->second : 0;
}

CHIP_ERROR ESPDiagnosticCounter::ReportMetrics(const char * label, CircularDiagnosticBuffer * storageInstance)
{
    VerifyOrReturnError(storageInstance != nullptr, CHIP_ERROR_INCORRECT_STATE,
                        ChipLogError(DeviceLayer, "Diagnostic Storage Instance cannot be NULL"));
    DiagnosticEntry entry;
    Platform::CopyString(entry.label, label);
    entry.uintValue                = GetInstanceCount(label);
    entry.type                     = Diagnostics::ValueType::kUnsignedInteger;
    entry.timestamps_ms_since_boot = static_cast<uint32_t>(chip::System::SystemClock().GetMonotonicMilliseconds64().count());

    return storageInstance->Store(entry);
}

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
