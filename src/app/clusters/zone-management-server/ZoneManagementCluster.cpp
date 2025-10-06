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
#include <app/persistence/AttributePersistence.h>
#include <app/persistence/AttributePersistenceProvider.h>
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
// TODO: canonical layout is for the cluster to live in the Clusters namespace, without the additional namespace. Remove the
// namespace here and fix the pattern in other clusters
namespace ZoneManagement {
// TODO: find a more reasonable value, enforce / check on save
constexpr size_t kMaxPersistedValueLengthSupported = 2048;

ZoneManagementCluster::ZoneManagementCluster(EndpointId endpointId, Delegate & aDelegate, const BitFlags<Feature> aFeatures,
                                             uint8_t aMaxUserDefinedZones, uint8_t aMaxZones, uint8_t aSensitivityMax, uint8_t aSensitivity, 
                                             const TwoDCartesianVertexStruct & aTwoDCartesianMax) :
    DefaultServerCluster({ endpointId, ZoneManagement::Id }),
    mDelegate(aDelegate), mFeatures(aFeatures), mMaxUserDefinedZones(aMaxUserDefinedZones), mMaxZones(aMaxZones),
    mSensitivityMax(aSensitivityMax), mSensitivity(aSensitivity), mTwoDCartesianMax(aTwoDCartesianMax)
{
    mDelegate.SetZoneMgmtServer(this);
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

CHIP_ERROR ZoneManagementCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    ReturnErrorOnFailure(LoadZones());
    ReturnErrorOnFailure(LoadTriggers());
    ReturnErrorOnFailure(LoadSensitivity());
    // Signal delegate that all persistent configuration attributes have been loaded.
    mDelegate.PersistentAttributesLoadedCallback();
    return CHIP_NO_ERROR;
}

bool ZoneManagementCluster::HasFeature(Feature feature) const
{
    return mFeatures.Has(feature);
}

CHIP_ERROR ZoneManagementCluster::LoadZones()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t buffer[kMaxPersistedValueLengthSupported];
    MutableByteSpan bufferSpan(buffer);

    // Load Zones
    mZones.clear();

    err = mContext->attributeStorage.ReadValue(ConcreteAttributePath(mPath.mEndpointId, ZoneManagement::Id, Attributes::Zones::Id),
                                               bufferSpan);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: No persistent info found. Continuing.", mPath.mEndpointId);
        return CHIP_NO_ERROR;
    }
    VerifyOrReturnError(CHIP_NO_ERROR == err, err,
                        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to load zones from the KVS.", mPath.mEndpointId));

    chip::TLV::TLVReader reader;
    chip::TLV::TLVType type;
    reader.Init(bufferSpan);
    reader.Next();
    VerifyOrReturnError(reader.GetType() == chip::TLV::kTLVType_Array, CHIP_ERROR_SCHEMA_MISMATCH);
    reader.EnterContainer(type);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        chip::app::Clusters::ZoneManagement::Structs::ZoneInformationStruct::DecodableType decodableZone;
        decodableZone.Decode(reader);

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
    if (err == CHIP_END_OF_TLV)
    {
        return CHIP_NO_ERROR;
    }
    else
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to iterate zones from the KVS.", mPath.mEndpointId);
        return err;
    }
}

CHIP_ERROR ZoneManagementCluster::LoadTriggers()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t buffer[kMaxPersistedValueLengthSupported];
    MutableByteSpan bufferSpan(buffer);

    // Load Triggers
    mTriggers.clear();

    err = mContext->attributeStorage.ReadValue(
        ConcreteAttributePath(mPath.mEndpointId, ZoneManagement::Id, Attributes::Triggers::Id), bufferSpan);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: No persistent info found. Continuing.", mPath.mEndpointId);
        return CHIP_NO_ERROR;
    }
    VerifyOrReturnError(CHIP_NO_ERROR == err, err,
                        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to load triggers from the KVS.", mPath.mEndpointId));

    chip::TLV::TLVReader reader;
    chip::TLV::TLVType type;
    reader.Init(bufferSpan);
    reader.Next();
    VerifyOrReturnError(reader.GetType() == chip::TLV::kTLVType_Array, CHIP_ERROR_SCHEMA_MISMATCH);
    reader.EnterContainer(type);

    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        chip::app::Clusters::ZoneManagement::Structs::ZoneTriggerControlStruct::DecodableType decodableTrigger;
        decodableTrigger.Decode(reader);
        mTriggers.push_back(decodableTrigger);
    }
    if (err == CHIP_END_OF_TLV)
    {
        return CHIP_NO_ERROR;
    }
    else
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to iterate triggers from the KVS.", mPath.mEndpointId);
        return err;
    }
}

