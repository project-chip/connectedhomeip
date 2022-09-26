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

#include <lib/support/SafeInt.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "FailSafeContext.h"

using namespace chip::DeviceLayer;

namespace chip {
namespace app {

void FailSafeContext::HandleArmFailSafeTimer(System::Layer * layer, void * aAppState)
{
    FailSafeContext * failSafeContext = reinterpret_cast<FailSafeContext *>(aAppState);
    failSafeContext->FailSafeTimerExpired();
}

void FailSafeContext::HandleMaxCumulativeFailSafeTimer(System::Layer * layer, void * aAppState)
{
    FailSafeContext * failSafeContext = reinterpret_cast<FailSafeContext *>(aAppState);
    failSafeContext->FailSafeTimerExpired();
}

void FailSafeContext::HandleDisarmFailSafe(intptr_t arg)
{
    FailSafeContext * failSafeContext = reinterpret_cast<FailSafeContext *>(arg);
    failSafeContext->DisarmFailSafe();
}

void FailSafeContext::FailSafeTimerExpired()
{
    if (!IsFailSafeArmed())
    {
        // In case this was a pending timer event in event loop, and we had
        // done CommissioningComplete or manual disarm.
        return;
    }

    ChipLogProgress(FailSafe, "Fail-safe timer expired");
    ScheduleFailSafeCleanup(mFabricIndex, mAddNocCommandHasBeenInvoked, mUpdateNocCommandHasBeenInvoked);
}

void FailSafeContext::ScheduleFailSafeCleanup(FabricIndex fabricIndex, bool addNocCommandInvoked, bool updateNocCommandInvoked)
{
    // Not armed, but busy so cannot rearm (via General Commissioning cluster) until the flushing
    // via `HandleDisarmFailSafe` path is complete.
    // TODO: This is hacky and we need to remove all this event pushing business, to keep all fail-safe logic-only.
    mFailSafeBusy  = true;
    mFailSafeArmed = false;

    ChipDeviceEvent event;
    event.Type                                                = DeviceEventType::kFailSafeTimerExpired;
    event.FailSafeTimerExpired.fabricIndex                    = fabricIndex;
    event.FailSafeTimerExpired.addNocCommandHasBeenInvoked    = addNocCommandInvoked;
    event.FailSafeTimerExpired.updateNocCommandHasBeenInvoked = updateNocCommandInvoked;
    CHIP_ERROR status                                         = PlatformMgr().PostEvent(&event);

    if (status != CHIP_NO_ERROR)
    {
        ChipLogError(FailSafe, "Failed to post fail-safe timer expired: %" CHIP_ERROR_FORMAT, status.Format());
    }

    PlatformMgr().ScheduleWork(HandleDisarmFailSafe, reinterpret_cast<intptr_t>(this));
}

CHIP_ERROR FailSafeContext::ArmFailSafe(FabricIndex accessingFabricIndex, System::Clock::Seconds16 expiryLengthSeconds)
{
    VerifyOrReturnError(!IsFailSafeBusy(), CHIP_ERROR_INCORRECT_STATE);

    CHIP_ERROR err           = CHIP_NO_ERROR;
    bool cancelTimersIfError = false;
    if (!mFailSafeArmed)
    {
        System::Clock::Timeout maxCumulativeTimeout = System::Clock::Seconds32(CHIP_DEVICE_CONFIG_MAX_CUMULATIVE_FAILSAFE_SEC);
        SuccessOrExit(err = DeviceLayer::SystemLayer().StartTimer(maxCumulativeTimeout, HandleMaxCumulativeFailSafeTimer, this));
        cancelTimersIfError = true;
    }

    SuccessOrExit(
        err = DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(expiryLengthSeconds), HandleArmFailSafeTimer, this));

    mFailSafeArmed = true;
    mFabricIndex   = accessingFabricIndex;

exit:

    if (err != CHIP_NO_ERROR && cancelTimersIfError)
    {
        DeviceLayer::SystemLayer().CancelTimer(HandleArmFailSafeTimer, this);
        DeviceLayer::SystemLayer().CancelTimer(HandleMaxCumulativeFailSafeTimer, this);
    }
    return err;
}

void FailSafeContext::DisarmFailSafe()
{
    DeviceLayer::SystemLayer().CancelTimer(HandleArmFailSafeTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleMaxCumulativeFailSafeTimer, this);

    ResetState();

    ChipLogProgress(FailSafe, "Fail-safe cleanly disarmed");
}

void FailSafeContext::ForceFailSafeTimerExpiry()
{
    if (!IsFailSafeArmed())
    {
        return;
    }

    // Cancel the timer since we force its action
    DeviceLayer::SystemLayer().CancelTimer(HandleArmFailSafeTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleMaxCumulativeFailSafeTimer, this);

    FailSafeTimerExpired();
}

} // namespace app
} // namespace chip
