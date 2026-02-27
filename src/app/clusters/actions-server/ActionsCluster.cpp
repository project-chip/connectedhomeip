/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "ActionsCluster.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/reporting/reporting.h>
#include <clusters/Actions/Metadata.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;
using namespace chip::app::Clusters::Actions::Attributes;
using namespace chip::Protocols::InteractionModel;

CHIP_ERROR ActionsCluster::Startup(ServerClusterContext & context)
{
    mContext = &context;
    return DefaultServerCluster::Startup(context);
}

void ActionsCluster::Shutdown(ClusterShutdownType type)
{
    // No cleanup needed
}

void ActionsCluster::OnStateChanged(uint16_t aActionId, uint32_t aInvokeId, ActionStateEnum aActionState)
{
    ChipLogProgress(Zcl, "ActionsCluster: OnStateChanged");

    // Generate StateChanged event
    Events::StateChanged::Type event{ aActionId, aInvokeId, aActionState };

    if (!mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId))
    {
        ChipLogError(Zcl, "ActionsCluster: Failed to generate OnStateChanged event");
    }
}

void ActionsCluster::OnActionFailed(uint16_t aActionId, uint32_t aInvokeId, ActionStateEnum aActionState,
                                    ActionErrorEnum aActionError)
{
    ChipLogProgress(Zcl, "ActionsCluster: OnActionFailed");

    // Generate ActionFailed event
    Events::ActionFailed::Type event{ aActionId, aInvokeId, aActionState, aActionError };

    if (!mContext->interactionContext.eventsGenerator.GenerateEvent(event, mPath.mEndpointId))
    {
        ChipLogError(Zcl, "ActionsCluster: Failed to generate OnActionFailed event");
    }
}

