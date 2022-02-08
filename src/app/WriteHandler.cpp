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
#include <app/AppBuildConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/EventPathIB.h>
#include <app/WriteHandler.h>
#include <app/reporting/Engine.h>
#include <app/util/MatterCallbacks.h>
#include <credentials/GroupDataProvider.h>
#include <lib/support/TypeTraits.h>

namespace chip {
namespace app {

using namespace Protocols::InteractionModel;

CHIP_ERROR WriteHandler::Init()
{
    VerifyOrReturnError(mpExchangeCtx == nullptr, CHIP_ERROR_INCORRECT_STATE);

    System::PacketBufferHandle packet = System::PacketBufferHandle::New(chip::app::kMaxSecureSduLengthBytes);
    VerifyOrReturnError(!packet.IsNull(), CHIP_ERROR_NO_MEMORY);

    mMessageWriter.Init(std::move(packet));
    ReturnErrorOnFailure(mWriteResponseBuilder.Init(&mMessageWriter));

    mWriteResponseBuilder.CreateWriteResponses();
    ReturnErrorOnFailure(mWriteResponseBuilder.GetError());

    MoveToState(State::Initialized);

    return CHIP_NO_ERROR;
}

void WriteHandler::Close()
{
    VerifyOrReturn(mState != State::Uninitialized);

    mMessageWriter.Reset();

    if (mpExchangeCtx != nullptr)
    {
        mpExchangeCtx->SetDelegate(nullptr);
        mpExchangeCtx = nullptr;
    }

    ClearState();
}

void WriteHandler::Abort()
{
#if 0
    // TODO: When chunking gets added, we should add this back.
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

    ClearState();
#else
    //
    // The WriteHandler should get synchronously allocated and destroyed in the same execution
    // context given that it's just a 2 message exchange (request + response). Consequently, we should
    // never arrive at a situation where we have active handlers at any time Abort() is called.
    //
    VerifyOrDie(mState == State::Uninitialized);
#endif
}

Status WriteHandler::OnWriteRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload,
                                    bool aIsTimedWrite)
{
    mpExchangeCtx = apExchangeContext;

    //
    // Let's take over further message processing on this exchange from the IM.
    // This is only relevant during chunked requests.
    //
    mpExchangeCtx->SetDelegate(this);

    Status status = ProcessWriteRequest(std::move(aPayload), aIsTimedWrite);

    // Do not send response on Group Write
    if (status == Status::Success && !apExchangeContext->IsGroupExchangeContext())
    {
        CHIP_ERROR err = SendWriteResponse();
        if (err != CHIP_NO_ERROR)
        {
            status = Status::Failure;
        }
    }

    Close();
    return status;
}

CHIP_ERROR WriteHandler::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                           System::PacketBufferHandle && aPayload)
{
    //
    // As part of write handling, the exchange should get closed sychronously given there is always
    // just a single response to a Write Request message before the exchange gets closed. There-after,
    // even if we get any more messages on that exchange from a non-compliant client, our exchange layer
    // should correctly discard those. If there is a bug there, this function here may get invoked.
    //
    // NOTE: Once chunking gets implemented, this will no longer be true.
    //
    VerifyOrDieWithMsg(false, DataManagement, "This function should never get invoked");
}

void WriteHandler::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    //
    // As part of write handling, the exchange should get closed sychronously given there is always
    // just a single response to a Write Request message before the exchange gets closed. That response
    // does not solicit any further responses back. Consequently, we never expect to get notified
    // of any response timeouts.
    //
    // NOTE: Once chunking gets implemented, this will no longer be true.
    //
    VerifyOrDieWithMsg(false, DataManagement, "This function should never get invoked");
}

CHIP_ERROR WriteHandler::FinalizeMessage(System::PacketBufferHandle & packet)
{
    VerifyOrReturnError(mState == State::AddStatus, CHIP_ERROR_INCORRECT_STATE);
    AttributeStatusIBs::Builder & attributeStatusIBs = mWriteResponseBuilder.GetWriteResponses().EndOfAttributeStatuses();
    ReturnErrorOnFailure(attributeStatusIBs.GetError());
    mWriteResponseBuilder.EndOfWriteResponseMessage();
    ReturnErrorOnFailure(mWriteResponseBuilder.GetError());
    ReturnErrorOnFailure(mMessageWriter.Finalize(&packet));
    return CHIP_NO_ERROR;
}

