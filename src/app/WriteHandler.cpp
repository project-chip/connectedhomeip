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

#include <app/AppConfig.h>
#include <app/AttributeAccessInterfaceRegistry.h>
#include <app/AttributeValueDecoder.h>
#include <app/ConcreteAttributePath.h>
#include <app/GlobalAttributes.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/EventPathIB.h>
#include <app/MessageDef/StatusIB.h>
#include <app/RequiredPrivilege.h>
#include <app/StatusResponse.h>
#include <app/WriteHandler.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataLookup.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/reporting/Engine.h>
#include <app/util/MatterCallbacks.h>
#include <credentials/GroupDataProvider.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/TypeTraits.h>
#include <lib/support/logging/TextOnlyLogging.h>
#include <messaging/ExchangeContext.h>
#include <protocols/interaction_model/StatusCode.h>

#include <optional>

namespace chip {
namespace app {

namespace {

using Protocols::InteractionModel::Status;

/// Wraps a EndpointIterator and ensures that `::Release()` is called
/// for the iterator (assuming it is non-null)
class AutoReleaseGroupEndpointIterator
{
public:
    explicit AutoReleaseGroupEndpointIterator(Credentials::GroupDataProvider::EndpointIterator * iterator) : mIterator(iterator) {}
    ~AutoReleaseGroupEndpointIterator()
    {
        if (mIterator != nullptr)
        {
            mIterator->Release();
        }
    }

