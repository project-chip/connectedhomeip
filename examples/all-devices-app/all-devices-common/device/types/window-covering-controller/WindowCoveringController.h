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
#include <binding-handler/BindingCommand.h>
#include <binding-handler/BindingHandler.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TimerDelegate.h>
#include <platform/PlatformManager.h>

namespace chip::app {

class WindowCoveringController : public SingleEndpoint
{
public:
    struct Config
    {
        Clusters::Binding::Table & bindingTable;
        Clusters::Binding::Manager & bindingManager;
        DeviceLayer::PlatformManager & platformManager;
        BindingHandler & bindingHandler;
        TimerDelegate & timerDelegate;
        bool withIdentifyServer = false;
        bool withIdentifyClient = false;
        bool withGroupsClient   = false;
    };

    explicit WindowCoveringController(const Config & config);
    ~WindowCoveringController() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

    /// Sends `command` to every binding on this endpoint for command->GetClusterId().
    ///
    /// Returns CHIP_ERROR_INCORRECT_STATE if the device is not registered, and
    /// CHIP_ERROR_INVALID_ARGUMENT if `command` is null or its cluster is not an enabled client cluster.
    CHIP_ERROR Dispatch(Platform::UniquePtr<BindingCommand> command);

    Clusters::BindingCluster & BindingCluster()
    {
        VerifyOrDie(mBindingCluster.IsConstructed());
        return mBindingCluster.Cluster();
    }

    /// Only available when Config::withIdentifyServer is set.
    Clusters::IdentifyCluster & IdentifyCluster()
    {
        VerifyOrDie(mIdentifyCluster.IsConstructed());
        return mIdentifyCluster.Cluster();
    }

private:
    bool IsEnabledClientCluster(ClusterId clusterId) const;

    Config mConfig;
    LazyRegisteredServerCluster<Clusters::BindingCluster> mBindingCluster;
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
};

} // namespace chip::app
