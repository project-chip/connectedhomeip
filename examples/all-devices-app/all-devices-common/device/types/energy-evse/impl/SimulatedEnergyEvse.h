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

#include <EnergyEvseTargetsStore.h>
#include <EvseSimulationDelegate.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/mode-base-server/ModeBaseCluster.h>
#include <credentials/FabricTable.h>
#include <device/types/energy-evse/EnergyEvse.h>
#include <device/types/energy-evse/impl/SimulatedEnergyEvseTriggers.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/support/TimerDelegate.h>
#include <platform/DiagnosticDataProvider.h>

namespace chip::app {

/**
 * Self-contained simulated Energy EVSE using the self-delegate pattern.
 *
 * Delegate bases are private so external code uses clusters (and test hooks below) instead of
 * calling EnergyEvse::Delegate callbacks directly.
 *
 * Delegate bases are listed before `EnergyEvse` so they are destroyed after the clusters that
 * reference them.
 */
class SimulatedEnergyEvse : private Clusters::EnergyEvse::EvseTargetsDelegate,
                            private Clusters::EnergyEvse::EvseSimulationDelegate,
                            private Clusters::ModeBase::AppDelegate,
                            private Clusters::IdentifyDelegate,
                            private TestEventTriggerHandler,
                            public EnergyEvse
{
public:
    using Clusters::EnergyEvse::EvseSimulationDelegate::HwSetCircuitCapacity;
    using Clusters::EnergyEvse::EvseSimulationDelegate::HwSetState;
    using EnergyEvse::GetEndpointId;

    struct Context
    {
        TimerDelegate & timerDelegate;
        TestEventTriggerDelegate & testEventTriggerDelegate;
        PersistentStorageDelegate & persistentStorageDelegate;
        FabricTable & fabricTable;
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
    TimerDelegate & mTimerDelegate;
    FabricTable & mFabricTable;
    TestEventTriggerDelegate & mTestEventTriggerDelegate;
    PersistentStorageDelegate & mPersistentStorageDelegate;
    SimulatedEnergyEvseTestEventSaveData mEvseTestEventSaveData;
};

} // namespace chip::app
