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

#include <app/AppBuildConfig.h>
#include <app/InteractionModelEngine.h>
#include <app/MessageDef/EventPathIB.h>
#include <app/WriteHandler.h>
#include <app/reporting/Engine.h>
#include <app/util/MatterCallbacks.h>
#include <lib/support/TypeTraits.h>

namespace chip {
namespace app {

using namespace Protocols::InteractionModel;

CHIP_ERROR WriteHandler::Init(InteractionModelDelegate * apDelegate)
{
    IgnoreUnusedVariable(apDelegate);
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

void WriteHandler::Shutdown()
{
    VerifyOrReturn(mState != State::Uninitialized);
    mMessageWriter.Reset();
    mpExchangeCtx = nullptr;
    ClearState();
}

Status WriteHandler::OnWriteRequest(Messaging::ExchangeContext * apExchangeContext, System::PacketBufferHandle && aPayload,
                                    bool aIsTimedWrite)
{
    mpExchangeCtx = apExchangeContext;

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

    Shutdown();
    return status;
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
        err                   = element.Init(reader);
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
        if (mpExchangeCtx->IsGroupExchangeContext())
        {
            // TODO retrieve Endpoint ID with GroupDataProvider using GroupId and FabricId
            // Issue 11075

            // Using endpoint 0 for test purposes
            clusterInfo.mEndpointId = 0;
        }
        else
        {
            err = attributePath.GetEndpoint(&(clusterInfo.mEndpointId));
            SuccessOrExit(err);
        }

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
            err = WriteSingleClusterData(subjectDescriptor, clusterInfo, dataReader, this);
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
    err = ProcessAttributeDataIBs(AttributeDataIBsReader);
    if (err == CHIP_NO_ERROR)
    {
        status = Status::Success;
    }

exit:
    return status;
}

CHIP_ERROR WriteHandler::AddStatus(const AttributePathParams & aAttributePathParams,
                                   const Protocols::InteractionModel::Status aStatus)
{
    AttributeStatusIBs::Builder & writeResponses   = mWriteResponseBuilder.GetWriteResponses();
    AttributeStatusIB::Builder & attributeStatusIB = writeResponses.CreateAttributeStatus();
    ReturnErrorOnFailure(writeResponses.GetError());

    AttributePathIB::Builder & path = attributeStatusIB.CreatePath();
    ReturnErrorOnFailure(attributeStatusIB.GetError());
    ReturnErrorOnFailure(path.Encode(aAttributePathParams));

    StatusIB statusIB;
    statusIB.mStatus                    = aStatus;
    StatusIB::Builder & statusIBBuilder = attributeStatusIB.CreateErrorStatus();
    ReturnErrorOnFailure(attributeStatusIB.GetError());
    statusIBBuilder.EncodeStatusIB(statusIB);
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
        fabric = mpExchangeCtx->GetSessionHandle()->AsSecureSession()->GetFabricIndex();
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
