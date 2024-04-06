/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include "messaging/ExchangeContext.h"
#include <app/AppConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/EventPathIB.h>
#include <app/StatusResponse.h>
#include <app/WriteHandler.h>
#include <app/reporting/Engine.h>
#include <app/util/MatterCallbacks.h>
#include <app/util/ember-compatibility-functions.h>
#include <credentials/GroupDataProvider.h>
#include <lib/support/TypeTraits.h>

namespace chip {
namespace app {

using namespace Protocols::InteractionModel;
using Status                         = Protocols::InteractionModel::Status;
constexpr uint8_t kListAttributeType = 0x48;

CHIP_ERROR WriteHandler::Init()
{
    VerifyOrReturnError(!mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);

    MoveToState(State::Initialized);

    mACLCheckCache.ClearValue();
    mProcessingAttributePath.ClearValue();

    return CHIP_NO_ERROR;
}

void WriteHandler::Close()
{
    VerifyOrReturn(mState != State::Uninitialized);

    // DeliverFinalListWriteEnd will be a no-op if we have called
    // DeliverFinalListWriteEnd in success conditions, so passing false for
    // wasSuccessful here is safe: if it does anything, we were in fact not
    // successful.
    DeliverFinalListWriteEnd(false /* wasSuccessful */);
    mExchangeCtx.Release();
    mSuppressResponse = false;
    MoveToState(State::Uninitialized);
}

Status WriteHandler::HandleWriteRequestMessage(Messaging::ExchangeContext * apExchangeContext,
                                               System::PacketBufferHandle && aPayload, bool aIsTimedWrite)
{
    System::PacketBufferHandle packet = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    VerifyOrReturnError(!packet.IsNull(), Status::Failure);

    System::PacketBufferTLVWriter messageWriter;
    messageWriter.Init(std::move(packet));
    VerifyOrReturnError(mWriteResponseBuilder.Init(&messageWriter) == CHIP_NO_ERROR, Status::Failure);

    mWriteResponseBuilder.CreateWriteResponses();
    VerifyOrReturnError(mWriteResponseBuilder.GetError() == CHIP_NO_ERROR, Status::Failure);

    Status status = ProcessWriteRequest(std::move(aPayload), aIsTimedWrite);

    // Do not send response on Group Write
    if (status == Status::Success && !apExchangeContext->IsGroupExchangeContext())
    {
        CHIP_ERROR err = SendWriteResponse(std::move(messageWriter));
        if (err != CHIP_NO_ERROR)
        {
            status = Status::Failure;
        }
    }

    return status;
}

Status WriteHandler::OnWriteRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload,
                                    bool aIsTimedWrite)
{
    //
    // Let's take over further message processing on this exchange from the IM.
    // This is only relevant during chunked requests.
    //
    mExchangeCtx.Grab(apExchangeContext);

    Status status = HandleWriteRequestMessage(apExchangeContext, std::move(aPayload), aIsTimedWrite);

    // The write transaction will be alive only when the message was handled successfully and there are more chunks.
    if (!(status == Status::Success && mHasMoreChunks))
    {
        Close();
    }

    return status;
}

CHIP_ERROR WriteHandler::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                           System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrDieWithMsg(apExchangeContext == mExchangeCtx.Get(), DataManagement,
                       "Incoming exchange context should be same as the initial request.");
    VerifyOrDieWithMsg(!apExchangeContext->IsGroupExchangeContext(), DataManagement,
                       "OnMessageReceived should not be called on GroupExchangeContext");
    if (!aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::WriteRequest))
    {
        if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::StatusResponse))
        {
            CHIP_ERROR statusError = CHIP_NO_ERROR;
            // Parse the status response so we can log it properly.
            StatusResponse::ProcessStatusResponse(std::move(aPayload), statusError);
        }
        ChipLogDetail(DataManagement, "Unexpected message type %d", aPayloadHeader.GetMessageType());
        StatusResponse::Send(Status::InvalidAction, apExchangeContext, false /*aExpectResponse*/);
        Close();
        return CHIP_ERROR_INVALID_MESSAGE_TYPE;
    }

    Status status =
        HandleWriteRequestMessage(apExchangeContext, std::move(aPayload), false /* chunked write should not be timed write */);
    if (status == Status::Success)
    {
        // We have no more chunks, the write response has been sent in HandleWriteRequestMessage, so close directly.
        if (!mHasMoreChunks)
        {
            Close();
        }
    }
    else
    {
        err = StatusResponse::Send(status, apExchangeContext, false /*aExpectResponse*/);
        Close();
    }
    return CHIP_NO_ERROR;
}

