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

#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/av-analysis-server/AvAnalysisCluster.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/util/util.h>
#include <clusters/AvAnalysis/Metadata.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip::app::Clusters::AvAnalysis;
using namespace chip::app::Clusters::AvAnalysis::Structs;
using namespace chip::app::Clusters::AvAnalysis::Attributes;
using namespace chip::Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {

AvAnalysisServerLogic::AvAnalysisServerLogic(
    EndpointId aEndpointId, BitFlags<Feature> aFeatures,
    const std::vector<Descriptor::Structs::SemanticTagStruct::Type> & aSupportedAmbientContexts,
    DataModel::Nullable<uint8_t> aMaxZones, uint8_t aMaxAnalysisStreamCount) :
    mEndpointId(aEndpointId),
    mFeatures(aFeatures), mSupportedAmbientContexts(aSupportedAmbientContexts), mMaxAnalysisStreamCount(aMaxAnalysisStreamCount),
    mMaxZones(aMaxZones)
{}

AvAnalysisServerLogic::~AvAnalysisServerLogic()
{
    if (mCameraClient != nullptr && mCameraInteraction.InFlight())
    {
        mCameraClient->Cancel();
    }
}

CHIP_ERROR AvAnalysisServerLogic::Startup(AttributePersistenceProvider & aAttributePersistenceProvider)
{
    VerifyOrReturnError(mDelegate != nullptr, CHIP_ERROR_INCORRECT_STATE);
    mAttributePersistenceProvider = &aAttributePersistenceProvider;

    // Make sure mandated Features are set, one and only one of Local or Remote has to be set
    //
    VerifyOrReturnError(HasFeature(Feature::kLocalContextDetection) ^ HasFeature(Feature::kRemoteContextDetection),
                        CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl,
                                     "AvAnalysis: Feature configuration error. One and only one of "
                                     "Local or Remote Context Detection must be supported"));

    // If we don't have PerZoneSensivity then mMaxZones has to be Null
    VerifyOrReturnError(!(HasFeature(Feature::kPerZoneContextDetection) ^ !mMaxZones.IsNull()), CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl, "AvAnalysis: If Per Zone Sensitivity is set, Zones must be present, and vice versa"));

    // With Remote Context Detection the MaxAnalysisStreamCount fixed attribute has to be non-zero, and backing
    // storage for the AnalysisStreams attribute is needed.
    if (HasFeature(Feature::kRemoteContextDetection))
    {
        VerifyOrReturnError(mMaxAnalysisStreamCount > 0, CHIP_ERROR_INVALID_ARGUMENT,
                            ChipLogError(Zcl, "AvAnalysis: MaxAnalysisStreamCount must be non-zero with Remote Detection"));
        if (mStreamTable.Capacity() == 0)
        {
            ReturnErrorOnFailure(mStreamTable.Init(mMaxAnalysisStreamCount));
        }
    }

    LoadPersistentAttributes();

    ChipLogProgress(Zcl, "AvAnalysis Cluster: Startup completed ");
    return CHIP_NO_ERROR;
}

void AvAnalysisServerLogic::Shutdown()
{
    if (mCameraClient != nullptr && mCameraInteraction.InFlight())
    {
        mCameraClient->Cancel();
    }

    // A command still waiting on a camera interaction can no longer be completed.
    ConcreteCommandPath commandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);
    auto handleRef = mCameraInteraction.Complete(commandPath);
    if (auto * handler = handleRef.Get(); handler != nullptr)
    {
        handler->AddStatus(commandPath, Status::Failure);
    }

    if (mDelegate != nullptr)
    {
        mDelegate->ShutdownApp();
    }
}

bool AvAnalysisServerLogic::HasFeature(Feature aFeature) const
{
    return mFeatures.Has(aFeature);
}

void AvAnalysisServerLogic::SetStreamState(AnalysisStreamEntry & aEntry, AnalysisStreamStateEnum aState)
{
    VerifyOrReturn(aEntry.state != aState);
    aEntry.state = aState;
    MarkDirty(Attributes::AnalysisStreams::Id);
}

void AvAnalysisServerLogic::OnVideoStreamAllocated(Status aStatus, uint16_t aVideoStreamId)
{
    VerifyOrReturn(mCameraInteraction.GetState() == AvAnalysis::CameraInteraction::State::kEstablishing,
                   ChipLogError(Zcl, "AvAnalysis[ep=%d]: unexpected allocation completion", mEndpointId));

    ScopedNodeId cameraNode = mCameraInteraction.CameraNode();
    ConcreteCommandPath commandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);
    // The client exchange may have died while the camera round-trip was in flight; the camera's
    // answer is ground truth and is recorded regardless, only the response needs a live handler.
    auto handleRef = mCameraInteraction.Complete(commandPath);
    auto * handler = handleRef.Get();

    // a non-SUCCESS camera response is propagated as the command status, no side-effects.
    if (aStatus != Status::Success)
    {
        VerifyOrReturn(handler != nullptr);
        handler->AddStatus(commandPath, aStatus);
        return;
    }

    // The camera answers a matching allocate with the existing VideoStreamID, so
    // a camera stream this table already tracks means the analysis stream already exists: respond
    // with its id rather than creating a second entry over the same stream.
    AnalysisStreamEntry * entry = mStreamTable.FindByCameraStream(cameraNode, aVideoStreamId);
    if (entry == nullptr)
    {
        entry = mStreamTable.Add(aVideoStreamId, cameraNode);
        if (entry == nullptr)
        {
            ChipLogError(Zcl, "AvAnalysis[ep=%d]: stream table rejected entry", mEndpointId);
            VerifyOrReturn(handler != nullptr);
            handler->AddStatus(commandPath, Status::ResourceExhausted);
            return;
        }

        MarkDirty(Attributes::CurrentAnalysisStreamCount::Id);
        MarkDirty(Attributes::AnalysisStreams::Id);
        LogErrorOnFailure(StoreAnalysisStreams());
    }

    VerifyOrReturn(handler != nullptr);
    Commands::EstablishAnalysisStreamResponse::Type response;
    response.analysisStreamID = entry->analysisStreamID;
    handler->AddResponse(commandPath, response);
}

