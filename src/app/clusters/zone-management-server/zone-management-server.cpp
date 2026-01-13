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

#include "zone-geometry.h"
#include <app/AttributeAccessInterface.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/zone-management-server/zone-management-server.h>
#include <app/reporting/reporting.h>
#include <app/util/util.h>
#include <clusters/ZoneManagement/Metadata.h>
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

ZoneMgmtServer::ZoneMgmtServer(Delegate & aDelegate, EndpointId aEndpointId, const BitFlags<Feature> aFeatures,
                               uint8_t aMaxUserDefinedZones, uint8_t aMaxZones, uint8_t aSensitivityMax,
                               const TwoDCartesianVertexStruct & aTwoDCartesianMax) :
    CommandHandlerInterface(MakeOptional(aEndpointId), ZoneManagement::Id),
    AttributeAccessInterface(MakeOptional(aEndpointId), ZoneManagement::Id), mDelegate(aDelegate), mEndpointId(aEndpointId),
    mFeatures(aFeatures), mMaxUserDefinedZones(aMaxUserDefinedZones), mMaxZones(aMaxZones), mSensitivityMax(aSensitivityMax),
    mTwoDCartesianMax(aTwoDCartesianMax)
{
    mDelegate.SetZoneMgmtServer(this);
}

ZoneMgmtServer::~ZoneMgmtServer()
{
    // Explicitly set the ZoneMgmtServer pointer in the Delegate to
    // null.
    mDelegate.SetZoneMgmtServer(nullptr);

    // Unregister command handler and attribute access interfaces
    TEMPORARY_RETURN_IGNORED CommandHandlerInterfaceRegistry::Instance().UnregisterCommandHandler(this);
    AttributeAccessInterfaceRegistry::Instance().Unregister(this);
}

CHIP_ERROR ZoneMgmtServer::Init()
{
    // Perform constraint checks
    if (HasFeature(Feature::kUserDefined))
    {
        VerifyOrReturnError(mMaxUserDefinedZones >= 5, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl, "ZoneManagement[ep=%d]: MaxUserDefinedZones configuration error", mEndpointId));
        VerifyOrReturnError(mMaxZones >= mMaxUserDefinedZones, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl, "ZoneManagement[ep=%d]: MaxZones configuration error", mEndpointId));
    }
    else
    {
        VerifyOrReturnError(mMaxZones >= 1, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl, "ZoneManagement[ep=%d]: MaxZones configuration error", mEndpointId));
    }

    VerifyOrReturnError(mSensitivityMax >= 2 && mSensitivityMax <= 10, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: SensitivityMax configuration error", mEndpointId));

    LoadPersistentAttributes();

    VerifyOrReturnError(AttributeAccessInterfaceRegistry::Instance().Register(this), CHIP_ERROR_INTERNAL);
    ReturnErrorOnFailure(CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(this));

    return CHIP_NO_ERROR;
}

bool ZoneMgmtServer::HasFeature(Feature feature) const
{
    return mFeatures.Has(feature);
}

void ZoneMgmtServer::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Load Zones
    err = mDelegate.LoadZones(mZones);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to load zones from the KVS.", mEndpointId);
    }

    // Load Triggers
    err = mDelegate.LoadTriggers(mTriggers);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to load triggers from the KVS.", mEndpointId);
    }

    // Load Sensitivity
    uint8_t sensitivity = 0;
    err                 = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Sensitivity::Id), sensitivity);
    if (err == CHIP_NO_ERROR)
    {
        mSensitivity = sensitivity;
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Loaded Sensitivity as %u", mEndpointId, mSensitivity);
    }
    else
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to load the Sensitivity from the KVS. Defaulting to %u", mEndpointId,
                      mSensitivity);
    }

    // Signal delegate that all persistent configuration attributes have been loaded.
    TEMPORARY_RETURN_IGNORED mDelegate.PersistentAttributesLoadedCallback();
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

const Optional<ZoneTriggerControlStruct> ZoneMgmtServer::GetTriggerForZone(uint16_t zoneID)
{
    auto foundTrigger = std::find_if(mTriggers.begin(), mTriggers.end(),
                                     [&](const ZoneTriggerControlStruct & zoneTrigger) { return zoneTrigger.zoneID == zoneID; });

    // If an item with the zoneID was not found
    if (foundTrigger == mTriggers.end())
    {
        return NullOptional;
    }

    return MakeOptional(*foundTrigger);
}

