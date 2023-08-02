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
#include <app/icd/ICDManager.h>
#include <app/icd/IcdManagementServer.h>
#include <app/icd/IcdMonitoringTable.h>
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

#ifndef ICD_REPORT_ON_ENTER_ACTIVE_MODE
// Enabling this makes the device emit subscription reports when transitioning from idle to active mode.
#define ICD_REPORT_ON_ENTER_ACTIVE_MODE 0
#endif

namespace chip {
namespace app {

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;

void ICDManager::Init(PersistentStorageDelegate * storage, FabricTable * fabricTable, ICDStateObserver * stateObserver)
{
    VerifyOrDie(storage != nullptr);
    VerifyOrDie(fabricTable != nullptr);
    VerifyOrDie(stateObserver != nullptr);

    mStorage       = storage;
    mFabricTable   = fabricTable;
    mStateObserver = stateObserver;

    uint32_t activeModeInterval = IcdManagementServer::GetInstance().GetActiveModeInterval();
    VerifyOrDie(kFastPollingInterval.count() < activeModeInterval);

    UpdateIcdMode();
    UpdateOperationState(OperationalState::ActiveMode);
}

void ICDManager::Shutdown()
{
    // cancel any running timer of the icd
    DeviceLayer::SystemLayer().CancelTimer(OnIdleModeDone, this);
    DeviceLayer::SystemLayer().CancelTimer(OnActiveModeDone, this);
    mICDMode          = ICDMode::SIT;
    mOperationalState = OperationalState::IdleMode;
    mStorage          = nullptr;
    mFabricTable      = nullptr;
}

bool ICDManager::SupportsCheckInProtocol()
{
    bool success;
    uint32_t featureMap;
    success = (Attributes::FeatureMap::Get(kRootEndpointId, &featureMap) == EMBER_ZCL_STATUS_SUCCESS);
    return success ? ((featureMap & to_underlying(Feature::kCheckInProtocolSupport)) != 0) : false;
}

void ICDManager::UpdateIcdMode()
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
            IcdMonitoringTable table(*mStorage, fabricInfo.GetFabricIndex(), 1 /*Table entry limit*/);
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
        mOperationalState         = OperationalState::IdleMode;
        uint32_t idleModeInterval = IcdManagementServer::GetInstance().GetIdleModeInterval();
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Timeout(idleModeInterval), OnIdleModeDone, this);

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
            uint32_t activeModeInterval = IcdManagementServer::GetInstance().GetActiveModeInterval();
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Timeout(activeModeInterval), OnActiveModeDone, this);

            CHIP_ERROR err = DeviceLayer::ConnectivityMgr().SetPollingInterval(GetFastPollingInterval());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to set Polling Interval: err %" CHIP_ERROR_FORMAT, err.Format());
            }

            mStateObserver->OnEnterActiveMode();
        }
        else
        {
            uint16_t activeModeThreshold = IcdManagementServer::GetInstance().GetActiveModeThreshold();
            DeviceLayer::SystemLayer().ExtendTimerTo(System::Clock::Timeout(activeModeThreshold), OnActiveModeDone, this);
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
    ICDManager * pIcdManager = reinterpret_cast<ICDManager *>(appState);
    pIcdManager->UpdateOperationState(OperationalState::ActiveMode);
}

void ICDManager::OnActiveModeDone(System::Layer * aLayer, void * appState)
{
    ICDManager * pIcdManager = reinterpret_cast<ICDManager *>(appState);

    // Don't go to idle mode when we have a keep active requirement
    if (!pIcdManager->mKeepActiveFlags.HasAny())
    {
        pIcdManager->UpdateOperationState(OperationalState::IdleMode);
    }
}
} // namespace app
} // namespace chip
