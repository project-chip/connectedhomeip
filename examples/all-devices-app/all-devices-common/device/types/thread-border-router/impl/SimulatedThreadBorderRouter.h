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

#include <app/clusters/thread-network-diagnostics-server/DirectThreadNetworkDiagnosticsProvider.h>
#include <app/clusters/thread-network-directory-server/DefaultThreadNetworkDirectoryStorage.h>
#include <app/clusters/thread-network-directory-server/ThreadNetworkDirectoryCluster.h>
#include <device/capabilities/breadcrumb/SimpleBreadCrumbTracker.h>
#include <device/types/thread-border-router/ThreadBorderRouter.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TimerDelegate.h>

#include <string>
#include <variant>

namespace chip {
namespace app {

/**
 * Concrete simulated implementation of ThreadBorderRouter.
 *
 * Inherits from ThreadBorderRouterManagementDelegate, SimpleBreadCrumbTracker, and
 * DirectThreadNetworkDiagnosticsProvider before ThreadBorderRouter so that these base
 * subobjects are fully constructed before ThreadBorderRouter's constructor receives
 * references to them.
 *
 * Note on BreadCrumbTracker:
 * See SimpleBreadCrumbTracker.h for details on Matter Core Spec 14.3.6.4.2. A future refactor
 * should route breadcrumb updates to the root node's GeneralCommissioningCluster.
 */
class SimulatedThreadBorderRouter : public Clusters::ThreadBorderRouterManagementDelegate,
                                    public SimpleBreadCrumbTracker,
                                    public Clusters::ThreadNetworkDiagnostics::DirectThreadNetworkDiagnosticsProvider,
                                    public ThreadBorderRouter
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
        PersistentStorageDelegate & storage;
        DeviceLayer::PlatformManager & platformManager;
        FailSafeContext & failSafeContext;
        std::string nodeLabel;
    };

    explicit SimulatedThreadBorderRouter(const Context & context);
    ~SimulatedThreadBorderRouter() override;

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

    // Access to optional Thread Network Directory cluster
    Clusters::ThreadNetworkDirectoryCluster & ThreadNetworkDirectoryCluster()
    {
        VerifyOrDie(mThreadNetworkDirectoryCluster.IsConstructed());
        return mThreadNetworkDirectoryCluster.Cluster();
    }

protected:
    CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    class ActiveDatasetTimerContext : public TimerContext
    {
    public:
        ActiveDatasetTimerContext(SimulatedThreadBorderRouter & router) : mRouter(router) {}
        void TimerFired() override { mRouter.OnActiveDatasetTimerFired(); }

    private:
        SimulatedThreadBorderRouter & mRouter;
    };

    class PendingDatasetTimerContext : public TimerContext
    {
    public:
        PendingDatasetTimerContext(SimulatedThreadBorderRouter & router) : mRouter(router) {}
        void TimerFired() override { mRouter.OnPendingDatasetTimerFired(); }

    private:
        SimulatedThreadBorderRouter & mRouter;
    };

    void OnActiveDatasetTimerFired();
    void OnPendingDatasetTimerFired();

    // State of the active dataset.
    //
    //   event                  from                           to
    //   ---------------------  -----------------------------  -----------------
    //   SetActiveDataset       NoActiveDataset                Activating
    //   active timer fires     Activating                     ActiveUncommitted
    //   CommitActiveDataset    ActiveUncommitted              ActiveCommitted
    //   RevertActiveDataset    Activating, ActiveUncommitted  NoActiveDataset
    //   pending timer fires    any                            ActiveCommitted
    //   Unregister             any                            NoActiveDataset
    struct NoActiveDataset
    {
    };
    struct Activating
    {
        Thread::OperationalDataset dataset;
        ActivateDatasetCallback * callback;
        uint32_t sequence;
    };
    struct ActiveUncommitted
    {
        Thread::OperationalDataset dataset;
    };
    struct ActiveCommitted
    {
        Thread::OperationalDataset dataset;
    };

    // Returns the dataset visible to GetDataset(kActive), or nullptr when there is none.
    const Thread::OperationalDataset * ActiveDataset() const;
    // Leaves the Activating state (if in it) and completes its callback with `error`.
    void CompleteActivation(CHIP_ERROR error);
    void ReportAttributeChange(AttributeId attributeId);

    TimerDelegate & mTimerDelegate;
    DefaultThreadNetworkDirectoryStorage mThreadNetworkDirectoryStorage;
    LazyRegisteredServerCluster<Clusters::ThreadNetworkDirectoryCluster> mThreadNetworkDirectoryCluster;
    std::string mBorderRouterName;

    ActiveDatasetTimerContext mActiveDatasetTimerContext{ *this };
    PendingDatasetTimerContext mPendingDatasetTimerContext{ *this };

    AttributeChangeCallback * mAttributeChangeCallback = nullptr;
    std::variant<NoActiveDataset, Activating, ActiveUncommitted, ActiveCommitted> mActive;
    Thread::OperationalDataset mPendingDataset; // empty when there is no pending dataset
};

} // namespace app
} // namespace chip
