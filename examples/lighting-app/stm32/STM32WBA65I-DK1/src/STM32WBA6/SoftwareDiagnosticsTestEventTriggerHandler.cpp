/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include <app/clusters/software-diagnostics-server/SoftwareDiagnosticsTestEventTriggerHandler.h>
#include <app/clusters/software-diagnostics-server/software-fault-listener.h>
#include <lib/support/CHIPMemString.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/DiagnosticDataProvider.h>

#include "FreeRTOS.h"
#include "task.h"

using namespace chip;
using namespace chip::app;
using namespace chip::DeviceLayer;

namespace {

void SetTestEventTrigger_SoftwareFaultOccurred()
{
    Clusters::SoftwareDiagnostics::Events::SoftwareFault::Type softwareFault;
    char threadName[kMaxThreadNameLength + 1];
    TaskHandle_t currentTaskHandle;
    TaskStatus_t currentTaskStatus;

    currentTaskHandle = xTaskGetCurrentTaskHandle();
    vTaskGetInfo(currentTaskHandle, &currentTaskStatus, pdFALSE, eInvalid);
    Platform::CopyString(threadName, currentTaskStatus.pcTaskName);

    softwareFault.id = currentTaskStatus.xTaskNumber;
    softwareFault.name.SetValue(CharSpan::fromCharString(threadName));

    char timeChar[50] = "";
    std::time_t result = std::time(nullptr);
    // Using size of 50 as it is double the expected 25 characters "Www Mmm dd hh:mm:ss yyyy\n".
    if (std::strftime(timeChar, sizeof(timeChar), "%c", std::localtime(&result)))
    {
        softwareFault.faultRecording.SetValue(ByteSpan(Uint8::from_const_char(timeChar), strlen(timeChar)));
    }
    softwareFault.faultRecording.SetValue(ByteSpan(Uint8::from_const_char(timeChar), strlen(timeChar)));

    Clusters::SoftwareDiagnostics::SoftwareFaultListener::GlobalNotifySoftwareFaultDetect(softwareFault);
}

} // namespace

bool HandleSoftwareDiagnosticsTestEventTrigger(uint64_t eventTrigger)
{
    SoftwareDiagnosticsTrigger trigger = static_cast<SoftwareDiagnosticsTrigger>(eventTrigger);

    switch (trigger)
    {
    case SoftwareDiagnosticsTrigger::kSoftwareFault:
        ChipLogProgress(Support, "[Software-Diagnostics-Test-Event] => SoftwareFault triggered");
        SetTestEventTrigger_SoftwareFaultOccurred();
        break;

    default:
        // Trigger value is unknown to this handler
        return false;
    }

    // Trigger handled successfully
    return true;
}
