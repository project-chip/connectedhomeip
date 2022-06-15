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

#include <lib/core/CHIPTLVUtilities.hpp>

extern bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);

namespace chip {
namespace app {
InteractionModelEngine sInteractionModelEngine;

InteractionModelEngine::InteractionModelEngine() {}

InteractionModelEngine * InteractionModelEngine::GetInstance()
{
    return &sInteractionModelEngine;
}

CHIP_ERROR InteractionModelEngine::Init(Messaging::ExchangeManager * apExchangeMgr, FabricTable * apFabricTable)
{
    mpExchangeMgr = apExchangeMgr;
    mpFabricTable = apFabricTable;

    ReturnErrorOnFailure(mpExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, this));
    VerifyOrReturnError(mpFabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

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
    mAttributePathPool.ReleaseAll();
    mEventPathPool.ReleaseAll();
    mDataVersionFilterPool.ReleaseAll();
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

uint32_t InteractionModelEngine::GetNumActiveReadHandlers(ReadHandler::InteractionType aType, FabricIndex aFabricIndex) const
{
    uint32_t count = 0;

    mReadHandlers.ForEachActiveObject([aType, aFabricIndex, &count](const ReadHandler * handler) {
        if (handler->IsType(aType) && handler->GetAccessingFabricIndex() == aFabricIndex)
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

void InteractionModelEngine::CloseTransactionsFromFabricIndex(FabricIndex aFabricIndex)
{
    //
    // Walk through all existing subscriptions and shut down those whose subscriber matches
    // that which just came in.
    //
    mReadHandlers.ForEachActiveObject([this, aFabricIndex](ReadHandler * handler) {
        if (handler->GetAccessingFabricIndex() == aFabricIndex)
        {
            ChipLogProgress(InteractionModel, "Deleting expired ReadHandler for NodeId: " ChipLogFormatX64 ", FabricIndex: %u",
                            ChipLogValueX64(handler->GetInitiatorNodeId()), aFabricIndex);
            mReadHandlers.ReleaseObject(handler);
        }

        return Loop::Continue;
    });
}

CHIP_ERROR InteractionModelEngine::ShutdownSubscription(SubscriptionId aSubscriptionId)
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

Protocols::InteractionModel::Status InteractionModelEngine::OnReadInitialRequest(Messaging::ExchangeContext * apExchangeContext,
                                                                                 const PayloadHeader & aPayloadHeader,
                                                                                 System::PacketBufferHandle && aPayload,
                                                                                 ReadHandler::InteractionType aInteractionType)
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

        if (apExchangeContext->GetSessionHandle()->GetFabricIndex() == kUndefinedFabricIndex)
        {
            // Subscriptions must be associated to a fabric.
            return Status::UnsupportedAccess;
        }

        reader.Init(aPayload.Retain());

        SubscribeRequestMessage::Parser subscribeRequestParser;
        CHIP_ERROR err = subscribeRequestParser.Init(reader);
        if (err != CHIP_NO_ERROR)
        {
            return Status::InvalidAction;
        }

        {
            size_t requestedAttributePathCount = 0;
            size_t requestedEventPathCount     = 0;
            AttributePathIBs::Parser attributePathListParser;
            err = subscribeRequestParser.GetAttributeRequests(&attributePathListParser);
            if (err == CHIP_NO_ERROR)
            {
                TLV::TLVReader pathReader;
                attributePathListParser.GetReader(&pathReader);
                err = TLV::Utilities::Count(pathReader, requestedAttributePathCount, false);
            }
            else if (err == CHIP_ERROR_END_OF_TLV)
            {
                err = CHIP_NO_ERROR;
            }
            if (err != CHIP_NO_ERROR)
            {
                return Status::InvalidAction;
            }

            EventPathIBs::Parser eventpathListParser;
            err = subscribeRequestParser.GetEventRequests(&eventpathListParser);
            if (err == CHIP_NO_ERROR)
            {
                TLV::TLVReader pathReader;
                eventpathListParser.GetReader(&pathReader);
                err = TLV::Utilities::Count(pathReader, requestedEventPathCount, false);
            }
            else if (err == CHIP_ERROR_END_OF_TLV)
            {
                err = CHIP_NO_ERROR;
            }
            if (err != CHIP_NO_ERROR)
            {
                return Status::InvalidAction;
            }

            // The following cast is safe, since we can only hold a few tens of paths in one request.
            if (!EnsureResourceForSubscription(apExchangeContext->GetSessionHandle()->GetFabricIndex(), requestedAttributePathCount,
                                               requestedEventPathCount))
            {
                return Status::PathsExhausted;
            }
        }

        err = subscribeRequestParser.GetKeepSubscriptions(&keepExistingSubscriptions);
        if (err != CHIP_NO_ERROR)
        {
            return Status::InvalidAction;
        }

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

#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    size_t handlerPoolCapacity = mReadHandlers.Capacity();
    if (mReadHandlerCapacityOverride != -1)
    {
        handlerPoolCapacity = (size_t) mReadHandlerCapacityOverride;
    }

    if ((handlerPoolCapacity - GetNumActiveReadHandlers()) == 0)
    {
        return Status::ResourceExhausted;
    }
#endif

    // We have already reserved enough resources for read requests, and have granted enough resources for current subscriptions, so
    // we should be able to allocate resources requested by this request.
    ReadHandler * handler = mReadHandlers.CreateObject(*this, apExchangeContext, aInteractionType);
    if (handler == nullptr)
    {
        ChipLogProgress(InteractionModel, "no resource for %s interaction",
                        aInteractionType == ReadHandler::InteractionType::Subscribe ? "Subscribe" : "Read");
        return Status::ResourceExhausted;
    }

    CHIP_ERROR err = handler->OnInitialRequest(std::move(aPayload));
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        return Status::ResourceExhausted;
    }

    // TODO: Should probably map various TLV errors into InvalidAction, here
    // or inside the read handler.
    return StatusIB(err).mStatus;
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

    SubscriptionId subscriptionId = 0;
    ReturnErrorOnFailure(report.GetSubscriptionId(&subscriptionId));
    ReturnErrorOnFailure(report.ExitContainer());

    for (auto * readClient = mpActiveReadClientList; readClient != nullptr; readClient = readClient->GetNextClient())
    {
        if (!readClient->IsSubscriptionActive())
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

CHIP_ERROR InteractionModelEngine::OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader,
                                                                ExchangeDelegate *& newDelegate)
{
    // TODO: Implement OnUnsolicitedMessageReceived, let messaging layer dispatch message to ReadHandler/ReadClient/TimedHandler
    // directly.
    newDelegate = this;
    return CHIP_NO_ERROR;
}

CHIP_ERROR InteractionModelEngine::OnMessageReceived(Messaging::ExchangeContext * apExchangeContext,
                                                     const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    using namespace Protocols::InteractionModel;

    Protocols::InteractionModel::Status status = Protocols::InteractionModel::Status::Failure;

    // Group Message can only be an InvokeCommandRequest or WriteRequest
    if (apExchangeContext->IsGroupExchangeContext() &&
        !aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest) &&
        !aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::WriteRequest))
    {
        ChipLogProgress(InteractionModel, "Msg type %d not supported for group message", aPayloadHeader.GetMessageType());
        return CHIP_NO_ERROR;
    }

    if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::InvokeCommandRequest))
    {
        OnInvokeCommandRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), /* aIsTimedInvoke = */ false, status);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReadRequest))
    {
        status = OnReadInitialRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), ReadHandler::InteractionType::Read);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::WriteRequest))
    {
        status = OnWriteRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), /* aIsTimedWrite = */ false);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::SubscribeRequest))
    {
        status =
            OnReadInitialRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), ReadHandler::InteractionType::Subscribe);
    }
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReportData))
    {
        ReturnErrorOnFailure(OnUnsolicitedReportData(apExchangeContext, aPayloadHeader, std::move(aPayload)));
        status = Protocols::InteractionModel::Status::Success;
    }
    else if (aPayloadHeader.HasMessageType(MsgType::TimedRequest))
    {
        OnTimedRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), status);
    }
    else
    {
        ChipLogProgress(InteractionModel, "Msg type %d not supported", aPayloadHeader.GetMessageType());
    }

    if (status != Protocols::InteractionModel::Status::Success && !apExchangeContext->IsGroupExchangeContext())
    {
        return StatusResponse::Send(status, apExchangeContext, false /*aExpectResponse*/);
    }

    return CHIP_NO_ERROR;
}

