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
#include <lib/core/CHIPConfig.h>
#include <lib/core/TLVData.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/TypeTraits.h>
#include <platform/LockTracker.h>
#include <protocols/secure_channel/Constants.h>

namespace chip {
namespace app {
using Status = Protocols::InteractionModel::Status;

CommandHandler::CommandHandler(Callback * apCallback) :
    mpCallback(apCallback), mResponseSenderDone(HandleOnResponseSenderDone, this), mSuppressResponse(false)
{}

CommandHandler::CommandHandler(TestOnlyMarker aTestMarker, Callback * apCallback, CommandPathRegistry * apCommandPathRegistry) :
    CommandHandler(apCallback)
{
    mMaxPathsPerInvoke   = apCommandPathRegistry->MaxSize();
    mCommandPathRegistry = apCommandPathRegistry;
}

CHIP_ERROR CommandHandler::AllocateBuffer()
{
    if (!mBufferAllocated)
    {
        mCommandMessageWriter.Reset();

        System::PacketBufferHandle commandPacket = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
        VerifyOrReturnError(!commandPacket.IsNull(), CHIP_ERROR_NO_MEMORY);

        mCommandMessageWriter.Init(std::move(commandPacket));
        ReturnErrorOnFailure(mInvokeResponseBuilder.InitWithEndBufferReserved(&mCommandMessageWriter));

        if (mReserveSpaceForMoreChunkMessages)
        {
            ReturnErrorOnFailure(mInvokeResponseBuilder.ReserveSpaceForMoreChunkedMessages());
        }

        // Sending an InvokeResponse to an InvokeResponse is going to be removed from the spec soon.
        // It was never implemented in the SDK, and there are no command responses that expect a
        // command response. This means we will never receive an InvokeResponse Message in response
        // to an InvokeResponse Message that we are sending. This means that the only response
        // we are expecting to receive in response to an InvokeResponse Message that we are
        // sending-out is a status when we are chunking multiple responses. As a result, to satisfy the
        // condition that we don't set SuppressResponse to true while also setting
        // MoreChunkedMessages to true, we are hardcoding the value to false here.
        mInvokeResponseBuilder.SuppressResponse(/* aSuppressResponse = */ false);
        ReturnErrorOnFailure(mInvokeResponseBuilder.GetError());

        mInvokeResponseBuilder.CreateInvokeResponses(/* aReserveEndBuffer = */ true);
        ReturnErrorOnFailure(mInvokeResponseBuilder.GetError());

        mBufferAllocated = true;
        MoveToState(State::NewResponseMessage);
    }

    return CHIP_NO_ERROR;
}

void CommandHandler::OnInvokeCommandRequest(Messaging::ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                            System::PacketBufferHandle && payload, bool isTimedInvoke)
{
    System::PacketBufferHandle response;
    Status status = Status::Failure;
    VerifyOrDieWithMsg(ec != nullptr, DataManagement, "Incoming exchange context should not be null");
    VerifyOrDieWithMsg(mState == State::Idle, DataManagement, "state should be Idle");

    // NOTE: we already know this is an InvokeCommand Request message because we explicitly registered with the
    // Exchange Manager for unsolicited InvokeCommand Requests.
    mResponseSender.SetExchangeContext(ec);

    // Use the RAII feature, if this is the only Handle when this function returns, DecrementHoldOff will trigger sending response.
    // TODO: This is broken!  If something under here returns error, we will try
    // to StartSendingCommandResponses(), and then our caller will try to send a status
    // response too.  Figure out at what point it's our responsibility to
    // handler errors vs our caller's.
    Handle workHandle(this);

    // TODO(#30453): It should be possible for SetExchangeContext to internally call WillSendMessage.
    // Unfortunately, doing so would require us to either:
    // * Make TestCommandInteraction a friend of CommandResponseSender to allow it to set the exchange
    //   context without calling WillSendMessage, or
    // * Understand why unit tests fail when WillSendMessage is called during the execution of
    //   SetExchangeContext.
    mResponseSender.WillSendMessage();
    status = ProcessInvokeRequest(std::move(payload), isTimedInvoke);
    if (status != Status::Success)
    {
        mResponseSender.SendStatusResponse(status);
        mSentStatusResponse = true;
    }

    mGoneAsync = true;
}

CHIP_ERROR CommandHandler::ValidateInvokeRequestMessageAndBuildRegistry(InvokeRequestMessage::Parser & invokeRequestMessage)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    size_t commandCount     = 0;
    bool commandRefExpected = false;
    InvokeRequests::Parser invokeRequests;

    ReturnErrorOnFailure(invokeRequestMessage.GetInvokeRequests(&invokeRequests));
    TLV::TLVReader invokeRequestsReader;
    invokeRequests.GetReader(&invokeRequestsReader);

    ReturnErrorOnFailure(TLV::Utilities::Count(invokeRequestsReader, commandCount, false /* recurse */));

