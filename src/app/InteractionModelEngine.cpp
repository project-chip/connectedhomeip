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

#include "access/RequestPath.h"
#include "access/SubjectDescriptor.h"
#include <app/RequiredPrivilege.h>
#include <lib/core/CHIPTLVUtilities.hpp>
#include <lib/support/CodeUtils.h>

extern bool emberAfContainsAttribute(chip::EndpointId endpoint, chip::ClusterId clusterId, chip::AttributeId attributeId);

namespace chip {
namespace app {

using Protocols::InteractionModel::Status;

InteractionModelEngine sInteractionModelEngine;

InteractionModelEngine::InteractionModelEngine() {}

InteractionModelEngine * InteractionModelEngine::GetInstance()
{
    return &sInteractionModelEngine;
}

CHIP_ERROR InteractionModelEngine::Init(Messaging::ExchangeManager * apExchangeMgr, FabricTable * apFabricTable,
                                        CASESessionManager * apCASESessionMgr)
{
    VerifyOrReturnError(apFabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(apExchangeMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mpExchangeMgr    = apExchangeMgr;
    mpFabricTable    = apFabricTable;
    mpCASESessionMgr = apCASESessionMgr;

    ReturnErrorOnFailure(mpFabricTable->AddFabricDelegate(this));
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
        CommandHandlerInterface * nextHandler = handlerIter->GetNext();
        handlerIter->SetNext(nullptr);
        handlerIter = nextHandler;
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

    // Shut down any subscription clients that are still around.  They won't be
    // able to work after this point anyway, since we're about to drop our refs
    // to them.
    ShutdownAllSubscriptions();

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

    mpCASESessionMgr = nullptr;

    //
    // We _should_ be clearing these out, but doing so invites a world
    // of trouble. #21233 tracks fixing the underlying assumptions to make
    // this possible.
    //
    // mpFabricTable    = nullptr;
    // mpExchangeMgr    = nullptr;
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

WriteHandler * InteractionModelEngine::ActiveWriteHandlerAt(unsigned int aIndex)
{
    unsigned int i = 0;

    for (auto & writeHandler : mWriteHandlers)
    {
        if (!writeHandler.IsFree())
        {
            if (i == aIndex)
            {
                return &writeHandler;
            }
            i++;
        }
    }
    return nullptr;
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

CHIP_ERROR InteractionModelEngine::ShutdownSubscription(const ScopedNodeId & aPeerNodeId, SubscriptionId aSubscriptionId)
{
    for (auto * readClient = mpActiveReadClientList; readClient != nullptr; readClient = readClient->GetNextClient())
    {
        if (readClient->IsSubscriptionType() && readClient->IsMatchingSubscriptionId(aSubscriptionId) &&
            readClient->GetFabricIndex() == aPeerNodeId.GetFabricIndex() && readClient->GetPeerNodeId() == aPeerNodeId.GetNodeId())
        {
            readClient->Close(CHIP_NO_ERROR);
            return CHIP_NO_ERROR;
        }
    }

    return CHIP_ERROR_KEY_NOT_FOUND;
}

void InteractionModelEngine::ShutdownSubscriptions(FabricIndex aFabricIndex, NodeId aPeerNodeId)
{
    ShutdownMatchingSubscriptions(MakeOptional(aFabricIndex), MakeOptional(aPeerNodeId));
}
void InteractionModelEngine::ShutdownSubscriptions(FabricIndex aFabricIndex)
{
    ShutdownMatchingSubscriptions(MakeOptional(aFabricIndex));
}

void InteractionModelEngine::ShutdownAllSubscriptions()
{
    ShutdownMatchingSubscriptions();
}

void InteractionModelEngine::ShutdownMatchingSubscriptions(const Optional<FabricIndex> & aFabricIndex,
                                                           const Optional<NodeId> & aPeerNodeId)
{
    // This is assuming that ReadClient::Close will not affect any other
    // ReadClients in the list.
    for (auto * readClient = mpActiveReadClientList; readClient != nullptr;)
    {
        // Grab the next client now, because we might be about to delete readClient.
        auto * nextClient = readClient->GetNextClient();
        if (readClient->IsSubscriptionType())
        {
            bool fabricMatches = !aFabricIndex.HasValue() || (aFabricIndex.Value() == readClient->GetFabricIndex());
            bool nodeIdMatches = !aPeerNodeId.HasValue() || (aPeerNodeId.Value() == readClient->GetPeerNodeId());
            if (fabricMatches && nodeIdMatches)
            {
                readClient->Close(CHIP_NO_ERROR);
            }
        }
        readClient = nextClient;
    }
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

Status InteractionModelEngine::OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext,
                                                      const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload,
                                                      bool aIsTimedInvoke)
{
    CommandHandler * commandHandler = mCommandHandlerObjs.CreateObject(this);
    if (commandHandler == nullptr)
    {
        ChipLogProgress(InteractionModel, "no resource for Invoke interaction");
        return Status::Busy;
    }
    commandHandler->OnInvokeCommandRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), aIsTimedInvoke);
    return Status::Success;
}

CHIP_ERROR InteractionModelEngine::ParseAttributePaths(const Access::SubjectDescriptor & aSubjectDescriptor,
                                                       AttributePathIBs::Parser & aAttributePathListParser,
                                                       bool & aHasValidAttributePath, size_t & aRequestedAttributePathCount)
{
    TLV::TLVReader pathReader;
    aAttributePathListParser.GetReader(&pathReader);
    CHIP_ERROR err = CHIP_NO_ERROR;

    aHasValidAttributePath       = false;
    aRequestedAttributePathCount = 0;

    while (CHIP_NO_ERROR == (err = pathReader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag() == pathReader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);

        AttributePathIB::Parser path;
        //
        // We create an iterator to point to a single item object list that tracks the path we just parsed.
        // This avoids the 'parse all paths' approach that is employed in ReadHandler since we want to
        // avoid allocating out of the path store during this minimal initial processing stage.
        //
        ObjectList<AttributePathParams> paramsList;

        ReturnErrorOnFailure(path.Init(pathReader));
        ReturnErrorOnFailure(path.ParsePath(paramsList.mValue));

        if (paramsList.mValue.IsWildcardPath())
        {
            AttributePathExpandIterator pathIterator(&paramsList);
            ConcreteAttributePath readPath;

            // The definition of "valid path" is "path exists and ACL allows access". The "path exists" part is handled by
            // AttributePathExpandIterator. So we just need to check the ACL bits.
            for (; pathIterator.Get(readPath); pathIterator.Next())
            {
                Access::RequestPath requestPath{ .cluster = readPath.mClusterId, .endpoint = readPath.mEndpointId };
                err = Access::GetAccessControl().Check(aSubjectDescriptor, requestPath,
                                                       RequiredPrivilege::ForReadAttribute(readPath));
                if (err == CHIP_NO_ERROR)
                {
                    aHasValidAttributePath = true;
                    break;
                }
            }
        }
        else
        {
            ConcreteAttributePath concretePath(paramsList.mValue.mEndpointId, paramsList.mValue.mClusterId,
                                               paramsList.mValue.mAttributeId);
            if (ConcreteAttributePathExists(concretePath))
            {
                Access::RequestPath requestPath{ .cluster = concretePath.mClusterId, .endpoint = concretePath.mEndpointId };

                err = Access::GetAccessControl().Check(aSubjectDescriptor, requestPath,
                                                       RequiredPrivilege::ForReadAttribute(concretePath));
                if (err == CHIP_NO_ERROR)
                {
                    aHasValidAttributePath = true;
                }
            }
        }

        aRequestedAttributePathCount++;
    }

    if (err == CHIP_ERROR_END_OF_TLV)
    {
        err = CHIP_NO_ERROR;
    }

    return err;
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
        VerifyOrReturnError(subscribeRequestParser.Init(reader) == CHIP_NO_ERROR, Status::InvalidAction);
        {
            size_t requestedAttributePathCount = 0;
            size_t requestedEventPathCount     = 0;
            AttributePathIBs::Parser attributePathListParser;
            bool hasValidAttributePath = false;

            CHIP_ERROR err = subscribeRequestParser.GetAttributeRequests(&attributePathListParser);
            if (err == CHIP_NO_ERROR)
            {
                auto subjectDescriptor = apExchangeContext->GetSessionHandle()->AsSecureSession()->GetSubjectDescriptor();
                err                    = ParseAttributePaths(subjectDescriptor, attributePathListParser, hasValidAttributePath,
                                          requestedAttributePathCount);
                if (err != CHIP_NO_ERROR)
                {
                    return Status::InvalidAction;
                }
            }
            else if (err != CHIP_ERROR_END_OF_TLV)
            {
                return Status::InvalidAction;
            }

            EventPathIBs::Parser eventpathListParser;
            err = subscribeRequestParser.GetEventRequests(&eventpathListParser);
            if (err == CHIP_NO_ERROR)
            {
                TLV::TLVReader pathReader;
                eventpathListParser.GetReader(&pathReader);
                ReturnErrorCodeIf(TLV::Utilities::Count(pathReader, requestedEventPathCount, false) != CHIP_NO_ERROR,
                                  Status::InvalidAction);
            }
            else if (err != CHIP_ERROR_END_OF_TLV)
            {
                return Status::InvalidAction;
            }

            if (requestedAttributePathCount == 0 && requestedEventPathCount == 0)
            {
                ChipLogError(InteractionModel,
                             "Subscription from [%u:" ChipLogFormatX64 "] has no attribute or event paths. Rejecting request.",
                             apExchangeContext->GetSessionHandle()->GetFabricIndex(),
                             ChipLogValueX64(apExchangeContext->GetSessionHandle()->AsSecureSession()->GetPeerNodeId()));
                return Status::InvalidAction;
            }

            //
            // TODO: We don't have an easy way to do a similar 'path expansion' for events to deduce
            // access so for now, assume that the presence of any path in the event list means they
            // might have some access to those events.
            //
            if (!hasValidAttributePath && requestedEventPathCount == 0)
            {
                ChipLogError(InteractionModel,
                             "Subscription from [%u:" ChipLogFormatX64 "] has no access at all. Rejecting request.",
                             apExchangeContext->GetSessionHandle()->GetFabricIndex(),
                             ChipLogValueX64(apExchangeContext->GetSessionHandle()->AsSecureSession()->GetPeerNodeId()));
                return Status::InvalidAction;
            }

            // The following cast is safe, since we can only hold a few tens of paths in one request.
            if (!EnsureResourceForSubscription(apExchangeContext->GetSessionHandle()->GetFabricIndex(), requestedAttributePathCount,
                                               requestedEventPathCount))
            {
                return Status::PathsExhausted;
            }
        }

        VerifyOrReturnError(subscribeRequestParser.GetKeepSubscriptions(&keepExistingSubscriptions) == CHIP_NO_ERROR,
                            Status::InvalidAction);
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
    else
    {
        System::PacketBufferTLVReader reader;
        reader.Init(aPayload.Retain());

        ReadRequestMessage::Parser readRequestParser;
        VerifyOrReturnError(readRequestParser.Init(reader) == CHIP_NO_ERROR, Status::InvalidAction);

        {
            size_t requestedAttributePathCount = 0;
            size_t requestedEventPathCount     = 0;
            AttributePathIBs::Parser attributePathListParser;
            CHIP_ERROR err = readRequestParser.GetAttributeRequests(&attributePathListParser);
            if (err == CHIP_NO_ERROR)
            {
                TLV::TLVReader pathReader;
                attributePathListParser.GetReader(&pathReader);
                ReturnErrorCodeIf(TLV::Utilities::Count(pathReader, requestedAttributePathCount, false) != CHIP_NO_ERROR,
                                  Status::InvalidAction);
            }
            else if (err != CHIP_ERROR_END_OF_TLV)
            {
                return Status::InvalidAction;
            }
            EventPathIBs::Parser eventpathListParser;
            err = readRequestParser.GetEventRequests(&eventpathListParser);
            if (err == CHIP_NO_ERROR)
            {
                TLV::TLVReader pathReader;
                eventpathListParser.GetReader(&pathReader);
                ReturnErrorCodeIf(TLV::Utilities::Count(pathReader, requestedEventPathCount, false) != CHIP_NO_ERROR,
                                  Status::InvalidAction);
            }
            else if (err != CHIP_ERROR_END_OF_TLV)
            {
                return Status::InvalidAction;
            }

            // The following cast is safe, since we can only hold a few tens of paths in one request.
            Status checkResult = EnsureResourceForRead(apExchangeContext->GetSessionHandle()->GetFabricIndex(),
                                                       requestedAttributePathCount, requestedEventPathCount);
            if (checkResult != Status::Success)
            {
                return checkResult;
            }
        }
    }

    // We have already reserved enough resources for read requests, and have granted enough resources for current subscriptions, so
    // we should be able to allocate resources requested by this request.
    ReadHandler * handler = mReadHandlers.CreateObject(*this, apExchangeContext, aInteractionType);
    if (handler == nullptr)
    {
        ChipLogProgress(InteractionModel, "no resource for %s interaction",
                        aInteractionType == ReadHandler::InteractionType::Subscribe ? "Subscribe" : "Read");
        return Status::ResourceExhausted;
    }

    handler->OnInitialRequest(std::move(aPayload));

    return Status::Success;
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
        aStatus = Status::Busy;
        return CHIP_ERROR_NO_MEMORY;
    }

    // The timed handler takes over handling of this exchange and will do its
    // own status reporting as needed.
    aStatus = Status::Success;
    apExchangeContext->SetDelegate(handler);
    return handler->OnMessageReceived(apExchangeContext, aPayloadHeader, std::move(aPayload));
}

Status InteractionModelEngine::OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext,
                                                       const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    System::PacketBufferTLVReader reader;
    reader.Init(aPayload.Retain());

    ReportDataMessage::Parser report;
    VerifyOrReturnError(report.Init(reader) == CHIP_NO_ERROR, Status::InvalidAction);

    SubscriptionId subscriptionId = 0;
    VerifyOrReturnError(report.GetSubscriptionId(&subscriptionId) == CHIP_NO_ERROR, Status::InvalidAction);
    VerifyOrReturnError(report.ExitContainer() == CHIP_NO_ERROR, Status::InvalidAction);

    for (auto * readClient = mpActiveReadClientList; readClient != nullptr; readClient = readClient->GetNextClient())
    {
        if (!readClient->IsSubscriptionActive())
        {
            continue;
        }
        auto peer = apExchangeContext->GetSessionHandle()->GetPeer();
        if (readClient->GetFabricIndex() != peer.GetFabricIndex() || readClient->GetPeerNodeId() != peer.GetNodeId() ||
            !readClient->IsMatchingSubscriptionId(subscriptionId))
        {
            continue;
        }
        readClient->OnUnsolicitedReportData(apExchangeContext, std::move(aPayload));
        return Status::Success;
    }

    return Status::InvalidSubscription;
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

    Protocols::InteractionModel::Status status = Status::Failure;

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
        status = OnInvokeCommandRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), /* aIsTimedInvoke = */ false);
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
        status = OnUnsolicitedReportData(apExchangeContext, aPayloadHeader, std::move(aPayload));
    }
    else if (aPayloadHeader.HasMessageType(MsgType::TimedRequest))
    {
        OnTimedRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), status);
    }
    else
    {
        ChipLogProgress(InteractionModel, "Msg type %d not supported", aPayloadHeader.GetMessageType());
        status = Status::InvalidAction;
    }

    if (status != Status::Success && !apExchangeContext->IsGroupExchangeContext())
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