    bool IsNull() const { return mIterator == nullptr; }
    bool Next(Credentials::GroupDataProvider::GroupEndpoint & item) { return mIterator->Next(item); }

private:
    Credentials::GroupDataProvider::EndpointIterator * mIterator;
};

} // namespace

using namespace Protocols::InteractionModel;
using Status = Protocols::InteractionModel::Status;

CHIP_ERROR WriteHandler::Init(DataModel::Provider * apProvider, WriteHandlerDelegate * apWriteHandlerDelegate)
{
    VerifyOrReturnError(!mExchangeCtx, CHIP_ERROR_INCORRECT_STATE);
    VerifyOrReturnError(apWriteHandlerDelegate, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(apProvider, CHIP_ERROR_INVALID_ARGUMENT);
    mDataModelProvider = apProvider;

    mDelegate = apWriteHandlerDelegate;
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
    mStateFlags.Clear(StateBits::kSuppressResponse);
    mDataModelProvider = nullptr;
    MoveToState(State::Uninitialized);
}

std::optional<bool> WriteHandler::IsListAttributePath(const ConcreteAttributePath & path)
{
    if (mDataModelProvider == nullptr)
    {
#if CHIP_CONFIG_DATA_MODEL_EXTRA_LOGGING
        ChipLogError(DataManagement, "Null data model while checking attribute properties.");
#endif
        return std::nullopt;
    }

    DataModel::AttributeFinder finder(mDataModelProvider);
    std::optional<DataModel::AttributeEntry> info = finder.Find(path);

    if (!info.has_value())
    {
        return std::nullopt;
    }

    return info->flags.Has(DataModel::AttributeQualityFlags::kListAttribute);
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
    if (!(status == Status::Success && mStateFlags.Has(StateBits::kHasMoreChunks)))
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
        if (!mStateFlags.Has(StateBits::kHasMoreChunks))
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
                                    mStateFlags.Has(StateBits::kHasMoreChunks) ? Messaging::SendMessageFlags::kExpectResponse
                                                                               : Messaging::SendMessageFlags::kNone);
    SuccessOrExit(err);

    MoveToState(State::Sending);

exit:
    return err;
}

void WriteHandler::DeliverListWriteBegin(const ConcreteAttributePath & aPath)
{
    if (mDataModelProvider != nullptr)
    {
        mDataModelProvider->ListAttributeWriteNotification(aPath, DataModel::ListWriteOperation::kListWriteBegin);
    }
}

void WriteHandler::DeliverListWriteEnd(const ConcreteAttributePath & aPath, bool writeWasSuccessful)
{
    if (mDataModelProvider != nullptr)
    {
        mDataModelProvider->ListAttributeWriteNotification(aPath,
                                                           writeWasSuccessful ? DataModel::ListWriteOperation::kListWriteSuccess
                                                                              : DataModel::ListWriteOperation::kListWriteFailure);
    }
}

void WriteHandler::DeliverFinalListWriteEnd(bool writeWasSuccessful)
{
    if (mProcessingAttributePath.HasValue() && mStateFlags.Has(StateBits::kProcessingAttributeIsList))
    {
        DeliverListWriteEnd(mProcessingAttributePath.Value(), writeWasSuccessful);
    }
    mProcessingAttributePath.ClearValue();
}

CHIP_ERROR WriteHandler::DeliverFinalListWriteEndForGroupWrite(bool writeWasSuccessful)
{
    VerifyOrReturnError(mProcessingAttributePath.HasValue() && mStateFlags.Has(StateBits::kProcessingAttributeIsList),
                        CHIP_NO_ERROR);

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

        VerifyOrReturnError(mDelegate, CHIP_ERROR_INCORRECT_STATE);
        if (!mDelegate->HasConflictWriteRequests(this, processingConcreteAttributePath))
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

    VerifyOrReturnError(mExchangeCtx, CHIP_ERROR_INTERNAL);
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

        if (!dataAttributePath.IsListOperation() && IsListAttributePath(dataAttributePath).value_or(false))
        {
            dataAttributePath.mListOp = ConcreteDataAttributePath::ListOperation::ReplaceAll;
        }

        VerifyOrExit(mDelegate, err = CHIP_ERROR_INCORRECT_STATE);
        if (mDelegate->HasConflictWriteRequests(this, dataAttributePath) ||
            // Per chunking protocol, we are processing the list entries, but the initial empty list is not processed, so we reject
            // it with Busy status code.
            (dataAttributePath.IsListItemOperation() && !IsSameAttribute(mProcessingAttributePath, dataAttributePath)))
        {
            err = AddStatusInternal(dataAttributePath, StatusIB(Status::Busy));
            continue;
        }

        if (ShouldReportListWriteEnd(mProcessingAttributePath, mStateFlags.Has(StateBits::kProcessingAttributeIsList),
                                     dataAttributePath))
        {
            DeliverListWriteEnd(mProcessingAttributePath.Value(), mStateFlags.Has(StateBits::kAttributeWriteSuccessful));
        }

        if (ShouldReportListWriteBegin(mProcessingAttributePath, mStateFlags.Has(StateBits::kProcessingAttributeIsList),
                                       dataAttributePath))
        {
            DeliverListWriteBegin(dataAttributePath);
            mStateFlags.Set(StateBits::kAttributeWriteSuccessful);
        }

        mStateFlags.Set(StateBits::kProcessingAttributeIsList, dataAttributePath.IsListOperation());
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
        err = WriteClusterData(subjectDescriptor, dataAttributePath, dataReader);
        if (err != CHIP_NO_ERROR)
        {
            mWriteResponseBuilder.GetWriteResponses().Rollback(backup);
            err = AddStatusInternal(dataAttributePath, StatusIB(err));
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

    if (!mStateFlags.Has(StateBits::kHasMoreChunks))
    {
        DeliverFinalListWriteEnd(mStateFlags.Has(StateBits::kAttributeWriteSuccessful));
    }

exit:
    return err;
}

CHIP_ERROR WriteHandler::ProcessGroupAttributeDataIBs(TLV::TLVReader & aAttributeDataIBsReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mExchangeCtx, CHIP_ERROR_INTERNAL);
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

        AutoReleaseGroupEndpointIterator iterator(Credentials::GetGroupDataProvider()->IterateEndpoints(fabric));
        VerifyOrExit(!iterator.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        bool shouldReportListWriteEnd = ShouldReportListWriteEnd(
            mProcessingAttributePath, mStateFlags.Has(StateBits::kProcessingAttributeIsList), dataAttributePath);
        bool shouldReportListWriteBegin = false; // This will be set below.

        std::optional<bool> isListAttribute = std::nullopt;

        Credentials::GroupDataProvider::GroupEndpoint mapping;
        while (iterator.Next(mapping))
        {
            if (groupId != mapping.group_id)
            {
                continue;
            }

            dataAttributePath.mEndpointId = mapping.endpoint_id;

            // Try to get the metadata from for the attribute from one of the expanded endpoints (it doesn't really matter which
            // endpoint we pick, as long as it's valid) and update the path info according to it and recheck if we need to report
            // list write begin.
            if (!isListAttribute.has_value())
            {
                isListAttribute             = IsListAttributePath(dataAttributePath);
                bool currentAttributeIsList = isListAttribute.value_or(false);

                if (!dataAttributePath.IsListOperation() && currentAttributeIsList)
                {
                    dataAttributePath.mListOp = ConcreteDataAttributePath::ListOperation::ReplaceAll;
                }
                ConcreteDataAttributePath pathForCheckingListWriteBegin(kInvalidEndpointId, dataAttributePath.mClusterId,
                                                                        dataAttributePath.mEndpointId, dataAttributePath.mListOp,
                                                                        dataAttributePath.mListIndex);
                shouldReportListWriteBegin =
                    ShouldReportListWriteBegin(mProcessingAttributePath, mStateFlags.Has(StateBits::kProcessingAttributeIsList),
                                               pathForCheckingListWriteBegin);
            }

            if (shouldReportListWriteEnd)
            {
                auto processingConcreteAttributePath        = mProcessingAttributePath.Value();
                processingConcreteAttributePath.mEndpointId = mapping.endpoint_id;
                VerifyOrExit(mDelegate, err = CHIP_ERROR_INCORRECT_STATE);
                if (mDelegate->HasConflictWriteRequests(this, processingConcreteAttributePath))
                {
                    DeliverListWriteEnd(processingConcreteAttributePath, true /* writeWasSuccessful */);
                }
            }

            VerifyOrExit(mDelegate, err = CHIP_ERROR_INCORRECT_STATE);
            if (mDelegate->HasConflictWriteRequests(this, dataAttributePath))
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
            err = WriteClusterData(subjectDescriptor, dataAttributePath, tmpDataReader);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DataManagement,
                             "WriteClusterData Endpoint=%u Cluster=" ChipLogFormatMEI " Attribute =" ChipLogFormatMEI
                             " failed: %" CHIP_ERROR_FORMAT,
                             mapping.endpoint_id, ChipLogValueMEI(dataAttributePath.mClusterId),
                             ChipLogValueMEI(dataAttributePath.mAttributeId), err.Format());
            }
            DataModelCallbacks::GetInstance()->AttributeOperation(DataModelCallbacks::OperationType::Write,
                                                                  DataModelCallbacks::OperationOrder::Post, dataAttributePath);
        }

