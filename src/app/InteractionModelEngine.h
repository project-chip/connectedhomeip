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

#pragma once

#include <access/AccessControl.h>
#include <app/AppConfig.h>
#include <app/AttributePathParams.h>
#include <app/CommandHandlerImpl.h>
#include <app/CommandResponseSender.h>
#include <app/CommandSender.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/ConcreteEventPath.h>
#include <app/DataVersionFilter.h>
#include <app/EventPathParams.h>
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/ReportDataMessage.h>
#include <app/ReadClient.h>
#include <app/ReadHandler.h>
#include <app/StatusResponse.h>
#include <app/SubscriptionResumptionSessionEstablisher.h>
#include <app/SubscriptionsInfoProvider.h>
#include <app/TimedHandler.h>
#include <app/WriteClient.h>
#include <app/WriteHandler.h>
#include <app/data-model-provider/MetadataTypes.h>
#include <app/data-model-provider/OperationTypes.h>
#include <app/data-model-provider/Provider.h>
#include <app/icd/server/ICDServerConfig.h>
#include <app/reporting/Engine.h>
#include <app/reporting/ReportScheduler.h>
#include <app/util/attribute-metadata.h>
#include <app/util/basic-types.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/LinkedList.h>
#include <lib/support/Pool.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <protocols/interaction_model/Constants.h>
#include <system/SystemPacketBuffer.h>

#include <app/CASESessionManager.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDManager.h> // nogncheck
#endif                                 // CHIP_CONFIG_ENABLE_ICD_SERVER

