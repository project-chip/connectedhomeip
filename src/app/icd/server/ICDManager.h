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

#include <app/AppConfig.h>
#include <app/SubscriptionsInfoProvider.h>
#include <app/TestEventTriggerDelegate.h>
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

#if CHIP_CONFIG_ENABLE_ICD_CIP
#include <app/icd/server/ICDCheckInBackOffStrategy.h> // nogncheck
#include <app/icd/server/ICDCheckInSender.h>          // nogncheck
#include <app/icd/server/ICDMonitoringTable.h>        // nogncheck
#endif                                                // CHIP_CONFIG_ENABLE_ICD_CIP

namespace chip {
namespace Crypto {
using SymmetricKeystore = SessionKeystore;
}
} // namespace chip

namespace chip {
namespace app {

// Forward declaration of TestICDManager tests to allow it to be friend with ICDManager
// Used in unit tests
class TestICDManager_TestShouldCheckInMsgsBeSentAtActiveModeFunction_Test;

/**
 * @brief ICD Manager is responsible of processing the events and triggering the correct action for an ICD
 */
class ICDManager : public ICDListener, public TestEventTriggerHandler
{
public:
    /**
     * @brief This structure is used for the creation an ObjectPool of ICDStateObserver pointers
     */
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

    /**
     * @brief This enum class represents all ICDStateObserver callbacks available from the
     *        mStateObserverPool for the ICDManager.
     *
     *        EnterActiveMode, TransitionToIdle and EnterIdleMode will always be called as a trio in the same order.
     *        Each event will only be called once per cycle.
     *        EnterActiveMode will always be called first, when the ICD has transitioned to ActiveMode.
     *        TransitionToIdle will always be second. This event will only be called the first time there is
     *        `ICD_ACTIVE_TIME_JITTER_MS` remaining to the ActiveMode timer.
     *         When this event is called, the ICD is still in ActiveMode.
     *        If the ActiveMode timer is increased due to the TransitionToIdle event, the event will not be called a second time in
     *        a given cycle.
     *        OnEnterIdleMode will always the third event and indicates that the ICD has transitioned to IdleMode.
     *
     *        The ICDModeChange event can occur independently from the EnterActiveMode, TransitionToIdle and EnterIdleMode.
     *        It will typically happen at the ICDManager init when a client is already registered with the ICD before the
     *        OnEnterIdleMode event or when a client sends a register command after the OnEnterActiveMode event. Nothing prevents
     *        the ICDModeChange event from happening multiple times per cycle or while the ICD is in IdleMode.
     *
     *        See src/app/icd/server/ICDStateObserver.h for more information on the APIs each event triggers
     */
    enum class ObserverEventType : uint8_t
    {
        EnterActiveMode,
        EnterIdleMode,
        TransitionToIdle,
        ICDModeChange,
    };

    /**
     * @brief Verifier template function
     *        This type can be used to implement specific verifiers that can be used in the CheckInMessagesWouldBeSent function.
     *        The goal is to avoid having multiple functions that implement the iterator loop with only the check changing.
     *
     * @return true: if at least one Check-In message would be sent
     *         false: No Check-In messages would be sent
     */
    using ShouldCheckInMsgsBeSentFunction = bool(FabricIndex aFabricIndex, NodeId subjectID);

    ICDManager()  = default;
    ~ICDManager() = default;

    /*
        Builder function to set all necessary members for the ICDManager class
    */

#if CHIP_CONFIG_ENABLE_ICD_CIP
    ICDManager & SetPersistentStorageDelegate(PersistentStorageDelegate * storage)
    {
        mStorage = storage;
        return *this;
    };

    ICDManager & SetFabricTable(FabricTable * fabricTable)
    {
        mFabricTable = fabricTable;
        return *this;
    };

    ICDManager & SetSymmetricKeyStore(Crypto::SymmetricKeystore * symmetricKeystore)
    {
        mSymmetricKeystore = symmetricKeystore;
        return *this;
    };

    ICDManager & SetExchangeManager(Messaging::ExchangeManager * exchangeManager)
    {
        mExchangeManager = exchangeManager;
        return *this;
    };

    ICDManager & SetSubscriptionsInfoProvider(SubscriptionsInfoProvider * subInfoProvider)
    {
        mSubInfoProvider = subInfoProvider;
        return *this;
    };

