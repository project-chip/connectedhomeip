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

#include <optional>

#include <app/clusters/microwave-oven-control-server/MicrowaveOvenControlCluster.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/clusters/operational-state-server/OperationalStateCluster.h>
#include <app/clusters/operational-state-server/OperationalStateDelegate.h>
#include <clusters/MicrowaveOvenMode/Enums.h>
#include <device/api/SingleEndpoint.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

class MicrowaveOven : public SingleEndpoint
{
public:
    struct Config
    {
        Clusters::OperationalState::OperationalStateCluster::Delegate & operationalStateDelegate;
        Clusters::MicrowaveOvenControl::IntegrationDelegate & controlIntegrationDelegate;
        Clusters::MicrowaveOvenControl::AppDelegate & controlAppDelegate;
        Clusters::ModeBase::AppDelegate & modeDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
    };

    explicit MicrowaveOven(const Config & config);
    ~MicrowaveOven() override = default;

    CHIP_ERROR Register(EndpointId endpoint, CodeDrivenDataModelProvider & provider, EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::OperationalState::OperationalStateCluster & OperationalState() { return mOperationalStateCluster.Cluster(); }
    Clusters::MicrowaveOvenControlCluster & MicrowaveOvenControl() { return mMicrowaveOvenControlCluster.Cluster(); }
    Clusters::ModeBaseCluster & MicrowaveOvenMode() { return mMicrowaveOvenModeCluster.Cluster(); }

private:
    DeviceLayer::DiagnosticDataProvider & mDiagnosticDataProvider;
    Clusters::OperationalState::OperationalStateCluster::Delegate & mOperationalStateDelegate;
    LazyRegisteredServerCluster<Clusters::OperationalState::OperationalStateCluster> mOperationalStateCluster;

    Clusters::MicrowaveOvenControl::IntegrationDelegate & mControlIntegrationDelegate;
    Clusters::MicrowaveOvenControl::AppDelegate & mControlAppDelegate;
    LazyRegisteredServerCluster<Clusters::MicrowaveOvenControlCluster> mMicrowaveOvenControlCluster;

    Clusters::ModeBase::AppDelegate & mMicrowaveOvenModeDelegate;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mMicrowaveOvenModeCluster;
};

} // namespace chip::app
