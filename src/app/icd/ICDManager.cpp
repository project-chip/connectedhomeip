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
    IcdManagement::Attributes::ActiveModeInterval::Get(kRootEndpointId, &activeModeInterval);
    VerifyOrDie(kFastPollingInterval.count() < activeModeInterval);
    UpdateIcdMode();
    UpdateOperationStates(ActiveMode);
}

bool ICDManager::SupportCheckInProtocol()
{
    bool success;
    uint32_t featureMap;
    success = (IcdManagement::Attributes::FeatureMap::Get(kRootEndpointId, &featureMap) == EMBER_ZCL_STATUS_SUCCESS);

    return success ? ((featureMap & to_underlying(IcdManagement::Feature::kCheckInProtocolSupport)) != 0) : false;
}

void ICDManager::UpdateIcdMode()
{
    assertChipStackLockedByCurrentThread();

    ICDMode tempMode = SIT;

    // The Check In Protocol Feature is required and the slow polling interval shall also be greater than 15 seconds
    // to run an ICD in LIT mode.
    if (kSlowPollingInterval > kICDSitModePollingThreashold && SupportCheckInProtocol())
    {
        // TODO ICD LIT FIX DEPENDENCY ISSUE with app/util/IcdMonitoringTable.h and app/server:server
        // Check In protocol not implented yet.
        // // We can only get to LIT Mode, if at least one client is registered to the ICD device
        // const auto & fabricTable = Server::GetInstance().GetFabricTable();
        // for (const auto & fabricInfo : fabricTable)
        // {
        //     PersistentStorageDelegate & storage = Server::GetInstance().GetPersistentStorage();
        //     IcdMonitoringTable table(storage, fabricInfo.GetFabricIndex(), 1);
        //     if (!table.IsEmpty())
        //     {
        //         tempMode = LIT;
        //         break;
        //     }
        // }
    }
    mIcdMode = tempMode;
}

void ICDManager::UpdateOperationStates(OperationalState state)
{
    assertChipStackLockedByCurrentThread();

    if (mOperationalState == IdleMode && state == IdleMode)
    {
        // Nothing to do in this case
        return;
    }

    if (state == IdleMode)
    {
        mOperationalState         = IdleMode;
        uint32_t idleModeInterval = 0;
        IcdManagement::Attributes::IdleModeInterval::Get(kRootEndpointId, &idleModeInterval);
        DeviceLayer::SystemLayer().StartTimer(System::Clock::Timeout(idleModeInterval), OnIdleModeDone, this);

        CHIP_ERROR err = DeviceLayer::ConnectivityMgr().SetPollingInterval(GetSlowPollingInterval());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to set Polling Interval: err %s", ErrorStr(err))
        }
    }
    else if (state == ActiveMode)
    {
        if (mOperationalState == IdleMode)
        {
            // An event could have brought use to the active mode.
            // Make sure the idle mode timer is stopped
            DeviceLayer::SystemLayer().CancelTimer(OnIdleModeDone, this);

            mOperationalState           = ActiveMode;
            uint32_t activeModeInterval = 0;
            IcdManagement::Attributes::ActiveModeInterval::Get(kRootEndpointId, &activeModeInterval);
            DeviceLayer::SystemLayer().StartTimer(System::Clock::Timeout(activeModeInterval), OnActiveModeDone, this);

            CHIP_ERROR err = DeviceLayer::ConnectivityMgr().SetPollingInterval(GetFastPollingInterval());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(AppServer, "Failed to set Polling Interval: err %s", ErrorStr(err))
            }
        }
        else
        {
            uint16_t activeModeThreshold = 0;
            IcdManagement::Attributes::ActiveModeThreshold::Get(kRootEndpointId, &activeModeThreshold);
            DeviceLayer::SystemLayer().ExtendTimerTo(System::Clock::Timeout(activeModeThreshold), OnActiveModeDone, this);
        }
    }
}

void ICDManager::SetKeepActiveModeRequirements(KeepActiveFlags flag, bool state)
{
    assertChipStackLockedByCurrentThread();

    mKeepActiveFlags.Set(flag, state);
    if (mOperationalState == IdleMode && mKeepActiveFlags.HasAny())
    {
        UpdateOperationStates(ActiveMode);
    }
    else if (mOperationalState == ActiveMode && !mKeepActiveFlags.HasAny() &&
             !DeviceLayer::SystemLayer().IsTimerActive(OnActiveModeDone, this))
    {
        // The normal active period had ended and nothing else requires the system to be active.
        UpdateOperationStates(IdleMode);
    }
}

void ICDManager::OnIdleModeDone(System::Layer * aLayer, void * appState)
{
    ICDManager * pIcdManager = reinterpret_cast<ICDManager *>(appState);
    pIcdManager->UpdateOperationStates(ActiveMode);
}

void ICDManager::OnActiveModeDone(System::Layer * aLayer, void * appState)
{
    ICDManager * pIcdManager = reinterpret_cast<ICDManager *>(appState);

    // Don't go to idle mode when we have a keep active requirement
    if (!pIcdManager->mKeepActiveFlags.HasAny())
    {
        pIcdManager->UpdateOperationStates(IdleMode);
    }
}
} // namespace app
} // namespace chip
