/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    All rights reserved.
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

/**
 *    @file
 *      This file defines object for a CHIP IM Invoke Command Handler
 *
 */

#include "CommandHandler.h"
#include "Command.h"
#include "CommandSender.h"
#include "InteractionModelEngine.h"
#include "messaging/ExchangeContext.h"

#include <app/util/MatterCallbacks.h>
#include <lib/support/TypeTraits.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace app {

CommandHandler::CommandHandler(Callback * apCallback) : mpCallback(apCallback), mSuppressResponse(false) {}

CHIP_ERROR CommandHandler::AllocateBuffer()
{
    if (!mBufferAllocated)
    {
        mCommandMessageWriter.Reset();

        System::PacketBufferHandle commandPacket = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
        VerifyOrReturnError(!commandPacket.IsNull(), CHIP_ERROR_NO_MEMORY);

        mCommandMessageWriter.Init(std::move(commandPacket));
        ReturnErrorOnFailure(mInvokeResponseBuilder.Init(&mCommandMessageWriter));

        mInvokeResponseBuilder.SuppressResponse(mSuppressResponse);
        ReturnErrorOnFailure(mInvokeResponseBuilder.GetError());

        mInvokeResponseBuilder.CreateInvokeResponses();
        ReturnErrorOnFailure(mInvokeResponseBuilder.GetError());

        mCommandIndex    = 0;
        mBufferAllocated = true;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::OnInvokeCommandRequest(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                  System::PacketBufferHandle && payload)
{
    System::PacketBufferHandle response;
    VerifyOrReturnError(mState == CommandState::Idle, CHIP_ERROR_INCORRECT_STATE);

    // NOTE: we already know this is an InvokeCommand Request message because we explicitly registered with the
    // Exchange Manager for unsolicited InvokeCommand Requests.
    mpExchangeCtx = ec;

    // Use the RAII feature, if this is the only Handle when this function returns, DecrementHoldOff will trigger sending response.
    Handle workHandle(this);
    mpExchangeCtx->WillSendMessage();
    ReturnErrorOnFailure(ProcessInvokeRequest(std::move(payload)));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::ProcessInvokeRequest(System::PacketBufferHandle && payload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;
    TLV::TLVReader invokeRequestsReader;
    InvokeRequestMessage::Parser invokeRequestMessage;
    InvokeRequests::Parser invokeRequests;
    reader.Init(std::move(payload));
    ReturnErrorOnFailure(reader.Next());
    ReturnErrorOnFailure(invokeRequestMessage.Init(reader));
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    ReturnErrorOnFailure(invokeRequestMessage.CheckSchemaValidity());
#endif
    ReturnErrorOnFailure(invokeRequestMessage.GetSuppressResponse(&mSuppressResponse));
    ReturnErrorOnFailure(invokeRequestMessage.GetTimedRequest(&mTimedRequest));
    ReturnErrorOnFailure(invokeRequestMessage.GetInvokeRequests(&invokeRequests));

    invokeRequests.GetReader(&invokeRequestsReader);
    while (CHIP_NO_ERROR == (err = invokeRequestsReader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag == invokeRequestsReader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        CommandDataIB::Parser commandData;
        ReturnErrorOnFailure(commandData.Init(invokeRequestsReader));
        ReturnErrorOnFailure(ProcessCommandDataIB(commandData));
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    return err;
}

void CommandHandler::Close()
{
    mSuppressResponse = false;
    MoveToState(CommandState::AwaitingDestruction);

    // We must finish all async work before we can shut down a CommandHandler. The actual CommandHandler MUST finish their work
    // in reasonable time or there is a bug. The only case for releasing CommandHandler without CommandHandler::Handle releasing its
    // reference is the stack shutting down, in which case Close() is not called. So the below check should always pass.
    VerifyOrDieWithMsg(mPendingWork == 0, DataManagement, "CommandHandler::Close() called with %zu unfinished async work items",
                       mPendingWork);

    Command::Close();

    if (mpCallback)
    {
        mpCallback->OnDone(*this);
    }
}

void CommandHandler::IncrementHoldOff()
{
    mPendingWork++;
}

void CommandHandler::DecrementHoldOff()
{
    mPendingWork--;
    ChipLogDetail(DataManagement, "Decreasing reference count for CommandHandler, remaining %zu", mPendingWork);
    if (mPendingWork != 0)
    {
        return;
    }
    CHIP_ERROR err = SendCommandResponse();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Failed to send command response: %" CHIP_ERROR_FORMAT, err.Format());
        // We marked the exchange as "WillSendMessage", need to shutdown the exchange manually to avoid leaking exchanges.
        if (mpExchangeCtx != nullptr)
        {
            mpExchangeCtx->Close();
        }
    }
    Close();
}

CHIP_ERROR CommandHandler::SendCommandResponse()
{
    System::PacketBufferHandle commandPacket;

    VerifyOrReturnError(mPendingWork == 0, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mState == CommandState::AddedCommand, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mpExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(Finalize(commandPacket));
    ReturnErrorOnFailure(
        mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandResponse, std::move(commandPacket)));
    // The ExchangeContext is automatically freed here, and it makes mpExchangeCtx be temporarily dangling, but in
    // all cases, we are going to call Close immediately after this function, which nulls out mpExchangeCtx.

    MoveToState(CommandState::CommandSent);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::ProcessCommandDataIB(CommandDataIB::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPath;
    TLV::TLVReader commandDataReader;
    ClusterId clusterId;
    CommandId commandId;
    EndpointId endpointId;

    err = aCommandElement.GetPath(&commandPath);
    SuccessOrExit(err);

    err = commandPath.GetClusterId(&clusterId);
    SuccessOrExit(err);

    err = commandPath.GetCommandId(&commandId);
    SuccessOrExit(err);

    err = commandPath.GetEndpointId(&endpointId);
    SuccessOrExit(err);
    VerifyOrExit(mpCallback->CommandExists(ConcreteCommandPath(endpointId, clusterId, commandId)),
                 err = CHIP_ERROR_INVALID_PROFILE_ID);
    err = aCommandElement.GetData(&commandDataReader);
    if (CHIP_END_OF_TLV == err)
    {
        ChipLogDetail(DataManagement,
                      "Received command without data for Endpoint=%" PRIu16 " Cluster=" ChipLogFormatMEI
                      " Command=" ChipLogFormatMEI,
                      endpointId, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId));
        err = CHIP_NO_ERROR;
    }
    if (CHIP_NO_ERROR == err)
    {
        ChipLogDetail(DataManagement,
                      "Received command for Endpoint=%" PRIu16 " Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                      endpointId, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId));
        const ConcreteCommandPath concretePath(endpointId, clusterId, commandId);
        SuccessOrExit(MatterPreCommandReceivedCallback(concretePath));
        mpCallback->DispatchCommand(*this, ConcreteCommandPath(endpointId, clusterId, commandId), commandDataReader);
        MatterPostCommandReceivedCallback(concretePath);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ConcreteCommandPath path(endpointId, clusterId, commandId);

        // The Path is the path in the request if there are any error occurred before we dispatch the command to clusters.
        // Currently, it could be failed to decode Path or failed to find cluster / command on desired endpoint.
        // TODO: The behavior when receiving a malformed message is not clear in the Spec. (Spec#3259)
        // TODO: The error code should be updated after #7072 added error codes required by IM.
        if (err == CHIP_ERROR_INVALID_PROFILE_ID)
        {
            ChipLogDetail(DataManagement, "No Cluster " ChipLogFormatMEI " on Endpoint 0x%" PRIx16, ChipLogValueMEI(clusterId),
                          endpointId);
        }

        // TODO:in particular different reasons for ServerClusterCommandExists to test false should result in different errors here
        AddStatus(path, Protocols::InteractionModel::Status::InvalidCommand);
    }

    // We have handled the error status above and put the error status in response, now return success status so we can process
    // other commands in the invoke request.
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::AddStatusInternal(const ConcreteCommandPath & aCommandPath,
                                             const Protocols::InteractionModel::Status aStatus,
                                             const Optional<ClusterStatus> & aClusterStatus)
{
    StatusIB::Builder statusIBBuilder;
    StatusIB statusIB;

    CommandPathParams commandPathParams = { aCommandPath.mEndpointId,
                                            0, // GroupId
                                            aCommandPath.mClusterId, aCommandPath.mCommandId,
                                            chip::app::CommandPathFlags::kEndpointIdValid };

    ReturnLogErrorOnFailure(PrepareStatus(commandPathParams));
    statusIBBuilder = mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetStatus().CreateErrorStatus();

    //
    // TODO: Most of the callers are incorrectly passing SecureChannel as the protocol ID, when in fact, the status code provided
    // above is always an IM code. Instead of fixing all the callers (which is a fairly sizeable change), we'll embark on fixing
    // this more completely when we fix #9530.
    //
    statusIB.mStatus        = aStatus;
    statusIB.mClusterStatus = aClusterStatus;
    statusIBBuilder.EncodeStatusIB(statusIB);
    ReturnLogErrorOnFailure(statusIBBuilder.GetError());
    return FinishStatus();
}

CHIP_ERROR CommandHandler::AddStatus(const ConcreteCommandPath & aCommandPath, const Protocols::InteractionModel::Status aStatus)
{
    Optional<ClusterStatus> clusterStatus = Optional<ClusterStatus>::Missing();
    return AddStatusInternal(aCommandPath, aStatus, clusterStatus);
}

CHIP_ERROR CommandHandler::AddClusterSpecificSuccess(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus)
{
    Optional<ClusterStatus> clusterStatus(aClusterStatus);
    return AddStatusInternal(aCommandPath, Protocols::InteractionModel::Status::Success, clusterStatus);
}

CHIP_ERROR CommandHandler::AddClusterSpecificFailure(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus)
{
    Optional<ClusterStatus> clusterStatus(aClusterStatus);
    return AddStatusInternal(aCommandPath, Protocols::InteractionModel::Status::Failure, clusterStatus);
}

CHIP_ERROR CommandHandler::PrepareResponse(const ConcreteCommandPath & aRequestCommandPath, CommandId aResponseCommand)
{
    CommandPathParams params = { aRequestCommandPath.mEndpointId,
                                 0, // GroupId
                                 aRequestCommandPath.mClusterId, aResponseCommand, (CommandPathFlags::kEndpointIdValid) };
    return PrepareCommand(params, false /* aStartDataStruct */);
}

CHIP_ERROR CommandHandler::PrepareCommand(const CommandPathParams & aCommandPathParams, bool aStartDataStruct)
{
    ReturnErrorOnFailure(AllocateBuffer());
    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrReturnError(mState == CommandState::Idle, CHIP_ERROR_INCORRECT_STATE);
    CommandDataIB::Builder commandData = mInvokeResponseBuilder.GetInvokeResponses().CreateInvokeResponse().CreateCommand();
    ReturnErrorOnFailure(commandData.GetError());
    ReturnErrorOnFailure(ConstructCommandPath(aCommandPathParams, commandData.CreatePath()));
    if (aStartDataStruct)
    {
        ReturnErrorOnFailure(commandData.GetWriter()->StartContainer(TLV::ContextTag(to_underlying(CommandDataIB::Tag::kData)),
                                                                     TLV::kTLVType_Structure, mDataElementContainerType));
    }
    MoveToState(CommandState::AddingCommand);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::FinishCommand(bool aStartDataStruct)
{
    VerifyOrReturnError(mState == CommandState::AddingCommand, CHIP_ERROR_INCORRECT_STATE);
    CommandDataIB::Builder commandData = mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetCommand();
    if (aStartDataStruct)
    {
        ReturnErrorOnFailure(commandData.GetWriter()->EndContainer(mDataElementContainerType));
    }
    ReturnErrorOnFailure(commandData.EndOfCommandDataIB().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().EndOfInvokeResponseIB().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().EndOfInvokeResponses().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.EndOfInvokeResponseMessage().GetError());
    MoveToState(CommandState::AddedCommand);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::PrepareStatus(const CommandPathParams & aCommandPathParams)
{
    ReturnErrorOnFailure(AllocateBuffer());
    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrReturnError(mState == CommandState::Idle, CHIP_ERROR_INCORRECT_STATE);
    CommandStatusIB::Builder commandStatus = mInvokeResponseBuilder.GetInvokeResponses().CreateInvokeResponse().CreateStatus();
    ReturnErrorOnFailure(commandStatus.GetError());
    ReturnErrorOnFailure(ConstructCommandPath(aCommandPathParams, commandStatus.CreatePath()));
    MoveToState(CommandState::AddingCommand);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::FinishStatus()
{
    VerifyOrReturnError(mState == CommandState::AddingCommand, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(
        mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetStatus().EndOfCommandStatusIB().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().EndOfInvokeResponseIB().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().EndOfInvokeResponses().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.EndOfInvokeResponseMessage().GetError());
    MoveToState(CommandState::AddedCommand);
    return CHIP_NO_ERROR;
}

TLV::TLVWriter * CommandHandler::GetCommandDataIBTLVWriter()
{
    if (mState != CommandState::AddingCommand)
    {
        return nullptr;
    }
    else
    {
        return mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetCommand().GetWriter();
    }
}

FabricIndex CommandHandler::GetAccessingFabricIndex() const
{
    return mpExchangeCtx->GetSessionHandle().GetFabricIndex();
}

CommandHandler * CommandHandler::Handle::Get()
{
    return (mMagic == InteractionModelEngine::GetInstance()->GetMagicNumber()) ? mpHandler : nullptr;
}

void CommandHandler::Handle::Release()
{
    if (mpHandler != nullptr)
    {
        if (mMagic == InteractionModelEngine::GetInstance()->GetMagicNumber())
        {
            mpHandler->DecrementHoldOff();
        }
        mpHandler = nullptr;
        mMagic    = 0;
    }
}

CommandHandler::Handle::Handle(CommandHandler * handle)
{
    if (handle != nullptr)
    {
        handle->IncrementHoldOff();
        mpHandler = handle;
        mMagic    = InteractionModelEngine::GetInstance()->GetMagicNumber();
    }
}

} // namespace app
} // namespace chip

CHIP_ERROR __attribute__((weak)) MatterPreCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath)
{
    return CHIP_NO_ERROR;
}
void __attribute__((weak)) MatterPostCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath) {}
