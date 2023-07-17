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
#include <app/clusters/operational-state-server/operational-state-delegate.h>
#include <app/util/af-enums.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

// This is an application level delegate to handle operational state commands according to the specific business logic.
class OperationalStateDelegate : public Delegate
{

public:
    /**
     * Get the current operational state.
     * @return The current operational state value
     */
    uint8_t GetCurrentOperationalState() override;

    /**
     * Get the list of supported operational states.
     * Fills in the provided GenericOperationalState with the state at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of states.
     * @param index The index of the state, with 0 representing the first state.
     * @param operationalState  The GenericOperationalState is filled.
     */
    CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) override;

    /**
     * Get the list of supported operational phases.
     * Fills in the provided GenericOperationalPhase with the phase at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of phases.
     * @param index The index of the phase, with 0 representing the first phase.
     * @param operationalPhase  The GenericOperationalPhase is filled.
     */
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, GenericOperationalPhase & operationalPhase) override;

    /**
     * Get current operational error.
     * @param error The GenericOperationalError to fill with the current operational error value
     */
    void GetCurrentOperationalError(GenericOperationalError & error) override;

    /**
     * Get current phase
     * @param phase The app::DataModel::Nullable<uint8_t> to fill with the current phase value
     */
    void GetCurrentPhase(app::DataModel::Nullable<uint8_t> & phase) override;

    /**
     * Get countdown time
     * @param time The app::DataModel::Nullable<uint32_t> to fill with the coutdown time value
     */
    void GetCountdownTime(app::DataModel::Nullable<uint32_t> & time) override;

    /**
     * Set operational error.
     * @param opErrState The new operational error.
     */
    CHIP_ERROR SetOperationalError(const GenericOperationalError & opErrState) override;

    /**
     * Set current operational state.
     * @param opState The operational state that should now be the current one.
     */
    CHIP_ERROR SetOperationalState(uint8_t opState) override;

    /**
     * Set operational phase.
     * @param phase The operational phase that should now be the current one.
     */
    CHIP_ERROR SetPhase(const app::DataModel::Nullable<uint8_t> & phase) override;

    /**
     * Set coutdown time.
     * @param time The coutdown time that should now be the current one.
     */
    CHIP_ERROR SetCountdownTime(const app::DataModel::Nullable<uint32_t> & time) override;

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

    OperationalStateDelegate(uint8_t aOperationalState, GenericOperationalError aOperationalError,
                             Span<const GenericOperationalState> aOperationalStateList,
                             Span<const GenericOperationalPhase> aOperationalPhaseList,
                             app::DataModel::Nullable<uint8_t> aPhase          = DataModel::Nullable<uint8_t>(),
                             app::DataModel::Nullable<uint32_t> aCountdownTime = DataModel::Nullable<uint32_t>()) :
        mOperationalState(aOperationalState),
        mOperationalError(aOperationalError), mOperationalStateList(aOperationalStateList),
        mOperationalPhaseList(aOperationalPhaseList), mOperationalPhase(aPhase), mCountdownTime(aCountdownTime)
    {}
    ~OperationalStateDelegate() = default;

private:
    uint8_t mOperationalState;
    GenericOperationalError mOperationalError;
    app::DataModel::List<const GenericOperationalState> mOperationalStateList;
    Span<const GenericOperationalPhase> mOperationalPhaseList;
    app::DataModel::Nullable<uint8_t> mOperationalPhase;
    app::DataModel::Nullable<uint32_t> mCountdownTime;
};

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
