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
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/ConnectivityManager.h>
#include <platform/LockTracker.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

namespace chip {
namespace app {

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;

void ICDManager::ICDManager::Init()
{
    uint32_t activeModeInterval;
    if (Attributes::ActiveModeInterval::Get(kRootEndpointId, &activeModeInterval) != EMBER_ZCL_STATUS_SUCCESS)
    {
        activeModeInterval = kMinActiveModeInterval;
    }
    VerifyOrDie(kFastPollingInterval.count() < activeModeInterval);
    UpdateIcdMode();
    UpdateOperationState(OperationalState::ActiveMode);
}

void ICDManager::ICDManager::Shutdown()
{
    // cancel any running timer of the icd
    DeviceLayer::SystemLayer().CancelTimer(OnIdleModeDone, this);
    DeviceLayer::SystemLayer().CancelTimer(OnActiveModeDone, this);
    mIcdMode          = ICDMode::SIT;
    mOperationalState = OperationalState::IdleMode;
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

    // TODO ICD LIT FIX DEPENDENCY ISSUE with app/util/IcdMonitoringTable.h and app/server:server
    // The Check In Protocol Feature is required and the slow polling interval shall also be greater than 15 seconds
    // to run an ICD in LIT mode.
    // if (kSlowPollingInterval > kICDSitModePollingThreashold && SupportsCheckInProtocol())
    // {
    //     // We can only get to LIT Mode, if at least one client is registered to the ICD device
    //     const auto & fabricTable = Server::GetInstance().GetFabricTable();
    //     for (const auto & fabricInfo : fabricTable)
    //     {
    //         PersistentStorageDelegate & storage = Server::GetInstance().GetPersistentStorage();
    //         IcdMonitoringTable table(storage, fabricInfo.GetFabricIndex(), 1);
    //         if (!table.IsEmpty())
    //         {
    //             tempMode = ICDMode::LIT;
    //             break;
    //         }
    //     }
    // }
    mIcdMode = tempMode;
}

void ICDManager::UpdateOperationState(OperationalState state)
{
    assertChipStackLockedByCurrentThread();
    // Active mode can be re-triggered.
    VerifyOrReturn(mOperationalState != state || state == OperationalState::ActiveMode);

    if (state == OperationalState::IdleMode)
    {
        mOperationalState         = OperationalState::IdleMode;
        uint32_t idleModeInterval = 0;
        if (Attributes::IdleModeInterval::Get(kRootEndpointId, &idleModeInterval) != EMBER_ZCL_STATUS_SUCCESS)
        {
            idleModeInterval = kMinIdleModeInterval;
        }
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Timeout(idleModeInterval), OnIdleModeDone, this);

        CHIP_ERROR err = DeviceLayer::ConnectivityMgr().SetPollingInterval(GetSlowPollingInterval());
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
            uint32_t activeModeInterval = 0;
            if (Attributes::ActiveModeInterval::Get(kRootEndpointId, &activeModeInterval) != EMBER_ZCL_STATUS_SUCCESS)
            {
                activeModeInterval = kMinActiveModeInterval;
            }
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Timeout(activeModeInterval), OnActiveModeDone, this);

            CHIP_ERROR err = DeviceLayer::ConnectivityMgr().SetPollingInterval(GetFastPollingInterval());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to set Polling Interval: err %" CHIP_ERROR_FORMAT, err.Format());
            }
        }
        else
        {
            uint16_t activeModeThreshold = 0;
            if (Attributes::ActiveModeThreshold::Get(kRootEndpointId, &activeModeThreshold) != EMBER_ZCL_STATUS_SUCCESS)
            {
                activeModeThreshold = kMinActiveModeThreshold;
            }
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
