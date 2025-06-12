/**
 *
 *    Copyright (c) 2025 Project CHIP Authors
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
 *
 */

#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/zone-management-server/zone-management-server.h>
#include <app/reporting/reporting.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/interaction_model/StatusCode.h>

#include <cmath>
#include <cstring>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;
using namespace chip::app::Clusters::ZoneManagement::Structs;
using namespace chip::app::Clusters::ZoneManagement::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

ZoneMgmtServer::ZoneMgmtServer(ZoneMgmtDelegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeatures,
                               const BitFlags<OptionalAttribute> aOptionalAttrs, uint8_t aMaxUserDefinedZones, uint8_t aMaxZones,
                               uint8_t aSensitivityMax, const TwoDCartesianVertexStruct & aTwoDCartesianMax) :
    CommandHandlerInterface(MakeOptional(aEndpointId), ZoneManagement::Id),
    AttributeAccessInterface(MakeOptional(aEndpointId), ZoneManagement::Id), mDelegate(aDelegate), mEndpointId(aEndpointId),
    mFeatures(aFeatures), mOptionalAttrs(aOptionalAttrs), mMaxUserDefinedZones(aMaxUserDefinedZones), mMaxZones(aMaxZones),
    mSensitivityMax(aSensitivityMax), mTwoDCartesianMax(aTwoDCartesianMax)
{
    mDelegate.SetZoneMgmtServer(this);
}

ZoneMgmtServer::~ZoneMgmtServer()
{
    // Explicitly set the ZoneMgmtServer pointer in the Delegate to
    // null.
    mDelegate.SetZoneMgmtServer(nullptr);

    // Unregister command handler and attribute access interfaces
    CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR ZoneMgmtServer::Init()
{
    // Perform constraint checks
    if (HasFeature(Feature::kUserDefined))
    {
        VerifyOrReturnError(mMaxUserDefinedZones >= 5, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl, "ZoneManagement[ep=%d]: MaxUserDefinedZones configuration error", mEndpointId));
    }

    VerifyOrReturnError(mMaxZones >= 1, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: MaxZones configuration error", mEndpointId));

    VerifyOrReturnError(mSensitivityMax >= 2 && mSensitivityMax <= 10, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: SensitivityMax configuration error", mEndpointId));

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    return CHIP_NO_ERROR;
}

bool ZoneMgmtServer::HasFeature(Feature feature) const
{
    return mFeatures.Has(feature);
}

bool ZoneMgmtServer::SupportsOptAttr(OptionalAttribute aOptionalAttr) const
{
    return mOptionalAttrs.Has(aOptionalAttr);
}

CHIP_ERROR ZoneMgmtServer::ReadAndEncodeZones(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & zone : mZones)
    {
        ReturnErrorOnFailure(encoder.Encode(zone));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneMgmtServer::ReadAndEncodeTriggers(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & trigger : mTriggers)
    {
        ReturnErrorOnFailure(encoder.Encode(trigger));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneMgmtServer::SetSensitivity(uint8_t aSensitivity)
{
    VerifyOrReturnValue(aSensitivity != mSensitivity, CHIP_NO_ERROR);

    if (aSensitivity < 1 || aSensitivity > mSensitivityMax)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    mDelegate.OnAttributeChanged(Attributes::Sensitivity::Id);
    MatterReportingAttributeChangeCallback(ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Sensitivity::Id));
}

// AttributeAccessInterface
CHIP_ERROR ZoneMgmtServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == ZoneManagement::Id);
    ChipLogProgress(Zcl, "Zone Management[ep=%d]: Reading", mEndpointId);

    switch (aPath.mAttributeId)
    {
    case FeatureMap::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mFeatures));
        break;
    case MaxUserDefinedZones::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kUserDefined), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "ZoneManagement[ep=%d]: can not get MaxUserDefinedZones, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mMaxUserDefinedZones));
        break;
    case MaxZones::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mMaxZones));
        break;
    case Zones::Id:
        ReturnErrorOnFailure(
            aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeZones(encoder); }));
        break;
    case Triggers::Id:
        ReturnErrorOnFailure(
            aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeTriggers(encoder); }));
        break;
    case SensitivityMax::Id:
        ReturnErrorOnFailure(aEncoder.Encode(mSensitivityMax));
        break;
    case Sensitivity::Id:
        VerifyOrReturnError(
            !HasFeature(Feature::kPerZoneSens), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "ZoneManagement[ep=%d]: can not get Sensitivity, PerZoneSens feature is supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSensitivity));
        break;
    case TwoDCartesianMax::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kTwoDCart), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "ZoneManagement[ep=%d]: can not get TwoDCartesianMax, feature is not supported", mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mTwoDCartesianMax));
        break;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneMgmtServer::Write(const ConcreteDataAttributePath & aPath, AttributeValueDecoder & aDecoder)
{
    VerifyOrDie(aPath.mClusterId == ZoneManagement::Id);

    switch (aPath.mAttributeId)
    {
    case Sensitivity::Id: {
        VerifyOrReturnError(
            !HasFeature(Feature::kPerZoneSens), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "ZoneManagement[ep=%d]: can not get Sensitivity, PerZoneSens feature is supported", mEndpointId));
        uint8_t sensitivity;
        ReturnErrorOnFailure(aDecoder.Decode(sensitivity));
        return SetSensitivity(sensitivity);
    }

    default:
        // Unknown attribute
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

// CommandHandlerInterface
void ZoneMgmtServer::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::CreateTwoDCartesianZone::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Creating TwoDCartesian Zone", mEndpointId);

        if (!HasFeature(Feature::kTwoDCart) || !HasFeature(Feature::kUserDefined))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::CreateTwoDCartesianZone::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleCreateTwoDCartesianZone(ctx, commandData); });
        }
        return;

    case Commands::UpdateTwoDCartesianZone::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Updating TwoDCartesian Zone", mEndpointId);

        if (!HasFeature(Feature::kTwoDCart) || !HasFeature(Feature::kUserDefined))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::UpdateTwoDCartesianZone::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleUpdateTwoDCartesianZone(ctx, commandData); });
        }
        return;

    case Commands::GetTwoDCartesianZone::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Get TwoDCartesian Zone", mEndpointId);

        if (!HasFeature(Feature::kTwoDCart))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::GetTwoDCartesianZone::DecodableType>(
                handlerContext,
                [this](HandlerContext & ctx, const auto & commandData) { HandleGetTwoDCartesianZone(ctx, commandData); });
        }
        return;

    case Commands::RemoveZone::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Removing TwoDCartesian Zone", mEndpointId);

        if (!HasFeature(Feature::kUserDefined))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand);
        }
        else
        {
            HandleCommand<Commands::RemoveZone::DecodableType>(
                handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleRemoveZone(ctx, commandData); });
        }
        return;

    case Commands::CreateOrUpdateTrigger::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Create or Update Trigger", mEndpointId);

        HandleCommand<Commands::CreateOrUpdateTrigger::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleCreateOrUpdateTrigger(ctx, commandData); });
        return;

    case Commands::RemoveTrigger::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Removing Trigger", mEndpointId);

        HandleCommand<Commands::RemoveTrigger::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleRemoveTrigger(ctx, commandData); });
        return;
    }
}