    // If this is a GroupRequest the only thing to check is that there is only one
    // CommandDataIB.
    if (IsGroupRequest())
    {
        VerifyOrReturnError(commandCount == 1, CHIP_ERROR_INVALID_ARGUMENT);
        return CHIP_NO_ERROR;
    }
    // While technically any commandCount == 1 should already be unique and does not need
    // any further validation, we do need to read and populate the registry to help
    // in building the InvokeResponse.

    VerifyOrReturnError(commandCount <= MaxPathsPerInvoke(), CHIP_ERROR_INVALID_ARGUMENT);

    // If there is more than one CommandDataIB, spec states that CommandRef must be provided.
    commandRefExpected = commandCount > 1;

    while (CHIP_NO_ERROR == (err = invokeRequestsReader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == invokeRequestsReader.GetTag(), CHIP_ERROR_INVALID_ARGUMENT);
        CommandDataIB::Parser commandData;
        ReturnErrorOnFailure(commandData.Init(invokeRequestsReader));

        // First validate that we can get a ConcreteCommandPath.
        CommandPathIB::Parser commandPath;
        ConcreteCommandPath concretePath(0, 0, 0);
        ReturnErrorOnFailure(commandData.GetPath(&commandPath));
        ReturnErrorOnFailure(commandPath.GetConcreteCommandPath(concretePath));

        // Grab the CommandRef if there is one, and validate that it's there when it
        // has to be.
        Optional<uint16_t> commandRef;
        uint16_t ref;
        err = commandData.GetRef(&ref);
        VerifyOrReturnError(err == CHIP_NO_ERROR || err == CHIP_END_OF_TLV, err);
        if (err == CHIP_END_OF_TLV && commandRefExpected)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        if (err == CHIP_NO_ERROR)
        {
            commandRef.SetValue(ref);
        }

        // Adding can fail if concretePath is not unique, or if commandRef is a value
        // and is not unique, or if we have already added more paths than we support.
        ReturnErrorOnFailure(GetCommandPathRegistry().Add(concretePath, commandRef));
    }

    // It's OK/expected to have reached the end of the container without failure.
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    ReturnErrorOnFailure(err);
    return invokeRequestMessage.ExitContainer();
}

Status CommandHandler::ProcessInvokeRequest(System::PacketBufferHandle && payload, bool isTimedInvoke)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;
    InvokeRequestMessage::Parser invokeRequestMessage;
    InvokeRequests::Parser invokeRequests;
    reader.Init(std::move(payload));
    VerifyOrReturnError(invokeRequestMessage.Init(reader) == CHIP_NO_ERROR, Status::InvalidAction);
#if CHIP_CONFIG_IM_PRETTY_PRINT
    invokeRequestMessage.PrettyPrint();
#endif
    if (mResponseSender.IsForGroup())
    {
        SetGroupRequest(true);
    }

    // When updating this code, please remember to make corresponding changes to TestOnlyInvokeCommandRequestWithFaultsInjected.
    VerifyOrReturnError(invokeRequestMessage.GetSuppressResponse(&mSuppressResponse) == CHIP_NO_ERROR, Status::InvalidAction);
    VerifyOrReturnError(invokeRequestMessage.GetTimedRequest(&mTimedRequest) == CHIP_NO_ERROR, Status::InvalidAction);
    VerifyOrReturnError(invokeRequestMessage.GetInvokeRequests(&invokeRequests) == CHIP_NO_ERROR, Status::InvalidAction);
    VerifyOrReturnError(mTimedRequest == isTimedInvoke, Status::TimedRequestMismatch);

    {
        InvokeRequestMessage::Parser validationInvokeRequestMessage = invokeRequestMessage;
        VerifyOrReturnError(ValidateInvokeRequestMessageAndBuildRegistry(validationInvokeRequestMessage) == CHIP_NO_ERROR,
                            Status::InvalidAction);
    }

    TLV::TLVReader invokeRequestsReader;
    invokeRequests.GetReader(&invokeRequestsReader);

    size_t commandCount = 0;
    VerifyOrReturnError(TLV::Utilities::Count(invokeRequestsReader, commandCount, false /* recurse */) == CHIP_NO_ERROR,
                        Status::InvalidAction);
    if (commandCount > 1)
    {
        mReserveSpaceForMoreChunkMessages = true;
    }

    while (CHIP_NO_ERROR == (err = invokeRequestsReader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == invokeRequestsReader.GetTag(), Status::InvalidAction);
        CommandDataIB::Parser commandData;
        VerifyOrReturnError(commandData.Init(invokeRequestsReader) == CHIP_NO_ERROR, Status::InvalidAction);
        Status status = Status::Success;
        if (IsGroupRequest())
        {
            status = ProcessGroupCommandDataIB(commandData);
        }
        else
        {
            status = ProcessCommandDataIB(commandData);
        }
        if (status != Status::Success)
        {
            return status;
        }
    }

    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::InvalidAction);
    VerifyOrReturnError(invokeRequestMessage.ExitContainer() == CHIP_NO_ERROR, Status::InvalidAction);
    return Status::Success;
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

    if (mpCallback)
    {
        mpCallback->OnDone(*this);
    }
}

