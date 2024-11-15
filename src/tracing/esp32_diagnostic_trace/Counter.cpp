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

#include <string.h>
#include <tracing/esp32_diagnostic_trace/Counter.h>

namespace chip {
namespace Tracing {
namespace Diagnostics {

// This is a one time allocation for counters. It is not supposed to be freed.
ESPDiagnosticCounter * ESPDiagnosticCounter::mHead = nullptr;

ESPDiagnosticCounter * ESPDiagnosticCounter::GetInstance(const char * label)
{
    ESPDiagnosticCounter * current = mHead;

    while (current != nullptr)
    {
        if (strcmp(current->label, label) == 0)
        {
            current->instanceCount++;
            return current;
        }
        current = current->mNext;
    }

    // Allocate a new instance if counter is not present in the list.
    void * ptr = Platform::MemoryAlloc(sizeof(ESPDiagnosticCounter));
    VerifyOrDie(ptr != nullptr);

    ESPDiagnosticCounter * newInstance = new (ptr) ESPDiagnosticCounter(label);
    newInstance->mNext                 = mHead;
    mHead                              = newInstance;

    return newInstance;
}

int32_t ESPDiagnosticCounter::GetInstanceCount() const
{
    return instanceCount;
}

void ESPDiagnosticCounter::ReportMetrics()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Counter counter(label, instanceCount, esp_log_timestamp());
    DiagnosticStorageImpl & diagnosticStorage = DiagnosticStorageImpl::GetInstance();
    err                                       = diagnosticStorage.Store(counter);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Failed to store Counter diagnostic data"));
}

} // namespace Diagnostics
} // namespace Tracing
} // namespace chip
