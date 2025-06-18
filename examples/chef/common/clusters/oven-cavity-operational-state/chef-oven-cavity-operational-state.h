/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app/util/attribute-metadata.h>
#include <app/util/config.h>
#include <protocols/interaction_model/StatusCode.h>

#ifdef MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER

namespace chip {
namespace app {
namespace Clusters {
namespace OvenCavityOperationalState {
/**
 * @brief Delegate handles heating cycle in 3 phases: pre-heating (0), pre-heated (1) and cooling down (2).
 * Cycle times are constants. TBD if cycle times need to be made run time modifiable.
 */
class ChefDelegate : public OperationalState::Delegate
{
private:
    const CharSpan kPhaseList[3]               = { "pre-heating"_span, "pre-heated"_span, "cooling down"_span };
    const uint8_t kPreHeatingIndex             = 0;
    const uint8_t kPreHeatedIndex              = 1;
    const uint8_t kCoolingDownIndex            = 2;
    const uint32_t kPreHeatingSeconds          = 10;
    const uint32_t kPreHeatedSeconds           = 50;
    const uint32_t kCoolingDownSeconds         = 10;
    const uint32_t kCycleSeconds               = kPreHeatingSeconds + kPreHeatedSeconds + kCoolingDownSeconds;
    const OperationalStateEnum kOpStateList[3] = {
        OperationalStateEnum::kStopped,
        OperationalStateEnum::kRunning,
        OperationalStateEnum::kError,
    };

    Span<const OperationalStateEnum> mOperationalStateList;
    Span<const CharSpan> mOperationalPhaseList;

    // Non-null when cycle is in progress.
    DataModel::Nullable<uint32_t> mRunningTime;

public:
    ChefDelegate()
    {
        mRunningTime.SetNull();
        mOperationalStateList = Span<const OperationalStateEnum>(kOpStateList);
        mOperationalPhaseList = Span<const CharSpan>(kPhaseList);
    }

    DataModel::Nullable<uint32_t> GetCountdownTime() override;
    CHIP_ERROR GetOperationalStateAtIndex(size_t index, OperationalState::GenericOperationalState & operationalState) override;
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;
    void HandlePauseStateCallback(OperationalState::GenericOperationalError & err) override;
    void HandleResumeStateCallback(OperationalState::GenericOperationalError & err) override;
    void HandleStartStateCallback(OperationalState::GenericOperationalError & err) override;
    void HandleStopStateCallback(OperationalState::GenericOperationalError & err) override;

    /**
     * @brief Starts a new cycle with Run time set to 0 and current phase set to 0.
     * Returns True if new cycle started successfully.
     * Returns False on failure or if a cycle is already active.
     */
    bool StartCycle();

    /**
     * @brief Increments run time by one and updates current Phase if changed.
     * NOP if no cycle is active or current cycle has finished.
     */
    void CycleSecondTick();

    /**
     * @brief Gets the current phase based on current running time. NULL if no cycle is active or cycle has
     * completed.
     */
    app::DataModel::Nullable<uint8_t> GetRunningPhase();

    /**
     * @brief Returns True if an active cycles run time has reached its cycle time. False otherwise.
     */
    bool CheckCycleComplete();

    /**
     * @brief Returns True if a cycle is active.
     */
    bool CheckCycleActive();

    /**
     * @brief Deactivates cycle by setting run-time and current phase to NULL.
     */
    void EndCycle();

    uint8_t GetCurrentOperationalState();
};

void InitChefOvenCavityOperationalStateCluster();

} // namespace OvenCavityOperationalState

} // namespace Clusters
} // namespace app
} // namespace chip

#endif // MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER
