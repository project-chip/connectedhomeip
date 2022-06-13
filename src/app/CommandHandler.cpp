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
#include "RequiredPrivilege.h"
#include "messaging/ExchangeContext.h"

#include <access/AccessControl.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/RequiredPrivilege.h>
#include <app/util/MatterCallbacks.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPTLVUtilities.hpp>
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
    ReturnErrorOnFailure(invokeRequestMessage.Init(reader));
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    ReturnErrorOnFailure(invokeRequestMessage.CheckSchemaValidity());
#endif
    ReturnErrorOnFailure(invokeRequestMessage.GetSuppressResponse(&mSuppressResponse));
    ReturnErrorOnFailure(invokeRequestMessage.GetTimedRequest(&mTimedRequest));
    ReturnErrorOnFailure(invokeRequestMessage.GetInvokeRequests(&invokeRequests));

    VerifyOrReturnError(mpExchangeCtx != nullptr, CHIP_ERROR_INCORRECT_STATE);

    if (mTimedRequest != isTimedInvoke)
    {
        // The message thinks it should be part of a timed interaction but it's
        // not, or vice versa.  Spec says to Respond with UNSUPPORTED_ACCESS.
        err = StatusResponse::Send(Protocols::InteractionModel::Status::UnsupportedAccess, mpExchangeCtx,
                                   /* aExpectResponse = */ false);

        if (err != CHIP_NO_ERROR)
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

    {
        // We don't support handling multiple commands but the protocol is ready to support it in the future, reject all of them and
        // IM Engine will send a status response.
        size_t commandCount = 0;
        TLV::Utilities::Count(invokeRequestsReader, commandCount, false /* recurse */);
        VerifyOrReturnError(commandCount == 1, CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    }

    while (CHIP_NO_ERROR == (err = invokeRequestsReader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == invokeRequestsReader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        CommandDataIB::Parser commandData;
        ReturnErrorOnFailure(commandData.Init(invokeRequestsReader));

        if (mpExchangeCtx->IsGroupExchangeContext())
        {
            ReturnErrorOnFailure(ProcessGroupCommandDataIB(commandData));
        }
        else
        {
            ReturnErrorOnFailure(ProcessCommandDataIB(commandData));
        }
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return invokeRequestMessage.ExitContainer();
}

void CommandHandler::Close()
{
    mSuppressResponse = false;
    MoveToState(State::AwaitingDestruction);

    // We must finish all async work before we can shut down a CommandHandler. The actual CommandHandler MUST finish their work
    // in reasonable time or there is a bug. The only case for releasing CommandHandler without CommandHandler::Handle releasing its
    // reference is the stack shutting down, in which case Close() is not called. So the below check should always pass.
    VerifyOrDieWithMsg(mPendingWork == 0, DataManagement, "CommandHandler::Close() called with %u unfinished async work items",
                       static_cast<unsigned int>(mPendingWork));

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
    ChipLogDetail(DataManagement, "Decreasing reference count for CommandHandler, remaining %u",
                  static_cast<unsigned int>(mPendingWork));
    if (mPendingWork != 0)
    {
        return;
    }

    if (mpExchangeCtx->IsGroupExchangeContext())
    {
        mpExchangeCtx->Close();
    }
    else
    {
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

    err = commandPath.GetEndpointId(&concretePath.mEndpointId);
    SuccessOrExit(err);

    using Protocols::InteractionModel::Status;
    {
        Status commandExists = mpCallback->CommandExists(concretePath);
        if (commandExists != Status::Success)
        {
            ChipLogDetail(DataManagement, "No command " ChipLogFormatMEI " in Cluster " ChipLogFormatMEI " on Endpoint 0x%x",
                          ChipLogValueMEI(concretePath.mCommandId), ChipLogValueMEI(concretePath.mClusterId),
                          concretePath.mEndpointId);
            return AddStatus(concretePath, commandExists);
        }
    }

    VerifyOrExit(mpExchangeCtx != nullptr && mpExchangeCtx->HasSessionHandle(), err = CHIP_ERROR_INCORRECT_STATE);

    {
        Access::SubjectDescriptor subjectDescriptor = GetSubjectDescriptor();
        Access::RequestPath requestPath{ .cluster = concretePath.mClusterId, .endpoint = concretePath.mEndpointId };
        Access::Privilege requestPrivilege = RequiredPrivilege::ForInvokeCommand(concretePath);
        err                                = Access::GetAccessControl().Check(subjectDescriptor, requestPath, requestPrivilege);
        if (err != CHIP_NO_ERROR)
        {
            if (err != CHIP_ERROR_ACCESS_DENIED)
            {
                return AddStatus(concretePath, Status::Failure);
            }
            // TODO: when wildcard invokes are supported, handle them to discard rather than fail with status
            return AddStatus(concretePath, Status::UnsupportedAccess);
        }
    }

    if (CommandNeedsTimedInvoke(concretePath.mClusterId, concretePath.mCommandId) && !IsTimedInvoke())
    {
        // TODO: when wildcard invokes are supported, discard a
        // wildcard-expanded path instead of returning a status.
        return AddStatus(concretePath, Protocols::InteractionModel::Status::NeedsTimedInteraction);
    }

    err = aCommandElement.GetFields(&commandDataReader);
    if (CHIP_END_OF_TLV == err)
    {
        ChipLogDetail(DataManagement,
                      "Received command without data for Endpoint=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                      concretePath.mEndpointId, ChipLogValueMEI(concretePath.mClusterId), ChipLogValueMEI(concretePath.mCommandId));
        err = CHIP_NO_ERROR;
    }
    if (CHIP_NO_ERROR == err)
    {
        ChipLogDetail(DataManagement, "Received command for Endpoint=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                      concretePath.mEndpointId, ChipLogValueMEI(concretePath.mClusterId), ChipLogValueMEI(concretePath.mCommandId));
        SuccessOrExit(MatterPreCommandReceivedCallback(concretePath));
        mpCallback->DispatchCommand(*this, concretePath, commandDataReader);
        MatterPostCommandReceivedCallback(concretePath);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        return AddStatus(concretePath, Status::InvalidCommand);
    }

    // We have handled the error status above and put the error status in response, now return success status so we can process
    // other commands in the invoke request.
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::ProcessGroupCommandDataIB(CommandDataIB::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPath;
    TLV::TLVReader commandDataReader;
    ClusterId clusterId;
    CommandId commandId;
    GroupId groupId;
    FabricIndex fabric;

    Credentials::GroupDataProvider::GroupEndpoint mapping;
    Credentials::GroupDataProvider * groupDataProvider = Credentials::GetGroupDataProvider();
    Credentials::GroupDataProvider::EndpointIterator * iterator;

    err = aCommandElement.GetPath(&commandPath);
    SuccessOrExit(err);

    err = commandPath.GetClusterId(&clusterId);
    SuccessOrExit(err);

    err = commandPath.GetCommandId(&commandId);
    SuccessOrExit(err);

    groupId = mpExchangeCtx->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
    fabric  = GetAccessingFabricIndex();

    ChipLogDetail(DataManagement, "Received group command for Group=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                  groupId, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId));

    err = aCommandElement.GetFields(&commandDataReader);
    if (CHIP_END_OF_TLV == err)
    {
        ChipLogDetail(DataManagement,
                      "Received command without data for Group=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI, groupId,
                      ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId));
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    // Per spec, we do the "is this a timed command?" check for every path, but
    // since all paths that fail it just get silently discarded we can do it
    // once up front and discard all the paths at once.  Ordering with respect
    // to ACL and command presence checks does not matter, because the behavior
    // is the same for all of them: ignore the path.
    if (CommandNeedsTimedInvoke(clusterId, commandId))
    {
        // Group commands are never timed.
        ExitNow();
    }

    iterator = groupDataProvider->IterateEndpoints(fabric);
    VerifyOrExit(iterator != nullptr, err = CHIP_ERROR_NO_MEMORY);

    while (iterator->Next(mapping))
    {
        if (groupId != mapping.group_id)
        {
            continue;
        }

        ChipLogDetail(DataManagement,
                      "Processing group command for Endpoint=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                      mapping.endpoint_id, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId));

        const ConcreteCommandPath concretePath(mapping.endpoint_id, clusterId, commandId);

        if (mpCallback->CommandExists(concretePath) != Protocols::InteractionModel::Status::Success)
        {
            ChipLogDetail(DataManagement, "No command " ChipLogFormatMEI " in Cluster " ChipLogFormatMEI " on Endpoint 0x%x",
                          ChipLogValueMEI(mapping.endpoint_id), ChipLogValueMEI(clusterId), mapping.endpoint_id);

            continue;
        }

        {
            Access::SubjectDescriptor subjectDescriptor = GetSubjectDescriptor();
            Access::RequestPath requestPath{ .cluster = concretePath.mClusterId, .endpoint = concretePath.mEndpointId };
            Access::Privilege requestPrivilege = RequiredPrivilege::ForInvokeCommand(concretePath);
            err                                = Access::GetAccessControl().Check(subjectDescriptor, requestPath, requestPrivilege);
            if (err != CHIP_NO_ERROR)
            {
                // NOTE: an expected error is CHIP_ERROR_ACCESS_DENIED, but there could be other unexpected errors;
                // therefore, keep processing subsequent commands, and if any errors continue, those subsequent
                // commands will likewise fail.
                continue;
            }
        }

        if ((err = MatterPreCommandReceivedCallback(concretePath)) == CHIP_NO_ERROR)
        {
            TLV::TLVReader dataReader(commandDataReader);
            mpCallback->DispatchCommand(*this, concretePath, dataReader);
            MatterPostCommandReceivedCallback(concretePath);
        }
        else
        {
            ChipLogError(DataManagement,
                         "Error when calling MatterPreCommandReceivedCallback for Endpoint=%u Cluster=" ChipLogFormatMEI
                         " Command=" ChipLogFormatMEI " : %" CHIP_ERROR_FORMAT,
                         mapping.endpoint_id, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId), err.Format());
            continue;
        }
    }
    iterator->Release();

exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::AddStatusInternal(const ConcreteCommandPath & aCommandPath, const StatusIB & aStatus)
{
    ReturnErrorOnFailure(PrepareStatus(aCommandPath));
    CommandStatusIB::Builder & commandStatus = mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetStatus();
    StatusIB::Builder & statusIBBuilder      = commandStatus.CreateErrorStatus();
    ReturnErrorOnFailure(commandStatus.GetError());
    statusIBBuilder.EncodeStatusIB(aStatus);
    ReturnErrorOnFailure(statusIBBuilder.GetError());
    return FinishStatus();
}

CHIP_ERROR CommandHandler::AddStatus(const ConcreteCommandPath & aCommandPath, const Protocols::InteractionModel::Status aStatus)
{
    return AddStatusInternal(aCommandPath, StatusIB(aStatus));
}

CHIP_ERROR CommandHandler::AddClusterSpecificSuccess(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus)
{
    using Protocols::InteractionModel::Status;
    return AddStatusInternal(aCommandPath, StatusIB(Status::Success, aClusterStatus));
}

CHIP_ERROR CommandHandler::AddClusterSpecificFailure(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus)
{
    using Protocols::InteractionModel::Status;
    return AddStatusInternal(aCommandPath, StatusIB(Status::Failure, aClusterStatus));
}

CHIP_ERROR CommandHandler::PrepareCommand(const ConcreteCommandPath & aCommandPath, bool aStartDataStruct)
{
    ReturnErrorOnFailure(AllocateBuffer());

    mInvokeResponseBuilder.Checkpoint(mBackupWriter);
    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrReturnError(mState == State::Idle, CHIP_ERROR_INCORRECT_STATE);
    MoveToState(State::Preparing);
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
        ReturnErrorOnFailure(commandData.GetWriter()->StartContainer(TLV::ContextTag(to_underlying(CommandDataIB::Tag::kFields)),
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
    MoveToState(State::Preparing);
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

CHIP_ERROR CommandHandler::RollbackResponse()
{
    VerifyOrReturnError(mState == State::Preparing || mState == State::AddingCommand, CHIP_ERROR_INCORRECT_STATE);
    mInvokeResponseBuilder.Rollback(mBackupWriter);
    mInvokeResponseBuilder.ResetError();
    // Note: We only support one command per request, so we reset the state to Idle here, need to review the states when adding
    // supports of having multiple requests in the same transaction.
    MoveToState(State::Idle);
    return CHIP_NO_ERROR;
}

TLV::TLVWriter * CommandHandler::GetCommandDataIBTLVWriter()
{
    if (mState != State::AddingCommand)
    {
        return nullptr;
    }

    return mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetCommand().GetWriter();
}

FabricIndex CommandHandler::GetAccessingFabricIndex() const
{
    return mpExchangeCtx->GetSessionHandle()->GetFabricIndex();
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

    case State::Preparing:
        return "Preparing";

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
