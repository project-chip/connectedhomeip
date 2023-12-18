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
    }

    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED; // Nothing writable in this cluster !
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

    if (HasFeature(Feature::kForecastAdjustment))
    {
        for (auto && cmd : {
                 StartTimeAdjustRequest::Id,
                 PauseRequest::Id,
                 ResumeRequest::Id,
             })
        {
            VerifyOrExit(callback(cmd, context) == Loop::Continue, /**/);
        }
    }

    if (SupportsOptCmd(OptionalCommands::kSupportsModifyForecastRequest))
    {
        VerifyOrExit(callback(ModifyForecastRequest::Id, context) == Loop::Continue, /**/);
    }

    if (SupportsOptCmd(OptionalCommands::kSupportsRequestConstraintBasedForecast))
    {
        VerifyOrExit(callback(RequestConstraintBasedForecast::Id, context) == Loop::Continue, /**/);
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
        if (!HasFeature(Feature::kForecastAdjustment))
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
        if (!HasFeature(Feature::kForecastAdjustment))
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
        if (!HasFeature(Feature::kForecastAdjustment))
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
        if (!SupportsOptCmd(OptionalCommands::kSupportsModifyForecastRequest))
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
        if (!SupportsOptCmd(OptionalCommands::kSupportsRequestConstraintBasedForecast))
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
    }
}