CHIP_ERROR ZoneManagementCluster::LoadSensitivity()
{
    uint8_t sensitivity = 0;
    AttributePersistence persistence(mContext->attributeStorage);

    if (persistence.LoadNativeEndianValue<uint8_t>({ mPath.mEndpointId, ZoneManagement::Id, Attributes::Sensitivity::Id },
                                                   sensitivity, static_cast<uint8_t>(0)))

    {
        mSensitivity = sensitivity;
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Loaded Sensitivity as %u", mPath.mEndpointId, mSensitivity);
    }
    else
    {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Unable to load the Sensitivity from the KVS. Defaulting to %u",
                      mPath.mEndpointId, mSensitivity);
    }
    return CHIP_NO_ERROR;
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

DataModel::ActionReturnStatus ZoneManagementCluster::ReadAttribute(const DataModel::ReadAttributeRequest & aRequest,
                                                                   AttributeValueEncoder & aEncoder)
{
    ChipLogProgress(Zcl, "Zone Management[ep=%d]: Reading", mPath.mEndpointId);

    switch (aRequest.path.mAttributeId)
    {
    case FeatureMap::Id:
        return aEncoder.Encode(mFeatures);
    case ClusterRevision::Id:
        return aEncoder.Encode(kRevision);
    case MaxUserDefinedZones::Id:
        VerifyOrReturnError(HasFeature(Feature::kUserDefined), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
                            ChipLogError(Zcl, "ZoneManagement[ep=%d]: can not get MaxUserDefinedZones, feature is not supported",
                                         mPath.mEndpointId));
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
                                         mPath.mEndpointId));
        return aEncoder.Encode(mSensitivity);
    case TwoDCartesianMax::Id:
        VerifyOrReturnError(
            HasFeature(Feature::kTwoDimensionalCartesianZone), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute),
            ChipLogError(Zcl, "ZoneManagement[ep=%d]: cannot read TwoDCartesianMax, feature is not supported", mPath.mEndpointId));
        return aEncoder.Encode(mTwoDCartesianMax);
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
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
                                         mPath.mEndpointId));
        uint8_t sensitivity;
        ReturnErrorOnFailure(aDecoder.Decode(sensitivity));

        VerifyOrReturnError(sensitivity >= 1 && sensitivity <= mSensitivityMax, CHIP_IM_GLOBAL_STATUS(ConstraintError));

        AttributePersistence persistence(mContext->attributeStorage);

        mSensitivity = sensitivity;

        ReturnErrorOnFailure(mContext->attributeStorage.WriteValue(
            { mPath.mEndpointId, ZoneManagement::Id, Attributes::Sensitivity::Id }, { reinterpret_cast<const uint8_t *>(&mSensitivity), sizeof(mSensitivity) }));

        mDelegate.OnAttributeChanged(Attributes::Sensitivity::Id);
        return CHIP_NO_ERROR;
    }

    default:
        // Unknown attribute
        return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
    }
}

