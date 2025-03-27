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

#include <app/util/attribute-metadata.h>
#include <protocols/interaction_model/StatusCode.h>

#ifdef MATTER_DM_PLUGIN_OPERATIONAL_STATE_SERVER
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

namespace OperationalState {

// This is an application level delegate to handle operational state commands according to the specific business logic.
class GenericOperationalStateDelegateImpl : public Delegate
{
public:
    uint32_t mRunningTime = 0;
    uint32_t mPausedTime  = 0;
    app::DataModel::Nullable<uint32_t> mCountDownTime;

    /**
     * Get the countdown time. This attribute is not used in this application.
     * @return The current countdown time.
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
    CHIP_ERROR GetOperationalPhaseAtIndex(size_t index, MutableCharSpan & operationalPhase) override;

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

protected:
    Span<const GenericOperationalState> mOperationalStateList;
    Span<const CharSpan> mOperationalPhaseList;
};

// This is an application level delegate to handle operational state commands according to the specific business logic.
class OperationalStateDelegate : public GenericOperationalStateDelegateImpl
{
private:
    const GenericOperationalState opStateList[4] = {
        GenericOperationalState(to_underlying(OperationalStateEnum::kStopped)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kRunning)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kPaused)),
        GenericOperationalState(to_underlying(OperationalStateEnum::kError)),
    };
    const CharSpan opPhaseList[3] = { "pre-soak"_span, "rinse"_span, "spin"_span };

public:
    const uint32_t kExampleCountDown = 30;

    OperationalStateDelegate()
    {
        GenericOperationalStateDelegateImpl::mOperationalStateList = Span<const GenericOperationalState>(opStateList);
        GenericOperationalStateDelegateImpl::mOperationalPhaseList = Span<const CharSpan>(opPhaseList);
    }

    /**
     * Handle Command Callback in application: Start
     * @param[out] get operational error after callback.
     */
    void HandleStartStateCallback(GenericOperationalError & err) override
    {
        mCountDownTime.SetNonNull(static_cast<uint32_t>(kExampleCountDown));
        GenericOperationalStateDelegateImpl::HandleStartStateCallback(err);
    }

    /**
     * Handle Command Callback in application: Stop
     * @param[out] get operational error after callback.
     */
    void HandleStopStateCallback(GenericOperationalError & err) override
    {
        GenericOperationalStateDelegateImpl::HandleStopStateCallback(err);
        mCountDownTime.SetNull();
    }
};

Instance * GetOperationalStateInstance();
OperationalStateDelegate * GetOperationalStateDelegate();

void Shutdown();

} // namespace OperationalState
} // namespace Clusters
} // namespace app
} // namespace chip

chip::Protocols::InteractionModel::Status chefOperationalStateWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                            const EmberAfAttributeMetadata * attributeMetadata,
                                                                            uint8_t * buffer);
chip::Protocols::InteractionModel::Status chefOperationalStateReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                           const EmberAfAttributeMetadata * attributeMetadata,
                                                                           uint8_t * buffer, uint16_t maxReadLength);

#endif // MATTER_DM_PLUGIN_OPERATIONAL_STATE_SERVER
