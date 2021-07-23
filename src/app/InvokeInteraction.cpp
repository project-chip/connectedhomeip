/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *      This file defines Base class for a CHIP IM Command
 *
 */
#include <platform/CHIPDeviceLayer.h>
#include "InvokeInteraction.h"
#include "InteractionModelEngine.h"
#include "MessageDef/CommandDataElement.h"
#include "core/CHIPTLVTags.h"
#include "messaging/ExchangeContext.h"
#include "system/SystemPacketBuffer.h"
#include "system/TLVPacketBufferBackingStore.h"
#include <core/CHIPTLVDebug.hpp>

namespace chip {
namespace app {

CHIP_ERROR InvokeResponder::Init(Messaging::ExchangeContext *apContext, System::PacketBufferHandle &&aBufferHandle)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;
    TLV::TLVReader commandListReader;
    InvokeCommand::Parser invokeCommandParser;
    CommandList::Parser commandListParser;

    VerifyOrReturnError(apContext != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mpExchangeCtx = apContext;
    mState = kStateReady;

    //
    // Increment the hold off to prevent transmission of any queued up responses till we've
    // aggregated them all.
    //
    IncrementHoldOffRef();

    reader.Init(std::move(aBufferHandle));

    err = reader.Next();
    SuccessOrExit(err);

    err = invokeCommandParser.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeCommandParser.CheckSchemaValidity();
    SuccessOrExit(err);
#endif

    err = invokeCommandParser.GetCommandList(&commandListParser);
    SuccessOrExit(err);

    commandListParser.GetReader(&commandListReader);

    while (CHIP_NO_ERROR == (err = commandListReader.Next()))
    {
        CommandParams params;
        CommandDataElement::Parser commandElement;
        CommandPath::Parser commandPath;
        TLV::TLVReader dataReader;
        TLV::TLVReader *pReader = &dataReader;

        VerifyOrExit(chip::TLV::AnonymousTag == commandListReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == commandListReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        err = commandElement.Init(commandListReader);
        SuccessOrExit(err);

        SuccessOrExit(commandElement.GetCommandPath(&commandPath));
        SuccessOrExit(commandPath.GetClusterId(&params.ClusterId));
        SuccessOrExit(commandPath.GetCommandId(&params.CommandId));
        SuccessOrExit(commandPath.GetEndpointId(&params.EndpointId));

        err = commandElement.GetData(pReader);
        if (err == CHIP_END_OF_TLV) {
            err = CHIP_NO_ERROR;
            pReader = NULL;
        }

        SuccessOrExit(err);

        {
            bool foundClusterInstance = false;

            InteractionModelEngine::GetInstance()->GetClusterServerSet().ForEachActiveObject([&](ClusterServer **s) {
                if ((*s)->GetClusterId() == params.ClusterId && (*s)->GetEndpoint() == params.EndpointId) {
                    foundClusterInstance = true;

                    err = (*s)->OnInvokeRequest(params, *this, pReader);
                    SuccessOrExit(err);

                    return false;
                }

            exit:
                if (err != CHIP_NO_ERROR) {
                    return true;
                }

                return true;
            });

            if (!foundClusterInstance) {
                ChipLogProgress(DataManagement, "Could not find a matching server cluster for command! (ClusterId = %" PRIx32 ", Endpoint = %lu, Command = %lu)",
                                params.ClusterId, (unsigned long)params.EndpointId, (unsigned long)params.CommandId);

                //
                // To permit a gradual transition away from the legacy Ember approach of handling commands, we need a way to 'fall-back' to the legacy approach
                // if we don't find a registered ClusterServer object. Returning a very specific error code here from this command signals to the caler to
                // switch back to the legacy call flow.
                //
                // TODO: Remove this once all legacy callers have moved over to the new model. In turn, re-enable the status report dispatch path below.
                SuccessOrExit((err = CHIP_ERROR_CLUSTER_NOT_FOUND));

                // err = AddStatusCode(params, Protocols::SecureChannel::GeneralStatusCode::kBadRequest,
                //                     Protocols::SecureChannel::Id, Protocols::SecureChannel::kProtocolCodeGeneralFailure);
                // SuccessOrExit(err);
            }
        }
    }

    if (err == CHIP_END_OF_TLV) {
        err = CHIP_NO_ERROR;
    }

exit:
    //
    // Decrement hold-off, and if zero, fire off any queued up responses and
    // shut outselves down.
    //
    DecrementHoldOffRef();
    return err;
}

CHIP_ERROR InvokeResponder::AddResponse(CommandParams aParams, IEncodableElement *serializable)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = StartCommandHeader(aParams);
    SuccessOrExit(err);

    //
    // Invoke the passed in closure that will actually write out the command payload if any
    //
    err = serializable->Encode(*mInvokeCommandBuilder.GetWriter(), TLV::ContextTag(CommandDataElement::kCsTag_Data));
    SuccessOrExit(err);

    mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().EndOfCommandDataElement();
    SuccessOrExit((err = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().GetError()));

exit:
    return err;
}

CHIP_ERROR InvokeResponder::AddStatusCode(const CommandParams &aParams, const StatusResponse &statusResponse)

{
    CHIP_ERROR err;
    StatusElement::Builder statusBuilder;

    // We don't need to sandwich this with Increment/Decrement calls, since this call to AddStatus
    // can only happen as part of an existing 'HandleMessage' call flow, which in turn is already handling
    // calling those two methods to correctly finalize a message if needed.

    err = StartCommandHeader(aParams);
    SuccessOrExit(err);

    statusBuilder = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().CreateStatusElementBuilder();
    statusBuilder.EncodeStatusElement(statusResponse.generalCode, statusResponse.protocolId.ToFullyQualifiedSpecForm(), statusResponse.protocolCode).EndOfStatusElement();

    err = statusBuilder.GetError();
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR InvokeResponder::StartCommandHeader(const CommandParams &aParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle txBuf;

    //
    // We only allocate the actual TX buffer once we have a command to add, which is now!
    //
    if (!mWriter.HasBuffer()) {
        CommandList::Builder commandListBuilder;

        txBuf = System::PacketBufferHandle::New(1024);
        VerifyOrExit(!txBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        mWriter.Init(std::move(txBuf));

        err = mInvokeCommandBuilder.Init(&mWriter);
        SuccessOrExit(err);

        commandListBuilder = mInvokeCommandBuilder.CreateCommandListBuilder();
        SuccessOrExit(commandListBuilder.GetError());
    }

    {
        CommandDataElement::Builder commandDataElement = mInvokeCommandBuilder.GetCommandListBuilder().CreateCommandDataElementBuilder();
        CommandPath::Builder commandPath = commandDataElement.CreateCommandPathBuilder();

        if (aParams.Flags.Has(CommandParams::TargetType::kTargetEndpoint))
        {
            commandPath.EndpointId(aParams.EndpointId);
        }
        else {
            commandPath.GroupId(aParams.GroupId);
        }

        commandPath.ClusterId(aParams.ClusterId).CommandId(aParams.CommandId).EndOfCommandPath();
        SuccessOrExit((err = commandPath.GetError()));
    }

exit:
    return err;
}

CHIP_ERROR InvokeResponder::Shutdown()
{
    if (mpExchangeCtx) {
#if 0
        // TODO: Not sure why the ExchangeManager assumes that we shouldn't close this exchange ourselves...
        mpExchangeCtx->Close();
#endif
        mpExchangeCtx = nullptr;
    }

    mState = kStateReleased;

    //DeviceLayer::PlatformMgr().ScheduleWork(InteractionModelEngine::FreeReleasedInvokeResponderObjects, (intptr_t)InteractionModelEngine::GetInstance());

    //
    // Call into our parent encapsulating entity to free ourselves up.
    // This may seem contrived, but fundamentally, our parent does not have sufficient knowledge to knwo when this object can be destroyed,
    // only because of the ability to support async. command responses. Otherwise, on completion of the call to Init, all work would have been
    // done and the parent entity would have destroyed this object.
    //
    InteractionModelEngine::FreeReleasedInvokeResponderObjects((intptr_t)InteractionModelEngine::GetInstance());
    return CHIP_NO_ERROR;
}

void InvokeResponder::IncrementHoldOffRef()
{
  assert(mHoldOffCount >= 0);
  mHoldOffCount++;
}

void InvokeResponder::DecrementHoldOffRef()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);

    assert(mHoldOffCount > 0);
    mHoldOffCount--;

    if (mHoldOffCount == 0) {
        if (mWriter.HasBuffer()) {
            System::PacketBufferHandle buf;

            err = FinalizeMessage(buf);
            SuccessOrExit(err);

            err = SendMessage(std::move(buf));
            SuccessOrExit(err);
        }

        Shutdown();
    }

exit:
    return;
}

// Only get here if we have a valid buffer
CHIP_ERROR InvokeResponder::FinalizeMessage(System::PacketBufferHandle &aBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);

