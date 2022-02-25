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
 *      This file defines objects for a CHIP Interaction Data model Engine which handle unsolicited IM message, and
 *      manage different kinds of IM client and handlers.
 *
 */

#include "InteractionModelEngine.h"
#include <cinttypes>

namespace chip {
namespace app {
InteractionModelEngine sInteractionModelEngine;

InteractionModelEngine::InteractionModelEngine() {}

InteractionModelEngine * InteractionModelEngine::GetInstance()
{
    return &sInteractionModelEngine;
}

CHIP_ERROR InteractionModelEngine::Init(Messaging::ExchangeManager * apExchangeMgr)
{
    mpExchangeMgr = apExchangeMgr;

    ReturnErrorOnFailure(mpExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, this));

    mReportingEngine.Init();
    mMagic++;

    StatusIB::RegisterErrorFormatter();

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

    // Increase magic number to invalidate all Handle-s.
    mMagic++;

    mCommandHandlerObjs.ReleaseAll();

    mTimedHandlers.ForEachActiveObject([this](TimedHandler * obj) -> Loop {
        mpExchangeMgr->CloseAllContextsForDelegate(obj);
        return Loop::Continue;
    });

    mTimedHandlers.ReleaseAll();

    mReadHandlers.ReleaseAll();

    //
    // We hold weak references to ReadClient objects. The application ultimately
    // actually owns them, so it's on them to eventually shut them down and free them
    // up.
    //
    // However, we should null out their pointers back to us at the very least so that
    // at destruction time, they won't attempt to reach back here to remove themselves
    // from this list.
    //
    for (auto * readClient = mpActiveReadClientList; readClient != nullptr;)
    {
        readClient->mpImEngine = nullptr;
        auto * tmpClient       = readClient->GetNextClient();
        readClient->SetNextClient(nullptr);
        readClient = tmpClient;
    }

    //
    // After that, we just null out our tracker.
    //
    mpActiveReadClientList = nullptr;

    for (auto & writeHandler : mWriteHandlers)
    {
        writeHandler.Abort();
    }

    mReportingEngine.Shutdown();
    mClusterInfoPool.ReleaseAll();

    mpExchangeMgr->UnregisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id);
}

uint32_t InteractionModelEngine::GetNumActiveReadHandlers() const
{
    return static_cast<uint32_t>(mReadHandlers.Allocated());
}

uint32_t InteractionModelEngine::GetNumActiveReadHandlers(ReadHandler::InteractionType aType) const
{
    uint32_t count = 0;

    mReadHandlers.ForEachActiveObject([aType, &count](const ReadHandler * handler) {
        if (handler->IsType(aType))
        {
            count++;
        }

        return Loop::Continue;
    });

    return count;
}