void CommandHandler::HandleOnResponseSenderDone(void * context)
{
    CommandHandler * const _this = static_cast<CommandHandler *>(context);
    VerifyOrDie(_this != nullptr);

    _this->Close();
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

    if (!mSentStatusResponse)
    {
        if (!mResponseSender.HasExchangeContext())
        {
            ChipLogProgress(DataManagement, "Skipping command response: exchange context is null");
        }
        else if (!IsGroupRequest())
        {
            CHIP_ERROR err = StartSendingCommandResponses();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DataManagement, "Failed to send command response: %" CHIP_ERROR_FORMAT, err.Format());
                // TODO(#30453): It should be our responsibility to send a Failure StatusResponse to the requestor
                // if there is a SessionHandle, but legacy unit tests explicitly check the behavior where
                // CommandHandler does not send any message. Changing this behavior should be done in a standalone
                // PR where only that specific change is made. Here is a possible solution that should
                // be done that fulfills our responsibility to send a Failure StatusResponse, but this causes unit
                // tests to start failing.
                //   ```
                //   if (mResponseSender.HasSessionHandle())
                //   {
                //       mResponseSender.SendStatusResponse(Status::Failure);
                //   }
                //   Close();
                //   return;
                //   ```
            }
        }
    }

    if (mResponseSender.AwaitingStatusResponse())
    {
        // If we are awaiting a status response, we want to call Close() only once the response sender is done.
        // Therefore, register to be notified when CommandResponseSender is done.
        mResponseSender.RegisterOnResponseSenderDoneCallback(&mResponseSenderDone);
        return;
    }
    Close();
}

CHIP_ERROR CommandHandler::StartSendingCommandResponses()
{
    VerifyOrReturnError(mPendingWork == 0, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mState == State::AddedCommand, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mResponseSender.HasExchangeContext(), CHIP_ERROR_INCORRECT_STATE);

    ReturnErrorOnFailure(FinalizeLastInvokeResponseMessage());
    ReturnErrorOnFailure(mResponseSender.StartSendingCommandResponses());
    return CHIP_NO_ERROR;
}

namespace {
// We use this when the sender did not actually provide a CommandFields struct,
// to avoid downstream consumers having to worry about cases when there is or is
// not a struct available.  We use an empty struct with anonymous tag, since we
// can't use a context tag at top level, and consumers should not care about the
// tag here).
constexpr uint8_t sNoFields[] = {
    CHIP_TLV_STRUCTURE(CHIP_TLV_TAG_ANONYMOUS),
    CHIP_TLV_END_OF_CONTAINER,
};
} // anonymous namespace