CHIP_ERROR WriteHandler::SendWriteResponse()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle packet;

    VerifyOrExit(mState == State::AddStatus, err = CHIP_ERROR_INCORRECT_STATE);

    err = FinalizeMessage(packet);
    SuccessOrExit(err);

    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_INCORRECT_STATE);
    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::WriteResponse, std::move(packet));
    SuccessOrExit(err);

    MoveToState(State::Sending);

exit:
    return err;
}

CHIP_ERROR WriteHandler::ProcessAttributeDataIBs(TLV::TLVReader & aAttributeDataIBsReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorCodeIf(mpExchangeCtx == nullptr, CHIP_ERROR_INTERNAL);
    const Access::SubjectDescriptor subjectDescriptor = mpExchangeCtx->GetSessionHandle()->GetSubjectDescriptor();

    while (CHIP_NO_ERROR == (err = aAttributeDataIBsReader.Next()))
    {
        chip::TLV::TLVReader dataReader;
        AttributeDataIB::Parser element;
        AttributePathIB::Parser attributePath;
        ClusterInfo clusterInfo;
        TLV::TLVReader reader = aAttributeDataIBsReader;

        err = element.Init(reader);
        SuccessOrExit(err);

        err = element.GetPath(&attributePath);
        SuccessOrExit(err);

        // We are using the feature that the parser won't touch the value if the field does not exist, since all fields in the
        // cluster info will be invalid / wildcard, it is safe ignore CHIP_END_OF_TLV directly.

        err = attributePath.GetNode(&(clusterInfo.mNodeId));
        if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }

        err = attributePath.GetEndpoint(&(clusterInfo.mEndpointId));
        SuccessOrExit(err);

        err = attributePath.GetCluster(&(clusterInfo.mClusterId));
        SuccessOrExit(err);

        err = attributePath.GetAttribute(&(clusterInfo.mAttributeId));
        SuccessOrExit(err);

        err = attributePath.GetListIndex(&(clusterInfo.mListIndex));
        if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }

        // We do not support Wildcard writes for now, reject all wildcard write requests.
        VerifyOrExit(clusterInfo.IsValidAttributePath() && !clusterInfo.HasAttributeWildcard(),
                     err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH);

        err = element.GetData(&dataReader);
        SuccessOrExit(err);

        {
            const ConcreteAttributePath concretePath =
                ConcreteAttributePath(clusterInfo.mEndpointId, clusterInfo.mClusterId, clusterInfo.mAttributeId);
            MatterPreAttributeWriteCallback(concretePath);
            TLV::TLVWriter backup;
            mWriteResponseBuilder.Checkpoint(backup);
            err = WriteSingleClusterData(subjectDescriptor, clusterInfo, dataReader, this);
            if (err != CHIP_NO_ERROR)
            {
                mWriteResponseBuilder.Rollback(backup);
                err = AddStatus(concretePath, StatusIB(err));
            }
            MatterPostAttributeWriteCallback(concretePath);
        }
        SuccessOrExit(err);
    }

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }

exit:
    return err;
}