ReadHandler * InteractionModelEngine::ActiveHandlerAt(unsigned int aIndex)
{
    if (aIndex >= mReadHandlers.Allocated())
    {
        return nullptr;
    }

    unsigned int i    = 0;
    ReadHandler * ret = nullptr;

    mReadHandlers.ForEachActiveObject([aIndex, &i, &ret](ReadHandler * handler) {
        if (i == aIndex)
        {
            ret = handler;
            return Loop::Break;
        }

        i++;
        return Loop::Continue;
    });

    return ret;
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
    for (auto * readClient = mpActiveReadClientList; readClient != nullptr; readClient = readClient->GetNextClient())
    {
        if (readClient->IsSubscriptionType() && readClient->IsMatchingClient(aSubscriptionId))
        {
            readClient->Close(CHIP_NO_ERROR);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_KEY_NOT_FOUND;
}

CHIP_ERROR InteractionModelEngine::ShutdownSubscriptions(FabricIndex aFabricIndex, NodeId aPeerNodeId)
{
    for (auto * readClient = mpActiveReadClientList; readClient != nullptr; readClient = readClient->GetNextClient())
    {
        if (readClient->IsSubscriptionType() && readClient->GetFabricIndex() == aFabricIndex &&
            readClient->GetPeerNodeId() == aPeerNodeId)
        {
            readClient->Close(CHIP_NO_ERROR);
        }
    }

    return CHIP_NO_ERROR;
}

void InteractionModelEngine::OnDone(CommandHandler & apCommandObj)
{
    mCommandHandlerObjs.ReleaseObject(&apCommandObj);
}

void InteractionModelEngine::OnDone(ReadHandler & apReadObj)
{
    //
    // Deleting an item can shift down the contents of the underlying pool storage,
    // rendering any tracker using positional indexes invalid. Let's reset it,
    // based on which readHandler we are getting rid of.
    //
    mReportingEngine.ResetReadHandlerTracker(&apReadObj);

    mReadHandlers.ReleaseObject(&apReadObj);
}

CHIP_ERROR InteractionModelEngine::OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext,
                                                          const PayloadHeader & aPayloadHeader,
                                                          System::PacketBufferHandle && aPayload, bool aIsTimedInvoke,
                                                          Protocols::InteractionModel::Status & aStatus)
{
    CommandHandler * commandHandler = mCommandHandlerObjs.CreateObject(this);
    if (commandHandler == nullptr)
    {
        ChipLogProgress(InteractionModel, "no resource for Invoke interaction");
        aStatus = Protocols::InteractionModel::Status::Busy;
        return CHIP_ERROR_NO_MEMORY;
    }
    ReturnErrorOnFailure(
        commandHandler->OnInvokeCommandRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), aIsTimedInvoke));
    aStatus = Protocols::InteractionModel::Status::Success;
    return CHIP_NO_ERROR;
}

CHIP_ERROR InteractionModelEngine::OnReadInitialRequest(Messaging::ExchangeContext * apExchangeContext,
                                                        const PayloadHeader & aPayloadHeader,
                                                        System::PacketBufferHandle && aPayload,
                                                        ReadHandler::InteractionType aInteractionType,
                                                        Protocols::InteractionModel::Status & aStatus)
{
    ChipLogDetail(InteractionModel, "Received %s request",
                  aInteractionType == ReadHandler::InteractionType::Subscribe ? "Subscribe" : "Read");

    //
    // Let's first figure out if the client has sent us a subscribe request and requested we keep any existing
    // subscriptions from that source.
    //
    if (aInteractionType == ReadHandler::InteractionType::Subscribe)
    {
        System::PacketBufferTLVReader reader;
        bool keepExistingSubscriptions = true;

        reader.Init(aPayload.Retain());

        SubscribeRequestMessage::Parser subscribeRequestParser;
        ReturnErrorOnFailure(subscribeRequestParser.Init(reader));

        ReturnErrorOnFailure(subscribeRequestParser.GetKeepSubscriptions(&keepExistingSubscriptions));

        if (!keepExistingSubscriptions)
        {
            //
            // Walk through all existing subscriptions and shut down those whose subscriber matches
            // that which just came in.
            //
            mReadHandlers.ForEachActiveObject([this, apExchangeContext](ReadHandler * handler) {
                if (handler->IsFromSubscriber(*apExchangeContext))
                {
                    ChipLogProgress(InteractionModel,
                                    "Deleting previous subscription from NodeId: " ChipLogFormatX64 ", FabricIndex: %u",
                                    ChipLogValueX64(apExchangeContext->GetSessionHandle()->AsSecureSession()->GetPeerNodeId()),
                                    apExchangeContext->GetSessionHandle()->GetFabricIndex());
                    mReadHandlers.ReleaseObject(handler);
                }

                return Loop::Continue;
            });
        }
    }

    size_t handlerPoolCapacity = mReadHandlers.Capacity();

#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    if (mReadHandlerCapacityOverride != -1)
    {
        handlerPoolCapacity = (size_t) mReadHandlerCapacityOverride;
    }
#endif

    // Reserve the last ReadHandler for ReadInteraction
    if (aInteractionType == ReadHandler::InteractionType::Subscribe && ((handlerPoolCapacity - GetNumActiveReadHandlers()) == 1) &&
        !HasActiveRead())
    {
        ChipLogProgress(InteractionModel, "Reserve the last ReadHandler for IM read Interaction");
        aStatus = Protocols::InteractionModel::Status::ResourceExhausted;
        return CHIP_NO_ERROR;
    }

#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    if ((handlerPoolCapacity - GetNumActiveReadHandlers()) == 0)
    {
        aStatus = Protocols::InteractionModel::Status::ResourceExhausted;
        return CHIP_NO_ERROR;
    }
#endif

    ReadHandler * handler = mReadHandlers.CreateObject(*this, apExchangeContext, aInteractionType);
    if (handler)
    {
        ReturnErrorOnFailure(handler->OnInitialRequest(std::move(aPayload)));

        aStatus = Protocols::InteractionModel::Status::Success;
        return CHIP_NO_ERROR;
    }

    ChipLogProgress(InteractionModel, "no resource for %s interaction",
                    aInteractionType == ReadHandler::InteractionType::Subscribe ? "Subscribe" : "Read");
    aStatus = Protocols::InteractionModel::Status::ResourceExhausted;

    return CHIP_NO_ERROR;
}

