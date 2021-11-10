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

    for (uint32_t index = 0; index < CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS - 1; index++)
    {
        mClusterInfoPool[index].mpNext = &mClusterInfoPool[index + 1];
    }
    mClusterInfoPool[CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS - 1].mpNext = nullptr;
    mpNextAvailableClusterInfo                                      = mClusterInfoPool;

    return CHIP_NO_ERROR;
}

void InteractionModelEngine::Shutdown()
{
    CommandHandlerInterface * handlerIter = mCommandHandlerList;

    //
    // Walk our list of command handlers and de-register them, before finally
    // nulling out the list entirely.
    //
    while (handlerIter)
    {
        CommandHandlerInterface * next = handlerIter->GetNext();
        handlerIter->SetNext(nullptr);
        handlerIter = next;
    }

    mCommandHandlerList = nullptr;

    //
    // Since modifying the pool during iteration is generally frowned upon,
    // I've chosen to just destroy the object but not necessarily de-allocate it.
    //
    // This poses a problem when shutting down and restarting the stack, since the
    // IMEngine is a statically constructed singleton, and this lingering state will
    // cause issues.
    //
    // This doesn't pose a problem right now because there shouldn't be any actual objects
    // left here due to the synchronous nature of command handling.
    //
    // Filed #10332 to track this.
    //
    mCommandHandlerObjs.ForEachActiveObject([](CommandHandler * obj) -> bool {
        obj->~CommandHandler();
        return true;
    });

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
        VerifyOrDie(writeHandler.IsFree());
    }

    mReportingEngine.Shutdown();

    for (uint32_t index = 0; index < CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS; index++)
    {
        mClusterInfoPool[index].mpNext = nullptr;
    }

    mpNextAvailableClusterInfo = nullptr;

    mpExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id);
}

CHIP_ERROR InteractionModelEngine::NewReadClient(ReadClient ** const apReadClient, ReadClient::InteractionType aInteractionType,
                                                 ReadClient::Callback * aCallback)
{
    *apReadClient = nullptr;

    for (auto & readClient : mReadClients)
    {
        if (readClient.IsFree())
        {
            CHIP_ERROR err;

            *apReadClient = &readClient;
            err           = readClient.Init(mpExchangeMgr, aCallback, aInteractionType);
            if (CHIP_NO_ERROR != err)
            {
                *apReadClient = nullptr;
            }
            return err;
        }
    }

    return CHIP_ERROR_NO_MEMORY;
}

uint32_t InteractionModelEngine::GetNumActiveReadClients() const
{
    uint32_t numActive = 0;

    for (auto & readClient : mReadClients)
    {
        if (!readClient.IsFree())
        {
            numActive++;
        }
    }

    return numActive;
}

uint32_t InteractionModelEngine::GetNumActiveReadHandlers() const
{
    uint32_t numActive = 0;

    for (auto & readHandler : mReadHandlers)
    {
        if (!readHandler.IsFree())
        {
            numActive++;
        }
    }

    return numActive;
}

uint32_t InteractionModelEngine::GetNumActiveWriteClients() const
{
    uint32_t numActive = 0;

    for (auto & writeClient : mWriteClients)
    {
        if (!writeClient.IsFree())
        {
            numActive++;
        }
    }

    return numActive;
}

uint32_t InteractionModelEngine::GetNumActiveWriteHandlers() const
{
    uint32_t numActive = 0;

    for (auto & writeHandler : mWriteHandlers)
    {
        if (!writeHandler.IsFree())
        {
            numActive++;
        }
    }

    return numActive;
}

CHIP_ERROR InteractionModelEngine::ShutdownSubscription(uint64_t aSubscriptionId)
{
    CHIP_ERROR err = CHIP_ERROR_KEY_NOT_FOUND;

    for (auto & readClient : mReadClients)
    {
        if (!readClient.IsFree() && readClient.IsSubscriptionType() && readClient.IsMatchingClient(aSubscriptionId))
        {
            readClient.Shutdown();
            err = CHIP_NO_ERROR;
        }
    }

    return err;
}