namespace chip {
namespace app {

/**
 * @class InteractionModelEngine
 *
 * @brief This is a singleton hosting all CHIP unsolicited message processing and managing interaction model related clients and
 * handlers
 *
 */
class InteractionModelEngine : public Messaging::UnsolicitedMessageHandler,
                               public Messaging::ExchangeDelegate,
                               private DataModel::ActionContext,
                               public CommandResponseSender::Callback,
                               public CommandHandlerImpl::Callback,
                               public ReadHandler::ManagementCallback,
                               public FabricTable::Delegate,
                               public SubscriptionsInfoProvider,
                               public TimedHandlerDelegate,
                               public WriteHandlerDelegate
{
public:
    /**
     * @brief Retrieve the singleton Interaction Model Engine.
     *
     *  @return  A pointer to the shared InteractionModel Engine
     *
     */
    static InteractionModelEngine * GetInstance(void);

    /**
     * Spec 8.5.1 A publisher SHALL always ensure that every fabric the node is commissioned into can create at least three
     * subscriptions to the publisher and that each subscription SHALL support at least 3 attribute/event paths.
     */
    static constexpr size_t kMinSupportedSubscriptionsPerFabric = 3;
    static constexpr size_t kMinSupportedPathsPerSubscription   = 3;
    static constexpr size_t kMinSupportedPathsPerReadRequest    = 9;
    static constexpr size_t kMinSupportedReadRequestsPerFabric  = 1;
    static constexpr size_t kReadHandlerPoolSize                = CHIP_IM_MAX_NUM_SUBSCRIPTIONS + CHIP_IM_MAX_NUM_READS;

    // TODO: Per spec, the above numbers should be 3, 3, 9, 1, however, we use a lower limit to reduce the memory usage and should
    // fix it when we have reduced the memory footprint of ReadHandlers.

    InteractionModelEngine(void);

    /**
     *  Initialize the InteractionModel Engine.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *  @param[in]    apFabricTable    A pointer to the FabricTable object.
     *  @param[in]    apCASESessionMgr An optional pointer to a CASESessionManager (used for re-subscriptions).
     *  @parma[in]    eventManagement  An optional pointer to a EventManagement. If null, the global instance will be used.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, FabricTable * apFabricTable,
                    reporting::ReportScheduler * reportScheduler, CASESessionManager * apCASESessionMgr = nullptr,
                    SubscriptionResumptionStorage * subscriptionResumptionStorage = nullptr,
                    EventManagement * eventManagement                             = nullptr);

    void Shutdown();

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    void SetICDManager(ICDManager * manager) { mICDManager = manager; };
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    Messaging::ExchangeManager * GetExchangeManager(void) const { return mpExchangeMgr; }

    /**
     * Returns a pointer to the CASESessionManager. This can return nullptr if one wasn't
     * provided in the call to Init().
     */
    CASESessionManager * GetCASESessionManager() const { return mpCASESessionMgr; }

#if CHIP_CONFIG_ENABLE_READ_CLIENT
    /**
     * Tears down an active subscription.
     *
     * @retval #CHIP_ERROR_KEY_NOT_FOUND If the subscription is not found.
     * @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR ShutdownSubscription(const ScopedNodeId & aPeerNodeId, SubscriptionId aSubscriptionId);

    /**
     * Tears down active subscriptions for a given peer node ID.
     */
    void ShutdownSubscriptions(FabricIndex aFabricIndex, NodeId aPeerNodeId);

    /**
     * Tears down all active subscriptions for a given fabric.
     */
    void ShutdownSubscriptions(FabricIndex aFabricIndex);

    /**
     * Tears down all active subscriptions.
     */
    void ShutdownAllSubscriptions();
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT

    uint32_t GetNumActiveReadHandlers() const;
    uint32_t GetNumActiveReadHandlers(ReadHandler::InteractionType type) const;

    /**
     * Returns the number of active readhandlers with a specific type on a specific fabric.
     */
    uint32_t GetNumActiveReadHandlers(ReadHandler::InteractionType type, FabricIndex fabricIndex) const;

    uint32_t GetNumActiveWriteHandlers() const;

    /**
     * Returns the handler at a particular index within the active handler list.
     */
    ReadHandler * ActiveHandlerAt(unsigned int aIndex);

    /**
     * Returns the write handler at a particular index within the active handler list.
     */
    WriteHandler * ActiveWriteHandlerAt(unsigned int aIndex);

    reporting::Engine & GetReportingEngine() { return mReportingEngine; }

    reporting::ReportScheduler * GetReportScheduler() { return mReportScheduler; }

    void ReleaseAttributePathList(SingleLinkedListNode<AttributePathParams> *& aAttributePathList);

    CHIP_ERROR PushFrontAttributePathList(SingleLinkedListNode<AttributePathParams> *& aAttributePathList,
                                          AttributePathParams & aAttributePath);

    // If a concrete path indicates an attribute that is also referenced by a wildcard path in the request,
    // the path SHALL be removed from the list.
    void RemoveDuplicateConcreteAttributePath(SingleLinkedListNode<AttributePathParams> *& aAttributePaths);

    void ReleaseEventPathList(SingleLinkedListNode<EventPathParams> *& aEventPathList);

    CHIP_ERROR PushFrontEventPathParamsList(SingleLinkedListNode<EventPathParams> *& aEventPathList, EventPathParams & aEventPath);

    void ReleaseDataVersionFilterList(SingleLinkedListNode<DataVersionFilter> *& aDataVersionFilterList);

    CHIP_ERROR PushFrontDataVersionFilterList(SingleLinkedListNode<DataVersionFilter> *& aDataVersionFilterList,
                                              DataVersionFilter & aDataVersionFilter);

    /*
     * Register an application callback to be notified of notable events when handling reads/subscribes.
     */
    void RegisterReadHandlerAppCallback(ReadHandler::ApplicationCallback * mpApplicationCallback)
    {
        mpReadHandlerApplicationCallback = mpApplicationCallback;
    }
    void UnregisterReadHandlerAppCallback() { mpReadHandlerApplicationCallback = nullptr; }

    // TimedHandlerDelegate implementation
    void OnTimedInteractionFailed(TimedHandler * apTimedHandler) override;
    void OnTimedInvoke(TimedHandler * apTimedHandler, Messaging::ExchangeContext * apExchangeContext,
                       const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload) override;
    void OnTimedWrite(TimedHandler * apTimedHandler, Messaging::ExchangeContext * apExchangeContext,
                      const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload) override;

    // WriteHandlerDelegate implementation
    bool HasConflictWriteRequests(const WriteHandler * apWriteHandler, const ConcreteAttributePath & apath) override;

#if CHIP_CONFIG_ENABLE_READ_CLIENT
    /**
     *  Activate the idle subscriptions.
     *
     *  When subscribing to ICD and liveness timeout reached, the read client will move to `InactiveICDSubscription` state and
     * resubscription can be triggered via OnActiveModeNotification().
     */
    void OnActiveModeNotification(ScopedNodeId aPeer);

    /**
     *  Used to notify when a peer becomes LIT ICD or vice versa.
     *
     *  ReadClient will call this function when it finds any updates of the OperatingMode attribute from ICD management
     * cluster. The application doesn't need to call this function, usually.
     */
    void OnPeerTypeChange(ScopedNodeId aPeer, ReadClient::PeerType aType);

    /**
     * Add a read client to the internally tracked list of weak references. This list is used to
     * correctly dispatch unsolicited reports to the right matching handler by subscription ID.
     */
    void AddReadClient(ReadClient * apReadClient);

    /**
     * Remove a read client from the internally tracked list of weak references.
     */
    void RemoveReadClient(ReadClient * apReadClient);

    /**
     * Test to see if a read client is in the actively tracked list.
     */
    bool InActiveReadClientList(ReadClient * apReadClient);

    /**
     * Return the number of active read clients being tracked by the engine.
     */
    size_t GetNumActiveReadClients();
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT

    /**
     * Returns the number of dirty subscriptions. Including the subscriptions that are generating reports.
     */
    size_t GetNumDirtySubscriptions() const;

    /**
     * Select the oldest (and the one that exceeds the per subscription resource minimum if there are any) read handler on the
     * fabric with the given fabric index. Evict it when the fabric uses more resources than the per fabric quota or aForceEvict is
     * true.
     *
     * @retval Whether we have evicted a subscription.
     */
    bool TrimFabricForSubscriptions(FabricIndex aFabricIndex, bool aForceEvict);

    /**
     * Select a read handler and abort the read transaction if the fabric is using more resources (number of paths or number of read
     * handlers) then we guaranteed.
     *
     * - The youngest oversized read handlers will be chosen first.
     * - If there are no oversized read handlers, the youngest read handlers will be chosen.
     *
     * @retval Whether we have evicted a read transaction.
     */
    bool TrimFabricForRead(FabricIndex aFabricIndex);

    /**
     * Returns the minimal value of guaranteed subscriptions per fabic. UINT16_MAX will be returned if current app is configured to
     * use heap for the object pools used by interaction model engine.
     *
     * @retval the minimal value of guaranteed subscriptions per fabic.
     */
    uint16_t GetMinGuaranteedSubscriptionsPerFabric() const;

    // virtual method from FabricTable::Delegate
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex) override;

