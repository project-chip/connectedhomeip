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

#include "GenericEventManagementTestEventTriggerHandler.h"

#include <app/clusters/general-diagnostics-server/general-diagnostics-server.h>
#include <platform/GeneralFaults.h>

namespace chip {
namespace app {

using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

CHIP_ERROR GenericEventManagementTestEventTriggerHandler::HandleFillUpEventLoggingBufferEventTriger()
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

    /* Fill up the info logging buffer. */
    FillUpEventLoggingBufferWithFakeSoftwareFault(CHIP_DEVICE_CONFIG_EVENT_LOGGING_INFO_BUFFER_SIZE);

    /* Fill up the debug logging buffer. */
    FillUpEventLoggingBufferWithFakeSoftwareFault(CHIP_DEVICE_CONFIG_EVENT_LOGGING_DEBUG_BUFFER_SIZE);

    return CHIP_NO_ERROR;
}

void GenericEventManagementTestEventTriggerHandler::FillUpEventLoggingBufferWithFakeSoftwareFault(size_t bufferSize)
{
    /* Create a fake fault message. */
    constexpr size_t kEncodingOverhead = 0x40;
    const size_t recordSize            = bufferSize - kEncodingOverhead;
    char * recordBuffer                = static_cast<char *>(Platform::MemoryAlloc(recordSize));
    VerifyOrReturn(recordBuffer != nullptr);
    std::unique_ptr<char, decltype(&Platform::MemoryFree)> recordString(recordBuffer, &Platform::MemoryFree);
    memset(recordString.get(), 0x55, recordSize);
    recordString.get()[recordSize - 1] = '\0';

    /* Fill up the logging buffer by a software fault. */
    TriggerSoftwareFaultEvent(recordString.get());
}

} // namespace app
} // namespace chip
