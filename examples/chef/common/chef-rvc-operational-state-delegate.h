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

#ifdef MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {

namespace RvcOperationalState {

// Enum for OperationalStateEnum that has custom values for us.
enum class ChefRvcOperationalStateEnum : uint8_t
{
    kStopped          = 0x00,
    kRunning          = 0x01,
    kPaused           = 0x02,
    kError            = 0x03,
    kSeekingCharger   = 0x40,
    kCharging         = 0x41,
    kDocked           = 0x42,
    kEmptyingDustBin  = 0x43,
    kCleaningMop      = 0x44,
    kFillingWaterTank = 0x45,
    kUpdatingMaps     = 0x46,

    // Custom manufacturer-range operational state. Pretend we are running in a state that
    // beeps really loud to annoy everyone.
    kRunningWhileBeeping = 0x81,
};

// This is an application level delegate to handle operational state commands according to the specific business logic.
class RvcOperationalStateDelegate : public RvcOperationalState::Delegate
{
public:
    RvcOperationalStateDelegate()
    {
        mOperationalStateList = Span<const OperationalState::GenericOperationalState>(mRvcOpStateList, std::size(mRvcOpStateList));
    }

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
    CHIP_ERROR GetOperationalStateAtIndex(size_t index, OperationalState::GenericOperationalState & operationalState) override;

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

    /**
     * Handle Command Callback in application: GoHome
     */
    void HandleGoHomeCommandCallback(OperationalState::GenericOperationalError & err) override;

    void SetCurrentRunningState(RvcOperationalState::ChefRvcOperationalStateEnum RunningState)
    {
        mCurrentRunningState = RunningState;
    };

    uint32_t mRunningTime = 0;
    uint32_t mPausedTime  = 0;
    app::DataModel::Nullable<uint32_t> mCountdownTime;
    const uint32_t kExampleCountDown = 30;

private:
    Span<const OperationalState::GenericOperationalState> mOperationalStateList;
    Span<const CharSpan> mOperationalPhaseList;

    const OperationalState::GenericOperationalState mRvcOpStateList[8] = {
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::ChefRvcOperationalStateEnum::kStopped)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::ChefRvcOperationalStateEnum::kRunning)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::ChefRvcOperationalStateEnum::kPaused)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::ChefRvcOperationalStateEnum::kError)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::ChefRvcOperationalStateEnum::kSeekingCharger)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::ChefRvcOperationalStateEnum::kCharging)),
        OperationalState::GenericOperationalState(to_underlying(RvcOperationalState::ChefRvcOperationalStateEnum::kDocked)),
        OperationalState::GenericOperationalState(
            to_underlying(RvcOperationalState::ChefRvcOperationalStateEnum::kRunningWhileBeeping),
            Optional<CharSpan>("RunningWhileBeeping"_span)),
    };

    RvcOperationalState::ChefRvcOperationalStateEnum mCurrentRunningState =
        RvcOperationalState::ChefRvcOperationalStateEnum::kRunning;
};

void Shutdown();

} // namespace RvcOperationalState
} // namespace Clusters
} // namespace app
} // namespace chip

chip::app::Clusters::RvcOperationalState::RvcOperationalStateDelegate * getRvcOperationalStateDelegate();

chip::Protocols::InteractionModel::Status chefRvcOperationalStateWriteCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                               const EmberAfAttributeMetadata * attributeMetadata,
                                                                               uint8_t * buffer);
chip::Protocols::InteractionModel::Status chefRvcOperationalStateReadCallback(chip::EndpointId endpoint, chip::ClusterId clusterId,
                                                                              const EmberAfAttributeMetadata * attributeMetadata,
                                                                              uint8_t * buffer, uint16_t maxReadLength);
#endif // MATTER_DM_PLUGIN_RVC_OPERATIONAL_STATE_SERVER