    SubscriptionResumptionStorage * GetSubscriptionResumptionStorage() { return mpSubscriptionResumptionStorage; };

    CHIP_ERROR ResumeSubscriptions();

    bool SubjectHasActiveSubscription(FabricIndex aFabricIndex, NodeId subjectID) override;

    bool SubjectHasPersistedSubscription(FabricIndex aFabricIndex, NodeId subjectID) override;

    bool FabricHasAtLeastOneActiveSubscription(FabricIndex aFabricIndex) override;

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    /**
     * @brief Function decrements the number of subscriptions to resume counter - mNumOfSubscriptionsToResume.
     *        This should be called after we have completed a re-subscribe attempt on a persisted subscription wether the attempt
     *        was successful or not.
     */
    void DecrementNumSubscriptionsToResume();
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    /**
     * @brief Function resets the number of retries of subscriptions resumption - mNumSubscriptionResumptionRetries.
     *        This should be called after we have completed a re-subscribe attempt successfully on a persisted subscription,
     *        or when the subscription resumption gets terminated.
     */
    void ResetNumSubscriptionsRetries();
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    //
    // Get direct access to the underlying read handler pool
    //
    auto & GetReadHandlerPool() { return mReadHandlers; }

    //
    // Override the maximal capacity of the fabric table only for interaction model engine
    //
    // If -1 is passed in, no override is instituted and default behavior resumes.
    //
    void SetConfigMaxFabrics(int32_t sz) { mMaxNumFabricsOverride = sz; }

    //
    // Override the maximal capacity of the underlying read handler pool to mimic
    // out of memory scenarios in unit-tests. You need to SetConfigMaxFabrics to make GetGuaranteedReadRequestsPerFabric
    // working correctly.
    //
    // If -1 is passed in, no override is instituted and default behavior resumes.
    //
    void SetHandlerCapacityForReads(int32_t sz) { mReadHandlerCapacityForReadsOverride = sz; }
    void SetHandlerCapacityForSubscriptions(int32_t sz) { mReadHandlerCapacityForSubscriptionsOverride = sz; }

