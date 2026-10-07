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

#include <device/types/irrigation-system/Irrigation.h>
#include <device/types/water-valve/WaterValve.h>

#include <memory>
#include <vector>

namespace chip {
namespace app {

class LoggingIrrigation : public Clusters::OperationalState::OperationalStateCluster::Delegate,
                          public Irrigation,
                          public WaterValve::WaterValveListener
{
public:
    LoggingIrrigation(Clusters::IdentifyDelegate & identifyDelegate, TimerDelegate & timerDelegate,
                      std::vector<Irrigation::ValveList> valves) :
        Irrigation(timerDelegate, identifyDelegate, this),
        mValveContext(std::move(valves))
    {}
    ~LoggingIrrigation() override = default;

    DataModel::Nullable<uint32_t> GetCountdownTime() override;

    CHIP_ERROR GetOperationalStateAtIndex(size_t index,
                                          Clusters::OperationalState::GenericOperationalState & operationalState) override;

    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;

    void HandlePauseStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    void HandleResumeStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    void HandleStartStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    void HandleStopStateCallback(Clusters::OperationalState::GenericOperationalError & err) override;

    static std::vector<Irrigation::ValveList> ValveConfiguration();

    void OnValveStateChanged() override;

private:
    CHIP_ERROR RegisterParts(EndpointIdAllocator & allocator, CodeDrivenDataModelProvider & provider,
                             EndpointComposition composition) override;
    void UnregisterParts(CodeDrivenDataModelProvider & provider) override;

    std::vector<Irrigation::ValveList> mValveContext;
    std::vector<std::unique_ptr<WaterValve>> mWaterValves;
};

} // namespace app
} // namespace chip
