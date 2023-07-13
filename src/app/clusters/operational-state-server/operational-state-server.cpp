/*
 *
 *    Copyright (c) 2023 Project CHIP Authors
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

/****************************************************************************
 * @file
 * @brief Implementation for the Operational State Server Cluster
 ***************************************************************************/
#include "operational-state-server.h"
#include "operational-state-delegate.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/error-mapping.h>
#include <lib/core/CHIPEncoding.h>

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::OperationalState;
using namespace chip::app::Clusters::OperationalState::Attributes;

using Status = Protocols::InteractionModel::Status;

CHIP_ERROR OperationalStateServer::Init()
{
    // Check if the cluster has been selected in zap
    if (!emberAfContainsServer(mEndpointId, mClusterId))
    {
        ChipLogError(Zcl, "Operational State: The cluster with ID %lu was not enabled in zap.", long(mClusterId));
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));

    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    return CHIP_NO_ERROR;
}

void OperationalStateServer::Shutdown()
{
    InteractionModelEngine::GetInstance()->UnregisterCommandHandler(this);
}

// This function is called by the interaction model engine when a command destined for this instance is received.
void OperationalStateServer::InvokeCommand(HandlerContext & handlerContext)
{
    ChipLogDetail(Zcl, "OperationalState: InvokeCommand");
    switch (handlerContext.mRequestPath.mCommandId)
    {
    case Commands::Pause::Id:
        ChipLogDetail(Zcl, "OperationalState: Entering handling Pause state");

        HandleCommand<Commands::Pause::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandlePauseState(ctx, req); });
        break;

    case Commands::Resume::Id:
        ChipLogDetail(Zcl, "OperationalState: Entering handling Resume state");

        HandleCommand<Commands::Resume::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleResumeState(ctx, req); });
        break;

    case Commands::Start::Id:
        ChipLogDetail(Zcl, "OperationalState: Entering handling Start state");

        HandleCommand<Commands::Start::DecodableType>(
            handlerContext, [this](HandlerContext & ctx, const auto & req) { HandleStartState(ctx, req); });
        break;

    case Commands::Stop::Id:
        ChipLogDetail(Zcl, "OperationalState: Entering handling Stop state");

        HandleCommand<Commands::Stop::DecodableType>(handlerContext,
                                                     [this](HandlerContext & ctx, const auto & req) { HandleStopState(ctx, req); });
        break;
    }
}

