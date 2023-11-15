/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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
#include <app/clusters/microwave-oven-control-server/microwave-oven-control-server.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app/util/af-enums.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace MicrowaveOvenControl {

// This is an application level delegate to handle operational state and microwave oven control commands according to the specific
// business logic.
class MicrowaveOvenDelegates : public MicrowaveOvenControl::Delegate, public OperationalState::Delegate
{

public:
    // Microwave Oven Control Cluster delegate
    /**
     * Handle Command Callback in application: set-cooking-parameters
     */
    Protocols::InteractionModel::Status HandleSetCookingParametersCallback(Optional<uint8_t> cookMode, Optional<uint32_t> cookTime,
                                                                           Optional<uint8_t> powerSetting) override;

    /**
     * Handle Command Callback in application: add-more-time
     */
    Protocols::InteractionModel::Status HandleAddMoreTimeCallback(uint32_t timeToAdd) override;

    /**
     * Get the value of MinPower.
     */
    uint8_t GetMinPower() override { return mMinPower; }

    /**
     * Set the value of MinPower.
     */
    void SetMinPower(uint8_t minPower) override { return; }

    /**
     * Get the value of MaxPower.
     */
    uint8_t GetMaxPower() override { return mMaxPower; }

    /**
     * Set the value of MaxPower.
     */
    void SetMaxPower(uint8_t maxPower) override { return; }

    /**
     * Get the value of PowerStep.
     */
    uint8_t GetPowerStep() override { return mPowerStep; }

    /**
     * Set the value of PowerStep.
     */
    void SetPowerStep(uint8_t powerStep) override { return; }

    // Operational State Cluster delegate
    /**
     * Get the countdown time.
     * @return The current countdown time.
     */
    app::DataModel::Nullable<uint32_t> GetCountdownTime() override;

    /**
     * Get the list of supported operational states.
     * Fills in the provided GenericOperationalState with the state at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of states.
     * @param index The index of the state, with 0 representing the first state.
     * @param operationalState  The GenericOperationalState is filled.
     */
    CHIP_ERROR GetOperationalStateAtIndex(size_t index, OperationalState::GenericOperationalState & operationalState) override;

    /**
     * Get the list of supported operational phases.
     * Fills in the provided GenericOperationalPhase with the phase at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of phases.
     * @param index The index of the phase, with 0 representing the first phase.
     * @param operationalPhase  The GenericOperationalPhase is filled.
     */
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, OperationalState::GenericOperationalPhase & operationalPhase) override;

    /**
     * Handle Command Callback in application: Pause
     * @param[out] get operational error after callback.
     */
    void HandlePauseStateCallback(OperationalState::GenericOperationalError & err) override;

    /**
     * Handle Command Callback in application: Resume
     * @param[out] get operational error after callback.
     */
    void HandleResumeStateCallback(OperationalState::GenericOperationalError & err) override;

    /**
     * Handle Command Callback in application: Start
     * @param[out] get operational error after callback.
     */
    void HandleStartStateCallback(OperationalState::GenericOperationalError & err) override;

    /**
     * Handle Command Callback in application: Stop
     * @param[out] get operational error after callback.
     */
    void HandleStopStateCallback(OperationalState::GenericOperationalError & err) override;

private:
    const OperationalState::GenericOperationalState rvcOpStateList[4] = {
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kStopped)),
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kRunning)),
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kPaused)),
        OperationalState::GenericOperationalState(to_underlying(OperationalState::OperationalStateEnum::kError)),
    };

    app::DataModel::List<const OperationalState::GenericOperationalState> mOperationalStateList =
        Span<const OperationalState::GenericOperationalState>(rvcOpStateList);

    const OperationalState::GenericOperationalPhase opPhaseList[1] = {
        // Phase List is null
        OperationalState::GenericOperationalPhase(DataModel::Nullable<CharSpan>()),
    };

    Span<const OperationalState::GenericOperationalPhase> mOperationalPhaseList =
        Span<const OperationalState::GenericOperationalPhase>(opPhaseList);

    /** @brief Check if the given cook time is in range
     *  @param cookTime    cookTime that given by user
     */
    bool IsCookTimeInRange(uint32_t cookTime);

    /** @brief Check if the given cooking power is in range
     *  @param powerSetting    power setting that given by user
     *  @param minCookPower    the min power setting that defined via zap configuration
     *  @param maxCookPower    the max power setting that defined via zap configuration
     */
    bool IsPowerSettingInRange(uint8_t powerSetting, uint8_t minCookPower, uint8_t maxCookPower);
};

void Shutdown();

} // namespace MicrowaveOvenControl
} // namespace Clusters
} // namespace app
} // namespace chip
