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
#include <app/MessageDef/AttributeReportIBs.h>
#include <app/MessageDef/ReportDataMessage.h>
#include <lib/core/CHIPCore.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/DLLUtil.h>
#include <lib/support/Pool.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <protocols/interaction_model/Constants.h>
#include <system/SystemPacketBuffer.h>

#include <app/AttributePathParams.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandSender.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/DataVersionFilter.h>
#include <app/EventPathParams.h>
#include <app/ObjectList.h>
#include <app/ReadClient.h>
#include <app/ReadHandler.h>
#include <app/StatusResponse.h>
#include <app/TimedHandler.h>
#include <app/WriteClient.h>
#include <app/WriteHandler.h>
#include <app/reporting/Engine.h>
#include <app/util/attribute-metadata.h>
#include <app/util/basic-types.h>

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
                               public CommandHandler::Callback,
                               public ReadHandler::ManagementCallback
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
    static constexpr size_t kMinSupportedSubscriptionsPerFabric           = 2;
    static constexpr size_t kMinSupportedPathsPerSubscription             = 2;
    static constexpr size_t kReservedPathsPerReadRequest                  = 9;
    static constexpr size_t kReservedReadHandlersPerFabricForReadRequests = 1;

    // TODO: Per spec, the above numbers should be 3, 3, 9, 1, however, we use a lower limit to reduce the memory usage and should
    // fix it when we have reduced the memory footprint of ReadHandlers.

    InteractionModelEngine(void);

    /**
     *  Initialize the InteractionModel Engine.
     *
     *  @param[in]    apExchangeMgr    A pointer to the ExchangeManager object.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, FabricTable * apFabricTable);

    void Shutdown();

    Messaging::ExchangeManager * GetExchangeManager(void) const { return mpExchangeMgr; };

    /**
     * Tears down an active subscription.
     *
     * @retval #CHIP_ERROR_KEY_NOT_FOUND If the subscription is not found.
     * @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR ShutdownSubscription(SubscriptionId aSubscriptionId);

    /**
     * Tears down active subscriptions for a given peer node ID.
     *
     * @retval #CHIP_ERROR_KEY_NOT_FOUND If no active subscription is found.
     * @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR ShutdownSubscriptions(FabricIndex aFabricIndex, NodeId aPeerNodeId);

    /**
     * Expire active transactions and release related objects for the given fabric index.
     * This is used for releasing transactions that won't be closed when a fabric is removed.
     */
    void CloseTransactionsFromFabricIndex(FabricIndex aFabricIndex);

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
     * The Magic number of this InteractionModelEngine, the magic number is set during Init()
     */
    uint32_t GetMagicNumber() const { return mMagic; }

    reporting::Engine & GetReportingEngine() { return mReportingEngine; }

    void ReleaseAttributePathList(ObjectList<AttributePathParams> *& aAttributePathList);

    CHIP_ERROR PushFrontAttributePathList(ObjectList<AttributePathParams> *& aAttributePathList,
                                          AttributePathParams & aAttributePath);

    // If a concrete path indicates an attribute that is also referenced by a wildcard path in the request,
    // the path SHALL be removed from the list.
    void RemoveDuplicateConcreteAttributePath(ObjectList<AttributePathParams> *& aAttributePaths);

    void ReleaseEventPathList(ObjectList<EventPathParams> *& aEventPathList);

    CHIP_ERROR PushFrontEventPathParamsList(ObjectList<EventPathParams> *& aEventPathList, EventPathParams & aEventPath);

    void ReleaseDataVersionFilterList(ObjectList<DataVersionFilter> *& aDataVersionFilterList);

    CHIP_ERROR PushFrontDataVersionFilterList(ObjectList<DataVersionFilter> *& aDataVersionFilterList,
                                              DataVersionFilter & aDataVersionFilter);

    CHIP_ERROR RegisterCommandHandler(CommandHandlerInterface * handler);
    CHIP_ERROR UnregisterCommandHandler(CommandHandlerInterface * handler);
    CommandHandlerInterface * FindCommandHandler(EndpointId endpointId, ClusterId clusterId);
    void UnregisterCommandHandlers(EndpointId endpointId);

    /*
     * Register an application callback to be notified of notable events when handling reads/subscribes.
     */
    void RegisterReadHandlerAppCallback(ReadHandler::ApplicationCallback * mpApplicationCallback)
    {
        mpReadHandlerApplicationCallback = mpApplicationCallback;
    }
    void UnregisterReadHandlerAppCallback() { mpReadHandlerApplicationCallback = nullptr; }

    /**
     * Called when a timed interaction has failed (i.e. the exchange it was
     * happening on has closed while the exchange delegate was the timed
     * handler).
     */
    void OnTimedInteractionFailed(TimedHandler * apTimedHandler);

    /**
     * Called when a timed invoke is received.  This function takes over all
     * handling of the exchange, status reporting, and so forth.
     */
    void OnTimedInvoke(TimedHandler * apTimedHandler, Messaging::ExchangeContext * apExchangeContext,
                       const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload);

    /**
     * Called when a timed write is received.  This function takes over all
     * handling of the exchange, status reporting, and so forth.
     */
    void OnTimedWrite(TimedHandler * apTimedHandler, Messaging::ExchangeContext * apExchangeContext,
                      const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload);

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

    /**
     * Returns the number of dirty subscriptions. Including the subscriptions that are generating reports.
     */
    size_t GetNumDirtySubscriptions() const;

    /**
     * Returns whether the write operation to the given path is conflict with another write operations. (i.e. another write
     * transaction is in the middle of processing the chunked value of the given path.)
     */
    bool HasConflictWriteRequests(const WriteHandler * apWriteHandler, const ConcreteAttributePath & aPath);

    /**
     * We only allow one active read transaction per fabric, and the number of paths used is limited by
     * kReservedPathsPerReadRequest. This function will check if the given ReadHandler will exceed the limitations for the accessing
     * fabric.
     *
     * If CHIP_NO_ERROR is returned, it's OK to proceed with the read.
     *
     * Otherwise the CHIP_ERROR encodes an interaction model status that needs
     * to turn into a Status Response to the client.
     *
     * TODO: (#17418) We are now reserving resources for read requests, could be changed to similar algorithm for read resources
     * minimas.
     */
    CHIP_ERROR CanEstablishReadTransaction(const ReadHandler * apReadHandler);

    /**
     * Select the oldest (and the one that exceeds the per subscription resource minimum if there are any) read handler on the
     * fabric with the given fabric index. Evict it when the fabric uses more resources than the per fabric quota or aForceEvict is
     * true.
     *
     * @retval Whether we have evicted a subscription.
     */
    bool TrimFabric(FabricIndex aFabricIndex, bool aForceEvict);

    uint16_t GetMinSubscriptionsPerFabric() const;

