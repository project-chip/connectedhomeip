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

#include "CommandSender.h"
#include "StatusResponse.h"
#include <app/InteractionModelTimeout.h>
#include <app/TimedRequest.h>
#include <platform/LockTracker.h>
#include <protocols/Protocols.h>
#include <protocols/interaction_model/Constants.h>

namespace chip {
namespace app {
namespace {

// Gets the CommandRef if available. Error returned if we expected CommandRef and it wasn't
// provided in the response.
template <typename ParserT>
CHIP_ERROR GetRef(ParserT aParser, Optional<uint16_t> & aRef, bool commandRefRequired)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t ref;
    err = aParser.GetRef(&ref);

    VerifyOrReturnError(err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV, err);
    if (err == CHIP_END_OF_TLV)
    {
        if (commandRefRequired)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        aRef = NullOptional;
        return CHIP_NO_ERROR;
    }

    aRef = MakeOptional(ref);
    return CHIP_NO_ERROR;
}

} // namespace

CommandSender::CommandSender(Callback * apCallback, Messaging::ExchangeManager * apExchangeMgr, bool aIsTimedRequest,
                             bool aSuppressResponse, bool aAllowLargePayload) :
    mExchangeCtx(*this),
    mCallbackHandle(apCallback), mpExchangeMgr(apExchangeMgr), mSuppressResponse(aSuppressResponse), mTimedRequest(aIsTimedRequest),
    mAllowLargePayload(aAllowLargePayload)
{
    assertChipStackLockedByCurrentThread();
}

CommandSender::CommandSender(ExtendableCallback * apExtendableCallback, Messaging::ExchangeManager * apExchangeMgr,
                             bool aIsTimedRequest, bool aSuppressResponse, bool aAllowLargePayload) :
    mExchangeCtx(*this),
    mCallbackHandle(apExtendableCallback), mpExchangeMgr(apExchangeMgr), mSuppressResponse(aSuppressResponse),
    mTimedRequest(aIsTimedRequest), mUseExtendableCallback(true), mAllowLargePayload(aAllowLargePayload)
{
    assertChipStackLockedByCurrentThread();
#if CHIP_CONFIG_COMMAND_SENDER_BUILTIN_SUPPORT_FOR_BATCHED_COMMANDS
    mpPendingResponseTracker = &mNonTestPendingResponseTracker;
#endif // CHIP_CONFIG_COMMAND_SENDER_BUILTIN_SUPPORT_FOR_BATCHED_COMMANDS
}

CommandSender::~CommandSender()
{
    assertChipStackLockedByCurrentThread();
}

CHIP_ERROR CommandSender::AllocateBuffer()
{
    if (!mBufferAllocated)
    {
        mCommandMessageWriter.Reset();

        System::PacketBufferHandle commandPacket;
        if (mAllowLargePayload)
        {
            commandPacket = System::PacketBufferHandle::New(kMaxLargeSecureSduLengthBytes);
        }
        else
        {
            commandPacket = System::PacketBufferHandle::New(kMaxSecureSduLengthBytes);
        }
        VerifyOrReturnError(!commandPacket.IsNull(), CHIP_ERROR_NO_MEMORY);

        mCommandMessageWriter.Init(std::move(commandPacket));
        ReturnErrorOnFailure(mInvokeRequestBuilder.InitWithEndBufferReserved(&mCommandMessageWriter));

        mInvokeRequestBuilder.SuppressResponse(mSuppressResponse).TimedRequest(mTimedRequest);
        ReturnErrorOnFailure(mInvokeRequestBuilder.GetError());

        mInvokeRequestBuilder.CreateInvokeRequests(/* aReserveEndBuffer = */ true);
        ReturnErrorOnFailure(mInvokeRequestBuilder.GetError());

        mBufferAllocated = true;
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::SendCommandRequestInternal(const SessionHandle & session, Optional<System::Clock::Timeout> timeout)
{
    VerifyOrReturnError(mState == State::AddedCommand, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(Finalize(mPendingInvokeData));

    // Create a new exchange context.
    auto exchange = mpExchangeMgr->NewContext(session, this);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_NO_MEMORY);

    mExchangeCtx.Grab(exchange);
    VerifyOrReturnError(!mExchangeCtx->IsGroupExchangeContext(), CHIP_ERROR_INVALID_MESSAGE_TYPE);

    mExchangeCtx->SetResponseTimeout(timeout.ValueOr(session->ComputeRoundTripTimeout(app::kExpectedIMProcessingTime)));

    if (mTimedInvokeTimeoutMs.HasValue())
    {
        ReturnErrorOnFailure(TimedRequest::Send(mExchangeCtx.Get(), mTimedInvokeTimeoutMs.Value()));
        MoveToState(State::AwaitingTimedStatus);
        return CHIP_NO_ERROR;
    }

    return SendInvokeRequest();
}

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
CHIP_ERROR CommandSender::TestOnlyCommandSenderTimedRequestFlagWithNoTimedInvoke(const SessionHandle & session,
                                                                                 Optional<System::Clock::Timeout> timeout)
{
    VerifyOrReturnError(mTimedRequest, CHIP_ERROR_INCORRECT_STATE);
    return SendCommandRequestInternal(session, timeout);
}
#endif

CHIP_ERROR CommandSender::SendCommandRequest(const SessionHandle & session, Optional<System::Clock::Timeout> timeout)
{
    // If the command is expected to be large, ensure that the underlying
    // session supports it.
    if (mAllowLargePayload)
    {
        VerifyOrReturnError(session->AllowsLargePayload(), CHIP_ERROR_INCORRECT_STATE);
    }

    if (mTimedRequest != mTimedInvokeTimeoutMs.HasValue())
    {
        ChipLogError(
            DataManagement,
            "Inconsistent timed request state in CommandSender: mTimedRequest (%d) != mTimedInvokeTimeoutMs.HasValue() (%d)",
            mTimedRequest, mTimedInvokeTimeoutMs.HasValue());
        return CHIP_ERROR_INCORRECT_STATE;
    }
    return SendCommandRequestInternal(session, timeout);
}

CHIP_ERROR CommandSender::SendGroupCommandRequest(const SessionHandle & session)
{
    VerifyOrReturnError(mState == State::AddedCommand, CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(Finalize(mPendingInvokeData));

    // Create a new exchange context.
    auto exchange = mpExchangeMgr->NewContext(session, this);
    VerifyOrReturnError(exchange != nullptr, CHIP_ERROR_NO_MEMORY);

    mExchangeCtx.Grab(exchange);
    VerifyOrReturnError(mExchangeCtx->IsGroupExchangeContext(), CHIP_ERROR_INVALID_MESSAGE_TYPE);

    ReturnErrorOnFailure(SendInvokeRequest());

    Close();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::SendInvokeRequest()
{
    using namespace Protocols::InteractionModel;
    using namespace Messaging;

    ReturnErrorOnFailure(
        mExchangeCtx->SendMessage(MsgType::InvokeCommandRequest, std::move(mPendingInvokeData), SendMessageFlags::kExpectResponse));
    MoveToState(State::AwaitingResponse);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                            System::PacketBufferHandle && aPayload)
{
    using namespace Protocols::InteractionModel;

    if (mState == State::AwaitingResponse)
    {
        MoveToState(State::ResponseReceived);
    }

    CHIP_ERROR err           = CHIP_NO_ERROR;
    bool sendStatusResponse  = false;
    bool moreChunkedMessages = false;
    VerifyOrExit(apExchangeContext == mExchangeCtx.Get(), err = CHIP_ERROR_INCORRECT_STATE);
    sendStatusResponse = true;

    if (mState == State::AwaitingTimedStatus)
    {
        if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
        {
            CHIP_ERROR statusError = CHIP_NO_ERROR;
            SuccessOrExit(err = StatusResponse::ProcessStatusResponse(std::move(aPayload), statusError));
            sendStatusResponse = false;
            SuccessOrExit(err = statusError);
            err = SendInvokeRequest();
        }
        else
        {
            err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
        }
        // Skip all other processing here (which is for the response to the
        // invoke request), no matter whether err is success or not.
        goto exit;
    }

    if (aPayloadHeader.HasMessageType(MsgType::InvokeCommandResponse))
    {
        mInvokeResponseMessageCount++;
        err = ProcessInvokeResponse(std::move(aPayload), moreChunkedMessages);
        SuccessOrExit(err);
        if (moreChunkedMessages)
        {
            StatusResponse::Send(Status::Success, apExchangeContext, /*aExpectResponse = */ true);
            MoveToState(State::AwaitingResponse);
            return CHIP_NO_ERROR;
        }
        sendStatusResponse = false;
    }
    else if (aPayloadHeader.HasMessageType(MsgType::StatusResponse))
    {
        CHIP_ERROR statusError = CHIP_NO_ERROR;
        SuccessOrExit(err = StatusResponse::ProcessStatusResponse(std::move(aPayload), statusError));
        SuccessOrExit(err = statusError);
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }
    else
    {
        err = CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        OnErrorCallback(err);
    }

    if (sendStatusResponse)
    {
        StatusResponse::Send(Status::InvalidAction, apExchangeContext, /*aExpectResponse = */ false);
    }

    if (mState != State::AwaitingResponse)
    {
        if (err == CHIP_NO_ERROR)
        {
            FlushNoCommandResponse();
        }
        Close();
    }
    // Else we got a response to a Timed Request and just sent the invoke.

    return err;
}

CHIP_ERROR CommandSender::ProcessInvokeResponse(System::PacketBufferHandle && payload, bool & moreChunkedMessages)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;
    TLV::TLVReader invokeResponsesReader;
    InvokeResponseMessage::Parser invokeResponseMessage;
    InvokeResponseIBs::Parser invokeResponses;
    bool suppressResponse = false;

    reader.Init(std::move(payload));
    ReturnErrorOnFailure(invokeResponseMessage.Init(reader));

#if CHIP_CONFIG_IM_PRETTY_PRINT
    invokeResponseMessage.PrettyPrint();
#endif

    ReturnErrorOnFailure(invokeResponseMessage.GetSuppressResponse(&suppressResponse));
    ReturnErrorOnFailure(invokeResponseMessage.GetInvokeResponses(&invokeResponses));
    invokeResponses.GetReader(&invokeResponsesReader);

    while (CHIP_NO_ERROR == (err = invokeResponsesReader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == invokeResponsesReader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        InvokeResponseIB::Parser invokeResponse;
        ReturnErrorOnFailure(invokeResponse.Init(invokeResponsesReader));
        ReturnErrorOnFailure(ProcessInvokeResponseIB(invokeResponse));
    }

    err = invokeResponseMessage.GetMoreChunkedMessages(&moreChunkedMessages);
    // If the MoreChunkedMessages element is absent, we receive CHIP_END_OF_TLV. In this
    // case, per the specification, a default value of false is used.
    if (CHIP_END_OF_TLV == err)
    {
        moreChunkedMessages = false;
        err                 = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);

    if (suppressResponse && moreChunkedMessages)
    {
        ChipLogError(DataManagement, "Spec violation! InvokeResponse has suppressResponse=true, and moreChunkedMessages=true");
        // TODO Is there a better error to return here?
        return CHIP_ERROR_INVALID_TLV_ELEMENT;
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return invokeResponseMessage.ExitContainer();
}

void CommandSender::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive invoke command response from Exchange: " ChipLogFormatExchange,
                    ChipLogValueExchange(apExchangeContext));

    OnErrorCallback(CHIP_ERROR_TIMEOUT);
    Close();
}

void CommandSender::FlushNoCommandResponse()
{
    if (mpPendingResponseTracker && mUseExtendableCallback && mCallbackHandle.extendableCallback)
    {
        Optional<uint16_t> commandRef = mpPendingResponseTracker->PopPendingResponse();
        while (commandRef.HasValue())
        {
            NoResponseData noResponseData = { commandRef.Value() };
            mCallbackHandle.extendableCallback->OnNoResponse(this, noResponseData);
            commandRef = mpPendingResponseTracker->PopPendingResponse();
        }
    }
}

void CommandSender::Close()
{
    mSuppressResponse = false;
    mTimedRequest     = false;
    MoveToState(State::AwaitingDestruction);
    OnDoneCallback();
}

CHIP_ERROR CommandSender::ProcessInvokeResponseIB(InvokeResponseIB::Parser & aInvokeResponse)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ClusterId clusterId;
    CommandId commandId;
    EndpointId endpointId;
    // Default to success when an invoke response is received.
    StatusIB statusIB;

    {
        bool hasDataResponse = false;
        TLV::TLVReader commandDataReader;
        Optional<uint16_t> commandRef;
        bool commandRefRequired = (mFinishedCommandCount > 1);

        CommandStatusIB::Parser commandStatus;
        err = aInvokeResponse.GetStatus(&commandStatus);
        if (CHIP_NO_ERROR == err)
        {
            CommandPathIB::Parser commandPath;
            ReturnErrorOnFailure(commandStatus.GetPath(&commandPath));
            ReturnErrorOnFailure(commandPath.GetClusterId(&clusterId));
            ReturnErrorOnFailure(commandPath.GetCommandId(&commandId));
            ReturnErrorOnFailure(commandPath.GetEndpointId(&endpointId));

            StatusIB::Parser status;
            commandStatus.GetErrorStatus(&status);
            ReturnErrorOnFailure(status.DecodeStatusIB(statusIB));
            ReturnErrorOnFailure(GetRef(commandStatus, commandRef, commandRefRequired));
        }
        else if (CHIP_END_OF_TLV == err)
        {
            CommandDataIB::Parser commandData;
            CommandPathIB::Parser commandPath;
            ReturnErrorOnFailure(aInvokeResponse.GetCommand(&commandData));
            ReturnErrorOnFailure(commandData.GetPath(&commandPath));
            ReturnErrorOnFailure(commandPath.GetEndpointId(&endpointId));
            ReturnErrorOnFailure(commandPath.GetClusterId(&clusterId));
            ReturnErrorOnFailure(commandPath.GetCommandId(&commandId));
            commandData.GetFields(&commandDataReader);
            ReturnErrorOnFailure(GetRef(commandData, commandRef, commandRefRequired));
            err             = CHIP_NO_ERROR;
            hasDataResponse = true;
        }

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DataManagement, "Received malformed Command Response, err=%" CHIP_ERROR_FORMAT, err.Format());
        }
        else
        {
            if (hasDataResponse)
            {
                ChipLogProgress(DataManagement,
                                "Received Command Response Data, Endpoint=%u Cluster=" ChipLogFormatMEI
                                " Command=" ChipLogFormatMEI,
                                endpointId, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId));
            }
            else
            {
                ChipLogProgress(DataManagement,
                                "Received Command Response Status for Endpoint=%u Cluster=" ChipLogFormatMEI
                                " Command=" ChipLogFormatMEI " Status=0x%x",
                                endpointId, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId),
                                to_underlying(statusIB.mStatus));
            }
        }
        ReturnErrorOnFailure(err);

