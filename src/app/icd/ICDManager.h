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

#include <app/icd/ICDMonitoringTable.h>
#include <app/icd/ICDNotifier.h>
#include <app/icd/ICDStateObserver.h>
#include <credentials/FabricTable.h>
#include <lib/support/BitFlags.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <system/SystemClock.h>

namespace chip {
namespace app {

// Forward declaration of TestICDManager to allow it to be friend with ICDManager
// Used in unit tests
class TestICDManager;

/**
 * @brief ICD Manager is responsible of processing the events and triggering the correct action for an ICD
 */
class ICDManager : public ICDListener
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

    ICDManager() {}
    void Init(PersistentStorageDelegate * storage, FabricTable * fabricTable, ICDStateObserver * stateObserver,
              Crypto::SymmetricKeystore * symmetricKeyStore);
    void Shutdown();
    void UpdateICDMode();
    void UpdateOperationState(OperationalState state);
    void SetKeepActiveModeRequirements(KeepActiveFlags flag, bool state);
    bool IsKeepActive() { return mKeepActiveFlags.HasAny(); }
    ICDMode GetICDMode() { return mICDMode; }
    OperationalState GetOperationalState() { return mOperationalState; }

    static System::Clock::Milliseconds32 GetSITPollingThreshold() { return kSITPollingThreshold; }
    static System::Clock::Milliseconds32 GetSlowPollingInterval() { return kSlowPollingInterval; }
    static System::Clock::Milliseconds32 GetFastPollingInterval() { return kFastPollingInterval; }

    // Implementation of ICDListener functions.
    // Callers must origin from the chip task context or be holding the ChipStack lock.
    void OnNetworkActivity() override;
    void OnKeepActiveRequest(KeepActiveFlags request) override;
    void OnActiveRequestWithdrawal(KeepActiveFlags request) override;

protected:
    friend class TestICDManager;

    static void OnIdleModeDone(System::Layer * aLayer, void * appState);
    static void OnActiveModeDone(System::Layer * aLayer, void * appState);
    /**
     * @brief Callback function called shortly before the device enters idle mode to allow checks to be made. This is currently only
     * called once to prevent entering in a loop if some events re-trigger this check (for instance if a check for subscription
     * before entering idle mode leads to emiting a report, we will re-enter UpdateOperationState and check again for subscription,
     * etc.)
     */
    static void OnTransitionToIdle(System::Layer * aLayer, void * appState);

    static uint8_t OpenExchangeContextCount;

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

    OperationalState mOperationalState             = OperationalState::IdleMode;
    ICDMode mICDMode                               = ICDMode::SIT;
    PersistentStorageDelegate * mStorage           = nullptr;
    FabricTable * mFabricTable                     = nullptr;
    ICDStateObserver * mStateObserver              = nullptr;
    bool mTransitionToIdleCalled                   = false;
    Crypto::SymmetricKeystore * mSymmetricKeystore = nullptr;
};

} // namespace app
} // namespace chip