    //
    // Override the maximal capacity of the underlying attribute path pool and event path pool to mimic
    // out of paths exhausted scenarios in unit-tests.
    //
    // If -1 is passed in, no override is instituted and default behavior resumes.
    //
    void SetPathPoolCapacityForReads(int32_t sz) { mPathPoolCapacityForReadsOverride = sz; }
    void SetPathPoolCapacityForSubscriptions(int32_t sz) { mPathPoolCapacityForSubscriptionsOverride = sz; }

    //
    // We won't limit the handler used per fabric on platforms that are using heap for memory pools, so we introduces a flag to
    // enforce such check based on the configured size. This flag is used for unit tests only, there is another compare time flag
    // CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK for stress tests.
    //
    void SetForceHandlerQuota(bool forceHandlerQuota) { mForceHandlerQuota = forceHandlerQuota; }

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    //
    // Override the subscription timeout resumption retry interval seconds. The default retry interval will be
    // 300s + GetFibonacciForIndex(retry_times) * 300s, which is too long for unit-tests.
    //
    // If -1 is passed in, no override is instituted and default behavior resumes.
    //
    void SetSubscriptionTimeoutResumptionRetryIntervalSeconds(int32_t seconds)
    {
        mSubscriptionResumptionRetrySecondsOverride = seconds;
    }
#endif

    //
    // When testing subscriptions using the high-level APIs in src/controller/ReadInteraction.h,
    // they don't provide for the ability to shut down those subscriptions after they've been established.
    //
    // So for the purposes of unit tests, add a helper here to shut down and clean-up all active handlers.
    //
    void ShutdownActiveReads()
    {
#if CHIP_CONFIG_ENABLE_READ_CLIENT
        for (auto * readClient = mpActiveReadClientList; readClient != nullptr;)
        {
            readClient->mpImEngine = nullptr;
            auto * tmpClient       = readClient->GetNextClient();
            readClient->SetNextClient(nullptr);
            readClient->Close(CHIP_NO_ERROR);
            readClient = tmpClient;
        }

        //
        // After that, we just null out our tracker.
        //
        mpActiveReadClientList = nullptr;
#endif // CHIP_CONFIG_ENABLE_READ_CLIENT

        mReadHandlers.ReleaseAll();
    }
#endif

    DataModel::Provider * GetDataModelProvider() const;

    // MUST NOT be used while the interaction model engine is running as interaction
    // model functionality (e.g. active reads/writes/subscriptions) rely on data model
    // state
    //
    // Returns the old data model provider value.
    DataModel::Provider * SetDataModelProvider(DataModel::Provider * model);

private:
    /* DataModel::ActionContext implementation */
    Messaging::ExchangeContext * CurrentExchange() override { return mCurrentExchange; }

    friend class reporting::Engine;
    friend class TestCommandInteraction;
    friend class TestInteractionModelEngine;
    friend class SubscriptionResumptionSessionEstablisher;
    using Status = Protocols::InteractionModel::Status;

    void OnDone(CommandResponseSender & apResponderObj) override;
    void OnDone(CommandHandlerImpl & apCommandObj) override;
    void OnDone(ReadHandler & apReadObj) override;

    void TryToResumeSubscriptions();

    ReadHandler::ApplicationCallback * GetAppCallback() override { return mpReadHandlerApplicationCallback; }

    InteractionModelEngine * GetInteractionModelEngine() override { return this; }

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override;

    /**
     * Called when Interaction Model receives a Command Request message.
     */
    Status OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                  System::PacketBufferHandle && aPayload, bool aIsTimedInvoke);
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

    /**
     * This parses the attribute path list to ensure it is well formed. If so, for each path in the list, it will expand to a list
     * of concrete paths and walk each path to check if it has privileges to read that attribute.
     *
     * If there is AT LEAST one "existent path" (as the spec calls it) that has sufficient privilege, aHasValidAttributePath
     * will be set to true. Otherwise, it will be set to false.
     *
     * aRequestedAttributePathCount will be updated to reflect the number of attribute paths in the request.
     *
     *
     */
    CHIP_ERROR ParseAttributePaths(const Access::SubjectDescriptor & aSubjectDescriptor,
                                   AttributePathIBs::Parser & aAttributePathListParser, bool & aHasValidAttributePath,
                                   size_t & aRequestedAttributePathCount);

