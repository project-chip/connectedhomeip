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
 *      This file defines objects for a CHIP Interaction Data model Engine which handle unsolicitied IM message, and
 *      manage different kinds of IM client and handlers.
 *
 */

#include <cinttypes>

#include "Command.h"
#include "CommandHandler.h"
#include "CommandSender.h"
#include "InteractionModelEngine.h"

namespace chip {
namespace app {
InteractionModelEngine sInteractionModelEngine;

InteractionModelEngine::InteractionModelEngine() {}

InteractionModelEngine * InteractionModelEngine::GetInstance()
{
    return &sInteractionModelEngine;
}

void InteractionModelEngine::SetEventCallback(void * apAppState, EventCallback aEventCallback)
{
    mpAppState     = apAppState;
    mEventCallback = aEventCallback;
}

void InteractionModelEngine::DefaultEventHandler(EventID aEvent, const InEventParam & aInParam, OutEventParam & aOutParam)
{
    IgnoreUnusedVariable(aInParam);
    IgnoreUnusedVariable(aOutParam);

    ChipLogDetail(DataManagement, "%s event: %d", __func__, aEvent);
}

CHIP_ERROR InteractionModelEngine::Init(Messaging::ExchangeManager * apExchangeMgr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Error if already initialized.
    if (mpExchangeMgr != nullptr)
        return CHIP_ERROR_INCORRECT_STATE;

    mpExchangeMgr = apExchangeMgr;

    err = mpExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::kProtocol_InteractionModel, this);
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
}

CHIP_ERROR InteractionModelEngine::NewCommandSender(CommandSender ** const apComandSender)
{
    CHIP_ERROR err  = CHIP_ERROR_NO_MEMORY;
    *apComandSender = nullptr;

    for (size_t i = 0; i < CHIP_MAX_NUM_COMMAND_SENDER_OBJECTS; ++i)
    {
        if (mCommandHandlerObjs[i].IsFree())
        {
            *apComandSender = &mCommandSenderObjs[i];
            err             = mCommandSenderObjs[i].Init(mpExchangeMgr);
            SuccessOrExit(err);
            if (CHIP_NO_ERROR != err)
            {
                *apComandSender = nullptr;
                ExitNow();
            }
            break;
        }
    }

exit:
    return err;
}

void InteractionModelEngine::OnUnknownMsgType(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader,
                                              const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DataManagement, "Msg type %d not supported", aPayloadHeader.GetMessageType());

    // Todo: Add status report
    // err = SendStatusReport(ec, kChipProfile_Common, kStatus_UnsupportedMessage);
    // SuccessOrExit(err);

    apEc->Close();
    apEc = NULL;

    ChipLogFunctError(err);

    if (NULL != apEc)
    {
        apEc->Abort();
        apEc = NULL;
    }
}

void InteractionModelEngine::OnInvokeCommandRequest(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader,
                                                    const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    CommandHandler * commandServer = nullptr;

    if (nullptr != mEventCallback)
    {
        InEventParam inParam;
        OutEventParam outParam;
        inParam.Clear();
        outParam.Clear();
        outParam.mIncomingInvokeCommandRequest.mShouldContinueProcessing = true;
        inParam.mIncomingInvokeCommandRequest.mpPacketHeader             = &aPacketHeader;

        mEventCallback(mpAppState, kEvent_OnIncomingInvokeCommandRequest, inParam, outParam);

        if (!outParam.mIncomingInvokeCommandRequest.mShouldContinueProcessing)
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
            err           = commandServer->Init(mpExchangeMgr);
            SuccessOrExit(err);
            commandServer->OnMessageReceived(apEc, aPacketHeader, aPayloadHeader, std::move(aPayload));
            apEc = nullptr;
            break;
        }
    }

exit:
    ChipLogFunctError(err);

    if (nullptr != apEc)
    {
        apEc->Abort();
        apEc = NULL;
    }
}

void InteractionModelEngine::OnMessageReceived(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader,
                                               const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest))
    {
        OnInvokeCommandRequest(apEc, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
    else
    {
        OnUnknownMsgType(apEc, aPacketHeader, aPayloadHeader, std::move(aPayload));
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

} // namespace app
} // namespace chip