void ZoneMgmtServer::HandleCreateTwoDCartesianZone(HandlerContext & ctx,
                                                   const Commands::CreateTwoDCartesianZone::DecodableType & commandData)
{
    Commands::CreateTwoDCartesianZoneResponse::Type response;
    uint16_t zoneID = 0;

    auto & zone = commandData.zone;

    // Call the delegate
    Status status = mDelegate.CreateTwoDCartesianZone(zone, zoneID);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Add to the vector

    response.zoneID = zoneID;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ZoneMgmtServer::HandleUpdateTwoDCartesianZone(HandlerContext & ctx,
                                                   const Commands::UpdateTwoDCartesianZone::DecodableType & commandData)
{
    auto & zoneID = commandData.zoneID;
    auto & zone   = commandData.zone;

    // Call the delegate
    Status status = mDelegate.UpdateTwoDCartesianZone(zoneID, zone);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    // Update vector

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
}

void ZoneMgmtServer::HandleGetTwoDCartesianZone(HandlerContext & ctx,
                                                const Commands::GetTwoDCartesianZone::DecodableType & commandData)
{
    Commands::GetTwoDCartesianZoneResponse::Type response;
    auto & zoneID = commandData.zoneID;

    std::vector<TwoDCartesianZoneStruct> zones;

    // Call the delegate
    Status status = mDelegate.GetTwoDCartesianZone(zoneID, zones);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    response.zones = zones;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ZoneMgmtServer::HandleRemoveZone(HandlerContext & ctx, const Commands::RemoveZone::DecodableType & commandData)
{
    auto & zoneID = commandData.zoneID;

    // Call the delegate
    Status status = mDelegate.RemoveZone(zoneID, zones);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
}

void ZoneMgmtServer::HandleCreateOrUpdateTrigger(HandlerContext & ctx,
                                                 const Commands::CreateOrUpdateTrigger::DecodableType & commandData)
{
    auto & trigger = commandData.trigger;

    // Call the delegate
    Status status = mDelegate.CreateOrUpdateTrigger(trigger);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
}

void ZoneMgmtServer::HandleRemoveTrigger(HandlerContext & ctx, const Commands::RemoveTrigger::DecodableType & commandData)
{
    auto & zoneID = commandData.zoneID;

    // Call the delegate
    Status status = mDelegate.RemoveTrigger(zoneID);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
}

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip

/** @brief Zone Management Cluster Server Init
 *
 * Server Init
 *
 */
void MatterZoneManagementPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initializing Zone Management cluster.");
}

void MatterZoneManagementPluginServerShutdownCallback()
{
    ChipLogProgress(Zcl, "Shutting down Zone Management cluster.");
}