    {
        System::PacketBufferHandle txBuf;
        CommandList::Builder commandListBuilder = mInvokeCommandBuilder.GetCommandListBuilder().EndOfCommandList();
        SuccessOrExit(commandListBuilder.GetError());

        mInvokeCommandBuilder.EndOfInvokeCommand();

        err = mInvokeCommandBuilder.GetError();
        SuccessOrExit(err);

        err = mWriter.Finalize(&txBuf);
        SuccessOrExit(err);

        VerifyOrExit(txBuf->EnsureReservedSize(System::PacketBuffer::kDefaultHeaderReserve),
                     err = CHIP_ERROR_BUFFER_TOO_SMALL);

        aBuf = std::move(txBuf);
    }

exit:
    return err;
}

CHIP_ERROR InvokeResponder::SendMessage(System::PacketBufferHandle aBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(!aBuf.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandRequest,
                                     std::move(aBuf), Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

exit:
    return err;
}

void InvokeInitiator::CloseExchange()
{
    if (mpExchangeCtx) {
        mpExchangeCtx->Close();
        mpExchangeCtx = nullptr;
    }
}

CHIP_ERROR InvokeInitiator::Init(Messaging::ExchangeManager *apExchangeMgr, ICommandHandler *aHandler, NodeId aNodeId, Transport::AdminId aAdminId, SecureSessionHandle * secureSession)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(apExchangeMgr != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    mpExchangeMgr = apExchangeMgr;
    mHandler = aHandler;

    CloseExchange();

    if (secureSession == nullptr) {
        mpExchangeCtx = mpExchangeMgr->NewContext({ aNodeId, 0, aAdminId }, this);
    }
    else {
        mpExchangeCtx = mpExchangeMgr->NewContext(*secureSession, this);
    }

    VerifyOrExit(mpExchangeCtx != nullptr, err = CHIP_ERROR_NO_MEMORY);

    mpExchangeCtx->SetResponseTimeout(kImMessageTimeoutMsec);

    mState = kStateReady;

exit:
    if (err != CHIP_NO_ERROR) {
        CloseExchange();
    }

    return err;
}

CHIP_ERROR InvokeInitiator::StartCommandHeader(const CommandParams &aParams)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle txBuf;

    VerifyOrExit(mState == kStateReady, err = CHIP_ERROR_INCORRECT_STATE);

    //
    // We only allocate the actual TX buffer once we have a command to add, which is now!
    //
    if (!mWriter.HasBuffer()) {
        CommandList::Builder commandListBuilder;

        txBuf = System::PacketBufferHandle::New(1024);
        VerifyOrExit(!txBuf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

        mWriter.Init(std::move(txBuf));

        err = mInvokeCommandBuilder.Init(&mWriter);
        SuccessOrExit(err);

        commandListBuilder = mInvokeCommandBuilder.CreateCommandListBuilder();
        SuccessOrExit(commandListBuilder.GetError());
    }

    {
        CommandDataElement::Builder commandDataElement = mInvokeCommandBuilder.GetCommandListBuilder().CreateCommandDataElementBuilder();
        CommandPath::Builder commandPath = commandDataElement.CreateCommandPathBuilder();

        if (aParams.Flags.Has(CommandParams::TargetType::kTargetEndpoint))
        {
            commandPath.EndpointId(aParams.EndpointId);
        }
        else {
            commandPath.GroupId(aParams.GroupId);
        }

        commandPath.ClusterId(aParams.ClusterId).CommandId(aParams.CommandId).EndOfCommandPath();
        SuccessOrExit((err = commandPath.GetError()));
    }

exit:
    return err;
}

CHIP_ERROR InvokeInitiator::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferTLVReader reader;
    TLV::TLVReader commandListReader;
    InvokeCommand::Parser invokeCommandParser;
    CommandList::Parser commandListParser;
    StatusElement::Parser statusElementParser;

    VerifyOrExit(mState == kStateAwaitingResponse, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mHandler != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // Now that we've received the response, move our state back to ready to permit adding commands
    mState = kStateReady;

    reader.Init(std::move(aPayload));

    err = reader.Next();
    SuccessOrExit(err);

    err = invokeCommandParser.Init(reader);
    SuccessOrExit(err);

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
    err = invokeCommandParser.CheckSchemaValidity();
    SuccessOrExit(err);
#endif

    err = invokeCommandParser.GetCommandList(&commandListParser);
    SuccessOrExit(err);

    commandListParser.GetReader(&commandListReader);

    while (CHIP_NO_ERROR == (err = commandListReader.Next()))
    {
        CommandParams params;
        CommandDataElement::Parser commandElement;
        CommandPath::Parser commandPath;
        TLV::TLVReader dataReader;
        TLV::TLVReader *pReader = &dataReader;
        //bool HasData = true;

        VerifyOrExit(chip::TLV::AnonymousTag == commandListReader.GetTag(), err = CHIP_ERROR_INVALID_TLV_TAG);
        VerifyOrExit(chip::TLV::kTLVType_Structure == commandListReader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

        err = commandElement.Init(commandListReader);
        SuccessOrExit(err);

        SuccessOrExit(commandElement.GetCommandPath(&commandPath));
        SuccessOrExit(commandPath.GetClusterId(&params.ClusterId));
        SuccessOrExit(commandPath.GetCommandId(&params.CommandId));
        SuccessOrExit(commandPath.GetEndpointId(&params.EndpointId));

        err = commandElement.GetStatusElement(&statusElementParser);
        if (err == CHIP_END_OF_TLV) {
            err = commandElement.GetData(pReader);
            SuccessOrExit(err);

            mHandler->OnResponse(*this, params, pReader);
        }
        else {
            StatusResponse statusResponse;
            uint32_t protocolId;

            err = statusElementParser.DecodeStatusElement(&statusResponse.generalCode, &protocolId, &statusResponse.protocolCode);
            SuccessOrExit(err);

            statusResponse.protocolId = Protocols::Id((chip::VendorId)((protocolId & 0xff00) >> 16), protocolId & 0xffff);

            if (!statusResponse.IsError()) {
                mHandler->OnResponse(*this, params, nullptr);
            }
            else {
                mHandler->OnError(*this, &params, CHIP_ERROR_STATUS_REPORT_RECEIVED, &statusResponse);
            }
        }

        SuccessOrExit(err);

        mState = kStateReady;
    }

    if (err == CHIP_END_OF_TLV) {
        err = CHIP_NO_ERROR;
    }

exit:
    CloseExchange();

   //
   // We're done with this object now. Let's signal the application to indicate the completion
   // of this interaction.
   //
   mHandler->OnEnd(*this);
   mHandler = nullptr;

   return err;
}

void InvokeInitiator::OnResponseTimeout(Messaging::ExchangeContext * apExchangeContext)
{
    if (mHandler) {
        mHandler->OnError(*this, NULL, CHIP_ERROR_TIMEOUT, nullptr);
        mHandler->OnEnd(*this);
        mHandler = nullptr;
    }
}

CHIP_ERROR InvokeInitiator::AddRequest(CommandParams aParams, IEncodableElement *serializable)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    //
    // Update our accumulated flag that tracks if any command going into this invoke
    // expects responses
    //
    mExpectsResponses |= aParams.ExpectsResponses;

    err = StartCommandHeader(aParams);
    SuccessOrExit(err);

    //
    // Invoke the passed in closure that will actually write out the command payload if any
    //
    err = serializable->Encode(*mInvokeCommandBuilder.GetWriter(), TLV::ContextTag(CommandDataElement::kCsTag_Data));
    SuccessOrExit(err);

    mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().EndOfCommandDataElement();
    SuccessOrExit((err = mInvokeCommandBuilder.GetCommandListBuilder().GetCommandDataElementBuidler().GetError()));

exit:
    if (err != CHIP_NO_ERROR && mHandler) {
        mHandler->OnEnd(*this);
        mHandler = nullptr;
    }

    return err;
}

CHIP_ERROR InvokeInitiator::AddRequestAndSend(CommandParams aParams, IEncodableElement *serializable)
{
    CHIP_ERROR err;

    err = AddRequest(aParams, serializable);
    SuccessOrExit(err);

    err = Send();
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR && mHandler) {
        mHandler->OnEnd(*this);
        mHandler = nullptr;
    }

    return err;
}

CHIP_ERROR InvokeInitiator::Send()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mState == kStateReady, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mWriter.HasBuffer(), );

    {
        System::PacketBufferHandle buf;

        err = FinalizeMessage(buf);
        SuccessOrExit(err);

        err = SendMessage(std::move(buf));
        SuccessOrExit(err);

        mState = kStateAwaitingResponse;
    }

