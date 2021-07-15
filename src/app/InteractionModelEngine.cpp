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
    mpExchangeMgr = apExchangeMgr;
    mpDelegate    = apDelegate;

    ReturnErrorOnFailure(mpExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, this));

    mReportingEngine.Init();

    for (uint32_t index = 0; index < IM_SERVER_MAX_NUM_PATH_GROUPS - 1; index++)
    {
        mClusterInfoPool[index].mpNext = &mClusterInfoPool[index + 1];
    }
    mClusterInfoPool[IM_SERVER_MAX_NUM_PATH_GROUPS - 1].mpNext = nullptr;
    mpNextAvailableClusterInfo                                 = mClusterInfoPool;

    return CHIP_NO_ERROR;
}

void InteractionModelEngine::Shutdown()
{
    for (auto & commandSender : mCommandSenderObjs)
    {
        if (!commandSender.IsFree())
        {
            commandSender.Shutdown();
        }
    }

    for (auto & commandHandler : mCommandHandlerObjs)
    {
        if (!commandHandler.IsFree())
        {
            commandHandler.Shutdown();
        }
    }

    for (auto & readClient : mReadClients)
    {
        if (!readClient.IsFree())
        {
            readClient.Shutdown();
        }
    }

    for (auto & readHandler : mReadHandlers)
    {
        if (!readHandler.IsFree())
        {
            readHandler.Shutdown();
        }
    }

    for (auto & writeClient : mWriteClients)
    {
        if (!writeClient.IsFree())
        {
            writeClient.Shutdown();
        }
    }

    for (auto & writeHandler : mWriteHandlers)
    {
        if (!writeHandler.IsFree())
        {
            writeHandler.Shutdown();
        }
    }

    for (uint32_t index = 0; index < IM_SERVER_MAX_NUM_PATH_GROUPS; index++)
    {
        mClusterInfoPool[index].mpNext = nullptr;
        mClusterInfoPool[index].ClearDirty();
    }

    mpNextAvailableClusterInfo = nullptr;

    mpExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id);
}

CHIP_ERROR InteractionModelEngine::NewCommandSender(CommandSender ** const apCommandSender)
{
    *apCommandSender = nullptr;

    for (auto & commandSender : mCommandSenderObjs)
    {
        if (commandSender.IsFree())
        {
            const CHIP_ERROR err = commandSender.Init(mpExchangeMgr, mpDelegate);
            if (err == CHIP_NO_ERROR)
            {
                *apCommandSender = &commandSender;
            }
            return err;
        }
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR InteractionModelEngine::NewReadClient(ReadClient ** const apReadClient, intptr_t aAppIdentifier)
{
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

    for (auto & readClient : mReadClients)
    {
        if (readClient.IsFree())
        {
            *apReadClient = &readClient;
            err           = readClient.Init(mpExchangeMgr, mpDelegate, aAppIdentifier);
            if (CHIP_NO_ERROR != err)
            {
                *apReadClient = nullptr;
            }
            return err;
        }
    }

    return err;
}

CHIP_ERROR InteractionModelEngine::NewWriteClient(WriteClient ** const apWriteClient)
{
    *apWriteClient = nullptr;

    for (auto & writeClient : mWriteClients)
    {
        if (!writeClient.IsFree())
        {
            continue;
        }

        ReturnErrorOnFailure(writeClient.Init(mpExchangeMgr, mpDelegate));
        *apWriteClient = &writeClient;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR InteractionModelEngine::OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext,
                                                    const PacketHeader & aPacketHeader, const PayloadHeader & aPayloadHeader,
                                                    System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DataManagement, "Msg type %d not supported", aPayloadHeader.GetMessageType());

    // Todo: Add status report
    // err = SendStatusReport(ec, kChipProfile_Common, kStatus_UnsupportedMessage);
    // SuccessOrExit(err);

    apExchangeContext = nullptr;

    ChipLogFunctError(err);

    // Todo: Fix the below check after the above status report is implemented.
    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
    }
    return err;
}

CHIP_ERROR InteractionModelEngine::OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext,
                                                          const PacketHeader & aPacketHeader, const PayloadHeader & aPayloadHeader,
                                                          System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    for (auto & commandHandler : mCommandHandlerObjs)
    {
        if (commandHandler.IsFree())
        {
            err = commandHandler.Init(mpExchangeMgr, mpDelegate);
            SuccessOrExit(err);
            err = commandHandler.OnInvokeCommandRequest(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
            apExchangeContext = nullptr;
            break;
        }
    }

exit:
    ChipLogFunctError(err);

    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
    }
    return err;
}

CHIP_ERROR InteractionModelEngine::OnReadRequest(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                                 const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DataManagement, "Receive Read request");

    for (auto & readHandler : mReadHandlers)
    {
        if (readHandler.IsFree())
        {
            err = readHandler.Init(mpDelegate);
            SuccessOrExit(err);
            err               = readHandler.OnReadRequest(apExchangeContext, std::move(aPayload));
            apExchangeContext = nullptr;
            break;
        }
    }