    /**
     * This parses the event path list to ensure it is well formed. If so, for each path in the list, it will expand to a list
     * of concrete paths and walk each path to check if it has privileges to read that event.
     *
     * If there is AT LEAST one "existent path" (as the spec calls it) that has sufficient privilege, aHasValidEventPath
     * will be set to true. Otherwise, it will be set to false.
     *
     * aRequestedEventPathCount will be updated to reflect the number of event paths in the request.
     */
    CHIP_ERROR ParseEventPaths(const Access::SubjectDescriptor & aSubjectDescriptor, EventPathIBs::Parser & aEventPathListParser,
                               bool & aHasValidEventPath, size_t & aRequestedEventPathCount);

    /**
     * Called when Interaction Model receives a Read Request message.  Errors processing
     * the Read Request are handled entirely within this function.  If the
     * status returned is not Status::Success, the caller will send a status
     * response message with that status.
     */
    Status OnReadInitialRequest(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                System::PacketBufferHandle && aPayload, ReadHandler::InteractionType aInteractionType);

    /**
     * Called when Interaction Model receives a Write Request message.  Errors processing
     * the Write Request are handled entirely within this function. If the
     * status returned is not Status::Success, the caller will send a status
     * response message with that status.
     */
    Status OnWriteRequest(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                          System::PacketBufferHandle && aPayload, bool aIsTimedWrite);

    /**
     * Called when Interaction Model receives a Timed Request message.  Errors processing
     * the Timed Request are handled entirely within this function. The caller pre-sets status to failure and the callee is
     * expected to set it to success if it does not want an automatic status response message to be sent.
     */
    CHIP_ERROR OnTimedRequest(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                              System::PacketBufferHandle && aPayload, Protocols::InteractionModel::Status & aStatus);

    /**This function handles processing of un-solicited ReportData messages on the client, which can
     * only occur post subscription establishment
     */
    Status OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                   System::PacketBufferHandle && aPayload);

    void DispatchCommand(CommandHandlerImpl & apCommandObj, const ConcreteCommandPath & aCommandPath,
                         TLV::TLVReader & apPayload) override;

    Protocols::InteractionModel::Status ValidateCommandCanBeDispatched(const DataModel::InvokeRequest & request) override;

    bool HasActiveRead();

    inline size_t GetPathPoolCapacityForReads() const
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        return (mPathPoolCapacityForReadsOverride == -1) ? CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS
                                                         : static_cast<size_t>(mPathPoolCapacityForReadsOverride);
#else
        return CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS;
#endif
    }

    inline size_t GetReadHandlerPoolCapacityForReads() const
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        return (mReadHandlerCapacityForReadsOverride == -1) ? CHIP_IM_MAX_NUM_READS
                                                            : static_cast<size_t>(mReadHandlerCapacityForReadsOverride);
#else
        return CHIP_IM_MAX_NUM_READS;
#endif
    }

    inline size_t GetPathPoolCapacityForSubscriptions() const
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        return (mPathPoolCapacityForSubscriptionsOverride == -1) ? CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS
                                                                 : static_cast<size_t>(mPathPoolCapacityForSubscriptionsOverride);
#else
        return CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS;
#endif
    }

    inline size_t GetReadHandlerPoolCapacityForSubscriptions() const
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        return (mReadHandlerCapacityForSubscriptionsOverride == -1)
            ? CHIP_IM_MAX_NUM_SUBSCRIPTIONS
            : static_cast<size_t>(mReadHandlerCapacityForSubscriptionsOverride);
#else
        return CHIP_IM_MAX_NUM_SUBSCRIPTIONS;
