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

#include <device/types/network-infrastructure-manager/NetworkInfrastructureManager.h>
#include <lib/support/TimerDelegate.h>

namespace chip {
namespace app {

class SimulatedNetworkInfrastructureManager : public NetworkInfrastructureManager,
                                              public Clusters::ThreadBorderRouterManagementDelegate
{
public:
    struct Context
    {
        TimerDelegate & timerDelegate;
        PersistentStorageDelegate & storage;
        DeviceLayer::PlatformManager & platformManager;
        FailSafeContext & failSafeContext;
        Clusters::BreadCrumbTracker * breadcrumbTracker = nullptr;
    };

    SimulatedNetworkInfrastructureManager(TimerDelegate & timerDelegate, PersistentStorageDelegate & storage,
                                          DeviceLayer::PlatformManager & platformManager, FailSafeContext & failSafeContext,
                                          Clusters::BreadCrumbTracker * breadcrumbTracker = nullptr);
    explicit SimulatedNetworkInfrastructureManager(const Context & context);
    ~SimulatedNetworkInfrastructureManager() override;

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

private:
    class ActiveDatasetTimerContext : public TimerContext
    {
    public:
        ActiveDatasetTimerContext(SimulatedNetworkInfrastructureManager & manager) : mManager(manager) {}
        void TimerFired() override { mManager.OnActiveDatasetTimerFired(); }

    private:
        SimulatedNetworkInfrastructureManager & mManager;
    };

    class PendingDatasetTimerContext : public TimerContext
    {
    public:
        PendingDatasetTimerContext(SimulatedNetworkInfrastructureManager & manager) : mManager(manager) {}
        void TimerFired() override { mManager.OnPendingDatasetTimerFired(); }

    private:
        SimulatedNetworkInfrastructureManager & mManager;
    };

    void OnActiveDatasetTimerFired();
    void OnPendingDatasetTimerFired();

    TimerDelegate & mTimerDelegate;

    ActiveDatasetTimerContext mActiveDatasetTimerContext{ *this };
    PendingDatasetTimerContext mPendingDatasetTimerContext{ *this };

    AttributeChangeCallback * mAttributeChangeCallback = nullptr;
    Thread::OperationalDataset mActiveDataset;
    Thread::OperationalDataset mStagedActiveDataset;
    Thread::OperationalDataset mPendingDataset;

    ActivateDatasetCallback * mActivateDatasetCallback = nullptr;
    uint32_t mActivateDatasetSequence                  = 0;
};

} // namespace app
} // namespace chip