        dataAttributePath.mEndpointId = kInvalidEndpointId;
        mStateFlags.Set(StateBits::kProcessingAttributeIsList, dataAttributePath.IsListOperation());
        mProcessingAttributePath.SetValue(dataAttributePath);
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

    mLastSuccessfullyWrittenPath = std::nullopt;

    reader.Init(std::move(aPayload));

    err = writeRequestParser.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    writeRequestParser.PrettyPrint();
#endif // CHIP_CONFIG_IM_PRETTY_PRINT
    bool boolValue;

    boolValue = mStateFlags.Has(StateBits::kSuppressResponse);
    err       = writeRequestParser.GetSuppressResponse(&boolValue);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);
    mStateFlags.Set(StateBits::kSuppressResponse, boolValue);

    boolValue = mStateFlags.Has(StateBits::kIsTimedRequest);
    err       = writeRequestParser.GetTimedRequest(&boolValue);
    SuccessOrExit(err);
    mStateFlags.Set(StateBits::kIsTimedRequest, boolValue);

    boolValue = mStateFlags.Has(StateBits::kHasMoreChunks);
    err       = writeRequestParser.GetMoreChunkedMessages(&boolValue);
    if (err == CHIP_ERROR_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);
    mStateFlags.Set(StateBits::kHasMoreChunks, boolValue);

    if (mStateFlags.Has(StateBits::kHasMoreChunks) &&
        (mExchangeCtx->IsGroupExchangeContext() || mStateFlags.Has(StateBits::kIsTimedRequest)))
    {
        // Sanity check: group exchange context should only have one chunk.
        // Also, timed requests should not have more than one chunk.
        ExitNow(err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
    }

    err = writeRequestParser.GetWriteRequests(&AttributeDataIBsParser);
    SuccessOrExit(err);

    if (mStateFlags.Has(StateBits::kIsTimedRequest) != aIsTimedWrite)
    {
        // The message thinks it should be part of a timed interaction but it's
        // not, or vice versa.
        status = Status::TimedRequestMismatch;
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

CHIP_ERROR WriteHandler::AddStatus(const ConcreteDataAttributePath & aPath,
                                   const Protocols::InteractionModel::ClusterStatusCode & aStatus)
{
    return AddStatusInternal(aPath, StatusIB{ aStatus });
}

CHIP_ERROR WriteHandler::AddClusterSpecificSuccess(const ConcreteDataAttributePath & aPath, ClusterStatus aClusterStatus)
{
    return AddStatus(aPath, Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificSuccess(aClusterStatus));
}

CHIP_ERROR WriteHandler::AddClusterSpecificFailure(const ConcreteDataAttributePath & aPath, ClusterStatus aClusterStatus)
{
    return AddStatus(aPath, Protocols::InteractionModel::ClusterStatusCode::ClusterSpecificFailure(aClusterStatus));
}

CHIP_ERROR WriteHandler::AddStatusInternal(const ConcreteDataAttributePath & aPath, const StatusIB & aStatus)
{
    AttributeStatusIBs::Builder & writeResponses   = mWriteResponseBuilder.GetWriteResponses();
    AttributeStatusIB::Builder & attributeStatusIB = writeResponses.CreateAttributeStatus();

    if (!aStatus.IsSuccess())
    {
        mStateFlags.Clear(StateBits::kAttributeWriteSuccessful);
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

DataModel::ActionReturnStatus WriteHandler::CheckWriteAllowed(const Access::SubjectDescriptor & aSubject,
                                                              const ConcreteAttributePath & aPath)
{
    // TODO: ordering is to check writability/existence BEFORE ACL and this seems wrong, however
    //       existing unit tests (TC_AcessChecker.py) validate that we get UnsupportedWrite instead of UnsupportedAccess
    //
    //       This should likely be fixed in spec (probably already fixed by
    //       https://github.com/CHIP-Specifications/connectedhomeip-spec/pull/9024)
    //       and tests and implementation
    //
    //       Open issue that needs fixing: https://github.com/project-chip/connectedhomeip/issues/33735

    std::optional<DataModel::AttributeEntry> attributeEntry;
    DataModel::AttributeFinder finder(mDataModelProvider);

    attributeEntry = finder.Find(aPath);

    // if path is not valid, return a spec-compliant return code.
    if (!attributeEntry.has_value())
    {
        // Global lists are not in metadata and not writable. Return the correct error code according to the spec
        Status attributeErrorStatus =
            IsSupportedGlobalAttributeNotInMetadata(aPath.mAttributeId) ? Status::UnsupportedWrite : Status::UnsupportedAttribute;

        return DataModel::ValidateClusterPath(mDataModelProvider, aPath, attributeErrorStatus);
    }

    // Allow writes on writable attributes only
    VerifyOrReturnValue(attributeEntry->writePrivilege.has_value(), Status::UnsupportedWrite);

    bool checkAcl = true;
    if (mLastSuccessfullyWrittenPath.has_value())
    {
        // only validate ACL if path has changed
        //
        // Note that this is NOT operator==: we could do `checkAcl == (aPath != *mLastSuccessfullyWrittenPath)`
        // however that seems to use more flash.
        if ((aPath.mEndpointId == mLastSuccessfullyWrittenPath->mEndpointId) &&
            (aPath.mClusterId == mLastSuccessfullyWrittenPath->mClusterId) &&
            (aPath.mAttributeId == mLastSuccessfullyWrittenPath->mAttributeId))
        {
            checkAcl = false;
        }
    }

    if (checkAcl)
    {
        Access::RequestPath requestPath{ .cluster     = aPath.mClusterId,
                                         .endpoint    = aPath.mEndpointId,
                                         .requestType = Access::RequestType::kAttributeWriteRequest,
                                         .entityId    = aPath.mAttributeId };
        CHIP_ERROR err = Access::GetAccessControl().Check(aSubject, requestPath, RequiredPrivilege::ForWriteAttribute(aPath));

        if (err != CHIP_NO_ERROR)
        {
            VerifyOrReturnValue(err != CHIP_ERROR_ACCESS_DENIED, Status::UnsupportedAccess);
            VerifyOrReturnValue(err != CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL, Status::AccessRestricted);

            return err;
        }
    }

    // validate that timed write is enforced
    VerifyOrReturnValue(IsTimedWrite() || !attributeEntry->flags.Has(DataModel::AttributeQualityFlags::kTimed),
                        Status::NeedsTimedInteraction);

    return Status::Success;
}

CHIP_ERROR WriteHandler::WriteClusterData(const Access::SubjectDescriptor & aSubject, const ConcreteDataAttributePath & aPath,
                                          TLV::TLVReader & aData)
{
    // Writes do not have a checked-path. If data model interface is enabled (both checked and only version)
    // the write is done via the DataModel interface
    VerifyOrReturnError(mDataModelProvider != nullptr, CHIP_ERROR_INCORRECT_STATE);

    ChipLogDetail(DataManagement, "Writing attribute: Cluster=" ChipLogFormatMEI " Endpoint=0x%x AttributeId=" ChipLogFormatMEI,
                  ChipLogValueMEI(aPath.mClusterId), aPath.mEndpointId, ChipLogValueMEI(aPath.mAttributeId));

    DataModel::ActionReturnStatus status = CheckWriteAllowed(aSubject, aPath);
    if (status.IsSuccess())
    {
        DataModel::WriteAttributeRequest request;

        request.path              = aPath;
        request.subjectDescriptor = &aSubject;
        request.writeFlags.Set(DataModel::WriteFlags::kTimed, IsTimedWrite());

        AttributeValueDecoder decoder(aData, aSubject);
        status = mDataModelProvider->WriteAttribute(request, decoder);
    }

    mLastSuccessfullyWrittenPath = status.IsSuccess() ? std::make_optional(aPath) : std::nullopt;

    return AddStatusInternal(aPath, StatusIB(status.GetStatusCode()));
}

} // namespace app
} // namespace chip
