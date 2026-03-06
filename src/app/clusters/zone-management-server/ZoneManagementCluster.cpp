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
#include "ZoneManagementCluster.h"

#include <app/persistence/AttributePersistence.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/ZoneManagement/Metadata.h>
#include <app/server-cluster/DefaultServerCluster.h>
#include <algorithm>
#include <cmath>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::ZoneManagement;
using namespace chip::app::Clusters::ZoneManagement::Attributes;
using namespace chip::app::Clusters::ZoneManagement::Structs;
using namespace Protocols::InteractionModel;

namespace {

size_t CountUserDefinedZones(const std::vector<ZoneInformationStorage> & zones)
{
    return static_cast<size_t>(std::count_if(
        zones.begin(), zones.end(), [](const ZoneInformationStorage & zone) { return zone.zoneSource == ZoneSourceEnum::kUser; }));
}

} // namespace

namespace chip {
namespace app {
namespace Clusters {
namespace ZoneManagement {

ZoneManagementCluster::ZoneManagementCluster(const Context & context) :
    DefaultServerCluster({ context.endpointId, ZoneManagement::Id }),
    mClusterContext(context),
    mDelegate(context.delegate),
    mFeatures(context.features),
    mMaxUserDefinedZones(context.config.maxUserDefinedZones),
    mMaxZones(context.config.maxZones),
    mSensitivityMax(context.config.sensitivityMax),
    mTwoDCartesianMax(context.config.twoDCartesianMax)
{}

ZoneManagementCluster::~ZoneManagementCluster() = default;


CHIP_ERROR ZoneManagementCluster::Startup(ServerClusterContext & context)
{
    return Startup(context, std::nullopt);
}

CHIP_ERROR ZoneManagementCluster::Startup(ServerClusterContext & context, std::optional<uint8_t> initialSensitivity)
{
    ReturnErrorOnFailure(ValidateConfiguration());
    if (initialSensitivity.has_value())
    {
        VerifyOrReturnError(*initialSensitivity >= 1 && *initialSensitivity <= mSensitivityMax,
                            CHIP_IM_GLOBAL_STATUS(ConstraintError));
    }
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    LoadPersistentAttributes(initialSensitivity);
    return CHIP_NO_ERROR;
}

void ZoneManagementCluster::Shutdown(ClusterShutdownType shutdownType)
{
    mZones.clear();
    mTriggers.clear();
    mUserDefinedZonesCount = 0;

    DefaultServerCluster::Shutdown(shutdownType);
}

CHIP_ERROR ZoneManagementCluster::ValidateConfiguration() const
{
    if (HasFeature(Feature::kUserDefined))
    {
        VerifyOrReturnError(mMaxUserDefinedZones >= 5, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl, "ZoneManagement[ep=%d]: MaxUserDefinedZones configuration error", mPath.mEndpointId));
        VerifyOrReturnError(mMaxZones >= mMaxUserDefinedZones, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl, "ZoneManagement[ep=%d]: MaxZones configuration error", mPath.mEndpointId));
    }
    else
    {
        VerifyOrReturnError(mMaxZones >= 1, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl, "ZoneManagement[ep=%d]: MaxZones configuration error", mPath.mEndpointId));
    }

    VerifyOrReturnError(mSensitivityMax >= 2 && mSensitivityMax <= 10, CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl, "ZoneManagement[ep=%d]: SensitivityMax configuration error", mPath.mEndpointId));

    return CHIP_NO_ERROR;
}

bool ZoneManagementCluster::HasFeature(Feature feature) const
{
    return mFeatures.Has(feature);
}

void ZoneManagementCluster::LoadPersistentAttributes(std::optional<uint8_t> initialSensitivity)
{
    VerifyOrReturn(mContext != nullptr);

    mZones.clear();
    mTriggers.clear();
    mUserDefinedZonesCount = 0;

    // Load Zones
    CHIP_ERROR err = mDelegate.LoadZones(mZones);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to load zones from storage.", mPath.mEndpointId);
    }

    // Load Triggers
    err = mDelegate.LoadTriggers(mTriggers);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to load triggers from storage.", mPath.mEndpointId);
    }

    const size_t userZonesCount = CountUserDefinedZones(mZones);
    mUserDefinedZonesCount      = static_cast<uint8_t>(std::min<size_t>(userZonesCount, mMaxUserDefinedZones));

    // Load Sensitivity
    const ConcreteAttributePath sensitivityPath(mPath.mEndpointId, mPath.mClusterId, Sensitivity::Id);
    AttributePersistence persistence(mContext->attributeStorage);

    if (initialSensitivity.has_value())
    {
        mSensitivity = *initialSensitivity;
        const ByteSpan value(reinterpret_cast<const uint8_t *>(&mSensitivity), sizeof(mSensitivity));
        err = mContext->attributeStorage.WriteValue(sensitivityPath, value);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to persist configured Sensitivity", mPath.mEndpointId);
        }
    }
    else
    {
        uint8_t loadedSensitivity = mSensitivity;
        if (persistence.LoadNativeEndianValue(sensitivityPath, loadedSensitivity, mSensitivity))
        {
            if (loadedSensitivity >= 1 && loadedSensitivity <= mSensitivityMax)
            {
                mSensitivity = loadedSensitivity;
                ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Loaded Sensitivity as %u", mPath.mEndpointId, mSensitivity);
            }
            else
            {
                ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Loaded invalid Sensitivity=%u. Keeping current value=%u", mPath.mEndpointId,
                              loadedSensitivity, mSensitivity);
            }
        }
        else
        {
            ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Sensitivity not found in storage. Using %u", mPath.mEndpointId, mSensitivity);
        }
    }

    // Signal delegate that all persistent configuration attributes have been loaded.
    LogErrorOnFailure(mDelegate.PersistentAttributesLoadedCallback());
}

CHIP_ERROR ZoneManagementCluster::ReadAndEncodeZones(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & zone : mZones)
    {
        ReturnErrorOnFailure(encoder.Encode(zone));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneManagementCluster::ReadAndEncodeTriggers(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    for (const auto & trigger : mTriggers)
    {
        ReturnErrorOnFailure(encoder.Encode(trigger));
    }

    return CHIP_NO_ERROR;
}

Optional<ZoneTriggerControlStruct> ZoneManagementCluster::GetTriggerForZone(uint16_t zoneID) const
{
    auto foundTrigger = std::find_if(mTriggers.begin(), mTriggers.end(),
                                     [&](const ZoneTriggerControlStruct & zoneTrigger) { return zoneTrigger.zoneID == zoneID; });

    if (foundTrigger == mTriggers.end())
    {
        return NullOptional;
    }

    return MakeOptional(*foundTrigger);
}

CHIP_ERROR ZoneManagementCluster::SetSensitivity(uint8_t aSensitivity)
{
    if (aSensitivity < 1 || aSensitivity > mSensitivityMax)
    {
        return CHIP_IM_GLOBAL_STATUS(ConstraintError);
    }

    VerifyOrReturnValue(aSensitivity != mSensitivity, CHIP_NO_ERROR);

    mSensitivity = aSensitivity;

    if (mContext != nullptr)
    {
        const ConcreteAttributePath path(mPath.mEndpointId, mPath.mClusterId, Sensitivity::Id);
        const ByteSpan value(reinterpret_cast<const uint8_t *>(&mSensitivity), sizeof(mSensitivity));
        ReturnErrorOnFailure(mContext->attributeStorage.WriteValue(path, value));
    }

    DefaultServerCluster::NotifyAttributeChanged(Sensitivity::Id);
    return CHIP_NO_ERROR;
}

DataModel::ActionReturnStatus ZoneManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                                   AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case FeatureMap::Id:
        return encoder.Encode(mFeatures);
    case ClusterRevision::Id:
        return encoder.Encode(kRevision);
    case MaxUserDefinedZones::Id:
        return encoder.Encode(mMaxUserDefinedZones);
    case MaxZones::Id:
        return encoder.Encode(mMaxZones);
    case Zones::Id:
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR { return ReadAndEncodeZones(listEncoder); });
    case Triggers::Id:
        return encoder.EncodeList([this](const auto & listEncoder) -> CHIP_ERROR { return ReadAndEncodeTriggers(listEncoder); });
    case SensitivityMax::Id:
        return encoder.Encode(mSensitivityMax);
    case Sensitivity::Id:
        return encoder.Encode(mSensitivity);
    case TwoDCartesianMax::Id:
        return encoder.Encode(mTwoDCartesianMax);
    }

    chipDie();
}

DataModel::ActionReturnStatus ZoneManagementCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                                    AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case Sensitivity::Id: {
        uint8_t sensitivity = 0;
        ReturnErrorOnFailure(decoder.Decode(sensitivity));
        VerifyOrReturnError(sensitivity >= 1 && sensitivity <= mSensitivityMax, Status::ConstraintError);
        VerifyOrReturnError(mContext != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnValue(sensitivity != mSensitivity, DataModel::ActionReturnStatus::FixedStatus::kWriteSuccessNoOp);

        const ConcreteAttributePath path(request.path.mEndpointId, request.path.mClusterId, request.path.mAttributeId);
        CHIP_ERROR err = mContext->attributeStorage.WriteValue(
            path, ByteSpan(reinterpret_cast<const uint8_t *>(&sensitivity), sizeof(sensitivity)));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DataManagement, "ZoneManagement[ep=%d]: Failed to persist sensitivity: %" CHIP_ERROR_FORMAT, mPath.mEndpointId,
                         err.Format());
            return err;
        }

        mSensitivity = sensitivity;
        mDelegate.OnAttributeChanged(request.path.mAttributeId);

        NotifyAttributeChanged(request.path.mAttributeId);
        return Status::Success;
    }
    }

    chipDie();
}

CHIP_ERROR ZoneManagementCluster::Attributes(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    const AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { HasFeature(Feature::kUserDefined), MaxUserDefinedZones::kMetadataEntry },
        { !HasFeature(Feature::kPerZoneSensitivity), Sensitivity::kMetadataEntry },
        { HasFeature(Feature::kTwoDimensionalCartesianZone), TwoDCartesianMax::kMetadataEntry },
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), Span(optionalAttributes));
}

CHIP_ERROR ZoneManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    if (HasFeature(Feature::kTwoDimensionalCartesianZone) && HasFeature(Feature::kUserDefined))
    {
        ReturnErrorOnFailure(
            builder.AppendElements({ Commands::CreateTwoDCartesianZone::kMetadataEntry, Commands::UpdateTwoDCartesianZone::kMetadataEntry }));
    }

    if (HasFeature(Feature::kUserDefined))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::RemoveZone::kMetadataEntry }));
    }

    ReturnErrorOnFailure(
        builder.AppendElements({ Commands::CreateOrUpdateTrigger::kMetadataEntry, Commands::RemoveTrigger::kMetadataEntry }));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneManagementCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    if (HasFeature(Feature::kTwoDimensionalCartesianZone) && HasFeature(Feature::kUserDefined))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::CreateTwoDCartesianZoneResponse::Id }));
    }

    return CHIP_NO_ERROR;
}

std::optional<DataModel::ActionReturnStatus> ZoneManagementCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                                  TLV::TLVReader & input_arguments,
                                                                                  CommandHandler * handler)
{
    // Feature-gated command support is enforced by AcceptedCommands() during IM
    // pre-dispatch validation (ValidateCommandCanBeDispatched).
    switch (request.path.mCommandId)
    {
    case Commands::CreateTwoDCartesianZone::Id: {
        Commands::CreateTwoDCartesianZone::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleCreateTwoDCartesianZone(request.path, handler, commandData);
    }
    case Commands::UpdateTwoDCartesianZone::Id: {
        Commands::UpdateTwoDCartesianZone::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleUpdateTwoDCartesianZone(commandData);
    }
    case Commands::RemoveZone::Id: {
        Commands::RemoveZone::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleRemoveZone(commandData);
    }
    case Commands::CreateOrUpdateTrigger::Id: {
        Commands::CreateOrUpdateTrigger::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleCreateOrUpdateTrigger(commandData);
    }
    case Commands::RemoveTrigger::Id: {
        Commands::RemoveTrigger::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandleRemoveTrigger(commandData);
    }
    }

    chipDie();
}

CHIP_ERROR ZoneManagementCluster::AddZone(const ZoneInformationStorage & zone)
{
    mZones.push_back(zone);
    NotifyAttributeChanged(Attributes::Zones::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneManagementCluster::UpdateZone(uint16_t zoneId, const ZoneInformationStorage & zoneInfo)
{
    auto it =
        std::find_if(mZones.begin(), mZones.end(), [zoneId](const ZoneInformationStorage & zone) { return zone.zoneID == zoneId; });

    if (it == mZones.end())
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    *it = zoneInfo;
    NotifyAttributeChanged(Attributes::Zones::Id);
    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneManagementCluster::RemoveZone(uint16_t zoneId)
{
    auto it =
        std::remove_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStorage & zone) { return zone.zoneID == zoneId; });

    if (it == mZones.end())
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    mZones.erase(it, mZones.end());
    NotifyAttributeChanged(Attributes::Zones::Id);
    return CHIP_NO_ERROR;
}

Status ZoneManagementCluster::AddOrUpdateTrigger(const ZoneTriggerControlStruct & trigger)
{
    auto foundTrigger = std::find_if(mTriggers.begin(), mTriggers.end(), [&](const ZoneTriggerControlStruct & zoneTrigger) {
        return zoneTrigger.zoneID == trigger.zoneID;
    });

    Status status = Status::Failure;
    if (foundTrigger == mTriggers.end())
    {
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
            *foundTrigger = trigger;
        }
    }

    NotifyAttributeChangedIfSuccess(Attributes::Triggers::Id, status);

    return status;
}

Status ZoneManagementCluster::RemoveTrigger(uint16_t zoneId)
{
    Status status = mDelegate.RemoveTrigger(zoneId);
    if (status == Status::Success)
    {
        mTriggers.erase(std::remove_if(mTriggers.begin(), mTriggers.end(),
                                       [&](const ZoneTriggerControlStruct & trigger) { return trigger.zoneID == zoneId; }),
                        mTriggers.end());
        NotifyAttributeChanged(Attributes::Triggers::Id);
    }

    return status;
}

Status ZoneManagementCluster::ValidateTwoDCartesianZone(const TwoDCartesianZoneDecodableStruct & zone)
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

std::optional<DataModel::ActionReturnStatus>
ZoneManagementCluster::HandleCreateTwoDCartesianZone(const ConcreteCommandPath & requestPath, CommandHandler * handler,
                                                     const Commands::CreateTwoDCartesianZone::DecodableType & commandData)
{
    uint16_t zoneID           = 0;
    const auto & zoneToCreate = commandData.zone;

    Status status = ValidateTwoDCartesianZone(zoneToCreate);
    VerifyOrReturnValue(status == Status::Success, status);

    VerifyOrReturnValue(mUserDefinedZonesCount < mMaxUserDefinedZones, Status::ResourceExhausted);

    std::vector<TwoDCartesianVertexStruct> twoDCartVertices;

    auto iter = zoneToCreate.vertices.begin();
    while (iter.Next())
    {
        auto vertex = iter.GetValue();
        if (vertex.x > mTwoDCartesianMax.x || vertex.y > mTwoDCartesianMax.y)
        {
            return Status::DynamicConstraintError;
        }
        twoDCartVertices.push_back(vertex);
    }

    CHIP_ERROR err = iter.GetStatus();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "CreateTwoDCartesianZone: Vertices list error: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::InvalidCommand;
    }

    if (ZoneAlreadyExists(zoneToCreate.use, twoDCartVertices, DataModel::NullNullable))
    {
        return Status::AlreadyExists;
    }

    if (ZoneGeometry::IsZoneSelfIntersecting(twoDCartVertices))
    {
        ChipLogError(Zcl, "CreateTwoDCartesianZone: Found self-intersecting polygon vertices for zone");
        return Status::DynamicConstraintError;
    }

    TwoDCartesianZoneStorage twoDCartZoneStorage;
    twoDCartZoneStorage.Set(zoneToCreate.name, zoneToCreate.use, twoDCartVertices, zoneToCreate.color);

    status = mDelegate.CreateTwoDCartesianZone(twoDCartZoneStorage, zoneID);
    VerifyOrReturnValue(status == Status::Success, status);

    ZoneInformationStorage zoneInfo;
    zoneInfo.Set(zoneID, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(twoDCartZoneStorage));
    ReturnErrorOnFailure(AddZone(zoneInfo));
    mUserDefinedZonesCount++;

    Commands::CreateTwoDCartesianZoneResponse::Type response;
    response.zoneID = zoneID;
    handler->AddResponse(requestPath, response);
    return std::nullopt;
}

DataModel::ActionReturnStatus
ZoneManagementCluster::HandleUpdateTwoDCartesianZone(const Commands::UpdateTwoDCartesianZone::DecodableType & commandData)
{
    const uint16_t zoneID     = commandData.zoneID;
    const auto & zoneToUpdate = commandData.zone;

    Status status = ValidateTwoDCartesianZone(zoneToUpdate);
    VerifyOrReturnValue(status == Status::Success, status);

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStruct & z) { return z.zoneID == zoneID; });
    VerifyOrReturnValue(foundZone != mZones.end(), Status::NotFound,
                        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mPath.mEndpointId, zoneID));

    VerifyOrReturnValue(foundZone->zoneSource != ZoneSourceEnum::kMfg, Status::ConstraintError);
    VerifyOrReturnValue(foundZone->zoneType == ZoneTypeEnum::kTwoDCARTZone, Status::DynamicConstraintError);

    std::vector<TwoDCartesianVertexStruct> twoDCartVertices;

    auto iter = zoneToUpdate.vertices.begin();
    while (iter.Next())
    {
        auto vertex = iter.GetValue();
        if (vertex.x > mTwoDCartesianMax.x || vertex.y > mTwoDCartesianMax.y)
        {
            return Status::DynamicConstraintError;
        }
        twoDCartVertices.push_back(vertex);
    }

    CHIP_ERROR err = iter.GetStatus();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "UpdateTwoDCartesianZone: Vertices list error: %" CHIP_ERROR_FORMAT, err.Format());
        return Status::InvalidCommand;
    }

    if (ZoneAlreadyExists(zoneToUpdate.use, twoDCartVertices, DataModel::MakeNullable(zoneID)))
    {
        return Status::AlreadyExists;
    }

    if (ZoneGeometry::IsZoneSelfIntersecting(twoDCartVertices))
    {
        ChipLogError(Zcl, "UpdateTwoDCartesianZone: Found self-intersecting polygon vertices for zone");
        return Status::DynamicConstraintError;
    }

    TwoDCartesianZoneStorage twoDCartZoneStorage;
    twoDCartZoneStorage.Set(zoneToUpdate.name, zoneToUpdate.use, twoDCartVertices, zoneToUpdate.color);

    status = mDelegate.UpdateTwoDCartesianZone(zoneID, twoDCartZoneStorage);
    VerifyOrReturnValue(status == Status::Success, status);

    ZoneInformationStorage zoneInfo;
    zoneInfo.Set(zoneID, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(twoDCartZoneStorage));
    ReturnErrorOnFailure(UpdateZone(zoneID, zoneInfo));

    return Status::Success;
}

DataModel::ActionReturnStatus ZoneManagementCluster::HandleRemoveZone(const Commands::RemoveZone::DecodableType & commandData)
{
    const uint16_t zoneID = commandData.zoneID;

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStruct & z) { return z.zoneID == zoneID; });
    VerifyOrReturnValue(foundZone != mZones.end(), Status::NotFound,
                        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mPath.mEndpointId, zoneID));

    VerifyOrReturnValue(foundZone->zoneSource == ZoneSourceEnum::kUser, Status::ConstraintError);

    auto foundTrigger =
        std::find_if(mTriggers.begin(), mTriggers.end(), [&](const ZoneTriggerControlStruct & t) { return t.zoneID == zoneID; });
    VerifyOrReturnValue(foundTrigger == mTriggers.end(), Status::InvalidInState,
                        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: Zone id %d is referenced in zone triggers", mPath.mEndpointId, zoneID));

    Status status = mDelegate.RemoveZone(zoneID);
    VerifyOrReturnValue(status == Status::Success, status);

    ReturnErrorOnFailure(RemoveZone(zoneID));
    if (mUserDefinedZonesCount > 0)
    {
        mUserDefinedZonesCount--;
    }

    return Status::Success;
}