Protocols::InteractionModel::Status InteractionModelEngine::OnWriteRequest(Messaging::ExchangeContext * apExchangeContext,
                                                                           const PayloadHeader & aPayloadHeader,
                                                                           System::PacketBufferHandle && aPayload,
                                                                           bool aIsTimedWrite)
{
    ChipLogDetail(InteractionModel, "Received Write request");

    for (auto & writeHandler : mWriteHandlers)
    {
        if (writeHandler.IsFree())
        {
            VerifyOrReturnError(writeHandler.Init() == CHIP_NO_ERROR, Status::Busy);
            return writeHandler.OnWriteRequest(apExchangeContext, std::move(aPayload), aIsTimedWrite);
        }
    }
    ChipLogProgress(InteractionModel, "no resource for write interaction");
    return Status::Busy;
}

CHIP_ERROR InteractionModelEngine::OnTimedRequest(Messaging::ExchangeContext * apExchangeContext,
                                                  const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload,
                                                  Protocols::InteractionModel::Status & aStatus)
{
    TimedHandler * handler = mTimedHandlers.CreateObject();
    if (handler == nullptr)
    {
        ChipLogProgress(InteractionModel, "no resource for Timed interaction");
        aStatus = Protocols::InteractionModel::Status::Busy;
        return CHIP_ERROR_NO_MEMORY;
    }

    // The timed handler takes over handling of this exchange and will do its
    // own status reporting as needed.
    aStatus = Protocols::InteractionModel::Status::Success;
    apExchangeContext->SetDelegate(handler);
    return handler->OnMessageReceived(apExchangeContext, aPayloadHeader, std::move(aPayload));
}