DataModel::ActionReturnStatus ActionsCluster::ReadAttribute(const DataModel::ReadAttributeRequest & request,
                                                            AttributeValueEncoder & encoder)
{
    VerifyOrDie(request.path.mClusterId == Actions::Id);

    switch (request.path.mAttributeId)
    {
    case ActionList::Id: {
        CHIP_ERROR err = encoder.EncodeList([this, &request](const auto & listEncoder) -> CHIP_ERROR {
            return this->ReadActionListAttribute(request, listEncoder);
        });
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        return Protocols::InteractionModel::Status::Success;
    }
    case EndpointLists::Id: {
        CHIP_ERROR err = encoder.EncodeList([this, &request](const auto & listEncoder) -> CHIP_ERROR {
            return this->ReadEndpointListAttribute(request, listEncoder);
        });
        if (err != CHIP_NO_ERROR)
        {
            return err;
        }
        return Protocols::InteractionModel::Status::Success;
    }
    default:
        break;
    }
    return CHIP_NO_ERROR;
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

bool ActionsCluster::HaveActionWithId(uint16_t aActionId, uint16_t & aActionIndex)
{
    return mDelegate.HaveActionWithId(aActionId, aActionIndex);
}

std::optional<DataModel::ActionReturnStatus> ActionsCluster::InvokeCommand(const DataModel::InvokeRequest & request,
                                                                           chip::TLV::TLVReader & input_arguments,
                                                                           CommandHandler * handler)
{
    switch (request.path.mCommandId)
    {
    case Actions::Commands::InstantAction::Id: {
        Commands::InstantAction::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandleInstantAction(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::InstantActionWithTransition::Id: {
        Commands::InstantActionWithTransition::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandleInstantActionWithTransition(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::StartAction::Id: {
        Commands::StartAction::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandleStartAction(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::StartActionWithDuration::Id: {
        Commands::StartActionWithDuration::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandleStartActionWithDuration(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::StopAction::Id: {
        Commands::StopAction::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandleStopAction(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::PauseAction::Id: {
        Commands::PauseAction::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandlePauseAction(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::PauseActionWithDuration::Id: {
        Commands::PauseActionWithDuration::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandlePauseActionWithDuration(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::ResumeAction::Id: {
        Commands::ResumeAction::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandleResumeAction(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::EnableAction::Id: {
        Commands::EnableAction::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandleEnableAction(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::EnableActionWithDuration::Id: {
        Commands::EnableActionWithDuration::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandleEnableActionWithDuration(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::DisableAction::Id: {
        Commands::DisableAction::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandleDisableAction(request, handler, commandData);
        return std::nullopt;
    }
    case Actions::Commands::DisableActionWithDuration::Id: {
        Commands::DisableActionWithDuration::DecodableType commandData;
        if (DataModel::Decode(input_arguments, commandData) != CHIP_NO_ERROR)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        HandleDisableActionWithDuration(request, handler, commandData);
        return std::nullopt;
    }
    }

    // Fall back to default implementation for unhandled commands
    return DefaultServerCluster::InvokeCommand(request, input_arguments, handler);
}

#include <app/server-cluster/AttributeListBuilder.h>

CHIP_ERROR ActionsCluster::Attributes(const ConcreteClusterPath & path, ReadOnlyBufferBuilder<DataModel::AttributeEntry> & builder)
{
    using namespace chip::app::Clusters::Actions::Attributes;

    // If Actions has any optional attributes, you evaluate them here.
    // If SetupURL is optional and you conditionally support it, you'd do: { true, SetupURL::kMetadataEntry }
    // AttributeListBuilder::OptionalAttributeEntry optionalAttributes[] = {
    //     // Add optional attributes here if your ActionsDelegate supports them.
    //     // Leave empty if everything is mandatory.
    // };

    AttributeListBuilder listBuilder(builder);

    // This automatically pulls in ActionList, EndpointLists, ClusterRevision, FeatureMap
    // assuming they are marked as mandatory in the ZCL specification!
    return listBuilder.Append(Span(chip::app::Clusters::Actions::Attributes::kMandatoryMetadata), {});
}

CHIP_ERROR ActionsCluster::AcceptedCommands(const ConcreteClusterPath & path,
                                            ReadOnlyBufferBuilder<DataModel::AcceptedCommandEntry> & builder)
{
    using namespace chip::app::Clusters::Actions::Commands;

    // Explicitly list out the commands supported by the Actions cluster
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

void ActionsCluster::HandleInstantAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                         const Commands::InstantAction::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandleInstantAction(commandData.actionID, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandleInstantActionWithTransition(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                                       const Commands::InstantActionWithTransition::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status =
        mDelegate.HandleInstantActionWithTransition(commandData.actionID, commandData.transitionTime, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandleStartAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                       const Commands::StartAction::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandleStartAction(commandData.actionID, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandleStartActionWithDuration(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                                   const Commands::StartActionWithDuration::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandleStartActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandleStopAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                      const Commands::StopAction::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandleStopAction(commandData.actionID, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandlePauseAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                       const Commands::PauseAction::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandlePauseAction(commandData.actionID, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandlePauseActionWithDuration(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                                   const Commands::PauseActionWithDuration::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandlePauseActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandleResumeAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                        const Commands::ResumeAction::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandleResumeAction(commandData.actionID, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandleEnableAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                        const Commands::EnableAction::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandleEnableAction(commandData.actionID, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandleEnableActionWithDuration(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                                    const Commands::EnableActionWithDuration::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandleEnableActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandleDisableAction(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                         const Commands::DisableAction::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandleDisableAction(commandData.actionID, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::HandleDisableActionWithDuration(const DataModel::InvokeRequest & request, CommandHandler * commandHandler,
                                                     const Commands::DisableActionWithDuration::DecodableType & commandData)
{
    uint16_t actionIndex = kMaxActionListLength;
    if (!HaveActionWithId(commandData.actionID, actionIndex))
    {
        commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::NotFound);
        return;
    }
    if (actionIndex != kMaxActionListLength)
    {
        ActionStructStorage action;
        TEMPORARY_RETURN_IGNORED mDelegate.ReadActionAtIndex(actionIndex, action);
        // Check if the command bit is set in the SupportedCommands of an action.
        if (!(action.supportedCommands.Raw() & (1 << request.path.mCommandId)))
        {
            commandHandler->AddStatus(request.path, Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }
    }

    Status status = mDelegate.HandleDisableActionWithDuration(commandData.actionID, commandData.duration, commandData.invokeID);
    commandHandler->AddStatus(request.path, status);
}

void ActionsCluster::ActionListModified()
{
    OnClusterAttributeChanged(Attributes::ActionList::Id);
}

void ActionsCluster::EndpointListsModified()
{
    OnClusterAttributeChanged(Attributes::EndpointLists::Id);
}

void ActionsCluster::OnClusterAttributeChanged(AttributeId attributeId)
{
    // Use DefaultServerCluster's NotifyAttributeChanged which:
    // 1. Increases the data version
    // 2. Notifies subscribers of the change
    NotifyAttributeChanged(attributeId);
}