CHIP_ERROR ZoneMgmtServer::SetSensitivity(uint8_t aSensitivity)
{
    VerifyOrReturnValue(aSensitivity != mSensitivity, CHIP_NO_ERROR);

    if (aSensitivity < 1 || aSensitivity > mSensitivityMax)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    // Set the value
    mSensitivity = aSensitivity;

    // Persist the set value in storage
    auto path = ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Sensitivity::Id);
    ReturnErrorOnFailure(GetSafeAttributePersistenceProvider()->WriteScalarValue(path, mSensitivity));

    mDelegate.OnAttributeChanged(Attributes::Sensitivity::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
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
    case ClusterRevision::Id:
        ReturnErrorOnFailure(aEncoder.Encode(kRevision));
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
        VerifyOrReturnError(!HasFeature(Feature::kPerZoneSensitivity), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "ZoneManagement[ep=%d]: cannot read Sensitivity, PerZoneSensitivity feature is supported",
                                         mEndpointId));
        ReturnErrorOnFailure(aEncoder.Encode(mSensitivity));
        break;
    case TwoDCartesianMax::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kTwoDimensionalCartesianZone), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "ZoneManagement[ep=%d]: cannot read TwoDCartesianMax, feature is not supported", mEndpointId));
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
        VerifyOrReturnError(!HasFeature(Feature::kPerZoneSensitivity), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "ZoneManagement[ep=%d]: cannot write Sensitivity, PerZoneSensitivity feature is supported",
                                         mEndpointId));
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

        VerifyOrReturn(HasFeature(Feature::kTwoDimensionalCartesianZone) && HasFeature(Feature::kUserDefined),
                       handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand));

        HandleCommand<Commands::CreateTwoDCartesianZone::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleCreateTwoDCartesianZone(ctx, commandData); });
        return;

    case Commands::UpdateTwoDCartesianZone::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Updating TwoDCartesian Zone", mEndpointId);

        VerifyOrReturn(HasFeature(Feature::kTwoDimensionalCartesianZone) && HasFeature(Feature::kUserDefined),
                       handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand));

        HandleCommand<Commands::UpdateTwoDCartesianZone::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleUpdateTwoDCartesianZone(ctx, commandData); });
        return;

    case Commands::RemoveZone::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Removing TwoDCartesian Zone", mEndpointId);

        VerifyOrReturn(HasFeature(Feature::kUserDefined),
                       handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Status::UnsupportedCommand));

        HandleCommand<Commands::RemoveZone::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleRemoveZone(ctx, commandData); });
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

CHIP_ERROR ZoneMgmtServer::AddZone(const ZoneInformationStorage & zone)
{
    mZones.push_back(zone);
    auto path = ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Zones::Id);
    mDelegate.OnAttributeChanged(Attributes::Zones::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneMgmtServer::UpdateZone(uint16_t zoneId, const ZoneInformationStorage & zoneInfo)
{
    // Find an iterator to the item with the matching ID
    auto it =
        std::find_if(mZones.begin(), mZones.end(), [zoneId](const ZoneInformationStorage & zone) { return zone.zoneID == zoneId; });

    // If an item with the zoneID was found
    if (it != mZones.end())
    {
        *it       = zoneInfo; // Replace the found item with the newItem
        auto path = ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Zones::Id);
        mDelegate.OnAttributeChanged(Attributes::Zones::Id);
        MatterReportingAttributeChangeCallback(path);

        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_NOT_FOUND;
    }
}

CHIP_ERROR ZoneMgmtServer::RemoveZone(uint16_t zoneId)
{
    auto it =
        std::remove_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStorage & zone) { return zone.zoneID == zoneId; });

    if (it != mZones.end())
    {
        mZones.erase(it, mZones.end());
        auto path = ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Zones::Id);
        mDelegate.OnAttributeChanged(Attributes::Zones::Id);
        MatterReportingAttributeChangeCallback(path);

        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_NOT_FOUND;
    }
}

