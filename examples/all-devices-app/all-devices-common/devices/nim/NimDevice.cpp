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

#include <app/server/Server.h>
#include <devices/Types.h>
#include <devices/nim/NimDevice.h>
#include <lib/support/Span.h>
#include <memory>
#include <platform/PlatformManager.h>

using namespace chip::app::Clusters;

namespace chip {
namespace app {

inline ByteSpan ByteSpanFromCharSpan(CharSpan span)
{
    return ByteSpan(Uint8::from_const_char(span.data()), span.size());
}

NimDevice::NimDevice(PersistentStorageDelegate & storage,
                     std::unique_ptr<Clusters::ThreadBorderRouterManagementDelegate> tbrDelegate) :
    SingleEndpointDevice(Span<const DataModel::DeviceTypeEntry>(&Device::Type::kNetworkInfrastructureManager, 1)),
    mTbrDelegate(std::move(tbrDelegate)), mThreadNetworkDirectoryStorage(storage)
{}

CHIP_ERROR NimDevice::Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointId parentId)
{
    ReturnErrorOnFailure(SingleEndpointRegistration(endpoint, provider, parentId));

    // 1. Thread Border Router Management
    VerifyOrReturnError(mTbrDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ThreadBorderRouterManagementCluster::Config tbrConfig(*mTbrDelegate, Server::GetInstance().GetFailSafeContext(),
                                                          mBreadCrumbTracker, DeviceLayer::PlatformMgr());
    mThreadBorderRouterManagementCluster.Create(endpoint, tbrConfig);
    ReturnErrorOnFailure(provider.AddCluster(mThreadBorderRouterManagementCluster.Registration()));

    // 2. Wi-Fi Network Management
    mWiFiNetworkManagementCluster.Create(endpoint);
    ReturnErrorOnFailure(provider.AddCluster(mWiFiNetworkManagementCluster.Registration()));

    // Set default credentials to match network-manager-app behavior
    ReturnErrorOnFailure(mWiFiNetworkManagementCluster.Cluster().SetNetworkCredentials(
        ByteSpanFromCharSpan("MatterAP"_span), ByteSpanFromCharSpan("Setec Astronomy"_span)));

    // 3. Thread Network Directory
    mThreadNetworkDirectoryCluster.Create(endpoint, mThreadNetworkDirectoryStorage);
    ReturnErrorOnFailure(provider.AddCluster(mThreadNetworkDirectoryCluster.Registration()));

    // 4. Thread Network Diagnostics
    mThreadNetworkDiagnosticsCluster.Create(endpoint, ThreadNetworkDiagnosticsCluster::ClusterType::kFull);
    ReturnErrorOnFailure(provider.AddCluster(mThreadNetworkDiagnosticsCluster.Registration()));

    return provider.AddEndpoint(mEndpointRegistration);
}

void NimDevice::Unregister(CodeDrivenDataModelProvider & provider)
{
    SingleEndpointUnregistration(provider);

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
