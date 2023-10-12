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
#include <app/icd/ICDManagementServer.h>
#include <app/icd/ICDManager.h>
#include <app/icd/ICDMonitoringTable.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/LockTracker.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <stdlib.h>

#ifndef ICD_ENFORCE_SIT_SLOW_POLL_LIMIT
// Set to 1 to enforce SIT Slow Polling Max value to 15seconds (spec 9.16.1.5)
#define ICD_ENFORCE_SIT_SLOW_POLL_LIMIT 0
#endif

namespace chip {
namespace app {

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;

uint8_t ICDManager::OpenExchangeContextCount = 0;
static_assert(UINT8_MAX >= CHIP_CONFIG_MAX_EXCHANGE_CONTEXTS,
              "ICDManager::OpenExchangeContextCount cannot hold count for the max exchange count");

void ICDManager::Init(PersistentStorageDelegate * storage, FabricTable * fabricTable, ICDStateObserver * stateObserver,
                      Crypto::SymmetricKeystore * symmetricKeystore)
{
    VerifyOrDie(storage != nullptr);
    VerifyOrDie(fabricTable != nullptr);
    VerifyOrDie(stateObserver != nullptr);
    VerifyOrDie(symmetricKeystore != nullptr);

    mStorage       = storage;
    mFabricTable   = fabricTable;
    mStateObserver = stateObserver;
    VerifyOrDie(ICDNotifier::GetInstance().Subscribe(this) == CHIP_NO_ERROR);
    mSymmetricKeystore = symmetricKeystore;

    uint32_t activeModeInterval = ICDManagementServer::GetInstance().GetActiveModeIntervalMs();

    ICDManagementServer::GetInstance().SetSymmetricKeystore(mSymmetricKeystore);

    VerifyOrDie(kFastPollingInterval.count() < activeModeInterval);

    UpdateICDMode();
    UpdateOperationState(OperationalState::ActiveMode);
}

void ICDManager::Shutdown()
{
    ICDNotifier::GetInstance().Unsubscribe(this);
    // cancel any running timer of the icd
    DeviceLayer::SystemLayer().CancelTimer(OnIdleModeDone, this);
    DeviceLayer::SystemLayer().CancelTimer(OnActiveModeDone, this);
    DeviceLayer::SystemLayer().CancelTimer(OnTransitionToIdle, this);
    mICDMode          = ICDMode::SIT;
    mOperationalState = OperationalState::IdleMode;
    mStorage          = nullptr;
    mFabricTable      = nullptr;
}

bool ICDManager::SupportsCheckInProtocol()
{
    bool success        = false;
    uint32_t featureMap = 0;
    // Can't use attribute accessors/Attributes::FeatureMap::Get in unit tests
#ifndef CONFIG_BUILD_FOR_HOST_UNIT_TEST
    success = (Attributes::FeatureMap::Get(kRootEndpointId, &featureMap) == EMBER_ZCL_STATUS_SUCCESS);
#endif
    return success ? ((featureMap & to_underlying(Feature::kCheckInProtocolSupport)) != 0) : false;
}

void ICDManager::UpdateICDMode()
{
    assertChipStackLockedByCurrentThread();

    ICDMode tempMode = ICDMode::SIT;

    // The Check In Protocol Feature is required and the slow polling interval shall also be greater than 15 seconds
    // to run an ICD in LIT mode.
    if (GetSlowPollingInterval() > GetSITPollingThreshold() && SupportsCheckInProtocol())
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
                tempMode = ICDMode::LIT;
                break;
            }
        }
    }
    mICDMode = tempMode;

    // When in SIT mode, the slow poll interval SHOULDN'T be greater than the SIT mode polling threshold, per spec.
    if (mICDMode == ICDMode::SIT && GetSlowPollingInterval() > GetSITPollingThreshold())
    {
        ChipLogDetail(AppServer, "The Slow Polling Interval of an ICD in SIT mode should be <= %" PRIu32 " seconds",
                      (GetSITPollingThreshold().count() / 1000));
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
        if (ICDManagementServer::GetInstance().GetActiveModeIntervalMs() > 0)
        {
            uint32_t idleModeInterval = ICDManagementServer::GetInstance().GetIdleModeIntervalSec();
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds32(idleModeInterval), OnIdleModeDone, this);
        }

        System::Clock::Milliseconds32 slowPollInterval = GetSlowPollingInterval();

