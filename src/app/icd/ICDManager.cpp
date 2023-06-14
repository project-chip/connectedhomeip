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

#include <app/clusters/icd-management-server/icd-management-server.cpp>
#include <app/icd/ICDManager.h>
#include <platform/ConnectivityManager.h>

#include
namespace chip {
namespace app {

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::IcdManagement;

ICDManager::ICDManager()
{
    uint32_t activeModeInterval;
    IcdManagement::Attributes::ActiveModeInterval::Get(kRootEndpointId, &activeModeInterval);
    VerifyOrDie(kFastPollingInterval < activeModeInterval);
    UpdateIcdMode();
    UpdateOperationStates(ActiveMode);
}

void ICDManager::UpdateIcdMode()
{
    ICDMode tempMode = SIT;

    // Per spec, To run an ICD in LIT mode, The Check In Protocol Feature is required.
    // The slow polling interval shall also be greater than 15 seconds.
    if (kSlowPollingInterval > kICDSitModePollingThreashold && IcdManagementServer::HasFeature(kCheckInProtocolSupport))
    {
        // We can only get to LIT Mode, if at least one client is registered to the ICD device
        const auto & fabricTable = Server::GetInstance().GetFabricTable();
        for (const auto & fabricInfo : fabricTable)
        {
            PersistentStorageDelegate & storage = chip::Server::GetInstance().GetPersistentStorage();
            IcdMonitoringTable table(storage, fabricInfo.GetFabricIndex(), 1);
            if (!table.IsEmpty())
            {
                tempMode = LIT;
                break;
            }
        }
    }
    mIcdMode = tempMode
}

void ICDManager::UpdateOperationStates(OperationalState state)
{
    // TODO ICD Threadsafety

    if (mOperationalState == IdleMode && state == IdleMode)
    {
        // Nothing to do in this case
        return
    }

    if (state == IdleMode)
    {
        mOperationalState = IdleMode;
        uint32_t idleModeInterval;
        IcdManagement::Attributes::IdleModeInterval::Get(kRootEndpointId, &idleModeInterval);
        DeviceLayer::SystemLayer().StartTimer(idleModeInterval, OnIdleModeDone, this);

        // TODO ICD What do we do if this fails?
        DeviceLayer::ConnectivityMgr().SetPollingInterval(kSlowPollingInterval);
    }
    else if (state == ActiveMode)
    {
        if (mOperationalState == IdleMode)
        {
            // An event could have brought use to the active mode.
            // Make sure the idle mode timer is stopped
            DeviceLayer::SystemLayer().CancelTimer(OnIdleModeDone, this);

            mOperationalState = ActiveMode;

            uint32_t activeModeInterval;
            IcdManagement::Attributes::ActiveModeInterval::Get(kRootEndpointId, &activeModeInterval);
            DeviceLayer::SystemLayer().StartTimer(activeModeInterval, OnActiveModeDone, this);

            // TODO ICD What do we do if this fails?
            DeviceLayer::ConnectivityMgr().SetPollingInterval(kFastPollingInterval);
        }
        else
        {
            uint32_t activeModeThreshold;
            IcdManagement::Attributes::ActiveModeThreshold::Get(kRootEndpointId, &activeModeThreshold);
            DeviceLayer::SystemLayer().ExtendTimerTo(activeModeThreshold, OnActiveModeDone, this);
        }
    }
}

void ICDManager::OnIdleModeDone(System::Layer * aLayer, void * appState)
{
    UpdateOperationStates(ActiveMode);
}

void ICDManager::OnActiveModeDone(System::Layer * aLayer, void * appState)
{
    UpdateOperationStates(IdleMode);
}

} // namespace app
} // namespace chip