CHIP_ERROR InteractionModelEngine::ShutdownSubscriptions(FabricIndex aFabricIndex, NodeId aPeerNodeId)
{
    CHIP_ERROR err = CHIP_ERROR_KEY_NOT_FOUND;

    for (ReadClient & readClient : mReadClients)
    {
        if (!readClient.IsFree() && readClient.IsSubscriptionType() && readClient.GetFabricIndex() == aFabricIndex &&
            readClient.GetPeerNodeId() == aPeerNodeId)
        {
            readClient.Shutdown();
            err = CHIP_NO_ERROR;
        }
    }

    return err;
}

CHIP_ERROR InteractionModelEngine::NewWriteClient(WriteClientHandle & apWriteClient, WriteClient::Callback * apCallback)
{
    apWriteClient.SetWriteClient(nullptr);

    for (auto & writeClient : mWriteClients)
    {
        if (!writeClient.IsFree())
        {
            continue;
        }

        ReturnLogErrorOnFailure(writeClient.Init(mpExchangeMgr, apCallback));
        apWriteClient.SetWriteClient(&writeClient);
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MEMORY;
}

CHIP_ERROR InteractionModelEngine::OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext,
                                                    const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(InteractionModel, "Msg type %d not supported", aPayloadHeader.GetMessageType());

    // Todo: Add status report
    // err = SendStatusReport(ec, kChipProfile_Common, kStatus_UnsupportedMessage);
    // SuccessOrExit(err);

    apExchangeContext = nullptr;

    // Todo: Fix the below check after the above status report is implemented.
    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
    }
    return err;
}

void InteractionModelEngine::OnDone(CommandHandler & apCommandObj)
{
    mCommandHandlerObjs.ReleaseObject(&apCommandObj);
}

CHIP_ERROR InteractionModelEngine::OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext,
                                                          const PayloadHeader & aPayloadHeader,
                                                          System::PacketBufferHandle && aPayload)
{
    CommandHandler * commandHandler = mCommandHandlerObjs.CreateObject(this);
    if (commandHandler == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    return commandHandler->OnInvokeCommandRequest(apExchangeContext, aPayloadHeader, std::move(aPayload));
}

CHIP_ERROR InteractionModelEngine::OnReadInitialRequest(Messaging::ExchangeContext * apExchangeContext,
                                                        const PayloadHeader & aPayloadHeader,
                                                        System::PacketBufferHandle && aPayload,
                                                        ReadHandler::InteractionType aInteractionType)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(InteractionModel, "Received %s request",
                  aInteractionType == ReadHandler::InteractionType::Subscribe ? "Subscribe" : "Read");

    for (auto & readHandler : mReadHandlers)
    {
        if (!readHandler.IsFree() && readHandler.IsSubscriptionType() &&
            readHandler.GetInitiatorNodeId() == apExchangeContext->GetSessionHandle().GetPeerNodeId() &&
            readHandler.GetFabricIndex() == apExchangeContext->GetSessionHandle().GetFabricIndex())
        {
            bool keepSubscriptions = true;
            System::PacketBufferTLVReader reader;
            reader.Init(aPayload.Retain());
            SuccessOrExit(err = reader.Next());
            SubscribeRequestMessage::Parser subscribeRequestParser;
            SuccessOrExit(err = subscribeRequestParser.Init(reader));
            err = subscribeRequestParser.GetKeepSubscriptions(&keepSubscriptions);
            if (err == CHIP_NO_ERROR && !keepSubscriptions)
            {
                readHandler.Shutdown(ReadHandler::ShutdownOptions::AbortCurrentExchange);
            }
        }
    }

    for (auto & readHandler : mReadHandlers)
    {
        if (readHandler.IsFree())
        {
            err = readHandler.Init(mpExchangeMgr, mpDelegate, apExchangeContext, aInteractionType);
            SuccessOrExit(err);
            err               = readHandler.OnReadInitialRequest(std::move(aPayload));
            apExchangeContext = nullptr;
            break;
        }
    }

exit:

    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
    }
    return err;
}

CHIP_ERROR InteractionModelEngine::OnWriteRequest(Messaging::ExchangeContext * apExchangeContext,
                                                  const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(InteractionModel, "Received Write request");

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

    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
    }
    return err;
}

CHIP_ERROR InteractionModelEngine::OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext,
                                                           const PayloadHeader & aPayloadHeader,
                                                           System::PacketBufferHandle && aPayload)
{
    System::PacketBufferTLVReader reader;
    reader.Init(aPayload.Retain());
    ReturnLogErrorOnFailure(reader.Next());

    ReportDataMessage::Parser report;
    ReturnLogErrorOnFailure(report.Init(reader));

    uint64_t subscriptionId = 0;
    ReturnLogErrorOnFailure(report.GetSubscriptionId(&subscriptionId));

    for (auto & readClient : mReadClients)
    {
        if (!readClient.IsSubscriptionTypeIdle())
        {
            continue;
        }
        if (!readClient.IsMatchingClient(subscriptionId))
        {
            continue;
        }

        return readClient.OnUnsolicitedReportData(apExchangeContext, std::move(aPayload));
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR InteractionModelEngine::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext,
                                                     const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest))
    {
        return OnInvokeCommandRequest(apExchangeContext, aPayloadHeader, std::move(aPayload));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReadRequest))
    {
        return OnReadInitialRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), ReadHandler::InteractionType::Read);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::WriteRequest))
    {
        return OnWriteRequest(apExchangeContext, aPayloadHeader, std::move(aPayload));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::SubscribeRequest))
    {
        return OnReadInitialRequest(apExchangeContext, aPayloadHeader, std::move(aPayload),
                                    ReadHandler::InteractionType::Subscribe);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReportData))
    {
        return OnUnsolicitedReportData(apExchangeContext, aPayloadHeader, std::move(aPayload));
    }
    else
    {
        return OnUnknownMsgType(apExchangeContext, aPayloadHeader, std::move(aPayload));
    }
}

void InteractionModelEngine::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(InteractionModel, "Time out! Failed to receive IM response from Exchange: " ChipLogFormatExchange,
                    ChipLogValueExchange(ec));
}

CHIP_ERROR InteractionModelEngine::SendReadRequest(ReadPrepareParams & aReadPrepareParams, ReadClient::Callback * aCallback)
{
    ReadClient * client = nullptr;
    CHIP_ERROR err      = CHIP_NO_ERROR;
    ReturnErrorOnFailure(NewReadClient(&client, ReadClient::InteractionType::Read, aCallback));
    err = client->SendReadRequest(aReadPrepareParams);
    if (err != CHIP_NO_ERROR)
    {
        client->Shutdown();
    }
    return err;
}

CHIP_ERROR InteractionModelEngine::SendSubscribeRequest(ReadPrepareParams & aReadPrepareParams, ReadClient::Callback * aCallback)
{
    ReadClient * client = nullptr;
    ReturnErrorOnFailure(NewReadClient(&client, ReadClient::InteractionType::Subscribe, aCallback));
    ReturnErrorOnFailure(client->SendSubscribeRequest(aReadPrepareParams));
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
        lastClusterInfo = lastClusterInfo->mpNext;
    }
    lastClusterInfo->mpNext    = mpNextAvailableClusterInfo;
    mpNextAvailableClusterInfo = aClusterInfo;
    aClusterInfo               = nullptr;
}

CHIP_ERROR InteractionModelEngine::PushFront(ClusterInfo *& aClusterInfoList, ClusterInfo & aClusterInfo)
{
    ClusterInfo * last = aClusterInfoList;
    if (mpNextAvailableClusterInfo == nullptr)
    {
        ChipLogError(InteractionModel, "ClusterInfo pool full, cannot handle more entries!");
        return CHIP_ERROR_NO_MEMORY;
    }
    aClusterInfoList           = mpNextAvailableClusterInfo;
    mpNextAvailableClusterInfo = mpNextAvailableClusterInfo->mpNext;
    *aClusterInfoList          = aClusterInfo;
    aClusterInfoList->mpNext   = last;
    return CHIP_NO_ERROR;
}