void InteractionModelEngine::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogError(InteractionModel, "Time out! Failed to receive IM response from Exchange: " ChipLogFormatExchange,
                 ChipLogValueExchange(ec));
}

void InteractionModelEngine::AddReadClient(ReadClient * apReadClient)
{
    apReadClient->SetNextClient(mpActiveReadClientList);
    mpActiveReadClientList = apReadClient;
}

bool InteractionModelEngine::TrimFabric(FabricIndex aFabricIndex, bool aForceEvict)
{
    const size_t pathPoolCapacity        = GetPathPoolCapacity();
    const size_t readHandlerPoolCapacity = GetReadHandlerPoolCapacity();

    uint8_t fabricCount                            = mpFabricTable->FabricCount();
    size_t attributePathsSubscribedByCurrentFabric = 0;
    size_t eventPathsSubscribedByCurrentFabric     = 0;
    size_t subscriptionsEstablishedByCurrentFabric = 0;

    if (fabricCount == 0)
    {
        return false;
    }

    size_t perFabricPathCapacity =
        static_cast<size_t>(pathPoolCapacity - kReservedPathsForReads) / static_cast<size_t>(fabricCount);
    size_t perFabricSubscriptionCapacity =
        static_cast<size_t>(readHandlerPoolCapacity - kReservedHandlersForReads) / static_cast<size_t>(fabricCount);

    ReadHandler * candidate            = nullptr;
    size_t candicateAttributePathsUsed = 0;
    size_t candicateEventPathsUsed     = 0;

    // It is safe to use & here since this function will be called on current stack.
    mReadHandlers.ForEachActiveObject([&](ReadHandler * handler) {
        if (handler->GetAccessingFabricIndex() != aFabricIndex || !handler->IsType(ReadHandler::InteractionType::Subscribe))
        {
            return Loop::Continue;
        }

        size_t attributePathsUsed = handler->GetAttributePathCount();
        size_t eventPathsUsed     = handler->GetEventPathCount();

        attributePathsSubscribedByCurrentFabric += attributePathsUsed;
        eventPathsSubscribedByCurrentFabric += eventPathsUsed;
        subscriptionsEstablishedByCurrentFabric++;

        if (candidate == nullptr)
        {
            candidate = handler;
        }
        // This handler uses more resources than the one we picked before.
        else if ((attributePathsUsed > perFabricPathCapacity || eventPathsUsed > perFabricPathCapacity) &&
                 (candicateAttributePathsUsed <= perFabricPathCapacity && candicateEventPathsUsed <= perFabricPathCapacity))
        {
            candidate                   = handler;
            candicateAttributePathsUsed = attributePathsUsed;
            candicateEventPathsUsed     = eventPathsUsed;
        }
        // This handler is older than the one we picked before.
        else if (handler->GetSubscriptionStartGeneration() < candidate->GetSubscriptionStartGeneration() &&
                 // And the level of resource usage is the same (both exceed or neither exceed)
                 ((attributePathsUsed > perFabricPathCapacity || eventPathsUsed > perFabricPathCapacity) ==
                  (candicateAttributePathsUsed > perFabricPathCapacity || candicateEventPathsUsed > perFabricPathCapacity)))
        {
            candidate = handler;
        }
        return Loop::Continue;
    });

    if (candidate != nullptr &&
        (aForceEvict || attributePathsSubscribedByCurrentFabric > perFabricPathCapacity ||
         eventPathsSubscribedByCurrentFabric > perFabricPathCapacity ||
         subscriptionsEstablishedByCurrentFabric > perFabricSubscriptionCapacity))
    {
        candidate->Abort();
        return true;
    }
    return false;
}

