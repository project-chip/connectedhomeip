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
                                             const std::vector<Descriptor::Structs::SemanticTagStruct::Type> & aSupportedAmbientContexts) :
    mEndpointId(aEndpointId),
    mFeatures(aFeatures),
    mSupportedAmbientContexts(aSupportedAmbientContexts)
    {}

AvAnalysisServerLogic::~AvAnalysisServerLogic() {}

CHIP_ERROR AvAnalysisServerLogic::Startup()
{
    // Make sure mandated Features are set
    //
    VerifyOrReturnError(HasFeature(Feature::kLocalContextDetection) ^ HasFeature(Feature::kRemoteContextDetection),
                        CHIP_ERROR_INVALID_ARGUMENT,
                        ChipLogError(Zcl,
                                     "AvAnalysis: Feature configuration error. One and only one of "
                                     "Local or Remote Context Detection must be supported"));

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
        for (const auto & activeContextTrigger : mActiveAmbientContextTriggers)
        {
            ReturnErrorOnFailure(encoder.Encode(activeContextTrigger));
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
    return CHIP_NO_ERROR;
}

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

std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleLocalEnableContextTriggers(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::EnableContextTriggers::DecodableType & commandData)
{   
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

            if (it == mSupportedAmbientContexts.end()) 
            {
                return Status::DynamicConstraintError;
            }
            
            // The trigger context is valid, now check the ZoneIDs, which can only be present of PERZONEDETECT is set
            //
            if (contextTrigger.zoneIDs.HasValue())
            {
                if (!HasFeature(AvAnalysis::Feature::kPerZoneContextDetection)) 
                {
                    return Status::InvalidCommand;
                }
            
                // Verify via the delegate that the provided list of ZoneIDs contains values present in ZoneManagement only
                // if the zoneIDs we have are not Null.
                //
                if (!contextTrigger.zoneIDs.Value().IsNull())
                {
                    CHIP_ERROR err = mDelegate->VerifyZoneIDsAreValid(contextTrigger.zoneIDs.Value().Value());
                    if (err != CHIP_NO_ERROR)
                    {
                        return Status::NotFound;
                    }
                }
            }
            
            // Check with the delegate that additional contexts can be added
            //
            if (!mDelegate->CanAddContextTriggers())
            { 
                return Status::ResourceExhausted;
            }
            
            // Update our active trigger set with this new context. 
            // If the context exists, update the zone IDs, otherise add a new entry
            //
            auto it2 = std::find_if(mActiveAmbientContextTriggers.begin(), mActiveAmbientContextTriggers.end(), 
                [&contextTrigger](const AvAnalysis::Structs::ContextTriggerStruct::Type& ct) {
                    return ct.context.namespaceID == contextTrigger.context.namespaceID && ct.context.tag == contextTrigger.context.tag;
            });

            // Convert the ZoneIDs from a DecodableList to a format we can use to instantiat a List
            //
            size_t size;
            CHIP_ERROR err = contextTrigger.zoneIDs.Value().Value().ComputeSize(&size);
            if (err != CHIP_NO_ERROR) 
            {
                return Status::Failure;
            }
            
            std::vector<uint16_t> zoneIDs(size);
            auto zone_iter = contextTrigger.zoneIDs.Value().Value().begin();
                
            while (zone_iter.Next())
            {
                zoneIDs.push_back(zone_iter.GetValue());
            }
            
            if (it2 == mActiveAmbientContextTriggers.end()) 
            {
                // This is a new context, add to the end
                //
                AvAnalysis::Structs::ContextTriggerStruct::Type newContextTrigger;
                newContextTrigger.context = contextTrigger.context;
                newContextTrigger.zoneIDs = chip::MakeOptional(DataModel::MakeNullable(DataModel::List<const uint16_t>(zoneIDs.data(), size)));
                mActiveAmbientContextTriggers.push_back(newContextTrigger);
            }
            else
            {
                // Update just the ZoneIDs
                it2->zoneIDs = chip::MakeOptional(DataModel::MakeNullable(DataModel::List<const uint16_t>(zoneIDs.data(), size)));
            }
        }
    }
    else
    {
        // Provided set is null, meaning all known context triggers should be activated
        // First check with the delegate that additional contexts can be added
        //
        if (!mDelegate->CanAddContextTriggers())
        { 
            return Status::ResourceExhausted; 
        }
        
        // Set the active triggers to be the supported triggers
        mActiveAmbientContextTriggers.clear();
        
        for (const auto& supportedContext : mSupportedAmbientContexts) 
        {
            AvAnalysis::Structs::ContextTriggerStruct::Type contextTrigger;
            contextTrigger.context = supportedContext;
            contextTrigger.zoneIDs = chip::MakeOptional(DataModel::NullNullable);
            mActiveAmbientContextTriggers.push_back(contextTrigger);
        }
    }
    
    // Inform the delegate of the new active context set. The delegate will read the updated contents
    // of the attribute
    //
    mDelegate->ActiveAmbientContextTriggersUpdated();
    
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleRemoteEnableContextTriggers(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::EnableContextTriggers::DecodableType & commandData)
{
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleDisableContextTriggers(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::DisableContextTriggers::DecodableType & commandData)
{
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleEstablishAnalysisStream(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::EstablishAnalysisStream::DecodableType & commandData)
{
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleActivateAnalysisStream(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::ActivateAnalysisStream::DecodableType & commandData)
{
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleDeactivateAnalysisStream(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::DeactivateAnalysisStream::DecodableType & commandData)
{
     return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> AvAnalysisServerLogic::HandleRemoveAnalysisStream(
    CommandHandler & handler, const ConcreteCommandPath & commandPath,
    const AvAnalysis::Commands::RemoveAnalysisStream::DecodableType & commandData)
{
     return Status::Success;
}


} // namespace Clusters
} // namespace app
} // namespace chip