exit:
    if (err != CHIP_NO_ERROR && mHandler) {
        mHandler->OnEnd(*this);
        mHandler = nullptr;
    }

    return err;
}

// Only get here if we have a valid buffer
CHIP_ERROR InvokeInitiator::FinalizeMessage(System::PacketBufferHandle &aBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mpExchangeCtx, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(mState != kStateAwaitingResponse, err = CHIP_ERROR_INCORRECT_STATE);

    {
        System::PacketBufferHandle txBuf;
        CommandList::Builder commandListBuilder = mInvokeCommandBuilder.GetCommandListBuilder().EndOfCommandList();
        SuccessOrExit(commandListBuilder.GetError());

        mInvokeCommandBuilder.EndOfInvokeCommand();

        err = mInvokeCommandBuilder.GetError();
        SuccessOrExit(err);

        err = mWriter.Finalize(&txBuf);
        SuccessOrExit(err);

        VerifyOrExit(txBuf->EnsureReservedSize(System::PacketBuffer::kDefaultHeaderReserve),
                     err = CHIP_ERROR_BUFFER_TOO_SMALL);

        aBuf = std::move(txBuf);
    }

exit:
    return err;
}

CHIP_ERROR InvokeInitiator::SendMessage(System::PacketBufferHandle aBuf)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(!aBuf.IsNull(), err = CHIP_ERROR_INVALID_ARGUMENT);

    err = mpExchangeCtx->SendMessage(Protocols::InteractionModel::MsgType::InvokeCommandRequest,
                                     std::move(aBuf), Messaging::SendFlags(Messaging::SendMessageFlags::kExpectResponse));
    SuccessOrExit(err);

exit:
    return err;
}

}
}
