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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/icd/server/ICDConfigurationData.h>
#include <app/icd/server/ICDManager.h>
#include <app/icd/server/ICDServerConfig.h>
#include <lib/core/ClusterEnums.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/LockTracker.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace {
enum class ICDTestEventTriggerEvent : uint64_t
{
    kAddActiveModeReq                = 0x0046'0000'00000001,
    kRemoveActiveModeReq             = 0x0046'0000'00000002,
    kInvalidateHalfCounterValues     = 0x0046'0000'00000003,
    kInvalidateAllCounterValues      = 0x0046'0000'00000004,
    kForceMaximumCheckInBackOffState = 0x0046'0000'00000005,
    kDSLSForceSitMode                = 0x0046'0000'00000006,
    kDSLSWithdrawSitMode             = 0x0046'0000'00000007,
};
} // namespace

namespace chip {
namespace app {

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;
using namespace System::Clock;

using chip::Protocols::InteractionModel::Status;

static_assert(UINT8_MAX >= CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS,
              "ICDManager::mOpenExchangeContextCount cannot hold count for the max exchange count");

void ICDManager::Init()
{
#if CHIP_CONFIG_ENABLE_ICD_CIP
    VerifyOrDie(mStorage != nullptr);
    VerifyOrDie(mFabricTable != nullptr);
    VerifyOrDie(mSymmetricKeystore != nullptr);
    VerifyOrDie(mExchangeManager != nullptr);
    VerifyOrDie(mSubInfoProvider != nullptr);
    VerifyOrDie(mICDCheckInBackOffStrategy != nullptr);

    VerifyOrDie(ICDConfigurationData::GetInstance().GetICDCounter().Init(mStorage, DefaultStorageKeyAllocator::ICDCheckInCounter(),
                                                                         ICDConfigurationData::kICDCounterPersistenceIncrement) ==
                CHIP_NO_ERROR);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

#if CHIP_CONFIG_ENABLE_ICD_LIT
    // LIT ICD Verification Checks
    if (SupportsFeature(Feature::kLongIdleTimeSupport))
    {
        VerifyOrDieWithMsg(SupportsFeature(Feature::kCheckInProtocolSupport), AppServer,
                           "The CheckIn protocol feature is required for LIT support.");
        VerifyOrDieWithMsg(SupportsFeature(Feature::kUserActiveModeTrigger), AppServer,
                           "The user ActiveMode trigger feature is required for LIT support.");
        VerifyOrDieWithMsg(ICDConfigurationData::GetInstance().GetMinLitActiveModeThreshold() <=
                               ICDConfigurationData::GetInstance().GetActiveModeThreshold(),
                           AppServer, "The minimum ActiveModeThreshold value for a LIT ICD is 5 seconds.");
    }
#endif // CHIP_CONFIG_ENABLE_ICD_LIT

    VerifyOrDie(ICDNotifier::GetInstance().Subscribe(this) == CHIP_NO_ERROR);

    UpdateICDMode();
    UpdateOperationState(OperationalState::IdleMode);
}

void ICDManager::Shutdown()
{
    ICDNotifier::GetInstance().Unsubscribe(this);

    // cancel any running timer of the icd
    DeviceLayer::SystemLayer().CancelTimer(OnIdleModeDone, this);
    DeviceLayer::SystemLayer().CancelTimer(OnActiveModeDone, this);
    DeviceLayer::SystemLayer().CancelTimer(OnTransitionToIdle, this);

    ICDConfigurationData::GetInstance().SetICDMode(ICDConfigurationData::ICDMode::SIT);
    mOperationalState = OperationalState::ActiveMode;
    mStateObserverPool.ReleaseAll();

#if CHIP_CONFIG_ENABLE_ICD_CIP
    mStorage         = nullptr;
    mFabricTable     = nullptr;
    mSubInfoProvider = nullptr;
    mICDSenderPool.ReleaseAll();

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    mIsBootUpResumeSubscriptionExecuted = false;
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
}

bool ICDManager::SupportsFeature(Feature feature)
{
    return ICDConfigurationData::GetInstance().GetFeatureMap().Has(feature);
}

uint32_t ICDManager::StayActiveRequest(uint32_t stayActiveDuration)
{
    // This should only be called when the device is in ActiveMode
    VerifyOrReturnValue(mOperationalState == OperationalState::ActiveMode, 0);

    uint32_t promisedActiveDuration =
        std::min(ICDConfigurationData::GetInstance().GetGuaranteedStayActiveDuration().count(), stayActiveDuration);

    // If the device is already in ActiveMode, we need to extend the active mode duration
    // for whichever is smallest between 30000 milliseconds and stayActiveDuration, taking in account the remaining active time.
    ExtendActiveMode(System::Clock::Milliseconds16(promisedActiveDuration));
    promisedActiveDuration = DeviceLayer::SystemLayer().GetRemainingTime(OnActiveModeDone, this).count();

    return promisedActiveDuration;
}

#if CHIP_CONFIG_ENABLE_ICD_CIP
void ICDManager::SendCheckInMsgs()
{
#if !(CONFIG_BUILD_FOR_HOST_UNIT_TEST)
    VerifyOrDie(mStorage != nullptr);
    VerifyOrDie(mFabricTable != nullptr);

    uint32_t counterValue   = ICDConfigurationData::GetInstance().GetICDCounter().GetNextCheckInCounterValue();
    bool counterIncremented = false;

    for (const auto & fabricInfo : *mFabricTable)
    {
        uint16_t supported_clients = ICDConfigurationData::GetInstance().GetClientsSupportedPerFabric();

        ICDMonitoringTable table(*mStorage, fabricInfo.GetFabricIndex(), supported_clients /*Table entry limit*/,
                                 mSymmetricKeystore);

        if (table.IsEmpty())
        {
            continue;
        }

        for (uint16_t i = 0; i < table.Limit(); i++)
        {
            ICDMonitoringEntry entry(mSymmetricKeystore);
            CHIP_ERROR err = table.Get(i, entry);
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                break;
            }

            if (err != CHIP_NO_ERROR)
            {
                // Try to fetch the next entry upon failure (should not happen).
                ChipLogError(AppServer, "Failed to retrieved ICDMonitoring entry for Check-In msg, will try next entry.");
                continue;
            }

            if (!ShouldCheckInMsgsBeSentAtActiveModeFunction(entry.fabricIndex, entry.monitoredSubject))
            {
                continue;
            }

            if (!mICDCheckInBackOffStrategy->ShouldSendCheckInMessage(entry))
            {
                // continue to next entry
                continue;
            }

            // Increment counter only once to prevent depletion of the available range.
            if (!counterIncremented)
            {
                counterIncremented = true;

                if (CHIP_NO_ERROR != ICDConfigurationData::GetInstance().GetICDCounter().Advance())
                {
                    ChipLogError(AppServer, "Incremented ICDCounter but failed to access/save to Persistent storage");
                }
            }

            // SenderPool will be released upon transition from active to idle state
            // This will happen when all ICD Check-In messages are sent on the network
            ICDCheckInSender * sender = mICDSenderPool.CreateObject(mExchangeManager);
            VerifyOrReturn(sender != nullptr, ChipLogError(AppServer, "Failed to allocate ICDCheckinSender"));

            if (CHIP_NO_ERROR != sender->RequestResolve(entry, mFabricTable, counterValue))
            {
                ChipLogError(AppServer, "Failed to send ICD Check-In");
            }
        }
    }
#endif // !(CONFIG_BUILD_FOR_HOST_UNIT_TEST)
}

bool ICDManager::CheckInMessagesWouldBeSent(const std::function<ShouldCheckInMsgsBeSentFunction> & shouldCheckInMsgsBeSentFunction)
{
    VerifyOrReturnValue(shouldCheckInMsgsBeSentFunction, false);

    for (const auto & fabricInfo : *mFabricTable)
    {
        uint16_t supported_clients = ICDConfigurationData::GetInstance().GetClientsSupportedPerFabric();

        ICDMonitoringTable table(*mStorage, fabricInfo.GetFabricIndex(), supported_clients /*Table entry limit*/,
                                 mSymmetricKeystore);
        if (table.IsEmpty())
        {
            continue;
        }

        for (uint16_t i = 0; i < table.Limit(); i++)
        {
            ICDMonitoringEntry entry(mSymmetricKeystore);
            CHIP_ERROR err = table.Get(i, entry);
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                break;
            }

            if (err != CHIP_NO_ERROR)
            {
                // Try to fetch the next entry upon failure (should not happen).
                ChipLogError(AppServer, "Failed to retrieved ICDMonitoring entry, will try next entry.");
                continue;
            }

            if (entry.clientType == ClientTypeEnum::kEphemeral)
            {
                // If the registered client is ephemeral, no Check-In message would be sent to this client
                continue;
            }

            // At least one registration would require a Check-In message
            VerifyOrReturnValue(!shouldCheckInMsgsBeSentFunction(entry.fabricIndex, entry.monitoredSubject), true);
        }
    }

