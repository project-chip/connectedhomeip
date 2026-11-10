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

#include <app/clusters/bindings/BindingCluster.h>
#include <app/clusters/bindings/BindingManager.h>
#include <app/clusters/bindings/binding-table.h>
#include <binding-handler/BindingHandler.h>
#include <clusters/ClosureControl/Enums.h>
#include <clusters/shared/Enums.h>
#include <device/api/SingleEndpoint.h>
#include <platform/PlatformManager.h>

#include <optional>

namespace chip::app {

/// Closure Controller device type: sends Closure Control commands to bound closures.
class ClosureController : public SingleEndpoint
{
public:
    struct Config
    {
        Clusters::Binding::Table & bindingTable;
        Clusters::Binding::Manager & bindingManager;
        DeviceLayer::PlatformManager & platformManager;
        BindingHandler & bindingHandler;
    };

    /// Fields of a MoveTo / GroupedMoveTo command. Each field is optional in the command;
    /// only set the fields that the target closure supports.
    struct MoveToArgs
    {
        std::optional<Clusters::ClosureControl::TargetPositionEnum> position;
        std::optional<bool> latch;
        std::optional<Clusters::Globals::ThreeLevelAutoEnum> speed;
    };

    explicit ClosureController(const Config & config);
    ~ClosureController() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    CHIP_ERROR ClientClusters(ReadOnlyBufferBuilder<ClusterId> & out) const override;

    CHIP_ERROR SendMoveTo(const MoveToArgs & args);
    CHIP_ERROR SendGroupedMoveTo(const MoveToArgs & args);
    CHIP_ERROR SendStop();
    CHIP_ERROR SendCalibrate();

    Clusters::BindingCluster & BindingCluster()
    {
        VerifyOrDie(mBindingCluster.IsConstructed());
        return mBindingCluster.Cluster();
    }

private:
    Config mConfig;
    LazyRegisteredServerCluster<Clusters::BindingCluster> mBindingCluster;
};

} // namespace chip::app
