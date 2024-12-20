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

#include "actions-server.h"
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/EventLogging.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions;
using namespace chip::app::Clusters::Actions::Attributes;
using namespace chip::Protocols::InteractionModel;

static constexpr size_t kActionsDelegateTableSize =
    MATTER_DM_ACTIONS_CLUSTER_SERVER_ENDPOINT_COUNT + CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

namespace {
Delegate * gDelegateTable[kActionsDelegateTableSize] = { nullptr };

Delegate * GetDelegate(EndpointId endpoint)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, Actions::Id, MATTER_DM_ACTIONS_CLUSTER_SERVER_ENDPOINT_COUNT);
    return (ep >= kActionsDelegateTableSize ? nullptr : gDelegateTable[ep]);
}

} // namespace

ActionsServer ActionsServer::sInstance;

void ActionsServer::SetDefaultDelegate(EndpointId endpoint, Delegate * delegate)
{
    uint16_t ep = emberAfGetClusterServerEndpointIndex(endpoint, Actions::Id, MATTER_DM_ACTIONS_CLUSTER_SERVER_ENDPOINT_COUNT);
    // if endpoint is found
    if (ep < kActionsDelegateTableSize)
    {
        gDelegateTable[ep] = delegate;
    }
}

ActionsServer & ActionsServer::Instance()
{
    return sInstance;
}

void ActionsServer::OnStateChanged(EndpointId endpoint, uint16_t actionId, uint32_t invokeId, ActionStateEnum actionState)
{
    ChipLogProgress(Zcl, "ActionsServer: OnStateChanged");

    // Generate StateChanged event
    EventNumber eventNumber;
    Events::StateChanged::Type event{ actionId, invokeId, actionState };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "ActionsServer: Failed to generate OnStateChanged event");
    }
}

void ActionsServer::OnActionFailed(EndpointId endpoint, uint16_t actionId, uint32_t invokeId, ActionStateEnum actionState,
                                   ActionErrorEnum actionError)
{
    ChipLogProgress(Zcl, "ActionsServer: OnActionFailed");

    // Generate ActionFailed event
    EventNumber eventNumber;
    Events::ActionFailed::Type event{ actionId, invokeId, actionState, actionError };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "ActionsServer: Failed to generate OnActionFailed event");
    }
}

CHIP_ERROR ActionsServer::ReadActionListAttribute(const ConcreteReadAttributePath & aPath,
                                                  const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    Delegate * delegate = GetDelegate(aPath.mEndpointId);
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Actions delegate is null!!!");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    for (uint16_t i = 0; i <= kMaxActionListLength; i++)
    {
        ActionStructStorage action;

        CHIP_ERROR err = delegate->ReadActionAtIndex(i, action);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(encoder.Encode(action));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ActionsServer::ReadEndpointListAttribute(const ConcreteReadAttributePath & aPath,
                                                    const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    Delegate * delegate = GetDelegate(aPath.mEndpointId);
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Actions delegate is null!!!");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    for (uint16_t i = 0; i <= kMaxEndpointListLength; i++)
    {
        EndpointListStorage epList;

        CHIP_ERROR err = delegate->ReadEndpointListAtIndex(i, epList);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(encoder.Encode(epList));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR ActionsServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Actions::Id);

    switch (aPath.mAttributeId)
    {
    case ActionList::Id: {
        return aEncoder.EncodeList(
            [this, aPath](const auto & encoder) -> CHIP_ERROR { return this->ReadActionListAttribute(aPath, encoder); });
    }
    case EndpointLists::Id: {
        return aEncoder.EncodeList(
            [this, aPath](const auto & encoder) -> CHIP_ERROR { return this->ReadEndpointListAttribute(aPath, encoder); });
    }
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

bool ActionsServer::FindActionIdInActionList(EndpointId endpointId, uint16_t actionId)
{
    Delegate * delegate = GetDelegate(endpointId);
    if (delegate == nullptr)
    {
        ChipLogError(Zcl, "Actions delegate is null!!!");
        return false;
    }
    return delegate->FindActionIdInActionList(actionId);
}

template <typename RequestT, typename FuncT>
void ActionsServer::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    if (!handlerContext.mCommandHandled && (handlerContext.mRequestPath.mCommandId == RequestT::GetCommandId()))
    {
        RequestT requestPayload;

        // If the command matches what the caller is looking for, let's mark this as being handled
        // even if errors happen after this. This ensures that we don't execute any fall-back strategies
        // to handle this command since at this point, the caller is taking responsibility for handling
        // the command in its entirety, warts and all.
        //
        handlerContext.SetCommandHandled();

        if (DataModel::Decode(handlerContext.mPayload, requestPayload) != CHIP_NO_ERROR)
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath,
                                                     Protocols::InteractionModel::Status::InvalidCommand);
            return;
        }

        if (FindActionIdInActionList(handlerContext.mRequestPath.mEndpointId, requestPayload.actionID))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Protocols::InteractionModel::Status::NotFound);
            return;
        }

        func(handlerContext, requestPayload);
    }
}