bool InteractionModelEngine::EnsureResourceForSubscription(FabricIndex aFabricIndex, size_t aRequestedAttributePathCount,
                                                           size_t aRequestedEventPathCount)
{
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK
#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    const bool allowUnlimited = !mForceHandlerQuota;
#else  // CONFIG_IM_BUILD_FOR_UNIT_TEST
       // If the resources are allocated on the heap, we should be able to handle as many Read / Subscribe requests as possible.
    const bool allowUnlimited = true;
#endif // CONFIG_IM_BUILD_FOR_UNIT_TEST
#else  // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK
    const bool allowUnlimited = false;
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK

    // Don't couple with read requests, always reserve enough resource for read requests.

    const size_t pathPoolCapacity        = GetPathPoolCapacity();
    const size_t readHandlerPoolCapacity = GetReadHandlerPoolCapacity();

    // If we return early here, the compiler will complain about the unreachable code, so we add a always-true check.
    const size_t attributePathCap = allowUnlimited ? SIZE_MAX : static_cast<size_t>(pathPoolCapacity - kReservedPathsForReads);
    const size_t eventPathCap     = allowUnlimited ? SIZE_MAX : static_cast<size_t>(pathPoolCapacity - kReservedPathsForReads);
    const size_t readHandlerCap =
        allowUnlimited ? SIZE_MAX : static_cast<size_t>(readHandlerPoolCapacity - kReservedHandlersForReads);

    size_t usedAttributePaths = 0;
    size_t usedEventPaths     = 0;
    size_t usedReadHandlers   = 0;

    auto countResourceUsage = [&]() {
        usedAttributePaths = 0;
        usedEventPaths     = 0;
        usedReadHandlers   = 0;
        mReadHandlers.ForEachActiveObject([&](auto * handler) {
            if (!handler->IsType(ReadHandler::InteractionType::Subscribe))
            {
                return Loop::Continue;
            }
            usedAttributePaths += handler->GetAttributePathCount();
            usedEventPaths += handler->GetEventPathCount();
            usedReadHandlers++;
            return Loop::Continue;
        });
    };

    countResourceUsage();

    if (usedAttributePaths + aRequestedAttributePathCount <= attributePathCap &&
        usedEventPaths + aRequestedEventPathCount <= eventPathCap && usedReadHandlers < readHandlerCap)
    {
        // We have enough resources, then we serve the requests in a best-effort manner.
        return true;
    }

    if ((aRequestedAttributePathCount > kMinSupportedPathsPerSubscription &&
         usedAttributePaths + aRequestedAttributePathCount > attributePathCap) ||
        (aRequestedEventPathCount > kMinSupportedPathsPerSubscription && usedEventPaths + aRequestedEventPathCount > eventPathCap))
    {
        // We cannot offer enough resources, and the subscription is requesting more than the spec limit.
        return false;
    }

    const auto evictAndUpdateResourceUsage = [&](FabricIndex fabricIndex, bool forceEvict) {
        bool ret = TrimFabric(fabricIndex, forceEvict);
        countResourceUsage();
        return ret;
    };

    //
    // At this point, we have an inbound request that respects minimas but we still don't have enough resources to handle it. Which
    // means that we definitely have handlers on existing fabrics that are over limits and need to evict at least one of them to
    // make space.
    //
    // There might be cases that one fabric has lots of subscriptions with one interested path, while the other fabrics are not
    // using excess resources. So we need to do this multiple times until we have enough room or no fabrics are using excess
    // resources.
    //
    bool didEvictHandler = true;
    while (didEvictHandler)
    {
        didEvictHandler = false;
        for (const auto & fabric : *mpFabricTable)
        {
            // The resources are enough to serve this request, do not evict anything.
            if (usedAttributePaths + aRequestedAttributePathCount <= attributePathCap &&
                usedEventPaths + aRequestedEventPathCount <= eventPathCap && usedReadHandlers < readHandlerCap)
            {
                break;
            }
            didEvictHandler = didEvictHandler || evictAndUpdateResourceUsage(fabric.GetFabricIndex(), false);
        }
    }

    // The above loop cannot guarantee the resources for the new subscriptions when the resource usage from all fabrics are exactly
    // within the quota (which means we have exactly used all resources). Evict (from the large subscriptions first then from
    // oldest) subscriptions from the current fabric until we have enough resource for the new subscription.
    didEvictHandler = true;
    while ((usedAttributePaths + aRequestedAttributePathCount > attributePathCap ||
            usedEventPaths + aRequestedEventPathCount > eventPathCap || usedReadHandlers >= readHandlerCap) &&
           // Avoid infinity loop
           didEvictHandler)
    {
        didEvictHandler = evictAndUpdateResourceUsage(aFabricIndex, true);
    }

    // If didEvictHandler is false, means the loop above evicted all subscriptions from the current fabric but we still don't have
    // enough resources for the new subscription, this should never happen.
    // This is safe as long as we have rejected subscriptions without a fabric associated (with a PASE session) before.
    // Note: Spec#5141: should reject subscription requests on PASE sessions.
    VerifyOrDieWithMsg(didEvictHandler, DataManagement, "Failed to get required resources by evicting existing subscriptions.");

    // We have ensured enough resources by the logic above.
    return true;
}