    // None of the registrations would require a Check-In message
    return false;
}

/**
 * ShouldCheckInMsgsBeSentAtActiveModeFunction is used to determine if a Check-In message is required for a given registration.
 * Due to how the ICD Check-In use-case interacts with the persistent subscription and subscription timeout resumption features,
 * having a single implementation of the function renders the implementation very difficult to understand and maintain.
 * Because of this, each valid feature combination has its own implementation of the function.
 */
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
/**
 * @brief Implementation for when the persistent subscription and subscription timeout resumption feature are present.
 *        Function checks that there are no active or persisted subscriptions for a given fabricIndex or subjectID.
 *
 * @note When the persistent subscription and subscription timeout resumption feature are present, we need to check for
 *       persisted subscription at each transition to ActiveMode since there will be persisted subscriptions during normal
 *       operation for the subscription timeout resumption feature. Once we have finished all our subscription resumption attempts
 *       for a given subscription, the entry is deleted from persisted storage which will enable us to send Check-In messages for
 *       the client registration. This logic avoids the device sending a Check-In message while trying to resume subscriptions.
 *
 * @param aFabricIndex
 * @param subjectID subjectID to check. Can be an operational node id or a CAT
 *
 * @return true Returns true if the fabricIndex and subjectId combination does not have an active or a persisted subscription.
 * @return false Returns false if the fabricIndex and subjectId combination has an active or persisted subscription.
 */
