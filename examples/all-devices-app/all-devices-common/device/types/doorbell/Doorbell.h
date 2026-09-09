/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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

#include <app/clusters/bindings/BindingCluster.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/switch-server/SwitchCluster.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/Span.h>
#include <lib/support/TimerDelegate.h>
#include <platform/PlatformManager.h>

namespace chip {
namespace app {

class Doorbell : public SingleEndpoint
{
public:
    Doorbell(TimerDelegate & timerDelegate, DeviceLayer::PlatformManager & platformManager, Clusters::Binding::Table & bindingTable,
             Clusters::Binding::Manager & bindingManager);
    ~Doorbell() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

    Clusters::IdentifyCluster & IdentifyCluster();
    Clusters::SwitchCluster & SwitchCluster();
    Clusters::BindingCluster & BindingCluster();

protected:
    TimerDelegate & mTimerDelegate;
    DeviceLayer::PlatformManager & mPlatformManager;
    Clusters::Binding::Table & mBindingTable;
    Clusters::Binding::Manager & mBindingManager;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::SwitchCluster> mSwitchCluster;
    LazyRegisteredServerCluster<Clusters::BindingCluster> mBindingCluster;
};

} // namespace app
} // namespace chip