Status CommandHandler::ProcessCommandDataIB(CommandDataIB::Parser & aCommandElement)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CommandPathIB::Parser commandPath;
    ConcreteCommandPath concretePath(0, 0, 0);
    TLV::TLVReader commandDataReader;

    // NOTE: errors may occur before the concrete command path is even fully decoded.

    err = aCommandElement.GetPath(&commandPath);
    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::InvalidAction);

    err = commandPath.GetConcreteCommandPath(concretePath);
    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::InvalidAction);

    {
        Status commandExists = mpCallback->CommandExists(concretePath);
        if (commandExists != Status::Success)
        {
            ChipLogDetail(DataManagement, "No command " ChipLogFormatMEI " in Cluster " ChipLogFormatMEI " on Endpoint 0x%x",
                          ChipLogValueMEI(concretePath.mCommandId), ChipLogValueMEI(concretePath.mClusterId),
                          concretePath.mEndpointId);
            return FallibleAddStatus(concretePath, commandExists) != CHIP_NO_ERROR ? Status::Failure : Status::Success;
        }
    }

    VerifyOrExit(mResponseSender.HasSessionHandle(), err = CHIP_ERROR_INCORRECT_STATE);

    {
        Access::SubjectDescriptor subjectDescriptor = GetSubjectDescriptor();
        Access::RequestPath requestPath{ .cluster = concretePath.mClusterId, .endpoint = concretePath.mEndpointId };
        Access::Privilege requestPrivilege = RequiredPrivilege::ForInvokeCommand(concretePath);
        err                                = Access::GetAccessControl().Check(subjectDescriptor, requestPath, requestPrivilege);
        if (err != CHIP_NO_ERROR)
        {
            if (err != CHIP_ERROR_ACCESS_DENIED)
            {
                return FallibleAddStatus(concretePath, Status::Failure) != CHIP_NO_ERROR ? Status::Failure : Status::Success;
            }
            // TODO: when wildcard invokes are supported, handle them to discard rather than fail with status
            return FallibleAddStatus(concretePath, Status::UnsupportedAccess) != CHIP_NO_ERROR ? Status::Failure : Status::Success;
        }
    }

    if (CommandNeedsTimedInvoke(concretePath.mClusterId, concretePath.mCommandId) && !IsTimedInvoke())
    {
        // TODO: when wildcard invokes are supported, discard a
        // wildcard-expanded path instead of returning a status.
        return FallibleAddStatus(concretePath, Status::NeedsTimedInteraction) != CHIP_NO_ERROR ? Status::Failure : Status::Success;
    }

    if (CommandIsFabricScoped(concretePath.mClusterId, concretePath.mCommandId))
    {
        // SPEC: Else if the command in the path is fabric-scoped and there is no accessing fabric,
        // a CommandStatusIB SHALL be generated with the UNSUPPORTED_ACCESS Status Code.

        // Fabric-scoped commands are not allowed before a specific accessing fabric is available.
        // This is mostly just during a PASE session before AddNOC.
        if (GetAccessingFabricIndex() == kUndefinedFabricIndex)
        {
            // TODO: when wildcard invokes are supported, discard a
            // wildcard-expanded path instead of returning a status.
            return FallibleAddStatus(concretePath, Status::UnsupportedAccess) != CHIP_NO_ERROR ? Status::Failure : Status::Success;
        }
    }

    err = aCommandElement.GetFields(&commandDataReader);
    if (CHIP_END_OF_TLV == err)
    {
        ChipLogDetail(DataManagement,
                      "Received command without data for Endpoint=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                      concretePath.mEndpointId, ChipLogValueMEI(concretePath.mClusterId), ChipLogValueMEI(concretePath.mCommandId));
        commandDataReader.Init(sNoFields);
        err = commandDataReader.Next();
    }
    if (CHIP_NO_ERROR == err)
    {
        ChipLogDetail(DataManagement, "Received command for Endpoint=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                      concretePath.mEndpointId, ChipLogValueMEI(concretePath.mClusterId), ChipLogValueMEI(concretePath.mCommandId));
        SuccessOrExit(err = DataModelCallbacks::GetInstance()->PreCommandReceived(concretePath, GetSubjectDescriptor()));
        mpCallback->DispatchCommand(*this, concretePath, commandDataReader);
        DataModelCallbacks::GetInstance()->PostCommandReceived(concretePath, GetSubjectDescriptor());
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        return FallibleAddStatus(concretePath, Status::InvalidCommand) != CHIP_NO_ERROR ? Status::Failure : Status::Success;
    }

    // We have handled the error status above and put the error status in response, now return success status so we can process
    // other commands in the invoke request.
    return Status::Success;
}

Status CommandHandler::ProcessGroupCommandDataIB(CommandDataIB::Parser & aCommandElement)
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
    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::InvalidAction);

    err = commandPath.GetGroupCommandPath(&clusterId, &commandId);
    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::InvalidAction);

    groupId = mResponseSender.GetGroupId();
    fabric  = GetAccessingFabricIndex();

    ChipLogDetail(DataManagement, "Received group command for Group=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI,
                  groupId, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId));

    err = aCommandElement.GetFields(&commandDataReader);
    if (CHIP_END_OF_TLV == err)
    {
        ChipLogDetail(DataManagement,
                      "Received command without data for Group=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI, groupId,
                      ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId));
        commandDataReader.Init(sNoFields);
        err = commandDataReader.Next();
        VerifyOrReturnError(err == CHIP_NO_ERROR, Status::InvalidAction);
    }
    VerifyOrReturnError(err == CHIP_NO_ERROR, Status::Failure);

    // Per spec, we do the "is this a timed command?" check for every path, but
    // since all paths that fail it just get silently discarded we can do it
    // once up front and discard all the paths at once.  Ordering with respect
    // to ACL and command presence checks does not matter, because the behavior
    // is the same for all of them: ignore the path.
    if (CommandNeedsTimedInvoke(clusterId, commandId))
    {
        // Group commands are never timed.
        return Status::Success;
    }

    // No check for `CommandIsFabricScoped` unlike in `ProcessCommandDataIB()` since group commands
    // always have an accessing fabric, by definition.

    // Find which endpoints can process the command, and dispatch to them.
    iterator = groupDataProvider->IterateEndpoints(fabric);
    VerifyOrReturnError(iterator != nullptr, Status::Failure);

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

        if (mpCallback->CommandExists(concretePath) != Status::Success)
        {
            ChipLogDetail(DataManagement, "No command " ChipLogFormatMEI " in Cluster " ChipLogFormatMEI " on Endpoint 0x%x",
                          ChipLogValueMEI(commandId), ChipLogValueMEI(clusterId), mapping.endpoint_id);

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
        if ((err = DataModelCallbacks::GetInstance()->PreCommandReceived(concretePath, GetSubjectDescriptor())) == CHIP_NO_ERROR)
        {
            TLV::TLVReader dataReader(commandDataReader);
            mpCallback->DispatchCommand(*this, concretePath, dataReader);
            DataModelCallbacks::GetInstance()->PostCommandReceived(concretePath, GetSubjectDescriptor());
        }
        else
        {
            ChipLogError(DataManagement,
                         "Error when calling PreCommandReceived for Endpoint=%u Cluster=" ChipLogFormatMEI
                         " Command=" ChipLogFormatMEI " : %" CHIP_ERROR_FORMAT,
                         mapping.endpoint_id, ChipLogValueMEI(clusterId), ChipLogValueMEI(commandId), err.Format());
            continue;
        }
    }
    iterator->Release();
    return Status::Success;
}

