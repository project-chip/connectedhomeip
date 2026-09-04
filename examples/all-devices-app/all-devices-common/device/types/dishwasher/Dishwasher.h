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

#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/clusters/operational-state-server/OperationalStateCluster.h>
#include <app/clusters/operational-state-server/OperationalStateDelegate.h>
#include <clusters/DishwasherMode/Enums.h>
#include <device/api/SingleEndpoint.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

class Dishwasher : public SingleEndpoint
{
public:
    struct Config
    {
        Clusters::OperationalState::OperationalStateCluster::Delegate & operationalStateDelegate;
        Clusters::ModeBase::AppDelegate & modeDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit Dishwasher(const Config & config);
    ~Dishwasher() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::OperationalState::OperationalStateCluster & OperationalState() { return mOperationalStateCluster.Cluster(); }
    Clusters::ModeBaseCluster & DishwasherMode() { return mDishwasherModeCluster.Cluster(); }

private:
    DeviceLayer::DiagnosticDataProvider & mDiagnosticDataProvider;
    Clusters::OperationalState::OperationalStateCluster::Delegate & mOperationalStateDelegate;
    LazyRegisteredServerCluster<Clusters::OperationalState::OperationalStateCluster> mOperationalStateCluster;

    Clusters::ModeBase::AppDelegate & mDishwasherModeDelegate;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mDishwasherModeCluster;
};

} // namespace chip::app
