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

#include <app/clusters/energy-evse-server/EnergyEvseCluster.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <data-model-providers/codedriven/CodeDrivenDataModelProvider.h>
#include <device/api/SingleEndpoint.h>
#include <lib/support/TimerDelegate.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

class EnergyEvse : public SingleEndpoint
{
public:
    struct Config
    {
        TimerDelegate & timerDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
        Clusters::EnergyEvse::Delegate & evseDelegate;
        Clusters::ModeBase::AppDelegate & evseModeDelegate;
        BitMask<Clusters::EnergyEvse::Feature> evseFeatures =
            BitMask<Clusters::EnergyEvse::Feature>(Clusters::EnergyEvse::Feature::kChargingPreferences);
        Clusters::EnergyEvse::EnergyEvseCluster::OptionalCommandSet evseOptionalCmds;
        BitMask<Clusters::EnergyEvse::OptionalAttributes> evseOptionalAttrs;
    };

    explicit EnergyEvse(const Config & config);
    ~EnergyEvse() override = default;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::EnergyEvse::EnergyEvseCluster & EnergyEvseCluster();
    Clusters::ModeBaseCluster & EnergyEvseModeCluster();

protected:
    virtual CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider)
    {
        return CHIP_NO_ERROR;
    }
    virtual void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) {}

    TimerDelegate & GetTimerDelegate() const { return mConfig.timerDelegate; }

private:
    const Config mConfig;

    LazyRegisteredServerCluster<Clusters::EnergyEvse::EnergyEvseCluster> mEvseCluster;
    LazyRegisteredServerCluster<Clusters::ModeBaseCluster> mEvseModeCluster;
};

} // namespace chip::app