bool ICDManager::ShouldCheckInMsgsBeSentAtActiveModeFunction(FabricIndex aFabricIndex, NodeId subjectID)
{
    return !(mSubInfoProvider->SubjectHasActiveSubscription(aFabricIndex, subjectID) ||
             mSubInfoProvider->SubjectHasPersistedSubscription(aFabricIndex, subjectID));
}
#else
/**
 * @brief Implementation for when the persistent subscription feature is present without the subscription timeout resumption
 * feature. Function checks that there are no active subscriptions. If the boot up subscription resumption has not been completed,
 *        function also checks if there are persisted subscriptions.
 *
 * @note The persistent subscriptions feature tries to resume subscriptions at the highest min interval
 *       of all the persisted subscriptions. As such, it is possible for the ICD to return to Idle Mode
 *       until the timer elaspses. We do not want to send Check-In messages to clients with persisted subscriptions
 *       until we have tried to resubscribe.
 *
 * @param aFabricIndex
 * @param subjectID subjectID to check. Can be an opperationnal node id or a CAT
 *
 * @return true Returns true if the fabricIndex and subjectId combination does not have an active subscription.
 *              If the boot up subscription resumption has not been completed, there must not be a persisted subscription either.
 * @return false Returns false if the fabricIndex and subjectId combination has an active subscription.
 *               If the boot up subscription resumption has not been completed,
 *               returns false if the fabricIndex and subjectId combination has a persisted subscription.
 */
bool ICDManager::ShouldCheckInMsgsBeSentAtActiveModeFunction(FabricIndex aFabricIndex, NodeId subjectID)
{
    bool mightHaveSubscription = mSubInfoProvider->SubjectHasActiveSubscription(aFabricIndex, subjectID);
    if (!mightHaveSubscription && !mIsBootUpResumeSubscriptionExecuted)
    {
        mightHaveSubscription = mSubInfoProvider->SubjectHasPersistedSubscription(aFabricIndex, subjectID);
    }

    return !mightHaveSubscription;
}
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
#else
/**
 * @brief Implementation for when neither the persistent subscription nor the subscription timeout resumption features are present.
 *        Function checks that there no active sbuscriptions for a given fabricIndex and subjectId combination.
 *
 * @note When neither the persistent subscription nor the subscription timeout resumption features are present, we only need to
 *       check for active subscription since we will never have any persisted subscription.
 *
 * @param aFabricIndex
 * @param subjectID subjectID to check. Can be an opperationnal node id or a CAT
 *
 * @return true Returns true if the fabricIndex and subjectId combination does not have an active subscription.
 * @return false Returns false if the fabricIndex and subjectId combination has an active subscription.
 */