void Instance::HandlePowerAdjustRequest(HandlerContext & ctx, const Commands::PowerAdjustRequest::DecodableType & commandData)
{
    int64_t power        = commandData.power;
    uint32_t durationSec = commandData.duration;
    bool validArgs       = false;
    Status status        = Status::Success;

    auto powerAdjustmentCapability = mDelegate.GetPowerAdjustmentCapability();

    if (powerAdjustmentCapability.IsNull())
    {
        ChipLogError(Zcl, "DEM: %s powerAdjustmentCapability IsNull", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    /* PowerAdjustmentCapability is a list - so iterate through checking if the command is within one of the offers */
    for (auto pas : powerAdjustmentCapability.Value())
    {
        if ((power >= pas.minPower) && (durationSec >= pas.minDuration) && (power <= pas.maxPower) &&
            (durationSec <= pas.maxDuration))
        {
            ChipLogProgress(Zcl, "DEM: %s Good PowerAdjustment args", __FUNCTION__);
            validArgs = true;
            break;
        }
    }

    if (!validArgs)
    {
        ChipLogError(Zcl, "DEM: %s invalid request range", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    ChipLogProgress(Zcl, "DEM: %s Good PowerAdjustRequest() args.", __FUNCTION__);

    /* Call the delegate to do the power adjustment */
    status = mDelegate.PowerAdjustRequest(power, durationSec);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: %s Failed to PowerAdjustRequest() args.", __FUNCTION__);
    }
    return;
}

void Instance::HandleCancelPowerAdjustRequest(HandlerContext & ctx,
                                              const Commands::CancelPowerAdjustRequest::DecodableType & commandData)
{
    Status status = Status::Success;
    ESAStateEnum esaStatus;

    /* Check that the ESA state is PowerAdjustActive */
    esaStatus = mDelegate.GetESAState();
    if (ESAStateEnum::kPowerAdjustActive != esaStatus)
    {
        ChipLogError(Zcl, "DEM: %s - kPowerAdjustActive != esaStatus", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* Call the delegate to cancel the power adjustment */
    status = mDelegate.CancelPowerAdjustRequest();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: %s Failed to CancelPowerAdjustRequest()", __FUNCTION__);
        return;
    }

    return;
}

void Instance::HandleStartTimeAdjustRequest(HandlerContext & ctx,
                                            const Commands::StartTimeAdjustRequest::DecodableType & commandData)
{
    Status status                                               = Status::Success;
    uint32_t earliestStartTime                                  = 0;
    uint32_t latestEndTime                                      = 0;
    DataModel::Nullable<Structs::ForecastStruct::Type> forecast = mDelegate.GetForecast();

    uint32_t duration;
    uint32_t requestedStartTime = commandData.requestedStartTime;

    if (forecast.IsNull())
    {
        ChipLogError(Zcl, "DEM: %s - Forecast is Null", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    if (ESAStateEnum::kUserOptOut == mDelegate.GetESAState())
    {
        ChipLogError(Zcl, "DEM: %s - ESAState = kUserOptOut", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* If the RequestedStartTime value resulted in a time shift which is
     * outside the time constraints of EarliestStartTime and
     * LatestEndTime, then the command SHALL be rejected with CONSTRAINT_ERROR;
     * otherwise the command SHALL be rejected with FAILURE
     */
    /* earliestStartTime is optional based on the FA (ForecastAdjust) feature AND can be nullable */
    if (forecast.Value().earliestStartTime.HasValue() || forecast.Value().latestEndTime.HasValue())
    {
        /* These Should not be NULL since this command requires FA feature and these are mandatory for that */
        ChipLogError(Zcl, "DEM: %s - EarliestStartTime / LatestEndTime not valid", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    chip::System::Clock::Milliseconds64 cTMs;
    auto chipError = chip::System::SystemClock().GetClock_RealTimeMS(cTMs);
    if (chipError != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "DEM: Unable to get current time - error=%d (%s)]", chipError.AsInteger(), chipError.AsString());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }
    uint32_t epochNow = std::chrono::duration_cast<chip::System::Clock::Seconds32>(cTMs).count();

    if (forecast.Value().earliestStartTime.Value().IsNull())
    {
        /* Null means - We can start immediately */
        earliestStartTime = epochNow; /* NOW */
    }
    else
    {
        earliestStartTime = forecast.Value().earliestStartTime.Value().Value();
    }
    /* Latest End Time is optional & cannot be null - unlike earliestStartTime! */
    latestEndTime = forecast.Value().latestEndTime.Value();

    duration = forecast.Value().endTime - forecast.Value().startTime; // the current entire forecast duration

    if ((requestedStartTime < earliestStartTime) || ((requestedStartTime + duration) > latestEndTime))
    {
        ChipLogError(Zcl, "DEM: %s - Bad requestedStartTime %d.", __FUNCTION__, requestedStartTime);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }
    else
    {
        ChipLogProgress(Zcl, "DEM: %s - Good requestedStartTime %d.", __FUNCTION__, requestedStartTime);

        status = mDelegate.StartTimeAdjustRequest(requestedStartTime);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        if (status != Status::Success)
        {
            ChipLogError(Zcl, "DEM: %s - StartTimeAdjustRequest(%d) FAILURE", __FUNCTION__, requestedStartTime);
            return;
        }
    }

    return;
}

void Instance::HandlePauseRequest(HandlerContext & ctx, const Commands::PauseRequest::DecodableType & commandData)
{
    Status status                                               = Status::Success;
    CHIP_ERROR err                                              = CHIP_NO_ERROR;
    DataModel::Nullable<Structs::ForecastStruct::Type> forecast = mDelegate.GetForecast();

    uint32_t duration = commandData.duration;

    if (forecast.IsNull())
    {
        ChipLogError(Zcl, "DEM: %s - Forecast is Null", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    if (ESAStateEnum::kUserOptOut == mDelegate.GetESAState())
    {
        ChipLogError(Zcl, "DEM: %s - ESAState = kUserOptOut", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* value SHALL be between the MinPauseDuration and MaxPauseDuration indicated in the
       ActiveSlotNumber index in the Slots list in the Forecast.
    */
    uint16_t activeSlotNumber;
    if (forecast.Value().activeSlotNumber.IsNull())
    {
        ChipLogError(Zcl, "DEM: %s - activeSlotNumber Is Null", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    activeSlotNumber = forecast.Value().activeSlotNumber.Value();
    if (activeSlotNumber >= forecast.Value().slots.size())
    {
        ChipLogError(Zcl, "DEM: %s - Bad activeSlotNumber %d , size()=%d.", __FUNCTION__, activeSlotNumber,
                     static_cast<int>(forecast.Value().slots.size()));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    if (!forecast.Value().slots[activeSlotNumber].slotIsPauseable)
    {
        ChipLogError(Zcl, "DEM: %s - activeSlotNumber %d is NOT pausible.", __FUNCTION__, activeSlotNumber);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    if ((duration < forecast.Value().slots[activeSlotNumber].minPauseDuration) &&
        (duration > forecast.Value().slots[activeSlotNumber].maxPauseDuration))
    {
        ChipLogError(Zcl, "DEM: %s - out of range pause duration %d", __FUNCTION__, duration);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    err = mDelegate.SetESAState(ESAStateEnum::kPaused);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "DEM: %s - SetESAState(paused) FAILURE", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* Finally - we can call the delegate to ask for a pause */
    status = mDelegate.PauseRequest(duration);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: %s - mDelegate.PauseRequest(%d) FAILURE", __FUNCTION__, duration);
        return;
    }

    return;
}

void Instance::HandleResumeRequest(HandlerContext & ctx, const Commands::ResumeRequest::DecodableType & commandData)
{
    Status status = Status::Success;

    DataModel::Nullable<Structs::ForecastStruct::Type> forecast = mDelegate.GetForecast();

    if (ESAStateEnum::kPaused != mDelegate.GetESAState())
    {
        ChipLogError(Zcl, "DEM: %s - ESAState not Paused.", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* Call the delegate to resume */
    status = mDelegate.ResumeRequest();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: %s - mDelegate.ResumeRequest() FAILURE", __FUNCTION__);
        return;
    }

    return;
}

void Instance::HandleModifyForecastRequest(HandlerContext & ctx, const Commands::ModifyForecastRequest::DecodableType & commandData)
{
    Status status       = Status::Success;
    uint32_t forecastId = commandData.forecastId;

    if (ESAStateEnum::kUserOptOut == mDelegate.GetESAState())
    {
        ChipLogError(Zcl, "DEM: %s - ESAState = kUserOptOut", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    auto forecast = mDelegate.GetForecast(); // DataModel::Nullable<Structs::ForecastStruct::Type>
    if (forecast.IsNull())
    {
        ChipLogError(Zcl, "DEM: %s - Forecast is Null", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* Call the delegate to potentially update the forecast */
    DataModel::DecodableList<Structs::SlotAdjustmentStruct::Type> slotAdjustments = commandData.slotAdjustments;
    status = mDelegate.ModifyForecastRequest(forecastId, slotAdjustments);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: %s - mDelegate.ModifyForecastRequest() FAILURE", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }
}

void Instance::HandleRequestConstraintBasedForecast(HandlerContext & ctx,
                                                    const Commands::RequestConstraintBasedForecast::DecodableType & commandData)
{
    Status status = Status::Success;

    if (ESAStateEnum::kUserOptOut == mDelegate.GetESAState())
    {
        ChipLogError(Zcl, "DEM: %s - ESAState = kUserOptOut", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* delegate requested to generate a new forecast */
    status = mDelegate.RequestConstraintBasedForecast(commandData.constraints);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "DEM: %s - mDelegate.commandData.constraints() FAILURE", __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }
}

} // namespace DeviceEnergyManagement
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterDeviceEnergyManagementPluginServerInitCallback() {}
