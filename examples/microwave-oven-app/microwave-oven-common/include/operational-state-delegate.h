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
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app/util/af-enums.h>
#include <functional>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

// This is an application level delegate to handle operational state commands according to the specific business logic.
class OperationalStateDelegate : public OperationalState::Delegate
{
private:
    const GenericOperationalState opStateList[4] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
    };

    app::DataModel::List<const GenericOperationalState> mOperationalStateList = Span<const GenericOperationalState>(opStateList);

    const GenericOperationalPhase opPhaseList[1] = {
        // Phase List is null
        GenericOperationalPhase(DataModel::Nullable<CharSpan>()),
    };

    Span<const GenericOperationalPhase> mOperationalPhaseList = Span<const GenericOperationalPhase>(opPhaseList);

    std::function<void(Clusters::OperationalState::GenericOperationalError & err)> mPauseCallback;
    std::function<void(Clusters::OperationalState::GenericOperationalError & err)> mResumeCallback;
    std::function<void(Clusters::OperationalState::GenericOperationalError & err)> mStartCallback;
    std::function<void(Clusters::OperationalState::GenericOperationalError & err)> mStopCallback;
    std::function<app::DataModel::Nullable<uint32_t>(void)> mGetCountdownTimeCallback;

public:
    /**
     * Get the countdown time.
     * @return Null.
     */
    app::DataModel::Nullable<uint32_t> GetCountdownTime() override;

    /**
     * Fills in the provided GenericOperationalState with the state at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of states.
     * Note: This is used by the SDK to populate the operational state list attribute. If the contents of this list changes,
     * the device SHALL call the Instance's ReportOperationalStateListChange method to report that this attribute has changed.
     * @param index The index of the state, with 0 representing the first state.
     * @param operationalState  The GenericOperationalState is filled.
     */
    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) override;

    /**
     * Fills in the provided GenericOperationalPhase with the phase at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of phases.
     * Note: This is used by the SDK to populate the phase list attribute. If the contents of this list changes, the
     * device SHALL call the Instance's ReportPhaseListChange method to report that this attribute has changed.
     * @param index The index of the phase, with 0 representing the first phase.
     * @param operationalPhase  The GenericOperationalPhase is filled.
     */
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, GenericOperationalPhase & operationalPhase) override;

    // command callback
    /**
     * Handle Command Callback in application: Pause
     * @param[out] get operational error after callback.
     */
    void HandlePauseStateCallback(GenericOperationalError & err) override;

    /**
     * Handle Command Callback in application: Resume
     * @param[out] get operational error after callback.
     */
    void HandleResumeStateCallback(GenericOperationalError & err) override;

    /**
     * Handle Command Callback in application: Start
     * @param[out] get operational error after callback.
     */
    void HandleStartStateCallback(GenericOperationalError & err) override;

    /**
     * Handle Command Callback in application: Stop
     * @param[out] get operational error after callback.
     */
    void HandleStopStateCallback(GenericOperationalError & err) override;

    /**
     * Set callback function for pause
     */
    void SetOpStatePauseCallback(std::function<void(Clusters::OperationalState::GenericOperationalError & err)> aCallback);

    /**
     * Set callback function for resume
     */
    void SetOpStateResumeCallback(std::function<void(Clusters::OperationalState::GenericOperationalError & err)> aCallback);

    /**
     * Set callback function for start
     */
    void SetOpStateStartCallback(std::function<void(Clusters::OperationalState::GenericOperationalError & err)> aCallback);

    /**
     * Set callback function for stop
     */
    void SetOpStateStopCallback(std::function<void(Clusters::OperationalState::GenericOperationalError & err)> aCallback);

    /**
     * Set callback function for get count down time
     */
    void SetOpStateGetCountdownTimeCallback(std::function<app::DataModel::Nullable<uint32_t>(void)> aCallback);
};

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