CHIP_ERROR WriteHandler::ProcessGroupAttributeDataIBs(TLV::TLVReader & aAttributeDataIBsReader)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ReturnErrorCodeIf(mpExchangeCtx == nullptr, CHIP_ERROR_INTERNAL);
    const Access::SubjectDescriptor subjectDescriptor = mpExchangeCtx->GetSessionHandle()->AsGroupSession()->GetSubjectDescriptor();

    while (CHIP_NO_ERROR == (err = aAttributeDataIBsReader.Next()))
    {
        chip::TLV::TLVReader dataReader;
        AttributeDataIB::Parser element;
        AttributePathIB::Parser attributePath;
        ClusterInfo clusterInfo;
        GroupId groupId;
        FabricIndex fabric;
        TLV::TLVReader reader = aAttributeDataIBsReader;

        Credentials::GroupDataProvider::GroupEndpoint mapping;
        Credentials::GroupDataProvider * groupDataProvider = Credentials::GetGroupDataProvider();
        Credentials::GroupDataProvider::EndpointIterator * iterator;

        err = element.Init(reader);
        SuccessOrExit(err);

        err = element.GetPath(&attributePath);
        SuccessOrExit(err);

        // We are using the feature that the parser won't touch the value if the field does not exist, since all fields in the
        // cluster info will be invalid / wildcard, it is safe to ignore CHIP_END_OF_TLV.

        err = attributePath.GetNode(&(clusterInfo.mNodeId));
        if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }

        err = attributePath.GetCluster(&(clusterInfo.mClusterId));
        SuccessOrExit(err);

        err = attributePath.GetAttribute(&(clusterInfo.mAttributeId));
        SuccessOrExit(err);

        groupId = mpExchangeCtx->GetSessionHandle()->AsGroupSession()->GetGroupId();
        fabric  = GetAccessingFabricIndex();

        err = attributePath.GetListIndex(&(clusterInfo.mListIndex));
        if (CHIP_END_OF_TLV == err)
        {
            err = CHIP_NO_ERROR;
        }

        err = element.GetData(&dataReader);
        SuccessOrExit(err);

        ChipLogDetail(DataManagement,
                      "Received group attribute write for Group=%" PRIu16 " Cluster=" ChipLogFormatMEI
                      " attribute=" ChipLogFormatMEI,
                      groupId, ChipLogValueMEI(clusterInfo.mClusterId), ChipLogValueMEI(clusterInfo.mAttributeId));

        iterator = groupDataProvider->IterateEndpoints(fabric);
        VerifyOrExit(iterator != nullptr, err = CHIP_ERROR_NO_MEMORY);

        while (iterator->Next(mapping))
        {
            if (groupId != mapping.group_id)
            {
                continue;
            }

            clusterInfo.mEndpointId = mapping.endpoint_id;

            if (!clusterInfo.IsValidAttributePath() || clusterInfo.HasAttributeWildcard())
            {
                ChipLogDetail(DataManagement,
                              "Invalid group attribute write for endpoint=%" PRIu16 " Cluster=" ChipLogFormatMEI
                              " attribute=" ChipLogFormatMEI,
                              mapping.endpoint_id, ChipLogValueMEI(clusterInfo.mClusterId),
                              ChipLogValueMEI(clusterInfo.mAttributeId));

                continue;
            }

            ChipLogDetail(DataManagement,
                          "Processing group attribute write for endpoint=%" PRIu16 " Cluster=" ChipLogFormatMEI
                          " attribute=" ChipLogFormatMEI,
                          mapping.endpoint_id, ChipLogValueMEI(clusterInfo.mClusterId), ChipLogValueMEI(clusterInfo.mAttributeId));

            chip::TLV::TLVReader tmpDataReader(dataReader);

            const ConcreteAttributePath concretePath(clusterInfo.mEndpointId, clusterInfo.mClusterId, clusterInfo.mAttributeId);

            MatterPreAttributeWriteCallback(concretePath);
            err = WriteSingleClusterData(subjectDescriptor, clusterInfo, tmpDataReader, this);

            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DataManagement,
                             "Error when calling WriteSingleClusterData for Endpoint=%" PRIu16 " Cluster=" ChipLogFormatMEI
                             " Attribute =" ChipLogFormatMEI " : %" CHIP_ERROR_FORMAT,
                             mapping.endpoint_id, ChipLogValueMEI(clusterInfo.mClusterId),
                             ChipLogValueMEI(clusterInfo.mAttributeId), err.Format());
            }
            MatterPostAttributeWriteCallback(concretePath);
        }

        iterator->Release();
    }

    if (CHIP_END_OF_TLV == err)
    {
        err = CHIP_NO_ERROR;
    }
