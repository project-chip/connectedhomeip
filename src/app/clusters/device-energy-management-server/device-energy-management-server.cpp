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

Instance::Instance(EndpointId aEndpointId, Delegate & aDelegate) :
    AttributeAccessInterface(MakeOptional(aEndpointId), Id), CommandHandlerInterface(MakeOptional(aEndpointId), Id),
    mDelegate(aDelegate), mEndpointId(aEndpointId)
{
    /* set the base class delegates endpointId */
    mDelegate.SetEndpointId(aEndpointId);
}

Instance::~Instance()
{
    Shutdown();
}

CHIP_ERROR Instance::Init()
{
    registerAttributeAccessOverride(this);
    InteractionModelEngine::GetInstance()->RegisterCommandHandler(this);
    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    unregisterAttributeAccessOverride(this);
    InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);
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
        return aEncoder.Encode(mDelegate.GetPowerAdjustmentCapability());
    case Forecast::Id:
        return aEncoder.Encode(mDelegate.GetForecast());
    }

    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    return CHIP_ERROR_NOT_IMPLEMENTED;    // Nothing writable in this cluster !
}

// CommandHandlerInterface
CHIP_ERROR Instance::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback, void * context)
{
    using namespace Commands;

    for (auto && id : {
             PowerAdjustRequest::Id,
             CancelPowerAdjustRequest::Id,
             StartTimeAdjustRequest::Id,
             PauseRequest::Id,
             ResumeRequest::Id,
             ModifyForecastRequest::Id,
             RequestConstraintBasedForecast::Id,
         })
    {
        if (callback(id, context) == Loop::Break)
        {
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case PowerAdjustRequest::Id:
        HandleCommand<PowerAdjustRequest::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandlePowerAdjustRequest(ctx, commandData); });
        return;
    case CancelPowerAdjustRequest::Id:
        HandleCommand<CancelPowerAdjustRequest::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleCancelPowerAdjustRequest(ctx, commandData); });
        return;
    case StartTimeAdjustRequest::Id:
        HandleCommand<StartTimeAdjustRequest::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleStartTimeAdjustRequest(ctx, commandData); });
        return;
    case PauseRequest::Id:
        HandleCommand<PauseRequest::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandlePauseRequest(ctx, commandData); });
        return;
    case ResumeRequest::Id:
        HandleCommand<ResumeRequest::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleResumeRequest(ctx, commandData); });
        return;
    case ModifyForecastRequest::Id:
        HandleCommand<ModifyForecastRequest::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleModifyForecastRequest(ctx, commandData); });
        return;
    case RequestConstraintBasedForecast::Id:
        HandleCommand<RequestConstraintBasedForecast::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleRequestConstraintBasedForecast(ctx, commandData); });
        return;
    }
}

void Instance::HandlePowerAdjustRequest(HandlerContext & ctx, const Commands::PowerAdjustRequest::DecodableType & commandData)
{
    int64_t  power       = commandData.power;
    uint32_t durationSec = commandData.duration;
    bool     validArgs   = false;
    Status   status      = Status::Success;

    auto powerAdjustmentCapability = mDelegate.GetPowerAdjustmentCapability();

    if (powerAdjustmentCapability.IsNull())
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s powerAdjustmentCapability IsNull",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    for (auto pas : powerAdjustmentCapability.Value())
    {
        if (   (power >= pas.minPower) && (durationSec >= pas.minDuration)
            && (power <= pas.maxPower) && (durationSec <= pas.maxDuration))
        {
            ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Good PowerAdjustment args",  __FUNCTION__);
            validArgs = true;
            break;
        }
    }

    if (!validArgs)
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s invalid request range",  __FUNCTION__);
        // TODO: raise event ?
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Good PowerAdjustRequest() args.",  __FUNCTION__);

    // TODO:  Generate a PowerAdjustStart Event, then begins to adjust its power. How is this done, Async?
    // When done, raise PowerAdjustEnd & ESAState set to kOnline.
    status = mDelegate.PowerAdjustRequest(power, durationSec);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Failed to PowerAdjustRequest() args.",  __FUNCTION__);
    }

    ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Partly implemented",  __FUNCTION__);
    return;
}


void Instance::HandleCancelPowerAdjustRequest(HandlerContext & ctx, const Commands::CancelPowerAdjustRequest::DecodableType & commandData)
{
    Status     status = Status::Success;
    ESAStateEnum  esaStatus;

    esaStatus = mDelegate.GetESAState();

    if (ESAStateEnum::kPowerAdjustActive != esaStatus)
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - kPowerAdjustActive != esaStatus",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* TODO:  If the command is accepted, the ESA SHALL generate an PowerAdjustEnd Event.    */

    ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Good CancelPowerAdjustRequest() args.",  __FUNCTION__);
    status = mDelegate.CancelPowerAdjustRequest();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Failed to PowerAdjustRequest() args.",  __FUNCTION__);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s partly implemented",  __FUNCTION__);
    return;
}