std::optional<DataModel::ActionReturnStatus> ZoneManagementCluster::InvokeCommand(const DataModel::InvokeRequest & aRequest,
                                                                                  TLV::TLVReader & aInputArgs,
                                                                                  CommandHandler * aHandler)
{
    using namespace chip::app::Clusters::ZoneManagement::Commands;

    switch (aRequest.path.mCommandId)
    {
    case CreateTwoDCartesianZone::Id: {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Creating TwoDCartesian Zone", mPath.mEndpointId);

        VerifyOrReturnError(HasFeature(Feature::kTwoDimensionalCartesianZone) && HasFeature(Feature::kUserDefined),
                            Status::UnsupportedCommand);
        CreateTwoDCartesianZone::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        HandleCreateTwoDCartesianZone(aHandler, aRequest.path, commandData);
        return std::nullopt;
    }
    case UpdateTwoDCartesianZone::Id: {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Updating TwoDCartesian Zone", mPath.mEndpointId);

        VerifyOrReturnError(HasFeature(Feature::kTwoDimensionalCartesianZone) && HasFeature(Feature::kUserDefined),
                            Status::UnsupportedCommand);
        UpdateTwoDCartesianZone::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        HandleUpdateTwoDCartesianZone(aHandler, aRequest.path, commandData);
        return std::nullopt;
    }
    case RemoveZone::Id: {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Removing TwoDCartesian Zone", mPath.mEndpointId);

        VerifyOrReturnError(HasFeature(Feature::kUserDefined), Status::UnsupportedCommand);
        RemoveZone::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        HandleRemoveZone(aHandler, aRequest.path, commandData);
        return std::nullopt;
    }
    case CreateOrUpdateTrigger::Id: {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Create or Update Trigger", mPath.mEndpointId);
        CreateOrUpdateTrigger::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        HandleCreateOrUpdateTrigger(aHandler, aRequest.path, commandData);
        return std::nullopt;
    }
    case RemoveTrigger::Id: {
        ChipLogDetail(Zcl, "ZoneManagement[ep=%d]: Removing Trigger", mPath.mEndpointId);
        RemoveTrigger::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(aInputArgs));
        HandleRemoveTrigger(aHandler, aRequest.path, commandData);
        return std::nullopt;
    }
    }
    return std::nullopt;
}

constexpr DataModel::AttributeEntry kMandatoryAttributes[] = { MaxZones::kMetadataEntry, Zones::kMetadataEntry,
                                                               Triggers::kMetadataEntry, SensitivityMax::kMetadataEntry };

CHIP_ERROR ZoneManagementCluster::Attributes(const ConcreteClusterPath & path,
                                             ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    if (HasFeature(Feature::kUserDefined))
    {
        ReturnErrorOnFailure(builder.AppendElements({ MaxUserDefinedZones::kMetadataEntry }));
    }

    if (!HasFeature(Feature::kPerZoneSensitivity))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Sensitivity::kMetadataEntry }));
    }

    if (HasFeature(Feature::kTwoDimensionalCartesianZone))
    {
        ReturnErrorOnFailure(builder.AppendElements({ TwoDCartesianMax::kMetadataEntry }));
    }

    ReturnErrorOnFailure(builder.AppendElements(kMandatoryAttributes));
    return builder.AppendElements(DefaultServerCluster::GlobalAttributes());
}

CHIP_ERROR ZoneManagementCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                                   ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace chip::app::Clusters::ZoneManagement::Commands;
    if (HasFeature(Feature::kTwoDimensionalCartesianZone) && HasFeature(Feature::kUserDefined))
    {
        ReturnErrorOnFailure(
            builder.AppendElements({ CreateTwoDCartesianZone::kMetadataEntry, UpdateTwoDCartesianZone::kMetadataEntry }));
    }
    if (HasFeature(Feature::kUserDefined))
    {
        ReturnErrorOnFailure(builder.AppendElements({ RemoveZone::kMetadataEntry }));
    }
    return builder.AppendElements({ CreateOrUpdateTrigger::kMetadataEntry, RemoveTrigger::kMetadataEntry });
}

