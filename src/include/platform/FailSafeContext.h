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

    inline bool MatchesFailSafeContextFabricIndex(FabricIndex accessingFabric) { return (accessingFabric == mFabricIndex); }

    inline bool IsFailSafeArmed(FabricIndex accessingFabric)
    {
        return mFailSafeArmed && MatchesFailSafeContextFabricIndex(accessingFabric);
    }

    inline void SetFailSafeArmed(bool armed)
    {
        if (!armed)
        {
            mFabricIndex = kUndefinedFabricIndex;
        }
        mFailSafeArmed = armed;
    }

    inline bool IsFailSafeArmed() { return mFailSafeArmed; }
    inline bool IsNocCommandInvoked() { return (mFabricIndex != kUndefinedFabricIndex); }
    inline void SetNocCommandInvoked(FabricIndex fabricId) { mFabricIndex = fabricId; }
    inline FabricIndex GetFabricIndex() { return mFabricIndex; }

private:
    // ===== Private members reserved for use by this class only.

    bool mFailSafeArmed      = false;
    FabricIndex mFabricIndex = kUndefinedFabricIndex;
};

} // namespace DeviceLayer
} // namespace chip
