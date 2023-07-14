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

#include <app/icd/IcdEventManager.h>

using namespace chip::DeviceLayer;

namespace chip {
namespace app {

uint8_t IcdEventManager::expectedMsgCount = 0;
static_assert(UINT8_MAX >= CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS,
              "IcdEventManager::expectedMsgCount cannot hold count for the max exchange count");

CHIP_ERROR IcdEventManager::Init(IcdManager * icdManager)
{
    VerifyOrReturnError(icdManager != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    mIcdManager = icdManager;
    PlatformMgr().AddEventHandler(ICDEventHandler, reinterpret_cast<intptr_t>(mIcdManager));

    return CHIP_NO_ERROR;
}

CHIP_ERROR IcdEventManager::Shutdown()
{
    PlatformMgr().RemoveEventHandler(ICDEventHandler, reinterpret_cast<intptr_t>(nullptr));
    mIcdManager = nullptr;

    return CHIP_NO_ERROR;
}

void IcdEventManager::ICDEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    IcdManager * icdManager = reinterpret_cast<IcdManager *>(arg);

    if (icdManager == nullptr)
    {
        return;
    }

    switch (event->Type)
    {
    case DeviceEventType::kCommissioningWindowStatusChanged:
        icdManager->SetKeepActiveModeRequirements(IcdManager::KeepActiveFlags::kCommissioningWindowOpen,
                                                  event->CommissioningWindowStatus.open);
        break;
    case DeviceEventType::kFailSafeStateChanged:
        icdManager->SetKeepActiveModeRequirements(IcdManager::KeepActiveFlags::kFailSafeArmed, event->FailSafeState.armed);
        break;
    case DeviceEventType::kChipMsgSentEvent:

        // When we expect a response to a message sent, We keep the ICD in active mode until it is received
        // Otherwise, just a kick off an active mode interval/active mode threshold
        if (event->MessageSent.ExpectResponse)
        {
            expectedMsgCount++;
            icdManager->SetKeepActiveModeRequirements(IcdManager::KeepActiveFlags::kExpectingMsgResponse, true);
        }
        else
        {
            icdManager->UpdateOperationState(IcdManager::OperationalState::ActiveMode);
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
                // Should we assert?
                ChipLogError(DeviceLayer, "No response was expected by the ICD Manager");
            }

            if (expectedMsgCount == 0)
            {
                icdManager->SetKeepActiveModeRequirements(IcdManager::KeepActiveFlags::kExpectingMsgResponse, false);
            }
        }
        else if (event->RxEventContext.wasReceived)
        {
            icdManager->UpdateOperationState(IcdManager::OperationalState::ActiveMode);
        }

        break;
    case DeviceEventType::kAppWakeUpEvent:
        icdManager->UpdateOperationState(IcdManager::OperationalState::ActiveMode);
        break;
    default:
        break;
    }
}
} // namespace app
} // namespace chip
