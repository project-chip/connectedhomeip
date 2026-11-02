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

#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <device/types/energy-evse/EnergyEvse.h>
#include <device/types/energy-evse/impl/simulation/AllDevicesEnergyEvseDelegate.h>
#include <device/types/energy-evse/impl/simulation/AllDevicesEvseTargetsStore.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TimerDelegate.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

class SimulatedEnergyEvse : public EnergyEvse,
                            public Clusters::EnergyEvse::AllDevicesEnergyEvseDelegate,
                            private Clusters::ModeBase::AppDelegate,
                            private Clusters::IdentifyDelegate,
                            private TestEventTriggerHandler
{
public:
    using EnergyEvse::GetEndpointId;

    struct Context
    {
        TimerDelegate & timerDelegate;
        TestEventTriggerDelegate & testEventTriggerDelegate;
        PersistentStorageDelegate & persistentStorageDelegate;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider = DeviceLayer::GetDiagnosticDataProvider();
    };

    explicit SimulatedEnergyEvse(const Context & context);
    ~SimulatedEnergyEvse() override;

    CHIP_ERROR Register(chip::EndpointId endpoint, CodeDrivenDataModelProvider & provider,
                        EndpointComposition composition = {}) override;
    void Unregister(CodeDrivenDataModelProvider & provider) override;

    Clusters::IdentifyCluster & IdentifyCluster() { return mIdentifyCluster.Cluster(); }

protected:
    CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    // Clusters::ModeBase::AppDelegate
    CHIP_ERROR Init() override;
    CHIP_ERROR GetModeLabelByIndex(uint8_t modeIndex, MutableCharSpan & label) override;
    CHIP_ERROR GetModeValueByIndex(uint8_t modeIndex, uint8_t & value) override;
    CHIP_ERROR GetModeTagsByIndex(uint8_t modeIndex,
                                  DataModel::List<Clusters::detail::Structs::ModeTagStruct::Type> & tags) override;
    void HandleChangeToMode(uint8_t newMode, Clusters::ModeBase::Commands::ChangeToModeResponse::Type & response) override;

    // Clusters::IdentifyDelegate
    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;
    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;
    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;
    bool IsTriggerEffectEnabled() const override { return true; }

    // TestEventTriggerHandler
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override;

    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    Clusters::EnergyEvse::AllDevicesEvseTargetsDelegate mEvseTargetsDelegate;
    TestEventTriggerDelegate & mTestEventTriggerDelegate;
    PersistentStorageDelegate & mPersistentStorageDelegate;
};

} // namespace chip::app