CHIP_ERROR CommandHandler::TryAddStatusInternal(const ConcreteCommandPath & aCommandPath, const StatusIB & aStatus)
{
    ReturnErrorOnFailure(PrepareStatus(aCommandPath));
    CommandStatusIB::Builder & commandStatus = mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetStatus();
    StatusIB::Builder & statusIBBuilder      = commandStatus.CreateErrorStatus();
    ReturnErrorOnFailure(commandStatus.GetError());
    statusIBBuilder.EncodeStatusIB(aStatus);
    ReturnErrorOnFailure(statusIBBuilder.GetError());
    return FinishStatus();
}

CHIP_ERROR CommandHandler::AddStatusInternal(const ConcreteCommandPath & aCommandPath, const StatusIB & aStatus)
{
    return TryAddingResponse([&]() -> CHIP_ERROR { return TryAddStatusInternal(aCommandPath, aStatus); });
}

void CommandHandler::AddStatus(const ConcreteCommandPath & aCommandPath, const Protocols::InteractionModel::Status aStatus,
                               const char * context)
{
    // Return early in case of requests targeted to a group, since they should not add a response.
    VerifyOrReturn(!IsGroupRequest());

    CHIP_ERROR error = FallibleAddStatus(aCommandPath, aStatus, context);

    if (error != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Failed to add command status: %" CHIP_ERROR_FORMAT, error.Format());

        // Do not crash if the status has not been added due to running out of packet buffers or other resources.
        // It is better to drop a single response than to go offline and lose all sessions and subscriptions.
        VerifyOrDie(error == CHIP_ERROR_NO_MEMORY);
    }
}

CHIP_ERROR CommandHandler::FallibleAddStatus(const ConcreteCommandPath & path, const Protocols::InteractionModel::Status status,
                                             const char * context)
{

    if (status != Status::Success)
    {
        if (context == nullptr)
        {
            context = "no additional context";
        }

        ChipLogError(DataManagement,
                     "Endpoint=%u Cluster=" ChipLogFormatMEI " Command=" ChipLogFormatMEI " status " ChipLogFormatIMStatus " (%s)",
                     path.mEndpointId, ChipLogValueMEI(path.mClusterId), ChipLogValueMEI(path.mCommandId),
                     ChipLogValueIMStatus(status), context);
    }

    return AddStatusInternal(path, StatusIB(status));
}

CHIP_ERROR CommandHandler::AddClusterSpecificSuccess(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus)
{
    return AddStatusInternal(aCommandPath, StatusIB(Status::Success, aClusterStatus));
}

CHIP_ERROR CommandHandler::AddClusterSpecificFailure(const ConcreteCommandPath & aCommandPath, ClusterStatus aClusterStatus)
{
    return AddStatusInternal(aCommandPath, StatusIB(Status::Failure, aClusterStatus));
}