void AvAnalysisServerLogic::OnVideoStreamDeallocated(Status aStatus, uint16_t aVideoStreamId)
{
    VerifyOrReturn(mCameraInteraction.GetState() == AvAnalysis::CameraInteraction::State::kRemoving,
                   ChipLogError(Zcl, "AvAnalysis[ep=%d]: unexpected deallocation completion", mEndpointId));

    // The camera client reports the camera's VideoStreamID; the table entry is keyed by the
    // AnalysisStreamID recorded when the interaction began
    uint16_t analysisStreamId = mCameraInteraction.AnalysisStreamId();
    ConcreteCommandPath commandPath(kInvalidEndpointId, kInvalidClusterId, kInvalidCommandId);
    // The camera has completed the deallocation, so the table is reconciled even if the
    // client exchange died; only the response needs a live handler.
    auto handleRef = mCameraInteraction.Complete(commandPath);
    auto * handler = handleRef.Get();

    // NOT_FOUND means the camera no longer has the stream, so the removal this
    // command asked for is already true on both sides: retaining it would leave
    // an entry occupying capacity that no retry could ever remove.
    if (aStatus != Status::Success && aStatus != Status::NotFound)
    {
        VerifyOrReturn(handler != nullptr);
        handler->AddStatus(commandPath, aStatus);
        return;
    }

    if (mStreamTable.Remove(analysisStreamId))
    {
        MarkDirty(Attributes::CurrentAnalysisStreamCount::Id);
        MarkDirty(Attributes::AnalysisStreams::Id);
        LogErrorOnFailure(StoreAnalysisStreams());
    }

    VerifyOrReturn(handler != nullptr);
    handler->AddStatus(commandPath, aStatus);
}

CHIP_ERROR
AvAnalysisServerLogic::AcceptedCommands(ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    ReturnErrorOnFailure(builder.AppendElements({ Commands::EnableContextTriggers::kMetadataEntry }));
    ReturnErrorOnFailure(builder.AppendElements({ Commands::DisableContextTriggers::kMetadataEntry }));

    if (HasFeature(Feature::kRemoteContextDetection))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::EstablishAnalysisStream::kMetadataEntry }));
        ReturnErrorOnFailure(builder.AppendElements({ Commands::ActivateAnalysisStream::kMetadataEntry }));
        ReturnErrorOnFailure(builder.AppendElements({ Commands::DeactivateAnalysisStream::kMetadataEntry }));
        ReturnErrorOnFailure(builder.AppendElements({ Commands::RemoveAnalysisStream::kMetadataEntry }));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AvAnalysisServerLogic::GeneratedCommands(ReadOnlyBufferBuilder<CommandId> & builder)
{
    if (HasFeature(Feature::kRemoteContextDetection))
    {
        ReturnErrorOnFailure(builder.AppendElements({ Commands::EstablishAnalysisStreamResponse::Id }));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR AvAnalysisServerLogic::Attributes(ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);

    // Attributes tat are set dependent on the Feature Flags
    AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
        { HasFeature(Feature::kRemoteContextDetection), MaxAnalysisStreamCount::kMetadataEntry },
        { HasFeature(Feature::kRemoteContextDetection), CurrentAnalysisStreamCount::kMetadataEntry },
        { HasFeature(Feature::kRemoteContextDetection), AnalysisStreams::kMetadataEntry },
    };

    return listBuilder.Append(Span(Attributes::kMandatoryMetadata), Span(optionalAttributes));
}

void AvAnalysisServerLogic::MarkDirty(AttributeId aAttributeId)
{
    if (mMarkDirtyCallback)
    {
        mMarkDirtyCallback(aAttributeId);
    }
}

CHIP_ERROR AvAnalysisServerLogic::ReadAndEncodeSupportedAmbientContexts(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & supportedAmbientContext : mSupportedAmbientContexts)
        {
            ReturnErrorOnFailure(encoder.Encode(supportedAmbientContext));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR AvAnalysisServerLogic::ReadAndEncodeActiveAmbientContextTriggers(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (auto & activeContextTrigger : mActiveAmbientContextTriggers)
        {
            AvAnalysis::Structs::ContextTriggerStruct::Type contextTrigger;
            contextTrigger.context = activeContextTrigger.GetContext();
            contextTrigger.zoneIDs = activeContextTrigger.GetZoneIDsForEncode();

            ReturnErrorOnFailure(encoder.Encode(contextTrigger));
        }

        return CHIP_NO_ERROR;
    });
}

CHIP_ERROR AvAnalysisServerLogic::ReadAndEncodeAnalysisStreams(AttributeValueEncoder & aEncoder)
{
    return aEncoder.EncodeList([this](const auto & encoder) -> CHIP_ERROR {
        for (const auto & analysisStream : mStreamTable)
        {
            ReturnErrorOnFailure(encoder.Encode(analysisStream.ToEncodableStruct()));
        }

        return CHIP_NO_ERROR;
    });
}

// Attribute mutators
CHIP_ERROR AvAnalysisServerLogic::SetTrackingEnabled(bool aTrackingEnabled)
{
    VerifyOrReturnValue(mTrackingEnabled != aTrackingEnabled, CHIP_NO_ERROR);

    mTrackingEnabled = aTrackingEnabled;
    MarkDirty(AvAnalysis::Attributes::TrackingEnabled::Id);
    LogErrorOnFailure(StoreTrackingEnabled());
    return CHIP_NO_ERROR;
}

/**
 * Persistence handling helper, stores the current value of the ActiveAmbientContextTriggers attribiute in the KVS
 */