void WriteHandler::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    ChipLogError(DataManagement, "Time out! failed to receive status response from Exchange: " ChipLogFormatExchange,
                 ChipLogValueExchange(apExchangeContext));
    Close();
}

CHIP_ERROR WriteHandler::FinalizeMessage(System::PacketBufferTLVWriter && aMessageWriter, System::PacketBufferHandle & packet)
{
    VerifyOrReturnError(mState == State::AddStatus, CHIP_ERROR_INCORRECT_STATE);
    ReturnErrorOnFailure(mWriteResponseBuilder.GetWriteResponses().EndOfAttributeStatuses());
    ReturnErrorOnFailure(mWriteResponseBuilder.EndOfWriteResponseMessage());
    ReturnErrorOnFailure(aMessageWriter.Finalize(&packet));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WriteHandler::SendWriteResponse(System::PacketBufferTLVWriter && aMessageWriter)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle packet;

    VerifyOrExit(mState == State::AddStatus, err = CHIP_ERROR_INCORRECT_STATE);

    err = FinalizeMessage(std::move(aMessageWriter), packet);
    SuccessOrExit(err);

    VerifyOrExit(mExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
    mExchangeCtx->UseSuggestedResponseTimeout(app::kExpectedIMProcessingTime);
    err = mExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::WriteResponse, std::move(packet),
                                    mHasMoreChunks ? Messaging::SendMessageFlags::kExpectResponse
                                                   : Messaging::SendMessageFlags::kNone);
    SuccessOrExit(err);

    MoveToState(State::Sending);

exit:
    return err;
}

void WriteHandler::DeliverListWriteBegin(const ConcreteAttributePath & aPath)
{
    if (auto * attrOverride = GetAttributeAccessOverride(aPath.mEndpointId, aPath.mClusterId))
    {
        attrOverride->OnListWriteBegin(aPath);
    }
}

void WriteHandler::DeliverListWriteEnd(const ConcreteAttributePath & aPath, bool writeWasSuccessful)
{
    if (auto * attrOverride = GetAttributeAccessOverride(aPath.mEndpointId, aPath.mClusterId))
    {
        attrOverride->OnListWriteEnd(aPath, writeWasSuccessful);
    }
}

void WriteHandler::DeliverFinalListWriteEnd(bool writeWasSuccessful)
{
    if (mProcessingAttributePath.HasValue() && mProcessingAttributeIsList)
    {
        DeliverListWriteEnd(mProcessingAttributePath.Value(), writeWasSuccessful);
    }
    mProcessingAttributePath.ClearValue();
}

CHIP_ERROR WriteHandler::DeliverFinalListWriteEndForGroupWrite(bool writeWasSuccessful)
{
    VerifyOrReturnError(mProcessingAttributePath.HasValue() && mProcessingAttributeIsList, CHIP_NO_ERROR);

    Credentials::GroupDataProvider::GroupEndpoint mapping;
    Credentials::GroupDataProvider * groupDataProvider = Credentials::GetGroupDataProvider();
    Credentials::GroupDataProvider::EndpointIterator * iterator;

    GroupId groupId         = mExchangeCtx->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
    FabricIndex fabricIndex = GetAccessingFabricIndex();

    auto processingConcreteAttributePath = mProcessingAttributePath.Value();
    mProcessingAttributePath.ClearValue();

    iterator = groupDataProvider->IterateEndpoints(fabricIndex);
    VerifyOrReturnError(iterator != nullptr, CHIP_ERROR_NO_MEMORY);

    while (iterator->Next(mapping))
    {
        if (groupId != mapping.group_id)
        {
            continue;
        }

        processingConcreteAttributePath.mEndpointId = mapping.endpoint_id;

        if (!InteractionModelEngine::GetInstance()->HasConflictWriteRequests(this, processingConcreteAttributePath))
        {
            DeliverListWriteEnd(processingConcreteAttributePath, writeWasSuccessful);
        }
    }
    iterator->Release();
    return CHIP_NO_ERROR;
}
namespace {

// To reduce the various use of previousProcessed.HasValue() && previousProcessed.Value() == nextAttribute to save code size.
bool IsSameAttribute(const Optional<ConcreteAttributePath> & previousProcessed, const ConcreteDataAttributePath & nextAttribute)
{
    return previousProcessed.HasValue() && previousProcessed.Value() == nextAttribute;
}

bool ShouldReportListWriteEnd(const Optional<ConcreteAttributePath> & previousProcessed, bool previousProcessedAttributeIsList,
                              const ConcreteDataAttributePath & nextAttribute)
{
    return previousProcessedAttributeIsList && !IsSameAttribute(previousProcessed, nextAttribute) && previousProcessed.HasValue();
}

bool ShouldReportListWriteBegin(const Optional<ConcreteAttributePath> & previousProcessed, bool previousProcessedAttributeIsList,
                                const ConcreteDataAttributePath & nextAttribute)
{
    return !IsSameAttribute(previousProcessed, nextAttribute) && nextAttribute.IsListOperation();
}

} // namespace