CHIP_ERROR InteractionModelEngine::CanEstablishReadTransaction(const ReadHandler * apReadHandler)
{
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK
#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    const bool allowUnlimited = !mForceHandlerQuota;
#else  // CONFIG_IM_BUILD_FOR_UNIT_TEST
       // If the resources are allocated on the heap, we should be able to handle as many Read / Subscribe requests as possible.
    const bool allowUnlimited = true;
#endif // CONFIG_IM_BUILD_FOR_UNIT_TEST
#else  // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK
    const bool allowUnlimited = false;
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK

    FabricIndex currentFabricIndex           = apReadHandler->GetAccessingFabricIndex();
    size_t activeReadHandlersOnCurrentFabric = 0;

    // It is safe to use & here since this function will be called on current stack.
    if (apReadHandler->GetAttributePathCount() > kReservedPathsPerReadRequest ||
        apReadHandler->GetEventPathCount() > kReservedPathsPerReadRequest ||
        apReadHandler->GetDataVersionFilterCount() > kReservedPathsPerReadRequest)
    {
        // Doesn't matter how many other reads are in progress; we are not
        // going to handle this request.
        return (allowUnlimited ? CHIP_NO_ERROR : CHIP_IM_GLOBAL_STATUS(PathsExhausted));
    }

    mReadHandlers.ForEachActiveObject([&](ReadHandler * handler) {
        if (handler->GetAccessingFabricIndex() == currentFabricIndex && handler->IsType(ReadHandler::InteractionType::Read))
        {
            activeReadHandlersOnCurrentFabric++;
        }
        return Loop::Continue;
    });

    // The incoming read handler here is also counted above.
    if (activeReadHandlersOnCurrentFabric > kReservedReadHandlersPerFabricForReadRequests)
    {
        return (allowUnlimited ? CHIP_NO_ERROR : CHIP_IM_GLOBAL_STATUS(Busy));
    }

    return CHIP_NO_ERROR;
}