CHIP_ERROR CommandHandler::PrepareInvokeResponseCommand(const ConcreteCommandPath & aResponseCommandPath,
                                                        const CommandHandler::InvokeResponseParameters & aPrepareParameters)
{
    auto commandPathRegistryEntry = GetCommandPathRegistry().Find(aPrepareParameters.mRequestCommandPath);
    VerifyOrReturnValue(commandPathRegistryEntry.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    return PrepareInvokeResponseCommand(commandPathRegistryEntry.Value(), aResponseCommandPath,
                                        aPrepareParameters.mStartOrEndDataStruct);
}

CHIP_ERROR CommandHandler::PrepareCommand(const ConcreteCommandPath & aResponseCommandPath, bool aStartDataStruct)
{
    // Legacy code is calling the deprecated version of PrepareCommand. If we are in a case where
    // there was a single command in the request, we can just assume this response is triggered by
    // the single command.
    size_t countOfPathRegistryEntries = GetCommandPathRegistry().Count();

    // At this point application supports Batch Invoke Commands since CommandPathRegistry has more than 1 entry,
    // but application is calling the deprecated PrepareCommand. We have no way to determine the associated CommandRef
    // to put into the InvokeResponse.
    VerifyOrDieWithMsg(countOfPathRegistryEntries == 1, DataManagement,
                       "Seemingly device supports batch commands, but is calling the deprecated PrepareCommand API");

    auto commandPathRegistryEntry = GetCommandPathRegistry().GetFirstEntry();
    VerifyOrReturnValue(commandPathRegistryEntry.HasValue(), CHIP_ERROR_INCORRECT_STATE);

    return PrepareInvokeResponseCommand(commandPathRegistryEntry.Value(), aResponseCommandPath, aStartDataStruct);
}

CHIP_ERROR CommandHandler::PrepareInvokeResponseCommand(const CommandPathRegistryEntry & apCommandPathRegistryEntry,
                                                        const ConcreteCommandPath & aCommandPath, bool aStartDataStruct)
{
    ReturnErrorOnFailure(AllocateBuffer());

    if (!mInternalCallToAddResponseData && mState == State::AddedCommand)
    {
        // An attempt is being made to add CommandData InvokeResponse using primitive
        // CommandHandler APIs. While not recommended, as this potentially leaves the
        // CommandHandler in an incorrect state upon failure, this approach is permitted
        // for legacy reasons. To maximize the likelihood of success, particularly when
        // handling large amounts of data, we try to obtain a new, completely empty
        // InvokeResponseMessage, as the existing one already has space occupied.
        ReturnErrorOnFailure(FinalizeInvokeResponseMessageAndPrepareNext());
    }

    CreateBackupForResponseRollback();
    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrReturnError(mState == State::NewResponseMessage || mState == State::AddedCommand, CHIP_ERROR_INCORRECT_STATE);

    // TODO(#30453): See if we can pass this back up the stack so caller can provide this instead of taking up
    // space in CommandHanlder.
    mRefForResponse = apCommandPathRegistryEntry.ref;

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
        ReturnErrorOnFailure(commandData.GetWriter()->StartContainer(TLV::ContextTag(CommandDataIB::Tag::kFields),
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

    if (mRefForResponse.HasValue())
    {
        ReturnErrorOnFailure(commandData.Ref(mRefForResponse.Value()));
    }

    ReturnErrorOnFailure(commandData.EndOfCommandDataIB());
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().EndOfInvokeResponseIB());
    MoveToState(State::AddedCommand);
    return CHIP_NO_ERROR;
}

CHIP_ERROR CommandHandler::PrepareStatus(const ConcreteCommandPath & aCommandPath)
{
    ReturnErrorOnFailure(AllocateBuffer());
    //
    // We must not be in the middle of preparing a command, or having prepared or sent one.
    //
    VerifyOrReturnError(mState == State::NewResponseMessage || mState == State::AddedCommand, CHIP_ERROR_INCORRECT_STATE);
    if (mState == State::AddedCommand)
    {
        CreateBackupForResponseRollback();
    }

    auto commandPathRegistryEntry = GetCommandPathRegistry().Find(aCommandPath);
    VerifyOrReturnError(commandPathRegistryEntry.HasValue(), CHIP_ERROR_INCORRECT_STATE);
    mRefForResponse = commandPathRegistryEntry.Value().ref;

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

    CommandStatusIB::Builder & commandStatus = mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetStatus();
    if (mRefForResponse.HasValue())
    {
        ReturnErrorOnFailure(commandStatus.Ref(mRefForResponse.Value()));
    }

    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().GetStatus().EndOfCommandStatusIB());
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().GetInvokeResponse().EndOfInvokeResponseIB());
    MoveToState(State::AddedCommand);
    return CHIP_NO_ERROR;
}

void CommandHandler::CreateBackupForResponseRollback()
{
    VerifyOrReturn(mState == State::NewResponseMessage || mState == State::AddedCommand);
    VerifyOrReturn(mInvokeResponseBuilder.GetInvokeResponses().GetError() == CHIP_NO_ERROR);
    VerifyOrReturn(mInvokeResponseBuilder.GetError() == CHIP_NO_ERROR);
    mInvokeResponseBuilder.Checkpoint(mBackupWriter);
    mBackupState         = mState;
    mRollbackBackupValid = true;
}