bool ICDManager::ShouldCheckInMsgsBeSentAtActiveModeFunction(FabricIndex aFabricIndex, NodeId subjectID)
{
    return !(mSubInfoProvider->SubjectHasActiveSubscription(aFabricIndex, subjectID));
}
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

void ICDManager::TriggerCheckInMessages(const std::function<ShouldCheckInMsgsBeSentFunction> & verifier)
{
    VerifyOrReturn(SupportsFeature(Feature::kCheckInProtocolSupport));

    // Only trigger Check-In messages when we are in IdleMode.
    // If we are already in ActiveMode, Check-In messages have already been sent.
    VerifyOrReturn(mOperationalState == OperationalState::IdleMode);

    // If we don't have any Check-In messages to send, do nothing
    VerifyOrReturn(CheckInMessagesWouldBeSent(verifier));
    UpdateOperationState(OperationalState::ActiveMode);
}
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

void ICDManager::UpdateICDMode()
{
    assertChipStackLockedByCurrentThread();

    ICDConfigurationData::ICDMode tempMode = ICDConfigurationData::ICDMode::SIT;

#if CHIP_CONFIG_ENABLE_ICD_LIT
    // Device can only switch to the LIT operating mode if LIT support is present
    if (SupportsFeature(Feature::kLongIdleTimeSupport))
    {
#if CHIP_CONFIG_ENABLE_ICD_DSLS
        // Ensure SIT mode is not requested
        if (SupportsFeature(Feature::kDynamicSitLitSupport) && !mSITModeRequested)
        {
#endif // CHIP_CONFIG_ENABLE_ICD_DSLS

            VerifyOrDie(mStorage != nullptr);
            VerifyOrDie(mFabricTable != nullptr);
            // We can only get to LIT Mode, if at least one client is registered with the ICD device
            for (const auto & fabricInfo : *mFabricTable)
            {
                // We only need 1 valid entry to ensure LIT compliance
                ICDMonitoringTable table(*mStorage, fabricInfo.GetFabricIndex(), 1 /*Table entry limit*/, mSymmetricKeystore);
                if (!table.IsEmpty())
                {
                    tempMode = ICDConfigurationData::ICDMode::LIT;
                    break;
                }
            }
#if CHIP_CONFIG_ENABLE_ICD_DSLS
        }
#endif // CHIP_CONFIG_ENABLE_ICD_DSLS
    }
#endif // CHIP_CONFIG_ENABLE_ICD_LIT

    if (ICDConfigurationData::GetInstance().GetICDMode() != tempMode)
    {
        ICDConfigurationData::GetInstance().SetICDMode(tempMode);
        postObserverEvent(ObserverEventType::ICDModeChange);
    }

    // When in SIT mode, the slow poll interval SHOULDN'T be greater than the SIT mode polling threshold, per spec.
    if (ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::SIT &&
        ICDConfigurationData::GetInstance().GetSlowPollingInterval() > ICDConfigurationData::GetInstance().GetSITPollingThreshold())
    {
        ChipLogDetail(AppServer, "The Slow Polling Interval of an ICD in SIT mode should be <= %" PRIu32 " seconds",
                      (ICDConfigurationData::GetInstance().GetSITPollingThreshold().count() / 1000));
    }
}

