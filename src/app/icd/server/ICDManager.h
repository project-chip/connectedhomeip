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

#include <app/icd/server/ICDServerConfig.h>

#include <app-common/zap-generated/cluster-enums.h>
#include <app/SubscriptionsInfoProvider.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/icd/server/ICDNotifier.h>
#include <app/icd/server/ICDStateObserver.h>
#include <credentials/FabricTable.h>
#include <crypto/SessionKeystore.h>
#include <functional>
#include <lib/support/BitFlags.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceConfig.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <system/SystemClock.h>

#if CHIP_CONFIG_ENABLE_ICD_CIP
#include <app/icd/server/ICDCheckInSender.h>   // nogncheck
#include <app/icd/server/ICDMonitoringTable.h> // nogncheck
#endif                                         // CHIP_CONFIG_ENABLE_ICD_CIP

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
class ICDManager : public ICDListener, public TestEventTriggerHandler
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

    /**
     * @brief Verifier template function
     *        This type can be used to implement specific verifiers that can be used in the CheckInMessagesWouldBeSent function.
     *        The goal is to avoid having multiple functions that implement the iterator loop with only the check changing.
     *
     * @return true if at least one Check-In message would be sent
     *         false No Check-In messages would be sent
     */

    using ShouldCheckInMsgsBeSentFunction = bool(FabricIndex aFabricIndex, NodeId subjectID);

    ICDManager() {}
    void Init(PersistentStorageDelegate * storage, FabricTable * fabricTable, Crypto::SymmetricKeystore * symmetricKeyStore,
              Messaging::ExchangeManager * exchangeManager, SubscriptionsInfoProvider * manager,
              TestEventTriggerDelegate * testEventTriggerDelegate);
    void Shutdown(TestEventTriggerDelegate * testEventTriggerDelegate);
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

    /**
     * @brief TestEventTriggerHandler for the ICD feature set
     *
     * @param eventTrigger Event trigger to handle.
     * @return CHIP_ERROR CHIP_NO_ERROR - No erros during the processing
     *                    CHIP_ERROR_INVALID_ARGUMENT - eventTrigger isn't a valid value
     */
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

#if CHIP_CONFIG_ENABLE_ICD_CIP
    void SendCheckInMsgs();

    /**
     * @brief Trigger the ICDManager to send Check-In message if necessary
     *
     * @param[in] function to use to determine if we need to send check-in messages
     */
    void TriggerCheckInMessages(const std::function<ShouldCheckInMsgsBeSentFunction> & function);

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    /**
     * @brief Set mSubCheckInBootCheckExecuted to true
     *        Function allows the InteractionModelEngine to notify the ICDManager that the boot up subscription resumption has been
     *        completed.
     */
    void SetBootUpResumeSubscriptionExecuted() { mIsBootUpResumeSubscriptionExecuted = true; };
#endif // !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    void SetTestFeatureMapValue(uint32_t featureMap) { mFeatureMap = featureMap; };
#if !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    bool GetIsBootUpResumeSubscriptionExecuted() { return mIsBootUpResumeSubscriptionExecuted; };
#endif // !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
#endif

    // Implementation of ICDListener functions.
    // Callers must origin from the chip task context or hold the ChipStack lock.
    void OnNetworkActivity() override;
    void OnKeepActiveRequest(KeepActiveFlags request) override;
    void OnActiveRequestWithdrawal(KeepActiveFlags request) override;
    void OnICDManagementServerEvent(ICDManagementEvents event) override;
    void OnSubscriptionReport() override;

protected:
    friend class TestICDManager;

    /**
     * @brief Hepler function that extends the Active Mode duration as well as the Active Mode Jitter timer for the transition to
     * iddle mode.
     */
    void ExtendActiveMode(System::Clock::Milliseconds16 extendDuration);

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
    enum class ICDTestEventTriggerEvent : uint64_t
    {
        kAddActiveModeReq    = 0x0046'0000'00000001,
        kRemoveActiveModeReq = 0x0046'0000'00000002,
    };

#if CHIP_CONFIG_ENABLE_ICD_CIP
    bool ShouldCheckInMsgsBeSentAtActiveModeFunction(FabricIndex aFabricIndex, NodeId subjectID);

    /**
     * @brief Function checks if at least one client registration would require a Check-In message
     *
     * @param[in] function  function to use to determine if a Check-In message would be sent for a given registration
     *
     * @return true At least one registration would require an Check-In message if we were entering ActiveMode.
     * @return false None of the registration would require a Check-In message either because there are no registration or
     * because they all have associated subscriptions.
     */
    bool CheckInMessagesWouldBeSent(const std::function<ShouldCheckInMsgsBeSentFunction> & function);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    KeepActiveFlags mKeepActiveFlags{ 0 };

    // Initialize mOperationalState to ActiveMode so the init sequence at bootup triggers the IdleMode behaviour first.
    OperationalState mOperationalState = OperationalState::ActiveMode;
    bool mTransitionToIdleCalled       = false;
    ObjectPool<ObserverPointer, CHIP_CONFIG_ICD_OBSERVERS_POOL_SIZE> mStateObserverPool;

#if CHIP_CONFIG_ENABLE_ICD_CIP
#if !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    bool mIsBootUpResumeSubscriptionExecuted = false;
#endif // !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
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
