/*
 *
 *    Copyright (c) 2020-2026 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
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

// stdlib
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// POSIX
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Local includes
#include <commissioning-proxy-delegate-impl.h>
#include "CommissioningProxyManager.h"
#include <app/clusters/commissioning-proxy-server/CommissioningProxyCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <lib/support/logging/CHIPLogging.h>

// Location of the executable to use by default to power dumb USB light up and
// down
#define UHUBCTL_PATHNAME "/usr/sbin/uhubctl"

// Name of the environment variable where additional UHUBCTL arguments may be
// specified
#define UHUBCTL_ARGUMENT_ENV_NAME "MORSE_MICRO_MATTER_UHUBCTL_COMMAND"

#if 0
// CommissioningProxyManager sCommissioningProxy;
//CommissioningProxyManager CommissioningProxyManager::sCommissioningProxy;
//CommissioningProxyManager::~CommissioningProxyManager() = default;

void CommissioningProxyManager::SetCommissioningProxyServer(chip::app::Clusters::CommissioningProxyCluster * server)
{
    mServer = server;
}

// Code Driven
// In a .cpp file
MyDEMDelegate gMyDEMDelegate;

chip::BitMask<chip::app::Clusters::DeviceEnergyManagement::Feature> gFeatures(
    chip::app::Clusters::DeviceEnergyManagement::Feature::kPowerAdjustment,
    chip::app::Clusters::DeviceEnergyManagement::Feature::kPowerForecastReporting
);

chip::app::RegisteredServerCluster<chip::app::Clusters::DeviceEnergyManagementCluster> gDEMCluster(
    chip::app::Clusters::DeviceEnergyManagementCluster::Config(kYourEndpointId, gFeatures, &gMyDEMDelegate)
);

// In a .cpp file
// CommissioningProxyManager gCommissioningProxyManager;
chip::app::Clusters::CommissioningProxy::MyCPDelegate gMyCPDelegate;

chip::BitMask<chip::app::Clusters::CommissioningProxy::Feature> gFeatures(
    chip::app::Clusters::CommissioningProxy::Feature::kBackgroundScan,
    chip::app::Clusters::CommissioningProxy::Feature::kWiFiNetworkInterface
);

uint8_t chip::app::Clusters::CommissioningProxy::MyCPDelegate::GetScanMaxTime()
{
    return 30;
}

CHIP_ERROR CommissioningProxyManager::Init()
{
    ChipLogProgress(AppServer, "===SHM %s()", __func__);
    chip::app::RegisteredServerCluster<chip::app::Clusters::CommissioningProxyCluster> gDEMCluster(
        chip::app::Clusters::CommissioningProxyCluster::Config(1 /* SHM kYourEndpointId */, gFeatures, &gCommissioningProxyManager));

    mMainCommissioningProxyState = kState_CPDisconnected;
    return CHIP_NO_ERROR;
}

bool CommissioningProxyManager::IsCPConnected()
{
    ChipLogProgress(AppServer, "===SHM %s()", __func__);
    return mMainCommissioningProxyState == kState_CPConnected;
}

void CommissioningProxyManager::SetCallbacks(CommissioningProxyCallback_fn aActionInitiated_CB, CommissioningProxyCallback_fn aActionCompleted_CB)
{
    mActionInitiated_CB = aActionInitiated_CB;
    mActionCompleted_CB = aActionCompleted_CB;
}

void CommissioningProxyManager::SetCPState(uint8_t level)
{
    ChipLogProgress(AppServer, "===SHM %s()", __func__);
   mMainCommissioningProxyState = kState_CPConnected;
    ChipLogDetail(AppServer, "%s: CP Connected", __func__);
} 

bool CommissioningProxyManager::InitiateAction(Action_t aAction)
{
    ChipLogProgress(AppServer, "===SHM %s()", __func__);
    // this function is called InitiateAction because we may want to implement some features such as ramping up here.
    bool action_initiated = false;
    //State_t new_state;

    switch (aAction)
    {
    case CONNECT_ACTION:
        ChipLogProgress(AppServer, "CommissioningProxyManager::InitiateAction(CONNECT_ACTION)");
        break;
    case DISCONNECT_ACTION:
        ChipLogProgress(AppServer, "CommissioningProxyManager::InitiateAction(DISCONNECT_ACTION)");
        break;
    default:
        ChipLogProgress(AppServer, "CommissioningProxyManager::InitiateAction(unknown)");
        break;
    }

    // Initiate On/Off Action only when the previous one is complete.
    if (mMainCommissioningProxyState == kState_CPDisconnected && aAction == CONNECT_ACTION)
    {
        action_initiated = true;
        //new_state        = kState_CPConnected;
    }
    else if (mMainCommissioningProxyState == kState_CPConnected && aAction == DISCONNECT_ACTION)
    {
        action_initiated = true;
        //new_state        = kState_CPDisconnected;
    }

    if (action_initiated)
    {
        if (mActionInitiated_CB)
        {
            mActionInitiated_CB(aAction);
        }

        //new_state == kState_CPConnected;

        if (mActionCompleted_CB)
        {
            mActionCompleted_CB(aAction);
        }
    }

    return action_initiated;
}
#endif