CHIP_ERROR ZoneManagementCluster::GeneratedCommands(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<CommandId> & builder)
{
    using namespace chip::app::Clusters::ZoneManagement::Commands;
    if (HasFeature(Feature::kTwoDimensionalCartesianZone))
    {
        return builder.AppendElements({ CreateTwoDCartesianZoneResponse::Id });
    }
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
    LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving zone: %s", mPath.mEndpointId, "start");

    for (const auto & zone : mZones)
    {
        err = zone.Encode(writer, chip::TLV::AnonymousTag());
        LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving zone: %s", mPath.mEndpointId, "element");
    }
    err = writer.EndContainer(arrayType);
    LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving zone: %s", mPath.mEndpointId, "end");

    bufferSpan.reduce_size(writer.GetLengthWritten());

    err = mContext->attributeStorage.WriteValue(ConcreteAttributePath(mPath.mEndpointId, ZoneManagement::Id, Attributes::Zones::Id),
                                                bufferSpan);
    LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving zone: %s", mPath.mEndpointId, "write");
}

void ZoneManagementCluster::PersistTriggers()
{
    uint8_t buffer[kMaxPersistedValueLengthSupported];
    MutableByteSpan bufferSpan(buffer);
    chip::TLV::TLVWriter writer;
    CHIP_ERROR err;
    TLV::TLVType arrayType;

    writer.Init(bufferSpan);

    err = writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType);
    LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving trigger: %s", mPath.mEndpointId, "start");

    for (const auto & trigger : mTriggers)
    {
        err = trigger.Encode(writer, chip::TLV::AnonymousTag());
        LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving trigger: %s", mPath.mEndpointId, "element");
    }
    err = writer.EndContainer(arrayType);
    LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving trigger: %s", mPath.mEndpointId, "end");

    bufferSpan.reduce_size(writer.GetLengthWritten());

    err = mContext->attributeStorage.WriteValue(
        ConcreteAttributePath(mPath.mEndpointId, ZoneManagement::Id, Attributes::Triggers::Id), bufferSpan);
    LogAndReturnOnFailure(err, Zcl, "ZoneManagement[ep=%d] failure saving trigger: %s", mPath.mEndpointId, "write");
}

