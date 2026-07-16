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
#include <app/InteractionModelEngine.h>
#include <app/clusters/av-analysis-server/AvAnalysisCluster.h>
#include <app/reporting/reporting.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <app/util/util.h>
#include <clusters/AvAnalysis/Metadata.h>
#include <lib/core/CHIPSafeCasts.h>
#include <lib/support/DefaultStorageKeyAllocator.h>
#include <protocols/interaction_model/StatusCode.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::AvAnalysis;
using namespace chip::app::Clusters::AvAnalysis::Structs;
using namespace chip::app::Clusters::AvAnalysis::Attributes;
using namespace Protocols::InteractionModel;

namespace chip {
namespace app {
namespace Clusters {

AvAnalysisServerLogic::AvAnalysisServerLogic(EndpointId aEndpointId,
                                             BitFlags<Feature> aFeatures,
                                             const std::vector<Descriptor::Structs::SemanticTagStruct::Type> & aSupportedAmbientContexts,
                                             DataModel::Nullable<uint8_t> aMaxZones) :
    mEndpointId(aEndpointId),
    mFeatures(aFeatures),
    mSupportedAmbientContexts(aSupportedAmbientContexts),
    mMaxZones(aMaxZones)
    {}

AvAnalysisServerLogic::~AvAnalysisServerLogic() {}

CHIP_ERROR AvAnalysisServerLogic::Startup(AttributePersistenceProvider & aAttributePersistenceProvider)
{
    mAttributePersistenceProvider = &aAttributePersistenceProvider;

    // Make sure mandated Features are set, one and only one of Local or Remote has to be set
    //
    VerifyOrReturnError(HasFeature(Feature::kLocalContextDetection) ^ HasFeature(Feature::kRemoteContextDetection),
                        CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl,
                                     "AvAnalysis: Feature configuration error. One and only one of "
                                     "Local or Remote Context Detection must be supported"));
                    
    // If we don't have PerZoneSensivity then mMaxZones has to be Null
    VerifyOrReturnError(!(HasFeature(Feature::kPerZoneContextDetection) ^ !mMaxZones.IsNull()),
                        CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl,
                                     "AvAnalysis: If Per Zone Sensitivity is set, Zones must be present, and vice versa"));

    LoadPersistentAttributes();
    return CHIP_NO_ERROR;
}

void AvAnalysisServerLogic::Shutdown()
{
    mDelegate->ShutdownApp();
}

bool AvAnalysisServerLogic::HasFeature(Feature aFeature) const
{
    return mFeatures.Has(aFeature);
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
        for (const auto & analysisStream : mAnalysisStreams)
        {
            ReturnErrorOnFailure(encoder.Encode(analysisStream));
        }

        return CHIP_NO_ERROR;
    });
}

// Attribute mutators
CHIP_ERROR AvAnalysisServerLogic::SetMaxAnalysisStreamCount(uint8_t aMaxAnalysisStreamCount)
{
    VerifyOrReturnError(HasFeature(Feature::kRemoteContextDetection), CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute));
    mMaxAnalysisStreamCount = aMaxAnalysisStreamCount;
    return CHIP_NO_ERROR;
}

CHIP_ERROR AvAnalysisServerLogic::SetTrackingEnabled(bool aTrackingEnabled)
{
    mTrackingEnabled = aTrackingEnabled;
    MarkDirty(AvAnalysis::Attributes::TrackingEnabled::Id);
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

    // Signal delegate that all persistent configuration attributes have been loaded.
    TEMPORARY_RETURN_IGNORED mDelegate->PersistentAttributesLoadedCallback();
}

/**
 * Handler for the EnableContextTriggers command. This invokes specific methods for local vs. remote as the logic
 * associated with each is different
 */
std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleEnableContextTriggers(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::EnableContextTriggers::DecodableType & commandData)
{
    // Are we locally or remotely processing, handle appropriately
    //
    if (HasFeature(AvAnalysis::Feature::kLocalContextDetection)) {
        return HandleLocalEnableContextTriggers(handler, commandPath, commandData);
    }
    else
    {
        return HandleRemoteEnableContextTriggers(handler, commandPath, commandData);
    }
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
                [&contextTrigger](const Descriptor::Structs::SemanticTagStruct::Type& stt) {
                    return stt.namespaceID == contextTrigger.context.namespaceID && stt.tag == contextTrigger.context.tag;
            });

            VerifyOrReturnError(it != mSupportedAmbientContexts.end(), Status::ConstraintError);
            
            // The trigger context is valid, now check the ZoneIDs, which can only be present of PERZONEDETECT is set, likewise,
            // if we have the feature, then ZoneIDs have to be present
            //
            bool hasZoneIDs = contextTrigger.zoneIDs.HasValue();
            bool hasNonNullZoneIDs = false;
            
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
                    CHIP_ERROR err = mDelegate->VerifyZoneIDsAreValid(contextTrigger.zoneIDs.Value().Value());
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
                    return acs.GetContext().namespaceID == contextTrigger.context.namespaceID && acs.GetContext().tag == contextTrigger.context.tag;
            });

            // Get the ZoneIDs, if present, into a format that can be used, that is convert the DecodableList to a List
            //
            std::vector<uint16_t> zoneIDs;
            size_t size;
            
            if (hasNonNullZoneIDs)
            {
                CHIP_ERROR err = contextTrigger.zoneIDs.Value().Value().ComputeSize(&size);
                VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);
            
                auto zone_iter = contextTrigger.zoneIDs.Value().Value().begin();
                
                while (zone_iter.Next())
                {
                    zoneIDs.push_back(zone_iter.GetValue());
                }
            }

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
        
        for (const auto& supportedContext : mSupportedAmbientContexts) 
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
std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleDisableContextTriggers(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
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
                [&contextTrigger](AmbientContextStorage& acs) {
                    return acs.GetContext().namespaceID == contextTrigger.context.namespaceID && acs.GetContext().tag == contextTrigger.context.tag;
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
                    CHIP_ERROR err = mDelegate->VerifyZoneIDsAreValid(contextTrigger.zoneIDs.Value().Value());
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
                    for (const auto& item : currentZoneIds)
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

/**
 * Placeholder method for when the functionality for remote context detection is implemented
 */
std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleEstablishAnalysisStream(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::EstablishAnalysisStream::DecodableType & commandData)
{
    return Status::Success;
}

/**
 * Placeholder method for when the functionality for remote context detection is implemented
 */
std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleActivateAnalysisStream(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::ActivateAnalysisStream::DecodableType & commandData)
{
    return Status::Success;
}

/**
 * Placeholder method for when the functionality for remote context detection is implemented
 */
std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleDeactivateAnalysisStream(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::DeactivateAnalysisStream::DecodableType & commandData)
{
     return Status::Success;
}

/**
 * Placeholder method for when the functionality for remote context detection is implemented
 */
std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleRemoveAnalysisStream(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::RemoveAnalysisStream::DecodableType & commandData)
{
     return Status::Success;
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


} // namespace Clusters
} // namespace app
} // namespace chip