CHIP_ERROR WriteHandler::ProcessAttributeDataIBs(TLV::TLVReader & aAttributeDataIBsReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorCodeIf(!mExchangeCtx, CHIP_ERROR_INTERNAL);
    const Access::SubjectDescriptor subjectDescriptor = mExchangeCtx->GetSessionHandle()->GetSubjectDescriptor();

    while (CHIP_NO_ERROR == (err = aAttributeDataIBsReader.Next()))
    {
        chip::TLV::TLVReader dataReader;
        AttributeDataIB::Parser element;
        AttributePathIB::Parser attributePath;
        ConcreteDataAttributePath dataAttributePath;
        TLV::TLVReader reader = aAttributeDataIBsReader;

        err = element.Init(reader);
        SuccessOrExit(err);

        err = element.GetPath(&attributePath);
        SuccessOrExit(err);

        err = attributePath.GetConcreteAttributePath(dataAttributePath);
        SuccessOrExit(err);

        err = element.GetData(&dataReader);
        SuccessOrExit(err);

        const auto attributeMetadata = GetAttributeMetadata(dataAttributePath);
        bool currentAttributeIsList  = (attributeMetadata != nullptr && attributeMetadata->attributeType == kListAttributeType);

        if (!dataAttributePath.IsListOperation() && currentAttributeIsList)
        {
            dataAttributePath.mListOp = ConcreteDataAttributePath::ListOperation::ReplaceAll;
        }

        if (InteractionModelEngine::GetInstance()->HasConflictWriteRequests(this, dataAttributePath) ||
            // Per chunking protocol, we are processing the list entries, but the initial empty list is not processed, so we reject
            // it with Busy status code.
            (dataAttributePath.IsListItemOperation() && !IsSameAttribute(mProcessingAttributePath, dataAttributePath)))
        {
            err = AddStatus(dataAttributePath, StatusIB(Status::Busy));
            continue;
        }

        if (ShouldReportListWriteEnd(mProcessingAttributePath, mProcessingAttributeIsList, dataAttributePath))
        {
            DeliverListWriteEnd(mProcessingAttributePath.Value(), mAttributeWriteSuccessful);
        }

        if (ShouldReportListWriteBegin(mProcessingAttributePath, mProcessingAttributeIsList, dataAttributePath))
        {
            DeliverListWriteBegin(dataAttributePath);
            mAttributeWriteSuccessful = true;
        }

        mProcessingAttributeIsList = dataAttributePath.IsListOperation();
        mProcessingAttributePath.SetValue(dataAttributePath);

        DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Write,
                                                              DataModelCallbacks::OperationOrder::Pre, dataAttributePath);

        TLV::TLVWriter backup;
        DataVersion version = 0;
        mWriteResponseBuilder.GetWriteResponses().Checkpoint(backup);
        err = element.GetDataVersion(&version);
        if (CHIP_NO_ERROR == err)
        {
            dataAttributePath.mDataVersion.SetValue(version);
        }
        else if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }
        SuccessOrExit(err);
        err = WriteSingleClusterData(subjectDescriptor, dataAttributePath, dataReader, this);
        if (err != CHIP_NO_ERROR)
        {
            mWriteResponseBuilder.GetWriteResponses().Rollback(backup);
            err = AddStatus(dataAttributePath, StatusIB(err));
        }

        DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Write,
                                                              DataModelCallbacks::OperationOrder::Post, dataAttributePath);
        SuccessOrExit(err);
    }

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    SuccessOrExit(err);

    if (!mHasMoreChunks)
    {
        DeliverFinalListWriteEnd(mAttributeWriteSuccessful);
    }

