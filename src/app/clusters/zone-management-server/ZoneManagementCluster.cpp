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
#include <app/clusters/zone-management-server/ZoneManagementCluster.h>
#include <app/persistence/AttributePersistenceProviderInstance.h>
#include <app/reporting/reporting.h>
#include <app/util/util.h>
#include <clusters/ZoneManagement/Metadata.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/core/TLVReader.h>
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

// TODO: find a more reasonable value, enforce / check on save
constexpr size_t kMaxPersistedValueLengthSupported = 2048;

ZoneManagementCluster::ZoneManagementCluster(EndpointId endpointId, Delegate & aDelegate, const BitFlags<Feature> aFeatures,
                                             uint8_t aMaxUserDefinedZones, uint8_t aMaxZones, uint8_t aSensitivityMax,
                                             const TwoDCartesianVertexStruct & aTwoDCartesianMax) :
    DefaultServerCluster({ endpointId, ZoneManagement::Id }),
    mDelegate(aDelegate), mFeatures(aFeatures), mMaxUserDefinedZones(aMaxUserDefinedZones), mMaxZones(aMaxZones),
    mSensitivityMax(aSensitivityMax), mTwoDCartesianMax(aTwoDCartesianMax)
{
    mDelegate.SetZoneMgmtServer(reinterpret_cast<ZoneMgmtServer *>(this));
}

ZoneManagementCluster::~ZoneManagementCluster()
{
    mDelegate.SetZoneMgmtServer(nullptr);
}

CHIP_ERROR ZoneManagementCluster::Init()
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

    return CHIP_NO_ERROR;
}

bool ZoneManagementCluster::HasFeature(Feature feature) const
{
    return mFeatures.Has(feature);
}

void ZoneManagementCluster::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t buffer[kMaxPersistedValueLengthSupported];
    MutableByteSpan bufferSpan(buffer);

    // Load Zones
    mZones.clear();
    DataModel::DecodableList<chip::app::Clusters::ZoneManagement::Structs::ZoneInformationStruct::DecodableType> decodableZones;

    err = GetAttributePersistenceProvider()->ReadValue(ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Zones::Id),
                                                       bufferSpan);

    VerifyOrReturn((CHIP_NO_ERROR == err) || (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND),
                   ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to load zones from the KVS.", mEndpointId));

    chip::TLV::TLVReader reader;
    reader.Init(bufferSpan);
    decodableZones.Decode(reader);

    auto iter = decodableZones.begin();
    while (iter.Next())
    {
        auto & decodableZone = iter.GetValue();
        ZoneInformationStorage zoneInfo;

        Optional<TwoDCartesianZoneStorage> twoDCartZoneStorage;
        if (decodableZone.twoDCartesianZone.HasValue())
        {
            auto & decodableTwoDCartZone = decodableZone.twoDCartesianZone.Value();
            std::vector<TwoDCartesianVertexStruct> vertices;
            auto vertexIter = decodableTwoDCartZone.vertices.begin();
            while (vertexIter.Next())
            {
                vertices.push_back(vertexIter.GetValue());
            }
            if (vertexIter.GetStatus() == CHIP_NO_ERROR)
            {
                TwoDCartesianZoneStorage storage;
                storage.Set(decodableTwoDCartZone.name, decodableTwoDCartZone.use, vertices, decodableTwoDCartZone.color);
                twoDCartZoneStorage.SetValue(storage);
            }
        }

        zoneInfo.Set(decodableZone.zoneID, decodableZone.zoneType, decodableZone.zoneSource, twoDCartZoneStorage);
        mZones.push_back(zoneInfo);
    }
    if (iter.GetStatus() != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to iterate zones from the KVS.", mEndpointId);
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
    mDelegate.PersistentAttributesLoadedCallback();
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

const Optional<ZoneTriggerControlStruct> ZoneManagementCluster::GetTriggerForZone(uint16_t zoneID)
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

CHIP_ERROR ZoneManagementCluster::SetSensitivity(uint8_t aSensitivity)
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

DataModel::ActionReturnStatus ZoneManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & aRequest,
                                                                   AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "Zone Management[ep=%d]: Reading", mEndpointId);

    switch (aRequest.path.mAttributeId)
    {
    case FeatureMap::Id:
        return aEncoder.Encode(mFeatures);
    case ClusterRevision::Id:
        return aEncoder.Encode(kRevision);
    case MaxUserDefinedZones::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kUserDefined), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "ZoneManagement[ep=%d]: can not get MaxUserDefinedZones, feature is not supported", mEndpointId));
        return aEncoder.Encode(mMaxUserDefinedZones);
    case MaxZones::Id:
        return aEncoder.Encode(mMaxZones);
    case Zones::Id:
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeZones(encoder); });
    case Triggers::Id:
        return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR { return this->ReadAndEncodeTriggers(encoder); });
    case SensitivityMax::Id:
        return aEncoder.Encode(mSensitivityMax);
    case Sensitivity::Id:
        VerifyOrReturnError(!HasFeature(Feature::kPerZoneSensitivity), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl,
                                         "ZoneManagement[ep=%d]: cannot read Sensitivity, PerZoneSensitivity feature is supported",
                                         mEndpointId));
        return aEncoder.Encode(mSensitivity);
    case TwoDCartesianMax::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kTwoDimensionalCartesianZone), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "ZoneManagement[ep=%d]: cannot read TwoDCartesianMax, feature is not supported", mEndpointId));
        return aEncoder.Encode(mTwoDCartesianMax);
    }

    return DataModel::ActionReturnStatus::Success;
}

DataModel::ActionReturnStatus ZoneManagementCluster::WriteAttribute(const DataModel::WriteAttributeRequest & aRequest,
                                                                    AttributeValueDecoder & aDecoder)
{
    switch (aRequest.path.mAttributeId)
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

std::optional<DataModel::ActionReturnStatus>
ZoneManagementCluster::InvokeCommand(const DataModel::InvokeRequest & aRequest, TLV::TLVReader & aInputArgs,
                                     CommandHandler * aHandler)
{
    switch (aRequest.path.mCommandId)
    {
    case Commands::CreateTwoDCartesianZone::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Creating TwoDCartesian Zone", mEndpointId);

        VerifyOrReturnError(HasFeature(Feature::kTwoDimensionalCartesianZone) && HasFeature(Feature::kUserDefined),
                            Status::UnsupportedCommand);

        return HandleCommand<Commands::CreateTwoDCartesianZone::DecodableType>(
            aRequest, aInputArgs, aHandler,
            [this](CommandHandler * handler, const auto & commandData) { HandleCreateTwoDCartesianZone(handler, commandData); });

    case Commands::UpdateTwoDCartesianZone::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Updating TwoDCartesian Zone", mEndpointId);

        VerifyOrReturnError(HasFeature(Feature::kTwoDimensionalCartesianZone) && HasFeature(Feature::kUserDefined),
                            Status::UnsupportedCommand);

        return HandleCommand<Commands::UpdateTwoDCartesianZone::DecodableType>(
            aRequest, aInputArgs, aHandler,
            [this](CommandHandler * handler, const auto & commandData) { HandleUpdateTwoDCartesianZone(handler, commandData); });

    case Commands::RemoveZone::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Removing TwoDCartesian Zone", mEndpointId);

        VerifyOrReturnError(HasFeature(Feature::kUserDefined), Status::UnsupportedCommand);

        return HandleCommand<Commands::RemoveZone::DecodableType>(
            aRequest, aInputArgs, aHandler,
            [this](CommandHandler * handler, const auto & commandData) { HandleRemoveZone(handler, commandData); });

    case Commands::CreateOrUpdateTrigger::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Create or Update Trigger", mEndpointId);

        return HandleCommand<Commands::CreateOrUpdateTrigger::DecodableType>(
            aRequest, aInputArgs, aHandler,
            [this](CommandHandler * handler, const auto & commandData) { HandleCreateOrUpdateTrigger(handler, commandData); });

    case Commands::RemoveTrigger::Id:
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Removing Trigger", mEndpointId);

        return HandleCommand<Commands::RemoveTrigger::DecodableType>(
            aRequest, aInputArgs, aHandler,
            [this](CommandHandler * handler, const auto & commandData) { HandleRemoveTrigger(handler, commandData); });
    }
    return std::nullopt;
}

CHIP_ERROR ZoneManagementCluster::Attributes(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using namespace Attributes;
    // TODO: Add all attributes
    return CHIP_NO_ERROR;
}

void ZoneManagementCluster::PersistZones()
{
    uint8_t buffer[kMaxPersistedValueLengthSupported];
    MutableByteSpan bufferSpan(buffer);
    chip::TLV::TLVWriter writer;
    CHIP_ERROR err;
    TLV::TLVType arrayType;

    writer.Init(bufferSpan);

    err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType);
    LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving zone: %s", mEndpointId, "start");

    for (const auto & zone : mZones)
    {
        err = zone.Encode(writer, chip::TLV::AnonymousTag());
        LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving zone: %s", mEndpointId, "element");
    }
    err = writer.EndContainer(arrayType);
    LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving zone: %s", mEndpointId, "end");

    bufferSpan.reduce_size(writer.GetLengthWritten());

    err = GetAttributePersistenceProvider()->WriteValue(ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Zones::Id),
                                                        bufferSpan);
    LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving zone: %s", mEndpointId, "write");
    ChipLogProgress(Zcl, "ZoneManagement[ep=%d] persisted %zu zones, wrote %d bytes", mEndpointId, mZones.size(),
                    writer.GetLengthWritten());
}

CHIP_ERROR ZoneManagementCluster::AddZone(const ZoneInformationStorage & zone)
{
    mZones.push_back(zone);
    PersistZones();
    auto path = ConcreteAttributePath(mEndpointId, ZoneManagement::Id, Attributes::Zones::Id);
    mDelegate.OnAttributeChanged(Attributes::Zones::Id);
    MatterReportingAttributeChangeCallback(path);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ZoneManagementCluster::UpdateZone(uint16_t zoneId, const ZoneInformationStorage & zoneInfo)
{
    // Find an iterator to the item with the matching ID
    auto it =
        std::find_if(mZones.begin(), mZones.end(), [zoneId](const ZoneInformationStorage & zone) { return zone.zoneID == zoneId; });

    // If an item with the zoneID was found
    if (it != mZones.end())
    {
        *it = zoneInfo; // Replace the found item with the newItem
        PersistZones();
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

CHIP_ERROR ZoneManagementCluster::RemoveZone(uint16_t zoneId)
{
    auto it =
        std::remove_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStorage & zone) { return zone.zoneID == zoneId; });

    if (it != mZones.end())
    {
        mZones.erase(it, mZones.end());
        PersistZones();
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

Status ZoneManagementCluster::AddOrUpdateTrigger(const ZoneTriggerControlStruct & trigger)
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

Status ZoneManagementCluster::RemoveTrigger(uint16_t zoneId)
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

void ZoneManagementCluster::HandleCreateTwoDCartesianZone(CommandHandler * handler,
                                                          const Commands::CreateTwoDCartesianZone::DecodableType & commandData)
{
    Commands::CreateTwoDCartesianZoneResponse::Type response;
    uint16_t zoneID     = 0;
    auto & zoneToCreate = commandData.zone;

    Status status = ValidateTwoDCartesianZone(zoneToCreate);
    if (status != Status::Success)
    {
        handler->AddStatus(status);
        return;
    }

    VerifyOrReturn(mUserDefinedZonesCount < mMaxUserDefinedZones, handler->AddStatus(Status::ResourceExhausted));

    if (zoneToCreate.use == ZoneUseEnum::kFocus && !HasFeature(Feature::kFocusZones))
    {
        handler->AddStatus(Status::ConstraintError);
        return;
    }

    std::vector<TwoDCartesianVertexStruct> twoDCartVertices;

    auto iter = zoneToCreate.vertices.begin();
    while (iter.Next())
    {
        auto vertex = iter.GetValue();
        if (vertex.x > mTwoDCartesianMax.x || vertex.y > mTwoDCartesianMax.y)
        {
            handler->AddStatus(Status::DynamicConstraintError);
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
        handler->AddStatus(Status::InvalidCommand);
        return;
    }

    if (ZoneAlreadyExists(zoneToCreate.use, twoDCartVertices, DataModel::NullNullable))
    {
        handler->AddStatus(Status::AlreadyExists);
        return;
    }

    if (ZoneGeometry::IsZoneSelfIntersecting(twoDCartVertices))
    {
        ChipLogError(Zcl, "HandleCreateTwoDCartesianZone: Found self-intersecting polygon vertices for zone");
        handler->AddStatus(Status::DynamicConstraintError);
        return;
    }

    // Form the TwoDCartesianZoneStorage object
    TwoDCartesianZoneStorage twoDCartZoneStorage;
    twoDCartZoneStorage.Set(zoneToCreate.name, zoneToCreate.use, twoDCartVertices, zoneToCreate.color);

    // Get a new zone ID
    zoneID = GetNewZoneId();

    // Call the delegate
    status = mDelegate.CreateTwoDCartesianZone(zoneID, twoDCartZoneStorage);
    if (status != Status::Success)
    {
        handler->AddStatus(status);
        return;
    }

    ZoneInformationStorage zoneInfo;
    zoneInfo.Set(zoneID, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(twoDCartZoneStorage));

    AddZone(zoneInfo);
    mUserDefinedZonesCount++;

    response.zoneID = zoneID;
    handler->AddResponse(response);
}

void ZoneManagementCluster::HandleUpdateTwoDCartesianZone(CommandHandler * handler,
                                                          const Commands::UpdateTwoDCartesianZone::DecodableType & commandData)
{
    uint16_t zoneID     = commandData.zoneID;
    auto & zoneToUpdate = commandData.zone;

    Status status = ValidateTwoDCartesianZone(zoneToUpdate);

    if (status != Status::Success)
    {
        handler->AddStatus(status);
        return;
    }

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStruct & z) { return z.zoneID == zoneID; });
    if (foundZone == mZones.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mEndpointId, zoneID);
        handler->AddStatus(Status::NotFound);
        return;
    }

    if (foundZone->zoneSource == ZoneSourceEnum::kMfg)
    {
        handler->AddStatus(Status::ConstraintError);
        return;
    }

    if (foundZone->zoneType != ZoneTypeEnum::kTwoDCARTZone)
    {
        handler->AddStatus(Status::DynamicConstraintError);
        return;
    }

    std::vector<TwoDCartesianVertexStruct> twoDCartVertices;

    auto iter = zoneToUpdate.vertices.begin();
    while (iter.Next())
    {
        auto vertex = iter.GetValue();
        if (vertex.x > mTwoDCartesianMax.x || vertex.y > mTwoDCartesianMax.y)
        {
            handler->AddStatus(Status::DynamicConstraintError);
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
        handler->AddStatus(Status::InvalidCommand);
        return;
    }

    if (ZoneAlreadyExists(zoneToUpdate.use, twoDCartVertices, DataModel::MakeNullable(zoneID)))
    {
        handler->AddStatus(Status::AlreadyExists);
        return;
    }

    if (ZoneGeometry::IsZoneSelfIntersecting(twoDCartVertices))
    {
        ChipLogError(Zcl, "HandleUpdateTwoDCartesianZone: Found self-intersecting polygon vertices for zone");
        handler->AddStatus(Status::DynamicConstraintError);
        return;
    }

    TwoDCartesianZoneStorage twoDCartZoneStorage;
    twoDCartZoneStorage.Set(zoneToUpdate.name, zoneToUpdate.use, twoDCartVertices, zoneToUpdate.color);

    // Call the delegate
    status = mDelegate.UpdateTwoDCartesianZone(zoneID, twoDCartZoneStorage);
    if (status != Status::Success)
    {
        handler->AddStatus(status);
        return;
    }

    ZoneInformationStorage zoneInfo;
    zoneInfo.Set(zoneID, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(twoDCartZoneStorage));

    UpdateZone(zoneID, zoneInfo);

    handler->AddStatus(Status::Success);
}

void ZoneManagementCluster::HandleRemoveZone(CommandHandler * handler, const Commands::RemoveZone::DecodableType & commandData)
{
    uint16_t zoneID = commandData.zoneID;

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStruct & z) { return z.zoneID == zoneID; });
    if (foundZone == mZones.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mEndpointId, zoneID);
        handler->AddStatus(Status::NotFound);
        return;
    }

    if (foundZone->zoneSource == ZoneSourceEnum::kMfg)
    {
        handler->AddStatus(Status::ConstraintError);
        return;
    }

    auto foundTrigger =
        std::find_if(mTriggers.begin(), mTriggers.end(), [&](const ZoneTriggerControlStruct & t) { return t.zoneID == zoneID; });
    if (foundTrigger != mTriggers.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: Zone id %d is referenced in zone triggers", mEndpointId, zoneID);
        handler->AddStatus(Status::InvalidInState);
        return;
    }

    // Call the delegate
    Status status = mDelegate.RemoveZone(zoneID);

    if (status == Status::Success)
    {
        RemoveZone(zoneID);
        mUserDefinedZonesCount--;
    }

    handler->AddStatus(status);
}

Status ZoneManagementCluster::ValidateTrigger(const ZoneTriggerControlStruct & trigger)
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

void ZoneManagementCluster::HandleCreateOrUpdateTrigger(CommandHandler * handler,
                                                        const Commands::CreateOrUpdateTrigger::DecodableType & commandData)
{
    ZoneTriggerControlStruct trigger = commandData.trigger;

    Status status = ValidateTrigger(trigger);

    if (status != Status::Success)
    {
        handler->AddStatus(status);
        return;
    }

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStorage & z) { return z.zoneID == trigger.zoneID; });
    if (foundZone == mZones.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mEndpointId, trigger.zoneID);
        handler->AddStatus(Status::NotFound);
        return;
    }

    if (foundZone->twoDCartZoneStorage.HasValue() && foundZone->twoDCartZoneStorage.Value().use != ZoneUseEnum::kMotion)
    {
        handler->AddStatus(Status::ConstraintError);
        return;
    }

    status = AddOrUpdateTrigger(trigger);

    handler->AddStatus(status);
}

void ZoneManagementCluster::HandleRemoveTrigger(CommandHandler * handler, const Commands::RemoveTrigger::DecodableType & commandData)
{
    uint16_t zoneID = commandData.zoneID;

    auto foundTrigger =
        std::find_if(mTriggers.begin(), mTriggers.end(), [&](const ZoneTriggerControlStruct & t) { return t.zoneID == zoneID; });
    if (foundTrigger == mTriggers.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: Trigger for zone id %d does not exist", mEndpointId, zoneID);
        handler->AddStatus(Status::NotFound);
        return;
    }

    Status status = RemoveTrigger(zoneID);

    handler->AddStatus(status);
}

bool ZoneManagementCluster::DoZoneUseAndVerticesMatch(ZoneUseEnum use, const std::vector<TwoDCartesianVertexStruct> & vertices,
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

uint16_t ZoneManagementCluster::GetNewZoneId()
{
    // TODO: Replace with a better algo; Maybe use a std::set
    uint16_t newId = 1;
    while (true)
    {
        bool idExists = false;
        for (const auto & zone : mZones)
        {
            if (zone.zoneID == newId)
            {
                idExists = true;
                break;
            }
        }
        if (!idExists)
        {
            return newId;
        }
        newId++;
    }
}

Status ZoneManagementCluster::GenerateZoneTriggeredEvent(uint16_t zoneID, ZoneEventTriggeredReasonEnum triggerReason)
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

Status ZoneManagementCluster::GenerateZoneStoppedEvent(uint16_t zoneID, ZoneEventStoppedReasonEnum stopReason)
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