bool InteractionModelEngine::TrimFabricForSubscriptions(FabricIndex aFabricIndex, bool aForceEvict)
{
    const size_t pathPoolCapacity        = GetPathPoolCapacityForSubscriptions();
    const size_t readHandlerPoolCapacity = GetReadHandlerPoolCapacityForSubscriptions();

    uint8_t fabricCount                            = mpFabricTable->FabricCount();
    size_t attributePathsSubscribedByCurrentFabric = 0;
    size_t eventPathsSubscribedByCurrentFabric     = 0;
    size_t subscriptionsEstablishedByCurrentFabric = 0;

    if (fabricCount == 0)
    {
        return false;
    }

    // Note: This is OK only when we have assumed the fabricCount is not zero. Should be revised when adding support to
    // subscriptions on PASE sessions.
    size_t perFabricPathCapacity         = pathPoolCapacity / static_cast<size_t>(fabricCount);
    size_t perFabricSubscriptionCapacity = readHandlerPoolCapacity / static_cast<size_t>(fabricCount);

    ReadHandler * candidate            = nullptr;
    size_t candidateAttributePathsUsed = 0;
    size_t candidateEventPathsUsed     = 0;

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
                 (candidateAttributePathsUsed <= perFabricPathCapacity && candidateEventPathsUsed <= perFabricPathCapacity))
        {
            candidate                   = handler;
            candidateAttributePathsUsed = attributePathsUsed;
            candidateEventPathsUsed     = eventPathsUsed;
        }
        // This handler is older than the one we picked before.
        else if (handler->GetTransactionStartGeneration() < candidate->GetTransactionStartGeneration() &&
                 // And the level of resource usage is the same (both exceed or neither exceed)
                 ((attributePathsUsed > perFabricPathCapacity || eventPathsUsed > perFabricPathCapacity) ==
                  (candidateAttributePathsUsed > perFabricPathCapacity || candidateEventPathsUsed > perFabricPathCapacity)))
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
        candidate->Close();
        return true;
    }
    return false;
}

bool InteractionModelEngine::EnsureResourceForSubscription(FabricIndex aFabricIndex, size_t aRequestedAttributePathCount,
                                                           size_t aRequestedEventPathCount)
{
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    const bool allowUnlimited = !mForceHandlerQuota;
#else  // CONFIG_BUILD_FOR_HOST_UNIT_TEST
       // If the resources are allocated on the heap, we should be able to handle as many Read / Subscribe requests as possible.
    const bool allowUnlimited = true;
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
#else  // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK
    const bool allowUnlimited = false;
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK

    // Don't couple with read requests, always reserve enough resource for read requests.

    const size_t pathPoolCapacity        = GetPathPoolCapacityForSubscriptions();
    const size_t readHandlerPoolCapacity = GetReadHandlerPoolCapacityForSubscriptions();

    // If we return early here, the compiler will complain about the unreachable code, so we add a always-true check.
    const size_t attributePathCap = allowUnlimited ? SIZE_MAX : pathPoolCapacity;
    const size_t eventPathCap     = allowUnlimited ? SIZE_MAX : pathPoolCapacity;
    const size_t readHandlerCap   = allowUnlimited ? SIZE_MAX : readHandlerPoolCapacity;

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
        bool ret = TrimFabricForSubscriptions(fabricIndex, forceEvict);
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

bool InteractionModelEngine::TrimFabricForRead(FabricIndex aFabricIndex)
{
    const size_t guaranteedReadRequestsPerFabric   = GetGuaranteedReadRequestsPerFabric();
    const size_t minSupportedPathsPerFabricForRead = guaranteedReadRequestsPerFabric * kMinSupportedPathsPerReadRequest;

    size_t attributePathsUsedByCurrentFabric = 0;
    size_t eventPathsUsedByCurrentFabric     = 0;
    size_t readTransactionsOnCurrentFabric   = 0;

    ReadHandler * candidate            = nullptr;
    size_t candidateAttributePathsUsed = 0;
    size_t candidateEventPathsUsed     = 0;

    // It is safe to use & here since this function will be called on current stack.
    mReadHandlers.ForEachActiveObject([&](ReadHandler * handler) {
        if (handler->GetAccessingFabricIndex() != aFabricIndex || !handler->IsType(ReadHandler::InteractionType::Read))
        {
            return Loop::Continue;
        }

        size_t attributePathsUsed = handler->GetAttributePathCount();
        size_t eventPathsUsed     = handler->GetEventPathCount();

        attributePathsUsedByCurrentFabric += attributePathsUsed;
        eventPathsUsedByCurrentFabric += eventPathsUsed;
        readTransactionsOnCurrentFabric++;

        if (candidate == nullptr)
        {
            candidate = handler;
        }
        // Oversized read handlers will be evicted first.
        else if ((attributePathsUsed > kMinSupportedPathsPerReadRequest || eventPathsUsed > kMinSupportedPathsPerReadRequest) &&
                 (candidateAttributePathsUsed <= kMinSupportedPathsPerReadRequest &&
                  candidateEventPathsUsed <= kMinSupportedPathsPerReadRequest))
        {
            candidate = handler;
        }
        // Read Handlers are "first come first served", so we give eariler read transactions a higher priority.
        else if (handler->GetTransactionStartGeneration() > candidate->GetTransactionStartGeneration() &&
                 // And the level of resource usage is the same (both exceed or neither exceed)
                 ((attributePathsUsed > kMinSupportedPathsPerReadRequest || eventPathsUsed > kMinSupportedPathsPerReadRequest) ==
                  (candidateAttributePathsUsed > kMinSupportedPathsPerReadRequest ||
                   candidateEventPathsUsed > kMinSupportedPathsPerReadRequest)))
        {
            candidate = handler;
        }

        if (candidate == handler)
        {
            candidateAttributePathsUsed = attributePathsUsed;
            candidateEventPathsUsed     = eventPathsUsed;
        }
        return Loop::Continue;
    });

    if (candidate != nullptr &&
        ((attributePathsUsedByCurrentFabric > minSupportedPathsPerFabricForRead ||
          eventPathsUsedByCurrentFabric > minSupportedPathsPerFabricForRead ||
          readTransactionsOnCurrentFabric > guaranteedReadRequestsPerFabric) ||
         // Always evict the transactions on PASE sessions if the fabric table is full.
         (aFabricIndex == kUndefinedFabricIndex && mpFabricTable->FabricCount() == GetConfigMaxFabrics())))
    {
        candidate->Close();
        return true;
    }
    return false;
}

Protocols::InteractionModel::Status InteractionModelEngine::EnsureResourceForRead(FabricIndex aFabricIndex,
                                                                                  size_t aRequestedAttributePathCount,
                                                                                  size_t aRequestedEventPathCount)
{
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    const bool allowUnlimited = !mForceHandlerQuota;
#else  // CONFIG_BUILD_FOR_HOST_UNIT_TEST
       // If the resources are allocated on the heap, we should be able to handle as many Read / Subscribe requests as possible.
    const bool allowUnlimited = true;
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST
#else  // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK
    const bool allowUnlimited = false;
#endif // CHIP_SYSTEM_CONFIG_POOL_USE_HEAP && !CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK

    // If we return early here, the compiler will complain about the unreachable code, so we add a always-true check.
    const size_t attributePathCap = allowUnlimited ? SIZE_MAX : GetPathPoolCapacityForReads();
    const size_t eventPathCap     = allowUnlimited ? SIZE_MAX : GetPathPoolCapacityForReads();
    const size_t readHandlerCap   = allowUnlimited ? SIZE_MAX : GetReadHandlerPoolCapacityForReads();

    const size_t guaranteedReadRequestsPerFabric = GetGuaranteedReadRequestsPerFabric();
    const size_t guaranteedPathsPerFabric        = kMinSupportedPathsPerReadRequest * guaranteedReadRequestsPerFabric;

    size_t usedAttributePaths = 0;
    size_t usedEventPaths     = 0;
    size_t usedReadHandlers   = 0;

    auto countResourceUsage = [&]() {
        usedAttributePaths = 0;
        usedEventPaths     = 0;
        usedReadHandlers   = 0;
        mReadHandlers.ForEachActiveObject([&](auto * handler) {
            if (!handler->IsType(ReadHandler::InteractionType::Read))
            {
                return Loop::Continue;
            }
            usedAttributePaths += handler->GetAttributePathCount();
            usedEventPaths += handler->GetEventPathCount();
            usedReadHandlers++;
            return Loop::Continue;
        });
    };

    auto haveEnoughResourcesForTheRequest = [&]() {
        return usedAttributePaths + aRequestedAttributePathCount <= attributePathCap &&
            usedEventPaths + aRequestedEventPathCount <= eventPathCap && usedReadHandlers < readHandlerCap;
    };

    countResourceUsage();

    if (haveEnoughResourcesForTheRequest())
    {
        // We have enough resources, then we serve the requests in a best-effort manner.
        return Status::Success;
    }

    if ((aRequestedAttributePathCount > kMinSupportedPathsPerReadRequest &&
         usedAttributePaths + aRequestedAttributePathCount > attributePathCap) ||
        (aRequestedEventPathCount > kMinSupportedPathsPerReadRequest && usedEventPaths + aRequestedEventPathCount > eventPathCap))
    {
        // We cannot offer enough resources, and the read transaction is requesting more than the spec limit.
        return Status::PathsExhausted;
    }

    // If we have commissioned CHIP_CONFIG_MAX_FABRICS already, and this transaction doesn't have an associated fabric index, reject
    // the request if we don't have sufficient resources for this request.
    if (mpFabricTable->FabricCount() == GetConfigMaxFabrics() && aFabricIndex == kUndefinedFabricIndex)
    {
        return Status::Busy;
    }

    size_t usedAttributePathsInFabric = 0;
    size_t usedEventPathsInFabric     = 0;
    size_t usedReadHandlersInFabric   = 0;
    mReadHandlers.ForEachActiveObject([&](auto * handler) {
        if (!handler->IsType(ReadHandler::InteractionType::Read) || handler->GetAccessingFabricIndex() != aFabricIndex)
        {
            return Loop::Continue;
        }
        usedAttributePathsInFabric += handler->GetAttributePathCount();
        usedEventPathsInFabric += handler->GetEventPathCount();
        usedReadHandlersInFabric++;
        return Loop::Continue;
    });

    // Busy, since there are already some read requests ongoing on this fabric, please retry later.
    if (usedAttributePathsInFabric + aRequestedAttributePathCount > guaranteedPathsPerFabric ||
        usedEventPathsInFabric + aRequestedEventPathCount > guaranteedPathsPerFabric ||
        usedReadHandlersInFabric >= guaranteedReadRequestsPerFabric)
    {
        return Status::Busy;
    }

    const auto evictAndUpdateResourceUsage = [&](FabricIndex fabricIndex) {
        bool ret = TrimFabricForRead(fabricIndex);
        countResourceUsage();
        return ret;
    };

    //
    // At this point, we have an inbound request that respects minimas but we still don't have enough resources to handle it. Which
    // means that we definitely have handlers on existing fabrics that are over limits and need to evict at least one of them to
    // make space.
    //
    bool didEvictHandler = true;
    while (didEvictHandler)
    {
        didEvictHandler = false;
        didEvictHandler = didEvictHandler || evictAndUpdateResourceUsage(kUndefinedFabricIndex);
        if (haveEnoughResourcesForTheRequest())
        {
            break;
        }
        // If the fabric table is full, we won't evict read requests from normal fabrics before we have evicted all read requests
        // from PASE sessions.
        if (mpFabricTable->FabricCount() == GetConfigMaxFabrics() && didEvictHandler)
        {
            continue;
        }
        for (const auto & fabric : *mpFabricTable)
        {
            didEvictHandler = didEvictHandler || evictAndUpdateResourceUsage(fabric.GetFabricIndex());
            // If we now have enough resources to serve this request, stop evicting things.
            if (haveEnoughResourcesForTheRequest())
            {
                break;
            }
        }
    }

    // Now all fabrics are not oversized (since we have trimmed the oversized fabrics in the loop above), and the read handler is
    // also not oversized, we should be able to handle this read transaction.
    VerifyOrDie(haveEnoughResourcesForTheRequest());

    return Status::Success;
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
        ObjectList<T> * nextObject = current->mpNext;
        aObjectPool.ReleaseObject(current);
        current = nextObject;
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

    Status status = OnInvokeCommandRequest(apExchangeContext, aPayloadHeader, std::move(aPayload), /* aIsTimedInvoke = */ true);
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

uint16_t InteractionModelEngine::GetMinGuaranteedSubscriptionsPerFabric() const
{
#if CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    return UINT16_MAX;
#else
    return static_cast<uint16_t>(
        min(GetReadHandlerPoolCapacityForSubscriptions() / GetConfigMaxFabrics(), static_cast<size_t>(UINT16_MAX)));
#endif
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

void InteractionModelEngine::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    mReadHandlers.ForEachActiveObject([fabricIndex](ReadHandler * handler) {
        if (handler->GetAccessingFabricIndex() == fabricIndex)
        {
            ChipLogProgress(InteractionModel, "Deleting expired ReadHandler for NodeId: " ChipLogFormatX64 ", FabricIndex: %u",
                            ChipLogValueX64(handler->GetInitiatorNodeId()), fabricIndex);
            handler->Close();
        }

        return Loop::Continue;
    });

    for (auto * readClient = mpActiveReadClientList; readClient != nullptr; readClient = readClient->GetNextClient())
    {
        if (readClient->GetFabricIndex() == fabricIndex)
        {
            ChipLogProgress(InteractionModel, "Fabric removed, deleting obsolete read client with FabricIndex: %u", fabricIndex);
            readClient->Close(CHIP_ERROR_IM_FABRIC_DELETED, false);
        }
    }

    for (auto & handler : mWriteHandlers)
    {
        if (!(handler.IsFree()) && handler.GetAccessingFabricIndex() == fabricIndex)
        {
            ChipLogProgress(InteractionModel, "Fabric removed, deleting obsolete write handler with FabricIndex: %u", fabricIndex);
            handler.Close();
        }
    }

    // Applications may hold references to CommandHandler instances for async command processing.
    // Therefore we can't forcible destroy CommandHandlers here.  Their exchanges will get closed by
    // the fabric removal, though, so they will fail when they try to actually send their command response
    // and will close at that point.
}
} // namespace app
} // namespace chip
