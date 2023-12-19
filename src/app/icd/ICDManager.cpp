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
#include <app/InteractionModelEngine.h>
#include <app/icd/ICDConfig.h>
#include <app/icd/ICDConfigurationData.h>
#include <app/icd/ICDManager.h>
#include <app/icd/ICDMonitoringTable.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/LockTracker.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <stdlib.h>

namespace chip {
namespace app {

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;

static_assert(UINT8_MAX >= CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS,
              "ICDManager::mOpenExchangeContextCount cannot hold count for the max exchange count");

void ICDManager::Init(PersistentStorageDelegate * storage, FabricTable * fabricTable, Crypto::SymmetricKeystore * symmetricKeystore,
                      Messaging::ExchangeManager * exchangeManager)
{
    VerifyOrDie(storage != nullptr);
    VerifyOrDie(fabricTable != nullptr);
    VerifyOrDie(symmetricKeystore != nullptr);
    VerifyOrDie(exchangeManager != nullptr);

    bool supportLIT = SupportsFeature(Feature::kLongIdleTimeSupport);
    VerifyOrDieWithMsg((supportLIT == false) || SupportsFeature(Feature::kCheckInProtocolSupport), AppServer,
                       "The CheckIn protocol feature is required for LIT support");
    VerifyOrDieWithMsg((supportLIT == false) || SupportsFeature(Feature::kUserActiveModeTrigger), AppServer,
                       "The user ActiveMode trigger feature is required for LIT support");

    // Disabling check until LIT support is compelte
    // VerifyOrDieWithMsg((supportLIT == false) && (GetSlowPollingInterval() <= GetSITPollingThreshold()) , AppServer,
    //                    "LIT support is required for slow polling intervals superior to 15 seconds");

    mStorage     = storage;
    mFabricTable = fabricTable;
    VerifyOrDie(ICDNotifier::GetInstance().Subscribe(this) == CHIP_NO_ERROR);
    mSymmetricKeystore = symmetricKeystore;
    mExchangeManager   = exchangeManager;

    VerifyOrDie(InitCounter() == CHIP_NO_ERROR);

    // Removing the check for now since it is possible for the Fast polling
    // to be larger than the ActiveModeDuration for now
    // uint32_t activeModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDurationMs();
    // VerifyOrDie(kFastPollingInterval.count() < activeModeDuration);

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
    mStorage          = nullptr;
    mFabricTable      = nullptr;
    mStateObserverPool.ReleaseAll();
    mICDSenderPool.ReleaseAll();
}

bool ICDManager::SupportsFeature(Feature feature)
{
    // Can't use attribute accessors/Attributes::FeatureMap::Get in unit tests
#if !CONFIG_BUILD_FOR_HOST_UNIT_TEST
    uint32_t featureMap = 0;
    bool success        = (Attributes::FeatureMap::Get(kRootEndpointId, &featureMap) == EMBER_ZCL_STATUS_SUCCESS);
    return success ? ((featureMap & to_underlying(feature)) != 0) : false;
#else
    return ((mFeatureMap & to_underlying(feature)) != 0);
#endif // !CONFIG_BUILD_FOR_HOST_UNIT_TEST
}

void ICDManager::SendCheckInMsgs()
{
#if !CONFIG_BUILD_FOR_HOST_UNIT_TEST
    VerifyOrDie(mStorage != nullptr);
    VerifyOrDie(mFabricTable != nullptr);
    uint32_t counter        = ICDConfigurationData::GetInstance().GetICDCounter();
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

            bool active =
                InteractionModelEngine::GetInstance()->SubjectHasActiveSubscription(entry.fabricIndex, entry.monitoredSubject);
            if (active)
            {
                continue;
            }

            // Increment counter only once to prevent depletion of the available range.
            if (!counterIncremented)
            {
                counterIncremented = true;

                if (CHIP_NO_ERROR != IncrementCounter())
                {
                    ChipLogError(AppServer, "Incremented ICDCounter but failed to access/save to Persistent storage");
                }
            }

            // SenderPool will be released upon transition from active to idle state
            // This will happen when all ICD Check-In messages are sent on the network
            ICDCheckInSender * sender = mICDSenderPool.CreateObject(mExchangeManager);
            VerifyOrReturn(sender != nullptr, ChipLogError(AppServer, "Failed to allocate ICDCheckinSender"));

            if (CHIP_NO_ERROR != sender->RequestResolve(entry, mFabricTable, counter))
            {
                ChipLogError(AppServer, "Failed to send ICD Check-In");
            }
        }
    }
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
}

CHIP_ERROR ICDManager::InitCounter()
{
    CHIP_ERROR err;
    uint32_t temp;
    uint16_t size = static_cast<uint16_t>(sizeof(uint32_t));

    err = mStorage->SyncGetKeyValue(DefaultStorageKeyAllocator::ICDCheckInCounter().KeyName(), &temp, size);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        // First time retrieving the counter
        temp = chip::Crypto::GetRandU32();
    }
    else if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    ICDConfigurationData::GetInstance().SetICDCounter(temp);
    temp += ICDConfigurationData::ICD_CHECK_IN_COUNTER_MIN_INCREMENT;

    // Increment the count directly to minimize flash write.
    return mStorage->SyncSetKeyValue(DefaultStorageKeyAllocator::ICDCheckInCounter().KeyName(), &temp, size);
}