void Instance::HandleStartTimeAdjustRequest(HandlerContext & ctx, const Commands::StartTimeAdjustRequest::DecodableType & commandData)
{
    Status      status = Status::Success;
    uint32_t earliestStartTime = 0;
    uint32_t latestEndTime     = 0;
    Structs::ForecastStruct::Type forecast = mDelegate.GetForecast();
    uint32_t duration = forecast.endTime - forecast.startTime;      // the current entire forecast duration
    uint32_t requestedStartTime = commandData.requestedStartTime;

    if (! HasFeature(Feature::kForecastAdjustment))
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Feature kForecastAdjustment not enabled",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedCommand);
        return;
    }

    if (ESAStateEnum::kUserOptOut == mDelegate.GetESAState())
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - ESAState = kUserOptOut",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* If the RequestedStartTime value resulted in a time shift which is outside the time constraints of EarliestStartTime and
       LatestEndTime, then the command SHALL be rejected with CONSTRAINT_ERROR; otherwise the command SHALL be rejected with FAILURE
    */
    if (forecast.earliestStartTime.HasValue() && !forecast.earliestStartTime.Value().IsNull())
    {
        earliestStartTime = forecast.earliestStartTime.Value().Value();
    }
    else
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - NullOptional earliestStartTime.",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    if (forecast.latestEndTime.HasValue())
    {
        latestEndTime = forecast.latestEndTime.Value();
    }
    else
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - NullOptional latestEndTime.",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    if (   (requestedStartTime < earliestStartTime)
        || ((requestedStartTime + duration) > latestEndTime))
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - Bad requestedStartTime %d.",  __FUNCTION__, requestedStartTime);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }
    else
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - Good requestedStartTime %d.",  __FUNCTION__, requestedStartTime);

        status = mDelegate.StartTimeAdjustRequest(requestedStartTime);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        if (status != Status::Success)
        {
            ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - StartTimeAdjustRequest(%d) FAILURE",  __FUNCTION__, requestedStartTime);
            return;
        }

        forecast.startTime = requestedStartTime;
        forecast.endTime   = requestedStartTime + duration;
        mDelegate.SetForecast(forecast);
    }

    ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Partially implemented",  __FUNCTION__);
    return;
}

void Instance::HandlePauseRequest(HandlerContext & ctx, const Commands::PauseRequest::DecodableType & commandData)
{
    Status      status = Status::Success;
    CHIP_ERROR  err    = CHIP_NO_ERROR;
    Structs::ForecastStruct::Type forecast = mDelegate.GetForecast();
    uint32_t duration = commandData.duration;

    if (! HasFeature(Feature::kForecastAdjustment))
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Feature kForecastAdjustment not enabled",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedCommand);
        return;
    }

    if (ESAStateEnum::kUserOptOut == mDelegate.GetESAState())
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - ESAState = kUserOptOut",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    /* value SHALL be between the MinPauseDuration and MaxPauseDuration indicated in the
       ActiveSlotNumber index in the Slots list in the Forecast.
    */
    uint16_t activeSlotNumber;
    if (forecast.activeSlotNumber.IsNull())
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - activeSlotNumber Is Null",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    activeSlotNumber = forecast.activeSlotNumber.Value();
    if (activeSlotNumber >= forecast.slots.size())
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - Bad activeSlotNumber %d , size()=%d.",  __FUNCTION__,
                        activeSlotNumber, static_cast<int>(forecast.slots.size()));
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    if (! forecast.slots[activeSlotNumber].slotIsPauseable )
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - activeSlotNumber %d Is not pausible.",  __FUNCTION__, activeSlotNumber);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    if (   (duration < forecast.slots[activeSlotNumber].minPauseDuration)
        && (duration > forecast.slots[activeSlotNumber].maxPauseDuration))
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - out of range pause duration %d",  __FUNCTION__, duration);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    err = mDelegate.SetESAState(ESAStateEnum::kPaused);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - SetESAState(paused) FAILURE",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    status = mDelegate.PauseRequest(duration);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - mDelegate.PauseRequest(%d) FAILURE",  __FUNCTION__, duration);
        return;
    }

    return;
}

void Instance::HandleResumeRequest(HandlerContext & ctx, const Commands::ResumeRequest::DecodableType & commandData)
{
    Status      status = Status::Success;
    Structs::ForecastStruct::Type forecast = mDelegate.GetForecast();

    if (! HasFeature(Feature::kForecastAdjustment))
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s Feature kForecastAdjustment not enabled",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::UnsupportedCommand);
        return;
    }

    if (ESAStateEnum::kPaused != mDelegate.GetESAState())
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - ESAState not Paused.",  __FUNCTION__);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
        return;
    }

    status = mDelegate.ResumeRequest();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure);
    if (status != Status::Success)
    {
        ChipLogProgress(Zcl, "zzzzzzzzzzDEM: %s - mDelegate.ResumeRequest() FAILURE",  __FUNCTION__);
        return;
    }

    // TODO: The ESA SHALL also generate a Resumed Event and the ESAState SHALL be updated accordingly to reflect its current state.
    ChipLogProgress(Zcl, "DEM: %s not fully implemented",  __FUNCTION__);
    return;
}

void Instance::HandleModifyForecastRequest(HandlerContext & ctx, const Commands::ModifyForecastRequest::DecodableType & commandData)
{

}

void Instance::HandleRequestConstraintBasedForecast(HandlerContext & ctx, const Commands::RequestConstraintBasedForecast::DecodableType & commandData)
{
}

} // chip
} // app
} // Clusters
} // DeviceEnergyManagement

void emberAfDeviceEnergyManagementClusterServerInitCallback(chip::EndpointId endpoint)  {}

void MatterDeviceEnergyManagementPluginServerInitCallback() {}
