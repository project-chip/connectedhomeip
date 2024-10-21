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

Instance Instance::instance;
Instance * Instance::GetInstance()
{
    return &instance;
}

void Instance::OnStateChanged(EndpointId endpoint, uint16_t actionId, uint32_t invokeId, ActionStateEnum actionState)
{
    ChipLogProgress(Zcl, "ActionsServer: OnStateChanged");

    // Record StateChanged event
    EventNumber eventNumber;
    Events::StateChanged::Type event{ actionId, invokeId, actionState };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "ActionsServer: Failed to record OnStateChanged event");
    }
}

void Instance::OnActionFailed(EndpointId endpoint, uint16_t actionId, uint32_t invokeId, ActionStateEnum actionState,
                              ActionErrorEnum actionError)
{
    ChipLogProgress(Zcl, "ActionsServer: OnActionFailed");

    // Record ActionFailed event
    EventNumber eventNumber;
    Events::ActionFailed::Type event{ actionId, invokeId, actionState, actionError };

    if (CHIP_NO_ERROR != LogEvent(event, endpoint, eventNumber))
    {
        ChipLogError(Zcl, "ActionsServer: Failed to record OnActionFailed event");
    }
}

CHIP_ERROR Instance::ReadActionListAttribute(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    if (GetInstance()->mDelegate == nullptr)
    {
        ChipLogError(Zcl, "Actions delegate is null!!!");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    for (uint16_t i = 0; true; i++)
    {
        ActionStructStorage action;

        CHIP_ERROR err = GetInstance()->mDelegate->ReadActionAtIndex(i, action);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(encoder.Encode(action));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR Instance::ReadEndpointListAttribute(const AttributeValueEncoder::ListEncodeHelper & encoder)
{
    if (GetInstance()->mDelegate == nullptr)
    {
        ChipLogError(Zcl, "Actions delegate is null!!!");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    for (uint16_t i = 0; true; i++)
    {
        EndpointListStorage epList;

        CHIP_ERROR err = GetInstance()->mDelegate->ReadEndpointListAtIndex(i, epList);
        if (err == CHIP_ERROR_PROVIDER_LIST_EXHAUSTED)
        {
            return CHIP_NO_ERROR;
        }

        ReturnErrorOnFailure(encoder.Encode(epList));
    }
    return CHIP_NO_ERROR;
}

void Instance::SetDefaultDelegate(Delegate * aDelegate)
{
    if (aDelegate == nullptr)
    {
        ChipLogError(Zcl, "Cannot set empty delegate!!!");
        return;
    }
    mDelegate = aDelegate;
}

CHIP_ERROR Instance::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Actions::Id);

    switch (aPath.mAttributeId)
    {
    case ActionList::Id: {
        Instance * d = this;
        CHIP_ERROR err =
            aEncoder.EncodeList([d](const auto & encoder) -> CHIP_ERROR { return d->ReadActionListAttribute(encoder); });
        return err;
    }
    case EndpointLists::Id: {
        Instance * d = this;
        CHIP_ERROR err =
            aEncoder.EncodeList([d](const auto & encoder) -> CHIP_ERROR { return d->ReadEndpointListAttribute(encoder); });
        return err;
    }
    default:
        break;
    }
    return CHIP_NO_ERROR;
}

bool Instance::FindActionIdInActionList(uint16_t actionId)
{
    if (GetInstance()->mDelegate == nullptr)
    {
        ChipLogError(Zcl, "Actions delegate is null!!!");
        return false;
    }
    return GetInstance()->mDelegate->FindActionIdInActionList(actionId);
}

template <typename RequestT, typename FuncT>
void Instance::HandleCommand(HandlerContext & handlerContext, FuncT func)
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

        uint16_t actionId = requestPayload.actionID;
        if (!GetInstance()->FindActionIdInActionList(actionId))
        {
            handlerContext.mCommandHandler.AddStatus(handlerContext.mRequestPath, Protocols::InteractionModel::Status::NotFound);
            return;
        }

        func(handlerContext, requestPayload);
    }
}

void Instance::HandleInstantAction(HandlerContext & ctx, const Commands::InstantAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Status status               = GetInstance()->mDelegate->HandleInstantAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleInstantActionWithTransition(HandlerContext & ctx,
                                                 const Commands::InstantActionWithTransition::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    uint16_t transitionTime     = commandData.transitionTime;
    Status status               = GetInstance()->mDelegate->HandleInstantActionWithTransition(actionId, transitionTime, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleStartAction(HandlerContext & ctx, const Commands::StartAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Status status               = GetInstance()->mDelegate->HandleStartAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleStartActionWithDuration(HandlerContext & ctx,
                                             const Commands::StartActionWithDuration::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    uint32_t duration           = commandData.duration;
    Status status               = GetInstance()->mDelegate->HandleStartActionWithDuration(actionId, duration, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleStopAction(HandlerContext & ctx, const Commands::StopAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Status status               = GetInstance()->mDelegate->HandleStopAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandlePauseAction(HandlerContext & ctx, const Commands::PauseAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Status status               = GetInstance()->mDelegate->HandlePauseAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandlePauseActionWithDuration(HandlerContext & ctx,
                                             const Commands::PauseActionWithDuration::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    uint32_t duration           = commandData.duration;
    Status status               = GetInstance()->mDelegate->HandlePauseActionWithDuration(actionId, duration, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleResumeAction(HandlerContext & ctx, const Commands::ResumeAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Status status               = GetInstance()->mDelegate->HandleResumeAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleEnableAction(HandlerContext & ctx, const Commands::EnableAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Status status               = GetInstance()->mDelegate->HandleEnableAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleEnableActionWithDuration(HandlerContext & ctx,
                                              const Commands::EnableActionWithDuration::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    uint32_t duration           = commandData.duration;
    Status status               = GetInstance()->mDelegate->HandleEnableActionWithDuration(actionId, duration, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleDisableAction(HandlerContext & ctx, const Commands::DisableAction::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    Status status               = GetInstance()->mDelegate->HandleDisableAction(actionId, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::HandleDisableActionWithDuration(HandlerContext & ctx,
                                               const Commands::DisableActionWithDuration::DecodableType & commandData)
{
    uint16_t actionId           = commandData.actionID;
    Optional<uint32_t> invokeId = commandData.invokeID;
    uint32_t duration           = commandData.duration;
    Status status               = GetInstance()->mDelegate->HandleDisableActionWithDuration(actionId, duration, invokeId);
    ctx.mCommandHandler.AddStatus(ctx.mRequestPath, status);
}

void Instance::InvokeCommand(HandlerContext & handlerContext)
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
    AttributeAccessInterfaceRegistry::Instance().Register(Instance::GetInstance());
    CommandHandlerInterfaceRegistry::Instance().RegisterCommandHandler(Instance::GetInstance());
}