void InteractionModelEngine::RemoveReadClient(ReadClient * apReadClient)
{
    ReadClient * pPrevListItem = nullptr;
    ReadClient * pCurListItem  = mpActiveReadClientList;

    while (pCurListItem != apReadClient)
    {
        //
        // Item must exist in this tracker list. If not, there's a bug somewhere.
        //
        VerifyOrDie(pCurListItem != nullptr);

        pPrevListItem = pCurListItem;
        pCurListItem  = pCurListItem->GetNextClient();
    }

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

bool InteractionModelEngine::HasConflictWriteRequests(const WriteHandler * apWriteHandler, const ConcreteAttributePath & aPath)
{
    for (auto & writeHandler : mWriteHandlers)
    {
        if (writeHandler.IsFree() || &writeHandler == apWriteHandler)
        {
            continue;
        }
        if (writeHandler.IsCurrentlyProcessingWritePath(aPath))
        {
            return true;
        }
    }
    return false;
}

void InteractionModelEngine::ReleaseAttributePathList(ObjectList<AttributePathParams> *& aAttributePathList)
{
    ReleasePool(aAttributePathList, mAttributePathPool);
}

CHIP_ERROR InteractionModelEngine::PushFrontAttributePathList(ObjectList<AttributePathParams> *& aAttributePathList,
                                                              AttributePathParams & aAttributePath)
{
    CHIP_ERROR err = PushFront(aAttributePathList, aAttributePath, mAttributePathPool);
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        ChipLogError(InteractionModel, "AttributePath pool full");
        return CHIP_IM_GLOBAL_STATUS(PathsExhausted);
    }
    return err;
}