        if (commandRef.HasValue() && mpPendingResponseTracker != nullptr)
        {
            err = mpPendingResponseTracker->Remove(commandRef.Value());
            if (err != CHIP_NO_ERROR)
            {
                // This can happen for two reasons:
                // 1. The current InvokeResponse is a duplicate (based on its commandRef).
                // 2. The current InvokeResponse is for a request we never sent (based on its commandRef).
                // Used when logging errors related to server violating spec.
                [[maybe_unused]] ScopedNodeId remoteScopedNode;
                if (mExchangeCtx.Get() && mExchangeCtx.Get()->HasSessionHandle())
                {
                    remoteScopedNode = mExchangeCtx.Get()->GetSessionHandle()->GetPeer();
                }
                ChipLogError(DataManagement,
                             "Received Unexpected Response from remote node " ChipLogFormatScopedNodeId ", commandRef=%u",
                             ChipLogValueScopedNodeId(remoteScopedNode), commandRef.Value());
                return err;
            }
        }

        if (!commandRef.HasValue() && !commandRefRequired && mpPendingResponseTracker != nullptr &&
            mpPendingResponseTracker->Count() == 1)
        {
            // We have sent out a single invoke request. As per spec, server in this case doesn't need to provide the CommandRef
            // in the response. This is allowed to support communicating with a legacy server. In this case we assume the response
            // is associated with the only command we sent out.
            commandRef = mpPendingResponseTracker->PopPendingResponse();
        }

