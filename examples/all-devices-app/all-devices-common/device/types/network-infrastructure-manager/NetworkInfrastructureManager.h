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
#include <app/clusters/thread-network-diagnostics-server/DirectThreadNetworkDiagnosticsProvider.h>
#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsCluster.h>
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
    class LocalBreadCrumbTracker : public Clusters::BreadCrumbTracker
    {
    public:
        void SetBreadCrumb(uint64_t value) override { mBreadCrumb = value; }
        uint64_t GetBreadCrumb() const { return mBreadCrumb; }

    private:
        uint64_t mBreadCrumb = 0;
    };

    struct Context
    {
        Clusters::ThreadBorderRouterManagementDelegate & delegate;
        FailSafeContext & failSafeContext;
        DeviceLayer::PlatformManager & platformManager;
        PersistentStorageDelegate & storage;
        Clusters::BreadCrumbTracker * breadcrumbTracker = nullptr;
    };

    explicit NetworkInfrastructureManager(const Context & context);
    ~NetworkInfrastructureManager() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public getters for programmatic control
    Clusters::ThreadBorderRouterManagementCluster & ThreadBorderRouterManagementCluster()
    {
        return mThreadBorderRouterManagementCluster.Cluster();
    }
    Clusters::WiFiNetworkManagementCluster & WiFiNetworkManagementCluster() { return mWiFiNetworkManagementCluster.Cluster(); }
    Clusters::ThreadNetworkDirectoryCluster & ThreadNetworkDirectoryCluster() { return mThreadNetworkDirectoryCluster.Cluster(); }
    Clusters::ThreadNetworkDiagnosticsCluster & ThreadNetworkDiagnosticsCluster()
    {
        return mThreadNetworkDiagnosticsCluster.Cluster();
    }
    Clusters::BreadCrumbTracker & GetBreadCrumbTracker() { return mBreadCrumbTracker; }

protected:
    Clusters::ThreadBorderRouterManagementDelegate & mDelegate;
    FailSafeContext & mFailSafeContext;
    DeviceLayer::PlatformManager & mPlatformManager;

    LocalBreadCrumbTracker mDefaultBreadCrumbTracker;
    Clusters::BreadCrumbTracker & mBreadCrumbTracker;

    DefaultThreadNetworkDirectoryStorage mThreadNetworkDirectoryStorage;
    Clusters::ThreadNetworkDiagnostics::DirectThreadNetworkDiagnosticsProvider mThreadDiagnosticsProvider;

    LazyRegisteredServerCluster<Clusters::ThreadBorderRouterManagementCluster> mThreadBorderRouterManagementCluster;
    LazyRegisteredServerCluster<Clusters::WiFiNetworkManagementCluster> mWiFiNetworkManagementCluster;
    LazyRegisteredServerCluster<Clusters::ThreadNetworkDirectoryCluster> mThreadNetworkDirectoryCluster;
    LazyRegisteredServerCluster<Clusters::ThreadNetworkDiagnosticsCluster> mThreadNetworkDiagnosticsCluster;
};

} // namespace app
} // namespace chip