CHIP_ERROR ICDManager::IncrementCounter()
{
    uint32_t temp      = 0;
    StorageKeyName key = DefaultStorageKeyAllocator::ICDCheckInCounter();
    uint16_t size      = static_cast<uint16_t>(sizeof(uint32_t));

    ICDConfigurationData::GetInstance().mICDCounter++;

    if (mStorage == nullptr)
    {
        return CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND;
    }

    ReturnErrorOnFailure(mStorage->SyncGetKeyValue(key.KeyName(), &temp, size));

    if (temp == ICDConfigurationData::GetInstance().mICDCounter)
    {
        temp = ICDConfigurationData::GetInstance().mICDCounter + ICDConfigurationData::ICD_CHECK_IN_COUNTER_MIN_INCREMENT;
        return mStorage->SyncSetKeyValue(key.KeyName(), &temp, size);
    }

    return CHIP_NO_ERROR;
}

void ICDManager::UpdateICDMode()
{
    assertChipStackLockedByCurrentThread();

    ICDConfigurationData::ICDMode tempMode = ICDConfigurationData::ICDMode::SIT;

    // Device can only switch to the LIT operating mode if LIT support is present
    if (SupportsFeature(Feature::kLongIdleTimeSupport))
    {
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
    }

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

        // When the active mode interval is 0, we stay in idleMode until a notification brings the icd into active mode
        if (ICDConfigurationData::GetInstance().GetActiveModeDurationMs() > 0)
        {
            uint32_t idleModeDuration = ICDConfigurationData::GetInstance().GetIdleModeDurationSec();
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(idleModeDuration), OnIdleModeDone, this);
        }

        System::Clock::Milliseconds32 slowPollInterval = ICDConfigurationData::GetInstance().GetSlowPollingInterval();

        // Going back to Idle, all Check-In messages are sent
        mICDSenderPool.ReleaseAll();

        CHIP_ERROR err = DeviceLayer::ConnectivityMgr().SetPollingInterval(slowPollInterval);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to set Slow Polling Interval: err %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    else if (state == OperationalState::ActiveMode)
    {
        if (mOperationalState == OperationalState::IdleMode)
        {
            // An event could have brought us to the active mode.
            // Make sure the idle mode timer is stopped
            DeviceLayer::SystemLayer().CancelTimer(OnIdleModeDone, this);

            mOperationalState           = OperationalState::ActiveMode;
            uint32_t activeModeDuration = ICDConfigurationData::GetInstance().GetActiveModeDurationMs();

            if (activeModeDuration == 0 && !mKeepActiveFlags.HasAny())
            {
                // A Network Activity triggered the active mode and activeModeDuration is 0.
                // Stay active for at least Active Mode Threshold.
                activeModeDuration = ICDConfigurationData::GetInstance().GetActiveModeThresholdMs();
            }

            DeviceLayer::SystemLayer().StartTimer(System::Clock::Timeout(activeModeDuration), OnActiveModeDone, this);

            uint32_t activeModeJitterInterval =
                (activeModeDuration >= ICD_ACTIVE_TIME_JITTER_MS) ? activeModeDuration - ICD_ACTIVE_TIME_JITTER_MS : 0;
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Timeout(activeModeJitterInterval), OnTransitionToIdle, this);

            CHIP_ERROR err =
                DeviceLayer::ConnectivityMgr().SetPollingInterval(ICDConfigurationData::GetInstance().GetFastPollingInterval());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to set Fast Polling Interval: err %" CHIP_ERROR_FORMAT, err.Format());
            }

            if (SupportsFeature(Feature::kCheckInProtocolSupport))
            {
                SendCheckInMsgs();
            }

            postObserverEvent(ObserverEventType::EnterActiveMode);
        }
        else
        {
            uint16_t activeModeThreshold = ICDConfigurationData::GetInstance().GetActiveModeThresholdMs();
            DeviceLayer::SystemLayer().ExtendTimerTo(System::Clock::Timeout(activeModeThreshold), OnActiveModeDone, this);
            uint16_t activeModeJitterThreshold =
                (activeModeThreshold >= ICD_ACTIVE_TIME_JITTER_MS) ? activeModeThreshold - ICD_ACTIVE_TIME_JITTER_MS : 0;
            if (!mTransitionToIdleCalled)
            {
                DeviceLayer::SystemLayer().ExtendTimerTo(System::Clock::Timeout(activeModeJitterThreshold), OnTransitionToIdle,
                                                         this);
            }
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

    // We only reset this flag when idle mode is complete to avoid re-triggering the check when an event brings us back to active,
    // which could cause a loop.
    pICDManager->mTransitionToIdleCalled = false;
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

    if (request.Has(KeepActiveFlag::kCheckInInProgress))
    {
        // There can be multiple check-in at the same time.
        // Keep track of the requests count.
        this->mCheckInRequestCount++;
    }

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

    if (request.Has(KeepActiveFlag::kCommissioningWindowOpen) || request.Has(KeepActiveFlag::kFailSafeArmed))
    {
        // Only 1 request per type (kCommissioningWindowOpen, kFailSafeArmed)
        // remove requirement directly
        this->SetKeepActiveModeRequirements(request, false /* state */);
    }
}

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

    case ICDManagementEvents::kStayActiveRequestReceived:
        // TODO : Implement the StayActiveRequest
        // https://github.com/project-chip/connectedhomeip/issues/24259
        break;
    default:
        break;
    }
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