Status ZoneMgmtServer::AddOrUpdateTrigger(const ZoneTriggerControlStruct & trigger)
{
    // Find an iterator to the item with the matching ID
    auto foundTrigger = std::find_if(mTriggers.begin(), mTriggers.end(), [&](const ZoneTriggerControlStruct & zoneTrigger) {
        return zoneTrigger.zoneID == trigger.zoneID;
    });

    Status status;
    // If an item with the zoneID was not found
    if (foundTrigger == mTriggers.end())
    {
        // Call the delegate
        status = mDelegate.CreateTrigger(trigger);
        if (status == Status::Success)
        {
            mTriggers.push_back(trigger);
        }
    }
    else
    {
        status = mDelegate.UpdateTrigger(trigger);
        if (status == Status::Success)
        {
            *foundTrigger = trigger; // Replace the found item with the newItem
        }
    }

    if (status == Status::Success)
    {
        auto path = ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Triggers::Id);
        mDelegate.OnAttributeChanged(Attributes::Triggers::Id);
        MatterReportingAttributeChangeCallback(path);
    }

    return status;
}

Status ZoneMgmtServer::RemoveTrigger(uint16_t zoneId)
{
    Status status = mDelegate.RemoveTrigger(zoneId);
    if (status == Status::Success)
    {
        mTriggers.erase(std::remove_if(mTriggers.begin(), mTriggers.end(),
                                       [&](const ZoneTriggerControlStruct & trigger) { return trigger.zoneID == zoneId; }),
                        mTriggers.end());
        auto path = ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Triggers::Id);
        mDelegate.OnAttributeChanged(Attributes::Triggers::Id);
        MatterReportingAttributeChangeCallback(path);
    }

    return status;
}

Status ZoneMgmtServer::ValidateTwoDCartesianZone(const TwoDCartesianZoneDecodableStruct & zone)
{
    VerifyOrReturnError(zone.name.size() <= 32, Status::ConstraintError);

    VerifyOrReturnError(zone.use != ZoneUseEnum::kUnknownEnumValue, Status::ConstraintError);

    size_t listCount = 0;
    VerifyOrReturnError(zone.vertices.ComputeSize(&listCount) == CHIP_NO_ERROR, Status::InvalidCommand);

    VerifyOrReturnError(listCount >= 3 && listCount <= 12, Status::ConstraintError);

    if (zone.color.HasValue())
    {
        VerifyOrReturnError(zone.color.Value().size() == 7 || zone.color.Value().size() == 9, Status::ConstraintError);
    }

    return Status::Success;
}

