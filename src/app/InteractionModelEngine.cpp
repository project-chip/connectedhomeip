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
 *      This file defines objects for a CHIP Interaction Data model Engine which handle unsolicitied IM message, and
 *      manage different kinds of IM client and handlers.
 *
 */

#include "InteractionModelEngine.h"
#include "Command.h"
#include "CommandHandler.h"
#include "CommandSender.h"
#include <cinttypes>

namespace chip {
namespace app {
InteractionModelEngine sInteractionModelEngine;

InteractionModelEngine::InteractionModelEngine() {}

InteractionModelEngine * InteractionModelEngine::GetInstance()
{
    return &sInteractionModelEngine;
}

CHIP_ERROR InteractionModelEngine::Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mpExchangeMgr  = apExchangeMgr;
    mpDelegate = apDelegate;

    err = mpExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::kProtocol_InteractionModel, this);
    SuccessOrExit(err);

    mReportingEngine.Init();
    SuccessOrExit(err);

exit:
    return err;
}

void InteractionModelEngine::Shutdown()
{
    for (size_t i = 0; i < CHIP_MAX_NUM_COMMAND_HANDLER_OBJECTS; ++i)
    {
        mCommandHandlerObjs[i].Shutdown();
    }

    for (size_t i = 0; i < CHIP_MAX_NUM_COMMAND_SENDER_OBJECTS; ++i)
    {
        mCommandSenderObjs[i].Shutdown();
    }

    for (size_t i = 0; i < CHIP_MAX_NUM_READ_HANDLER; ++i)
    {
        mReadHandlers[i].Shutdown();
    }

    for (size_t i = 0; i < CHIP_MAX_NUM_READ_CLIENT; ++i)
    {
        mReadClients[i].Shutdown();
    }
}

CHIP_ERROR InteractionModelEngine::NewCommandSender(CommandSender ** const apCommandSender, InteractionModelDelegate * apDelegate)
{
    CHIP_ERROR err  = CHIP_ERROR_NO_MEMORY;
    *apCommandSender = nullptr;

    for (size_t i = 0; i < CHIP_MAX_NUM_COMMAND_SENDER_OBJECTS; ++i)
    {
        if (mCommandHandlerObjs[i].IsFree())
        {
            *apCommandSender = &mCommandSenderObjs[i];
            err             = mCommandSenderObjs[i].Init(mpExchangeMgr, apDelegate);
            SuccessOrExit(err);
            if (CHIP_NO_ERROR != err)
            {
                *apCommandSender = nullptr;
                ExitNow();
            }
            break;
        }
    }

exit:
    return err;
}

CHIP_ERROR InteractionModelEngine::NewReadClient(ReadClient ** const apReadClient, InteractionModelDelegate * apDelegate)
{
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;
    *apReadClient  = nullptr;

    for (size_t i = 0; i < CHIP_MAX_NUM_READ_CLIENT; ++i)
    {
        if (mReadClients[i].IsFree())
        {
            *apReadClient = &mReadClients[i];
            err           = mReadClients[i].Init(mpExchangeMgr, apDelegate);
            if (CHIP_NO_ERROR != err)
            {
                *apReadClient = nullptr;
                return err;
            }
            break;
        }
    }

    return err;
}

void InteractionModelEngine::OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                              const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DataManagement, "Msg type %d not supported", aPayloadHeader.GetMessageType());

    // Todo: Add status report
    // err = SendStatusReport(ec, kChipProfile_Common, kStatus_UnsupportedMessage);
    // SuccessOrExit(err);

    apExchangeContext->Close();
    apExchangeContext = NULL;

    ChipLogFunctError(err);

    if (NULL != apExchangeContext)
    {
        apExchangeContext->Abort();
        apExchangeContext = NULL;
    }
}

void InteractionModelEngine::OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                                    const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    CommandHandler * commandServer = nullptr;

    if (nullptr != mpDelegate)
    {
        chip::app::InteractionModelDelegate::InEventParam inParam;
        chip::app::InteractionModelDelegate::OutEventParam outParam;
        outParam.mIncomingInvokeCommandRequest.invokeCommandAllowed = true;
        inParam.mIncomingInvokeCommandRequest.packetHeader             = &aPacketHeader;

        mpDelegate->HandleEvent(app::InteractionModelDelegate::EventId::kIncomingInvokeCommandRequest, inParam, outParam);

        if (!outParam.mIncomingInvokeCommandRequest.invokeCommandAllowed)
        {
            ChipLogDetail(DataManagement, "Command not allowed");
            ExitNow();
        }
    }

    for (size_t i = 0; i < CHIP_MAX_NUM_COMMAND_HANDLER_OBJECTS; ++i)
    {
        if (mCommandHandlerObjs[i].IsFree())
        {
            commandServer = &mCommandHandlerObjs[i];
            err           = commandServer->Init(mpExchangeMgr, mpDelegate);
            SuccessOrExit(err);
            commandServer->OnMessageReceived(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
            apExchangeContext = nullptr;
            break;
        }
    }

exit:
    ChipLogFunctError(err);

    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
        apExchangeContext = NULL;
    }
}

void InteractionModelEngine::OnReadRequest(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    ReadHandler * readHandler = nullptr;

    ChipLogDetail(DataManagement, "Receive Read request");

    if (nullptr != mpDelegate)
    {
        chip::app::InteractionModelDelegate::InEventParam inParam;
        chip::app::InteractionModelDelegate::OutEventParam outParam;
        outParam.mIncomingReadRequest.readRequestAllowed = true;
        inParam.mIncomingReadRequest.packetHeader             = &aPacketHeader;

        mpDelegate->HandleEvent(app::InteractionModelDelegate::EventId::kIncomingReadRequest, inParam, outParam);

        if (!outParam.mIncomingReadRequest.readRequestAllowed)
        {
            ChipLogDetail(DataManagement, "Read request not allowed");
            ExitNow();
        }
    }

    for (size_t i = 0; i < CHIP_MAX_NUM_READ_HANDLER; ++i)
    {
        if (mReadHandlers[i].IsFree())
        {
            readHandler = &mReadHandlers[i];
            err         = readHandler->Init(mpExchangeMgr, mpDelegate);
            SuccessOrExit(err);
            readHandler->OnMessageReceived(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
            apExchangeContext = nullptr;
            break;
        }
    }

exit:
    ChipLogFunctError(err);

    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
        apExchangeContext = NULL;
    }
}

void InteractionModelEngine::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                               const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest))
    {

        OnInvokeCommandRequest(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReadRequest))
    {
        OnReadRequest(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
    else
    {
        OnUnknownMsgType(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
}

void InteractionModelEngine::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive echo response from Exchange: %d", ec->GetExchangeId());
}

// The default implementation to make compiler happy before codegen for this is ready.
// TODO: Remove this after codegen is ready.
void __attribute__((weak))
DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                             chip::TLV::TLVReader & aReader, Command * apCommandObj)
{
    ChipLogDetail(DataManagement, "Received Cluster Command: Cluster=%" PRIx16 " Command=%" PRIx8 " Endpoint=%" PRIx8, aClusterId,
                  aCommandId, aEndPointId);
    ChipLogError(
        DataManagement,
        "Default DispatchSingleClusterCommand is called, this should be replaced by actual dispatched for cluster commands");
}

uint16_t InteractionModelEngine::GetReadClientArrayIndex(const ReadClient * const apClient) const
{
    return static_cast<uint16_t>(apClient - mReadClients);
}
} // namespace app
} // namespace chip