#if ICD_ENFORCE_SIT_SLOW_POLL_LIMIT
        // When in SIT mode, the slow poll interval SHOULDN'T be greater than the SIT mode polling threshold, per spec.
        if (mICDMode == ICDMode::SIT && GetSlowPollingInterval() > GetSITPollingThreshold())
        {
            slowPollInterval = GetSITPollingThreshold();
        }
#endif

        CHIP_ERROR err = DeviceLayer::ConnectivityMgr().SetPollingInterval(slowPollInterval);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to set Polling Interval: err %" CHIP_ERROR_FORMAT, err.Format());
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
            uint32_t activeModeInterval = ICDManagementServer::GetInstance().GetActiveModeIntervalMs();

            if (activeModeInterval == 0 && !mKeepActiveFlags.HasAny())
            {
                // A Network Activity triggered the active mode and activeModeInterval is 0.
                // Stay active for at least Active Mode Threshold.
                activeModeInterval = ICDManagementServer::GetInstance().GetActiveModeThresholdMs();
            }

            DeviceLayer::SystemLayer().StartTimer(System::Clock::Timeout(activeModeInterval), OnActiveModeDone, this);

            uint32_t activeModeJitterInterval =
                (activeModeInterval >= ICD_ACTIVE_TIME_JITTER_MS) ? activeModeInterval - ICD_ACTIVE_TIME_JITTER_MS : 0;
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Timeout(activeModeJitterInterval), OnTransitionToIdle, this);

            CHIP_ERROR err = DeviceLayer::ConnectivityMgr().SetPollingInterval(GetFastPollingInterval());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to set Polling Interval: err %" CHIP_ERROR_FORMAT, err.Format());
            }

            mStateObserver->OnEnterActiveMode();
        }
        else
        {
            uint16_t activeModeThreshold = ICDManagementServer::GetInstance().GetActiveModeThresholdMs();
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
    pICDManager->mStateObserver->OnTransitionToIdle();
}

/* ICDListener functions. */
void ICDManager::OnKeepActiveRequest(KeepActiveFlags request)
{
    assertChipStackLockedByCurrentThread();

    if (request == KeepActiveFlags::kExchangeContextOpen)
    {
        // There can be multiple open exchange contexts at the same time.
        // Keep track of the requests count.
        this->OpenExchangeContextCount++;
        this->SetKeepActiveModeRequirements(request, true /* state */);
    }
    else /* !kExchangeContextOpen */
    {
        // Only 1 request per type (kCommissioningWindowOpen, kFailSafeArmed)
        // set requirement directly
        this->SetKeepActiveModeRequirements(request, true /* state */);
    }
}

void ICDManager::OnActiveRequestWithdrawal(KeepActiveFlags request)
{
    assertChipStackLockedByCurrentThread();

    if (request == KeepActiveFlags::kExchangeContextOpen)
    {
        // There can be multiple open exchange contexts at the same time.
        // Keep track of the requests count.
        if (this->OpenExchangeContextCount > 0)
        {
            this->OpenExchangeContextCount--;
        }
        else
        {
            ChipLogError(DeviceLayer, "The ICD Manager did not account for ExchangeContext closure");
        }

        if (this->OpenExchangeContextCount == 0)
        {
            this->SetKeepActiveModeRequirements(request, false /* state */);
        }
    }
    else /* !kExchangeContextOpen */
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
} // namespace app
} // namespace chip
