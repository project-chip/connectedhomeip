/*
 *    Copyright (c) 2023 Project CHIP Authors
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

#include "device-energy-management-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::DeviceEnergyManagement;
using namespace chip::app::Clusters::DeviceEnergyManagement::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace DeviceEnergyManagement {

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));
    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);
    unregisterAttributeAccessOverride(this);
}

bool Instance::HasFeature(Feature aFeature) const
{
    return mFeature.Has(aFeature);
}

// AttributeAccessInterface
CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case ESAType::Id:
        return aEncoder.Encode(mDelegate.GetESAType());
    case ESACanGenerate::Id:
        return aEncoder.Encode(mDelegate.GetESACanGenerate());
    case ESAState::Id:
        return aEncoder.Encode(mDelegate.GetESAState());
    case AbsMinPower::Id:
        return aEncoder.Encode(mDelegate.GetAbsMinPower());
    case AbsMaxPower::Id:
        return aEncoder.Encode(mDelegate.GetAbsMaxPower());
    case PowerAdjustmentCapability::Id:
        /* PA - PowerAdjustment */
        if (!HasFeature(Feature::kPowerAdjustment))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetPowerAdjustmentCapability());
    case Forecast::Id:
        /* PFR | SFR - Power Forecast Reporting or State Forecast Reporting */
        if (!HasFeature(Feature::kPowerForecastReporting) && !HasFeature(Feature::kStateForecastReporting))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetForecast());
    case OptOutState::Id:
        /* PA | STA | PAU | FA | CON */
        if (!HasFeature(Feature::kPowerAdjustment) && !HasFeature(Feature::kStartTimeAdjustment) &&
            !HasFeature(Feature::kPausable) && !HasFeature(Feature::kForecastAdjustment) &&
            !HasFeature(Feature::kConstraintBasedAdjustment))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetOptOutState());

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);
    }

    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

// CommandHandlerInterface
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Commands;

    if (HasFeature(Feature::kPowerAdjustment))
    {
        for (auto && cmd : {
                 PowerAdjustRequest::Id,
                 CancelPowerAdjustRequest::Id,
             })
        {
            VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
        }
    }

    if (HasFeature(Feature::kStartTimeAdjustment))
    {
        VerifyOrExit(callback(StartTimeAdjustRequest::Id, context) == Loop::Continue, /**/);
    }

    if (HasFeature(Feature::kPausable))
    {
        VerifyOrExit(callback(PauseRequest::Id, context) == Loop::Continue, /**/);
        VerifyOrExit(callback(ResumeRequest::Id, context) == Loop::Continue, /**/);
    }

    if (HasFeature(Feature::kForecastAdjustment))
    {
        VerifyOrExit(callback(ModifyForecastRequest::Id, context) == Loop::Continue, /**/);
    }

    if (HasFeature(Feature::kConstraintBasedAdjustment))
    {
        VerifyOrExit(callback(RequestConstraintBasedForecast::Id, context) == Loop::Continue, /**/);
    }

    if (HasFeature(Feature::kStartTimeAdjustment) || HasFeature(Feature::kForecastAdjustment) ||
        HasFeature(Feature::kConstraintBasedAdjustment))
    {
        VerifyOrExit(callback(CancelRequest::Id, context) == Loop::Continue, /**/);
    }

exit:
    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case PowerAdjustRequest::Id:
        if (!HasFeature(Feature::kPowerAdjustment))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<PowerAdjustRequest::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandlePowerAdjustRequest(ctx, commandData); });
        }
        return;
    case CancelPowerAdjustRequest::Id:
        if (!HasFeature(Feature::kPowerAdjustment))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<CancelPowerAdjustRequest::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleCancelPowerAdjustRequest(ctx, commandData); });
        }
        return;
    case StartTimeAdjustRequest::Id:
        if (!HasFeature(Feature::kStartTimeAdjustment))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<StartTimeAdjustRequest::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleStartTimeAdjustRequest(ctx, commandData); });
        }
        return;
    case PauseRequest::Id:
        if (!HasFeature(Feature::kPausable))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<PauseRequest::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandlePauseRequest(ctx, commandData); });
        }
        return;
    case ResumeRequest::Id:
        if (!HasFeature(Feature::kPausable))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<ResumeRequest::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleResumeRequest(ctx, commandData); });
        }
        return;
    case ModifyForecastRequest::Id:
        if (!HasFeature(Feature::kForecastAdjustment))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<ModifyForecastRequest::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleModifyForecastRequest(ctx, commandData); });
        }
        return;
    case RequestConstraintBasedForecast::Id:
        if (!HasFeature(Feature::kConstraintBasedAdjustment))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<RequestConstraintBasedForecast::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleRequestConstraintBasedForecast(ctx, commandData); });
        }
        return;
    case CancelRequest::Id:
        if (!HasFeature(Feature::kStartTimeAdjustment) && !HasFeature(Feature::kForecastAdjustment) &&
            !HasFeature(Feature::kConstraintBasedAdjustment))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<CancelRequest::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleCancelRequest(ctx, commandData); });
        }
        return;
    }
}