void InteractionModelEngine::RemoveDuplicateConcreteAttributePath(ObjectList<AttributePathParams> *& aAttributePaths)
{
    ObjectList<AttributePathParams> * prev = nullptr;
    auto * path1                           = aAttributePaths;

    while (path1 != nullptr)
    {
        bool duplicate = false;
        // skip all wildcard paths and invalid concrete attribute
        if (path1->mValue.IsWildcardPath() ||
            !emberAfContainsAttribute(path1->mValue.mEndpointId, path1->mValue.mClusterId, path1->mValue.mAttributeId))
        {
            prev  = path1;
            path1 = path1->mpNext;
            continue;
        }

        // Check whether a wildcard path expands to something that includes this concrete path.
        for (auto * path2 = aAttributePaths; path2 != nullptr; path2 = path2->mpNext)
        {
            if (path2 == path1)
            {
                continue;
            }

            if (path2->mValue.IsWildcardPath() && path2->mValue.IsAttributePathSupersetOf(path1->mValue))
            {
                duplicate = true;
                break;
            }
        }

        // if path1 duplicates something from wildcard expansion, discard path1
        if (!duplicate)
        {
            prev  = path1;
            path1 = path1->mpNext;
            continue;
        }

        if (path1 == aAttributePaths)
        {
            aAttributePaths = path1->mpNext;
            mAttributePathPool.ReleaseObject(path1);
            path1 = aAttributePaths;
        }
        else
        {
            prev->mpNext = path1->mpNext;
            mAttributePathPool.ReleaseObject(path1);
            path1 = prev->mpNext;
        }
    }
}

void InteractionModelEngine::ReleaseEventPathList(ObjectList<EventPathParams> *& aEventPathList)
{
    ReleasePool(aEventPathList, mEventPathPool);
}

CHIP_ERROR InteractionModelEngine::PushFrontEventPathParamsList(ObjectList<EventPathParams> *& aEventPathList,
                                                                EventPathParams & aEventPath)
{
    CHIP_ERROR err = PushFront(aEventPathList, aEventPath, mEventPathPool);
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        ChipLogError(InteractionModel, "EventPath pool full");
        return CHIP_IM_GLOBAL_STATUS(PathsExhausted);
    }
    return err;
}

void InteractionModelEngine::ReleaseDataVersionFilterList(ObjectList<DataVersionFilter> *& aDataVersionFilterList)
{
    ReleasePool(aDataVersionFilterList, mDataVersionFilterPool);
}

CHIP_ERROR InteractionModelEngine::PushFrontDataVersionFilterList(ObjectList<DataVersionFilter> *& aDataVersionFilterList,
                                                                  DataVersionFilter & aDataVersionFilter)
{
    CHIP_ERROR err = PushFront(aDataVersionFilterList, aDataVersionFilter, mDataVersionFilterPool);
    if (err == CHIP_ERROR_NO_MEMORY)
    {
        ChipLogError(InteractionModel, "DataVersionFilter pool full, ignore this filter");
        err = CHIP_NO_ERROR;
    }
    return err;
}

template <typename T, size_t N>
void InteractionModelEngine::ReleasePool(ObjectList<T> *& aObjectList, ObjectPool<ObjectList<T>, N> & aObjectPool)
{
    ObjectList<T> * current = aObjectList;
    while (current != nullptr)
    {
        ObjectList<T> * next = current->mpNext;
        aObjectPool.ReleaseObject(current);
        current = next;
    }

    aObjectList = nullptr;
}

template <typename T, size_t N>
CHIP_ERROR InteractionModelEngine::PushFront(ObjectList<T> *& aObjectList, T & aData, ObjectPool<ObjectList<T>, N> & aObjectPool)
{
    ObjectList<T> * object = aObjectPool.CreateObject();
    if (object == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    object->mValue = aData;
    object->mpNext = aObjectList;
    aObjectList    = object;
    return CHIP_NO_ERROR;
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

uint16_t InteractionModelEngine::GetMinSubscriptionsPerFabric() const
{
    uint8_t fabricCount                  = mpFabricTable->FabricCount();
    const size_t readHandlerPoolCapacity = GetReadHandlerPoolCapacity();

    if (fabricCount == 0)
    {
        return kMinSupportedSubscriptionsPerFabric;
    }

    size_t perFabricSubscriptionCapacity = static_cast<size_t>(readHandlerPoolCapacity - kReservedPathsForReads) / fabricCount;

    return static_cast<uint16_t>(perFabricSubscriptionCapacity);
}

size_t InteractionModelEngine::GetNumDirtySubscriptions() const
{
    size_t numDirtySubscriptions = 0;
    mReadHandlers.ForEachActiveObject([&](const auto readHandler) {
        if (readHandler->IsType(ReadHandler::InteractionType::Subscribe) && readHandler->IsDirty())
        {
            numDirtySubscriptions++;
        }
        return Loop::Continue;
    });
    return numDirtySubscriptions;
}

} // namespace app
} // namespace chip