CHIP_ERROR AvAnalysisServerLogic::StoreActiveAmbientContextTriggers()
{
    VerifyOrReturnError(mAttributePersistenceProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);
    Platform::ScopedMemoryBuffer<uint8_t> contextTriggers;
    MutableByteSpan bufferSpan;

    size_t maxBufferSize;
    if (!mMaxZones.IsNull())
    {
        size_t zoneIDsSize = static_cast<size_t>(sizeof(uint16_t) * mMaxZones.Value());
        maxBufferSize = static_cast<size_t>((kSemanticTagStructSerializedSize + zoneIDsSize) * kMaxActiveAmbientContextTriggers);
    }
    else
    {
        maxBufferSize = static_cast<size_t>(kSemanticTagStructSerializedSize * kMaxActiveAmbientContextTriggers);
    }

    if (!contextTriggers.Alloc(maxBufferSize))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    bufferSpan = MutableByteSpan{ contextTriggers.Get(), maxBufferSize };

    TLV::TLVWriter writer;

    writer.Init(bufferSpan);
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(writer.StartContainer(TLV::AnonymousTag(), TLV::kTLVType_Array, arrayType));

    for (auto & contextTrigger : mActiveAmbientContextTriggers)
    {
        // Get the details to encode from the storage
        //
        ContextTriggerStruct::Type triggerStruct;
        triggerStruct.context = contextTrigger.GetContext();
        triggerStruct.zoneIDs = contextTrigger.GetZoneIDsForEncode();

        ReturnErrorOnFailure(triggerStruct.Encode(writer, TLV::AnonymousTag()));
    }

    ReturnErrorOnFailure(writer.EndContainer(arrayType));

    auto path = ConcreteAttributePath(mEndpointId, AvAnalysis::Id, Attributes::ActiveAmbientContextTriggers::Id);
    bufferSpan.reduce_size(writer.GetLengthWritten());
    ReturnErrorOnFailure(mAttributePersistenceProvider->WriteValue(path, bufferSpan));

    return CHIP_NO_ERROR;
}

/**
 * Persistence handling helper, reads the current value of the ActiveAmbientContextTriggers attribiute from the KVS
 */
CHIP_ERROR AvAnalysisServerLogic::LoadActiveAmbientContextTriggers()
{
    VerifyOrReturnError(mAttributePersistenceProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);
    Platform::ScopedMemoryBuffer<uint8_t> contextTriggers;
    MutableByteSpan bufferSpan;

    size_t maxBufferSize;
    if (!mMaxZones.IsNull())
    {
        size_t zoneIDsSize = static_cast<size_t>(sizeof(uint16_t) * mMaxZones.Value());
        maxBufferSize = static_cast<size_t>((kSemanticTagStructSerializedSize + zoneIDsSize) * kMaxActiveAmbientContextTriggers);
    }
    else
    {
        maxBufferSize = static_cast<size_t>(kSemanticTagStructSerializedSize * kMaxActiveAmbientContextTriggers);
    }

    if (!contextTriggers.Alloc(maxBufferSize))
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    bufferSpan = MutableByteSpan{ contextTriggers.Get(), maxBufferSize };

    auto path = ConcreteAttributePath(mEndpointId, AvAnalysis::Id, Attributes::ActiveAmbientContextTriggers::Id);

    CHIP_ERROR err = mAttributePersistenceProvider->ReadValue(path, bufferSpan);

    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogDetail(Zcl, "AvAnalysis[ep=%d]: No persisted ActiveAmbientContextTriggers.", mEndpointId);
        mActiveAmbientContextTriggers.clear();
        return CHIP_NO_ERROR;
    }

    ReturnErrorOnFailure(err);

    TLV::TLVReader reader;
    reader.Init(bufferSpan);

    ReturnErrorOnFailure(reader.Next(TLV::kTLVType_Array, TLV::AnonymousTag()));
    TLV::TLVType arrayType;
    ReturnErrorOnFailure(reader.EnterContainer(arrayType));

    mActiveAmbientContextTriggers.clear();
    while ((err = reader.Next()) == CHIP_NO_ERROR)
    {
        ContextTriggerStruct::DecodableType trigger;
        ReturnErrorOnFailure(DataModel::Decode(reader, trigger));

        AvAnalysis::AmbientContextStorage triggerStorage;
        triggerStorage.SetContext(trigger.context);

        // If we have no max zones then we have no zone triggers, set to Null. Otherwise convert List to Vector.
        //
        if (mMaxZones.IsNull())
        {
            triggerStorage.SetZoneIDs(MakeOptional(DataModel::NullNullable));
        }
        else
        {
            std::vector<uint16_t> zoneIDs;
            size_t size;

            if (!trigger.zoneIDs.Value().IsNull())
            {
                err = trigger.zoneIDs.Value().Value().ComputeSize(&size);
                VerifyOrReturnError(err == CHIP_NO_ERROR, err);
                zoneIDs.reserve(size);

                auto zone_iter = trigger.zoneIDs.Value().Value().begin();

                while (zone_iter.Next())
                {
                    zoneIDs.push_back(zone_iter.GetValue());
                }
                triggerStorage.SetZoneIDs(MakeOptional(DataModel::MakeNullable(zoneIDs)));
            }
            else
            {
                triggerStorage.SetZoneIDs(MakeOptional(DataModel::NullNullable));
            }
        }

        mActiveAmbientContextTriggers.push_back(triggerStorage);
    }

    VerifyOrReturnError(err == CHIP_ERROR_END_OF_TLV, err);

    ReturnErrorOnFailure(reader.ExitContainer(arrayType));

    return reader.VerifyEndOfContainer();
}

/**
 * Persistence handling helper, stores the current contents of the analysis stream table in the KVS
 */