#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    //
    // Get direct access to the underlying read handler pool
    //
    auto & GetReadHandlerPool() { return mReadHandlers; }

    //
    // Override the maximal capacity of the underlying read handler pool to mimic
    // out of memory scenarios in unit-tests.
    //
    // This function did not considered the resources reserved for read handlers,
    // SetHandlerCapacityForSubscriptions if there are subscriptions in the tests.
    //
    // If -1 is passed in, no override is instituted and default behavior resumes.
    //
    void SetHandlerCapacity(int32_t sz) { mReadHandlerCapacityOverride = sz; }

    //
    // Override the maximal capacity of the underlying attribute path pool and event path pool to mimic
    // out of paths exhausted scenarios in unit-tests.
    //
    // This function did not considered the resources reserved for read handlers,
    // SetPathPoolCapacityForSubscriptions if there are subscriptions in the tests.
    //
    // If -1 is passed in, no override is instituted and default behavior resumes.
    //
    void SetPathPoolCapacity(int32_t sz) { mPathPoolCapacityOverride = sz; }

    //
    // Override the maximal capacity of the underlying read handler pool to mimic
    // out of memory scenarios in unit-tests.
    //
    // If -1 is passed in, no override is instituted and default behavior resumes.
    //
    void SetHandlerCapacityForSubscriptions(int32_t sz)
    {
        SetHandlerCapacity(sz == -1 ? -1 : sz + static_cast<int32_t>(kReservedHandlersForReads));
    }

    //
    // Override the maximal capacity of the underlying attribute path pool and event path pool to mimic
    // out of paths exhausted scenarios in unit-tests.
    //
    // If -1 is passed in, no override is instituted and default behavior resumes.
    //
    void SetPathPoolCapacityForSubscriptions(int32_t sz)
    {
        SetPathPoolCapacity(sz == -1 ? -1 : sz + static_cast<int32_t>(kReservedPathsForReads));
    }

    //
    // We won't limit the handler used per fabric on platforms that are using heap for memory pools, so we introduces a flag to
    // enforce such check based on the configured size. This flag is used for unit tests only, there is another compare time flag
    // CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK for stress tests.
    //
    void SetForceHandlerQuota(bool forceHandlerQuota) { mForceHandlerQuota = forceHandlerQuota; }

    //
    // When testing subscriptions using the high-level APIs in src/controller/ReadInteraction.h,
    // they don't provide for the ability to shut down those subscriptions after they've been established.
    //
    // So for the purposes of unit tests, add a helper here to shut down and clean-up all active handlers.
    //
    void ShutdownActiveReads()
    {
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

        mReadHandlers.ReleaseAll();
    }