void ZoneMgmtServer::HandleCreateTwoDCartesianZone(HandlerContext & ctx,
                                                   const Commands::CreateTwoDCartesianZone::DecodableType & commandData)
{
    Commands::CreateTwoDCartesianZoneResponse::Type response;
    uint16_t zoneID     = 0;
    auto & zoneToCreate = commandData.zone;

    Status status = ValidateTwoDCartesianZone(zoneToCreate);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    VerifyOrReturn(mUserDefinedZonesCount < mMaxUserDefinedZones,
                   ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ResourceExhausted));

    if (zoneToCreate.use == ZoneUseEnum::kFocus && !HasFeature(Feature::kFocusZones))
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    std::vector<TwoDCartesianVertexStruct> twoDCartVertices;

    auto iter = zoneToCreate.vertices.begin();
    while (iter.Next())
    {
        auto vertex = iter.GetValue();
        if (vertex.x > mTwoDCartesianMax.x || vertex.y > mTwoDCartesianMax.y)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
            return;
        }
        // Store the vertices
        twoDCartVertices.push_back(vertex);
    }

    // Check the list validity
    CHIP_ERROR err = iter.GetStatus();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleCreateTwoDCartesianZone: Vertices list error: %" CHIP_ERROR_FORMAT, err.Format());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    if (ZoneAlreadyExists(zoneToCreate.use, twoDCartVertices, DataModel::NullNullable))
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::AlreadyExists);
        return;
    }

    if (ZoneGeometry::IsZoneSelfIntersecting(twoDCartVertices))
    {
        ChipLogError(Zcl, "HandleCreateTwoDCartesianZone: Found self-intersecting polygon vertices for zone");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
        return;
    }

    // Form the TwoDCartesianZoneStorage object
    TwoDCartesianZoneStorage twoDCartZoneStorage;
    twoDCartZoneStorage.Set(zoneToCreate.name, zoneToCreate.use, twoDCartVertices, zoneToCreate.color);

    // Call the delegate
    status = mDelegate.CreateTwoDCartesianZone(twoDCartZoneStorage, zoneID);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ZoneInformationStorage zoneInfo;
    zoneInfo.Set(zoneID, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(twoDCartZoneStorage));

    TEMPORARY_RETURN_IGNORED AddZone(zoneInfo);
    mUserDefinedZonesCount++;

    response.zoneID = zoneID;
    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void ZoneMgmtServer::HandleUpdateTwoDCartesianZone(HandlerContext & ctx,
                                                   const Commands::UpdateTwoDCartesianZone::DecodableType & commandData)
{
    uint16_t zoneID     = commandData.zoneID;
    auto & zoneToUpdate = commandData.zone;

    Status status = ValidateTwoDCartesianZone(zoneToUpdate);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStruct & z) { return z.zoneID == zoneID; });
    if (foundZone == mZones.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mEndpointId, zoneID);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (foundZone->zoneSource == ZoneSourceEnum::kMfg)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    if (foundZone->zoneType != ZoneTypeEnum::kTwoDCARTZone)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
        return;
    }

    std::vector<TwoDCartesianVertexStruct> twoDCartVertices;

    auto iter = zoneToUpdate.vertices.begin();
    while (iter.Next())
    {
        auto vertex = iter.GetValue();
        if (vertex.x > mTwoDCartesianMax.x || vertex.y > mTwoDCartesianMax.y)
        {
            ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
            return;
        }
        // Store the vertices
        twoDCartVertices.push_back(vertex);
    }

    // Check the list validity
    CHIP_ERROR err = iter.GetStatus();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "HandleUpdateTwoDCartesianZone: Vertices list error: %" CHIP_ERROR_FORMAT, err.Format());
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidCommand);
        return;
    }

    if (ZoneAlreadyExists(zoneToUpdate.use, twoDCartVertices, DataModel::MakeNullable(zoneID)))
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::AlreadyExists);
        return;
    }

    if (ZoneGeometry::IsZoneSelfIntersecting(twoDCartVertices))
    {
        ChipLogError(Zcl, "HandleUpdateTwoDCartesianZone: Found self-intersecting polygon vertices for zone");
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::DynamicConstraintError);
        return;
    }

    TwoDCartesianZoneStorage twoDCartZoneStorage;
    twoDCartZoneStorage.Set(zoneToUpdate.name, zoneToUpdate.use, twoDCartVertices, zoneToUpdate.color);

    // Call the delegate
    status = mDelegate.UpdateTwoDCartesianZone(zoneID, twoDCartZoneStorage);
    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    ZoneInformationStorage zoneInfo;
    zoneInfo.Set(zoneID, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(twoDCartZoneStorage));

    TEMPORARY_RETURN_IGNORED UpdateZone(zoneID, zoneInfo);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Success);
}

void ZoneMgmtServer::HandleRemoveZone(HandlerContext & ctx, const Commands::RemoveZone::DecodableType & commandData)
{
    uint16_t zoneID = commandData.zoneID;

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStruct & z) { return z.zoneID == zoneID; });
    if (foundZone == mZones.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mEndpointId, zoneID);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (foundZone->zoneSource == ZoneSourceEnum::kMfg)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    auto foundTrigger =
        std::find_if(mTriggers.begin(), mTriggers.end(), [&](const ZoneTriggerControlStruct & t) { return t.zoneID == zoneID; });
    if (foundTrigger != mTriggers.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: Zone id %d is referenced in zone triggers", mEndpointId, zoneID);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::InvalidInState);
        return;
    }

    // Call the delegate
    Status status = mDelegate.RemoveZone(zoneID);

    if (status == Status::Success)
    {
        TEMPORARY_RETURN_IGNORED RemoveZone(zoneID);
        mUserDefinedZonesCount--;
    }

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

Status ZoneMgmtServer::ValidateTrigger(const ZoneTriggerControlStruct & trigger)
{
    VerifyOrReturnError(trigger.initialDuration >= 1 && trigger.initialDuration <= 65535, Status::ConstraintError);

    VerifyOrReturnError(trigger.augmentationDuration <= trigger.initialDuration, Status::ConstraintError);

    VerifyOrReturnError(trigger.maxDuration >= trigger.initialDuration, Status::ConstraintError);

    // If PerZoneSensitivity feature is supported, then command should have the
    // sensitivity field in the Trigger. Or, if it is not supported, then command should
    // not have the field.
    VerifyOrReturnError((HasFeature(Feature::kPerZoneSensitivity) == trigger.sensitivity.HasValue()), Status::InvalidCommand);
    if (HasFeature(Feature::kPerZoneSensitivity))
    {
        VerifyOrReturnError(trigger.sensitivity.Value() >= 1 && trigger.sensitivity.Value() <= mSensitivityMax,
                            Status::ConstraintError);
    }

    return Status::Success;
}

