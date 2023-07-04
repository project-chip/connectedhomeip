/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

#include <app/icd/ICDEventManager.h>

using namespace chip::DeviceLayer;

namespace chip {
namespace app {

uint8_t ICDEventManager::expectedMsgCount = 0;

CHIP_ERROR ICDEventManager::Init(ICDManager * icdManager)
{
    VerifyOrReturnError(icdManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mICDManager = icdManager;
    PlatformMgr().AddEventHandler(ICDEventHandler, reinterpret_cast<intptr_t>(mICDManager));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ICDEventManager::Shutdown()
{
    PlatformMgr().RemoveEventHandler(ICDEventHandler, reinterpret_cast<intptr_t>(nullptr));
    mICDManager = nullptr;

    return CHIP_NO_ERROR;
}

void ICDEventManager::ICDEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    ICDManager * pIcdManager = reinterpret_cast<ICDManager *>(arg);

    if (pIcdManager == nullptr)
    {
        return;
    }

    switch (event->Type)
    {
    case DeviceEventType::kCommissioningWindowStatusChanged:
        pIcdManager->SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kCommissioningWindowOpen,
                                                   event->CommissioningWindowStatus.open);
        break;
    case DeviceEventType::kFailSafeStateChanged:
        pIcdManager->SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kFailSafeArmed, event->FailSafeState.armed);
        break;
    case DeviceEventType::kChipMsgSentEvent:

        // When we expect a response to a message sent, We keep the ICD in active mode until it is received
        // Otherwise, just a kick off an active mode interval/active mode threshold
        if (event->MessageSent.ExpectResponse)
        {
            expectedMsgCount++;
            pIcdManager->SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kExpectingMsgResponse, true);
        }
        else
        {
            pIcdManager->UpdateOperationState(ICDManager::OperationalState::ActiveMode);
        }
        break;
    case DeviceEventType::kChipMsgRxEventHandled:
        if (event->RxEventContext.clearsExpectedResponse)
        {
            if (expectedMsgCount > 0)
            {
                expectedMsgCount--;
            }
            else
            {
                // Should we assert
                ChipLogError(DeviceLayer, "No response was expected by the ICD Manager");
            }

            if (expectedMsgCount == 0)
            {
                pIcdManager->SetKeepActiveModeRequirements(ICDManager::KeepActiveFlags::kExpectingMsgResponse, false);
            }
        }
        else if (event->RxEventContext.wasReceived)
        {
            pIcdManager->UpdateOperationState(ICDManager::OperationalState::ActiveMode);
        }

        break;
    case DeviceEventType::kAppWakeUpEvent:
        pIcdManager->UpdateOperationState(ICDManager::OperationalState::ActiveMode);
        break;
    default:
        break;
    }
}
} // namespace app
} // namespace chip