#endif

private:
    friend class reporting::Engine;
    friend class TestCommandInteraction;
    using Status = Protocols::InteractionModel::Status;

    void OnDone(CommandHandler & apCommandObj) override;
    void OnDone(ReadHandler & apReadObj) override;

    ReadHandler::ApplicationCallback * GetAppCallback() override { return mpReadHandlerApplicationCallback; }

    CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) override;

    /**
     * Called when Interaction Model receives a Command Request message.  Errors processing
     * the Command Request are handled entirely within this function. The caller pre-sets status to failure and the callee is
     * expected to set it to success if it does not want an automatic status response message to be sent.
     */
    CHIP_ERROR OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                      System::PacketBufferHandle && aPayload, bool aIsTimedInvoke,
                                      Protocols::InteractionModel::Status & aStatus);
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload) override;
    void OnResponseTimeout(Messaging::ExchangeContext * ec) override;

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
    CHIP_ERROR OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                       System::PacketBufferHandle && aPayload);

    void DispatchCommand(CommandHandler & apCommandObj, const ConcreteCommandPath & aCommandPath,
                         TLV::TLVReader & apPayload) override;
    Protocols::InteractionModel::Status CommandExists(const ConcreteCommandPath & aCommandPath) override;

    bool HasActiveRead();

    CHIP_ERROR ShutdownExistingSubscriptionsIfNeeded(Messaging::ExchangeContext * apExchangeContext,
                                                     System::PacketBufferHandle && aPayload);

    inline size_t GetPathPoolCapacity() const
    {
#if CONFIG_IM_BUILD_FOR_UNIT_TEST
        return (mPathPoolCapacityOverride == -1) ? CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS
                                                 : static_cast<size_t>(mPathPoolCapacityOverride);
#else
        return CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS;
#endif
    }

    inline size_t GetReadHandlerPoolCapacity() const
    {
#if CONFIG_IM_BUILD_FOR_UNIT_TEST
        return (mReadHandlerCapacityOverride == -1) ? CHIP_IM_MAX_NUM_READ_HANDLER
                                                    : static_cast<size_t>(mReadHandlerCapacityOverride);
#else
        return CHIP_IM_MAX_NUM_READ_HANDLER;
#endif
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

    template <typename T, size_t N>
    void ReleasePool(ObjectList<T> *& aObjectList, ObjectPool<ObjectList<T>, N> & aObjectPool);
    template <typename T, size_t N>
    CHIP_ERROR PushFront(ObjectList<T> *& aObjectList, T & aData, ObjectPool<ObjectList<T>, N> & aObjectPool);

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;

    CommandHandlerInterface * mCommandHandlerList = nullptr;

    ObjectPool<CommandHandler, CHIP_IM_MAX_NUM_COMMAND_HANDLER> mCommandHandlerObjs;
    ObjectPool<TimedHandler, CHIP_IM_MAX_NUM_TIMED_HANDLER> mTimedHandlers;
    WriteHandler mWriteHandlers[CHIP_IM_MAX_NUM_WRITE_HANDLER];
    reporting::Engine mReportingEngine;

    static constexpr size_t kReservedHandlersForReads = kReservedReadHandlersPerFabricForReadRequests * (CHIP_CONFIG_MAX_FABRICS);
    static constexpr size_t kReservedPathsForReads    = kReservedPathsPerReadRequest * kReservedHandlersForReads;

#if !CHIP_SYSTEM_CONFIG_POOL_USE_HEAP
    static_assert(CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS >= CHIP_CONFIG_MAX_FABRICS *
                          (kMinSupportedPathsPerSubscription * kMinSupportedSubscriptionsPerFabric + kReservedPathsPerReadRequest),
                  "CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS is too small to match the requirements of spec 8.5.1");
    static_assert(CHIP_IM_MAX_NUM_READ_HANDLER >= CHIP_CONFIG_MAX_FABRICS *
                          (kMinSupportedSubscriptionsPerFabric + kReservedReadHandlersPerFabricForReadRequests),
                  "CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS is too small to match the requirements of spec 8.5.1");
#endif

    ObjectPool<ObjectList<AttributePathParams>, CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS> mAttributePathPool;
    ObjectPool<ObjectList<EventPathParams>, CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS> mEventPathPool;
    ObjectPool<ObjectList<DataVersionFilter>, CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS> mDataVersionFilterPool;

    ObjectPool<ReadHandler, CHIP_IM_MAX_NUM_READ_HANDLER> mReadHandlers;

    ReadClient * mpActiveReadClientList = nullptr;

    ReadHandler::ApplicationCallback * mpReadHandlerApplicationCallback = nullptr;

#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    int mReadHandlerCapacityOverride = -1;
    int mPathPoolCapacityOverride    = -1;

    // We won't limit the handler used per fabric on platforms that are using heap for memory pools, so we introduces a flag to
    // enforce such check based on the configured size. This flag is used for unit tests only, there is another compare time flag
    // CHIP_CONFIG_IM_FORCE_FABRIC_QUOTA_CHECK for stress tests.
    bool mForceHandlerQuota = false;
#endif

    FabricTable * mpFabricTable;

    // A magic number for tracking values between stack Shutdown()-s and Init()-s.
    // An ObjectHandle is valid iff. its magic equals to this one.
    uint32_t mMagic = 0;
};

void DispatchSingleClusterCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj);

/**
 *  Check whether the given cluster exists on the given endpoint and supports
 *  the given command.  If it does, Success will be returned.  If it does not,
 *  one of UnsupportedEndpoint, UnsupportedCluster, or UnsupportedCommand
 *  will be returned, depending on how the command fails to exist.
 */
Protocols::InteractionModel::Status ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath);

/**
 *  Fetch attribute value and version info and write to the AttributeReport provided.
 *  The ReadSingleClusterData will do everything required for encoding an attribute, i.e. it will try to put one or more
 * AttributeReportIB to the AttributeReportIBs::Builder.
 *  When the endpoint / cluster / attribute data specified by aPath does not exist, corresponding interaction
 * model error code will be put into aAttributeReports, and CHIP_NO_ERROR will be returned. If the data exists on the server, the
 * data (with tag kData) and the data version (with tag kDataVersion) will be put into aAttributeReports. TLVWriter error will be
 * returned if any error occurred while encoding these values. This function is implemented by CHIP as a part of cluster data
 * storage & management.
 *
 *  @param[in]    aSubjectDescriptor    The subject descriptor for the read.
 *  @param[in]    aPath                 The concrete path of the data being read.
 *  @param[in]    aAttributeReports      The TLV Builder for Cluter attribute builder.
 *
 *  @retval  CHIP_NO_ERROR on success
 */
CHIP_ERROR ReadSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor, bool aIsFabricFiltered,
                                 const ConcreteReadAttributePath & aPath, AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeValueEncoder::AttributeEncodeState * apEncoderState);

/**
 *  Get the registered attribute access override. nullptr when attribute access override is not found.
 *
 * TODO(#16806): This function and registerAttributeAccessOverride can be member functions of InteractionModelEngine.
 */
AttributeAccessInterface * GetAttributeAccessOverride(EndpointId aEndpointId, ClusterId aClusterId);

/**
 * TODO: Document.
 */
CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor,
                                  const ConcreteDataAttributePath & aAttributePath, TLV::TLVReader & aReader,
                                  WriteHandler * apWriteHandler);

/**
 * Check if the given cluster has the given DataVersion.
 */
bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion);

/**
 * Returns true if device type is on endpoint, false otherwise.
 */
bool IsDeviceTypeOnEndpoint(DeviceTypeId deviceType, EndpointId endpoint);

/**
 * Returns the metadata of the attribute for the given path.
 *
 * @retval The metadata of the attribute, will return null if the given attribute does not exists.
 */
const EmberAfAttributeMetadata * GetAttributeMetadata(const ConcreteAttributePath & aConcreteClusterPath);

} // namespace app
} // namespace chip