    ICDManager & SetICDCheckInBackOffStrategy(ICDCheckInBackOffStrategy * strategy)
    {
        mICDCheckInBackOffStrategy = strategy;
        return *this;
    };
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    /**
     * @brief Validates that the ICDManager has all the necessary members to function and initializes the class
     */
    void Init();
    void Shutdown();

    /**
     * @brief SupportsFeature verifies if a given FeatureMap bit is enabled
     *
     * @param[in] feature FeatureMap bit to verify
     *
     * @return true: if the FeatureMap bit is enabled in the ICDM cluster attribute.
     *         false: if the FeatureMap bit is not enabled in the ICDM cluster attribute.
     *                if we failed to read the FeatureMap attribute.
     */
    bool SupportsFeature(Clusters::IcdManagement::Feature feature);

    // See ICDConfigurationData::SetModeDurations
    CHIP_ERROR SetModeDurations(Optional<System::Clock::Milliseconds32> activeModeDuration,
                                Optional<System::Clock::Milliseconds32> idleModeDuration)
    {
        return ICDConfigurationData::GetInstance().SetModeDurations(activeModeDuration, idleModeDuration);
    };

    ICDConfigurationData::ICDMode GetICDMode() { return ICDConfigurationData::GetInstance().GetICDMode(); };

    OperationalState GetOperaionalState() { return mOperationalState; };

    /**
     * @brief Adds the referenced observer in parameters to the mStateObserverPool
     * A maximum of CHIP_CONFIG_ICD_OBSERVERS_POOL_SIZE observers can be concurrently registered
     *
     * @return The pointer to the pool object, or null if it could not be added.
     */
    ObserverPointer * RegisterObserver(ICDStateObserver * observer);

    /**
     * @brief Remove the referenced observer in parameters from the mStateObserverPool
     *        If the observer is not present in the object pool, we do nothing
     */
    void ReleaseObserver(ICDStateObserver * observer);

    /**
     * @brief Ensures that the remaining Active Mode duration is at least the smaller of 30000 milliseconds and stayActiveDuration.
     *
     * @param[in] stayActiveDuration The duration (in milliseconds) requested by the client to stay in Active Mode
     * @return The duration (in milliseconds) the device will stay in Active Mode
     */
    uint32_t StayActiveRequest(uint32_t stayActiveDuration);

    /**
     * @brief TestEventTriggerHandler for the ICD feature set
     *
     * @param[in] eventTrigger Event trigger to handle.
     *
     * @return CHIP_ERROR CHIP_NO_ERROR - No erros during the processing
     *                    CHIP_ERROR_INVALID_ARGUMENT - eventTrigger isn't a valid value
     */
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

#if CHIP_CONFIG_ENABLE_ICD_CIP
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
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    bool GetIsBootUpResumeSubscriptionExecuted() { return mIsBootUpResumeSubscriptionExecuted; };
#endif // !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

    // Implementation of ICDListener functions.
    // Callers must origin from the chip task context or hold the ChipStack lock.

    void OnNetworkActivity() override;
    void OnKeepActiveRequest(KeepActiveFlags request) override;
    void OnActiveRequestWithdrawal(KeepActiveFlags request) override;

#if CHIP_CONFIG_ENABLE_ICD_DSLS
    void OnSITModeRequest() override;
    void OnSITModeRequestWithdrawal() override;
#endif

    void OnICDManagementServerEvent(ICDManagementEvents event) override;
    void OnSubscriptionReport() override;

private:
    // TODO : Once <gtest/gtest_prod.h> can be included, use FRIEND_TEST for the friend class.
    friend class TestICDManager_TestShouldCheckInMsgsBeSentAtActiveModeFunction_Test;

    /**
     * @brief UpdateICDMode evaluates in which mode the ICD can be in; SIT or LIT mode.
     *        If the current operating mode does not match the evaluated operating mode, function updates the ICDMode and triggers
     *        all necessary operations.
     *        For a SIT ICD, this function does nothing.
     *        For a LIT ICD, the function checks if the ICD has a registration in the ICDMonitoringTable to determine which ICDMode
     *        the ICD must be in.
     */
    void UpdateICDMode();

