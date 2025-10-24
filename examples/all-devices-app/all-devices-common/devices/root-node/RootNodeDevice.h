/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app/clusters/access-control-server/access-control-cluster.h>
#include <app/clusters/administrator-commissioning-server/AdministratorCommissioningCluster.h>
#include <app/clusters/basic-information/BasicInformationCluster.h>
#include <app/clusters/general-commissioning-server/general-commissioning-cluster.h>
#include <app/clusters/general-diagnostics-server/general-diagnostics-cluster.h>
#include <app/clusters/group-key-mgmt-server/group-key-mgmt-cluster.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/clusters/operational-credentials-server/operational-credentials-cluster.h>
#include <app/clusters/software-diagnostics-server/software-diagnostics-cluster.h>
#include <app/clusters/wifi-network-diagnostics-server/wifi-network-diagnostics-cluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <devices/base-device/Device.h>
#include <platform/Linux/NetworkCommissioningDriver.h>

namespace chip {
namespace app {

class RootNodeDevice : public Device
{
public:
    ~RootNodeDevice() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void UnRegister(CodeDrivenDataModelProvider & provider) override;

protected:
    // Most implementations require network commissioning, so only subclasses have access to this.
    //TODO: Update the hard coded device type once #41602 is merged
    RootNodeDevice() : Device(Device::DeviceType{ .deviceType = static_cast<uint32_t>(0x0016), .revision = 3 }) {}

private:
    LazyRegisteredServerCluster<Clusters::BasicInformationCluster> mBasicInformationCluster;
    LazyRegisteredServerCluster<Clusters::GeneralCommissioningCluster> mGeneralCommissioningCluster;
    LazyRegisteredServerCluster<Clusters::AdministratorCommissioningWithBasicCommissioningWindowCluster>
        mAdministratorCommissioningCluster;
    LazyRegisteredServerCluster<Clusters::GeneralDiagnosticsCluster> mGeneralDiagnosticsCluster;
    LazyRegisteredServerCluster<Clusters::GroupKeyManagementCluster> mGroupKeyManagementCluster;
    LazyRegisteredServerCluster<Clusters::SoftwareDiagnosticsServerCluster> mSoftwareDiagnosticsServerCluster;
    LazyRegisteredServerCluster<Clusters::AccessControlCluster> mAccessControlCluster;
    LazyRegisteredServerCluster<Clusters::OperationalCredentialsCluster> mOperationalCredentialsCluster;
};

class WifiRootNodeDevice : public RootNodeDevice
{
public:
    WifiRootNodeDevice(DeviceLayer::NetworkCommissioning::WiFiDriver * wifiDriver) : RootNodeDevice(), mWifiDriver(wifiDriver) {}

    ~WifiRootNodeDevice() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void UnRegister(CodeDrivenDataModelProvider & provider) override;

private:
    LazyRegisteredServerCluster<Clusters::NetworkCommissioningCluster> mNetworkCommissioningCluster;
    LazyRegisteredServerCluster<Clusters::WiFiDiagnosticsServerCluster> mWifiDiagnosticsCluster;
    DeviceLayer::NetworkCommissioning::WiFiDriver * mWifiDriver;
};

} // namespace app
} // namespace chip
