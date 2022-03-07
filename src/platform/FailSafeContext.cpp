/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *          Provides the implementation of the FailSafeContext object.
 */

#include <platform/FailSafeContext.h>

#include <platform/ConfigurationManager.h>

namespace chip {
namespace DeviceLayer {

void FailSafeContext::HandleArmFailSafe(System::Layer * layer, void * aAppState)
{
    FailSafeContext * context = reinterpret_cast<FailSafeContext *>(aAppState);
    context->CommissioningFailedTimerComplete();
}

void FailSafeContext::CommissioningFailedTimerComplete()
{
    // TODO: If the fail-safe timer expires before the CommissioningComplete command is
    // successfully invoked, conduct clean-up steps.

    ChipDeviceEvent event;
    event.Type                         = DeviceEventType::kCommissioningComplete;
    event.CommissioningComplete.Status = CHIP_ERROR_TIMEOUT;
    CHIP_ERROR status                  = PlatformMgr().PostEvent(&event);

    mFailSafeArmed            = false;
    mNocCommandHasBeenInvoked = false;

    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to post commissioning complete: %" CHIP_ERROR_FORMAT, status.Format());
    }
}

CHIP_ERROR FailSafeContext::ArmFailSafe(FabricIndex accessingFabricIndex, System::Clock::Timeout expiryLength)
{
    mFailSafeArmed = true;
    mFabricIndex   = accessingFabricIndex;
    DeviceLayer::SystemLayer().StartTimer(expiryLength, HandleArmFailSafe, this);
    return CHIP_NO_ERROR;
}

CHIP_ERROR FailSafeContext::DisarmFailSafe()
{
    mFailSafeArmed            = false;
    mNocCommandHasBeenInvoked = false;
    DeviceLayer::SystemLayer().CancelTimer(HandleArmFailSafe, this);
    return CHIP_NO_ERROR;
}

} // namespace DeviceLayer
} // namespace chip