CHIP_ERROR ZoneManagementCluster::AddZone(const ZoneInformationStorage & zone)
{
    mZones.push_back(zone);
    PersistZones();
    auto path = ConcreteAttributePath(mPath.mEndpointId, ZoneManagement::Id, Attributes::Zones::Id);
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
        auto path = ConcreteAttributePath(mPath.mEndpointId, ZoneManagement::Id, Attributes::Zones::Id);
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
        auto path = ConcreteAttributePath(mPath.mEndpointId, ZoneManagement::Id, Attributes::Zones::Id);
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
        PersistTriggers();
        auto path = ConcreteAttributePath(mPath.mEndpointId, ZoneManagement::Id, Attributes::Triggers::Id);
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
        PersistTriggers();
        auto path = ConcreteAttributePath(mPath.mEndpointId, ZoneManagement::Id, Attributes::Triggers::Id);
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

void ZoneManagementCluster::HandleCreateTwoDCartesianZone(CommandHandler * handler, const ConcreteCommandPath & commandPath,
                                                          const Commands::CreateTwoDCartesianZone::DecodableType & commandData)
{
    Commands::CreateTwoDCartesianZoneResponse::Type response;
    uint16_t zoneID     = 0;
    auto & zoneToCreate = commandData.zone;

    Status status = ValidateTwoDCartesianZone(zoneToCreate);
    if (status != Status::Success)
    {
        handler->AddStatus(commandPath, status, "invalidZone");
        return;
    }

    VerifyOrReturn(mUserDefinedZonesCount < mMaxUserDefinedZones,
                   handler->AddStatus(commandPath, Status::ResourceExhausted, "maxZones"));

    if (zoneToCreate.use == ZoneUseEnum::kFocus && !HasFeature(Feature::kFocusZones))
    {
        handler->AddStatus(commandPath, Status::ConstraintError, "focusZones");
        return;
    }

    std::vector<TwoDCartesianVertexStruct> twoDCartVertices;

    auto iter = zoneToCreate.vertices.begin();
    while (iter.Next())
    {
        auto vertex = iter.GetValue();
        if (vertex.x > mTwoDCartesianMax.x || vertex.y > mTwoDCartesianMax.y)
        {
            handler->AddStatus(commandPath, Status::DynamicConstraintError, "boundingBox");
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
        handler->AddStatus(commandPath, Status::InvalidCommand, "vertexList");
        return;
    }

    if (ZoneAlreadyExists(zoneToCreate.use, twoDCartVertices, DataModel::NullNullable))
    {
        handler->AddStatus(commandPath, Status::AlreadyExists, "zoneExists");
        return;
    }

    if (ZoneGeometry::IsZoneSelfIntersecting(twoDCartVertices))
    {
        ChipLogError(Zcl, "HandleCreateTwoDCartesianZone: Found self-intersecting polygon vertices for zone");
        handler->AddStatus(commandPath, Status::DynamicConstraintError, "selfIntersection");
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
        handler->AddStatus(commandPath, status, "delegate");
        return;
    }

    ZoneInformationStorage zoneInfo;
    zoneInfo.Set(zoneID, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(twoDCartZoneStorage));

    AddZone(zoneInfo);
    mUserDefinedZonesCount++;

    response.zoneID = zoneID;
    handler->AddResponse(commandPath, response);
}

void ZoneManagementCluster::HandleUpdateTwoDCartesianZone(CommandHandler * handler, const ConcreteCommandPath & commandPath,
                                                          const Commands::UpdateTwoDCartesianZone::DecodableType & commandData)
{
    uint16_t zoneID     = commandData.zoneID;
    auto & zoneToUpdate = commandData.zone;

    Status status = ValidateTwoDCartesianZone(zoneToUpdate);

    if (status != Status::Success)
    {
        handler->AddStatus(commandPath, status, "invalidZone");
        return;
    }

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStruct & z) { return z.zoneID == zoneID; });
    if (foundZone == mZones.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mPath.mEndpointId, zoneID);
        handler->AddStatus(commandPath, Status::NotFound, "notFound");
        return;
    }

    if (foundZone->zoneSource == ZoneSourceEnum::kMfg)
    {
        handler->AddStatus(commandPath, Status::ConstraintError, "zoneSource");
        return;
    }

    if (foundZone->zoneType != ZoneTypeEnum::kTwoDCARTZone)
    {
        handler->AddStatus(commandPath, Status::DynamicConstraintError, "zoneType");
        return;
    }

    std::vector<TwoDCartesianVertexStruct> twoDCartVertices;

    auto iter = zoneToUpdate.vertices.begin();
    while (iter.Next())
    {
        auto vertex = iter.GetValue();
        if (vertex.x > mTwoDCartesianMax.x || vertex.y > mTwoDCartesianMax.y)
        {
            handler->AddStatus(commandPath, Status::DynamicConstraintError, "boundingBox");
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
        handler->AddStatus(commandPath, Status::InvalidCommand, "vertexList");
        return;
    }

    if (ZoneAlreadyExists(zoneToUpdate.use, twoDCartVertices, DataModel::MakeNullable(zoneID)))
    {
        handler->AddStatus(commandPath, Status::AlreadyExists, "zoneExists");
        return;
    }

    if (ZoneGeometry::IsZoneSelfIntersecting(twoDCartVertices))
    {
        ChipLogError(Zcl, "HandleUpdateTwoDCartesianZone: Found self-intersecting polygon vertices for zone");
        handler->AddStatus(commandPath, Status::DynamicConstraintError, "selfIntersection");
        return;
    }

    TwoDCartesianZoneStorage twoDCartZoneStorage;
    twoDCartZoneStorage.Set(zoneToUpdate.name, zoneToUpdate.use, twoDCartVertices, zoneToUpdate.color);

    // Call the delegate
    status = mDelegate.UpdateTwoDCartesianZone(zoneID, twoDCartZoneStorage);
    if (status != Status::Success)
    {
        handler->AddStatus(commandPath, status, "delegate");
        return;
    }

    ZoneInformationStorage zoneInfo;
    zoneInfo.Set(zoneID, ZoneTypeEnum::kTwoDCARTZone, ZoneSourceEnum::kUser, MakeOptional(twoDCartZoneStorage));

    UpdateZone(zoneID, zoneInfo);

    handler->AddStatus(commandPath, Status::Success);
}

