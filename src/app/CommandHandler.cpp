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
#include "InteractionModelEngine.h"
#include "messaging/ExchangeContext.h"

#include <access/AccessControl.h>
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
        mBufferAllocated = true;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::OnInvokeCommandRequest(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                                  System::PacketBufferHandle && payload, bool isTimedInvoke)
{
    System::PacketBufferHandle response;
    VerifyOrReturnError(mState == State::Idle, CHIP_ERROR_INCORRECT_STATE);

    // NOTE: we already know this is an InvokeCommand Request message because we explicitly registered with the
    // Exchange Manager for unsolicited InvokeCommand Requests.
    mpExchangeCtx = ec;

    // Use the RAII feature, if this is the only Handle when this function returns, DecrementHoldOff will trigger sending response.
    // TODO: This is broken!  If something under here returns error, we will try
    // to SendCommandResponse(), and then our caller will try to send a status
    // response too.  Figure out at what point it's our responsibility to
    // handler errors vs our caller's.
    Handle workHandle(this);
    mpExchangeCtx->WillSendMessage();
    ReturnErrorOnFailure(ProcessInvokeRequest(std::move(payload), isTimedInvoke));

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::ProcessInvokeRequest(System::PacketBufferHandle && payload, bool isTimedInvoke)
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

    if (mTimedRequest != isTimedInvoke)
    {
        // The message thinks it should be part of a timed interaction but it's
        // not, or vice versa.  Spec says to Respond with UNSUPPORTED_ACCESS.
        err = StatusResponse::Send(Protocols::InteractionModel::Status::UnsupportedAccess, mpExchangeCtx,
                                   /* aExpectResponse = */ false);

        // Some unit tests call this function in an abnormal state when we don't
        // even have an exchange.
        if (err != CHIP_NO_ERROR && mpExchangeCtx)
        {
            // We have to manually close the exchange, because we called
            // WillSendMessage already.
            mpExchangeCtx->Close();
        }

        // Null out the (now-closed) exchange, so that when we try to
        // SendCommandResponse() later (when our holdoff count drops to 0) it
        // just fails and we don't double-respond.
        mpExchangeCtx = nullptr;
        return err;
    }

    invokeRequests.GetReader(&invokeRequestsReader);
    while (CHIP_NO_ERROR == (err = invokeRequestsReader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == invokeRequestsReader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
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
    MoveToState(State::AwaitingDestruction);

    // We must finish all async work before we can shut down a CommandHandler. The actual CommandHandler MUST finish their work
    // in reasonable time or there is a bug. The only case for releasing CommandHandler without CommandHandler::Handle releasing its
    // reference is the stack shutting down, in which case Close() is not called. So the below check should always pass.
    VerifyOrDieWithMsg(mPendingWork == 0, DataManagement, "CommandHandler::Close() called with %zu unfinished async work items",
                       mPendingWork);

    // OnDone below can destroy us before we unwind all the way back into the
    // exchange code and it tries to close itself.  Make sure that it doesn't
    // try to notify us that it's closing, since we will be dead.
    //
    // For more details, see #10344.
    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->SetDelegate(nullptr);
    }

    mpExchangeCtx = nullptr;

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

    CHIP_ERROR err = CHIP_NO_ERROR;
    if (!mpExchangeCtx->IsGroupExchangeContext())
    {
        err = SendCommandResponse();
    }

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
    VerifyOrReturnError(mState == State::AddedCommand, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mpExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(Finalize(commandPacket));
    ReturnErrorOnFailure(
        mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandResponse, std::move(commandPacket)));
    // The ExchangeContext is automatically freed here, and it makes mpExchangeCtx be temporarily dangling, but in
    // all cases, we are going to call Close immediately after this function, which nulls out mpExchangeCtx.

    MoveToState(State::CommandSent);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::ProcessCommandDataIB(CommandDataIB::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPath;
    ConcreteCommandPath concretePath(0, 0, 0);
    TLV::TLVReader commandDataReader;

    // NOTE: errors may occur before the concrete command path is even fully decoded.

    err = aCommandElement.GetPath(&commandPath);
    SuccessOrExit(err);

    err = commandPath.GetClusterId(&concretePath.mClusterId);
    SuccessOrExit(err);

    err = commandPath.GetCommandId(&concretePath.mCommandId);
    SuccessOrExit(err);

    if (mpExchangeCtx != nullptr && mpExchangeCtx->IsGroupExchangeContext())
    {
        // TODO retrieve Endpoint ID with GroupDataProvider using GroupId and FabricId
        // Issue 11075

        // Using endpoint 1 for test purposes
        concretePath.mEndpointId = 1;
        err                      = CHIP_NO_ERROR;
    }
    else
    {
        err = commandPath.GetEndpointId(&concretePath.mEndpointId);
    }
    SuccessOrExit(err);

    VerifyOrExit(mpCallback->CommandExists(concretePath), err = CHIP_ERROR_INVALID_PROFILE_ID);

    {
        Access::SubjectDescriptor subjectDescriptor; // TODO: get actual subject descriptor
        Access::RequestPath requestPath{ .cluster = concretePath.mClusterId, .endpoint = concretePath.mEndpointId };
        Access::Privilege requestPrivilege = Access::Privilege::kOperate; // TODO: get actual request privilege
        err                                = Access::GetAccessControl().Check(subjectDescriptor, requestPath, requestPrivilege);
        err                                = CHIP_NO_ERROR; // TODO: remove override
        if (err != CHIP_NO_ERROR)
        {
            if (err != CHIP_ERROR_ACCESS_DENIED)
            {
                return AddStatus(concretePath, Protocols::InteractionModel::Status::Failure);
            }
            // TODO: when wildcard/group invokes are supported, handle them to discard rather than fail with status
            return AddStatus(concretePath, Protocols::InteractionModel::Status::UnsupportedAccess);
        }
    }

    err = aCommandElement.GetData(&commandDataReader);
    if (CHIP_END_OF_TLV == err)
    {
        ChipLogDetail(DataManagement,
                      "Received command without data for Endpoint=%" PRIu16 " Cluster=" ChipLogFormatMEI
                      " Command=" ChipLogFormatMEI,
                      concretePath.mEndpointId, ChipLogValueMEI(concretePath.mClusterId), ChipLogValueMEI(concretePath.mCommandId));
        err = CHIP_NO_ERROR;
    }
    if (CHIP_NO_ERROR == err)
    {
        ChipLogDetail(DataManagement,
                      "Received command for Endpoint=%" PRIu16 " Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                      concretePath.mEndpointId, ChipLogValueMEI(concretePath.mClusterId), ChipLogValueMEI(concretePath.mCommandId));
        SuccessOrExit(MatterPreCommandReceivedCallback(concretePath));
        mpCallback->DispatchCommand(*this, concretePath, commandDataReader);
        MatterPostCommandReceivedCallback(concretePath);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        // The Path is the path in the request if there are any error occurred before we dispatch the command to clusters.
        // Currently, it could be failed to decode Path or failed to find cluster / command on desired endpoint.
        // TODO: The behavior when receiving a malformed message is not clear in the Spec. (Spec#3259)
        // TODO: The error code should be updated after #7072 added error codes required by IM.
        if (err == CHIP_ERROR_INVALID_PROFILE_ID)
        {
            ChipLogDetail(DataManagement, "No Cluster " ChipLogFormatMEI " on Endpoint 0x%" PRIx16,
                          ChipLogValueMEI(concretePath.mClusterId), concretePath.mEndpointId);
        }

        // TODO:in particular different reasons for ServerClusterCommandExists to test false should result in different errors here
        AddStatus(concretePath, Protocols::InteractionModel::Status::InvalidCommand);
    }

    // We have handled the error status above and put the error status in response, now return success status so we can process
    // other commands in the invoke request.
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::AddStatusInternal(const ConcreteCommandPath & aCommandPath,
                                             const Protocols::InteractionModel::Status aStatus,
                                             const Optional<ClusterStatus> & aClusterStatus)
{
    StatusIB statusIB;
    ReturnLogErrorOnFailure(PrepareStatus(aCommandPath));
    CommandStatusIB::Builder & commandStatus = mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetStatus();
    StatusIB::Builder & statusIBBuilder      = commandStatus.CreateErrorStatus();
    ReturnErrorOnFailure(commandStatus.GetError());
    //
    // TODO: Most of the callers are incorrectly passing SecureChannel as the protocol ID, when in fact, the status code provided
    // above is always an IM code. Instead of fixing all the callers (which is a fairly sizeable change), we'll embark on fixing
    // this more completely when we fix #9530.
    //
    statusIB.mStatus        = aStatus;
    statusIB.mClusterStatus = aClusterStatus;
    statusIBBuilder.EncodeStatusIB(statusIB);
    ReturnErrorOnFailure(statusIBBuilder.GetError());
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

CHIP_ERROR CommandHandler::PrepareCommand(const ConcreteCommandPath & aCommandPath, bool aStartDataStruct)
{
    ReturnErrorOnFailure(AllocateBuffer());
    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrReturnError(mState == State::Idle, CHIP_ERROR_INCORRECT_STATE);
    InvokeResponseIBs::Builder & invokeResponses = mInvokeResponseBuilder.GetInvokeResponses();
    InvokeResponseIB::Builder & invokeResponse   = invokeResponses.CreateInvokeResponse();
    ReturnErrorOnFailure(invokeResponses.GetError());

    CommandDataIB::Builder & commandData = invokeResponse.CreateCommand();
    ReturnErrorOnFailure(commandData.GetError());
    CommandPathIB::Builder & path = commandData.CreatePath();
    ReturnErrorOnFailure(commandData.GetError());
    ReturnErrorOnFailure(path.Encode(aCommandPath));
    if (aStartDataStruct)
    {
        ReturnErrorOnFailure(commandData.GetWriter()->StartContainer(TLV::ContextTag(to_underlying(CommandDataIB::Tag::kData)),
                                                                     TLV::kTLVType_Structure, mDataElementContainerType));
    }
    MoveToState(State::AddingCommand);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::FinishCommand(bool aStartDataStruct)
{
    VerifyOrReturnError(mState == State::AddingCommand, CHIP_ERROR_INCORRECT_STATE);
    CommandDataIB::Builder & commandData = mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetCommand();
    if (aStartDataStruct)
    {
        ReturnErrorOnFailure(commandData.GetWriter()->EndContainer(mDataElementContainerType));
    }
    ReturnErrorOnFailure(commandData.EndOfCommandDataIB().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().EndOfInvokeResponseIB().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().EndOfInvokeResponses().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.EndOfInvokeResponseMessage().GetError());
    MoveToState(State::AddedCommand);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::PrepareStatus(const ConcreteCommandPath & aCommandPath)
{
    ReturnErrorOnFailure(AllocateBuffer());
    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrReturnError(mState == State::Idle, CHIP_ERROR_INCORRECT_STATE);
    InvokeResponseIBs::Builder & invokeResponses = mInvokeResponseBuilder.GetInvokeResponses();
    InvokeResponseIB::Builder & invokeResponse   = invokeResponses.CreateInvokeResponse();
    ReturnErrorOnFailure(invokeResponses.GetError());
    CommandStatusIB::Builder & commandStatus = invokeResponse.CreateStatus();
    ReturnErrorOnFailure(commandStatus.GetError());
    CommandPathIB::Builder & path = commandStatus.CreatePath();
    ReturnErrorOnFailure(commandStatus.GetError());
    ReturnErrorOnFailure(path.Encode(aCommandPath));
    MoveToState(State::AddingCommand);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::FinishStatus()
{
    VerifyOrReturnError(mState == State::AddingCommand, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(
        mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetStatus().EndOfCommandStatusIB().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().EndOfInvokeResponseIB().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().EndOfInvokeResponses().GetError());
    ReturnErrorOnFailure(mInvokeResponseBuilder.EndOfInvokeResponseMessage().GetError());
    MoveToState(State::AddedCommand);
    return CHIP_NO_ERROR;
}

TLV::TLVWriter * CommandHandler::GetCommandDataIBTLVWriter()
{
    if (mState != State::AddingCommand)
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

CHIP_ERROR CommandHandler::Finalize(System::PacketBufferHandle & commandPacket)
{
    VerifyOrReturnError(mState == State::AddedCommand, CHIP_ERROR_INCORRECT_STATE);
    return mCommandMessageWriter.Finalize(&commandPacket);
}

const char * CommandHandler::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case State::Idle:
        return "Idle";

    case State::AddingCommand:
        return "AddingCommand";

    case State::AddedCommand:
        return "AddedCommand";

    case State::CommandSent:
        return "CommandSent";

    case State::AwaitingDestruction:
        return "AwaitingDestruction";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void CommandHandler::MoveToState(const State aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "ICR moving to [%10.10s]", GetStateStr());
}

void CommandHandler::Abort()
{
    //
    // If the exchange context hasn't already been gracefully closed
    // (signaled by setting it to null), then we need to forcibly
    // tear it down.
    //
    if (mpExchangeCtx != nullptr)
    {
        // We might be a delegate for this exchange, and we don't want the
        // OnExchangeClosing notification in that case.  Null out the delegate
        // to avoid that.
        //
        // TODO: This makes all sorts of assumptions about what the delegate is
        // (notice the "might" above!) that might not hold in practice.  We
        // really need a better solution here....
        mpExchangeCtx->SetDelegate(nullptr);
        mpExchangeCtx->Abort();
        mpExchangeCtx = nullptr;
    }
}
} // namespace app
} // namespace chip

CHIP_ERROR __attribute__((weak)) MatterPreCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath)
{
    return CHIP_NO_ERROR;
}
void __attribute__((weak)) MatterPostCommandReceivedCallback(const chip::app::ConcreteCommandPath & commandPath) {}
