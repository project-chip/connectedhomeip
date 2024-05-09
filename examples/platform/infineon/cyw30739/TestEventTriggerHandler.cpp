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

#include "TestEventTriggerHandler.h"

#include "SoftwareDiagnostics.h"

#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <platform/CHIPDeviceLayer.h>

namespace chip {
namespace DeviceLayer {
namespace Infineon {
namespace CYW30739 {

using namespace chip::app::Clusters;

CHIP_ERROR TestEventTriggerHandler::HandleEventTrigger(uint64_t eventTrigger)
{
    switch (eventTrigger)
    {
    case kFillUpEventLoggingBuffer:
        return HandleFillUpEventLoggingBufferEventTriger();
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

CHIP_ERROR TestEventTriggerHandler::HandleFillUpEventLoggingBufferEventTriger()
{
    /* Create a fake hardware fault list. */
    GeneralFaults<kMaxHardwareFaults> hardwareFaults;
    for (uint8_t hardwareFault = to_underlying(GeneralDiagnostics::HardwareFaultEnum::kUnspecified);
         hardwareFault < kMaxHardwareFaults; hardwareFault++)
    {
        hardwareFaults.add(hardwareFault);
    }

    /* Fill up the critical logging buffer by 10 hardware faults. */
    constexpr uint8_t kHardwareFaultCountForCriticalBuffer = 10;
    for (uint8_t i = 0; i < kHardwareFaultCountForCriticalBuffer; i++)
    {
        GeneralDiagnosticsServer::Instance().OnHardwareFaultsDetect(hardwareFaults, hardwareFaults);
    }

    /* Create a fake fault message. */
    constexpr uint64_t kEncodingOverhead        = 0x40;
    constexpr uint64_t kMaxEventLoggingInfoSize = CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE - kEncodingOverhead;
    static char recordString[kMaxEventLoggingInfoSize + 1];
    memset(recordString, 0x55, kMaxEventLoggingInfoSize);
    recordString[kMaxEventLoggingInfoSize] = '\0';

    /* Fill up the info logging buffer by a software fault. */
    OnSoftwareFaultEventHandler(recordString);

    /* Fill up the debug logging buffer by a software fault. */
    OnSoftwareFaultEventHandler(recordString);

    return CHIP_NO_ERROR;
}

} // namespace CYW30739
} // namespace Infineon
} // namespace DeviceLayer
} // namespace chip
