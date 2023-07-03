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

/*
 * An example state (not in spec) to present a device that is unable to honour the Pause/Resume/Start/Stop command
 */
enum class ManufactureOperationalStateEnum : uint8_t
{
    kChildSafetyLock = 0x80,
};

// This is an application level delegate to handle operational state commands according to the specific business logic.
class OperationalStateDelegate : public Delegate
{

public:
    /**
     * Get current operational state.
     * @param op Put a struct instance on the state, then call the delegate to fill it in.
     * @return void.
     */
    void GetCurrentOperationalState(GenericOperationalState & op) override;

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
    void GetOperationalError(GenericOperationalError & error) override;

    /**
     * Set operational error.
     * @param opErrState The reference of operational error.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR SetOperationalError(const GenericOperationalError & opErrState) override;

    /**
     * Set operational state.
     * @param opState The operational state for which to set.
     * @return CHIP_ERROR CHIP_NO_ERROR on success, or corresponding error code.
     */
    CHIP_ERROR SetOperationalState(const GenericOperationalState & opState) override;

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

    OperationalStateDelegate(EndpointId aEndpointId, ClusterId aClusterId, GenericOperationalState aOperationalState,
                             GenericOperationalError aOperationalError, Span<const GenericOperationalState> aOperationalStateList,
                             Span<const GenericOperationalPhase> aOperationalPhaseList) :
        Delegate(aEndpointId, aClusterId),
        mOperationalState(aOperationalState), mOperationalError(aOperationalError), mOperationalStateList(aOperationalStateList),
        mOperationalPhaseList(aOperationalPhaseList)
    {}
    ~OperationalStateDelegate() = default;

private:
    GenericOperationalState mOperationalState;
    GenericOperationalError mOperationalError;
    app::DataModel::List<const GenericOperationalState> mOperationalStateList;
    app::DataModel::List<const GenericOperationalPhase> mOperationalPhaseList;
};

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip
