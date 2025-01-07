/*
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "water-heater-management-server.h"

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/util/attribute-storage.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::WaterHeaterManagement;
using namespace chip::app::Clusters::WaterHeaterManagement::Attributes;

using chip::Protocols::InteractionModel::Status;

namespace chip {
namespace app {
namespace Clusters {
namespace WaterHeaterManagement {

constexpr uint16_t kClusterRevision = 2;

/***************************************************************************
 *
 * The Delegate implementation
 *
 ***************************************************************************/

CHIP_ERROR Delegate::GenerateBoostStartedEvent(uint32_t durationSecs, Optional<bool> oneShot, Optional<bool> emergencyBoost,
                                               Optional<int16_t> temporarySetpoint, Optional<Percent> targetPercentage,
                                               Optional<Percent> targetReheat)
{
    Events::BoostStarted::Type event;
    EventNumber eventNumber;

    event.boostInfo.duration          = durationSecs;
    event.boostInfo.oneShot           = oneShot;
    event.boostInfo.emergencyBoost    = emergencyBoost;
    event.boostInfo.temporarySetpoint = temporarySetpoint;
    event.boostInfo.targetPercentage  = targetPercentage;
    event.boostInfo.targetReheat      = targetReheat;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to generate BoostStarted event: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return err;
}

CHIP_ERROR Delegate::GenerateBoostEndedEvent()
{
    Events::BoostEnded::Type event;
    EventNumber eventNumber;
    ChipLogError(AppServer, "Delegate::GenerateBoostEndedEvent");

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Unable to generate BoostEnded event: %" CHIP_ERROR_FORMAT, err.Format());
        return err;
    }

    return err;
}

/***************************************************************************
 *
 * The Instance implementation
 *
 ***************************************************************************/

CHIP_ERROR Instance::Init()
{
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));
    VerifyOrReturnError(chip::app::AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void Instance::Shutdown()
{
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    chip::app::AttributeAccessInterfaceRegistry::Instance().Unregister(this);
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
    case HeaterTypes::Id:
        return aEncoder.Encode(mDelegate.GetHeaterTypes());
    case HeatDemand::Id:
        return aEncoder.Encode(mDelegate.GetHeatDemand());
    case TankVolume::Id:
        if (!HasFeature(Feature::kEnergyManagement))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetTankVolume());
    case EstimatedHeatRequired::Id:
        if (!HasFeature(Feature::kEnergyManagement))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetEstimatedHeatRequired());
    case TankPercentage::Id:
        if (!HasFeature(Feature::kTankPercent))
        {
            return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
        }
        return aEncoder.Encode(mDelegate.GetTankPercentage());
    case BoostState::Id:
        return aEncoder.Encode(mDelegate.GetBoostState());

    /* FeatureMap - is held locally */
    case FeatureMap::Id:
        return aEncoder.Encode(mFeature);
    case ClusterRevision::Id:
        return aEncoder.Encode(kClusterRevision);
    }

    /* Allow all other unhandled attributes to fall through to Ember */
    return CHIP_NO_ERROR;
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
{
    using namespace Commands;

    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Boost::Id:
        HandleCommand<Boost::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleBoost(ctx, commandData); });
        return;
    case CancelBoost::Id:
        HandleCommand<CancelBoost::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleCancelBoost(ctx, commandData); });
        return;
    }
}

void Instance::HandleBoost(HandlerContext & ctx, const Commands::Boost::DecodableType & commandData)
{
    uint32_t duration                   = commandData.boostInfo.duration;
    Optional<bool> oneShot              = commandData.boostInfo.oneShot;
    Optional<bool> emergencyBoost       = commandData.boostInfo.emergencyBoost;
    Optional<int16_t> temporarySetpoint = commandData.boostInfo.temporarySetpoint;
    Optional<Percent> targetPercentage  = commandData.boostInfo.targetPercentage;
    Optional<Percent> targetReheat      = commandData.boostInfo.targetReheat;

    //  Notify the appliance if the appliance hardware cannot be adjusted, then return Failure
    if (HasFeature(WaterHeaterManagement::Feature::kTankPercent))
    {
        if (targetPercentage.HasValue())
        {
            if (targetPercentage.Value() > 100)
            {
                ChipLogError(Zcl, "Bad targetPercentage %u", targetPercentage.Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
                return;
            }
        }

        if (targetReheat.HasValue())
        {
            if (targetReheat.Value() > 100)
            {
                ChipLogError(Zcl, "Bad targetReheat %u", targetReheat.Value());
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
                return;
            }

            if (!targetPercentage.HasValue())
            {
                ChipLogError(Zcl, "targetPercentage must be specified if targetReheat specified");
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
                return;
            }

            if (oneShot.HasValue())
            {
                ChipLogError(Zcl, "Cannot specify targetReheat with targetPercentage and oneShot. oneShot must be excluded");
                ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
                return;
            }
        }
    }
    else if (targetPercentage.HasValue() || targetReheat.HasValue())
    {
        ChipLogError(Zcl, "Cannot specify targetPercentage or targetReheat if the feature TankPercent is not supported");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    Status status = mDelegate.HandleBoost(duration, oneShot, emergencyBoost, temporarySetpoint, targetPercentage, targetReheat);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "WHM: Boost command failed. status " ChipLogFormatIMStatus, ChipLogValueIMStatus(status));
    }
}

void Instance::HandleCancelBoost(HandlerContext & ctx, const Commands::CancelBoost::DecodableType & commandData)
{
    Status status = mDelegate.HandleCancelBoost();
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
    if (status != Status::Success)
    {
        ChipLogError(Zcl, "WHM: CancelBoost command failed. status " ChipLogFormatIMStatus, ChipLogValueIMStatus(status));
        return;
    }
}

} // namespace WaterHeaterManagement
} // namespace Clusters
} // namespace app
} // namespace chip
