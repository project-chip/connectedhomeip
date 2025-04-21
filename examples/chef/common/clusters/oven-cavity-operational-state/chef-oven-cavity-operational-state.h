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
#include <protocols/interaction_model/StatusCode.h>

#ifdef MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER

using namespace chip;
using namespace chip::app::Clusters;

namespace chef {
namespace OvenCavityOperationalState {

/**
 * @brief Delegate handles heating cycle in 3 phases: pre-heating (0), pre-heated (1) and cooling down (2).
 * Cycle times are compile constants. TBD if cycle times need to be made run time modifiable.
 */
class Delegate : public OperationalState::Delegate
{
private:
    constexpr CharSpan kPhaseList[3]       = { "pre-heating"_span, "pre-heated"_span, "cooling down"_span };
    constexpr uint32_t kPreHeatingSeconds  = 10;
    constexpr uint32_t kPreHeatedSeconds   = 50;
    constexpr uint32_t kCoolingDownSeconds = 10;
    constexpr uint32_t kCycleSeconds       = kPreHeatingSeconds + kPreHeatedSeconds + kCoolingDownSeconds;
    constexpr OvenCavityOperationalState::OperationalStateEnum kOpStateList[4] = {
        OperationalStateEnum::kStopped,
        OperationalStateEnum::kRunning,
        OperationalStateEnum::kPaused,
        OperationalStateEnum::kError,
    };

    Span<constexpr OvenCavityOperationalState::OperationalStateEnum> mOperationalStateList;
    Span<constexpr CharSpan> mOperationalPhaseList;

    // Non-null when cycle is in progress.
    app::DataModel::Nullable<uint32_t> mRunningTime;
    app::DataModel::Nullable<uint32_t> mPausedTime;

public:
    Delegate()
    {
        mRunningTime.SetNull();
        mPausedTime.SetNull();
        mOperationalStateList = Span<constexpr OvenCavityOperationalState::OperationalStateEnum>(kOpStateList);
        mOperationalPhaseList = Span<constexpr CharSpan>(kPhaseList);
    }

    app::DataModel::Nullable<uint32_t> GetCountdownTime() override;
    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) override;
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;
    void HandlePauseStateCallback(GenericOperationalError & err) override;
    void HandleResumeStateCallback(GenericOperationalError & err) override;
    void HandleStartStateCallback(GenericOperationalError & err) override;
    void HandleStopStateCallback(GenericOperationalError & err) override;

    /**
     * @brief Starts a new cycle with Run/Pause times set to 0. Returns True if new cycle started successfully.
     * Returns False on failure / current cycle in progress.
     */
    bool StartCycle();

    /**
     * @brief Increments run/paused (based on operational state) by one and updates current Phase if changed.
     * NOP if no cycle is in progress / cycle has finished.
     */
    void CycleSecondTick();

    /**
     * @brief Gets the current phase based on current running time. NULL if cycle is not in progress or has
     * completed.
     */
    app::DataModel::Nullable<uint8_t> GetRunningPhase();

    /**
     * @brief Returns True if an in-progress cycle run time has reached its cycle time. False otherwise.
     */
    bool CheckCycleComplete();

    /**
     * @brief Returns True if cycle was started but not ended.
     */
    bool CheckCycleActive();

    /**
     * @brief Sets run/pause times and current phase to NULL.
     */
    void EndCycle();

    uint8_t GetCurrentOperationalState();
};

void InitChefOvenCavityOperationalStateCluster();

} // namespace OvenCavityOperationalState
} // namespace chef

#endif // MATTER_DM_PLUGIN_OVEN_CAVITY_OPERATIONAL_STATE_SERVER