        // When using ExtendableCallbacks, we are adhering to a different API contract where path
        // specific errors are sent to the OnResponse callback. For more information on the history
        // of this issue please see https://github.com/project-chip/connectedhomeip/issues/30991
        if (statusIB.IsSuccess() || mUseExtendableCallback)
        {
            const ConcreteCommandPath concretePath = ConcreteCommandPath(endpointId, clusterId, commandId);
            ResponseData responseData              = { concretePath, statusIB };
            responseData.data                      = hasDataResponse ? &commandDataReader : nullptr;
            responseData.commandRef                = commandRef;
            OnResponseCallback(responseData);
        }
        else
        {
            OnErrorCallback(statusIB.ToChipError());
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::SetCommandSenderConfig(CommandSender::ConfigParameters & aConfigParams)
{
    VerifyOrReturnError(mState == State::Idle, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(aConfigParams.remoteMaxPathsPerInvoke > 0, CHIP_ERROR_INVALID_ARGUMENT);
    if (mpPendingResponseTracker != nullptr)
    {

        mRemoteMaxPathsPerInvoke = aConfigParams.remoteMaxPathsPerInvoke;
        mBatchCommandsEnabled    = (aConfigParams.remoteMaxPathsPerInvoke > 1);
    }
    else
    {
        VerifyOrReturnError(aConfigParams.remoteMaxPathsPerInvoke == 1, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::PrepareCommand(const CommandPathParams & aCommandPathParams,
                                         PrepareCommandParameters & aPrepareCommandParams)
{
    ReturnErrorOnFailure(AllocateBuffer());

    //
    // We must not be in the middle of preparing a command, and must not have already sent InvokeRequestMessage.
    //
    bool canAddAnotherCommand = (mState == State::AddedCommand && mBatchCommandsEnabled && mUseExtendableCallback);
    VerifyOrReturnError(mState == State::Idle || canAddAnotherCommand, CHIP_ERROR_INCORRECT_STATE);

    VerifyOrReturnError(mFinishedCommandCount < mRemoteMaxPathsPerInvoke, CHIP_ERROR_MAXIMUM_PATHS_PER_INVOKE_EXCEEDED);

    if (mBatchCommandsEnabled)
    {
        VerifyOrReturnError(mpPendingResponseTracker != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(aPrepareCommandParams.commandRef.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        uint16_t commandRef = aPrepareCommandParams.commandRef.Value();
        VerifyOrReturnError(!mpPendingResponseTracker->IsTracked(commandRef), CHIP_ERROR_INVALID_ARGUMENT);
    }

    InvokeRequests::Builder & invokeRequests = mInvokeRequestBuilder.GetInvokeRequests();
    CommandDataIB::Builder & invokeRequest   = invokeRequests.CreateCommandData();
    ReturnErrorOnFailure(invokeRequests.GetError());
    CommandPathIB::Builder & path = invokeRequest.CreatePath();
    ReturnErrorOnFailure(invokeRequest.GetError());
    ReturnErrorOnFailure(path.Encode(aCommandPathParams));

    if (aPrepareCommandParams.startDataStruct)
    {
        ReturnErrorOnFailure(invokeRequest.GetWriter()->StartContainer(TLV::ContextTag(CommandDataIB::Tag::kFields),
                                                                       TLV::kTLVType_Structure, mDataElementContainerType));
    }

    MoveToState(State::AddingCommand);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::FinishCommand(FinishCommandParameters & aFinishCommandParams)
{
    if (mBatchCommandsEnabled)
    {
        VerifyOrReturnError(mpPendingResponseTracker != nullptr, CHIP_ERROR_INCORRECT_STATE);
        VerifyOrReturnError(aFinishCommandParams.commandRef.HasValue(), CHIP_ERROR_INVALID_ARGUMENT);
        uint16_t commandRef = aFinishCommandParams.commandRef.Value();
        VerifyOrReturnError(!mpPendingResponseTracker->IsTracked(commandRef), CHIP_ERROR_INVALID_ARGUMENT);
    }

    return FinishCommandInternal(aFinishCommandParams);
}

CHIP_ERROR CommandSender::AddRequestData(const CommandPathParams & aCommandPath, const DataModel::EncodableToTLV & aEncodable,
                                         AddRequestDataParameters & aAddRequestDataParams)
{
    ReturnErrorOnFailure(AllocateBuffer());

    RollbackInvokeRequest rollback(*this);
    PrepareCommandParameters prepareCommandParams(aAddRequestDataParams);
    ReturnErrorOnFailure(PrepareCommand(aCommandPath, prepareCommandParams));
    TLV::TLVWriter * writer = GetCommandDataIBTLVWriter();
    VerifyOrReturnError(writer != nullptr, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(aEncodable.EncodeTo(*writer, TLV::ContextTag(CommandDataIB::Tag::kFields)));
    FinishCommandParameters finishCommandParams(aAddRequestDataParams);
    ReturnErrorOnFailure(FinishCommand(finishCommandParams));
    rollback.DisableAutomaticRollback();
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandSender::FinishCommandInternal(FinishCommandParameters & aFinishCommandParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mState == State::AddingCommand, err = CHIP_ERROR_INCORRECT_STATE);

    CommandDataIB::Builder & commandData = mInvokeRequestBuilder.GetInvokeRequests().GetCommandData();

    if (aFinishCommandParams.endDataStruct)
    {
        ReturnErrorOnFailure(commandData.GetWriter()->EndContainer(mDataElementContainerType));
    }

    if (aFinishCommandParams.commandRef.HasValue())
    {
        ReturnErrorOnFailure(commandData.Ref(aFinishCommandParams.commandRef.Value()));
    }

    ReturnErrorOnFailure(commandData.EndOfCommandDataIB());

    MoveToState(State::AddedCommand);
    mFinishedCommandCount++;

    if (mpPendingResponseTracker && aFinishCommandParams.commandRef.HasValue())
    {
        mpPendingResponseTracker->Add(aFinishCommandParams.commandRef.Value());
    }

    if (aFinishCommandParams.timedInvokeTimeoutMs.HasValue())
    {
        SetTimedInvokeTimeoutMs(aFinishCommandParams.timedInvokeTimeoutMs);
    }

    return CHIP_NO_ERROR;
}

TLV::TLVWriter * CommandSender::GetCommandDataIBTLVWriter()
{
    if (mState != State::AddingCommand)
    {
        return nullptr;
    }

    return mInvokeRequestBuilder.GetInvokeRequests().GetCommandData().GetWriter();
}

void CommandSender::SetTimedInvokeTimeoutMs(const Optional<uint16_t> & aTimedInvokeTimeoutMs)
{
    if (!mTimedInvokeTimeoutMs.HasValue())
    {
        mTimedInvokeTimeoutMs = aTimedInvokeTimeoutMs;
    }
    else if (aTimedInvokeTimeoutMs.HasValue())
    {
        uint16_t newValue = std::min(mTimedInvokeTimeoutMs.Value(), aTimedInvokeTimeoutMs.Value());
        mTimedInvokeTimeoutMs.SetValue(newValue);
    }
}

size_t CommandSender::GetInvokeResponseMessageCount()
{
    return static_cast<size_t>(mInvokeResponseMessageCount);
}

CHIP_ERROR CommandSender::Finalize(System::PacketBufferHandle & commandPacket)
{
    VerifyOrReturnError(mState == State::AddedCommand, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mInvokeRequestBuilder.GetInvokeRequests().EndOfInvokeRequests());
    ReturnErrorOnFailure(mInvokeRequestBuilder.EndOfInvokeRequestMessage());
    return mCommandMessageWriter.Finalize(&commandPacket);
}

const char * CommandSender::GetStateStr() const
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

    case State::AwaitingTimedStatus:
        return "AwaitingTimedStatus";

    case State::AwaitingResponse:
        return "AwaitingResponse";

    case State::ResponseReceived:
        return "ResponseReceived";

    case State::AwaitingDestruction:
        return "AwaitingDestruction";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void CommandSender::MoveToState(const State aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "ICR moving to [%10.10s]", GetStateStr());
}

CommandSender::RollbackInvokeRequest::RollbackInvokeRequest(CommandSender & aCommandSender) : mCommandSender(aCommandSender)
{
    VerifyOrReturn(mCommandSender.mBufferAllocated);
    VerifyOrReturn(mCommandSender.mState == State::Idle || mCommandSender.mState == State::AddedCommand);
    VerifyOrReturn(mCommandSender.mInvokeRequestBuilder.GetInvokeRequests().GetError() == CHIP_NO_ERROR);
    VerifyOrReturn(mCommandSender.mInvokeRequestBuilder.GetError() == CHIP_NO_ERROR);
    mCommandSender.mInvokeRequestBuilder.Checkpoint(mBackupWriter);
    mBackupState          = mCommandSender.mState;
    mRollbackInDestructor = true;
}

CommandSender::RollbackInvokeRequest::~RollbackInvokeRequest()
{
    VerifyOrReturn(mRollbackInDestructor);
    VerifyOrReturn(mCommandSender.mState == State::AddingCommand);
    ChipLogDetail(DataManagement, "Rolling back response");
    // TODO(#30453): Rollback of mInvokeRequestBuilder should handle resetting
    // InvokeRequests.
    mCommandSender.mInvokeRequestBuilder.GetInvokeRequests().ResetError();
    mCommandSender.mInvokeRequestBuilder.Rollback(mBackupWriter);
    mCommandSender.MoveToState(mBackupState);
    mRollbackInDestructor = false;
}

void CommandSender::RollbackInvokeRequest::DisableAutomaticRollback()
{
    mRollbackInDestructor = false;
}

} // namespace app
} // namespace chip