Status Instance::CheckOptOutAllowsRequest(AdjustmentCauseEnum adjustmentCause)
{
    OptOutStateEnum optOutState = mDelegate.GetOptOutState();

    if (adjustmentCause == AdjustmentCauseEnum::kUnknownEnumValue)
    {
        ChipLogError(Zcl, "DEM: adjustment cause is invalid (%d)", static_cast<int>(adjustmentCause));
        return Status::InvalidValue;
    }

    switch (optOutState)
    {
    case OptOutStateEnum::kNoOptOut: /* User has NOT opted out so allow it */
        ChipLogProgress(Zcl, "DEM: OptOutState = kNoOptOut");
        return Status::Success;

    case OptOutStateEnum::kLocalOptOut: /* User has opted out from Local only*/
        ChipLogProgress(Zcl, "DEM: OptOutState = kLocalOptOut");
        switch (adjustmentCause)
        {
        case AdjustmentCauseEnum::kGridOptimization:
            return Status::Success;
        case AdjustmentCauseEnum::kLocalOptimization:
        default:
            return Status::Failure;
        }

    case OptOutStateEnum::kGridOptOut: /* User has opted out from Grid only */
        ChipLogProgress(Zcl, "DEM: OptOutState = kGridOptOut");
        switch (adjustmentCause)
        {
        case AdjustmentCauseEnum::kLocalOptimization:
            return Status::Success;
        case AdjustmentCauseEnum::kGridOptimization:
        default:
            return Status::Failure;
        }

    case OptOutStateEnum::kOptOut: /* User has opted out from both local and grid */
        ChipLogProgress(Zcl, "DEM: OptOutState = kOptOut");
        return Status::Failure;

    default:
        ChipLogError(Zcl, "DEM: invalid optOutState %d", static_cast<int>(optOutState));
        return Status::InvalidValue;
    }
}