#endif
    }

    inline uint8_t GetConfigMaxFabrics() const
    {
#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
        return (mMaxNumFabricsOverride == -1) ? CHIP_CONFIG_MAX_FABRICS : static_cast<uint8_t>(mMaxNumFabricsOverride);
#else
        return CHIP_CONFIG_MAX_FABRICS;
#endif
    }

    inline size_t GetGuaranteedReadRequestsPerFabric() const
    {
        return GetReadHandlerPoolCapacityForReads() / GetConfigMaxFabrics();
    }

    /**
     * Verify and ensure (by killing oldest read handlers that make the resources used by the current fabric exceed the fabric
     * quota)
     * - If the subscription uses resources within the per subscription limit, this function will always success by evicting
     * existing subscriptions.
     * - If the subscription uses more than per subscription limit, this function will return PATHS_EXHAUSTED if we are running out
     * of paths.
     *
     * After the checks above, we will try to ensure we have a free Readhandler for processing the subscription.
     *
     * @retval true when we have enough resources for the incoming subscription, false if not.
     */
    bool EnsureResourceForSubscription(FabricIndex aFabricIndex, size_t aRequestedAttributePathCount,
                                       size_t aRequestedEventPathCount);

    /**
     * Verify and ensure (by killing oldest read handlers that make the resources used by the current fabric exceed the fabric
     * quota) the resources for handling a new read transaction with the given resource requirments.
     * - PASE sessions will be counted in a virtual fabric (i.e. kInvalidFabricIndex will be consided as a "valid" fabric in this
     * function)
     * - If the existing resources can serve this read transaction, this function will return Status::Success.
     * - or if the resources used by read transactions in the fabric index meets the per fabric resource limit (i.e. 9 paths & 1
     * read) after accepting this read request, this function will always return Status::Success by evicting existing read
     * transactions from other fabrics which are using more than the guaranteed minimum number of read.
     * - or if the resources used by read transactions in the fabric index will exceed the per fabric resource limit (i.e. 9 paths &
     * 1 read) after accepting this read request, this function will return a failure status without evicting any existing
     * transaction.
     * - However, read transactions on PASE sessions won't evict any existing read transactions when we have already commissioned
     * CHIP_CONFIG_MAX_FABRICS fabrics on the device.
     *
     * @retval Status::Success: The read transaction can be accepted.
     * @retval Status::Busy: The remaining resource is insufficient to handle this read request, and the accessing fabric for this
     * read request will use more resources than we guaranteed, the client is expected to retry later.
     * @retval Status::PathsExhausted: The attribute / event path pool is exhausted, and the read request is requesting more
     * resources than we guaranteed.
     */
    Status EnsureResourceForRead(FabricIndex aFabricIndex, size_t aRequestedAttributePathCount, size_t aRequestedEventPathCount);

    /**
     * Helper for various ShutdownSubscriptions functions.  The subscriptions
     * that match all the provided arguments will be shut down.
     */
    void ShutdownMatchingSubscriptions(const Optional<FabricIndex> & aFabricIndex = NullOptional,
                                       const Optional<NodeId> & aPeerNodeId       = NullOptional);

    /**
     * Validates that the command exists and on success returns the data for the command in `entry`.
     */
    Status CheckCommandExistence(const ConcreteCommandPath & aCommandPath, DataModel::AcceptedCommandEntry & entry);
    Status CheckCommandAccess(const DataModel::InvokeRequest & aRequest, const DataModel::AcceptedCommandEntry & entry);
    Status CheckCommandFlags(const DataModel::InvokeRequest & aRequest, const DataModel::AcceptedCommandEntry & entry);

    /**
     * Check if the given attribute path is a valid path in the data model provider.
     */
    bool IsExistentAttributePath(const ConcreteAttributePath & path);

    static void ResumeSubscriptionsTimerCallback(System::Layer * apSystemLayer, void * apAppState);

    template <typename T, size_t N>
    void ReleasePool(SingleLinkedListNode<T> *& aObjectList, ObjectPool<SingleLinkedListNode<T>, N> & aObjectPool);
    template <typename T, size_t N>
    CHIP_ERROR PushFront(SingleLinkedListNode<T> *& aObjectList, T & aData, ObjectPool<SingleLinkedListNode<T>, N> & aObjectPool);

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    ICDManager * mICDManager = nullptr;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

    ObjectPool<CommandResponseSender, CHIP_IM_MAX_NUM_COMMAND_HANDLER> mCommandResponderObjs;
    ObjectPool<TimedHandler, CHIP_IM_MAX_NUM_TIMED_HANDLER> mTimedHandlers;
    WriteHandler mWriteHandlers[CHIP_IM_MAX_NUM_WRITE_HANDLER];
    reporting::Engine mReportingEngine;
    reporting::ReportScheduler * mReportScheduler = nullptr;

    static constexpr size_t kReservedHandlersForReads = kMinSupportedReadRequestsPerFabric * (CHIP_CONFIG_MAX_FABRICS);
    static constexpr size_t kReservedPathsForReads    = kMinSupportedPathsPerReadRequest * kReservedHandlersForReads;

