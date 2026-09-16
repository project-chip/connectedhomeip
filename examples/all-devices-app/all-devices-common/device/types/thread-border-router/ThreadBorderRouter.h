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
#include <app/clusters/thread-border-router-management-server/ThreadBorderRouterManagementDelegate.h>
#include <app/clusters/thread-network-diagnostics-server/DirectThreadNetworkDiagnosticsProvider.h>
#include <app/clusters/thread-network-diagnostics-server/ThreadNetworkDiagnosticsCluster.h>
#include <app/clusters/thread-network-directory-server/DefaultThreadNetworkDirectoryStorage.h>
#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryCluster.h>
#include <device/api/SingleEndpoint.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TimerDelegate.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace app {

class ThreadBorderRouter : public SingleEndpoint, public Clusters::ThreadBorderRouterManagementDelegate
{
public:
    ThreadBorderRouter(TimerDelegate & timerDelegate, PersistentStorageDelegate & storage,
                       DeviceLayer::PlatformManager & platformManager, FailSafeContext & failSafeContext);
    ~ThreadBorderRouter() override;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    // ThreadBorderRouterManagementDelegate
    CHIP_ERROR Init(AttributeChangeCallback * attributeChangeCallback) override;
    bool GetPanChangeSupported() override;
    void GetBorderRouterName(MutableCharSpan & borderRouterName) override;
    CHIP_ERROR GetBorderAgentId(MutableByteSpan & borderAgentId) override;
    uint16_t GetThreadVersion() override;
    bool GetInterfaceEnabled() override;
    CHIP_ERROR GetDataset(Thread::OperationalDataset & dataset, DatasetType type) override;
    void SetActiveDataset(const Thread::OperationalDataset & activeDataset, uint32_t sequenceNum,
                          ActivateDatasetCallback * callback) override;
    CHIP_ERROR CommitActiveDataset() override;
    CHIP_ERROR RevertActiveDataset() override;
    CHIP_ERROR SetPendingDataset(const Thread::OperationalDataset & pendingDataset) override;

    Clusters::ThreadBorderRouterManagementCluster & ThreadBorderRouterManagementCluster()
    {
        return mThreadBorderRouterManagementCluster.Cluster();
    }
    Clusters::ThreadNetworkDirectoryCluster & ThreadNetworkDirectoryCluster() { return mThreadNetworkDirectoryCluster.Cluster(); }
    Clusters::ThreadNetworkDiagnosticsCluster & ThreadNetworkDiagnosticsCluster()
    {
        return mThreadNetworkDiagnosticsCluster.Cluster();
    }

protected:
    class LocalBreadCrumbTracker : public Clusters::BreadCrumbTracker
    {
    public:
        void SetBreadCrumb(uint64_t value) override { mBreadCrumb = value; }
        uint64_t GetBreadCrumb() const { return mBreadCrumb; }

    private:
        uint64_t mBreadCrumb = 0;
    };

    LocalBreadCrumbTracker mBreadCrumbTracker;
    DefaultThreadNetworkDirectoryStorage mThreadNetworkDirectoryStorage;
    Clusters::ThreadNetworkDiagnostics::DirectThreadNetworkDiagnosticsProvider mThreadDiagnosticsProvider;

    LazyRegisteredServerCluster<Clusters::ThreadBorderRouterManagementCluster> mThreadBorderRouterManagementCluster;
    LazyRegisteredServerCluster<Clusters::ThreadNetworkDirectoryCluster> mThreadNetworkDirectoryCluster;
    LazyRegisteredServerCluster<Clusters::ThreadNetworkDiagnosticsCluster> mThreadNetworkDiagnosticsCluster;

private:
    class ActiveDatasetTimerContext : public TimerContext
    {
    public:
        ActiveDatasetTimerContext(ThreadBorderRouter & router) : mRouter(router) {}
        void TimerFired() override { mRouter.OnActiveDatasetTimerFired(); }

    private:
        ThreadBorderRouter & mRouter;
    };

    class PendingDatasetTimerContext : public TimerContext
    {
    public:
        PendingDatasetTimerContext(ThreadBorderRouter & router) : mRouter(router) {}
        void TimerFired() override { mRouter.OnPendingDatasetTimerFired(); }

    private:
        ThreadBorderRouter & mRouter;
    };

    void OnActiveDatasetTimerFired();
    void OnPendingDatasetTimerFired();

    TimerDelegate & mTimerDelegate;
    DeviceLayer::PlatformManager & mPlatformManager;
    FailSafeContext & mFailSafeContext;

    ActiveDatasetTimerContext mActiveDatasetTimerContext{ *this };
    PendingDatasetTimerContext mPendingDatasetTimerContext{ *this };

    AttributeChangeCallback * mAttributeChangeCallback = nullptr;
    Thread::OperationalDataset mActiveDataset;
    Thread::OperationalDataset mPendingDataset;

    ActivateDatasetCallback * mActivateDatasetCallback = nullptr;
    uint32_t mActivateDatasetSequence;
};

} // namespace app
} // namespace chip