exit:
    ChipLogFunctError(err);

    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
    }
    return err;
}

CHIP_ERROR InteractionModelEngine::OnWriteRequest(Messaging::ExchangeContext * apExchangeContext,
                                                  const PacketHeader & aPacketHeader, const PayloadHeader & aPayloadHeader,
                                                  System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DataManagement, "Receive Write request");

    for (auto & writeHandler : mWriteHandlers)
    {
        if (writeHandler.IsFree())
        {
            err = writeHandler.Init(mpDelegate);
            SuccessOrExit(err);
            err               = writeHandler.OnWriteRequest(apExchangeContext, std::move(aPayload));
            apExchangeContext = nullptr;
            break;
        }
    }

exit:
    ChipLogFunctError(err);

    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
    }
    return err;
}

CHIP_ERROR InteractionModelEngine::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext,
                                                     const PacketHeader & aPacketHeader, const PayloadHeader & aPayloadHeader,
                                                     System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest))
    {
        err = OnInvokeCommandRequest(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReadRequest))
    {
        err = OnReadRequest(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::WriteRequest))
    {
        err = OnWriteRequest(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
    else
    {
        err = OnUnknownMsgType(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
    }
    return err;
}

void InteractionModelEngine::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(DataManagement, "Time out! failed to receive echo response from Exchange: %d", ec->GetExchangeId());
}

CHIP_ERROR InteractionModelEngine::SendReadRequest(NodeId aNodeId, Transport::AdminId aAdminId,
                                                   SecureSessionHandle * apSecureSession, EventPathParams * apEventPathParamsList,
                                                   size_t aEventPathParamsListSize, AttributePathParams * apAttributePathParamsList,
                                                   size_t aAttributePathParamsListSize, EventNumber aEventNumber,
                                                   intptr_t aAppIdentifier)
{
    ReadClient * client = nullptr;
    CHIP_ERROR err      = CHIP_NO_ERROR;
    ReturnErrorOnFailure(NewReadClient(&client, aAppIdentifier));
    err = client->SendReadRequest(aNodeId, aAdminId, apSecureSession, apEventPathParamsList, aEventPathParamsListSize,
                                  apAttributePathParamsList, aAttributePathParamsListSize, aEventNumber);
    if (err != CHIP_NO_ERROR)
    {
        client->Shutdown();
    }
    return err;
}

CHIP_ERROR __attribute__((weak))
WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler * apWriteHandler)
{
    ChipLogDetail(DataManagement,
                  "Received Cluster Attribute: Cluster=%" PRIx32 " NodeId=0x" ChipLogFormatX64 " Endpoint=%" PRIx16
                  " FieldId=%" PRIx32 " ListIndex=%" PRIx16,
                  aClusterInfo.mClusterId, ChipLogValueX64(aClusterInfo.mNodeId), aClusterInfo.mEndpointId, aClusterInfo.mFieldId,
                  aClusterInfo.mListIndex);
    ChipLogError(DataManagement,
                 "Default WriteSingleClusterData is called, this should be replaced by actual dispatched for cluster");
    return CHIP_NO_ERROR;
}

uint16_t InteractionModelEngine::GetReadClientArrayIndex(const ReadClient * const apReadClient) const
{
    return static_cast<uint16_t>(apReadClient - mReadClients);
}

uint16_t InteractionModelEngine::GetWriteClientArrayIndex(const WriteClient * const apWriteClient) const
{
    return static_cast<uint16_t>(apWriteClient - mWriteClients);
}

void InteractionModelEngine::ReleaseClusterInfoList(ClusterInfo *& aClusterInfo)
{
    ClusterInfo * lastClusterInfo = aClusterInfo;
    if (lastClusterInfo == nullptr)
    {
        return;
    }

    while (lastClusterInfo != nullptr && lastClusterInfo->mpNext != nullptr)
    {
        lastClusterInfo->ClearDirty();
        lastClusterInfo = lastClusterInfo->mpNext;
    }
    lastClusterInfo->ClearDirty();
    lastClusterInfo->mFlags.ClearAll();
    lastClusterInfo->mpNext    = mpNextAvailableClusterInfo;
    mpNextAvailableClusterInfo = aClusterInfo;
    aClusterInfo               = nullptr;
}

CHIP_ERROR InteractionModelEngine::PushFront(ClusterInfo *& aClusterInfoList, ClusterInfo & aClusterInfo)
{
    ClusterInfo * last = aClusterInfoList;
    if (mpNextAvailableClusterInfo == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    aClusterInfoList           = mpNextAvailableClusterInfo;
    mpNextAvailableClusterInfo = mpNextAvailableClusterInfo->mpNext;
    *aClusterInfoList          = aClusterInfo;
    aClusterInfoList->mpNext   = last;
    return CHIP_NO_ERROR;
}

} // namespace app
} // namespace chip