bool InteractionModelEngine::MergeOverlappedAttributePath(ClusterInfo * apAttributePathList, ClusterInfo & aAttributePath)
{
    ClusterInfo * runner = apAttributePathList;
    while (runner != nullptr)
    {
        // If overlapped, we would skip this target path,
        // --If targetPath is part of previous path, return true
        // --If previous path is part of target path, update filedid and listindex and mflags with target path, return true
        if (runner->IsAttributePathSupersetOf(aAttributePath))
        {
            return true;
        }
        if (aAttributePath.IsAttributePathSupersetOf(*runner))
        {
            runner->mListIndex   = aAttributePath.mListIndex;
            runner->mAttributeId = aAttributePath.mAttributeId;
            return true;
        }
        runner = runner->mpNext;
    }
    return false;
}

bool InteractionModelEngine::IsOverlappedAttributePath(ClusterInfo & aAttributePath)
{
    for (auto & handler : mReadHandlers)
    {
        if (handler.IsSubscriptionType() && (handler.IsGeneratingReports() || handler.IsAwaitingReportResponse()))
        {
            for (auto clusterInfo = handler.GetAttributeClusterInfolist(); clusterInfo != nullptr;
                 clusterInfo      = clusterInfo->mpNext)
            {
                if (clusterInfo->IsAttributePathSupersetOf(aAttributePath) ||
                    aAttributePath.IsAttributePathSupersetOf(*clusterInfo))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void InteractionModelEngine::DispatchCommand(CommandHandler & apCommandObj, const ConcreteCommandPath & aCommandPath,
                                             TLV::TLVReader & apPayload)
{
    CommandHandlerInterface * handler = FindCommandHandler(aCommandPath.mEndpointId, aCommandPath.mClusterId);

    if (handler)
    {
        CommandHandlerInterface::HandlerContext context(apCommandObj, aCommandPath, apPayload);
        handler->InvokeCommand(context);

        //
        // If the command was handled, don't proceed any further and return successfully.
        //
        if (context.mCommandHandled)
        {
            return;
        }
    }

    DispatchSingleClusterCommand(aCommandPath, apPayload, &apCommandObj);
}

bool InteractionModelEngine::CommandExists(const ConcreteCommandPath & aCommandPath)
{
    return ServerClusterCommandExists(aCommandPath);
}

CHIP_ERROR InteractionModelEngine::RegisterCommandHandler(CommandHandlerInterface * handler)
{
    VerifyOrReturnError(handler != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    for (auto * cur = mCommandHandlerList; cur; cur = cur->GetNext())
    {
        if (cur->Matches(*handler))
        {
            ChipLogError(InteractionModel, "Duplicate command handler registration failed");
            return CHIP_ERROR_INCORRECT_STATE;
        }
    }

    handler->SetNext(mCommandHandlerList);
    mCommandHandlerList = handler;

    return CHIP_NO_ERROR;
}

void InteractionModelEngine::UnregisterCommandHandlers(EndpointId endpointId)
{
    CommandHandlerInterface * prev = nullptr;

    for (auto * cur = mCommandHandlerList; cur; cur = cur->GetNext())
    {
        if (cur->MatchesEndpoint(endpointId))
        {
            if (prev == nullptr)
            {
                mCommandHandlerList = cur->GetNext();
            }
            else
            {
                prev->SetNext(cur->GetNext());
            }

            cur->SetNext(nullptr);
        }
        else
        {
            prev = cur;
        }
    }
}

CHIP_ERROR InteractionModelEngine::UnregisterCommandHandler(CommandHandlerInterface * handler)
{
    VerifyOrReturnError(handler != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    CommandHandlerInterface * prev = nullptr;

    for (auto * cur = mCommandHandlerList; cur; cur = cur->GetNext())
    {
        if (cur->Matches(*handler))
        {
            if (prev == nullptr)
            {
                mCommandHandlerList = cur->GetNext();
            }
            else
            {
                prev->SetNext(cur->GetNext());
            }

            cur->SetNext(nullptr);

            return CHIP_NO_ERROR;
        }

        prev = cur;
    }

    return CHIP_ERROR_KEY_NOT_FOUND;
}

CommandHandlerInterface * InteractionModelEngine::FindCommandHandler(EndpointId endpointId, ClusterId clusterId)
{
    for (auto * cur = mCommandHandlerList; cur; cur = cur->GetNext())
    {
        if (cur->Matches(endpointId, clusterId))
        {
            return cur;
        }
    }

    return nullptr;
}

} // namespace app
} // namespace chip
