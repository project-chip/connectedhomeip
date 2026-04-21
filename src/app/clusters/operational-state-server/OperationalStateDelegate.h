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

#include "operational-state-cluster-objects.h"
#include <app/data-model/Nullable.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/Span.h>

namespace chip {
namespace app {
namespace Clusters {
namespace OperationalState {

class Instance;

/**
 * A delegate to handle application logic of the Operational State aliased Cluster.
 * The delegate API assumes there will be separate delegate objects for each cluster instance.
 * (i.e. each separate operational state cluster derivation, on each separate endpoint),
 * since the delegate methods are not handed the cluster id or endpoint.
 */
class Delegate
{
public:
    Delegate() = default;

    virtual ~Delegate() = default;

    /**
     * Get the countdown time. This will get called on many edges such as
     * commands to change operational state, or when the delegate deals with
     * changes. Make sure it becomes null whenever it is appropriate.
     *
     * @return The current countdown time.
     */
    virtual app::DataModel::Nullable<uint32_t> GetCountdownTime() = 0;

    /**
     * Fills in the provided GenericOperationalState with the state at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of states.
     * Note: This is used by the SDK to populate the operational state list attribute. If the contents of this list changes,
     * the device SHALL call the Instance's ReportOperationalStateListChange method to report that this attribute has changed.
     * @param index The index of the state, with 0 representing the first state.
     * @param operationalState  The GenericOperationalState is filled.
     */
    virtual CHIP_ERROR GetOperationalStateAtIndex(size_t index, GenericOperationalState & operationalState) = 0;

    /**
     * Fills in the provided MutableCharSpan with the phase at index `index` if there is one,
     * or returns CHIP_ERROR_NOT_FOUND if the index is out of range for the list of phases.
     *
     * If CHIP_ERROR_NOT_FOUND is returned for index 0, that indicates that the PhaseList attribute is null
     * (there are no phases defined at all).
     *
     * Note: This is used by the SDK to populate the phase list attribute. If the contents of this list changes, the
     * device SHALL call the Instance's ReportPhaseListChange method to report that this attribute has changed.
     * @param index The index of the phase, with 0 representing the first phase.
     * @param operationalPhase  The MutableCharSpan is filled.
     */
    virtual CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) = 0;

    // command callback
    /**
     * Handle Command Callback in application: Pause
     * @param[out] err operational error after callback.
     */
    virtual void HandlePauseStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Resume
     * @param[out] err operational error after callback.
     */
    virtual void HandleResumeStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Start
     * @param[out] err operational error after callback.
     */
    virtual void HandleStartStateCallback(GenericOperationalError & err) = 0;

    /**
     * Handle Command Callback in application: Stop
     * @param[out] err operational error after callback.
     */
    virtual void HandleStopStateCallback(GenericOperationalError & err) = 0;

    /**
     * This method is used by the SDK to set the instance pointer. This is done during the instantiation of a Instance object.
     * @param aInstance A pointer to the Instance object related to this delegate object.
     * @note This method is for internal SDK use and should only be called by the `Instance` constructor and destructor.
     */
    void SetInstance(Instance * aInstance)
    {
        VerifyOrDie(mInstance == nullptr || aInstance == nullptr || mInstance == aInstance);
        mInstance = aInstance;
    }

private:
    Instance * mInstance = nullptr;

protected:
    /**
     * @brief Provides access to the const Instance pointer.
     *
     * @return A const pointer to the Instance object associated with this delegate.
     */
    const Instance * GetInstance() const { return mInstance; }

    /**
     * @brief Provides access to the Instance pointer.
     *
     * @return A pointer to the Instance object associated with this delegate.
     */
    Instance * GetInstance() { return mInstance; }
};

} // namespace OperationalState

namespace RvcOperationalState {

class Delegate : public OperationalState::Delegate
{
public:
    /**
     * Handle Command Callback in application: GoHome
     * @param[out] err operational error after callback.
     */
    virtual void HandleGoHomeCommandCallback(OperationalState::GenericOperationalError & err)
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnknownEnumValue));
    }

    /**
     * The start command is not supported by the RvcOperationalState cluster hence this method should never be called.
     * This is a dummy implementation of the handler method so the consumer of this class does not need to define it.
     */
    void HandleStartStateCallback(OperationalState::GenericOperationalError & err) override
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnknownEnumValue));
    }

    /**
     * The stop command is not supported by the RvcOperationalState cluster hence this method should never be called.
     * This is a dummy implementation of the handler method so the consumer of this class does not need to define it.
     */
    void HandleStopStateCallback(OperationalState::GenericOperationalError & err) override
    {
        err.Set(to_underlying(OperationalState::ErrorStateEnum::kUnknownEnumValue));
    }
};

} // namespace RvcOperationalState

} // namespace Clusters
} // namespace app
} // namespace chip
