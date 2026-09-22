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
#include <device/api/SingleEndpoint.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace app {

class ThreadBorderRouter : public SingleEndpoint
{
public:
    struct Context
    {
        Clusters::ThreadBorderRouterManagementDelegate & delegate;
        FailSafeContext & failSafeContext;
        DeviceLayer::PlatformManager & platformManager;
        Clusters::BreadCrumbTracker & breadcrumbTracker;
        Clusters::ThreadNetworkDiagnostics::ThreadNetworkDiagnosticsProvider & diagnosticsProvider;
    };

    explicit ThreadBorderRouter(const Context & context);
    ~ThreadBorderRouter() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // Public getters for programmatic control
    Clusters::ThreadBorderRouterManagementCluster & ThreadBorderRouterManagementCluster()
    {
        return mThreadBorderRouterManagementCluster.Cluster();
    }
    Clusters::ThreadNetworkDiagnosticsCluster & ThreadNetworkDiagnosticsCluster()
    {
        return mThreadNetworkDiagnosticsCluster.Cluster();
    }
    Clusters::BreadCrumbTracker & GetBreadCrumbTracker() { return mBreadCrumbTracker; }

protected:
    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }
    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

    Clusters::ThreadBorderRouterManagementDelegate & mDelegate;
    FailSafeContext & mFailSafeContext;
    DeviceLayer::PlatformManager & mPlatformManager;
    Clusters::BreadCrumbTracker & mBreadCrumbTracker;
    Clusters::ThreadNetworkDiagnostics::ThreadNetworkDiagnosticsProvider & mDiagnosticsProvider;

    LazyRegisteredServerCluster<Clusters::ThreadBorderRouterManagementCluster> mThreadBorderRouterManagementCluster;
    LazyRegisteredServerCluster<Clusters::ThreadNetworkDiagnosticsCluster> mThreadNetworkDiagnosticsCluster;
};

} // namespace app
} // namespace chip