CHIP_ERROR InteractionModelEngine::OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext,
                                                           const PayloadHeader & aPayloadHeader,
                                                           System::PacketBufferHandle && aPayload)
{
    System::PacketBufferTLVReader reader;
    reader.Init(aPayload.Retain());

    ReportDataMessage::Parser report;
    ReturnErrorOnFailure(report.Init(reader));

    uint64_t subscriptionId = 0;
    ReturnErrorOnFailure(report.GetSubscriptionId(&subscriptionId));
    ReturnErrorOnFailure(report.ExitContainer());

    for (auto * readClient = mpActiveReadClientList; readClient != nullptr; readClient = readClient->GetNextClient())
    {
        if (!readClient->IsSubscriptionIdle())
        {
            continue;
        }

        if (!readClient->IsMatchingClient(subscriptionId))
        {
            continue;
        }

        return readClient->OnUnsolicitedReportData(apExchangeContext, std::move(aPayload));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR InteractionModelEngine::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext,
                                                     const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    using namespace Protocols::InteractionModel;

    CHIP_ERROR err                             = CHIP_NO_ERROR;
    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Failure;

    // Group Message can only be an InvokeCommandRequest or WriteRequest
    if (apExchangeContext->IsGroupExchangeContext() &&
        !aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest) &&
        !aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::WriteRequest))
    {
        ChipLogProgress(InteractionModel, "Msg type %d not supported for group message", aPayloadHeader.GetMessageType());
        return err;
    }

    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest))
    {
        SuccessOrExit(
            OnInvokeCommandRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), /* aIsTimedInvoke = */ false, status));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReadRequest))
    {
        SuccessOrExit(OnReadInitialRequest(apExchangeContext, aPayloadHeader, std::move(aPayload),
                                           ReadHandler::InteractionType::Read, status));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::WriteRequest))
    {
        status = OnWriteRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), /* aIsTimedWrite = */ false);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::SubscribeRequest))
    {
        SuccessOrExit(OnReadInitialRequest(apExchangeContext, aPayloadHeader, std::move(aPayload),
                                           ReadHandler::InteractionType::Subscribe, status));
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReportData))
    {
        ReturnErrorOnFailure(OnUnsolicitedReportData(apExchangeContext, aPayloadHeader, std::move(aPayload)));
        status = Protocols::InteractionModel::Status::Success;
    }
    else if (aPayloadHeader.HasMessageType(MsgType::TimedRequest))
    {
        SuccessOrExit(OnTimedRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), status));
    }
    else
    {
        ChipLogProgress(InteractionModel, "Msg type %d not supported", aPayloadHeader.GetMessageType());
    }

exit:
    if (status != Protocols::InteractionModel::Status::Success && !apExchangeContext->IsGroupExchangeContext())
    {
        err = StatusResponse::Send(status, apExchangeContext, false /*aExpectResponse*/);
    }
    return err;
}

void InteractionModelEngine::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(InteractionModel, "Time out! Failed to receive IM response from Exchange: " ChipLogFormatExchange,
                    ChipLogValueExchange(ec));
}

void InteractionModelEngine::AddReadClient(ReadClient * apReadClient)
{
    apReadClient->SetNextClient(mpActiveReadClientList);
    mpActiveReadClientList = apReadClient;
}

void InteractionModelEngine::RemoveReadClient(ReadClient * apReadClient)
{
    ReadClient * pPrevListItem = nullptr;
    ReadClient * pCurListItem  = mpActiveReadClientList;

    while (pCurListItem != apReadClient)
    {
        pPrevListItem = pCurListItem;
        pCurListItem  = pCurListItem->GetNextClient();
    }

    //
    // Item must exist in this tracker list. If not, there's a bug somewhere.
    //
    VerifyOrDie(pCurListItem != nullptr);

    if (pPrevListItem)
    {
        pPrevListItem->SetNextClient(apReadClient->GetNextClient());
    }
    else
    {
        mpActiveReadClientList = apReadClient->GetNextClient();
    }

    apReadClient->SetNextClient(nullptr);
}

size_t InteractionModelEngine::GetNumActiveReadClients()
{
    ReadClient * pListItem = mpActiveReadClientList;
    size_t count           = 0;

    while (pListItem)
    {
        pListItem = pListItem->GetNextClient();
        count++;
    }

    return count;
}

bool InteractionModelEngine::InActiveReadClientList(ReadClient * apReadClient)
{
    ReadClient * pListItem = mpActiveReadClientList;

    while (pListItem)
    {
        if (pListItem == apReadClient)
        {
            return true;
        }

        pListItem = pListItem->GetNextClient();
    }

    return false;
}

void InteractionModelEngine::ReleaseClusterInfoList(ClusterInfo *& aClusterInfo)
{
    ClusterInfo * current = aClusterInfo;
    while (current != nullptr)
    {
        ClusterInfo * next = current->mpNext;
        mClusterInfoPool.ReleaseObject(current);
        current = next;
    }

    aClusterInfo = nullptr;
}

