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
 *          A 'Fail Safe Context' SHALL be created on the receiver, to track fail-safe
 *          state information while the fail-safe is armed.
 */

#pragma once

#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace DeviceLayer {

class FailSafeContext
{
public:
    // ===== Members for internal use by other Device Layer components.

    /**
     * @brief
     *  Only a single fail-safe timer is started on the device, if this function is called again
     *  when the fail-safe timer is currently armed, the currently-running fail-safe timer will
     *  first be cancelled, then the fail-safe timer will be re-armed.
     */
    CHIP_ERROR ArmFailSafe(FabricIndex accessingFabricIndex, System::Clock::Timeout expiryLength);
    CHIP_ERROR DisarmFailSafe();

    inline bool IsFailSafeArmed(FabricIndex accessingFabricIndex)
    {
        return mFailSafeArmed && MatchesFabricIndex(accessingFabricIndex);
    }

    inline bool IsFailSafeArmed() const { return mFailSafeArmed; }

    inline bool MatchesFabricIndex(FabricIndex accessingFabricIndex) const
    {
        VerifyOrDie(mFailSafeArmed);
        return (accessingFabricIndex == mFabricIndex);
    }

    inline bool NocCommandHasBeenInvoked() const { return mAddNocCommandHasBeenInvoked || mUpdateNocCommandHasBeenInvoked; }
    inline bool AddNocCommandHasBeenInvoked() { return mAddNocCommandHasBeenInvoked; }
    inline bool UpdateNocCommandHasBeenInvoked() { return mUpdateNocCommandHasBeenInvoked; }

    inline void SetAddNocCommandInvoked(FabricIndex nocFabricIndex)
    {
        mAddNocCommandHasBeenInvoked = true;
        mFabricIndex                 = nocFabricIndex;
    }

    inline void SetUpdateNocCommandInvoked(FabricIndex nocFabricIndex)
    {
        mUpdateNocCommandHasBeenInvoked = true;
        mFabricIndex                    = nocFabricIndex;
    }

    inline FabricIndex GetFabricIndex() const
    {
        VerifyOrDie(mFailSafeArmed);
        return mFabricIndex;
    }

private:
    // ===== Private members reserved for use by this class only.

    bool mFailSafeArmed                  = false;
    bool mAddNocCommandHasBeenInvoked    = false;
    bool mUpdateNocCommandHasBeenInvoked = false;
    FabricIndex mFabricIndex             = kUndefinedFabricIndex;

    // TODO:: Track the state of what was mutated during fail-safe.

    static void HandleArmFailSafe(System::Layer * layer, void * aAppState);
    void FailSafeTimerExpired();
};

} // namespace DeviceLayer
} // namespace chip