void OperationalStateServer::HandlePauseState(HandlerContext & ctx, const Commands::Pause::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandlePauseState");
    Commands::OperationalCommandResponse::Type response;
    Delegate * delegate = OperationalState::GetOperationalStateDelegate(mEndpointId, mClusterId);
    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    GenericOperationalState opState;

    VerifyOrReturn(delegate != nullptr, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure));
    delegate->GetCurrentOperationalState(opState);

    if (opState.operationalStateID != to_underlying(OperationalStateEnum::kPaused))
    {
        delegate->HandlePauseStateCallback(err);
    }
    response.commandResponseState = err;

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void OperationalStateServer::HandleResumeState(HandlerContext & ctx, const Commands::Resume::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandleResumeState");
    Commands::OperationalCommandResponse::Type response;
    Delegate * delegate = OperationalState::GetOperationalStateDelegate(mEndpointId, mClusterId);
    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    GenericOperationalState opState;

    VerifyOrReturn(delegate != nullptr, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure));

    delegate->GetCurrentOperationalState(opState);

    if (opState.operationalStateID != to_underlying(OperationalStateEnum::kPaused) &&
        opState.operationalStateID != to_underlying(OperationalStateEnum::kRunning))
    {
        err.Set(to_underlying(ErrorStateEnum::kCommandInvalidInState));
    }
    else if (opState.operationalStateID == to_underlying(OperationalStateEnum::kPaused))
    {
        delegate->HandleResumeStateCallback(err);
    }
    response.commandResponseState = err;

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void OperationalStateServer::HandleStartState(HandlerContext & ctx, const Commands::Start::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandleStartState");
    Commands::OperationalCommandResponse::Type response;
    Delegate * delegate = OperationalState::GetOperationalStateDelegate(mEndpointId, mClusterId);
    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    GenericOperationalState opState;

    VerifyOrReturn(delegate != nullptr, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure));

    delegate->GetCurrentOperationalState(opState);

    if (opState.operationalStateID != to_underlying(OperationalStateEnum::kRunning))
    {
        delegate->HandleStartStateCallback(err);
    }
    response.commandResponseState = err;

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void OperationalStateServer::HandleStopState(HandlerContext & ctx, const Commands::Stop::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandleStopState");
    Commands::OperationalCommandResponse::Type response;
    Delegate * delegate = OperationalState::GetOperationalStateDelegate(mEndpointId, mClusterId);
    GenericOperationalError err(to_underlying(ErrorStateEnum::kNoError));
    GenericOperationalState opState;

    VerifyOrReturn(delegate != nullptr, ctx.mCommandHandler.AddStatus(ctx.mRequestPath, Status::Failure));

    delegate->GetCurrentOperationalState(opState);

    if (opState.operationalStateID != to_underlying(OperationalStateEnum::kStopped))
    {
        delegate->HandleStopStateCallback(err);
    }
    response.commandResponseState = err;

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

template <typename RequestT, typename FuncT>
void OperationalStateServer::HandleCommand(HandlerContext & handlerContext, FuncT func)
{
    if (!handlerContext.mCommandHandled && (handlerContext.mRequestPath.mCommandId == RequestT::GetCommandId()))
    {
        RequestT requestPayload;

        //
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

        func(handlerContext, requestPayload);
    }
}

CHIP_ERROR OperationalStateServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    switch (aPath.mAttributeId)
    {
    case OperationalState::Attributes::OperationalStateList::Id: {
        Delegate * delegate = OperationalState::GetOperationalStateDelegate(mEndpointId, mClusterId);
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is nullptr"));

        return aEncoder.EncodeList([delegate](const auto & encoder) -> CHIP_ERROR {
            GenericOperationalState opState;
            size_t index   = 0;
            CHIP_ERROR err = CHIP_NO_ERROR;
            while ((err = delegate->GetOperationalStateAtIndex(index, opState)) == CHIP_NO_ERROR)
            {
                ReturnErrorOnFailure(encoder.Encode(opState));
                index++;
            }
            if (err == CHIP_ERROR_NOT_FOUND)
            {
                return CHIP_NO_ERROR;
            }
            return err;
        });
    }
    break;

    case OperationalState::Attributes::OperationalState::Id: {

        Delegate * delegate = OperationalState::GetOperationalStateDelegate(mEndpointId, mClusterId);
        GenericOperationalState opState;
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is nullptr"));
        delegate->GetCurrentOperationalState(opState);
        return aEncoder.Encode(opState);
    }
    break;

    case OperationalState::Attributes::OperationalError::Id: {
        Delegate * delegate = OperationalState::GetOperationalStateDelegate(mEndpointId, mClusterId);
        GenericOperationalError opErr(to_underlying(ErrorStateEnum::kNoError));
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is nullptr"));
        delegate->GetCurrentOperationalError(opErr);
        return aEncoder.Encode(opErr);
    }
    break;

    case OperationalState::Attributes::PhaseList::Id: {
        Delegate * delegate = OperationalState::GetOperationalStateDelegate(mEndpointId, mClusterId);

        GenericOperationalPhase phase = GenericOperationalPhase(DataModel::Nullable<CharSpan>());
        size_t index                  = 0;
        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is nullptr"));

        if (delegate->GetOperationalPhaseAtIndex(index, phase) == CHIP_ERROR_NOT_FOUND || phase.IsMissing())
        {
            return aEncoder.EncodeNull();
        }
        return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
            while (delegate->GetOperationalPhaseAtIndex(index, phase) != CHIP_ERROR_NOT_FOUND)
            {
                ReturnErrorOnFailure(encoder.Encode(phase.mPhaseName));
                index++;
            }
            return CHIP_NO_ERROR;
        });
    }
    break;

    case OperationalState::Attributes::CurrentPhase::Id: {
        DataModel::Nullable<uint8_t> currentPhase;
        Delegate * delegate = OperationalState::GetOperationalStateDelegate(mEndpointId, mClusterId);

        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is nullptr"));
        delegate->GetCurrentPhase(currentPhase);
        return aEncoder.Encode(currentPhase);
    }
    break;

    case OperationalState::Attributes::CountdownTime::Id: {
        DataModel::Nullable<uint32_t> countdownTime;
        Delegate * delegate = OperationalState::GetOperationalStateDelegate(mEndpointId, mClusterId);

        VerifyOrReturnError(delegate != nullptr, CHIP_ERROR_INCORRECT_STATE, ChipLogError(Zcl, "Delegate is nullptr"));
        delegate->GetCountdownTime(countdownTime);
        return aEncoder.Encode(countdownTime);
    }
    break;
    }
    return CHIP_NO_ERROR;
}
