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
#pragma once

#include <app/icd/ICDStateObserver.h>
#include <credentials/FabricTable.h>
#include <lib/support/BitFlags.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {

/**
 * @brief ICD Manager is responsible of processing the events and triggering the correct action for an ICD
 */
class ICDManager
{
public:
    enum class OperationalState : uint8_t
    {
        IdleMode,
        ActiveMode,
    };

    enum class ICDMode : uint8_t
    {
        SIT, // Short Interval Time ICD
        LIT, // Long Interval Time ICD
    };

    enum class KeepActiveFlags : uint8_t
    {
        kCommissioningWindowOpen = 0x01,
        kFailSafeArmed           = 0x02,
        kExpectingMsgResponse    = 0x03,
        kAwaitingMsgAck          = 0x04,
    };

    ICDManager() {}
    void Init(PersistentStorageDelegate * storage, FabricTable * fabricTable, ICDStateObserver * stateObserver);
    void Shutdown();
    void UpdateIcdMode();
    void UpdateOperationState(OperationalState state);
    void SetKeepActiveModeRequirements(KeepActiveFlags flag, bool state);
    bool IsKeepActive() { return mKeepActiveFlags.HasAny(); }
    ICDMode GetICDMode() { return mICDMode; }
    OperationalState GetOperationalState() { return mOperationalState; }

    static System::Clock::Milliseconds32 GetSITPollingThreshold() { return kSITPollingThreshold; }
    static System::Clock::Milliseconds32 GetSlowPollingInterval() { return kSlowPollingInterval; }
    static System::Clock::Milliseconds32 GetFastPollingInterval() { return kFastPollingInterval; }

protected:
    static void OnIdleModeDone(System::Layer * aLayer, void * appState);
    static void OnActiveModeDone(System::Layer * aLayer, void * appState);

private:
    // SIT ICDs should have a SlowPollingThreshold shorter than or equal to 15s (spec 9.16.1.5)
    static constexpr System::Clock::Milliseconds32 kSITPollingThreshold = System::Clock::Milliseconds32(15000);
    static constexpr System::Clock::Milliseconds32 kSlowPollingInterval = CHIP_DEVICE_CONFIG_ICD_SLOW_POLL_INTERVAL;
    static constexpr System::Clock::Milliseconds32 kFastPollingInterval = CHIP_DEVICE_CONFIG_ICD_FAST_POLL_INTERVAL;

    // Minimal constraint value of the the ICD attributes.
    static constexpr uint32_t kMinIdleModeInterval    = 500;
    static constexpr uint32_t kMinActiveModeInterval  = 300;
    static constexpr uint16_t kMinActiveModeThreshold = 300;

    bool SupportsCheckInProtocol();

    BitFlags<KeepActiveFlags> mKeepActiveFlags{ 0 };
    OperationalState mOperationalState   = OperationalState::IdleMode;
    ICDMode mICDMode                     = ICDMode::SIT;
    PersistentStorageDelegate * mStorage = nullptr;
    FabricTable * mFabricTable           = nullptr;
    ICDStateObserver * mStateObserver    = nullptr;
};

} // namespace app
} // namespace chip