exit:
    return err;
}

CHIP_ERROR WriteHandler::ProcessGroupAttributeDataIBs(TLV::TLVReader & aAttributeDataIBsReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorCodeIf(!mExchangeCtx, CHIP_ERROR_INTERNAL);
    const Access::SubjectDescriptor subjectDescriptor =
        mExchangeCtx->GetSessionHandle()->AsIncomingGroupSession()->GetSubjectDescriptor();

    GroupId groupId    = mExchangeCtx->GetSessionHandle()->AsIncomingGroupSession()->GetGroupId();
    FabricIndex fabric = GetAccessingFabricIndex();

    while (CHIP_NO_ERROR == (err = aAttributeDataIBsReader.Next()))
    {
        chip::TLV::TLVReader dataReader;
        AttributeDataIB::Parser element;
        AttributePathIB::Parser attributePath;
        ConcreteDataAttributePath dataAttributePath;
        TLV::TLVReader reader = aAttributeDataIBsReader;

        Credentials::GroupDataProvider::GroupEndpoint mapping;
        Credentials::GroupDataProvider * groupDataProvider = Credentials::GetGroupDataProvider();
        Credentials::GroupDataProvider::EndpointIterator * iterator;

        err = element.Init(reader);
        SuccessOrExit(err);

        err = element.GetPath(&attributePath);
        SuccessOrExit(err);

        err = attributePath.GetGroupAttributePath(dataAttributePath);
        SuccessOrExit(err);

        err = element.GetData(&dataReader);
        SuccessOrExit(err);

        if (!dataAttributePath.IsListOperation() && dataReader.GetType() == TLV::TLVType::kTLVType_Array)
        {
            dataAttributePath.mListOp = ConcreteDataAttributePath::ListOperation::ReplaceAll;
        }

        ChipLogDetail(DataManagement,
                      "Received group attribute write for Group=%u Cluster=" ChipLogFormatMEI " attribute=" ChipLogFormatMEI,
                      groupId, ChipLogValueMEI(dataAttributePath.mClusterId), ChipLogValueMEI(dataAttributePath.mAttributeId));

        iterator = groupDataProvider->IterateEndpoints(fabric);
        VerifyOrExit(iterator != nullptr, err = CHIP_ERROR_NO_MEMORY);

        bool shouldReportListWriteEnd =
            ShouldReportListWriteEnd(mProcessingAttributePath, mProcessingAttributeIsList, dataAttributePath);
        bool shouldReportListWriteBegin = false; // This will be set below.

        const EmberAfAttributeMetadata * attributeMetadata = nullptr;

        while (iterator->Next(mapping))
        {
            if (groupId != mapping.group_id)
            {
                continue;
            }

            dataAttributePath.mEndpointId = mapping.endpoint_id;

            // Try to get the metadata from for the attribute from one of the expanded endpoints (it doesn't really matter which
            // endpoint we pick, as long as it's valid) and update the path info according to it and recheck if we need to report
            // list write begin.
            if (attributeMetadata == nullptr)
            {
                attributeMetadata = GetAttributeMetadata(dataAttributePath);
                bool currentAttributeIsList =
                    (attributeMetadata != nullptr && attributeMetadata->attributeType == kListAttributeType);
                if (!dataAttributePath.IsListOperation() && currentAttributeIsList)
                {
                    dataAttributePath.mListOp = ConcreteDataAttributePath::ListOperation::ReplaceAll;
                }
                ConcreteDataAttributePath pathForCheckingListWriteBegin(kInvalidEndpointId, dataAttributePath.mClusterId,
                                                                        dataAttributePath.mEndpointId, dataAttributePath.mListOp,
                                                                        dataAttributePath.mListIndex);
                shouldReportListWriteBegin =
                    ShouldReportListWriteBegin(mProcessingAttributePath, mProcessingAttributeIsList, pathForCheckingListWriteBegin);
            }

            if (shouldReportListWriteEnd)
            {
                auto processingConcreteAttributePath        = mProcessingAttributePath.Value();
                processingConcreteAttributePath.mEndpointId = mapping.endpoint_id;
                if (!InteractionModelEngine::GetInstance()->HasConflictWriteRequests(this, processingConcreteAttributePath))
                {
                    DeliverListWriteEnd(processingConcreteAttributePath, true /* writeWasSuccessful */);
                }
            }

            if (InteractionModelEngine::GetInstance()->HasConflictWriteRequests(this, dataAttributePath))
            {
                ChipLogDetail(DataManagement,
                              "Writing attribute endpoint=%u Cluster=" ChipLogFormatMEI " attribute=" ChipLogFormatMEI
                              " is conflict with other write transactions.",
                              mapping.endpoint_id, ChipLogValueMEI(dataAttributePath.mClusterId),
                              ChipLogValueMEI(dataAttributePath.mAttributeId));
                continue;
            }

            if (shouldReportListWriteBegin)
            {
                DeliverListWriteBegin(dataAttributePath);
            }

            ChipLogDetail(DataManagement,
                          "Processing group attribute write for endpoint=%u Cluster=" ChipLogFormatMEI
                          " attribute=" ChipLogFormatMEI,
                          mapping.endpoint_id, ChipLogValueMEI(dataAttributePath.mClusterId),
                          ChipLogValueMEI(dataAttributePath.mAttributeId));

            chip::TLV::TLVReader tmpDataReader(dataReader);

            DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Write,
                                                                  DataModelCallbacks::OperationOrder::Pre, dataAttributePath);
            err = WriteSingleClusterData(subjectDescriptor, dataAttributePath, tmpDataReader, this);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DataManagement,
                             "WriteSingleClusterData Endpoint=%u Cluster=" ChipLogFormatMEI " Attribute =" ChipLogFormatMEI
                             " failed: %" CHIP_ERROR_FORMAT,
                             mapping.endpoint_id, ChipLogValueMEI(dataAttributePath.mClusterId),
                             ChipLogValueMEI(dataAttributePath.mAttributeId), err.Format());
            }
            DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Write,
                                                                  DataModelCallbacks::OperationOrder::Post, dataAttributePath);
        }

        dataAttributePath.mEndpointId = kInvalidEndpointId;
        mProcessingAttributeIsList    = dataAttributePath.IsListOperation();
        mProcessingAttributePath.SetValue(dataAttributePath);
        iterator->Release();
    }

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

    err = DeliverFinalListWriteEndForGroupWrite(true);