void ActionsServer::HandleInstantAction(HandlerContext & ctx, const Commands::InstantAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandleInstantAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandleInstantActionWithTransition(HandlerContext & ctx,
                                                      const Commands::InstantActionWithTransition::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    uint16_t transitionTime     = commandData.transitionTime;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandleInstantActionWithTransition(actionId, transitionTime, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandleStartAction(HandlerContext & ctx, const Commands::StartAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandleStartAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandleStartActionWithDuration(HandlerContext & ctx,
                                                  const Commands::StartActionWithDuration::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    uint32_t duration           = commandData.duration;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandleStartActionWithDuration(actionId, duration, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandleStopAction(HandlerContext & ctx, const Commands::StopAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandleStopAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandlePauseAction(HandlerContext & ctx, const Commands::PauseAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandlePauseAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandlePauseActionWithDuration(HandlerContext & ctx,
                                                  const Commands::PauseActionWithDuration::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    uint32_t duration           = commandData.duration;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandlePauseActionWithDuration(actionId, duration, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandleResumeAction(HandlerContext & ctx, const Commands::ResumeAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandleResumeAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandleEnableAction(HandlerContext & ctx, const Commands::EnableAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandleEnableAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandleEnableActionWithDuration(HandlerContext & ctx,
                                                   const Commands::EnableActionWithDuration::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    uint32_t duration           = commandData.duration;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandleEnableActionWithDuration(actionId, duration, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandleDisableAction(HandlerContext & ctx, const Commands::DisableAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandleDisableAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::HandleDisableActionWithDuration(HandlerContext & ctx,
                                                    const Commands::DisableActionWithDuration::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    uint32_t duration           = commandData.duration;
    Delegate * delegate         = GetDelegate(ctx.mRequestPath.mEndpointId);
    Status status               = delegate->HandleDisableActionWithDuration(actionId, duration, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void ActionsServer::InvokeCommand(HandlerContext & handlerContext)
{
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Actions::Commands::InstantAction::Id:
        HandleCommand<Commands::InstantAction::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleInstantAction(ctx, commandData); });
        return;
    case Actions::Commands::InstantActionWithTransition::Id:
        HandleCommand<Commands::InstantActionWithTransition::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleInstantActionWithTransition(ctx, commandData); });
        return;
    case Actions::Commands::StartAction::Id:
        HandleCommand<Commands::StartAction::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleStartAction(ctx, commandData); });
        return;
    case Actions::Commands::StartActionWithDuration::Id:
        HandleCommand<Commands::StartActionWithDuration::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleStartActionWithDuration(ctx, commandData); });
        return;
    case Actions::Commands::StopAction::Id:
        HandleCommand<Commands::StopAction::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleStopAction(ctx, commandData); });
        return;
    case Actions::Commands::PauseAction::Id:
        HandleCommand<Commands::PauseAction::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandlePauseAction(ctx, commandData); });
        return;
    case Actions::Commands::PauseActionWithDuration::Id:
        HandleCommand<Commands::PauseActionWithDuration::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandlePauseActionWithDuration(ctx, commandData); });
        return;
    case Actions::Commands::ResumeAction::Id:
        HandleCommand<Commands::ResumeAction::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleResumeAction(ctx, commandData); });
        return;
    case Actions::Commands::EnableAction::Id:
        HandleCommand<Commands::EnableAction::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleEnableAction(ctx, commandData); });
        return;
    case Actions::Commands::EnableActionWithDuration::Id:
        HandleCommand<Commands::EnableActionWithDuration::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleEnableActionWithDuration(ctx, commandData); });
        return;
    case Actions::Commands::DisableAction::Id:
        HandleCommand<Commands::DisableAction::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & commandData) { HandleDisableAction(ctx, commandData); });
        return;
    case Actions::Commands::DisableActionWithDuration::Id:
        HandleCommand<Commands::DisableActionWithDuration::DecodableType>(
            handlerContext,
            [this](HandlerContext & ctx, const auto & commandData) { HandleDisableActionWithDuration(ctx, commandData); });
        return;
    }
}
void MatterActionsPluginServerInitCallback()
{
    AttributeAccessInterfaceRegistry::Instance().Register(&ActionsServer::Instance());
    CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(&ActionsServer::Instance());
}