    /**
     * @brief UpdateOperationState updates the OperationState of the ICD to the requested one.
     *        IdleMode -> IdleMode     : No actions are necessary, do nothing.
     *        IdleMode -> ActiveMode   : Transition the device to ActiveMode, start the  ActiveMode timer and trigger all necessary
     *                                   operations. These operations could be : Send Check-In messages
     *                                                                           Send subscription reports
     *                                                                           Process user actions
     *        ActiveMode -> ActiveMode : Increase remaining ActiveMode timer to one ActiveModeThreshold.
     *                                   If ActiveModeThreshold is 0, do nothing.
     *        ActiveMode -> IdleMode   : Transition ICD to IdleMode and start the IdleMode timer.
     *
     * @param state requested OperationalState for the ICD to transition to
     */
    void UpdateOperationState(OperationalState state);

    /**
     * @brief Set or Remove a keep ActiveMode requirement for the given flag
     *        If state is true and the ICD is in IdleMode, transition the ICD to ActiveMode
     *        If state is false and the ICD is in ActiveMode, check whether we can transition the ICD to IdleMode.
     *        If we can, transition the ICD to IdleMode.
     *
     * @param flag KeepActiveFlag to remove or add
     * @param state true: adding a flag requirement
     *              false: removing a flag requirement
     */
    void SetKeepActiveModeRequirements(KeepActiveFlags flag, bool state);

    /**
     * @brief Associates the ObserverEventType parameters to the correct
     *  ICDStateObservers function and calls it for all observers in the mStateObserverPool
     */
    void postObserverEvent(ObserverEventType event);

    /**
     * @brief Hepler function that extends the ActiveMode timer as well as the Active Mode Jitter timer for the transition to
     *        idle mode event.
     */
    void ExtendActiveMode(System::Clock::Milliseconds16 extendDuration);

    /**
     * @brief Timer callback function for when the IdleMode timer expires
     *
     * @param appState pointer to the ICDManager
     */
    static void OnIdleModeDone(System::Layer * aLayer, void * appState);

    /**
     * @brief Timer callback function for when the ActiveMode timer expires
     *
     * @param appState pointer to the ICDManager
     */
    static void OnActiveModeDone(System::Layer * aLayer, void * appState);

    /**
     * @brief Timer Callback function called shortly before the device enters idle mode to allow checks to be made.
     *        This is currently only called once to prevent entering in a loop if some events re-trigger this check (for instance if
     *        a check for subscriptions before entering idle mode leads to emiting a report, we will re-enter UpdateOperationState
     *        and check again for subscription, etc.)
     *
     * @param appState pointer to the ICDManager
     */
    static void OnTransitionToIdle(System::Layer * aLayer, void * appState);

#if CHIP_CONFIG_ENABLE_ICD_CIP
    /**
     * @brief Function triggers all necessary Check-In messages to be sent.
     *
     * @note For each ICDMonitoring entry, we check if should send a Check-In message with
     *       ShouldCheckInMsgsBeSentAtActiveModeFunction. If we should, we allocate an ICDCheckInSender which tries to send a
     *       Check-In message to the registered client.
     */
    void SendCheckInMsgs();

    /**
     * @brief See function implementation in .cpp for details on this function.
     */
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
    uint8_t mOpenExchangeContextCount = 0;

#if CHIP_CONFIG_ENABLE_ICD_DSLS
    bool mSITModeRequested = false;
#endif

#if CHIP_CONFIG_ENABLE_ICD_CIP
    uint8_t mCheckInRequestCount = 0;

#if !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    bool mIsBootUpResumeSubscriptionExecuted = false;
#endif // !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION && CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

    PersistentStorageDelegate * mStorage                   = nullptr;
    FabricTable * mFabricTable                             = nullptr;
    Messaging::ExchangeManager * mExchangeManager          = nullptr;
    Crypto::SymmetricKeystore * mSymmetricKeystore         = nullptr;
    SubscriptionsInfoProvider * mSubInfoProvider           = nullptr;
    ICDCheckInBackOffStrategy * mICDCheckInBackOffStrategy = nullptr;
    ObjectPool<ICDCheckInSender, (CHIP_CONFIG_ICD_CLIENTS_SUPPORTED_PER_FABRIC * CHIP_CONFIG_MAX_FABRICS)> mICDSenderPool;
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
};

} // namespace app
} // namespace chip
