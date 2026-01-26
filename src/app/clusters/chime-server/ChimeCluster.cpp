/*
 *
 *    Copyright (c) 2024-2025 Project CHIP Authors
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

/****************************************************************************'
 * @file
 * @brief Implementation for the Chime Server Cluster
 ***************************************************************************/

#include "ChimeCluster.h"

#include <app/EventLogging.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Chime/Attributes.h>
#include <clusters/Chime/Commands.h>
#include <clusters/Chime/Events.h>
#include <clusters/Chime/Metadata.h>
#include <clusters/Chime/Structs.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Chime;
using namespace chip::app::Clusters::Chime::Attributes;
using chip::Protocols::InteractionModel::Status;
using ChimeSoundStructType = Structs::ChimeSoundStruct::Type;

namespace chip {
namespace app {
namespace Clusters {

ChimeCluster::ChimeCluster(EndpointId endpointId, ChimeDelegate & delegate) :
    DefaultServerCluster({ endpointId, Chime::Id }), mDelegate(delegate)
{
    mDelegate.SetChimeCluster(this);
}

ChimeCluster::~ChimeCluster()
{
    mDelegate.SetChimeCluster(nullptr);
}

CHIP_ERROR ChimeCluster::Startup(ServerClusterContext & context)
{
    ReturnErrorOnFailure(DefaultServerCluster::Startup(context));

    LoadPersistentAttributes();
    return CHIP_NO_ERROR;
}

CHIP_ERROR ChimeCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                          ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    static constexpr DataModel::AcceptedCommandEntry kAcceptedCommands[] = {
        Chime::Commands::PlayChimeSound::kMetadataEntry,
    };
    return builder.ReferenceExisting(kAcceptedCommands);
}

CHIP_ERROR ChimeCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    AttributeListBuilder listBuilder(builder);
    // Chime does not have optional attributes implemented yet (or exposed via options),
    // so we just return mandatory ones.
    return listBuilder.Append(Span(Chime::Attributes::kMandatoryMetadata), {});
}

// TODO: Migrate to use context.attributeStorage instead of SafeAttributePersistenceProvider
void ChimeCluster::LoadPersistentAttributes()
{
    // Load Active Chime ID
    uint8_t storedSelectedChime = 0;
    CHIP_ERROR err              = GetSafeAttributePersistenceProvider()->ReadScalarValue(
        ConcreteAttributePath(mPath.mEndpointId, Chime::Id, SelectedChime::Id), storedSelectedChime);
    if (err == CHIP_NO_ERROR)
    {
        mSelectedChime = storedSelectedChime;
    }
    else
    {
        // otherwise defaults
        ChipLogDetail(Zcl, "Chime: Unable to load the SelectedChime attribute from the KVS. Defaulting to %u", mSelectedChime);
    }

    // Load Enabled
    bool storedEnabled = false;
    err = GetSafeAttributePersistenceProvider()->ReadScalarValue(ConcreteAttributePath(mPath.mEndpointId, Chime::Id, Enabled::Id),
                                                                 storedEnabled);
    if (err == CHIP_NO_ERROR)
    {
        mEnabled = storedEnabled;
    }
    else
    {
        // otherwise take the default
        ChipLogDetail(Zcl, "Chime: Unable to load the Enabled attribute from the KVS. Defaulting to %u", mEnabled);
    }
}

DataModel::ActionReturnStatus ChimeCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                          AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(Chime::kRevision);
    case FeatureMap::Id:
        return encoder.Encode(static_cast<uint32_t>(0));
    case SelectedChime::Id:
        return encoder.Encode(mSelectedChime);
    case Enabled::Id:
        return encoder.Encode(mEnabled);
    case InstalledChimeSounds::Id:
        return encoder.EncodeList(
            [this](const auto & subEncoder) -> CHIP_ERROR { return this->EncodeSupportedChimeSounds(subEncoder); });
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

uint8_t ChimeCluster::GetSelectedChime() const
{
    return mSelectedChime;
}

bool ChimeCluster::GetEnabled() const
{
    return mEnabled;
}

// helper method to get the Chime Sounds one by one and encode into a list
CHIP_ERROR ChimeCluster::EncodeSupportedChimeSounds(const AttributeValueEncoder::ListEncodeHelper & encoder)
{

    for (uint8_t i = 0; true; i++)
    {
        ChimeSoundStructType chimeSound;

        // Get the chime sound
        // We pass in a MutableCharSpan to avoid any ownership issues - Delegate needs to use
        // CopyCharSpanToMutableCharSpan to copy data in
        char buffer[kMaxChimeSoundNameSize];
        MutableCharSpan name(buffer);
        auto err = mDelegate.GetChimeSoundByIndex(i, chimeSound.chimeID, name);

        // return if we've run off the end of the Chime Sound List on the delegate
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(err);

        // set the name on the struct
        chimeSound.name = name;

        // and now encode the struct
        ReturnErrorOnFailure(encoder.Encode(chimeSound));

        if (i == UINT8_MAX) // prevent overflow / infinite loop
        {
            // This should never happen if the delegate is well-behaved
            return CHIP_ERROR_INTERNAL;
        }
    }
    return CHIP_NO_ERROR;
}

// helper method to check if the chimeID param is supported by the delegate
bool ChimeCluster::IsSupportedChimeID(uint8_t chimeID)
{
    uint8_t supportedChimeID;
    for (uint8_t i = 0; mDelegate.GetChimeIDByIndex(i, supportedChimeID) != CHIP_ERROR_PROVIDER_LIST_EXHAUSTED; i++)
    {
        if (supportedChimeID == chimeID)
        {
            return true;
        }

        if (i == UINT8_MAX) // prevent overflow / infinite loop
        {
            // This should never happen if the delegate is well-behaved
            return false;
        }
    }
    return false;
}

DataModel::ActionReturnStatus ChimeCluster::WriteAttribute(const DataModel::WriteAttributeRequest & request,
                                                           AttributeValueDecoder & decoder)
{
    switch (request.path.mAttributeId)
    {
    case SelectedChime::Id: {
        uint8_t newValue;
        ReturnErrorOnFailure(decoder.Decode(newValue));
        return DataModel::ActionReturnStatus(SetSelectedChime(newValue));
    }
    case Enabled::Id: {
        bool newValue;
        ReturnErrorOnFailure(decoder.Decode(newValue));
        return DataModel::ActionReturnStatus(SetEnabled(newValue));
    }
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

Status ChimeCluster::SetSelectedChime(uint8_t chimeID)
{
    if (!IsSupportedChimeID(chimeID))
    {
        return Protocols::InteractionModel::Status::NotFound;
    }
    if (SetAttributeValue(mSelectedChime, chimeID, Attributes::SelectedChime::Id))
    {
        // TODO: Migrate to use context.attributeStorage
        TEMPORARY_RETURN_IGNORED GetSafeAttributePersistenceProvider()->WriteScalarValue(
            { mPath.mEndpointId, Chime::Id, Attributes::SelectedChime::Id }, mSelectedChime);
    }
    return Protocols::InteractionModel::Status::Success;
}

Status ChimeCluster::SetEnabled(bool enabled)
{
    if (SetAttributeValue(mEnabled, enabled, Attributes::Enabled::Id))
    {
        // TODO: Migrate to use context.attributeStorage
        TEMPORARY_RETURN_IGNORED GetSafeAttributePersistenceProvider()->WriteScalarValue(
            { mPath.mEndpointId, Chime::Id, Attributes::Enabled::Id }, mEnabled);
    }
    return Protocols::InteractionModel::Status::Success;
}

std::optional<DataModel::ActionReturnStatus> ChimeCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                         chip::TLV::TLVReader & input_arguments,
                                                                         CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Commands::PlayChimeSound::Id: {
        ChipLogDetail(Zcl, "Chime: PlayChimeSound");

        Commands::PlayChimeSound::DecodableType commandData;
        ReturnErrorOnFailure(commandData.Decode(input_arguments));
        return HandlePlayChimeSound(*handler, request.path, commandData);
    }
    default:
        return Status::UnsupportedCommand;
    }
}

DataModel::ActionReturnStatus ChimeCluster::HandlePlayChimeSound(CommandHandler & aHandler, const ConcreteCommandPath & aPath,
                                                                 const Commands::PlayChimeSound::DecodableType & commandData)
{
    // Immediately return if we're not enabled
    //
    if (!mEnabled)
    {
        return Status::Success;
    }

    // If we have a provided Chime ID, ensure it is valid
    //
    uint8_t chimeIDToUse = mSelectedChime;
    if (commandData.chimeID.HasValue())
    {
        if (!IsSupportedChimeID(commandData.chimeID.Value()))
        {
            ChipLogError(Zcl, "Chime: Unknown Chime ID provided to PlayChimeSound");
            return Status::NotFound;
        }

        chimeIDToUse = commandData.chimeID.Value();
    }

    Status status = mDelegate.PlayChimeSound(chimeIDToUse);
    if (status == Status::Success)
    {
        // Generate the event
        GenerateChimeStartedPlayingEvent(chimeIDToUse);
    }
    return status;
}

void ChimeCluster::GenerateChimeStartedPlayingEvent(const uint8_t chimeID)
{
    Events::ChimeStartedPlaying::Type event;
    EventNumber eventNumber;

    event.chimeID = chimeID;

    CHIP_ERROR err = LogEvent(event, GetPaths().data()->mEndpointId, eventNumber);
    if (CHIP_NO_ERROR != err)
    {
        ChipLogError(Zcl, "Unable to generate ChimeStartedPlaying event");
    }
}

} // namespace Clusters
} // namespace app
} // namespace chip
