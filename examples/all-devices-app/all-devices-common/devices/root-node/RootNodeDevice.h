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
#include <app/clusters/general-commissioning-server/BreadCrumbTracker.h>
#include <app/clusters/general-commissioning-server/GeneralCommissioningCluster.h>
#include <app/clusters/general-diagnostics-server/GeneralDiagnosticsCluster.h>
#include <app/clusters/group-key-mgmt-server/GroupKeyManagementCluster.h>
#include <app/clusters/network-commissioning/NetworkCommissioningCluster.h>
#include <app/clusters/operational-credentials-server/OperationalCredentialsCluster.h>
#include <app/clusters/software-diagnostics-server/SoftwareDiagnosticsCluster.h>
#include <app/clusters/wifi-network-diagnostics-server/WiFiNetworkDiagnosticsCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <credentials/GroupDataProvider.h>
#include <devices/Types.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <platform/NetworkCommissioning.h>

namespace chip {
namespace app {

class RootNodeDevice : public SingleEndpointDevice
{
public:
    struct Context
    {
        CommissioningWindowManager & commissioningWindowManager;
        DeviceLayer::ConfigurationManager & configurationManager;
        DeviceLayer::DeviceControlServer & deviceControlServer;
        FabricTable & fabricTable;
        FailSafeContext & failsafeContext;
        DeviceLayer::PlatformManager & platformManager;
        Credentials::GroupDataProvider & groupDataProvider;
        SessionManager & sessionManager;
        DnssdServer & dnssdServer;

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        TermsAndConditionsProvider & termsAndConditionsProvider;
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    };

    ~RootNodeDevice() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void UnRegister(CodeDrivenDataModelProvider & provider) override;

protected:
    // Most implementations require network commissioning, so only subclasses have access to this.
    RootNodeDevice(const Context & context) :
        SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kRootNode, 1)), mContext(context)
    {}
    LazyRegisteredServerCluster<Clusters::GeneralCommissioningCluster> mGeneralCommissioningCluster;

private:
    Context mContext;

    LazyRegisteredServerCluster<Clusters::BasicInformationCluster> mBasicInformationCluster;
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
    struct WifiContext
    {
        DeviceLayer::NetworkCommissioning::WiFiDriver & wifiDriver;
    };

    WifiRootNodeDevice(const Context & context, const WifiContext & wifiContext) :
        RootNodeDevice(context), mWifiContext(wifiContext)
    {}

    ~WifiRootNodeDevice() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void UnRegister(CodeDrivenDataModelProvider & provider) override;

private:
    WifiContext mWifiContext;

    LazyRegisteredServerCluster<Clusters::NetworkCommissioningCluster> mNetworkCommissioningCluster;
    LazyRegisteredServerCluster<Clusters::WiFiDiagnosticsServerCluster> mWifiDiagnosticsCluster;
};

} // namespace app
} // namespace chip