exit:
    return err;
}

Status WriteHandler::ProcessWriteRequest(System::PacketBufferHandle && aPayload, bool aIsTimedWrite)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;

    WriteRequestMessage::Parser writeRequestParser;
    AttributeDataIBs::Parser AttributeDataIBsParser;
    TLV::TLVReader AttributeDataIBsReader;
    bool needSuppressResponse = false;
    // Default to InvalidAction for our status; that's what we want if any of
    // the parsing of our overall structure or paths fails.  Once we have a
    // successfully parsed path, the only way we will get a failure return is if
    // our path handling fails to AddStatus on us.
    //
    // TODO: That's not technically InvalidAction, and we should probably make
    // our callees hand out Status as well.
    Status status = Status::InvalidAction;

    reader.Init(std::move(aPayload));

    err = reader.Next();
    SuccessOrExit(err);

    err = writeRequestParser.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = writeRequestParser.CheckSchemaValidity();
    SuccessOrExit(err);
#endif
    err = writeRequestParser.GetSuppressResponse(&needSuppressResponse);
    if (err == CHIP_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }
    SuccessOrExit(err);

    err = writeRequestParser.GetTimedRequest(&mIsTimedRequest);
    SuccessOrExit(err);

    err = writeRequestParser.GetIsFabricFiltered(&mIsFabricFiltered);
    SuccessOrExit(err);

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

    if (mpExchangeCtx->IsGroupExchangeContext())
    {
        err = ProcessGroupAttributeDataIBs(AttributeDataIBsReader);
    }
    else
    {
        err = ProcessAttributeDataIBs(AttributeDataIBsReader);
    }

    if (err == CHIP_NO_ERROR)
    {
        status = Status::Success;
    }

exit:
    return status;
}

CHIP_ERROR WriteHandler::AddStatus(const ConcreteAttributePath & aPath, const Protocols::InteractionModel::Status aStatus)
{
    return AddStatus(aPath, StatusIB(aStatus));
}

CHIP_ERROR WriteHandler::AddStatus(const ConcreteAttributePath & aPath, const StatusIB & aStatus)
{
    AttributeStatusIBs::Builder & writeResponses   = mWriteResponseBuilder.GetWriteResponses();
    AttributeStatusIB::Builder & attributeStatusIB = writeResponses.CreateAttributeStatus();
    ReturnErrorOnFailure(writeResponses.GetError());

    AttributePathIB::Builder & path = attributeStatusIB.CreatePath();
    ReturnErrorOnFailure(attributeStatusIB.GetError());
    ReturnErrorOnFailure(path.Encode(AttributePathParams(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId)));

    StatusIB::Builder & statusIBBuilder = attributeStatusIB.CreateErrorStatus();
    ReturnErrorOnFailure(attributeStatusIB.GetError());
    statusIBBuilder.EncodeStatusIB(aStatus);
    ReturnErrorOnFailure(statusIBBuilder.GetError());
    attributeStatusIB.EndOfAttributeStatusIB();
    ReturnErrorOnFailure(attributeStatusIB.GetError());

    MoveToState(State::AddStatus);
    return CHIP_NO_ERROR;
}

FabricIndex WriteHandler::GetAccessingFabricIndex() const
{
    FabricIndex fabric = kUndefinedFabricIndex;
    if (mpExchangeCtx->GetSessionHandle()->IsGroupSession())
    {
        fabric = mpExchangeCtx->GetSessionHandle()->AsGroupSession()->GetFabricIndex();
    }
    else
    {
        fabric = mpExchangeCtx->GetSessionHandle()->GetFabricIndex();
    }

    return fabric;
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

void WriteHandler::ClearState()
{
    MoveToState(State::Uninitialized);
}

} // namespace app
} // namespace chip

void __attribute__((weak)) MatterPreAttributeWriteCallback(const chip::app::ConcreteAttributePath & attributePath) {}
void __attribute__((weak)) MatterPostAttributeWriteCallback(const chip::app::ConcreteAttributePath & attributePath) {}
