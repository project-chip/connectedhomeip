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

#include <app/clusters/general-commissioning-server/BreadCrumbTracker.h>
#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementCluster.h>
#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsCluster.h>
#include <app/clusters/thread-network-directory-server/DefaultThreadNetworkDirectoryStorage.h>
#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryCluster.h>
#include <app/clusters/wifi-network-management-server/WiFiNetworkManagementCluster.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <devices/nim/FakeBorderRouterDelegate.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>

namespace chip {
namespace app {

class SimpleBreadCrumbTracker : public Clusters::BreadCrumbTracker
{
public:
    void SetBreadCrumb(uint64_t value) override { mBreadCrumb = value; }
    uint64_t GetBreadCrumb() const { return mBreadCrumb; }

private:
    uint64_t mBreadCrumb = 0;
};

class NimDevice : public SingleEndpointDevice
{
public:
    NimDevice(PersistentStorageDelegate & storage);
    ~NimDevice() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointId parentId = kInvalidEndpointId) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

protected:
    FakeBorderRouterDelegate mBorderRouterDelegate;
    SimpleBreadCrumbTracker mBreadCrumbTracker;
    DefaultThreadNetworkDirectoryStorage mThreadNetworkDirectoryStorage;

    LazyRegisteredServerCluster<Clusters::ThreadBorderRouterManagementCluster> mThreadBorderRouterManagementCluster;
    LazyRegisteredServerCluster<Clusters::WiFiNetworkManagementCluster> mWiFiNetworkManagementCluster;
    LazyRegisteredServerCluster<Clusters::ThreadNetworkDirectoryCluster> mThreadNetworkDirectoryCluster;
    LazyRegisteredServerCluster<Clusters::ThreadNetworkDiagnosticsCluster> mThreadNetworkDiagnosticsCluster;
};

} // namespace app
} // namespace chip