CHIP_ERROR CommandHandler::RollbackResponse()
{
    VerifyOrReturnError(mRollbackBackupValid, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(mState == State::Preparing || mState == State::AddingCommand, CHIP_ERROR_INCORRECT_STATE);
    // TODO(#30453): Rollback of mInvokeResponseBuilder should handle resetting
    // InvokeResponses.
    mInvokeResponseBuilder.GetInvokeResponses().ResetError();
    mInvokeResponseBuilder.Rollback(mBackupWriter);
    MoveToState(mBackupState);
    mRollbackBackupValid = false;
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
    VerifyOrDie(!mGoneAsync);
    return mResponseSender.GetAccessingFabricIndex();
}

CommandHandler * CommandHandler::Handle::Get()
{
    // Not safe to work with CommandHandler in parallel with other Matter work.
    assertChipStackLockedByCurrentThread();

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

CHIP_ERROR CommandHandler::FinalizeInvokeResponseMessageAndPrepareNext()
{
    ReturnErrorOnFailure(FinalizeInvokeResponseMessage(/* aHasMoreChunks = */ true));
    // After successfully finalizing InvokeResponseMessage, no buffer should remain
    // allocated.
    VerifyOrDie(!mBufferAllocated);
    CHIP_ERROR err = AllocateBuffer();
    if (err != CHIP_NO_ERROR)
    {
        // TODO(#30453): Improve ResponseDropped calls to occur only when dropping is
        // definitively guaranteed.
        // Response dropping is not yet definitive as a subsequent call
        // to AllocateBuffer might succeed.
        mResponseSender.ResponseDropped();
    }
    return err;
}

CHIP_ERROR CommandHandler::FinalizeInvokeResponseMessage(bool aHasMoreChunks)
{
    System::PacketBufferHandle packet;

    VerifyOrReturnError(mState == State::AddedCommand, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mInvokeResponseBuilder.GetInvokeResponses().EndOfInvokeResponses());
    if (aHasMoreChunks)
    {
        // Unreserving space previously reserved for MoreChunkedMessages is done
        // in the call to mInvokeResponseBuilder.MoreChunkedMessages.
        mInvokeResponseBuilder.MoreChunkedMessages(aHasMoreChunks);
        ReturnErrorOnFailure(mInvokeResponseBuilder.GetError());
    }
    ReturnErrorOnFailure(mInvokeResponseBuilder.EndOfInvokeResponseMessage());
    ReturnErrorOnFailure(mCommandMessageWriter.Finalize(&packet));
    mResponseSender.AddInvokeResponseToSend(std::move(packet));
    mBufferAllocated     = false;
    mRollbackBackupValid = false;
    return CHIP_NO_ERROR;
}

const char * CommandHandler::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case State::Idle:
        return "Idle";

    case State::NewResponseMessage:
        return "NewResponseMessage";

    case State::Preparing:
        return "Preparing";

    case State::AddingCommand:
        return "AddingCommand";

    case State::AddedCommand:
        return "AddedCommand";

    case State::DispatchResponses:
        return "DispatchResponses";

    case State::AwaitingDestruction:
        return "AwaitingDestruction";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void CommandHandler::MoveToState(const State aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "Command handler moving to [%10.10s]", GetStateStr());
}

#if CHIP_WITH_NLFAULTINJECTION

namespace {

CHIP_ERROR TestOnlyExtractCommandPathFromNextInvokeRequest(TLV::TLVReader & invokeRequestsReader,
                                                           ConcreteCommandPath & concretePath)
{
    ReturnErrorOnFailure(invokeRequestsReader.Next(TLV::AnonymousTag()));
    CommandDataIB::Parser commandData;
    ReturnErrorOnFailure(commandData.Init(invokeRequestsReader));
    CommandPathIB::Parser commandPath;
    ReturnErrorOnFailure(commandData.GetPath(&commandPath));
    return commandPath.GetConcreteCommandPath(concretePath);
}

[[maybe_unused]] const char * GetFaultInjectionTypeStr(CommandHandler::NlFaultInjectionType faultType)
{
    switch (faultType)
    {
    case CommandHandler::NlFaultInjectionType::SeparateResponseMessages:
        return "Each response will be sent in a separate InvokeResponseMessage. The order of responses will be the same as the "
               "original request.";
    case CommandHandler::NlFaultInjectionType::SeparateResponseMessagesAndInvertedResponseOrder:
        return "Each response will be sent in a separate InvokeResponseMessage. The order of responses will be reversed from the "
               "original request.";
    case CommandHandler::NlFaultInjectionType::SkipSecondResponse:
        return "Single InvokeResponseMessages. Dropping response to second request";
    }
    VerifyOrDieWithMsg(false, DataManagement, "TH Failure: Unexpected fault type");
}

} // anonymous namespace

// This method intentionally duplicates code from other sections. While code consolidation
// is generally preferred, here we prioritize generating a clear crash message to aid in
// troubleshooting test failures.
void CommandHandler::TestOnlyInvokeCommandRequestWithFaultsInjected(Messaging::ExchangeContext * ec,
                                                                    System::PacketBufferHandle && payload, bool isTimedInvoke,
                                                                    NlFaultInjectionType faultType)
{
    VerifyOrDieWithMsg(ec != nullptr, DataManagement, "TH Failure: Incoming exchange context should not be null");
    VerifyOrDieWithMsg(mState == State::Idle, DataManagement, "TH Failure: state should be Idle, issue with TH");

    ChipLogProgress(DataManagement, "Response to InvokeRequestMessage overridden by fault injection");
    ChipLogProgress(DataManagement, "   Injecting the following response:%s", GetFaultInjectionTypeStr(faultType));

    mResponseSender.SetExchangeContext(ec);
    Handle workHandle(this);
    mResponseSender.WillSendMessage();
    VerifyOrDieWithMsg(!mResponseSender.IsForGroup(), DataManagement, "DUT Failure: Unexpected Group Command");

    System::PacketBufferTLVReader reader;
    InvokeRequestMessage::Parser invokeRequestMessage;
    InvokeRequests::Parser invokeRequests;
    reader.Init(std::move(payload));
    VerifyOrDieWithMsg(invokeRequestMessage.Init(reader) == CHIP_NO_ERROR, DataManagement,
                       "TH Failure: Failed 'invokeRequestMessage.Init(reader)'");
#if CHIP_CONFIG_IM_PRETTY_PRINT
    invokeRequestMessage.PrettyPrint();
#endif

    VerifyOrDieWithMsg(invokeRequestMessage.GetSuppressResponse(&mSuppressResponse) == CHIP_NO_ERROR, DataManagement,
                       "DUT Failure: Mandatory SuppressResponse field missing");
    VerifyOrDieWithMsg(invokeRequestMessage.GetTimedRequest(&mTimedRequest) == CHIP_NO_ERROR, DataManagement,
                       "DUT Failure: Mandatory TimedRequest field missing");
    VerifyOrDieWithMsg(invokeRequestMessage.GetInvokeRequests(&invokeRequests) == CHIP_NO_ERROR, DataManagement,
                       "DUT Failure: Mandatory InvokeRequests field missing");
    VerifyOrDieWithMsg(mTimedRequest == isTimedInvoke, DataManagement,
                       "DUT Failure: TimedRequest value in message mismatches action");

    {
        InvokeRequestMessage::Parser validationInvokeRequestMessage = invokeRequestMessage;
        VerifyOrDieWithMsg(ValidateInvokeRequestMessageAndBuildRegistry(validationInvokeRequestMessage) == CHIP_NO_ERROR,
                           DataManagement, "DUT Failure: InvokeRequestMessage contents were invalid");
    }

    TLV::TLVReader invokeRequestsReader;
    invokeRequests.GetReader(&invokeRequestsReader);

    size_t commandCount = 0;
    VerifyOrDieWithMsg(TLV::Utilities::Count(invokeRequestsReader, commandCount, false /* recurse */) == CHIP_NO_ERROR,
                       DataManagement,
                       "TH Failure: Failed to get the length of InvokeRequests after InvokeRequestMessage validation");

    // The command count check (specifically for a count of 2) is tied to IDM_1_3. This may need adjustment for
    // compatibility with future test plans.
    VerifyOrDieWithMsg(commandCount == 2, DataManagement, "DUT failure: We were strictly expecting exactly 2 InvokeRequests");
    mReserveSpaceForMoreChunkMessages = true;

    {
        // Response path is the same as request path since we are replying with a failure message.
        ConcreteCommandPath concreteResponsePath1;
        ConcreteCommandPath concreteResponsePath2;
        VerifyOrDieWithMsg(
            TestOnlyExtractCommandPathFromNextInvokeRequest(invokeRequestsReader, concreteResponsePath1) == CHIP_NO_ERROR,
            DataManagement, "DUT Failure: Issues encountered while extracting the ConcreteCommandPath from the first request");
        VerifyOrDieWithMsg(
            TestOnlyExtractCommandPathFromNextInvokeRequest(invokeRequestsReader, concreteResponsePath2) == CHIP_NO_ERROR,
            DataManagement, "DUT Failure: Issues encountered while extracting the ConcreteCommandPath from the second request");

        if (faultType == NlFaultInjectionType::SeparateResponseMessagesAndInvertedResponseOrder)
        {
            ConcreteCommandPath temp(concreteResponsePath1);
            concreteResponsePath1 = concreteResponsePath2;
            concreteResponsePath2 = temp;
        }

        VerifyOrDieWithMsg(FallibleAddStatus(concreteResponsePath1, Status::Failure) == CHIP_NO_ERROR, DataManagement,
                           "TH Failure: Error adding the first InvokeResponse");
        if (faultType == NlFaultInjectionType::SeparateResponseMessages ||
            faultType == NlFaultInjectionType::SeparateResponseMessagesAndInvertedResponseOrder)
        {
            VerifyOrDieWithMsg(FinalizeInvokeResponseMessageAndPrepareNext() == CHIP_NO_ERROR, DataManagement,
                               "TH Failure: Failed to create second InvokeResponseMessage");
        }
        if (faultType != NlFaultInjectionType::SkipSecondResponse)
        {
            VerifyOrDieWithMsg(FallibleAddStatus(concreteResponsePath2, Status::Failure) == CHIP_NO_ERROR, DataManagement,
                               "TH Failure: Error adding the second InvokeResponse");
        }
    }

    VerifyOrDieWithMsg(invokeRequestsReader.Next() == CHIP_END_OF_TLV, DataManagement,
                       "DUT Failure: Unexpected TLV ending of InvokeRequests");
    VerifyOrDieWithMsg(invokeRequestMessage.ExitContainer() == CHIP_NO_ERROR, DataManagement,
                       "DUT Failure: InvokeRequestMessage TLV is not properly terminated");
}
#endif // CHIP_WITH_NLFAULTINJECTION

} // namespace app
} // namespace chip