#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    static_assert(CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS >=
                      CHIP_CONFIG_MAX_FABRICS * (kMinSupportedPathsPerSubscription * kMinSupportedSubscriptionsPerFabric),
                  "CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS is too small to match the requirements of spec 8.5.1");
    static_assert(CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS >=
                      CHIP_CONFIG_MAX_FABRICS * (kMinSupportedReadRequestsPerFabric * kMinSupportedPathsPerReadRequest),
                  "CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS is too small to match the requirements of spec 8.5.1");
    static_assert(CHIP_IM_MAX_NUM_SUBSCRIPTIONS >= CHIP_CONFIG_MAX_FABRICS * kMinSupportedSubscriptionsPerFabric,
                  "CHIP_IM_MAX_NUM_SUBSCRIPTIONS is too small to match the requirements of spec 8.5.1");
    static_assert(CHIP_IM_MAX_NUM_READS >= CHIP_CONFIG_MAX_FABRICS * kMinSupportedReadRequestsPerFabric,
                  "CHIP_IM_MAX_NUM_READS is too small to match the requirements of spec 8.5.1");
#endif

    ObjectPool<SingleLinkedListNode<AttributePathParams>,
               CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS + CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS>
        mAttributePathPool;
    ObjectPool<SingleLinkedListNode<EventPathParams>,
               CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS + CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS>
        mEventPathPool;
    ObjectPool<SingleLinkedListNode<DataVersionFilter>,
               CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_READS + CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS_FOR_SUBSCRIPTIONS>
        mDataVersionFilterPool;

    ObjectPool<ReadHandler, CHIP_IM_MAX_NUM_READS + CHIP_IM_MAX_NUM_SUBSCRIPTIONS> mReadHandlers;

#if CHIP_CONFIG_ENABLE_READ_CLIENT
    ReadClient * mpActiveReadClientList = nullptr;
#endif

    ReadHandler::ApplicationCallback * mpReadHandlerApplicationCallback = nullptr;

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    int mReadHandlerCapacityForSubscriptionsOverride = -1;
    int mPathPoolCapacityForSubscriptionsOverride    = -1;

    int mReadHandlerCapacityForReadsOverride = -1;
    int mPathPoolCapacityForReadsOverride    = -1;

    int mMaxNumFabricsOverride = -1;

    // We won't limit the handler used per fabric on platforms that are using heap for memory pools, so we introduces a flag to
    // enforce such check based on the configured size. This flag is used for unit tests only, there is another compare time flag
    // CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK for stress tests.
    bool mForceHandlerQuota = false;
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    int mSubscriptionResumptionRetrySecondsOverride = -1;
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS
    /**
     * mNumOfSubscriptionsToResume tracks the number of subscriptions that the device will try to resume at its next resumption
     * attempt. At boot up, the attempt will be at the highest min interval of all the subscriptions to resume.
     * When the subscription timeout resumption feature is present, after the boot up attempt, the next attempt will be determined
     * by ComputeTimeSecondsTillNextSubscriptionResumption.
     */
    int8_t mNumOfSubscriptionsToResume = 0;
#if CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    bool HasSubscriptionsToResume();
    uint32_t ComputeTimeSecondsTillNextSubscriptionResumption();
    uint32_t mNumSubscriptionResumptionRetries = 0;
    bool mSubscriptionResumptionScheduled      = false;
#endif // CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS

    FabricTable * mpFabricTable = nullptr;

    CASESessionManager * mpCASESessionMgr = nullptr;

    SubscriptionResumptionStorage * mpSubscriptionResumptionStorage = nullptr;

    DataModel::Provider * mDataModelProvider      = nullptr;
    Messaging::ExchangeContext * mCurrentExchange = nullptr;

    enum class State : uint8_t
    {
        kUninitialized, // The object has not been initialized.
        kInitializing,  // Initial setup is in progress (e.g. setting up mpExchangeMgr).
        kInitialized    // The object has been fully initialized and is ready for use.
    };
    State mState = State::kUninitialized;

    // Changes the current exchange context of a InteractionModelEngine to a given context
    class CurrentExchangeValueScope
    {
    public:
        CurrentExchangeValueScope(InteractionModelEngine & engine, Messaging::ExchangeContext * context) : mEngine(engine)
        {
            mEngine.mCurrentExchange = context;
        }

        ~CurrentExchangeValueScope() { mEngine.mCurrentExchange = nullptr; }

    private:
        InteractionModelEngine & mEngine;
    };
};

} // namespace app
} // namespace chip
