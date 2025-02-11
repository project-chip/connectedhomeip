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

#include <access/AccessRestrictionProvider.h>
#include <access/Privilege.h>
#include <access/RequestPath.h>
#include <access/SubjectDescriptor.h>
#include <app/AppConfig.h>
#include <app/CommandHandlerInterfaceRegistry.h>
#include <app/ConcreteClusterPath.h>
#include <app/EventPathParams.h>
#include <app/RequiredPrivilege.h>
#include <app/data-model-provider/ActionReturnStatus.h>
#include <app/data-model-provider/MetadataList.h>
#include <app/data-model-provider/MetadataLookup.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/data-model/List.h>
#include <app/util/IMClusterCommandHandler.h>
#include <app/util/af-types.h>
#include <app/util/endpoint-config-api.h>
#include <lib/core/CHIPError.h>
#include <lib/core/DataModelTypes.h>
#include <lib/core/Global.h>
#include <lib/core/TLVUtilities.h>
#include <lib/support/CHIPFaultInjection.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/FibonacciUtils.h>
#include <protocols/interaction_model/StatusCode.h>

namespace chip {
namespace app {
namespace {

/**
 * Helper to handle wildcard events in the event path.
 *
 * Validates that ACL access is permitted to:
 *    - Cluster::View in case the path is a wildcard for the event id
 *    - Event read if the path is a concrete event path
 */
bool MayHaveAccessibleEventPathForEndpointAndCluster(const ConcreteClusterPath & path, const EventPathParams & aEventPath,
                                                     const Access::SubjectDescriptor & aSubjectDescriptor)
{
    Access::RequestPath requestPath{ .cluster     = path.mClusterId,
                                     .endpoint    = path.mEndpointId,
                                     .requestType = Access::RequestType::kEventReadRequest };

    Access::Privilege requiredPrivilege = Access::Privilege::kView;

    if (!aEventPath.HasWildcardEventId())
    {
        requestPath.entityId = aEventPath.mEventId;
        requiredPrivilege =
            RequiredPrivilege::ForReadEvent(ConcreteEventPath(path.mEndpointId, path.mClusterId, aEventPath.mEventId));
    }

    return (Access::GetAccessControl().Check(aSubjectDescriptor, requestPath, requiredPrivilege) == CHIP_NO_ERROR);
}

bool MayHaveAccessibleEventPathForEndpoint(DataModel::Provider * aProvider, EndpointId aEndpoint,
                                           const EventPathParams & aEventPath, const Access::SubjectDescriptor & aSubjectDescriptor)
{
    if (!aEventPath.HasWildcardClusterId())
    {
        return MayHaveAccessibleEventPathForEndpointAndCluster(ConcreteClusterPath(aEndpoint, aEventPath.mClusterId), aEventPath,
                                                               aSubjectDescriptor);
    }

    for (auto & cluster : aProvider->ServerClustersIgnoreError(aEventPath.mEndpointId))
    {
        if (MayHaveAccessibleEventPathForEndpointAndCluster(ConcreteClusterPath(aEventPath.mEndpointId, cluster.clusterId),
                                                            aEventPath, aSubjectDescriptor))
        {
            return true;
        }
    }

    return false;
}

bool MayHaveAccessibleEventPath(DataModel::Provider * aProvider, const EventPathParams & aEventPath,
                                const Access::SubjectDescriptor & subjectDescriptor)
{
    VerifyOrReturnValue(aProvider != nullptr, false);

    if (!aEventPath.HasWildcardEndpointId())
    {
        return MayHaveAccessibleEventPathForEndpoint(aProvider, aEventPath.mEndpointId, aEventPath, subjectDescriptor);
    }

    for (const DataModel::EndpointEntry & ep : aProvider->EndpointsIgnoreError())
    {
        if (MayHaveAccessibleEventPathForEndpoint(aProvider, ep.id, aEventPath, subjectDescriptor))
        {
            return true;
        }
    }
    return false;
}

} // namespace

class AutoReleaseSubscriptionInfoIterator
{
public:
    AutoReleaseSubscriptionInfoIterator(SubscriptionResumptionStorage::SubscriptionInfoIterator * iterator) : mIterator(iterator){};
    ~AutoReleaseSubscriptionInfoIterator() { mIterator->Release(); }

