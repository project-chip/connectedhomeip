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
#include <app/AttributeAccessInterface.h>
#include <app/CommandHandlerInterface.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/StatusIB.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <lib/core/CHIPError.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

class Delegate
{
public:
    virtual ~Delegate() = default;

    void SetEndpointId(EndpointId aEndpoint) { mEndpointId = aEndpoint; }

    /**
     * @brief Delegate should implement a handler to begin to adjust client power
     *        consumption/generation to the level requested.
     *
     *        Note callers must call GetPowerAdjustmentCapability and ensure the return value is not null
     *        before calling PowerAdjustRequest.
     *
     * @param power Milli-Watts the ESA SHALL use during the adjustment period.
     * @param duration The duration that the ESA SHALL maintain the requested power for.
     * @return  Success if the adjustment is accepted; otherwise the command SHALL be rejected with appropriate error.
     */
    virtual Protocols::InteractionModel::Status PowerAdjustRequest(const int64_t power, const uint32_t duration,
                                                                   AdjustmentCauseEnum cause) = 0;

    /**
     * @brief Delegate SHALL make the ESA end the active power adjustment session & return to normal (or idle) power levels.
     *        The ESA SHALL also generate an PowerAdjustEnd Event and the ESAState SHALL be restored to Online.
     *
     * @return It should report SUCCESS if successful and FAILURE otherwise.
     */
    virtual Protocols::InteractionModel::Status CancelPowerAdjustRequest() = 0;

    /**
     * @brief Delegate for the ESA SHALL update its Forecast attribute with the RequestedStartTime including a new ForecastID.
     *
     *   If the ESA supports ForecastAdjustment, and the ESAState is not UserOptOut and the RequestedStartTime is after
     *   the EarliestStartTime and the resulting EndTime is before the LatestEndTime, then ESA SHALL accept the request
     *   to modify the Start Time.
     *   A client can estimate the entire Forecast sequence duration by computing the EndTime - StartTime fields from the
     *   Forecast attribute, and therefore avoid scheduling the start time too late.
     *
     * @param requestedStartTime The requested start time in UTC that the client would like the appliance to shift its power
     * forecast to.
     * @param cause    Who (Grid/local) is triggering this change.
     *
     * @return Success if the StartTime in the Forecast is updated, otherwise the command SHALL be rejected with appropriate
     * IM_Status.
     */
    virtual Protocols::InteractionModel::Status StartTimeAdjustRequest(const uint32_t requestedStartTime,
                                                                       AdjustmentCauseEnum cause) = 0;

    /**
     * @brief Delegate handler for PauseRequest command
     *
     *   If the ESA supports FA and the SlotIsPauseable field is true in the ActiveSlotNumber
     *   index in the Slots list, and the ESAState is not UserOptOut then the ESA SHALL allow its current
     *   operation to be Paused.
     *
     *   During this state the ESA SHALL not consume or produce significant power (other than required to keep its
     *   basic control system operational).
     *
     * @param duration Duration that the ESA SHALL be paused for.
     * @return  Success if the ESA is paused, otherwise returns other IM_Status.
     */
    virtual Protocols::InteractionModel::Status PauseRequest(const uint32_t duration, AdjustmentCauseEnum cause) = 0;

    /**
     * @brief Delegate handler for ResumeRequest command
     *
     *   If the ESA supports FA and it is currently Paused then the ESA SHALL resume its operation.
     *   The ESA SHALL also generate a Resumed Event and the ESAState SHALL be updated accordingly to
     *   reflect its current state.
     *
     * @return  Success if the ESA is resumed, otherwise returns other IM_Status.
     */
    virtual Protocols::InteractionModel::Status ResumeRequest() = 0;

    /**
     * @brief Delegate handler for ModifyForecastRequest
     *
     *   If the ESA supports FA, and the ESAState is not UserOptOut it SHALL attempt to adjust its power forecast.
     *   This allows a one or more modifications in a single command by sending a list of modifications (one for each 'slot').
     *   Attempts to modify slots which have already past, SHALL result in the entire command being rejected.
     *   If the ESA accepts the requested Forecast then it SHALL update its Forecast attribute (incrementing its ForecastID)
     *   and run the revised Forecast as its new intended operation.
     *
     * @param forecastID Indicates the ESA ForecastID that is to be modified.
     * @param slotAdjustments List of adjustments to be applied to the ESA, corresponding to the expected ESA forecastID.
     * @return  Success if the entire list of SlotAdjustmentStruct are accepted, otherwise the command
     *          SHALL be rejected returning other IM_Status.
     */
    virtual Protocols::InteractionModel::Status
    ModifyForecastRequest(const uint32_t forecastID,
                          const DataModel::DecodableList<Structs::SlotAdjustmentStruct::Type> & slotAdjustments,
                          AdjustmentCauseEnum cause) = 0;

