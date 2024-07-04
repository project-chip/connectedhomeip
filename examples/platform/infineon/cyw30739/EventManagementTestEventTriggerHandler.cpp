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

#include "EventManagementTestEventTriggerHandler.h"

#include "SoftwareDiagnostics.h"

namespace chip {
namespace DeviceLayer {
namespace Infineon {
namespace CYW30739 {

CHIP_ERROR EventManagementTestEventTriggerHandler::HandleEventTrigger(uint64_t eventTrigger)
{
    switch (eventTrigger)
    {
    case kFillUpEventLoggingBuffer:
        return HandleFillUpEventLoggingBufferEventTriger();
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
}

void EventManagementTestEventTriggerHandler::TriggerSoftwareFaultEvent(const char * faultRecordString)
{
    OnSoftwareFaultEventHandler(faultRecordString);
}

} // namespace CYW30739
} // namespace Infineon
} // namespace DeviceLayer
} // namespace chip