    SubscriptionResumptionStorage::SubscriptionInfoIterator * operator->() const { return mIterator; }

private:
    SubscriptionResumptionStorage::SubscriptionInfoIterator * mIterator;
};

using Protocols::InteractionModel::Status;

Global<InteractionModelEngine> sInteractionModelEngine;

InteractionModelEngine::InteractionModelEngine() : mReportingEngine(this) {}

InteractionModelEngine * InteractionModelEngine::GetInstance()
{
    return &sInteractionModelEngine.get();
}

CHIP_ERROR InteractionModelEngine::Init(Messaging::ExchangeManager * apExchangeMgr, FabricTable * apFabricTable,
                                        reporting::ReportScheduler * reportScheduler, CASESessionManager * apCASESessionMgr,
                                        SubscriptionResumptionStorage * subscriptionResumptionStorage,
                                        EventManagement * eventManagement)
{
    VerifyOrReturnError(apFabricTable != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(apExchangeMgr != nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(reportScheduler != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    mState                          = State::kInitializing;
    mpExchangeMgr                   = apExchangeMgr;
    mpFabricTable                   = apFabricTable;
    mpCASESessionMgr                = apCASESessionMgr;
    mpSubscriptionResumptionStorage = subscriptionResumptionStorage;
    mReportScheduler                = reportScheduler;

    ReturnErrorOnFailure(mpFabricTable->AddFabricDelegate(this));
    ReturnErrorOnFailure(mpExchangeMgr->RegisterUnsolicitedMessageHandlerForProtocol(Protocols::InteractionModel::Id, this));

    mReportingEngine.Init((eventManagement != nullptr) ? eventManagement : &EventManagement::GetInstance());

    StatusIB::RegisterErrorFormatter();

    mState = State::kInitialized;
    return CHIP_NO_ERROR;
}

void InteractionModelEngine::Shutdown()
{
    VerifyOrReturn(State::kUninitialized != mState);

    mpExchangeMgr->GetSessionManager()->SystemLayer()->CancelTimer(ResumeSubscriptionsTimerCallback, this);

    // TODO: individual object clears the entire command handler interface registry.
    //       This may not be expected as IME does NOT own the command handler interface registry.
    //
    //       This is to be cleaned up once InteractionModelEngine maintains a data model fully and
    //       the code-generation model can do its clear in its shutdown method.
    CommandHandlerInterfaceRegistry::Instance().UnregisterAllHandlers();
    mCommandResponderObjs.ReleaseAll();

    mTimedHandlers.ForEachActiveObject([this](TimedHandler * obj) -> Loop {
        mpExchangeMgr->CloseAllContextsForDelegate(obj);
        return Loop::Continue;
    });

    mTimedHandlers.ReleaseAll();

    mReadHandlers.ReleaseAll();

#if CHIP_CONFIG_ENABLE_READ_CLIENT
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
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT

    for (auto & writeHandler : mWriteHandlers)
    {
        if (!writeHandler.IsFree())
        {
            writeHandler.Close();
        }
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

    mState = State::kUninitialized;
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

#if CHIP_CONFIG_ENABLE_READ_CLIENT
CHIP_ERROR InteractionModelEngine::ShutdownSubscription(const ScopedNodeId & aPeerNodeId, SubscriptionId aSubscriptionId)
{
    assertChipStackLockedByCurrentThread();
    for (auto * readClient = mpActiveReadClientList; readClient != nullptr;)
    {
        // Grab the next client now, because we might be about to delete readClient.
        auto * nextClient = readClient->GetNextClient();
        if (readClient->IsSubscriptionType() && readClient->IsMatchingSubscriptionId(aSubscriptionId) &&
            readClient->GetFabricIndex() == aPeerNodeId.GetFabricIndex() && readClient->GetPeerNodeId() == aPeerNodeId.GetNodeId())
        {
            readClient->Close(CHIP_NO_ERROR);
            return CHIP_NO_ERROR;
        }
        readClient = nextClient;
    }

    return CHIP_ERROR_KEY_NOT_FOUND;
}

void InteractionModelEngine::ShutdownSubscriptions(FabricIndex aFabricIndex, NodeId aPeerNodeId)
{
    assertChipStackLockedByCurrentThread();
    ShutdownMatchingSubscriptions(MakeOptional(aFabricIndex), MakeOptional(aPeerNodeId));
}
void InteractionModelEngine::ShutdownSubscriptions(FabricIndex aFabricIndex)
{
    assertChipStackLockedByCurrentThread();
    ShutdownMatchingSubscriptions(MakeOptional(aFabricIndex));
}

void InteractionModelEngine::ShutdownAllSubscriptions()
{
    assertChipStackLockedByCurrentThread();
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
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT

bool InteractionModelEngine::SubjectHasActiveSubscription(FabricIndex aFabricIndex, NodeId subjectID)
{
    bool isActive = false;
    mReadHandlers.ForEachActiveObject([aFabricIndex, subjectID, &isActive](ReadHandler * handler) {
        VerifyOrReturnValue(handler->IsType(ReadHandler::InteractionType::Subscribe), Loop::Continue);

        Access::SubjectDescriptor subject = handler->GetSubjectDescriptor();
        VerifyOrReturnValue(subject.fabricIndex == aFabricIndex, Loop::Continue);

        if (subject.authMode == Access::AuthMode::kCase)
        {
            if (subject.cats.CheckSubjectAgainstCATs(subjectID) || subjectID == subject.subject)
            {
                isActive = handler->IsActiveSubscription();

                // Exit loop only if isActive is set to true.
                // Otherwise keep looking for another subscription that could match the subject.
                VerifyOrReturnValue(!isActive, Loop::Break);
            }
        }

        return Loop::Continue;
    });

    return isActive;
}

bool InteractionModelEngine::SubjectHasPersistedSubscription(FabricIndex aFabricIndex, NodeId subjectID)
{
    bool persistedSubMatches = false;

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    auto * iterator = mpSubscriptionResumptionStorage->IterateSubscriptions();
    // Verify that we were able to allocate an iterator. If not, we are probably currently trying to resubscribe to our persisted
    // subscriptions. As such, we assume we have a persisted subscription and return true.
    // If we don't have a persisted subscription for the given fabric index and subjectID, we will send a Check-In message next time
    // we transition to ActiveMode.
    VerifyOrReturnValue(iterator, true);

    SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo;
    while (iterator->Next(subscriptionInfo))
    {
        // TODO(#31873): Persistent subscription only stores the NodeID for now. We cannot check if the CAT matches
        if (subscriptionInfo.mFabricIndex == aFabricIndex && subscriptionInfo.mNodeId == subjectID)
        {
            persistedSubMatches = true;
            break;
        }
    }
    iterator->Release();
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

    return persistedSubMatches;
}

bool InteractionModelEngine::FabricHasAtLeastOneActiveSubscription(FabricIndex aFabricIndex)
{
    bool hasActiveSubscription = false;
    mReadHandlers.ForEachActiveObject([aFabricIndex, &hasActiveSubscription](ReadHandler * handler) {
        VerifyOrReturnValue(handler->IsType(ReadHandler::InteractionType::Subscribe), Loop::Continue);

        Access::SubjectDescriptor subject = handler->GetSubjectDescriptor();
        VerifyOrReturnValue(subject.fabricIndex == aFabricIndex, Loop::Continue);

        if ((subject.authMode == Access::AuthMode::kCase) && handler->IsActiveSubscription())
        {
            // On first subscription found for fabric, we can immediately stop checking.
            hasActiveSubscription = true;
            return Loop::Break;
        }

        return Loop::Continue;
    });

    return hasActiveSubscription;
}

void InteractionModelEngine::OnDone(CommandResponseSender & apResponderObj)
{
    mCommandResponderObjs.ReleaseObject(&apResponderObj);
}

// TODO(#30453): Follow up refactor. Remove need for InteractionModelEngine::OnDone(CommandHandlerImpl).
void InteractionModelEngine::OnDone(CommandHandlerImpl & apCommandObj)
{
    // We are no longer expecting to receive this callback. With the introduction of CommandResponseSender, it is now
    // responsible for receiving this callback.
    VerifyOrDie(false);
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
    TryToResumeSubscriptions();
}

void InteractionModelEngine::TryToResumeSubscriptions()
{
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    if (!mSubscriptionResumptionScheduled && HasSubscriptionsToResume())
    {
        mSubscriptionResumptionScheduled            = true;
        auto timeTillNextSubscriptionResumptionSecs = ComputeTimeSecondsTillNextSubscriptionResumption();
        mpExchangeMgr->GetSessionManager()->SystemLayer()->StartTimer(
            System::Clock::Seconds32(timeTillNextSubscriptionResumptionSecs), ResumeSubscriptionsTimerCallback, this);
        mNumSubscriptionResumptionRetries++;
        ChipLogProgress(InteractionModel, "Schedule subscription resumption when failing to establish session, Retries: %" PRIu32,
                        mNumSubscriptionResumptionRetries);
    }
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
}

Status InteractionModelEngine::OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext,
                                                      const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload,
                                                      bool aIsTimedInvoke)
{
    // TODO(#30453): Refactor CommandResponseSender's constructor to accept an exchange context parameter.
    CommandResponseSender * commandResponder = mCommandResponderObjs.CreateObject(this, this);
    if (commandResponder == nullptr)
    {
        ChipLogProgress(InteractionModel, "no resource for Invoke interaction");
        return Status::Busy;
    }
    CHIP_FAULT_INJECT(FaultInjection::kFault_IMInvoke_SeparateResponses,
                      commandResponder->TestOnlyInvokeCommandRequestWithFaultsInjected(
                          apExchangeContext, std::move(aPayload), aIsTimedInvoke,
                          CommandHandlerImpl::NlFaultInjectionType::SeparateResponseMessages);
                      return Status::Success;);
    CHIP_FAULT_INJECT(FaultInjection::kFault_IMInvoke_SeparateResponsesInvertResponseOrder,
                      commandResponder->TestOnlyInvokeCommandRequestWithFaultsInjected(
                          apExchangeContext, std::move(aPayload), aIsTimedInvoke,
                          CommandHandlerImpl::NlFaultInjectionType::SeparateResponseMessagesAndInvertedResponseOrder);
                      return Status::Success;);
    CHIP_FAULT_INJECT(
        FaultInjection::kFault_IMInvoke_SkipSecondResponse,
        commandResponder->TestOnlyInvokeCommandRequestWithFaultsInjected(
            apExchangeContext, std::move(aPayload), aIsTimedInvoke, CommandHandlerImpl::NlFaultInjectionType::SkipSecondResponse);
        return Status::Success;);
    commandResponder->OnInvokeCommandRequest(apExchangeContext, std::move(aPayload), aIsTimedInvoke);
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

    while (CHIP_NO_ERROR == (err = pathReader.Next(TLV::AnonymousTag())))
    {
        AttributePathIB::Parser path;
        //
        // We create an iterator to point to a single item object list that tracks the path we just parsed.
        // This avoids the 'parse all paths' approach that is employed in ReadHandler since we want to
        // avoid allocating out of the path store during this minimal initial processing stage.
        //
        SingleLinkedListNode<AttributePathParams> paramsList;

        ReturnErrorOnFailure(path.Init(pathReader));
        ReturnErrorOnFailure(path.ParsePath(paramsList.mValue));

        if (paramsList.mValue.IsWildcardPath())
        {

            auto state = AttributePathExpandIterator::Position::StartIterating(&paramsList);
            AttributePathExpandIterator pathIterator(GetDataModelProvider(), state);
            ConcreteAttributePath readPath;

            // The definition of "valid path" is "path exists and ACL allows access". The "path exists" part is handled by
            // AttributePathExpandIterator. So we just need to check the ACL bits.
            while (pathIterator.Next(readPath))
            {
                // leave requestPath.entityId optional value unset to indicate wildcard
                Access::RequestPath requestPath{ .cluster     = readPath.mClusterId,
                                                 .endpoint    = readPath.mEndpointId,
                                                 .requestType = Access::RequestType::kAttributeReadRequest };
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

            if (IsExistentAttributePath(concretePath))
            {
                Access::RequestPath requestPath{ .cluster     = concretePath.mClusterId,
                                                 .endpoint    = concretePath.mEndpointId,
                                                 .requestType = Access::RequestType::kAttributeReadRequest,
                                                 .entityId    = paramsList.mValue.mAttributeId };

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

CHIP_ERROR InteractionModelEngine::ParseEventPaths(const Access::SubjectDescriptor & aSubjectDescriptor,
                                                   EventPathIBs::Parser & aEventPathListParser, bool & aHasValidEventPath,
                                                   size_t & aRequestedEventPathCount)
{
    TLV::TLVReader pathReader;
    aEventPathListParser.GetReader(&pathReader);
    CHIP_ERROR err = CHIP_NO_ERROR;

    aHasValidEventPath       = false;
    aRequestedEventPathCount = 0;

    while (CHIP_NO_ERROR == (err = pathReader.Next(TLV::AnonymousTag())))
    {
        EventPathIB::Parser path;
        ReturnErrorOnFailure(path.Init(pathReader));

        EventPathParams eventPath;
        ReturnErrorOnFailure(path.ParsePath(eventPath));

        ++aRequestedEventPathCount;

        if (aHasValidEventPath)
        {
            // Can skip all the rest of the checking.
            continue;
        }

        // The definition of "valid path" is "path exists and ACL allows
        // access".  We need to do some expansion of wildcards to handle that.
        aHasValidEventPath = MayHaveAccessibleEventPath(mDataModelProvider, eventPath, aSubjectDescriptor);
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

#if CHIP_CONFIG_IM_PRETTY_PRINT
        subscribeRequestParser.PrettyPrint();
#endif

        VerifyOrReturnError(subscribeRequestParser.GetKeepSubscriptions(&keepExistingSubscriptions) == CHIP_NO_ERROR,
                            Status::InvalidAction);
        if (!keepExistingSubscriptions)
        {
            //
            // Walk through all existing subscriptions and shut down those whose subscriber matches
            // that which just came in.
            //
            mReadHandlers.ForEachActiveObject([apExchangeContext](ReadHandler * handler) {
                if (handler->IsFromSubscriber(*apExchangeContext))
                {
                    ChipLogProgress(InteractionModel,
                                    "Deleting previous subscription from NodeId: " ChipLogFormatX64 ", FabricIndex: %u",
                                    ChipLogValueX64(apExchangeContext->GetSessionHandle()->AsSecureSession()->GetPeerNodeId()),
                                    apExchangeContext->GetSessionHandle()->GetFabricIndex());
                    handler->Close();
                }

                return Loop::Continue;
            });
        }

        {
            size_t requestedAttributePathCount = 0;
            size_t requestedEventPathCount     = 0;
            AttributePathIBs::Parser attributePathListParser;
            bool hasValidAttributePath = false;
            bool mayHaveValidEventPath = false;

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

            EventPathIBs::Parser eventPathListParser;
            err = subscribeRequestParser.GetEventRequests(&eventPathListParser);
            if (err == CHIP_NO_ERROR)
            {
                auto subjectDescriptor = apExchangeContext->GetSessionHandle()->AsSecureSession()->GetSubjectDescriptor();
                err = ParseEventPaths(subjectDescriptor, eventPathListParser, mayHaveValidEventPath, requestedEventPathCount);
                if (err != CHIP_NO_ERROR)
                {
                    return Status::InvalidAction;
                }
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

            if (!hasValidAttributePath && !mayHaveValidEventPath)
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
    }
    else
    {
        System::PacketBufferTLVReader reader;
        reader.Init(aPayload.Retain());

        ReadRequestMessage::Parser readRequestParser;
        VerifyOrReturnError(readRequestParser.Init(reader) == CHIP_NO_ERROR, Status::InvalidAction);

#if CHIP_CONFIG_IM_PRETTY_PRINT
        readRequestParser.PrettyPrint();
#endif
        {
            size_t requestedAttributePathCount = 0;
            size_t requestedEventPathCount     = 0;
            AttributePathIBs::Parser attributePathListParser;
            CHIP_ERROR err = readRequestParser.GetAttributeRequests(&attributePathListParser);
            if (err == CHIP_NO_ERROR)
            {
                TLV::TLVReader pathReader;
                attributePathListParser.GetReader(&pathReader);
                VerifyOrReturnError(TLV::Utilities::Count(pathReader, requestedAttributePathCount, false) == CHIP_NO_ERROR,
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
                VerifyOrReturnError(TLV::Utilities::Count(pathReader, requestedEventPathCount, false) == CHIP_NO_ERROR,
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
    ReadHandler * handler = mReadHandlers.CreateObject(*this, apExchangeContext, aInteractionType, mReportScheduler);
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
            VerifyOrReturnError(writeHandler.Init(GetDataModelProvider(), this) == CHIP_NO_ERROR, Status::Busy);
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
    TimedHandler * handler = mTimedHandlers.CreateObject(this);
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

#if CHIP_CONFIG_ENABLE_READ_CLIENT
Status InteractionModelEngine::OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext,
                                                       const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload)
{
    System::PacketBufferTLVReader reader;
    reader.Init(aPayload.Retain());

    ReportDataMessage::Parser report;
    VerifyOrReturnError(report.Init(reader) == CHIP_NO_ERROR, Status::InvalidAction);

#if CHIP_CONFIG_IM_PRETTY_PRINT
    report.PrettyPrint();
#endif

    SubscriptionId subscriptionId = 0;
    VerifyOrReturnError(report.GetSubscriptionId(&subscriptionId) == CHIP_NO_ERROR, Status::InvalidAction);
    VerifyOrReturnError(report.ExitContainer() == CHIP_NO_ERROR, Status::InvalidAction);

    ReadClient * foundSubscription = nullptr;
    for (auto * readClient = mpActiveReadClientList; readClient != nullptr; readClient = readClient->GetNextClient())
    {
        auto peer = apExchangeContext->GetSessionHandle()->GetPeer();
        if (readClient->GetFabricIndex() != peer.GetFabricIndex() || readClient->GetPeerNodeId() != peer.GetNodeId())
        {
            continue;
        }

        // Notify Subscriptions about incoming communication from node
        readClient->OnUnsolicitedMessageFromPublisher();

        if (!readClient->IsSubscriptionActive())
        {
            continue;
        }

        if (!readClient->IsMatchingSubscriptionId(subscriptionId))
        {
            continue;
        }

        if (!foundSubscription)
        {
            foundSubscription = readClient;
        }
    }

    if (foundSubscription)
    {
        foundSubscription->OnUnsolicitedReportData(apExchangeContext, std::move(aPayload));
        return Status::Success;
    }

    ChipLogDetail(InteractionModel, "Received report with invalid subscriptionId %" PRIu32, subscriptionId);

    return Status::InvalidSubscription;
}
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT

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

    // Ensure that DataModel::Provider has access to the exchange the message was received on.
    CurrentExchangeValueScope scopedExchangeContext(*this, apExchangeContext);

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
#if CHIP_CONFIG_ENABLE_READ_CLIENT
    else if (aPayloadHeader.HasMessageType(Protocols::InteractionModel::MsgType::ReportData))
    {
        status = OnUnsolicitedReportData(apExchangeContext, aPayloadHeader, std::move(aPayload));
    }
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT
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

#if CHIP_CONFIG_ENABLE_READ_CLIENT
void InteractionModelEngine::OnActiveModeNotification(ScopedNodeId aPeer)
{
    for (ReadClient * pListItem = mpActiveReadClientList; pListItem != nullptr;)
    {
        auto pNextItem = pListItem->GetNextClient();
        // It is possible that pListItem is destroyed by the app in OnActiveModeNotification.
        // Get the next item before invoking `OnActiveModeNotification`.
        if (ScopedNodeId(pListItem->GetPeerNodeId(), pListItem->GetFabricIndex()) == aPeer)
        {
            pListItem->OnActiveModeNotification();
        }
        pListItem = pNextItem;
    }
}

void InteractionModelEngine::OnPeerTypeChange(ScopedNodeId aPeer, ReadClient::PeerType aType)
{
    // TODO: Follow up to use a iterator function to avoid copy/paste here.
    for (ReadClient * pListItem = mpActiveReadClientList; pListItem != nullptr;)
    {
        // It is possible that pListItem is destroyed by the app in OnPeerTypeChange.
        // Get the next item before invoking `OnPeerTypeChange`.
        auto pNextItem = pListItem->GetNextClient();
        if (ScopedNodeId(pListItem->GetPeerNodeId(), pListItem->GetFabricIndex()) == aPeer)
        {
            pListItem->OnPeerTypeChange(aType);
        }
        pListItem = pNextItem;
    }
}

void InteractionModelEngine::AddReadClient(ReadClient * apReadClient)
{
    apReadClient->SetNextClient(mpActiveReadClientList);
    mpActiveReadClientList = apReadClient;
}
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT

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
        SubscriptionId subId;
        candidate->GetSubscriptionId(subId);
        ChipLogProgress(DataManagement, "Evicting Subscription ID %u:0x%" PRIx32, candidate->GetSubjectDescriptor().fabricIndex,
                        subId);
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

#if CHIP_CONFIG_ENABLE_READ_CLIENT
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
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT

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

void InteractionModelEngine::ReleaseAttributePathList(SingleLinkedListNode<AttributePathParams> *& aAttributePathList)
{
    ReleasePool(aAttributePathList, mAttributePathPool);
}

CHIP_ERROR InteractionModelEngine::PushFrontAttributePathList(SingleLinkedListNode<AttributePathParams> *& aAttributePathList,
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

bool InteractionModelEngine::IsExistentAttributePath(const ConcreteAttributePath & path)
{
    DataModel::AttributeFinder finder(mDataModelProvider);
    return finder.Find(path).has_value();
}

void InteractionModelEngine::RemoveDuplicateConcreteAttributePath(SingleLinkedListNode<AttributePathParams> *& aAttributePaths)
{
    SingleLinkedListNode<AttributePathParams> * prev = nullptr;
    auto * path1                                     = aAttributePaths;

    while (path1 != nullptr)
    {
        bool duplicate = false;

        // skip all wildcard paths and invalid concrete attribute
        if (path1->mValue.IsWildcardPath() ||
            !IsExistentAttributePath(
                ConcreteAttributePath(path1->mValue.mEndpointId, path1->mValue.mClusterId, path1->mValue.mAttributeId)))
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

void InteractionModelEngine::ReleaseEventPathList(SingleLinkedListNode<EventPathParams> *& aEventPathList)
{
    ReleasePool(aEventPathList, mEventPathPool);
}

CHIP_ERROR InteractionModelEngine::PushFrontEventPathParamsList(SingleLinkedListNode<EventPathParams> *& aEventPathList,
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

void InteractionModelEngine::ReleaseDataVersionFilterList(SingleLinkedListNode<DataVersionFilter> *& aDataVersionFilterList)
{
    ReleasePool(aDataVersionFilterList, mDataVersionFilterPool);
}

CHIP_ERROR InteractionModelEngine::PushFrontDataVersionFilterList(SingleLinkedListNode<DataVersionFilter> *& aDataVersionFilterList,
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
void InteractionModelEngine::ReleasePool(SingleLinkedListNode<T> *& aObjectList,
                                         ObjectPool<SingleLinkedListNode<T>, N> & aObjectPool)
{
    SingleLinkedListNode<T> * current = aObjectList;
    while (current != nullptr)
    {
        SingleLinkedListNode<T> * nextObject = current->mpNext;
        aObjectPool.ReleaseObject(current);
        current = nextObject;
    }

    aObjectList = nullptr;
}

template <typename T, size_t N>
CHIP_ERROR InteractionModelEngine::PushFront(SingleLinkedListNode<T> *& aObjectList, T & aData,
                                             ObjectPool<SingleLinkedListNode<T>, N> & aObjectPool)
{
    SingleLinkedListNode<T> * object = aObjectPool.CreateObject();
    if (object == nullptr)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    object->mValue = aData;
    object->mpNext = aObjectList;
    aObjectList    = object;
    return CHIP_NO_ERROR;
}

void InteractionModelEngine::DispatchCommand(CommandHandlerImpl & apCommandObj, const ConcreteCommandPath & aCommandPath,
                                             TLV::TLVReader & apPayload)
{
    Access::SubjectDescriptor subjectDescriptor = apCommandObj.GetSubjectDescriptor();

    DataModel::InvokeRequest request;
    request.path = aCommandPath;
    request.invokeFlags.Set(DataModel::InvokeFlags::kTimed, apCommandObj.IsTimedInvoke());
    request.subjectDescriptor = &subjectDescriptor;

    std::optional<DataModel::ActionReturnStatus> status = GetDataModelProvider()->Invoke(request, apPayload, &apCommandObj);

    // Provider indicates that handler status or data was already set (or will be set asynchronously) by
    // returning std::nullopt. If any other value is returned, it is requesting that a status is set. This
    // includes CHIP_NO_ERROR: in this case CHIP_NO_ERROR would mean set a `status success on the command`
    if (status.has_value())
    {
        apCommandObj.AddStatus(aCommandPath, status->GetStatusCode());
    }
}

Protocols::InteractionModel::Status InteractionModelEngine::ValidateCommandCanBeDispatched(const DataModel::InvokeRequest & request)
{

    DataModel::AcceptedCommandEntry acceptedCommandEntry;

    Status status = CheckCommandExistence(request.path, acceptedCommandEntry);

    if (status != Status::Success)
    {
        ChipLogDetail(DataManagement, "No command " ChipLogFormatMEI " in Cluster " ChipLogFormatMEI " on Endpoint %u",
                      ChipLogValueMEI(request.path.mCommandId), ChipLogValueMEI(request.path.mClusterId), request.path.mEndpointId);
        return status;
    }

    status = CheckCommandAccess(request, acceptedCommandEntry);
    VerifyOrReturnValue(status == Status::Success, status);

    return CheckCommandFlags(request, acceptedCommandEntry);
}

Protocols::InteractionModel::Status InteractionModelEngine::CheckCommandAccess(const DataModel::InvokeRequest & aRequest,
                                                                               const DataModel::AcceptedCommandEntry & entry)
{
    if (aRequest.subjectDescriptor == nullptr)
    {
        return Status::UnsupportedAccess; // we require a subject for invoke
    }

    Access::RequestPath requestPath{ .cluster     = aRequest.path.mClusterId,
                                     .endpoint    = aRequest.path.mEndpointId,
                                     .requestType = Access::RequestType::kCommandInvokeRequest,
                                     .entityId    = aRequest.path.mCommandId };

    CHIP_ERROR err = Access::GetAccessControl().Check(*aRequest.subjectDescriptor, requestPath, entry.invokePrivilege);
    if (err != CHIP_NO_ERROR)
    {
        if ((err != CHIP_ERROR_ACCESS_DENIED) && (err != CHIP_ERROR_ACCESS_RESTRICTED_BY_ARL))
        {
            return Status::Failure;
        }
        return err == CHIP_ERROR_ACCESS_DENIED ? Status::UnsupportedAccess : Status::AccessRestricted;
    }

    return Status::Success;
}

Protocols::InteractionModel::Status InteractionModelEngine::CheckCommandFlags(const DataModel::InvokeRequest & aRequest,
                                                                              const DataModel::AcceptedCommandEntry & entry)
{
    const bool commandNeedsTimedInvoke = entry.flags.Has(DataModel::CommandQualityFlags::kTimed);
    const bool commandIsFabricScoped   = entry.flags.Has(DataModel::CommandQualityFlags::kFabricScoped);

    if (commandNeedsTimedInvoke && !aRequest.invokeFlags.Has(DataModel::InvokeFlags::kTimed))
    {
        return Status::NeedsTimedInteraction;
    }

    if (commandIsFabricScoped)
    {
        // SPEC: Else if the command in the path is fabric-scoped and there is no accessing fabric,
        // a CommandStatusIB SHALL be generated with the UNSUPPORTED_ACCESS Status Code.

        // Fabric-scoped commands are not allowed before a specific accessing fabric is available.
        // This is mostly just during a PASE session before AddNOC.
        if (aRequest.GetAccessingFabricIndex() == kUndefinedFabricIndex)
        {
            return Status::UnsupportedAccess;
        }
    }

    return Status::Success;
}

Protocols::InteractionModel::Status InteractionModelEngine::CheckCommandExistence(const ConcreteCommandPath & aCommandPath,
                                                                                  DataModel::AcceptedCommandEntry & entry)
{
    auto provider = GetDataModelProvider();

    DataModel::ListBuilder<DataModel::AcceptedCommandEntry> acceptedCommands;
    (void) provider->AcceptedCommands(aCommandPath, acceptedCommands);
    for (auto & existing : acceptedCommands.TakeBuffer())
    {
        if (existing.commandId == aCommandPath.mCommandId)
        {
            entry = existing;
            return Protocols::InteractionModel::Status::Success;
        }
    }

    // invalid command, return the right failure status
    return DataModel::ValidateClusterPath(provider, aCommandPath, Protocols::InteractionModel::Status::UnsupportedCommand);
}

DataModel::Provider * InteractionModelEngine::SetDataModelProvider(DataModel::Provider * model)
{
    // Altering data model should not be done while IM is actively handling requests.
    VerifyOrDie(mReadHandlers.begin() == mReadHandlers.end());

    if (model == mDataModelProvider)
    {
        // no-op, just return
        return model;
    }

    DataModel::Provider * oldModel = mDataModelProvider;
    if (oldModel != nullptr)
    {
        CHIP_ERROR err = oldModel->Shutdown();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(InteractionModel, "Failure on interaction model shutdown: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    mDataModelProvider = model;
    if (mDataModelProvider != nullptr)
    {
        DataModel::InteractionModelContext context;

        context.eventsGenerator         = &EventManagement::GetInstance();
        context.dataModelChangeListener = &mReportingEngine;
        context.actionContext           = this;

        CHIP_ERROR err = mDataModelProvider->Startup(context);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(InteractionModel, "Failure on interaction model startup: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }

    return oldModel;
}

DataModel::Provider * InteractionModelEngine::GetDataModelProvider() const
{
    // These should be called within the CHIP processing loop.
    assertChipStackLockedByCurrentThread();

    return mDataModelProvider;
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
        std::min(GetReadHandlerPoolCapacityForSubscriptions() / GetConfigMaxFabrics(), static_cast<size_t>(UINT16_MAX)));
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

#if CHIP_CONFIG_ENABLE_READ_CLIENT
    for (auto * readClient = mpActiveReadClientList; readClient != nullptr;)
    {
        // ReadClient::Close may delete the read client so that readClient->GetNextClient() will be use-after-free.
        // We need save readClient as nextReadClient before closing.
        if (readClient->GetFabricIndex() == fabricIndex)
        {
            ChipLogProgress(InteractionModel, "Fabric removed, deleting obsolete read client with FabricIndex: %u", fabricIndex);
            auto * nextReadClient = readClient->GetNextClient();
            readClient->Close(CHIP_ERROR_IM_FABRIC_DELETED, false);
            readClient = nextReadClient;
        }
        else
        {
            readClient = readClient->GetNextClient();
        }
    }
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT

    for (auto & handler : mWriteHandlers)
    {
        if (!(handler.IsFree()) && handler.GetAccessingFabricIndex() == fabricIndex)
        {
            ChipLogProgress(InteractionModel, "Fabric removed, deleting obsolete write handler with FabricIndex: %u", fabricIndex);
            handler.Close();
        }
    }

    // Applications may hold references to CommandHandlerImpl instances for async command processing.
    // Therefore we can't forcible destroy CommandHandlers here.  Their exchanges will get closed by
    // the fabric removal, though, so they will fail when they try to actually send their command response
    // and will close at that point.
}

CHIP_ERROR InteractionModelEngine::ResumeSubscriptions()
{
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    VerifyOrReturnError(mpSubscriptionResumptionStorage, CHIP_NO_ERROR);
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    VerifyOrReturnError(!mSubscriptionResumptionScheduled, CHIP_NO_ERROR);
#endif

    // To avoid the case of a reboot loop causing rapid traffic generation / power consumption, subscription resumption should make
    // use of the persisted min-interval values, and wait before resumption. Ideally, each persisted subscription should wait their
    // own min-interval value before resumption, but that both A) potentially runs into a timer resource issue, and B) having a
    // low-powered device wake many times also has energy use implications. The logic below waits the largest of the persisted
    // min-interval values before resuming subscriptions.

    // Even though this causes subscription-to-subscription interaction by linking the min-interval values, this is the right thing
    // to do for now because it's both simple and avoids the timer resource and multiple-wake problems. This issue is to track
    // future improvements: https://github.com/project-chip/connectedhomeip/issues/25439

    SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo;
    auto * iterator             = mpSubscriptionResumptionStorage->IterateSubscriptions();
    mNumOfSubscriptionsToResume = 0;
    uint16_t minInterval        = 0;
    while (iterator->Next(subscriptionInfo))
    {
        mNumOfSubscriptionsToResume++;
        minInterval = std::max(minInterval, subscriptionInfo.mMinInterval);
    }
    iterator->Release();

    if (mNumOfSubscriptionsToResume)
    {
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
        mSubscriptionResumptionScheduled = true;
#endif
        ChipLogProgress(InteractionModel, "Resuming %d subscriptions in %u seconds", mNumOfSubscriptionsToResume, minInterval);
        ReturnErrorOnFailure(mpExchangeMgr->GetSessionManager()->SystemLayer()->StartTimer(System::Clock::Seconds16(minInterval),
                                                                                           ResumeSubscriptionsTimerCallback, this));
    }
    else
    {
        ChipLogProgress(InteractionModel, "No subscriptions to resume");
    }
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

    return CHIP_NO_ERROR;
}

void InteractionModelEngine::ResumeSubscriptionsTimerCallback(System::Layer * apSystemLayer, void * apAppState)
{
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    VerifyOrReturn(apAppState != nullptr);
    InteractionModelEngine * imEngine = static_cast<InteractionModelEngine *>(apAppState);
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    imEngine->mSubscriptionResumptionScheduled = false;
    bool resumedSubscriptions                  = false;
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo;
    AutoReleaseSubscriptionInfoIterator iterator(imEngine->mpSubscriptionResumptionStorage->IterateSubscriptions());
    while (iterator->Next(subscriptionInfo))
    {
        // If subscription happens between reboot and this timer callback, it's already live and should skip resumption
        if (Loop::Break == imEngine->mReadHandlers.ForEachActiveObject([&](ReadHandler * handler) {
                SubscriptionId subscriptionId;
                handler->GetSubscriptionId(subscriptionId);
                if (subscriptionId == subscriptionInfo.mSubscriptionId)
                {
                    return Loop::Break;
                }
                return Loop::Continue;
            }))
        {
            ChipLogProgress(InteractionModel, "Skip resuming live subscriptionId %" PRIu32, subscriptionInfo.mSubscriptionId);
            continue;
        }

        auto subscriptionResumptionSessionEstablisher = Platform::MakeUnique<SubscriptionResumptionSessionEstablisher>();
        if (subscriptionResumptionSessionEstablisher == nullptr)
        {
            ChipLogProgress(InteractionModel, "Failed to create SubscriptionResumptionSessionEstablisher");
            return;
        }

        if (subscriptionResumptionSessionEstablisher->ResumeSubscription(*imEngine->mpCASESessionMgr, subscriptionInfo) !=
            CHIP_NO_ERROR)
        {
            ChipLogProgress(InteractionModel, "Failed to ResumeSubscription 0x%" PRIx32, subscriptionInfo.mSubscriptionId);
            return;
        }
        subscriptionResumptionSessionEstablisher.release();
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
        resumedSubscriptions = true;
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    }

#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    // If no persisted subscriptions needed resumption then all resumption retries are done
    if (!resumedSubscriptions)
    {
        imEngine->mNumSubscriptionResumptionRetries = 0;
    }
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
}

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
uint32_t InteractionModelEngine::ComputeTimeSecondsTillNextSubscriptionResumption()
{
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    if (mSubscriptionResumptionRetrySecondsOverride > 0)
    {
        return static_cast<uint32_t>(mSubscriptionResumptionRetrySecondsOverride);
    }
#endif
    if (mNumSubscriptionResumptionRetries > CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_FIBONACCI_STEP_INDEX)
    {
        return CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MAX_RETRY_INTERVAL_SECS;
    }

    return CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_MIN_RETRY_INTERVAL_SECS +
        GetFibonacciForIndex(mNumSubscriptionResumptionRetries) *
        CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION_WAIT_TIME_MULTIPLIER_SECS;
}

bool InteractionModelEngine::HasSubscriptionsToResume()
{
    VerifyOrReturnValue(mpSubscriptionResumptionStorage != nullptr, false);

    // Look through persisted subscriptions and see if any aren't already in mReadHandlers pool
    SubscriptionResumptionStorage::SubscriptionInfo subscriptionInfo;
    auto * iterator                = mpSubscriptionResumptionStorage->IterateSubscriptions();
    bool foundSubscriptionToResume = false;
    while (iterator->Next(subscriptionInfo))
    {
        if (Loop::Break == mReadHandlers.ForEachActiveObject([&](ReadHandler * handler) {
                SubscriptionId subscriptionId;
                handler->GetSubscriptionId(subscriptionId);
                if (subscriptionId == subscriptionInfo.mSubscriptionId)
                {
                    return Loop::Break;
                }
                return Loop::Continue;
            }))
        {
            continue;
        }

        foundSubscriptionToResume = true;
        break;
    }
    iterator->Release();

    return foundSubscriptionToResume;
}
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
void InteractionModelEngine::DecrementNumSubscriptionsToResume()
{
    VerifyOrReturn(mNumOfSubscriptionsToResume > 0);
#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    VerifyOrDie(mICDManager);
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION

    mNumOfSubscriptionsToResume--;

#if CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    if (!mNumOfSubscriptionsToResume)
    {
        mICDManager->SetBootUpResumeSubscriptionExecuted();
    }
#endif // CHIP_CONFIG_ENABLE_ICD_CIP && !CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
}
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

} // namespace app
} // namespace chip
