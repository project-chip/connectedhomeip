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

#include <app/clusters/flow-measurement-server/FlowMeasurementCluster.h>
#include <app/clusters/identify-server/IdentifyCluster.h>
#include <app/clusters/operational-state-server/OperationalStateCluster.h>
#include <app/clusters/valve-configuration-and-control-server/ValveConfigurationAndControlCluster.h>
#include <device/types/irrigation-system/IrrigationSystem.h>
#include <device/types/water-valve/impl/LoggingWaterValve.h>

#include <memory>
#include <optional>
#include <vector>

namespace chip {
namespace app {

/// Water valve of the irrigation system. Reports Running/Stopped and CountdownTime on the irrigation system's
/// OperationalState cluster.
class IrrigationWaterValve : public LoggingWaterValve
{
public:
    IrrigationWaterValve(TimerDelegate & timerDelegate, const WaterValveConfig & config,
                         LazyRegisteredServerCluster<Clusters::OperationalState::OperationalStateCluster> & operationalState,
                         size_t & openValves) :
        LoggingWaterValve(timerDelegate, config),
        mOperationalState(operationalState), mOpenValves(openValves)
    {}

    DataModel::Nullable<Percent> HandleOpenValve(DataModel::Nullable<Percent> level) override;
    CHIP_ERROR HandleCloseValve() override;
    void HandleRemainingDurationTick(uint32_t duration) override;

private:
    void UpdateCountdownTime();

    LazyRegisteredServerCluster<Clusters::OperationalState::OperationalStateCluster> & mOperationalState;
    size_t & mOpenValves; // number of open valves, shared by all water valves of the irrigation system.
    bool mOpen = false;   // a second Open on an open valve (level change) must not count it twice
};

class LoggingIrrigationSystem : public Clusters::OperationalState::OperationalStateCluster::Delegate,
                                public IrrigationSystem,
                                public Clusters::IdentifyDelegate
{
public:
    struct ValveList
    {
        WaterValveConfig config;
        Span<const EndpointComposition::SemanticTag> tags;
    };

    struct PausedZone
    {
        DataModel::Nullable<Percent> level; // null for valves without the Level feature
        DataModel::Nullable<uint32_t> remainingDuration;
    };

    LoggingIrrigationSystem(TimerDelegate & timerDelegate, std::vector<ValveList> valves) :
        IrrigationSystem(timerDelegate), mValveContext(std::move(valves))
    {}
    ~LoggingIrrigationSystem() override = default;

    DataModel::Nullable<uint32_t> GetCountdownTime() override;

    CHIP_ERROR GetOperationalStateAtIndex(size_t index,
                                          Clusters::OperationalState::GenericOperationalState & operationalState) override;

    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;

    void HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    void HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    void HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    void HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    static std::vector<ValveList> ValveConfiguration();

    void OnIdentifyStart(Clusters::IdentifyCluster & cluster) override;

    void OnIdentifyStop(Clusters::IdentifyCluster & cluster) override;

    void OnTriggerEffect(Clusters::IdentifyCluster & cluster) override;

    bool IsTriggerEffectEnabled() const override { return true; }

    bool HasOperationalState() const { return mOperationalStateCluster.IsConstructed(); }

    Clusters::IdentifyCluster & IdentifyCluster()
    {
        VerifyOrDie(mIdentifyCluster.IsConstructed());
        return mIdentifyCluster.Cluster();
    }
    Clusters::OperationalState::OperationalStateCluster & OperationalStateCluster()
    {
        VerifyOrDie(mOperationalStateCluster.IsConstructed());
        return mOperationalStateCluster.Cluster();
    }

protected:
    CHIP_ERROR RegisterOptionalClusters(EndpointId endpoint, CodeDrivenDataModelProvider & provider) override;
    void UnregisterOptionalClusters(CodeDrivenDataModelProvider & provider) override;

private:
    CHIP_ERROR RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                             EndpointComposition composition) override;
    void UnregisterParts(CodeDrivenDataModelProvider & provider) override;
    std::vector<std::optional<PausedZone>> mPausedZones;
    std::vector<ValveList> mValveContext;
    std::vector<std::unique_ptr<IrrigationWaterValve>> mWaterValves;
    size_t mOpenValves = 0; // shared by mWaterValves
    LazyRegisteredServerCluster<Clusters::IdentifyCluster> mIdentifyCluster;
    LazyRegisteredServerCluster<Clusters::OperationalState::OperationalStateCluster> mOperationalStateCluster;
    LazyRegisteredServerCluster<Clusters::FlowMeasurementCluster> mFlowMeasurementCluster;
};

} // namespace app
} // namespace chip
