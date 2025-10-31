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
#include <devices/root-node/RootNodeDevice.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::DeviceLayer;

namespace chip {
namespace app {

CHIP_ERROR RootNodeDevice::Register(EndpointId endpointId, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpointId, provider, parentId));

    mBasicInformationCluster.Create();

    // TODO: This needs to be refactored so the optional attributes being set for
    //  the cluster are configurable to allow different settings
    mBasicInformationCluster.Cluster()
        .OptionalAttributes()
        .Set<BasicInformation::Attributes::ManufacturingDate::Id>()
        .Set<BasicInformation::Attributes::PartNumber::Id>()
        .Set<BasicInformation::Attributes::ProductURL::Id>()
        .Set<BasicInformation::Attributes::ProductLabel::Id>()
        .Set<BasicInformation::Attributes::SerialNumber::Id>()
        .Set<BasicInformation::Attributes::LocalConfigDisabled::Id>()
        .Set<BasicInformation::Attributes::Reachable::Id>()
        .Set<BasicInformation::Attributes::ProductAppearance::Id>();

    ReturnErrorOnFailure(provider.AddCluster(mBasicInformationCluster.Registration()));

    mGeneralCommissioningCluster.Create();
    ReturnErrorOnFailure(provider.AddCluster(mGeneralCommissioningCluster.Registration()));

    mAdministratorCommissioningCluster.Create(endpointId, BitFlags<AdministratorCommissioning::Feature>{});
    ReturnErrorOnFailure(provider.AddCluster(mAdministratorCommissioningCluster.Registration()));

    mGeneralDiagnosticsCluster.Create(GeneralDiagnosticsCluster::OptionalAttributeSet{});
    ReturnErrorOnFailure(provider.AddCluster(mGeneralDiagnosticsCluster.Registration()));

    mGroupKeyManagementCluster.Create();
    ReturnErrorOnFailure(provider.AddCluster(mGroupKeyManagementCluster.Registration()));

    mSoftwareDiagnosticsServerCluster.Create(SoftwareDiagnosticsLogic::OptionalAttributeSet{});
    ReturnErrorOnFailure(provider.AddCluster(mSoftwareDiagnosticsServerCluster.Registration()));

    mAccessControlCluster.Create();
    ReturnErrorOnFailure(provider.AddCluster(mAccessControlCluster.Registration()));

    mOperationalCredentialsCluster.Create(
        endpointId,
        OperationalCredentialsCluster::Context{ .fabricTable     = Server::GetInstance().GetFabricTable(),
                                                .failSafeContext = Server::GetInstance().GetFailSafeContext(),
                                                .sessionManager  = Server::GetInstance().GetSecureSessionManager(),
                                                .dnssdServer     = app::DnssdServer::Instance(),
                                                .commissioningWindowManager =
                                                    Server::GetInstance().GetCommissioningWindowManager() });
    ReturnErrorOnFailure(provider.AddCluster(mOperationalCredentialsCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void RootNodeDevice::UnRegister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);
    if (mBasicInformationCluster.IsConstructed())
    {
        provider.RemoveCluster(&mBasicInformationCluster.Cluster());
        mBasicInformationCluster.Destroy();
    }
    if (mGeneralCommissioningCluster.IsConstructed())
    {
        provider.RemoveCluster(&mGeneralCommissioningCluster.Cluster());
        mGeneralCommissioningCluster.Destroy();
    }
    if (mAdministratorCommissioningCluster.IsConstructed())
    {
        provider.RemoveCluster(&mAdministratorCommissioningCluster.Cluster());
        mAdministratorCommissioningCluster.Destroy();
    }
    if (mGeneralDiagnosticsCluster.IsConstructed())
    {
        provider.RemoveCluster(&mGeneralDiagnosticsCluster.Cluster());
        mGeneralDiagnosticsCluster.Destroy();
    }
    if (mGroupKeyManagementCluster.IsConstructed())
    {
        provider.RemoveCluster(&mGroupKeyManagementCluster.Cluster());
        mGroupKeyManagementCluster.Destroy();
    }
    if (mSoftwareDiagnosticsServerCluster.IsConstructed())
    {
        provider.RemoveCluster(&mSoftwareDiagnosticsServerCluster.Cluster());
        mSoftwareDiagnosticsServerCluster.Destroy();
    }
    if (mAccessControlCluster.IsConstructed())
    {
        provider.RemoveCluster(&mAccessControlCluster.Cluster());
        mAccessControlCluster.Destroy();
    }
    if (mOperationalCredentialsCluster.IsConstructed())
    {
        provider.RemoveCluster(&mOperationalCredentialsCluster.Cluster());
        mOperationalCredentialsCluster.Destroy();
    }
}

CHIP_ERROR WifiRootNodeDevice::Register(EndpointId endpointId, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(RootNodeDevice::Register(endpointId, provider, parentId));

    mWifiDiagnosticsCluster.Create(endpointId, DeviceLayer::GetDiagnosticDataProvider(),
                                   WiFiDiagnosticsServerCluster::OptionalAttributeSet{},
                                   BitFlags<WiFiNetworkDiagnostics::Feature>{});
    ReturnErrorOnFailure(provider.AddCluster(mWifiDiagnosticsCluster.Registration()));

    mNetworkCommissioningCluster.Create(endpointId, mWifiDriver);
    ReturnErrorOnFailure(provider.AddCluster(mNetworkCommissioningCluster.Registration()));

    return CHIP_NO_ERROR;
}

void WifiRootNodeDevice::UnRegister(CodeDrivenDataModelProvider & provider)
{
    RootNodeDevice::UnRegister(provider);
    if (mNetworkCommissioningCluster.IsConstructed())
    {
        provider.RemoveCluster(&mNetworkCommissioningCluster.Cluster());
        mNetworkCommissioningCluster.Destroy();
    }
    if (mWifiDiagnosticsCluster.IsConstructed())
    {
        provider.RemoveCluster(&mWifiDiagnosticsCluster.Cluster());
        mWifiDiagnosticsCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