CHIP_ERROR AvAnalysisServerLogic::StoreAnalysisStreams()
{
    VerifyOrReturnError(mAttributePersistenceProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t maxBufferSize =
        AnalysisStreamTable::kEntrySerializedSize * mStreamTable.Capacity() + AnalysisStreamTable::kArraySerializedOverhead;
    Platform::ScopedMemoryBuffer<uint8_t> buffer;
    VerifyOrReturnError(buffer.Alloc(maxBufferSize), CHIP_ERROR_NO_MEMORY);

    TLV::TLVWriter writer;
    writer.Init(buffer.Get(), maxBufferSize);
    ReturnErrorOnFailure(mStreamTable.Encode(writer));

    auto path = ConcreteAttributePath(mEndpointId, AvAnalysis::Id, Attributes::AnalysisStreams::Id);
    return mAttributePersistenceProvider->WriteValue(path, ByteSpan(buffer.Get(), writer.GetLengthWritten()));
}

/**
 * Persistence handling helper, restores the analysis stream table from the KVS. Restored entries restart
 * from PendingInitiation as sessions do not survive a reboot.
 */
CHIP_ERROR AvAnalysisServerLogic::LoadAnalysisStreams()
{
    VerifyOrReturnError(mAttributePersistenceProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    size_t maxBufferSize =
        AnalysisStreamTable::kEntrySerializedSize * mStreamTable.Capacity() + AnalysisStreamTable::kArraySerializedOverhead;
    Platform::ScopedMemoryBuffer<uint8_t> buffer;
    VerifyOrReturnError(buffer.Alloc(maxBufferSize), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan bufferSpan(buffer.Get(), maxBufferSize);

    auto path      = ConcreteAttributePath(mEndpointId, AvAnalysis::Id, Attributes::AnalysisStreams::Id);
    CHIP_ERROR err = mAttributePersistenceProvider->ReadValue(path, bufferSpan);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        ChipLogDetail(Zcl, "AvAnalysis[ep=%d]: No persisted AnalysisStreams.", mEndpointId);
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    TLV::TLVReader reader;
    reader.Init(bufferSpan);
    return mStreamTable.Decode(reader);
}

/**
 * Persistence handling helper, stores the TrackingEnabled attribute in the KVS
 */
CHIP_ERROR AvAnalysisServerLogic::StoreTrackingEnabled()
{
    VerifyOrReturnError(mAttributePersistenceProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint8_t value = mTrackingEnabled ? 1 : 0;
    auto path     = ConcreteAttributePath(mEndpointId, AvAnalysis::Id, Attributes::TrackingEnabled::Id);
    return mAttributePersistenceProvider->WriteValue(path, ByteSpan(&value, sizeof(value)));
}

/**
 * Persistence handling helper, restores the TrackingEnabled attribute from the KVS
 */
CHIP_ERROR AvAnalysisServerLogic::LoadTrackingEnabled()
{
    VerifyOrReturnError(mAttributePersistenceProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    uint8_t value = 0;
    MutableByteSpan valueSpan(&value, sizeof(value));
    auto path      = ConcreteAttributePath(mEndpointId, AvAnalysis::Id, Attributes::TrackingEnabled::Id);
    CHIP_ERROR err = mAttributePersistenceProvider->ReadValue(path, valueSpan);
    if (err == CHIP_ERROR_PERSISTED_STORAGE_VALUE_NOT_FOUND)
    {
        return CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    VerifyOrReturnError(valueSpan.size() == sizeof(value), CHIP_ERROR_INVALID_TLV_ELEMENT);
    mTrackingEnabled = (value != 0);
    return CHIP_NO_ERROR;
}

/**
 * Persistence handling helper, loads all non-volatile attributes from the KVS
 */
void AvAnalysisServerLogic::LoadPersistentAttributes()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Load ActiveAmbientContexts
    err = LoadActiveAmbientContextTriggers();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "AvAnalysis[ep=%d]: Unable to load the ActiveAmbientContexts from the KVS.", mEndpointId);

        // Clear the local storage
        mActiveAmbientContextTriggers.clear();
    }
    else
    {
        ChipLogDetail(Zcl, "AvAnalysis[ep=%d]: Loaded ActiveAmbientContexts", mEndpointId);
    }

    if (LoadTrackingEnabled() != CHIP_NO_ERROR)
    {
        ChipLogDetail(Zcl, "AvAnalysis[ep=%d]: Unable to load TrackingEnabled from the KVS.", mEndpointId);
        mTrackingEnabled = false;
    }

    if (HasFeature(Feature::kRemoteContextDetection) && LoadAnalysisStreams() != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysis[ep=%d]: Unable to load AnalysisStreams from the KVS.", mEndpointId);
    }

    // Signal delegate that all persistent configuration attributes have been loaded.
    if (mDelegate != nullptr)
    {
        TEMPORARY_RETURN_IGNORED mDelegate->PersistentAttributesLoadedCallback();
    }
}

/**
 * Handler for the EnableContextTriggers command. This invokes specific methods for local vs. remote as the logic
 * associated with each is different
 */
std::optional<DataModel::ActionReturnStatus>
AvAnalysisServerLogic::HandleEnableContextTriggers(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                   const AvAnalysis::Commands::EnableContextTriggers::DecodableType & commandData)
{
    // Are we locally or remotely processing, handle appropriately
    //
    if (HasFeature(AvAnalysis::Feature::kLocalContextDetection))
    {
        return HandleLocalEnableContextTriggers(handler, commandPath, commandData);
    }

    return HandleRemoteEnableContextTriggers(handler, commandPath, commandData);
}

/**
 * Handler for EnableContextTriggers when the local detect feature is set.
 */
std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleLocalEnableContextTriggers(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::EnableContextTriggers::DecodableType & commandData)
{
    // Verify spec constraints, provided list is 50 entries or less if not null
    //
    if (!commandData.contextTriggers.IsNull())
    {
        size_t size;
        CHIP_ERROR err = commandData.contextTriggers.Value().ComputeSize(&size);
        VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
        VerifyOrReturnError(size <= AvAnalysis::kMaxContextTriggers, Status::InvalidCommand);
    }

    // Server command logic starts here
    //
    if (!commandData.contextTriggers.IsNull())
    {
        // Loop over the provided context triggers
        auto iter = commandData.contextTriggers.Value().begin();

        while (iter.Next())
        {
            Structs::ContextTriggerStruct::DecodableType contextTrigger = iter.GetValue();

            // Make sure the context is part of our supported set
            //
            auto it = std::find_if(mSupportedAmbientContexts.begin(), mSupportedAmbientContexts.end(),
                                   [&contextTrigger](const Descriptor::Structs::SemanticTagStruct::Type & stt) {
                                       return stt.namespaceID == contextTrigger.context.namespaceID &&
                                           stt.tag == contextTrigger.context.tag;
                                   });

            VerifyOrReturnError(it != mSupportedAmbientContexts.end(), Status::ConstraintError);

            // The trigger context is valid, now check the ZoneIDs, which can only be present of PERZONEDETECT is set, likewise,
            // if we have the feature, then ZoneIDs have to be present
            //
            bool hasZoneIDs        = contextTrigger.zoneIDs.HasValue();
            bool hasNonNullZoneIDs = false;

            if ((hasZoneIDs && !HasFeature(AvAnalysis::Feature::kPerZoneContextDetection)) ||
                (!hasZoneIDs && HasFeature(AvAnalysis::Feature::kPerZoneContextDetection)))
            {
                return Status::InvalidCommand;
            }

            // Get the ZoneIDs, if present, into a format that can be used, that is convert the DecodableList to a List
            //
            std::vector<uint16_t> zoneIDs;
            size_t size;

            if (hasZoneIDs)
            {
                // Verify via the delegate that the provided list of ZoneIDs contains values present in ZoneManagement only
                // if the zoneIDs we have are not Null.
                //
                if (!contextTrigger.zoneIDs.Value().IsNull())
                {
                    CHIP_ERROR err = contextTrigger.zoneIDs.Value().Value().ComputeSize(&size);
                    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
                    zoneIDs.reserve(size);

                    auto zone_iter = contextTrigger.zoneIDs.Value().Value().begin();

                    while (zone_iter.Next())
                    {
                        zoneIDs.push_back(zone_iter.GetValue());
                    }
                    err = mDelegate->VerifyZoneIDsAreValid(zoneIDs);
                    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::NotFound);
                    hasNonNullZoneIDs = true;
                }
            }

            // Check with the delegate that additional contexts can be added
            //
            VerifyOrReturnError(mDelegate->CanAddContextTriggers(), Status::ResourceExhausted);

            // Update our active trigger set with this new context.
            // If the context exists, update the zone IDs, otherise add a new entry
            //
            auto it2 = std::find_if(mActiveAmbientContextTriggers.begin(), mActiveAmbientContextTriggers.end(),
                                    [&contextTrigger](AvAnalysis::AmbientContextStorage acs) {
                                        return acs.GetContext().namespaceID == contextTrigger.context.namespaceID &&
                                            acs.GetContext().tag == contextTrigger.context.tag;
                                    });

            // Does an entry with this context already exist?
            //
            if (it2 == mActiveAmbientContextTriggers.end())
            {
                // No existing context, so just add this new one to the end
                //
                AvAnalysis::AmbientContextStorage newContextTrigger;
                newContextTrigger.SetContext(contextTrigger.context);

                // if we have Per Zone Sensitivity, then we have ZoneIDs (which could be null), add those, this is empty otherwise
                if (HasFeature(AvAnalysis::Feature::kPerZoneContextDetection))
                {
                    if (!hasNonNullZoneIDs)
                    {
                        newContextTrigger.SetZoneIDs(chip::MakeOptional(DataModel::NullNullable));
                    }
                    else
                    {
                        newContextTrigger.SetZoneIDs(chip::MakeOptional(DataModel::MakeNullable(zoneIDs)));
                    }
                }
                mActiveAmbientContextTriggers.push_back(newContextTrigger);
            }
            else
            {
                // Existing context, update the ZoneIDs, if Per Zone Sensitivity is set, to be that provided
                //
                if (HasFeature(AvAnalysis::Feature::kPerZoneContextDetection))
                {
                    if (hasNonNullZoneIDs)
                    {
                        it2->SetZoneIDs(chip::MakeOptional(DataModel::MakeNullable(zoneIDs)));
                    }
                    else
                    {
                        it2->SetZoneIDs(chip::MakeOptional(DataModel::NullNullable));
                    }
                }
            }
        }
    }
    else
    {
        // Provided set is null, meaning all known context triggers should be activated
        // First check with the delegate that additional contexts can be added
        //
        VerifyOrReturnError(mDelegate->CanAddContextTriggers(), Status::ResourceExhausted);

        // Set the active triggers to be the supported triggers
        mActiveAmbientContextTriggers.clear();

        for (const auto & supportedContext : mSupportedAmbientContexts)
        {
            AvAnalysis::AmbientContextStorage contextTrigger;
            contextTrigger.SetContext(supportedContext);

            // Set the ZoneIDs to null meaning all zones only if Per Zone Sensitivity is supported
            if (HasFeature(AvAnalysis::Feature::kPerZoneContextDetection))
            {
                contextTrigger.SetZoneIDs(chip::MakeOptional(DataModel::NullNullable));
            }
            mActiveAmbientContextTriggers.push_back(contextTrigger);
        }
    }

    // Inform the delegate of the new active context set. The delegate will read the updated contents
    // of the attribute
    //
    mDelegate->ActiveAmbientContextTriggersUpdated();
    MarkDirty(AvAnalysis::Attributes::ActiveAmbientContextTriggers::Id);
    LogErrorOnFailure(StoreActiveAmbientContextTriggers());

    return Status::Success;
}

/**
 * Placeholder method for when the functionality for remote context detection is implemented
 */
std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleRemoteEnableContextTriggers(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::EnableContextTriggers::DecodableType & commandData)
{
    return Status::Success;
}

/**
 * Handler for the DisableContextTriggers command.  The functionality is the same for both local and remote
 * cases.
 */
std::optional<DataModel::ActionReturnStatus>
AvAnalysisServerLogic::HandleDisableContextTriggers(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                    const AvAnalysis::Commands::DisableContextTriggers::DecodableType & commandData)
{
    // Verify spec constraints, provided list is 50 entries or less if not null
    //
    if (!commandData.contextTriggers.IsNull())
    {
        size_t size;
        CHIP_ERROR err = commandData.contextTriggers.Value().ComputeSize(&size);
        VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
        VerifyOrReturnError(size <= AvAnalysis::kMaxContextTriggers, Status::InvalidCommand);
    }

    // Server command logic starts here
    //
    if (!commandData.contextTriggers.IsNull())
    {
        // Loop over the provided context triggers
        auto iter = commandData.contextTriggers.Value().begin();

        while (iter.Next())
        {
            Structs::ContextTriggerStruct::DecodableType contextTrigger = iter.GetValue();

            // Make sure the context is part of our active set
            //
            auto it = std::find_if(mActiveAmbientContextTriggers.begin(), mActiveAmbientContextTriggers.end(),
                                   [&contextTrigger](AmbientContextStorage & acs) {
                                       return acs.GetContext().namespaceID == contextTrigger.context.namespaceID &&
                                           acs.GetContext().tag == contextTrigger.context.tag;
                                   });

            if (it == mActiveAmbientContextTriggers.end())
            {
                return Status::DynamicConstraintError;
            }

            // The trigger context is valid, now check the ZoneIDs, which can only be present of PERZONEDETECT is set, likewise,
            // if we have the feature, then ZoneIDs have to be present
            //
            bool hasZoneIDs = contextTrigger.zoneIDs.HasValue();

            if ((hasZoneIDs && !HasFeature(AvAnalysis::Feature::kPerZoneContextDetection)) ||
                (!hasZoneIDs && HasFeature(AvAnalysis::Feature::kPerZoneContextDetection)))
            {
                return Status::InvalidCommand;
            }

            if (hasZoneIDs)
            {
                // Verify via the delegate that the provided list of ZoneIDs contains values present in ZoneManagement only
                // if the zoneIDs we have are not Null.
                //
                if (!contextTrigger.zoneIDs.Value().IsNull())
                {
                    std::vector<uint16_t> zoneIDs;
                    size_t size;

                    CHIP_ERROR err = contextTrigger.zoneIDs.Value().Value().ComputeSize(&size);
                    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
                    zoneIDs.reserve(size);

                    auto zone_iter = contextTrigger.zoneIDs.Value().Value().begin();

                    while (zone_iter.Next())
                    {
                        zoneIDs.push_back(zone_iter.GetValue());
                    }
                    err = mDelegate->VerifyZoneIDsAreValid(zoneIDs);
                    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::NotFound);
                }
            }

            // If Per Zone Sensitivity is not supported, remove the current context
            //
            if (!HasFeature(AvAnalysis::Feature::kPerZoneContextDetection))
            {
                mActiveAmbientContextTriggers.erase(it);
            }
            else
            {
                // Per Zone Sensitivity is supported.
                // If the ZoneIDs are null, remove the current context
                //
                if (contextTrigger.zoneIDs.Value().IsNull())
                {
                    mActiveAmbientContextTriggers.erase(it);
                }
                else
                {
                    // If the current set is null, that means all zones, disable is only valid also for all zones
                    //
                    if (it->GetZoneIDs().Value().IsNull())
                    {
                        return Status::DynamicConstraintError;
                    }

                    // Remove the ZoneIds provided from the current set, if this results in an empty list, remove the entry
                    //
                    DataModel::DecodableList<uint16_t> zoneIDsToRemove = contextTrigger.zoneIDs.Value().Value();
                    std::vector<uint16_t> updatedZoneIDList;
                    std::vector<uint16_t> currentZoneIds = it->GetZoneIDs().Value().Value();
                    for (const auto & item : currentZoneIds)
                    {
                        if (!ZoneIDListContains(zoneIDsToRemove, item))
                        {
                            updatedZoneIDList.push_back(item);
                        }
                    }

                    // If the result is an empty list, remove, otherwise update
                    //
                    if (updatedZoneIDList.size() == 0)
                    {
                        mActiveAmbientContextTriggers.erase(it);
                    }
                    else
                    {
                        it->SetZoneIDs(chip::MakeOptional(DataModel::MakeNullable(updatedZoneIDList)));
                    }
                }
            }
        }
    }
    else
    {
        // Provided set is null, meaning the active triggers attribute shall be set to an empty list
        //
        mActiveAmbientContextTriggers.clear();
    }

    // Inform the delegate of the new active context set. The delegate will read the updated contents
    // of the attribute
    //
    mDelegate->ActiveAmbientContextTriggersUpdated();
    MarkDirty(AvAnalysis::Attributes::ActiveAmbientContextTriggers::Id);
    LogErrorOnFailure(StoreActiveAmbientContextTriggers());

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleEstablishAnalysisStream(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::EstablishAnalysisStream::DecodableType & commandData)
{
    // Spec 11.9.8: CurrentAnalysisStreamCount == MaxAnalysisStreamCount -> RESOURCE_EXHAUSTED
    VerifyOrReturnValue(!mStreamTable.IsFull(), Status::ResourceExhausted);

    // Without a camera client no camera interaction can be started
    VerifyOrReturnValue(mCameraClient != nullptr, Status::Failure,
                        ChipLogError(Zcl, "AvAnalysis[ep=%d]: no camera client configured", mEndpointId));

    // One camera-bound command at a time; the response of this one depends on the camera's answer
    VerifyOrReturnValue(!mCameraInteraction.InFlight(), Status::Busy);

    // The camera SHALL be on the same fabric as the Analysis Node: reach it on the invoking client's fabric
    ScopedNodeId cameraNode(commandData.nodeID, handler.GetAccessingFabricIndex());

    mCameraInteraction.Begin(AvAnalysis::CameraInteraction::State::kEstablishing, handler, commandPath, cameraNode);
    CHIP_ERROR err = mCameraClient->RequestVideoStreamAllocation(cameraNode, *this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysis[ep=%d]: failed to start stream allocation: %" CHIP_ERROR_FORMAT, mEndpointId, err.Format());
        mCameraInteraction.Abort();
        return (err == CHIP_ERROR_BUSY) ? Status::Busy : Status::Failure;
    }

    // Response is produced in OnVideoStreamAllocated once the camera answers
    return std::nullopt;
}

/**
 * TODO: implement WebRTC session activation (stream state machine beyond PendingInitiation).
 * Streams cannot be activated yet, so INVALID_IN_STATE is sent as response.
 */
std::optional<DataModel::ActionReturnStatus>
AvAnalysisServerLogic::HandleActivateAnalysisStream(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                    const AvAnalysis::Commands::ActivateAnalysisStream::DecodableType & commandData)
{
    VerifyOrReturnValue(mStreamTable.Find(commandData.analysisStreamID) != nullptr, Status::NotFound);
    return Status::InvalidInState;
}

/**
 * TODO: implement WebRTC session deactivation. No stream can be in an active state yet,
 * so INVALID_IN_STATE is sent as response.
 */
std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleDeactivateAnalysisStream(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::DeactivateAnalysisStream::DecodableType & commandData)
{
    VerifyOrReturnValue(mStreamTable.Find(commandData.analysisStreamID) != nullptr, Status::NotFound);
    return Status::InvalidInState;
}

std::optional<DataModel::ActionReturnStatus>
AvAnalysisServerLogic::HandleRemoveAnalysisStream(CommandHandler & handler, const ConcreteCommandPath & commandPath,
                                                  const AvAnalysis::Commands::RemoveAnalysisStream::DecodableType & commandData)
{
    AnalysisStreamEntry * entry = mStreamTable.Find(commandData.analysisStreamID);
    VerifyOrReturnValue(entry != nullptr, Status::NotFound);

    // only a stream in PendingInitiation may be removed
    VerifyOrReturnValue(entry->state == AnalysisStreamStateEnum::kPendingInitiation, Status::InvalidInState);

    VerifyOrReturnValue(mCameraClient != nullptr, Status::Failure,
                        ChipLogError(Zcl, "AvAnalysis[ep=%d]: no camera client configured", mEndpointId));

    // One camera-bound command at a time; the response of this one depends on the camera's answer
    VerifyOrReturnValue(!mCameraInteraction.InFlight(), Status::Busy);

    mCameraInteraction.Begin(AvAnalysis::CameraInteraction::State::kRemoving, handler, commandPath, entry->cameraNode,
                             entry->analysisStreamID);
    CHIP_ERROR err = mCameraClient->RequestVideoStreamDeallocation(entry->cameraNode, entry->videoStreamID, *this);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "AvAnalysis[ep=%d]: failed to start stream deallocation: %" CHIP_ERROR_FORMAT, mEndpointId, err.Format());
        mCameraInteraction.Abort();
        return (err == CHIP_ERROR_BUSY) ? Status::Busy : Status::Failure;
    }

    // Response is produced in OnVideoStreamDeallocated once the camera answers
    return std::nullopt;
}

