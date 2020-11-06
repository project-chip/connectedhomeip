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

#include "InteractionModelEngine.h"
#include "Command.h"
#include "CommandHandler.h"
#include "CommandSender.h"

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

    err = mpExchangeMgr->RegisterUnsolicitedMessageHandler(Protocols::kProtocol_InteractionModel, this);
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
    mHandlersMap.clear();
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
                                              uint32_t aProtocolId, uint8_t aMsgType, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DataManagement, "Msg type %d not supported", (int) aMsgType);

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
                                                    uint32_t aProtocolId, uint8_t aMsgType, System::PacketBufferHandle aPayload)
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
            commandServer->OnMessageReceived(apEc, aPacketHeader, aProtocolId, aMsgType, std::move(aPayload));
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
                                               uint32_t aProtocolId, uint8_t aMsgType, System::PacketBufferHandle aPayload)
{
    switch (aMsgType)
    {
    case kMsgType_InvokeCommandRequest:
        OnInvokeCommandRequest(apEc, aPacketHeader, aProtocolId, aMsgType, std::move(aPayload));
        break;
    default:
        OnUnknownMsgType(apEc, aPacketHeader, aProtocolId, aMsgType, std::move(aPayload));
        break;
    }
}

void InteractionModelEngine::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive echo response from Exchange: %d", ec->GetExchangeId());
}

InteractionModelEngine::HandlerKey::HandlerKey(chip::ClusterId aClusterId, chip::CommandId aCommandId,
                                               Command::CommandRoleId aCommandRoleId) :
    mClusterId(aClusterId),
    mCommandId(aCommandId), mCommandRoleId(aCommandRoleId)
{}

inline bool InteractionModelEngine::HandlerKey::operator<(const HandlerKey & aOtherKey) const
{
    return ((mClusterId != aOtherKey.mClusterId)
                ? (mClusterId < aOtherKey.mClusterId)
                : ((mCommandId != aOtherKey.mCommandId) ? (mCommandId < aOtherKey.mCommandId)
                                                        : ((mCommandRoleId < aOtherKey.mCommandRoleId))));
}

CHIP_ERROR InteractionModelEngine::RegisterClusterCommandHandler(chip::ClusterId aClusterId, chip::CommandId aCommandId,
                                                                 Command::CommandRoleId aCommandRoleId, CommandCbFunct aDispatcher)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    std::pair<HandlersMapType::iterator, bool> insertResult =
        mHandlersMap.insert(HandlersMapType::value_type(HandlerKey(aClusterId, aCommandId, aCommandRoleId), aDispatcher));

    if (!insertResult.second)
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        ChipLogDetail(DataManagement,
                      "RegisterClusterCommandHandler registered handler for ClusterId = %d, CommandId = %d, CommandRoleId = "
                      "%d, Dispatcher = %p",
                      aClusterId, aCommandId, aCommandRoleId, (void *) aDispatcher);
    }

    return err;
}

CHIP_ERROR InteractionModelEngine::DeregisterClusterCommandHandler(chip::ClusterId aClusterId, chip::CommandId aCommandId,
                                                                   Command::CommandRoleId aCommandRoleId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    HandlersMapType::iterator handlerIt = mHandlersMap.find(HandlerKey(aClusterId, aCommandId, aCommandRoleId));
    if (handlerIt != mHandlersMap.end())
    {
        CommandCbFunct pDispatcher = handlerIt->second;

        ChipLogDetail(DataManagement,
                      "DeregisterClusterCommandHandler unregistered handler for ClusterId = %d, CommandId = %d, CommandRoleId "
                      "= %d, Dispatcher = %p",
                      aClusterId, aCommandId, aCommandRoleId, (void *) pDispatcher);
        mHandlersMap.erase(handlerIt);
    }
    else
    {
        err = CHIP_ERROR_INVALID_ARGUMENT;
    }

    return err;
}

void InteractionModelEngine::ProcessCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::TLV::TLVReader & aReader,
                                            Command * apCommandObj, Command::CommandRoleId aCommandRoleId)
{
    HandlersMapType::iterator handlerIt;

    handlerIt = mHandlersMap.find(HandlerKey(aClusterId, aCommandId, aCommandRoleId));

    ChipLogDetail(DataManagement, "ClusterId = %d, CommandId = %d, CommandRoleId = %d", (int) (aClusterId), (int) (aCommandId),
                  aCommandRoleId);

    if (handlerIt != mHandlersMap.end())
    {
        handlerIt->second(aReader, apCommandObj);
    }

    return;
}
} // namespace app
} // namespace chip