Status ZoneManagementCluster::ValidateTrigger(const ZoneTriggerControlStruct & trigger)
{
    VerifyOrReturnError(trigger.initialDuration >= 1, Status::ConstraintError);
    VerifyOrReturnError(trigger.augmentationDuration <= trigger.initialDuration, Status::ConstraintError);
    VerifyOrReturnError(trigger.maxDuration >= trigger.initialDuration, Status::ConstraintError);

    if (trigger.sensitivity.HasValue())
    {
        VerifyOrReturnError(trigger.sensitivity.Value() >= 1 && trigger.sensitivity.Value() <= mSensitivityMax,
                            Status::ConstraintError);
    }

    return Status::Success;
}

DataModel::ActionReturnStatus
ZoneManagementCluster::HandleCreateOrUpdateTrigger(const Commands::CreateOrUpdateTrigger::DecodableType & commandData)
{
    ZoneTriggerControlStruct trigger = commandData.trigger;

    Status status = ValidateTrigger(trigger);
    VerifyOrReturnValue(status == Status::Success, status);

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStorage & z) { return z.zoneID == trigger.zoneID; });
    VerifyOrReturnValue(foundZone != mZones.end(), Status::NotFound,
                        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mPath.mEndpointId, trigger.zoneID));

    VerifyOrReturnValue(!foundZone->twoDCartZoneStorage.HasValue() ||
                            foundZone->twoDCartZoneStorage.Value().use == ZoneUseEnum::kMotion,
                        Status::ConstraintError);

    return AddOrUpdateTrigger(trigger);
}

DataModel::ActionReturnStatus ZoneManagementCluster::HandleRemoveTrigger(const Commands::RemoveTrigger::DecodableType & commandData)
{
    const uint16_t zoneID = commandData.zoneID;

    auto foundTrigger =
        std::find_if(mTriggers.begin(), mTriggers.end(), [&](const ZoneTriggerControlStruct & t) { return t.zoneID == zoneID; });
    VerifyOrReturnValue(foundTrigger != mTriggers.end(), Status::NotFound,
                        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: Trigger for zone id %d does not exist", mPath.mEndpointId, zoneID));

    return RemoveTrigger(zoneID);
}

bool ZoneManagementCluster::DoZoneUseAndVerticesMatch(ZoneUseEnum use, const std::vector<TwoDCartesianVertexStruct> & vertices,
                                                      const TwoDCartesianZoneStorage & zone)
{
    if (use != zone.use)
    {
        return false;
    }

    const size_t inputVerticesCount = vertices.size();
    const size_t zoneVerticesCount  = zone.vertices.size();

    if (inputVerticesCount != zoneVerticesCount)
    {
        return false;
    }

    return std::equal(vertices.begin(), vertices.end(), zone.verticesVector.begin(),
                      [](const TwoDCartesianVertexStruct & v1, const TwoDCartesianVertexStruct & v2) {
                          return ZoneGeometry::AreTwoDCartVerticesEqual(v1, v2);
                      });
}

bool ZoneManagementCluster::ZoneAlreadyExists(ZoneUseEnum zoneUse, const std::vector<TwoDCartesianVertexStruct> & vertices,
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

Status ZoneManagementCluster::GenerateZoneTriggeredEvent(uint16_t zoneID, ZoneEventTriggeredReasonEnum triggerReason)
{
    VerifyOrReturnValue(mContext != nullptr, Status::Failure);

    Events::ZoneTriggered::Type event;
    event.zone   = zoneID;
    event.reason = triggerReason;

    if (!mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId).has_value())
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate ZoneTriggered event", mPath.mEndpointId);
        return Status::Failure;
    }

    return Status::Success;
}

Status ZoneManagementCluster::GenerateZoneStoppedEvent(uint16_t zoneID, ZoneEventStoppedReasonEnum stopReason)
{
    VerifyOrReturnValue(mContext != nullptr, Status::Failure);

    Events::ZoneStopped::Type event;
    event.zone   = zoneID;
    event.reason = stopReason;

    if (!mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId).has_value())
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate ZoneStopped event", mPath.mEndpointId);
        return Status::Failure;
    }

    return Status::Success;
}

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip

void MatterZoneManagementPluginServerInitCallback()
{
    ChipLogProgress(Zcl, "Initializing Zone Management cluster.");
}

void MatterZoneManagementPluginServerShutdownCallback()
{
    ChipLogProgress(Zcl, "Shutting down Zone Management cluster.");
}
