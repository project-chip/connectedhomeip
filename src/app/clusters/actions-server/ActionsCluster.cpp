/*
 *
 *    Copyright (c) 2024-2026 Project CHIP Authors
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

#include <app/clusters/actions-server/ActionsCluster.h>
#include <app/clusters/actions-server/ActionsStructs.h>
#include <app/server-cluster/AttributeListBuilder.h>
#include <clusters/Actions/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Protocols::InteractionModel;

namespace chip::app::Clusters {

using namespace Actions;

namespace {
static constexpr size_t kMaxEndpointListLength = 256u;
static constexpr size_t kMaxActionListLength   = 256u;
} // namespace

void ActionsCluster::GenerateEvent(const Events::StateChanged::Type & event)
{
    VerifyOrReturn(mContext != nullptr, ChipLogError(Zcl, "ActionsCluster: mContext is null, cannot generate event"));
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void ActionsCluster::GenerateEvent(const Events::ActionFailed::Type & event)
{
    VerifyOrReturn(mContext != nullptr, ChipLogError(Zcl, "ActionsCluster: mContext is null, cannot generate event"));
    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

DataModel::ActionReturnStatus ActionsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                            AttributeValueEncoder & encoder)
{
    using namespace Actions::Attributes;
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(Actions::kRevision);
    case FeatureMap::Id:
        // The Actions cluster currently doesn't use feature maps, so we encode 0
        return encoder.Encode(static_cast<uint32_t>(0));
    case SetupURL::Id: {
        if (!mSetupURL.has_value())
        {
            return Protocols::InteractionModel::Status::Failure;
        }
        // Safety fallback: if data() pointer is nullptr, encode empty string with valid pointer
        const CharSpan & urlToEncode = mSetupURL.value_or(""_span);
        return encoder.Encode(urlToEncode);
    }
    case ActionList::Id:
        return encoder.EncodeList([this, &request](const auto & listEncoder) -> CHIP_ERROR {
            return this->ReadActionListAttribute(request, listEncoder);
        });
    case EndpointLists::Id:
        return encoder.EncodeList([this, &request](const auto & listEncoder) -> CHIP_ERROR {
            return this->ReadEndpointListAttribute(request, listEncoder);
        });
    default:
        return Protocols::InteractionModel::Status::UnsupportedAttribute;
    }
}

CHIP_ERROR ActionsCluster::ReadActionListAttribute(const DataModel::ReadAttributeRequest & request,
                                                   const AttributeValueEncoder::ListEncodeHelper & aEncoder)
{
    for (uint16_t i = 0; i < kMaxActionListLength; i++)
    {
        ActionStructStorage action;
        CHIP_ERROR err = mDelegate.ReadActionAtIndex(i, action);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(aEncoder.Encode(action));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ActionsCluster::ReadEndpointListAttribute(const DataModel::ReadAttributeRequest & request,
                                                     const AttributeValueEncoder::ListEncodeHelper & aEncoder)
{
    for (uint16_t i = 0; i < kMaxEndpointListLength; i++)
    {
        EndpointListStorage epList;
        CHIP_ERROR err = mDelegate.ReadEndpointListAtIndex(i, epList);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(aEncoder.Encode(epList));
    }
    return CHIP_NO_ERROR;
}

Status ActionsCluster::ValidateActionExists(uint16_t actionID, uint16_t & outActionIndex)
{
    outActionIndex = kMaxActionListLength;
    VerifyOrReturnValue(mDelegate.HaveActionWithId(actionID, outActionIndex), Status::NotFound);
    return Status::Success;
}

Status ActionsCluster::ValidateCommandSupported(uint16_t actionIndex, CommandId commandId)
{
    // If index wasn't populated by the delegate but it exists, bypass command check
    if (actionIndex == kMaxActionListLength)
    {
        return Status::Success;
    }

    ActionStructStorage action;
    CHIP_ERROR err = mDelegate.ReadActionAtIndex(actionIndex, action);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Zcl, "Failed to read action at index %u: %" CHIP_ERROR_FORMAT, actionIndex, err.Format());
        return Status::Failure;
    }

    // Ensure the command is supported via bitmask
    VerifyOrReturnValue((action.supportedCommands.Raw() & (1 << commandId)), Status::InvalidCommand);
    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> ActionsCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                           chip::TLV::TLVReader & input_arguments,
                                                                           CommandHandler * handler)
{
    using namespace Actions::Commands;

    // Common validation shared by every command: decode, find the action, check support.
#define DECODE_AND_VALIDATE(CommandType)                                                                                           \
    CommandType::DecodableType commandData;                                                                                        \
    VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);                 \
    uint16_t actionIndex = kMaxActionListLength;                                                                                   \
    {                                                                                                                              \
        Status s = ValidateActionExists(commandData.actionID, actionIndex);                                                        \
        VerifyOrReturnValue(s == Status::Success, s);                                                                              \
        s = ValidateCommandSupported(actionIndex, request.path.mCommandId);                                                        \
        VerifyOrReturnValue(s == Status::Success, s);                                                                              \
    }

    switch (request.path.mCommandId)
    {
    case InstantAction::Id: {
        DECODE_AND_VALIDATE(InstantAction)
        return mDelegate.HandleInstantAction(commandData.actionID, commandData.invokeID);
    }
    case InstantActionWithTransition::Id: {
        DECODE_AND_VALIDATE(InstantActionWithTransition)
        return mDelegate.HandleInstantActionWithTransition(commandData.actionID, commandData.transitionTime, commandData.invokeID);
    }
    case StartAction::Id: {
        DECODE_AND_VALIDATE(StartAction)
        return mDelegate.HandleStartAction(commandData.actionID, commandData.invokeID);
    }
    case StartActionWithDuration::Id: {
        DECODE_AND_VALIDATE(StartActionWithDuration)
        return mDelegate.HandleStartActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    }
    case StopAction::Id: {
        DECODE_AND_VALIDATE(StopAction)
        return mDelegate.HandleStopAction(commandData.actionID, commandData.invokeID);
    }
    case PauseAction::Id: {
        DECODE_AND_VALIDATE(PauseAction)
        return mDelegate.HandlePauseAction(commandData.actionID, commandData.invokeID);
    }
    case PauseActionWithDuration::Id: {
        DECODE_AND_VALIDATE(PauseActionWithDuration)
        return mDelegate.HandlePauseActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    }
    case ResumeAction::Id: {
        DECODE_AND_VALIDATE(ResumeAction)
        return mDelegate.HandleResumeAction(commandData.actionID, commandData.invokeID);
    }
    case EnableAction::Id: {
        DECODE_AND_VALIDATE(EnableAction)
        return mDelegate.HandleEnableAction(commandData.actionID, commandData.invokeID);
    }
    case EnableActionWithDuration::Id: {
        DECODE_AND_VALIDATE(EnableActionWithDuration)
        return mDelegate.HandleEnableActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    }
    case DisableAction::Id: {
        DECODE_AND_VALIDATE(DisableAction)
        return mDelegate.HandleDisableAction(commandData.actionID, commandData.invokeID);
    }
    case DisableActionWithDuration::Id: {
        DECODE_AND_VALIDATE(DisableActionWithDuration)
        return mDelegate.HandleDisableActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    }
    }

#undef DECODE_AND_VALIDATE

    return Status::UnsupportedCommand;
}

CHIP_ERROR ActionsCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using namespace Actions::Attributes;
    constexpr DataModel::AttributeEntry optionalAttributesMeta[] = {
        SetupURL::kMetadataEntry,
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), Span{ optionalAttributesMeta }, mOptionalAttributes);
}

CHIP_ERROR ActionsCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace Actions::Commands;

    static const DataModel::AcceptedCommandEntry kCommands[] = {
        InstantAction::kMetadataEntry,
        InstantActionWithTransition::kMetadataEntry,
        StartAction::kMetadataEntry,
        StartActionWithDuration::kMetadataEntry,
        StopAction::kMetadataEntry,
        PauseAction::kMetadataEntry,
        PauseActionWithDuration::kMetadataEntry,
        ResumeAction::kMetadataEntry,
        EnableAction::kMetadataEntry,
        EnableActionWithDuration::kMetadataEntry,
        DisableAction::kMetadataEntry,
        DisableActionWithDuration::kMetadataEntry,
    };

    return builder.ReferenceExisting(kCommands);
}

void ActionsCluster::ActionListModified()
{
    NotifyAttributeChanged(Actions::Attributes::ActionList::Id);
}

void ActionsCluster::EndpointListsModified()
{
    NotifyAttributeChanged(Actions::Attributes::EndpointLists::Id);
}

} // namespace chip::app::Clusters
