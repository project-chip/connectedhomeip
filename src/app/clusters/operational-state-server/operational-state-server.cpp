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
#include <app/util/af.h>

#include <app/util/attribute-storage.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/callback.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/enums.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/CommandHandler.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelEngine.h>
#include <app/clusters/operational-state-server/operational-state-delegate.h>
#include <app/clusters/operational-state-server/operational-state-server.h>
#include <app/server/Server.h>
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

    mOperationalStateDataProvider.Init(Server::GetInstance().GetPersistentStorage());
    ReturnErrorOnFailure(InteractionModelEngine::GetInstance()->RegisterCommandHandler(this));

    VerifyOrReturnError(registerAttributeAccessOverride(this), CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mDelegate, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mDelegate->Init());

    return CHIP_NO_ERROR;
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

CHIP_ERROR OperationalStateServer::EnumerateAcceptedCommands(const ConcreteClusterPath & cluster, CommandIdCallback callback,
                                                             void * context)
{
    callback(Commands::Pause::Id, context);
    callback(Commands::Resume::Id, context);
    callback(Commands::Start::Id, context);
    callback(Commands::Stop::Id, context);
    return CHIP_NO_ERROR;
}

void OperationalStateServer::HandlePauseState(HandlerContext & ctx, const Commands::Pause::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandlePauseState");
    Commands::OperationalCommandResponse::Type response;
    GenericOperationalErrorState currentErrState;
    GenericOperationalState currentOperationalState;

    // copy current operational state
    currentOperationalState = mOperationalState;
    // copy current operational error
    currentErrState = mOperationalError;

    // callback
    mDelegate->HandlePauseState(currentOperationalState, currentErrState);

    // set operational error
    SetOperationalState(currentOperationalState);

    // copy current operational error
    mOperationalError = currentErrState;

    // return operational error
    response.commandResponseState.errorStateID = static_cast<OperationalState::ErrorStateEnum>(mOperationalError.ErrorStateID);

    if (mOperationalError.ErrorStateLabelHasValue)
    {
        response.commandResponseState.errorStateLabel.SetNonNull(mOperationalError.ErrorStateLabel, sizeof(mOperationalError.ErrorStateLabel));
    }
    else
    {
        response.commandResponseState.errorStateLabel.SetNull();
    }

    response.commandResponseState.errorStateDetails = mOperationalError.ErrorStateDetailsHasValue ? Optional<chip::CharSpan>::Value(mOperationalError.ErrorStateDetails) : Optional<chip::CharSpan>::Missing();

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void OperationalStateServer::HandleResumeState(HandlerContext & ctx, const Commands::Resume::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandleResumeState");
    Commands::OperationalCommandResponse::Type response;
    GenericOperationalErrorState currentErrState;
    GenericOperationalState currentOperationalState;

    // copy current operational state
    currentOperationalState = mOperationalState;
    // copy current operational error
    currentErrState = mOperationalError;

    // callback
    mDelegate->HandleResumeState(currentOperationalState, currentErrState);

    // set operational error
    SetOperationalState(currentOperationalState);

    // copy current operational error
    mOperationalError = currentErrState;

    // return operational error
    response.commandResponseState.errorStateID = static_cast<OperationalState::ErrorStateEnum>(mOperationalError.ErrorStateID);

    if (mOperationalError.ErrorStateLabelHasValue)
    {
        response.commandResponseState.errorStateLabel.SetNonNull(mOperationalError.ErrorStateLabel, sizeof(mOperationalError.ErrorStateLabel));
    }
    else
    {
        response.commandResponseState.errorStateLabel.SetNull();
    }
    response.commandResponseState.errorStateDetails = mOperationalError.ErrorStateDetailsHasValue ? Optional<chip::CharSpan>::Value(mOperationalError.ErrorStateDetails) : Optional<chip::CharSpan>::Missing();

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void OperationalStateServer::HandleStartState(HandlerContext & ctx, const Commands::Start::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandleStartState");
    Commands::OperationalCommandResponse::Type response;
    GenericOperationalErrorState currentErrState;
    GenericOperationalState currentOperationalState;

    // copy current operational state
    currentOperationalState = mOperationalState;
    // copy current operational error
    currentErrState = mOperationalError;

    // callback
    mDelegate->HandleStartState(currentOperationalState, currentErrState);

    // set operational error
    SetOperationalState(currentOperationalState);

    // copy current operational error
    mOperationalError = currentErrState;

    // return operational error
    response.commandResponseState.errorStateID = static_cast<OperationalState::ErrorStateEnum>(mOperationalError.ErrorStateID);

    if (mOperationalError.ErrorStateLabelHasValue)
    {
        response.commandResponseState.errorStateLabel.SetNonNull(mOperationalError.ErrorStateLabel, sizeof(mOperationalError.ErrorStateLabel));
    }
    else
    {
        response.commandResponseState.errorStateLabel.SetNull();
    }
    response.commandResponseState.errorStateDetails = mOperationalError.ErrorStateDetailsHasValue ? Optional<chip::CharSpan>::Value(mOperationalError.ErrorStateDetails) : Optional<chip::CharSpan>::Missing();

    ctx.mCommandHandler.AddResponse(ctx.mRequestPath, response);
}

void OperationalStateServer::HandleStopState(HandlerContext & ctx, const Commands::Stop::DecodableType & req)
{
    ChipLogDetail(Zcl, "OperationalState: HandleStopState");
    Commands::OperationalCommandResponse::Type response;
    GenericOperationalErrorState currentErrState;
    GenericOperationalState currentOperationalState;

    // copy current operational state
    currentOperationalState = mOperationalState;
    // copy current operational error
    currentErrState = mOperationalError;

    // callback
    mDelegate->HandleStopState(currentOperationalState, currentErrState);

    // set operational error
    SetOperationalState(currentOperationalState);

    // copy current operational error
    mOperationalError = currentErrState;

    // return operational error
    response.commandResponseState.errorStateID = static_cast<OperationalState::ErrorStateEnum>(mOperationalError.ErrorStateID);

    if (mOperationalError.ErrorStateLabelHasValue)
    {
        response.commandResponseState.errorStateLabel.SetNonNull(mOperationalError.ErrorStateLabel, sizeof(mOperationalError.ErrorStateLabel));
    }
    else
    {
        response.commandResponseState.errorStateLabel.SetNull();
    }
    response.commandResponseState.errorStateDetails = mOperationalError.ErrorStateDetailsHasValue ? Optional<chip::CharSpan>::Value(mOperationalError.ErrorStateDetails) : Optional<chip::CharSpan>::Missing();

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

CHIP_ERROR OperationalStateServer::GetOperationalStateList(OperationalStateStructDynamicList ** operationalStateList, size_t & size)
{
    return mOperationalStateDataProvider.LoadOperationalStateList(mEndpointId, mClusterId, operationalStateList, size);
}

void OperationalStateServer::ReleaseOperationalStateList(OperationalStateStructDynamicList * operationalStateList)
{
    mOperationalStateDataProvider.ReleaseOperationalStateList(operationalStateList);
}

CHIP_ERROR OperationalStateServer::ClearOperationalStateList()
{
    return mOperationalStateDataProvider.ClearOperationalStateList(mEndpointId, mClusterId);
}

CHIP_ERROR OperationalStateServer::SetPhaseList(const PhaseList & phaseList)
{
    return mOperationalStateDataProvider.StorePhaseList(mEndpointId, mClusterId, phaseList);
}

CHIP_ERROR OperationalStateServer::GetPhaseList(PhaseListCharSpan ** phaseList, size_t & size)
{
    return mOperationalStateDataProvider.LoadPhaseList(mEndpointId, mClusterId, phaseList, size);
}

void OperationalStateServer::ReleasePhaseList(PhaseListCharSpan * phaseList)
{
    mOperationalStateDataProvider.ReleasePhaseList(phaseList);
}

CHIP_ERROR OperationalStateServer::ClearPhaseStateList()
{
    return mOperationalStateDataProvider.ClearPhaseStateList(mEndpointId, mClusterId);
}

CHIP_ERROR OperationalStateServer::SetOperationalState(GenericOperationalState & opState)
{
    OperationalStateStructDynamicList op;

    op.operationalStateID = static_cast<Clusters::OperationalState::OperationalStateEnum>(opState.OperationalStateID);

    ReturnErrorOnFailure(mOperationalStateDataProvider.UseOpStateIDGetOpStateStruct(mEndpointId, mClusterId, op));

    if (&mOperationalState != &opState)
    {
        char * dest = mOperationalState.OperationalStateLabel;
        size_t len  = op.operationalStateLabel.size();
        memset(dest, 0, sizeof(mOperationalState.OperationalStateLabel));
        memcpy(dest, op.operationalStateLabel.data(), len);
        mOperationalState.OperationalStateID = static_cast<uint8_t>(op.operationalStateID);
    }
    return CHIP_NO_ERROR;
}

const GenericOperationalState & OperationalStateServer::GetOperationalState()
{
    return mOperationalState;
}

CHIP_ERROR OperationalStateServer::SetOperationalError(GenericOperationalErrorState & op)
{
    mOperationalError.ErrorStateID = op.ErrorStateID;

    memset(mOperationalError.ErrorStateLabel, 0, sizeof(mOperationalError.ErrorStateLabel));
    memset(mOperationalError.ErrorStateDetails, 0, sizeof(mOperationalError.ErrorStateDetails));

    if (op.ErrorStateLabelHasValue)
    {
        memcpy(mOperationalError.ErrorStateLabel, op.ErrorStateLabel, sizeof(op.ErrorStateLabel));
        mOperationalError.ErrorStateLabelHasValue = true;
    }
    else
    {
        mOperationalError.ErrorStateLabelHasValue = false;
    }
    if (op.ErrorStateDetailsHasValue)
    {
       memcpy(mOperationalError.ErrorStateDetails, op.ErrorStateDetails, sizeof(op.ErrorStateDetails));
       mOperationalError.ErrorStateDetailsHasValue = true;
    }
    else
    {
        mOperationalError.ErrorStateDetailsHasValue = false;
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR OperationalStateServer::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (aPath.mAttributeId)
    {
    case OperationalState::Attributes::OperationalStateList::Id: {
        OperationalStateStructDynamicList * pOpList = nullptr;
        size_t size                                 = 0;
        err = GetOperationalStateList(&pOpList, size);
        if (err != CHIP_NO_ERROR)
        {
            err = aEncoder.EncodeNull();
        }
        else
        {
            return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
                for (OperationalStateStructDynamicList * pHead = pOpList; pHead != nullptr; pHead = pHead->Next)
                {
                    ReturnErrorOnFailure(encoder.Encode(*pHead));
                }
                ReleaseOperationalStateList(pOpList);
                pOpList = nullptr;
                return CHIP_NO_ERROR;
            });
        }
    }
    break;

    case OperationalState::Attributes::OperationalState::Id: {
        OperationalState::Structs::OperationalStateStruct::Type opState;

        opState.operationalStateID    = static_cast<OperationalState::OperationalStateEnum>(mOperationalState.OperationalStateID);
        opState.operationalStateLabel = CharSpan::fromCharString(mOperationalState.OperationalStateLabel);

        return aEncoder.Encode(opState);
    }
    break;

    case OperationalState::Attributes::OperationalError::Id: {

        OperationalState::Structs::ErrorStateStruct::Type opError;

        opError.errorStateID = static_cast<OperationalState::ErrorStateEnum>(mOperationalError.ErrorStateID);

        if (mOperationalError.ErrorStateLabelHasValue)
        {
            opError.errorStateLabel.SetNonNull(mOperationalError.ErrorStateLabel, sizeof(mOperationalError.ErrorStateLabel));
        }
        else
        {
            opError.errorStateLabel.SetNull();
        }
        opError.errorStateDetails = mOperationalError.ErrorStateDetailsHasValue ? Optional<chip::CharSpan>::Value(mOperationalError.ErrorStateDetails) : Optional<chip::CharSpan>::Missing();
        return aEncoder.Encode(opError);
    }
    break;

    case OperationalState::Attributes::PhaseList::Id: {
        PhaseListCharSpan * phaseList = nullptr;
        size_t size                   = 0;
        err = GetPhaseList(&phaseList, size);
        if (err != CHIP_NO_ERROR)
        {
            err = aEncoder.EncodeNull();
        }
        else
        {
            return aEncoder.EncodeList([&](const auto & encoder) -> CHIP_ERROR {
                for (PhaseListCharSpan * pHead = phaseList; pHead != nullptr; pHead = pHead->Next)
                {
                    ReturnErrorOnFailure(encoder.Encode(pHead->phase));
                }
                ReleasePhaseList(phaseList);
                phaseList = nullptr;
                return CHIP_NO_ERROR;
            });
        }
    }
    break;
    }
    return CHIP_NO_ERROR;
}