CHIP_ERROR InteractionModelEngine::PushFront(ClusterInfo *& aClusterInfoList, ClusterInfo & aClusterInfo)
{
    ClusterInfo * clusterInfo = mClusterInfoPool.CreateObject();
    if (clusterInfo == nullptr)
    {
        ChipLogError(InteractionModel, "ClusterInfo pool full, cannot handle more entries!");
        return CHIP_ERROR_NO_MEMORY;
    }
    *clusterInfo        = aClusterInfo;
    clusterInfo->mpNext = aClusterInfoList;
    aClusterInfoList    = clusterInfo;
    return CHIP_NO_ERROR;
}

bool InteractionModelEngine::IsOverlappedAttributePath(ClusterInfo & aAttributePath)
{
    return (mReadHandlers.ForEachActiveObject([&aAttributePath](ReadHandler * handler) {
        if (handler->IsType(ReadHandler::InteractionType::Subscribe) &&
            (handler->IsGeneratingReports() || handler->IsAwaitingReportResponse()))
        {
            for (auto clusterInfo = handler->GetAttributeClusterInfolist(); clusterInfo != nullptr;
                 clusterInfo      = clusterInfo->mpNext)
            {
                if (clusterInfo->IsAttributePathSupersetOf(aAttributePath) ||
                    aAttributePath.IsAttributePathSupersetOf(*clusterInfo))
                {
                    return Loop::Break;
                }
            }
        }

        return Loop::Continue;
    }) == Loop::Break);
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

Protocols::InteractionModel::Status InteractionModelEngine::CommandExists(const ConcreteCommandPath & aCommandPath)
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

void InteractionModelEngine::OnTimedInteractionFailed(TimedHandler * apTimedHandler)
{
    mTimedHandlers.ReleaseObject(apTimedHandler);
}

void InteractionModelEngine::OnTimedInvoke(TimedHandler * apTimedHandler, Messaging::ExchangeContext * apExchangeContext,
                                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    using namespace Protocols::InteractionModel;

    // Reset the ourselves as the exchange delegate for now, to match what we'd
    // do with an initial unsolicited invoke.
    apExchangeContext->SetDelegate(this);
    mTimedHandlers.ReleaseObject(apTimedHandler);

    VerifyOrDie(aPayloadHeader.HasMessageType(MsgType::InvokeCommandRequest));
    VerifyOrDie(!apExchangeContext->IsGroupExchangeContext());

    Status status = Status::Failure;
    OnInvokeCommandRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), /* aIsTimedInvoke = */ true, status);
    if (status != Status::Success)
    {
        StatusResponse::Send(status, apExchangeContext, /* aExpectResponse = */ false);
    }
}

void InteractionModelEngine::OnTimedWrite(TimedHandler * apTimedHandler, Messaging::ExchangeContext * apExchangeContext,
                                          const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    using namespace Protocols::InteractionModel;

    // Reset the ourselves as the exchange delegate for now, to match what we'd
    // do with an initial unsolicited write.
    apExchangeContext->SetDelegate(this);
    mTimedHandlers.ReleaseObject(apTimedHandler);

    VerifyOrDie(aPayloadHeader.HasMessageType(MsgType::WriteRequest));
    VerifyOrDie(!apExchangeContext->IsGroupExchangeContext());

    Status status = OnWriteRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), /* aIsTimedWrite = */ true);
    if (status != Status::Success)
    {
        StatusResponse::Send(status, apExchangeContext, /* aExpectResponse = */ false);
    }
}

bool InteractionModelEngine::HasActiveRead()
{
    return ((mReadHandlers.ForEachActiveObject([](ReadHandler * handler) {
        if (handler->IsType(ReadHandler::InteractionType::Read))
        {
            return Loop::Break;
        }

        return Loop::Continue;
    }) == Loop::Break));
}

} // namespace app
} // namespace chip
