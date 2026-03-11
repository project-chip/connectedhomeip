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
#include <clusters/Actions/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "ActionsStructs.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;
using namespace chip::app::Clusters::Actions::Attributes;
using namespace chip::Protocols::InteractionModel;
namespace {
static constexpr size_t kMaxEndpointListLength = 256u;
static constexpr size_t kMaxActionListLength   = 256u;
} // namespace
void ActionsCluster::OnStateChanged(uint16_t aActionId, uint32_t aInvokeId, ActionStateEnum aActionState)
{
    ChipLogProgress(Zcl, "ActionsCluster: OnStateChanged");
    VerifyOrReturn(mContext != nullptr, ChipLogError(Zcl, "ActionsCluster: mContext is null, cannot generate event"));

    // Generate StateChanged event
    Events::StateChanged::Type event{ aActionId, aInvokeId, aActionState };

    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

void ActionsCluster::OnActionFailed(uint16_t aActionId, uint32_t aInvokeId, ActionStateEnum aActionState,
                                    ActionErrorEnum aActionError)
{
    ChipLogProgress(Zcl, "ActionsCluster: OnActionFailed");
    VerifyOrReturn(mContext != nullptr, ChipLogError(Zcl, "ActionsCluster: mContext is null, cannot generate event"));

    // Generate ActionFailed event
    Events::ActionFailed::Type event{ aActionId, aInvokeId, aActionState, aActionError };

    mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId);
}

DataModel::ActionReturnStatus ActionsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                            AttributeValueEncoder & encoder)
{
    switch (request.path.mAttributeId)
    {
    case ClusterRevision::Id:
        return encoder.Encode(Actions::kRevision);
    case FeatureMap::Id:
        // The Actions cluster currently doesn't use feature maps, so we encode 0
        return encoder.Encode(static_cast<uint32_t>(0));
    case SetupURL::Id: {
        if (!mOptionalAttributes.IsSet(SetupURL::Id))
        {
            return Protocols::InteractionModel::Status::UnsupportedAttribute;
        }
        // If it's supported but we don't have a value, that's an internal error
        if (!mSetupURL.has_value())
        {
            return Protocols::InteractionModel::Status::Failure;
        }
        return encoder.Encode(mSetupURL.value());
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

Status ActionsCluster::ValidateActionCommand(uint16_t actionID, CommandId commandId)
{
    uint16_t actionIndex = kMaxActionListLength;
    VerifyOrReturnValue(mDelegate.HaveActionWithId(actionID, actionIndex), Status::NotFound);
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        CHIP_ERROR err = mDelegate.ReadActionAtIndex(actionIndex, action);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(Zcl, "Failed to read action at index %u: %" CHIP_ERROR_FORMAT, actionIndex, err.Format());
            return Status::Failure;
        }
        VerifyOrReturnValue((action.supportedCommands.Raw() & (1 << commandId)), Status::InvalidCommand);
    }

    return Status::Success;
}

std::optional<DataModel::ActionReturnStatus> ActionsCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                           chip::TLV::TLVReader & input_arguments,
                                                                           CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Actions::Commands::InstantAction::Id: {
        Commands::InstantAction::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandleInstantAction(commandData.actionID, commandData.invokeID);
    }
    case Actions::Commands::InstantActionWithTransition::Id: {
        Commands::InstantActionWithTransition::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandleInstantActionWithTransition(commandData.actionID, commandData.transitionTime, commandData.invokeID);
    }
    case Actions::Commands::StartAction::Id: {
        Commands::StartAction::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandleStartAction(commandData.actionID, commandData.invokeID);
    }
    case Actions::Commands::StartActionWithDuration::Id: {
        Commands::StartActionWithDuration::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandleStartActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    }
    case Actions::Commands::StopAction::Id: {
        Commands::StopAction::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandleStopAction(commandData.actionID, commandData.invokeID);
    }
    case Actions::Commands::PauseAction::Id: {
        Commands::PauseAction::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandlePauseAction(commandData.actionID, commandData.invokeID);
    }
    case Actions::Commands::PauseActionWithDuration::Id: {
        Commands::PauseActionWithDuration::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandlePauseActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    }
    case Actions::Commands::ResumeAction::Id: {
        Commands::ResumeAction::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandleResumeAction(commandData.actionID, commandData.invokeID);
    }
    case Actions::Commands::EnableAction::Id: {
        Commands::EnableAction::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandleEnableAction(commandData.actionID, commandData.invokeID);
    }
    case Actions::Commands::EnableActionWithDuration::Id: {
        Commands::EnableActionWithDuration::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandleEnableActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    }
    case Actions::Commands::DisableAction::Id: {
        Commands::DisableAction::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandleDisableAction(commandData.actionID, commandData.invokeID);
    }
    case Actions::Commands::DisableActionWithDuration::Id: {
        Commands::DisableActionWithDuration::DecodableType commandData;
        VerifyOrReturnValue(DataModel::Decode(input_arguments, commandData) == CHIP_NO_ERROR, Status::InvalidCommand);

        Status status = ValidateActionCommand(commandData.actionID, request.path.mCommandId);
        VerifyOrReturnValue(status == Status::Success, status);

        return mDelegate.HandleDisableActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    }
    }

    // Fall back to default implementation for unhandled commands
    return DefaultServerCluster::InvokeCommand(request, input_arguments, handler);
}

#include <app/server-cluster/AttributeListBuilder.h>

CHIP_ERROR ActionsCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using namespace chip::app::Clusters::Actions::Attributes;
    constexpr DataModel::AttributeEntry optionalAttributesMeta[] = {
        SetupURL::kMetadataEntry,
    };

    AttributeListBuilder listBuilder(builder);
    return listBuilder.Append(Span(kMandatoryMetadata), Span{ optionalAttributesMeta }, mOptionalAttributes);
}

CHIP_ERROR ActionsCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace chip::app::Clusters::Actions::Commands;

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
    NotifyAttributeChanged(Attributes::ActionList::Id);
}

void ActionsCluster::EndpointListsModified()
{
    NotifyAttributeChanged(Attributes::EndpointLists::Id);
}
