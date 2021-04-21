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

    mpExchangeMgr = apExchangeMgr;
    mpDelegate    = apDelegate;

    err = mpExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, this);
    SuccessOrExit(err);

    mReportingEngine.Init();
    SuccessOrExit(err);

exit:
    return err;
}

void InteractionModelEngine::Shutdown()
{
    for (auto & commandSender : mCommandSenderObjs)
    {
        commandSender.Shutdown();
    }

    for (auto & commandHandler : mCommandHandlerObjs)
    {
        commandHandler.Shutdown();
    }

    for (auto & readClient : mReadClients)
    {
        readClient.Shutdown();
    }

    for (auto & readHandler : mReadHandlers)
    {
        readHandler.Shutdown();
    }
}

CHIP_ERROR InteractionModelEngine::NewCommandSender(CommandSender ** const apCommandSender)
{
    CHIP_ERROR err   = CHIP_ERROR_NO_MEMORY;
    *apCommandSender = nullptr;

    for (auto & commandSender : mCommandSenderObjs)
    {
        if (commandSender.IsFree())
        {
            *apCommandSender = &commandSender;
            err              = commandSender.Init(mpExchangeMgr, mpDelegate);
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

CHIP_ERROR InteractionModelEngine::NewReadClient(ReadClient ** const apReadClient)
{
    CHIP_ERROR err = CHIP_ERROR_NO_MEMORY;

    for (auto & readClient : mReadClients)
    {
        if (readClient.IsFree())
        {
            *apReadClient = &readClient;
            err           = readClient.Init(mpExchangeMgr, mpDelegate);
            if (CHIP_NO_ERROR != err)
            {
                *apReadClient = nullptr;
            }
            return err;
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
    apExchangeContext = nullptr;

    ChipLogFunctError(err);

    // Todo: Fix the below check after the above status report is implemented.
    if (nullptr != apExchangeContext)
    {
        apExchangeContext->Abort();
    }
}

void InteractionModelEngine::OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext,
                                                    const PacketHeader & aPacketHeader, const PayloadHeader & aPayloadHeader,
                                                    System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    for (auto & commandHandler : mCommandHandlerObjs)
    {
        if (commandHandler.IsFree())
        {
            err = commandHandler.Init(mpExchangeMgr, mpDelegate);
            SuccessOrExit(err);
            commandHandler.OnMessageReceived(apExchangeContext, aPacketHeader, aPayloadHeader, std::move(aPayload));
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
}

void InteractionModelEngine::OnReadRequest(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DataManagement, "Receive Read request");

    for (auto & readHandler : mReadHandlers)
    {
        if (readHandler.IsFree())
        {
            err = readHandler.Init(mpDelegate);
            SuccessOrExit(err);
            err = readHandler.OnReadRequest(apExchangeContext, std::move(aPayload));
            SuccessOrExit(err);
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

void InteractionModelEngine::ReleaseClusterInfoListToPool(ReadHandler * const apReadHandler)
{
    ClusterInfo * const clusterInfoList = apReadHandler->GetClusterInfoList();
    size_t numClusterInfos              = apReadHandler->GetNumClusterInfos();
    size_t numClusterInfosToBeAffected;

    apReadHandler->ClearClusterInfo();

    if (numClusterInfos == 0)
    {
        ChipLogDetail(DataManagement, "No cluster instances allocated");
        ExitNow();
    }

    // make sure everything is still sane
    ChipLogIfFalse(clusterInfoList >= mClusterInfoPool);
    ChipLogIfFalse(numClusterInfos <= mNumClusterInfos);

    // mPathGroupPool + IM_PUBLISHER_MAX_NUM_PATH_GROUPS is a pointer which points to the last+1byte of this array
    // clusterInfoList is a pointer to the first cluster instance to be released
    // the result of subtraction is the number of cluster instances from clusterInfoList to the end of this array
    numClusterInfosToBeAffected = ((size_t) mClusterInfoPool + mNumClusterInfos) - (size_t) clusterInfoList;

    // Shrink the clusterInfosInPool by the number of cluster instances.
    mNumClusterInfos -= numClusterInfos;

    ChipLogDetail(DataManagement, "numClusterInfos is %d, and numClusterInfosToBeAffected is %d", numClusterInfos,
                  numClusterInfosToBeAffected);
    if (numClusterInfos == numClusterInfosToBeAffected)
    {
        ChipLogDetail(DataManagement, "Releasing the last block of cluster instances");
        ExitNow();
    }

    ChipLogDetail(DataManagement, "Moving %d cluster infos forward", numClusterInfosToBeAffected - numClusterInfos);

    memmove(clusterInfoList, clusterInfoList + numClusterInfos,
            sizeof(ClusterInfo) * (numClusterInfosToBeAffected - numClusterInfos));

    for (size_t i = 0; i < CHIP_MAX_NUM_READ_HANDLER; ++i)
    {
        ReadHandler * const handler = mReadHandlers + i;

        if ((apReadHandler != handler) && (handler->GetClusterInfoList() > clusterInfoList))
        {
            handler->ShrinkClusterInfo(numClusterInfos);
        }
    }

exit:
    ChipLogDetail(DataManagement, "Number of allocated cluster instances: %u", mNumClusterInfos);
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

CHIP_ERROR __attribute__((weak))
ReadSingleClusterData(NodeId aNodeId, ClusterId aClusterId, EndpointId aEndPointId, FieldId aFieldId, TLV::TLVWriter & aWriter)
{
    ChipLogDetail(DataManagement,
                  "Received Cluster Command: Cluster=%" PRIx16 " NodeId=%" PRIx64 " Endpoint=%" PRIx8 " FieldId=%" PRIx8,
                  aClusterId, aNodeId, aEndPointId, aFieldId);
    ChipLogError(DataManagement,
                 "Default ReadSingleClusterData is called, this should be replaced by actual dispatched for cluster");
    return CHIP_NO_ERROR;
}

CHIP_ERROR __attribute__((weak))
WriteSingleClusterData(NodeId aNodeId, ClusterId aClusterId, EndpointId aEndPointId, FieldId aFieldId, TLV::TLVReader & aReader)
{
    ChipLogDetail(DataManagement,
                  "Received Cluster Command: Cluster=%" PRIx16 " NodeId=%" PRIx64 " Endpoint=%" PRIx8 " FieldId=%" PRIx8,
                  aClusterId, aNodeId, aEndPointId, aFieldId);
    ChipLogError(DataManagement,
                 "Default WriteSingleClusterData is called, this should be replaced by actual dispatched for cluster");
    return CHIP_NO_ERROR;
}

uint16_t InteractionModelEngine::GetReadClientArrayIndex(const ReadClient * const apReadClient) const
{
    return static_cast<uint16_t>(apReadClient - mReadClients);
}

CHIP_ERROR InteractionModelEngine::GetFirstAvailableClusterInfo(ClusterInfo *& apClusterInfo)
{
    if (mNumClusterInfos >= IM_SERVER_MAX_NUM_PATH_GROUPS)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    apClusterInfo = &mClusterInfoPool[mNumClusterInfos];
    mNumClusterInfos++;
    apClusterInfo->ClearDirty();
    return CHIP_NO_ERROR;
}
} // namespace app
} // namespace chip