void ZoneMgmtServer::HandleCreateOrUpdateTrigger(HandlerContext & ctx,
                                                 const Commands::CreateOrUpdateTrigger::DecodableType & commandData)
{
    ZoneTriggerControlStruct trigger = commandData.trigger;

    Status status = ValidateTrigger(trigger);

    if (status != Status::Success)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
        return;
    }

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStorage & z) { return z.zoneID == trigger.zoneID; });
    if (foundZone == mZones.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mEndpointId, trigger.zoneID);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    if (foundZone->twoDCartZoneStorage.HasValue() && foundZone->twoDCartZoneStorage.Value().use != ZoneUseEnum::kMotion)
    {
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::ConstraintError);
        return;
    }

    status = AddOrUpdateTrigger(trigger);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ZoneMgmtServer::HandleRemoveTrigger(HandlerContext & ctx, const Commands::RemoveTrigger::DecodableType & commandData)
{
    uint16_t zoneID = commandData.zoneID;

    auto foundTrigger =
        std::find_if(mTriggers.begin(), mTriggers.end(), [&](const ZoneTriggerControlStruct & t) { return t.zoneID == zoneID; });
    if (foundTrigger == mTriggers.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: Trigger for zone id %d does not exist", mEndpointId, zoneID);
        ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::NotFound);
        return;
    }

    Status status = RemoveTrigger(zoneID);

    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

bool ZoneMgmtServer::DoZoneUseAndVerticesMatch(ZoneUseEnum use, const std::vector<TwoDCartesianVertexStruct> & vertices,
                                               const TwoDCartesianZoneStorage & zone)
{
    if (use != zone.use)
    {
        return false;
    }

    size_t inputVerticesCount = vertices.size();
    size_t zoneVerticesCount  = zone.vertices.size();

    if (inputVerticesCount != zoneVerticesCount)
    {
        return false;
    }

    // Exact match check for vertices
    return std::equal(vertices.begin(), vertices.end(), zone.verticesVector.begin(),
                      [](const TwoDCartesianVertexStruct & v1, const TwoDCartesianVertexStruct & v2) {
                          return ZoneGeometry::AreTwoDCartVerticesEqual(v1, v2);
                      });
}

bool ZoneMgmtServer::ZoneAlreadyExists(ZoneUseEnum zoneUse, const std::vector<TwoDCartesianVertexStruct> & vertices,
                                       const DataModel::Nullable<uint16_t> & excludeZoneId)
{
    for (auto & zone : mZones)
    {
        if (!excludeZoneId.IsNull() && zone.zoneID == excludeZoneId.Value())
        {
            continue;
        }

        if (zone.twoDCartZoneStorage.HasValue() && DoZoneUseAndVerticesMatch(zoneUse, vertices, zone.twoDCartZoneStorage.Value()))
        {
            return true;
        }
    }
    return false;
}

Status ZoneMgmtServer::GenerateZoneTriggeredEvent(uint16_t zoneID, ZoneEventTriggeredReasonEnum triggerReason)
{
    Events::ZoneTriggered::Type event;
    EventNumber eventNumber;

    event.zone   = zoneID;
    event.reason = triggerReason;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate ZoneTriggered event: %" CHIP_ERROR_FORMAT, mEndpointId,
                     err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

Status ZoneMgmtServer::GenerateZoneStoppedEvent(uint16_t zoneID, ZoneEventStoppedReasonEnum stopReason)
{
    Events::ZoneStopped::Type event;
    EventNumber eventNumber;

    event.zone   = zoneID;
    event.reason = stopReason;

    CHIP_ERROR err = LogEvent(event, mEndpointId, eventNumber);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate ZoneTriggered event: %" CHIP_ERROR_FORMAT, mEndpointId,
                     err.Format());
        return Status::Failure;
    }
    return Status::Success;
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