void ICDManager::UpdateOperationState(OperationalState state)
{
    assertChipStackLockedByCurrentThread();
    // Active mode can be re-triggered.
    VerifyOrReturn(mOperationalState != state || state == OperationalState::ActiveMode);

    if (state == OperationalState::IdleMode)
    {
        mOperationalState = OperationalState::IdleMode;

#if CHIP_CONFIG_ENABLE_ICD_CIP
        std::function<ShouldCheckInMsgsBeSentFunction> sendCheckInMessagesOnActiveMode =
            std::bind(&ICDManager::ShouldCheckInMsgsBeSentAtActiveModeFunction, this, std::placeholders::_1, std::placeholders::_2);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

        // When the active mode interval is 0, we stay in idleMode until a notification brings the icd into active mode
        // unless the device would need to send Check-In messages
        if (ICDConfigurationData::GetInstance().GetActiveModeDuration() > kZero
#if CHIP_CONFIG_ENABLE_ICD_CIP
            || CheckInMessagesWouldBeSent(sendCheckInMessagesOnActiveMode)
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
        )
        {
            DeviceLayer::SystemLayer().StartTimer(ICDConfigurationData::GetInstance().GetIdleModeDuration(), OnIdleModeDone, this);
        }

        Milliseconds32 slowPollInterval = ICDConfigurationData::GetInstance().GetSlowPollingInterval();

#if CHIP_CONFIG_ENABLE_ICD_CIP
        // Going back to Idle, all Check-In messages are sent
        mICDSenderPool.ReleaseAll();
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

        CHIP_ERROR err = DeviceLayer::ConnectivityMgr().SetPollingInterval(slowPollInterval);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to set Slow Polling Interval: err %" CHIP_ERROR_FORMAT, err.Format());
        }

        postObserverEvent(ObserverEventType::EnterIdleMode);
    }
    else if (state == OperationalState::ActiveMode)
    {
        if (mOperationalState == OperationalState::IdleMode)
        {
            // An event could have brought us to the active mode.
            // Make sure the idle mode timer is stopped
            DeviceLayer::SystemLayer().CancelTimer(OnIdleModeDone, this);

            mOperationalState                 = OperationalState::ActiveMode;
            Milliseconds32 activeModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDuration();

            if (activeModeDuration == kZero && !mKeepActiveFlags.HasAny())
            {
                // Network Activity triggered the active mode and activeModeDuration is 0.
                // Stay active for at least Active Mode Threshold.
                activeModeDuration = ICDConfigurationData::GetInstance().GetActiveModeThreshold();
            }

            DeviceLayer::SystemLayer().StartTimer(activeModeDuration, OnActiveModeDone, this);

            Milliseconds32 activeModeJitterInterval = Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS);
            // TODO(#33074): Edge case when we transition to IdleMode with this condition being true
            // (activeModeDuration == kZero && !mKeepActiveFlags.HasAny())
            activeModeJitterInterval =
                (activeModeDuration >= activeModeJitterInterval) ? activeModeDuration - activeModeJitterInterval : kZero;

            // Reset this flag when we enter ActiveMode to avoid having a feedback loop that keeps us indefinitly in
            // ActiveMode.
            mTransitionToIdleCalled = false;
            DeviceLayer::SystemLayer().StartTimer(activeModeJitterInterval, OnTransitionToIdle, this);

            CHIP_ERROR err =
                DeviceLayer::ConnectivityMgr().SetPollingInterval(ICDConfigurationData::GetInstance().GetFastPollingInterval());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to set Fast Polling Interval: err %" CHIP_ERROR_FORMAT, err.Format());
            }

#if CHIP_CONFIG_ENABLE_ICD_CIP
            if (SupportsFeature(Feature::kCheckInProtocolSupport))
            {
                SendCheckInMsgs();
            }
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

            postObserverEvent(ObserverEventType::EnterActiveMode);
        }
        else
        {
            ExtendActiveMode(ICDConfigurationData::GetInstance().GetActiveModeThreshold());
        }
    }
}

void ICDManager::SetKeepActiveModeRequirements(KeepActiveFlags flag, bool state)
{
    assertChipStackLockedByCurrentThread();

    mKeepActiveFlags.Set(flag, state);
    if (mOperationalState == OperationalState::IdleMode && mKeepActiveFlags.HasAny())
    {
        UpdateOperationState(OperationalState::ActiveMode);
    }
    else if (mOperationalState == OperationalState::ActiveMode && !mKeepActiveFlags.HasAny() &&
             !DeviceLayer::SystemLayer().IsTimerActive(OnActiveModeDone, this))
    {
        // The normal active period had ended and nothing else requires the system to be active.
        UpdateOperationState(OperationalState::IdleMode);
    }
}

void ICDManager::OnIdleModeDone(System::Layer * aLayer, void * appState)
{
    ICDManager * pICDManager = reinterpret_cast<ICDManager *>(appState);
    pICDManager->UpdateOperationState(OperationalState::ActiveMode);
}

void ICDManager::OnActiveModeDone(System::Layer * aLayer, void * appState)
{
    ICDManager * pICDManager = reinterpret_cast<ICDManager *>(appState);

    // Don't go to idle mode when we have a keep active requirement
    if (!pICDManager->mKeepActiveFlags.HasAny())
    {
        pICDManager->UpdateOperationState(OperationalState::IdleMode);
    }
}