exit:
    // The DeliverFinalListWriteEndForGroupWrite above will deliver the successful state of the list write and clear the
    // mProcessingAttributePath making the following call no-op. So we call it again after the exit label to deliver a failure state
    // to the clusters. Ignore the error code since we need to deliver other more important failures.
    DeliverFinalListWriteEndForGroupWrite(false);
    return err;
}

Status WriteHandler::ProcessWriteRequest(System::PacketBufferHandle && aPayload, bool aIsTimedWrite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;

    WriteRequestMessage::Parser writeRequestParser;
    AttributeDataIBs::Parser AttributeDataIBsParser;
    TLV::TLVReader AttributeDataIBsReader;
    // Default to InvalidAction for our status; that's what we want if any of
    // the parsing of our overall structure or paths fails.  Once we have a
    // successfully parsed path, the only way we will get a failure return is if
    // our path handling fails to AddStatus on us.
    //
    // TODO: That's not technically InvalidAction, and we should probably make
    // our callees hand out Status as well.
    Status status = Status::InvalidAction;

    reader.Init(std::move(aPayload));

    err = writeRequestParser.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    writeRequestParser.PrettyPrint();
#endif
    err = writeRequestParser.GetSuppressResponse(&mSuppressResponse);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    err = writeRequestParser.GetTimedRequest(&mIsTimedRequest);
    SuccessOrExit(err);

    err = writeRequestParser.GetMoreChunkedMessages(&mHasMoreChunks);
    if (err == CHIP_ERROR_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    if (mHasMoreChunks && (mExchangeCtx->IsGroupExchangeContext() || mIsTimedRequest))
    {
        // Sanity check: group exchange context should only have one chunk.
        // Also, timed requests should not have more than one chunk.
        ExitNow(err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
    }

    err = writeRequestParser.GetWriteRequests(&AttributeDataIBsParser);
    SuccessOrExit(err);

    if (mIsTimedRequest != aIsTimedWrite)
    {
        // The message thinks it should be part of a timed interaction but it's
        // not, or vice versa.  Spec says to Respond with UNSUPPORTED_ACCESS.
        status = Status::UnsupportedAccess;
        goto exit;
    }

    AttributeDataIBsParser.GetReader(&AttributeDataIBsReader);

    if (mExchangeCtx->IsGroupExchangeContext())
    {
        err = ProcessGroupAttributeDataIBs(AttributeDataIBsReader);
    }
    else
    {
        err = ProcessAttributeDataIBs(AttributeDataIBsReader);
    }
    SuccessOrExit(err);
    SuccessOrExit(err = writeRequestParser.ExitContainer());

    if (err == CHIP_NO_ERROR)
    {
        status = Status::Success;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DataManagement, "Failed to process write request: %" CHIP_ERROR_FORMAT, err.Format());
    }
    return status;
}

CHIP_ERROR WriteHandler::AddStatus(const ConcreteDataAttributePath & aPath, const Protocols::InteractionModel::Status aStatus)
{
    return AddStatus(aPath, StatusIB(aStatus));
}

CHIP_ERROR WriteHandler::AddClusterSpecificSuccess(const ConcreteDataAttributePath & aPath, ClusterStatus aClusterStatus)
{
    return AddStatus(aPath, StatusIB(Status::Success, aClusterStatus));
}

CHIP_ERROR WriteHandler::AddClusterSpecificFailure(const ConcreteDataAttributePath & aPath, ClusterStatus aClusterStatus)
{
    return AddStatus(aPath, StatusIB(Status::Failure, aClusterStatus));
}

CHIP_ERROR WriteHandler::AddStatus(const ConcreteDataAttributePath & aPath, const StatusIB & aStatus)
{
    AttributeStatusIBs::Builder & writeResponses   = mWriteResponseBuilder.GetWriteResponses();
    AttributeStatusIB::Builder & attributeStatusIB = writeResponses.CreateAttributeStatus();

    if (!aStatus.IsSuccess())
    {
        mAttributeWriteSuccessful = false;
    }

    ReturnErrorOnFailure(writeResponses.GetError());

    AttributePathIB::Builder & path = attributeStatusIB.CreatePath();
    ReturnErrorOnFailure(attributeStatusIB.GetError());
    ReturnErrorOnFailure(path.Encode(aPath));

    StatusIB::Builder & statusIBBuilder = attributeStatusIB.CreateErrorStatus();
    ReturnErrorOnFailure(attributeStatusIB.GetError());
    statusIBBuilder.EncodeStatusIB(aStatus);
    ReturnErrorOnFailure(statusIBBuilder.GetError());
    ReturnErrorOnFailure(attributeStatusIB.EndOfAttributeStatusIB());

    MoveToState(State::AddStatus);
    return CHIP_NO_ERROR;
}

FabricIndex WriteHandler::GetAccessingFabricIndex() const
{
    return mExchangeCtx->GetSessionHandle()->GetFabricIndex();
}

const char * WriteHandler::GetStateStr() const
{
#if CHIP_DETAIL_LOGGING
    switch (mState)
    {
    case State::Uninitialized:
        return "Uninitialized";

    case State::Initialized:
        return "Initialized";

    case State::AddStatus:
        return "AddStatus";
    case State::Sending:
        return "Sending";
    }
#endif // CHIP_DETAIL_LOGGING
    return "N/A";
}

void WriteHandler::MoveToState(const State aTargetState)
{
    mState = aTargetState;
    ChipLogDetail(DataManagement, "IM WH moving to [%s]", GetStateStr());
}

} // namespace app
} // namespace chip