/**
 * Local helper method that checks whether a provided zoneID is in a List of known zoneIDs
 *
 * @param list   the list of known zoneIDs
 * @param value  the zoneID we're checking for the presence of in the list
 * @return bool  true if the provided value is in the list
 */
bool AvAnalysisServerLogic::ZoneIDListContains(const DataModel::DecodableList<uint16_t> list, uint16_t value)
{
    auto iter = list.begin();

    while (iter.Next())
    {
        if (iter.GetValue() == value)
        {
            return true;
        }
    }
    return false;
}

CHIP_ERROR AvAnalysisServerLogic::AnalysisSessionStart(uint16_t & aSessionId,
                                                       const DataModel::Nullable<std::vector<uint16_t>> & aZoneList,
                                                       ServerClusterContext * aContext)
{
    VerifyOrReturnError(aContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Validate the information received - are the zoneIDs known (if provided)
    if (!aZoneList.IsNull())
    {
        ReturnErrorOnFailure(mDelegate->VerifyZoneIDsAreValid(aZoneList.Value()));
    }

    // Get our current session ID, and increment for next use
    aSessionId = mNextAnalysisSessionID++;

    // Capture our new active session information
    AvAnalysis::ActiveAmbientContextSession newSession;
    newSession.SetSessionId(aSessionId);
    mActiveSessions.push_back(newSession);

    // Create the Initial Event
    Events::AnalysisSessionStart::Type startEvent;

    startEvent.sessionID = aSessionId;

    // The zones could be null, meaning that no zone information is available
    if (aZoneList.IsNull())
    {
        startEvent.triggeredZones = DataModel::NullNullable;
    }
    else
    {
        startEvent.triggeredZones =
            DataModel::MakeNullable(DataModel::List<const uint16_t>(aZoneList.Value().data(), aZoneList.Value().size()));
    }

    VerifyOrReturnError(aContext->interactionContext.eventsGenerator.GenerateEvent(startEvent, mEndpointId).has_value(),
                        CHIP_ERROR_INTERNAL, ChipLogError(Zcl, "Unable to generate AnalysisSessionStart event"));

    return CHIP_NO_ERROR;
}

CHIP_ERROR AvAnalysisServerLogic::InitialTriggeringContextDetected(
    uint16_t aSessionId, const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aTriggeringContext,
    ServerClusterContext * aContext)
{
    VerifyOrReturnError(aContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Make sure the provided session ID is one we know about
    auto session_it =
        std::find_if(mActiveSessions.begin(), mActiveSessions.end(),
                     [aSessionId](const ActiveAmbientContextSession & session) { return session.GetSessionId() == aSessionId; });

    // Check if the element was actually found
    if (session_it == mActiveSessions.end())
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    // Are the contexts part of our active set
    if (!IsContextPartOfActiveContextTriggers(aTriggeringContext))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    session_it->AddTrackedContext(aTriggeringContext);

    // Now create the first Perceived Context Event with the tiggering context
    Events::PerceivedContext::Type perceivedEvent;

    perceivedEvent.sessionID             = aSessionId;
    perceivedEvent.newIdentifiedContexts = chip::MakeOptional(
        DataModel::List<const Structs::TrackedContext::Type>(aTriggeringContext.data(), aTriggeringContext.size()));

    VerifyOrReturnError(aContext->interactionContext.eventsGenerator.GenerateEvent(perceivedEvent, mEndpointId).has_value(),
                        CHIP_ERROR_INTERNAL, ChipLogError(Zcl, "Unable to generate PerceivedContext event"));

    return CHIP_NO_ERROR;
}

CHIP_ERROR AvAnalysisServerLogic::NewContextDetected(uint16_t aSessionId,
                                                     const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aNewContext,
                                                     ServerClusterContext * aContext)
{
    VerifyOrReturnError(aContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Make sure the provided session ID is one we know about
    auto it =
        std::find_if(mActiveSessions.begin(), mActiveSessions.end(),
                     [aSessionId](const ActiveAmbientContextSession & session) { return session.GetSessionId() == aSessionId; });

    // Check if the element was actually found
    if (it == mActiveSessions.end())
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    // Are the contexts part of our active set
    if (!IsContextPartOfActiveContextTriggers(aNewContext))
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    // Now create the Perceived Context Event with newly detected context
    Events::PerceivedContext::Type perceivedEvent;

    perceivedEvent.sessionID = aSessionId;
    perceivedEvent.newIdentifiedContexts =
        chip::MakeOptional(DataModel::List<const Structs::TrackedContext::Type>(aNewContext.data(), aNewContext.size()));
    perceivedEvent.currentIdentifiedContexts = chip::MakeOptional(
        DataModel::List<const Structs::TrackedContext::Type>(it->GetTrackedContexts().data(), it->GetTrackedContexts().size()));

    VerifyOrReturnError(aContext->interactionContext.eventsGenerator.GenerateEvent(perceivedEvent, mEndpointId).has_value(),
                        CHIP_ERROR_INTERNAL, ChipLogError(Zcl, "Unable to generate PerceivedContext event"));

    // Add the new context triggers to our current set for the session
    it->AddTrackedContext(aNewContext);

    return CHIP_NO_ERROR;
}

CHIP_ERROR
AvAnalysisServerLogic::ContextNoLongerDetected(uint16_t aSessionId,
                                               const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aOldContext,
                                               ServerClusterContext * aContext)
{
    VerifyOrReturnError(aContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Make sure the provided session ID is one we know about
    auto it =
        std::find_if(mActiveSessions.begin(), mActiveSessions.end(),
                     [aSessionId](const ActiveAmbientContextSession & session) { return session.GetSessionId() == aSessionId; });

    // Check if the element was actually found
    if (it == mActiveSessions.end())
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    for (const auto & context : aOldContext)
    {
        // Make sure the context actually exists in the session
        auto context_it =
            std::find_if(it->GetTrackedContexts().begin(), it->GetTrackedContexts().end(),
                         [context](const Structs::TrackedContext::Type & mContext) {
                             return ((context.identifiedContext.namespaceID == mContext.identifiedContext.namespaceID) &&
                                     (context.identifiedContext.tag == mContext.identifiedContext.tag));
                         });

        // Check if the element was actually found
        if (context_it == it->GetTrackedContexts().end())
        {
            return CHIP_ERROR_NOT_FOUND;
        }
    }

    // Remove the old context triggers from our current set for the session
    it->RemoveTrackedContext(aOldContext);

    // Now create the Perceived Context Event with newly removed context
    Events::PerceivedContext::Type perceivedEvent;

    perceivedEvent.sessionID                 = aSessionId;
    perceivedEvent.currentIdentifiedContexts = chip::MakeOptional(
        DataModel::List<const Structs::TrackedContext::Type>(it->GetTrackedContexts().data(), it->GetTrackedContexts().size()));
    perceivedEvent.expiredContexts =
        chip::MakeOptional(DataModel::List<const Structs::TrackedContext::Type>(aOldContext.data(), aOldContext.size()));

    VerifyOrReturnError(aContext->interactionContext.eventsGenerator.GenerateEvent(perceivedEvent, mEndpointId).has_value(),
                        CHIP_ERROR_INTERNAL, ChipLogError(Zcl, "Unable to generate PerceivedContext event"));

    return CHIP_NO_ERROR;
}

CHIP_ERROR AvAnalysisServerLogic::AnalysisSessionEnd(uint16_t aSessionId, ServerClusterContext * aContext)
{
    VerifyOrReturnError(aContext != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Make sure the provided session ID is one we know about
    auto it =
        std::find_if(mActiveSessions.begin(), mActiveSessions.end(),
                     [aSessionId](const ActiveAmbientContextSession & session) { return session.GetSessionId() == aSessionId; });

    // Check if the element was actually found
    if (it == mActiveSessions.end())
    {
        return CHIP_ERROR_NOT_FOUND;
    }

    // Now create the End Session Event
    Events::AnalysisSessionEnd::Type endSessionEvent;
    endSessionEvent.sessionID = aSessionId;

    VerifyOrReturnError(aContext->interactionContext.eventsGenerator.GenerateEvent(endSessionEvent, mEndpointId).has_value(),
                        CHIP_ERROR_INTERNAL, ChipLogError(Zcl, "Unable to generate EndSession event"));

    // Remove the session from our active contexts
    it = mActiveSessions.erase(it);

    return CHIP_NO_ERROR;
}

bool AvAnalysisServerLogic::IsContextPartOfActiveContextTriggers(
    const std::vector<AvAnalysis::Structs::TrackedContext::Type> & aContext)
{
    // Are the contexts part of our active set
    for (const auto & contextTrigger : aContext)
    {
        auto trigger_it = std::find_if(mActiveAmbientContextTriggers.begin(), mActiveAmbientContextTriggers.end(),
                                       [&contextTrigger](AmbientContextStorage & acs) {
                                           return acs.GetContext().namespaceID == contextTrigger.identifiedContext.namespaceID &&
                                               acs.GetContext().tag == contextTrigger.identifiedContext.tag;
                                       });

        if (trigger_it == mActiveAmbientContextTriggers.end())
        {
            return false;
        }
    }

    return true;
}

} // namespace Clusters
} // namespace app
} // namespace chip