void Instance::HandlePowerAdjustRequest(HandlerContext & ctx, const Commands::PowerAdjustRequest::DecodableType & commandData)
{
    Status status;
    bool validArgs = false;
    PowerAdjustmentCapability::TypeInfo::Type powerAdjustmentCapability;

    int64_t power                       = commandData.power;
    uint32_t durationSec                = commandData.duration;
    AdjustmentCauseEnum adjustmentCause = commandData.cause;

    status = CheckOptOutAllowsRequest(adjustmentCause);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: PowerAdjustRequest command rejected");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    powerAdjustmentCapability = mDelegate.GetPowerAdjustmentCapability();
    if (powerAdjustmentCapability.IsNull())
    {
        ChipLogError(Zcl, "DEM: powerAdjustmentCapability IsNull");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    /* PowerAdjustmentCapability is a list - so iterate through checking if the command is within one of the offers */
    for (auto pas : powerAdjustmentCapability.Value())
    {
        if ((power >= pas.minPower) && (durationSec >= pas.minDuration) && (power <= pas.maxPower) &&
            (durationSec <= pas.maxDuration))
        {
            ChipLogProgress(Zcl, "DEM: Good PowerAdjustment args");
            validArgs = true;
            break;
        }
    }

    if (!validArgs)
    {
        ChipLogError(Zcl, "DEM: invalid request range");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    ChipLogProgress(Zcl, "DEM: Good PowerAdjustRequest() args.");

    status = mDelegate.PowerAdjustRequest(power, durationSec, adjustmentCause);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: Failed to PowerAdjustRequest() args.");
    }
}

void Instance::HandleCancelPowerAdjustRequest(HandlerContext & ctx,
                                              const Commands::CancelPowerAdjustRequest::DecodableType & commandData)
{
    Status status;

    /* Check that the ESA state is PowerAdjustActive */
    ESAStateEnum esaStatus = mDelegate.GetESAState();
    if (ESAStateEnum::kPowerAdjustActive != esaStatus)
    {
        ChipLogError(Zcl, "DEM: kPowerAdjustActive != esaStatus");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    status = mDelegate.CancelPowerAdjustRequest();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: Failed to CancelPowerAdjustRequest()");
        return;
    }
}

void Instance::HandleStartTimeAdjustRequest(HandlerContext & ctx,
                                            const Commands::StartTimeAdjustRequest::DecodableType & commandData)
{
    Status status;
    uint32_t earliestStartTimeEpoch = 0;
    uint32_t latestEndTimeEpoch     = 0;
    uint32_t duration;

    uint32_t requestedStartTimeEpoch    = commandData.requestedStartTime;
    AdjustmentCauseEnum adjustmentCause = commandData.cause;

    status = CheckOptOutAllowsRequest(adjustmentCause);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: StartTimeAdjustRequest command rejected");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    DataModel::Nullable<Structs::ForecastStruct::Type> forecastNullable = mDelegate.GetForecast();

    if (forecastNullable.IsNull())
    {
        ChipLogError(Zcl, "DEM: Forecast is Null");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* Temporary variable to save calling .Value() on forecastNullable */
    auto & forecast = forecastNullable.Value();

    /**
     * If the RequestedStartTime value resulted in a time shift which is
     * outside the time constraints of EarliestStartTime and
     * LatestEndTime, then the command SHALL be rejected with CONSTRAINT_ERROR;
     * in other failure scenarios the command SHALL be rejected with FAILURE
     */
    /* earliestStartTime is optional based on the StartTimeAdjust (STA) feature AND is nullable */
    if (!(forecast.earliestStartTime.HasValue()) || !(forecast.latestEndTime.HasValue()))
    {
        /* These should have values, since this command requires STA feature and these are mandatory for that */
        ChipLogError(Zcl, "DEM: EarliestStartTime / LatestEndTime do not have values");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* Temporary variable to save keep calling .Value() on the Optional element */
    DataModel::Nullable<uint32_t> & earliestStartTimeNullable = forecast.earliestStartTime.Value();
    /* Latest End Time is optional & cannot be null - unlike earliestStartTime! */
    latestEndTimeEpoch = forecast.latestEndTime.Value();

    if (earliestStartTimeNullable.IsNull())
    {
        System::Clock::Milliseconds64 cTMs;
        CHIP_ERROR err = System::SystemClock().GetClock_RealTimeMS(cTMs);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "DEM: Unable to get current time - err:%" CHIP_ERROR_FORMAT, err.Format());
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
            return;
        }

        auto unixEpoch     = std::chrono::duration_cast<System::Clock::Seconds32>(cTMs).count();
        uint32_t chipEpoch = 0;
        if (!UnixEpochToChipEpochTime(unixEpoch, chipEpoch))
        {
            ChipLogError(Zcl, "DEM: unable to convert Unix Epoch time to Matter Epoch Time");
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
            return;
        }

        /* Null means - We can start immediately */
        earliestStartTimeEpoch = chipEpoch; /* NOW */
    }
    else
    {
        earliestStartTimeEpoch = earliestStartTimeNullable.Value();
    }

    duration = forecast.endTime - forecast.startTime; // the current entire forecast duration
    if (requestedStartTimeEpoch < earliestStartTimeEpoch)
    {
        ChipLogError(Zcl, "DEM: Bad requestedStartTime %ld, earlier than earliestStartTime %ld.",
                     static_cast<long unsigned int>(requestedStartTimeEpoch),
                     static_cast<long unsigned int>(earliestStartTimeEpoch));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    if ((requestedStartTimeEpoch + duration) > latestEndTimeEpoch)
    {
        ChipLogError(Zcl, "DEM: Bad requestedStartTimeEpoch + duration %ld, later than latestEndTime %ld.",
                     static_cast<long unsigned int>(requestedStartTimeEpoch + duration),
                     static_cast<long unsigned int>(latestEndTimeEpoch));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    ChipLogProgress(Zcl, "DEM: Good requestedStartTimeEpoch %ld.", static_cast<long unsigned int>(requestedStartTimeEpoch));
    status = mDelegate.StartTimeAdjustRequest(requestedStartTimeEpoch, adjustmentCause);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: StartTimeAdjustRequest(%ld) FAILURE", static_cast<long unsigned int>(requestedStartTimeEpoch));
        return;
    }
}

void Instance::HandlePauseRequest(HandlerContext & ctx, const Commands::PauseRequest::DecodableType & commandData)
{
    Status status                                               = Status::Success;
    CHIP_ERROR err                                              = CHIP_NO_ERROR;
    DataModel::Nullable<Structs::ForecastStruct::Type> forecast = mDelegate.GetForecast();

    uint32_t duration                   = commandData.duration;
    AdjustmentCauseEnum adjustmentCause = commandData.cause;

    status = CheckOptOutAllowsRequest(adjustmentCause);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: PauseRequest command rejected");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    if (forecast.IsNull())
    {
        ChipLogError(Zcl, "DEM: Forecast is Null");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* value SHALL be between the MinPauseDuration and MaxPauseDuration indicated in the
       ActiveSlotNumber index in the Slots list in the Forecast.
    */
    uint16_t activeSlotNumber;
    if (forecast.Value().activeSlotNumber.IsNull())
    {
        ChipLogError(Zcl, "DEM: activeSlotNumber Is Null");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    activeSlotNumber = forecast.Value().activeSlotNumber.Value();
    if (activeSlotNumber >= forecast.Value().slots.size())
    {
        ChipLogError(Zcl, "DEM: Bad activeSlotNumber %d , size()=%d.", activeSlotNumber,
                     static_cast<int>(forecast.Value().slots.size()));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* We expect that there should be a slotIsPauseable entry (but it is optional) */
    if (!forecast.Value().slots[activeSlotNumber].slotIsPauseable.HasValue())
    {
        ChipLogError(Zcl, "DEM: activeSlotNumber %d does not include slotIsPauseable.", activeSlotNumber);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    if (!forecast.Value().slots[activeSlotNumber].minPauseDuration.HasValue())
    {
        ChipLogError(Zcl, "DEM: activeSlotNumber %d does not include minPauseDuration.", activeSlotNumber);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    if (!forecast.Value().slots[activeSlotNumber].maxPauseDuration.HasValue())
    {
        ChipLogError(Zcl, "DEM: activeSlotNumber %d does not include minPauseDuration.", activeSlotNumber);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    if (!forecast.Value().slots[activeSlotNumber].slotIsPauseable.Value())
    {
        ChipLogError(Zcl, "DEM: activeSlotNumber %d is NOT pauseable.", activeSlotNumber);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    if ((duration < forecast.Value().slots[activeSlotNumber].minPauseDuration.Value()) &&
        (duration > forecast.Value().slots[activeSlotNumber].maxPauseDuration.Value()))
    {
        ChipLogError(Zcl, "DEM: out of range pause duration %ld", static_cast<long unsigned int>(duration));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    err = mDelegate.SetESAState(ESAStateEnum::kPaused);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "DEM: SetESAState(paused) FAILURE");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    status = mDelegate.PauseRequest(duration, adjustmentCause);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: PauseRequest(%ld) FAILURE", static_cast<long unsigned int>(duration));
        return;
    }
}

void Instance::HandleResumeRequest(HandlerContext & ctx, const Commands::ResumeRequest::DecodableType & commandData)
{
    Status status;

    if (ESAStateEnum::kPaused != mDelegate.GetESAState())
    {
        ChipLogError(Zcl, "DEM: ESAState not Paused.");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    status = mDelegate.ResumeRequest();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: ResumeRequest FAILURE");
        return;
    }
}

void Instance::HandleModifyForecastRequest(HandlerContext & ctx, const Commands::ModifyForecastRequest::DecodableType & commandData)
{
    Status status;
    DataModel::Nullable<Structs::ForecastStruct::Type> forecast;

    uint32_t forecastId                                                           = commandData.forecastId;
    DataModel::DecodableList<Structs::SlotAdjustmentStruct::Type> slotAdjustments = commandData.slotAdjustments;
    AdjustmentCauseEnum adjustmentCause                                           = commandData.cause;

    status = CheckOptOutAllowsRequest(adjustmentCause);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: ModifyForecastRequest command rejected");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    forecast = mDelegate.GetForecast();
    if (forecast.IsNull())
    {
        ChipLogError(Zcl, "DEM: Forecast is Null");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    status = mDelegate.ModifyForecastRequest(forecastId, slotAdjustments, adjustmentCause);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: ModifyForecastRequest FAILURE");
        return;
    }
}

void Instance::HandleRequestConstraintBasedForecast(HandlerContext & ctx,
                                                    const Commands::RequestConstraintBasedForecast::DecodableType & commandData)
{
    Status status;

    DataModel::DecodableList<Structs::ConstraintsStruct::DecodableType> constraints = commandData.constraints;
    AdjustmentCauseEnum adjustmentCause                                             = commandData.cause;

    status = CheckOptOutAllowsRequest(adjustmentCause);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: RequestConstraintBasedForecast command rejected");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    status = mDelegate.RequestConstraintBasedForecast(constraints, adjustmentCause);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: RequestConstraintBasedForecast FAILURE");
        return;
    }
}

void Instance::HandleCancelRequest(HandlerContext & ctx, const Commands::CancelRequest::DecodableType & commandData)
{
    Status status;

    status = mDelegate.CancelRequest();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: CancelRequest FAILURE");
        return;
    }
}

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterDeviceEnergyManagementPluginServerInitCallback() {}