    /**
     * @brief Delegate handler for RequestConstraintBasedForecast
     *
     *   The ESA SHALL inspect the requested power limits to ensure that there are no overlapping elements. The ESA
     *   manufacturer may also reject the request if it could cause the user’s preferences to be breached (e.g. may
     *   cause the home to be too hot or too cold, or a battery to be insufficiently charged).
     *   If the ESA can meet the requested power limits, it SHALL regenerate a new Power Forecast with a new ForecastID.
     *
     * @param constraints  Sequence of turn up/down power requests that the ESA is being asked to constrain its operation within.
     * @return  Success if successful, otherwise the command SHALL be rejected returning other IM_Status.
     */
    virtual Protocols::InteractionModel::Status
    RequestConstraintBasedForecast(const DataModel::DecodableList<Structs::ConstraintsStruct::Type> & constraints,
                                   AdjustmentCauseEnum cause) = 0;

    /**
     * @brief Delegate handler for CancelRequest
     *
     *   The ESA SHALL attempt to cancel the effects of any previous adjustment request commands, and re-evaluate its
     *   forecast for intended operation ignoring those previous requests.
     *
     *   If the ESA ForecastStruct ForecastUpdateReason was already `Internal Optimization`, then the command SHALL
     *   be rejected with FAILURE.
     *
     *   If the command is accepted, the ESA SHALL update its ESAState if required, and the command status returned
     *   SHALL be SUCCESS.
     *
     *   The ESA SHALL update its Forecast attribute to match its new intended operation, and update the
     *   ForecastStruct.ForecastUpdateReason to `Internal Optimization`
     *
     * @return  Success if successful, otherwise the command SHALL be rejected returning other IM_Status.
     */
    virtual Protocols::InteractionModel::Status CancelRequest() = 0;

    // ------------------------------------------------------------------
    // Get attribute methods
    virtual ESATypeEnum GetESAType()         = 0;
    virtual bool GetESACanGenerate()         = 0;
    virtual ESAStateEnum GetESAState()       = 0;
    virtual int64_t GetAbsMinPower()         = 0;
    virtual int64_t GetAbsMaxPower()         = 0;
    virtual OptOutStateEnum GetOptOutState() = 0;

    /**
     * @brief Returns the current PowerAdjustCapability object
     *
     * The reference returned from GetPowerAdjustmentCapability() is only valid until the next Matter event
     * is processed.  Callers must not hold on to that reference for any asynchronous processing.
     *
     * Once another Matter event has had a chance to run, the memory associated with the
     * PowerAdjustCapabilityStruct is likely to change or be re-allocated, so would become invalid.
     *
     * @return  The current PowerAdjustCapability object
     */
    virtual const DataModel::Nullable<Structs::PowerAdjustCapabilityStruct::Type> & GetPowerAdjustmentCapability() = 0;

    /**
     * @brief Returns the current Forecast object
     *
     * The reference returned from GetForecast() is only valid until the next Matter event
     * is processed.  Callers must not hold on to that reference for any asynchronous processing.
     *
     * Once another Matter event has had a chance to run, the memory associated with the
     * ForecastStruct is likely to change or be re-allocated, so would become invalid.
     *
     * @return  The current Forecast object
     */
    virtual const DataModel::Nullable<Structs::ForecastStruct::Type> & GetForecast() = 0;

    // ------------------------------------------------------------------
    // Set attribute methods
    virtual CHIP_ERROR SetESAState(ESAStateEnum) = 0;

protected:
    EndpointId mEndpointId = 0;
};

class Instance : public AttributeAccessInterface, public CommandHandlerInterface
{
public:
    Instance(EndpointId aEndpointId, Delegate & aDelegate, Feature aFeature) :
        AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
        mDelegate(aDelegate), mFeature(aFeature)
    {
        /* set the base class delegates endpointId */
        mDelegate.SetEndpointId(aEndpointId);
    }

    ~Instance() { Shutdown(); }

    CHIP_ERROR Init();
    void Shutdown();

    bool HasFeature(Feature aFeature) const;

private:
    Protocols::InteractionModel::Status GetMatterEpochTimeFromUnixTime(uint32_t & currentUtcTime) const;

private:
    Delegate & mDelegate;
    BitMask<Feature> mFeature;

    // AttributeAccessInterface
    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;
    // NOTE there are no writable attributes

    // CommandHandlerInterface
    void InvokeCommand(HandlerContext & handlerContext) override;
    CHIP_ERROR EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context) override;

    Protocols::InteractionModel::Status CheckOptOutAllowsRequest(AdjustmentCauseEnum adjustmentCause);
    void HandlePowerAdjustRequest(HandlerContext & ctx, const Commands::PowerAdjustRequest::DecodableType & commandData);
    void HandleCancelPowerAdjustRequest(HandlerContext & ctx,
                                        const Commands::CancelPowerAdjustRequest::DecodableType & commandData);
    void HandleStartTimeAdjustRequest(HandlerContext & ctx, const Commands::StartTimeAdjustRequest::DecodableType & commandData);
    void HandlePauseRequest(HandlerContext & ctx, const Commands::PauseRequest::DecodableType & commandData);
    void HandleResumeRequest(HandlerContext & ctx, const Commands::ResumeRequest::DecodableType & commandData);
    void HandleModifyForecastRequest(HandlerContext & ctx, const Commands::ModifyForecastRequest::DecodableType & commandData);
    void HandleRequestConstraintBasedForecast(HandlerContext & ctx,
                                              const Commands::RequestConstraintBasedForecast::DecodableType & commandData);
    void HandleCancelRequest(HandlerContext & ctx, const Commands::CancelRequest::DecodableType & commandData);
};

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip
