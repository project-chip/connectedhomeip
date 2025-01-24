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

#include <esp_log.h>
#include <tracing/esp32_diagnostic_trace/Counter.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {

std::map<const char *, uint32_t> ESPDiagnosticCounter::mCounterList;

void ESPDiagnosticCounter::IncreaseCount(const char * label)
{
    if (mCounterList.find(label) != mCounterList.end())
    {
        mCounterList[label]++;
    }
    else
    {
        mCounterList[label] = 1;
    }
}

uint32_t ESPDiagnosticCounter::GetInstanceCount(const char * label) const
{
    return mCounterList[label];
}

CHIP_ERROR ESPDiagnosticCounter::ReportMetrics(const char * label, DiagnosticStorageInterface * storageInstance)
{
    VerifyOrReturnError(storageInstance != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(DeviceLayer, "Diagnostic Storage Instance cannot be NULL"));
    Diagnostic<uint32_t> counter(label, GetInstanceCount(label), esp_log_timestamp());
    return storageInstance->Store(counter);
}

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
