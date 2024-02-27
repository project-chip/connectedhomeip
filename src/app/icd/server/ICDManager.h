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

#include <app-common/zap-generated/cluster-enums.h>

#include <app/icd/server/ICDServerConfig.h>

#if CHIP_CONFIG_ENABLE_ICD_CIP
#include <app/icd/server/ICDCheckInSender.h>   // nogncheck
#include <app/icd/server/ICDMonitoringTable.h> // nogncheck
#endif                                         // CHIP_CONFIG_ENABLE_ICD_CIP

#include <app/SubscriptionsInfoProvider.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/icd/server/ICDNotifier.h>
#include <app/icd/server/ICDStateObserver.h>
#include <credentials/FabricTable.h>
#include <crypto/SessionKeystore.h>
#include <lib/support/BitFlags.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <system/SystemClock.h>

namespace chip {
namespace Crypto {
using SymmetricKeystore = SessionKeystore;
}
} // namespace chip

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
    // This structure is used for the creation an ObjectPool of ICDStateObserver pointers
    struct ObserverPointer
    {
        ObserverPointer(ICDStateObserver * obs) : mObserver(obs) {}
        ~ObserverPointer() { mObserver = nullptr; }
        ICDStateObserver * mObserver;
    };

    enum class OperationalState : uint8_t
    {
        IdleMode,
        ActiveMode,
    };

    // This enum class represents to all ICDStateObserver callbacks available from the
    // mStateObserverPool for the ICDManager.
    enum class ObserverEventType : uint8_t
    {
        EnterActiveMode,
        TransitionToIdle,
        ICDModeChange,
    };

    ICDManager() {}
    void Init(PersistentStorageDelegate * storage, FabricTable * fabricTable, Crypto::SymmetricKeystore * symmetricKeyStore,
              Messaging::ExchangeManager * exchangeManager, SubscriptionsInfoProvider * manager);
    void Shutdown();
    void UpdateICDMode();
    void UpdateOperationState(OperationalState state);
    void SetKeepActiveModeRequirements(KeepActiveFlags flag, bool state);
    bool IsKeepActive() { return mKeepActiveFlags.HasAny(); }
    bool SupportsFeature(Clusters::IcdManagement::Feature feature);
    ICDConfigurationData::ICDMode GetICDMode() { return ICDConfigurationData::GetInstance().GetICDMode(); };
    /**
     * @brief Adds the referenced observer in parameters to the mStateObserverPool
     * A maximum of CHIP_CONFIG_ICD_OBSERVERS_POOL_SIZE observers can be concurrently registered
     *
     * @return The pointer to the pool object, or null if it could not be added.
     */
    ObserverPointer * RegisterObserver(ICDStateObserver * observer);

    /**
     * @brief Remove the referenced observer in parameters from the mStateObserverPool
     */
    void ReleaseObserver(ICDStateObserver * observer);

    /**
     * @brief Associates the ObserverEventType parameters to the correct
     *  ICDStateObservers function and calls it for all observers in the mStateObserverPool
     */
    void postObserverEvent(ObserverEventType event);
    OperationalState GetOperationalState() { return mOperationalState; }

    /**
     * @brief Ensures that the remaining Active Mode duration is at least the smaller of 30000 milliseconds and stayActiveDuration.
     *
     * @param stayActiveDuration The duration (in milliseconds) requested by the client to stay in Active Mode
     * @return The duration (in milliseconds) the device will stay in Active Mode
     */
    uint32_t StayActiveRequest(uint32_t stayActiveDuration);

#if CHIP_CONFIG_ENABLE_ICD_CIP
    void SendCheckInMsgs();

    /**
     * @brief Trigger the ICDManager to send Check-In message if necessary
     */
    void TriggerCheckInMessages();
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

#ifdef CONFIG_BUILD_FOR_HOST_UNIT_TEST
    void SetTestFeatureMapValue(uint32_t featureMap) { mFeatureMap = featureMap; };
#endif

    // Implementation of ICDListener functions.
    // Callers must origin from the chip task context or hold the ChipStack lock.
    void OnNetworkActivity() override;
    void OnKeepActiveRequest(KeepActiveFlags request) override;
    void OnActiveRequestWithdrawal(KeepActiveFlags request) override;
    void OnICDManagementServerEvent(ICDManagementEvents event) override;
    void OnSubscriptionReport() override;

protected:
    /**
     * @brief Hepler function that extends the Active Mode duration as well as the Active Mode Jitter timer for the transition to
     * iddle mode.
     */
    void ExtendActiveMode(System::Clock::Milliseconds16 extendDuration);

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

#if CHIP_CONFIG_ENABLE_ICD_CIP
    uint8_t mCheckInRequestCount = 0;
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    uint8_t mOpenExchangeContextCount = 0;

private:
#if CHIP_CONFIG_ENABLE_ICD_CIP
    /**
     * @brief Function checks if at least one client registration would require a Check-In message
     *
     * @return true At least one registration would require an Check-In message if we were entering ActiveMode.
     * @return false None of the registration would require a Check-In message either because there are no registration or because
     *               they all have associated subscriptions.
     */
    bool CheckInMessagesWouldBeSent();
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    KeepActiveFlags mKeepActiveFlags{ 0 };

    // Initialize mOperationalState to ActiveMode so the init sequence at bootup triggers the IdleMode behaviour first.
    OperationalState mOperationalState = OperationalState::ActiveMode;
    bool mTransitionToIdleCalled       = false;
    ObjectPool<ObserverPointer, CHIP_CONFIG_ICD_OBSERVERS_POOL_SIZE> mStateObserverPool;

#if CHIP_CONFIG_ENABLE_ICD_CIP
    PersistentStorageDelegate * mStorage           = nullptr;
    FabricTable * mFabricTable                     = nullptr;
    Messaging::ExchangeManager * mExchangeManager  = nullptr;
    Crypto::SymmetricKeystore * mSymmetricKeystore = nullptr;
    SubscriptionsInfoProvider * mSubInfoProvider   = nullptr;
    ObjectPool<ICDCheckInSender, (CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC * CHIP_CONFIG_MAX_FABRICS)> mICDSenderPool;
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

#ifdef CONFIG_BUILD_FOR_HOST_UNIT_TEST
    // feature map that can be changed at runtime for testing purposes
    uint32_t mFeatureMap = 0;
#endif
};

} // namespace app
} // namespace chip