void ICDManager::OnTransitionToIdle(System::Layer * aLayer, void * appState)
{
    ICDManager * pICDManager = reinterpret_cast<ICDManager *>(appState);

    // OnTransitionToIdle will trigger a report message if reporting is needed, which should extend the active mode until the
    // ack for the report is received.
    pICDManager->mTransitionToIdleCalled = true;
    pICDManager->postObserverEvent(ObserverEventType::TransitionToIdle);
}

/* ICDListener functions. */

void ICDManager::OnKeepActiveRequest(KeepActiveFlags request)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturn(request < KeepActiveFlagsValues::kInvalidFlag);

    if (request.Has(KeepActiveFlag::kExchangeContextOpen))
    {
        // There can be multiple open exchange contexts at the same time.
        // Keep track of the requests count.
        this->mOpenExchangeContextCount++;
    }

#if CHIP_CONFIG_ENABLE_ICD_CIP
    if (request.Has(KeepActiveFlag::kCheckInInProgress))
    {
        // There can be multiple check-in at the same time.
        // Keep track of the requests count.
        this->mCheckInRequestCount++;
    }
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    this->SetKeepActiveModeRequirements(request, true /* state */);
}

void ICDManager::OnActiveRequestWithdrawal(KeepActiveFlags request)
{
    assertChipStackLockedByCurrentThread();
    VerifyOrReturn(request < KeepActiveFlagsValues::kInvalidFlag);

    if (request.Has(KeepActiveFlag::kExchangeContextOpen))
    {
        // There can be multiple open exchange contexts at the same time.
        // Keep track of the requests count.
        if (this->mOpenExchangeContextCount > 0)
        {
            this->mOpenExchangeContextCount--;
        }
        else
        {
            ChipLogError(DeviceLayer, "The ICD Manager did not account for ExchangeContext closure");
        }

        if (this->mOpenExchangeContextCount == 0)
        {
            this->SetKeepActiveModeRequirements(KeepActiveFlag::kExchangeContextOpen, false /* state */);
        }
    }

#if CHIP_CONFIG_ENABLE_ICD_CIP
    if (request.Has(KeepActiveFlag::kCheckInInProgress))
    {
        // There can be multiple open exchange contexts at the same time.
        // Keep track of the requests count.
        if (this->mCheckInRequestCount > 0)
        {
            this->mCheckInRequestCount--;
        }
        else
        {
            ChipLogError(DeviceLayer, "The ICD Manager did not account for Check-In Sender start");
        }

        if (this->mCheckInRequestCount == 0)
        {
            this->SetKeepActiveModeRequirements(KeepActiveFlag::kCheckInInProgress, false /* state */);
        }
    }
#endif // CHIP_CONFIG_ENABLE_ICD_CIP

    if (request.Has(KeepActiveFlag::kCommissioningWindowOpen) || request.Has(KeepActiveFlag::kFailSafeArmed))
    {
        // Only 1 request per type (kCommissioningWindowOpen, kFailSafeArmed)
        // remove requirement directly
        this->SetKeepActiveModeRequirements(request, false /* state */);
    }
}

#if CHIP_CONFIG_ENABLE_ICD_DSLS
void ICDManager::OnSITModeRequest()
{
    mSITModeRequested = true;
    this->UpdateICDMode();
    // Update the poll interval also to comply with SIT requirements
    UpdateOperationState(OperationalState::ActiveMode);
}

void ICDManager::OnSITModeRequestWithdrawal()
{
    mSITModeRequested = false;
    this->UpdateICDMode();
    // Update the poll interval also to comply with LIT requirements
    UpdateOperationState(OperationalState::ActiveMode);
}
#endif // CHIP_CONFIG_ENABLE_ICD_DSLS

void ICDManager::OnNetworkActivity()
{
    this->UpdateOperationState(OperationalState::ActiveMode);
}

void ICDManager::OnICDManagementServerEvent(ICDManagementEvents event)
{
    switch (event)
    {
    case ICDManagementEvents::kTableUpdated:
        this->UpdateICDMode();
        break;
    default:
        break;
    }
}

void ICDManager::OnSubscriptionReport()
{
    // If the device is already in ActiveMode, that means that all active subscriptions have already been marked dirty.
    // Since we only mark them dirty when we enter ActiveMode, it is not necessary to update the operational state a second time.
    // Doing so will only add an ActiveModeThreshold to the active time which we don't want to do here.
    VerifyOrReturn(mOperationalState == OperationalState::IdleMode);
    this->UpdateOperationState(OperationalState::ActiveMode);
}

void ICDManager::ExtendActiveMode(Milliseconds16 extendDuration)
{
    DeviceLayer::SystemLayer().ExtendTimerTo(extendDuration, OnActiveModeDone, this);

    Milliseconds32 activeModeJitterThreshold = Milliseconds32(ICD_ACTIVE_TIME_JITTER_MS);
    activeModeJitterThreshold = (extendDuration >= activeModeJitterThreshold) ? extendDuration - activeModeJitterThreshold : kZero;

    if (!mTransitionToIdleCalled)
    {
        DeviceLayer::SystemLayer().ExtendTimerTo(activeModeJitterThreshold, OnTransitionToIdle, this);
    }
}

CHIP_ERROR ICDManager::HandleEventTrigger(uint64_t eventTrigger)
{
    ICDTestEventTriggerEvent trigger = static_cast<ICDTestEventTriggerEvent>(eventTrigger);
    CHIP_ERROR err                   = CHIP_NO_ERROR;

    switch (trigger)
    {
    case ICDTestEventTriggerEvent::kAddActiveModeReq:
        SetKeepActiveModeRequirements(KeepActiveFlag::kTestEventTriggerActiveMode, true);
        break;
    case ICDTestEventTriggerEvent::kRemoveActiveModeReq:
        SetKeepActiveModeRequirements(KeepActiveFlag::kTestEventTriggerActiveMode, false);
        break;
#if CHIP_CONFIG_ENABLE_ICD_CIP
    case ICDTestEventTriggerEvent::kInvalidateHalfCounterValues:
        err = ICDConfigurationData::GetInstance().GetICDCounter().InvalidateHalfCheckInCounterValues();
        break;
    case ICDTestEventTriggerEvent::kInvalidateAllCounterValues:
        err = ICDConfigurationData::GetInstance().GetICDCounter().InvalidateAllCheckInCounterValues();
        break;
    case ICDTestEventTriggerEvent::kForceMaximumCheckInBackOffState:
        err = mICDCheckInBackOffStrategy->ForceMaximumCheckInBackoff();
        break;
#endif // CHIP_CONFIG_ENABLE_ICD_CIP
#if CHIP_CONFIG_ENABLE_ICD_DSLS
    case ICDTestEventTriggerEvent::kDSLSForceSitMode:
        OnSITModeRequest();
        break;
    case ICDTestEventTriggerEvent::kDSLSWithdrawSitMode:
        OnSITModeRequestWithdrawal();
        break;
#endif // CHIP_CONFIG_ENABLE_ICD_DSLS
    default:
        err = CHIP_ERROR_INVALID_ARGUMENT;
        break;
    }

    return err;
}

ICDManager::ObserverPointer * ICDManager::RegisterObserver(ICDStateObserver * observer)
{
    return mStateObserverPool.CreateObject(observer);
}

void ICDManager::ReleaseObserver(ICDStateObserver * observer)
{
    mStateObserverPool.ForEachActiveObject([this, observer](ObserverPointer * obs) {
        if (obs->mObserver == observer)
        {
            mStateObserverPool.ReleaseObject(obs);
            return Loop::Break;
        }
        return Loop::Continue;
    });
}

void ICDManager::postObserverEvent(ObserverEventType event)
{
    mStateObserverPool.ForEachActiveObject([event](ObserverPointer * obs) {
        switch (event)
        {
        case ObserverEventType::EnterActiveMode: {
            obs->mObserver->OnEnterActiveMode();
            return Loop::Continue;
        }
        case ObserverEventType::EnterIdleMode: {
            obs->mObserver->OnEnterIdleMode();
            return Loop::Continue;
        }
        case ObserverEventType::TransitionToIdle: {
            obs->mObserver->OnTransitionToIdle();
            return Loop::Continue;
        }
        case ObserverEventType::ICDModeChange: {
            obs->mObserver->OnICDModeChange();
            return Loop::Continue;
        }
        default: {
            ChipLogError(DeviceLayer, "Invalid ICD Observer event type");
            return Loop::Break;
        }
        }
    });
}

} // namespace app
} // namespace chip
