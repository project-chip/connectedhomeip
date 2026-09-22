/*
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <device/types/network-infrastructure-manager/NetworkInfrastructureManager.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <device/api/Interface.h>
#include <devices/Types.h>
#include <lib/support/Span.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

NetworkInfrastructureManager::NetworkInfrastructureManager(const Context & context) :
    SingleEndpoint(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kNetworkInfrastructureManager, 1)),
    mDelegate(context.delegate), mFailSafeContext(context.failSafeContext), mPlatformManager(context.platformManager),
    mBreadCrumbTracker(context.breadcrumbTracker != nullptr ? *context.breadcrumbTracker : mDefaultBreadCrumbTracker),
    mThreadNetworkDirectoryStorage(context.storage)
{}

CHIP_ERROR NetworkInfrastructureManager::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                                                  EndpointComposition composition)
{
    VerifyOrReturnError(mEndpointId == kInvalidEndpointId, CHIP_ERROR_INCORRECT_STATE);
    DeviceRegistrationTransaction transaction(*this, provider);

    ReturnErrorOnFailure(RegisterDescriptor(endpoint, provider, composition));

    // 1. Thread Border Router Management
    ThreadBorderRouterManagementCluster::Config tbrConfig(mDelegate, mFailSafeContext, mBreadCrumbTracker, mPlatformManager);
    mThreadBorderRouterManagementCluster.Create(endpoint, tbrConfig);
    ReturnErrorOnFailure(provider.AddCluster(mThreadBorderRouterManagementCluster.Registration()));

    // 2. WiFi Network Management
    mWiFiNetworkManagementCluster.Create(endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mWiFiNetworkManagementCluster.Registration()));
    ReturnErrorOnFailure(mWiFiNetworkManagementCluster.Cluster().SetNetworkCredentials(
        ByteSpan::fromCharSpan("MatterAP"_span), ByteSpan::fromCharSpan("Setec Astronomy"_span)));

    // 3. Thread Network Directory
    mThreadNetworkDirectoryCluster.Create(endpoint, mThreadNetworkDirectoryStorage);
    ReturnErrorOnFailure(provider.AddCluster(mThreadNetworkDirectoryCluster.Registration()));

    // 4. Thread Network Diagnostics
    mThreadNetworkDiagnosticsCluster.Create(endpoint, ThreadNetworkDiagnosticsCluster::ClusterType::kFull,
                                            mThreadDiagnosticsProvider);
    ReturnErrorOnFailure(provider.AddCluster(mThreadNetworkDiagnosticsCluster.Registration()));

    ReturnErrorOnFailure(provider.AddEndpoint(mEndpointRegistration));
    transaction.Commit();
    return CHIP_NO_ERROR;
}

void NetworkInfrastructureManager::Unregister(CodeDrivenDataModelProvider & provider)
{
    UnregisterDescriptor(provider);

    if (mThreadNetworkDiagnosticsCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThreadNetworkDiagnosticsCluster.Cluster()));
        mThreadNetworkDiagnosticsCluster.Destroy();
    }
    if (mThreadNetworkDirectoryCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThreadNetworkDirectoryCluster.Cluster()));
        mThreadNetworkDirectoryCluster.Destroy();
    }
    if (mWiFiNetworkManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mWiFiNetworkManagementCluster.Cluster()));
        mWiFiNetworkManagementCluster.Destroy();
    }
    if (mThreadBorderRouterManagementCluster.IsConstructed())
    {
        LogErrorOnFailure(provider.RemoveCluster(&mThreadBorderRouterManagementCluster.Cluster()));
        mThreadBorderRouterManagementCluster.Destroy();
    }
}

} // namespace app
} // namespace chip
