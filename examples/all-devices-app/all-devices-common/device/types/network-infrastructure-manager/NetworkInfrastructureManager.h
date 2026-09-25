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

#pragma once

#include <app/FailSafeContext.h>
#include <app/clusters/general-commissioning-server/BreadCrumbTracker.h>
#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementCluster.h>
#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementDelegate.h>
#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsCluster.h>
#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsProvider.h>
#include <app/clusters/thread-network-directory-server/DefaultThreadNetworkDirectoryStorage.h>
#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryCluster.h>
#include <app/clusters/wifi-network-management-server/WiFiNetworkManagementCluster.h>
#include <device/api/SingleEndpoint.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace app {

class NetworkInfrastructureManager : public SingleEndpoint
{
public:
    struct Context
    {
        Clusters::ThreadBorderRouterManagementDelegate & delegate;
        FailSafeContext & failSafeContext;
        DeviceLayer::PlatformManager & platformManager;
        PersistentStorageDelegate & storage;
        Clusters::BreadCrumbTracker & breadcrumbTracker;
        Clusters::ThreadNetworkDiagnostics::ThreadNetworkDiagnosticsProvider & diagnosticsProvider;
    };

    explicit NetworkInfrastructureManager(const Context & context);
    ~NetworkInfrastructureManager() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    CHIP_ERROR SetWiFiNetworkCredentials(ByteSpan ssid, ByteSpan passphrase)
    {
        VerifyOrReturnError(mWiFiNetworkManagementCluster.IsConstructed(), CHIP_ERROR_INCORRECT_STATE);
        return mWiFiNetworkManagementCluster.Cluster().SetNetworkCredentials(ssid, passphrase);
    }

    // Public getters for programmatic control
    Clusters::ThreadBorderRouterManagementCluster & ThreadBorderRouterManagementCluster()
    {
        VerifyOrDie(mThreadBorderRouterManagementCluster.IsConstructed());
        return mThreadBorderRouterManagementCluster.Cluster();
    }
    Clusters::WiFiNetworkManagementCluster & WiFiNetworkManagementCluster()
    {
        VerifyOrDie(mWiFiNetworkManagementCluster.IsConstructed());
        return mWiFiNetworkManagementCluster.Cluster();
    }
    Clusters::ThreadNetworkDiagnosticsCluster & ThreadNetworkDiagnosticsCluster()
    {
        VerifyOrDie(mThreadNetworkDiagnosticsCluster.IsConstructed());
        return mThreadNetworkDiagnosticsCluster.Cluster();
    }
    Clusters::ThreadNetworkDirectoryCluster & ThreadNetworkDirectoryCluster()
    {
        VerifyOrDie(mThreadNetworkDirectoryCluster.IsConstructed());
        return mThreadNetworkDirectoryCluster.Cluster();
    }
    Clusters::BreadCrumbTracker & GetBreadCrumbTracker() { return mBreadCrumbTracker; }

protected:
    Clusters::ThreadBorderRouterManagementDelegate & mDelegate;
    FailSafeContext & mFailSafeContext;
    DeviceLayer::PlatformManager & mPlatformManager;
    Clusters::BreadCrumbTracker & mBreadCrumbTracker;
    Clusters::ThreadNetworkDiagnostics::ThreadNetworkDiagnosticsProvider & mDiagnosticsProvider;

    DefaultThreadNetworkDirectoryStorage mThreadNetworkDirectoryStorage;

    LazyRegisteredServerCluster<Clusters::ThreadBorderRouterManagementCluster> mThreadBorderRouterManagementCluster;
    LazyRegisteredServerCluster<Clusters::WiFiNetworkManagementCluster> mWiFiNetworkManagementCluster;
    LazyRegisteredServerCluster<Clusters::ThreadNetworkDiagnosticsCluster> mThreadNetworkDiagnosticsCluster;
    LazyRegisteredServerCluster<Clusters::ThreadNetworkDirectoryCluster> mThreadNetworkDirectoryCluster;
};

} // namespace app
} // namespace chip