void ZoneManagementCluster::HandleRemoveZone(CommandHandler * handler, const ConcreteCommandPath & commandPath,
                                             const Commands::RemoveZone::DecodableType & commandData)
{
    uint16_t zoneID = commandData.zoneID;

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStruct & z) { return z.zoneID == zoneID; });
    if (foundZone == mZones.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mPath.mEndpointId, zoneID);
        handler->AddStatus(commandPath, Status::NotFound);
        return;
    }

    if (foundZone->zoneSource == ZoneSourceEnum::kMfg)
    {
        handler->AddStatus(commandPath, Status::ConstraintError);
        return;
    }

    auto foundTrigger =
        std::find_if(mTriggers.begin(), mTriggers.end(), [&](const ZoneTriggerControlStruct & t) { return t.zoneID == zoneID; });
    if (foundTrigger != mTriggers.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: Zone id %d is referenced in zone triggers", mPath.mEndpointId, zoneID);
        handler->AddStatus(commandPath, Status::InvalidInState);
        return;
    }

    // Call the delegate
    Status status = mDelegate.RemoveZone(zoneID);

    if (status == Status::Success)
    {
        RemoveZone(zoneID);
        mUserDefinedZonesCount--;
    }

    handler->AddStatus(commandPath, status);
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

void ZoneManagementCluster::HandleCreateOrUpdateTrigger(CommandHandler * handler, const ConcreteCommandPath & commandPath,
                                                        const Commands::CreateOrUpdateTrigger::DecodableType & commandData)
{
    ZoneTriggerControlStruct trigger = commandData.trigger;

    Status status = ValidateTrigger(trigger);

    if (status != Status::Success)
    {
        handler->AddStatus(commandPath, status);
        return;
    }

    auto foundZone =
        std::find_if(mZones.begin(), mZones.end(), [&](const ZoneInformationStorage & z) { return z.zoneID == trigger.zoneID; });
    if (foundZone == mZones.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: No zone exists by id %d", mPath.mEndpointId, trigger.zoneID);
        handler->AddStatus(commandPath, Status::NotFound);
        return;
    }

    if (foundZone->twoDCartZoneStorage.HasValue() && foundZone->twoDCartZoneStorage.Value().use != ZoneUseEnum::kMotion)
    {
        handler->AddStatus(commandPath, Status::ConstraintError);
        return;
    }

    status = AddOrUpdateTrigger(trigger);

    handler->AddStatus(commandPath, status);
}

void ZoneManagementCluster::HandleRemoveTrigger(CommandHandler * handler, const ConcreteCommandPath & commandPath,
                                                const Commands::RemoveTrigger::DecodableType & commandData)
{
    uint16_t zoneID = commandData.zoneID;

    auto foundTrigger =
        std::find_if(mTriggers.begin(), mTriggers.end(), [&](const ZoneTriggerControlStruct & t) { return t.zoneID == zoneID; });
    if (foundTrigger == mTriggers.end())
    {
        ChipLogError(Zcl, "ZoneMgmt[ep=%d]: Trigger for zone id %d does not exist", mPath.mEndpointId, zoneID);
        handler->AddStatus(commandPath, Status::NotFound);
        return;
    }

    Status status = RemoveTrigger(zoneID);

    handler->AddStatus(commandPath, status);
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

    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate ZoneTriggered event: %" CHIP_ERROR_FORMAT, mPath.mEndpointId,
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

    CHIP_ERROR err = LogEvent(event, mPath.mEndpointId, eventNumber);

    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(AppServer, "Endpoint %d - Unable to generate ZoneTriggered event: %" CHIP_ERROR_FORMAT, mPath.mEndpointId,
                     err.Format());
        return Status::Failure;
    }
    return Status::Success;
}

} // namespace ZoneManagement
} // namespace Clusters
} // namespace app
} // namespace chip
void MatterZoneManagementPluginServerInitCallback() {}
