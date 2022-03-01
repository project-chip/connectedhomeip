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

#include <app/ClusterInfo.h>
#include <app/CommandHandler.h>
#include <app/CommandHandlerInterface.h>
#include <app/CommandSender.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/ReadClient.h>
#include <app/ReadHandler.h>
#include <app/StatusResponse.h>
#include <app/TimedHandler.h>
#include <app/WriteClient.h>
#include <app/WriteHandler.h>
#include <app/reporting/Engine.h>
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
class InteractionModelEngine : public Messaging::ExchangeDelegate, public CommandHandler::Callback, public ReadHandler::Callback
{
public:
    /**
     * @brief Retrieve the singleton Interaction Model Engine.
     *
     *  @return  A pointer to the shared InteractionModel Engine
     *
     */
    static InteractionModelEngine * GetInstance(void);

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
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr);

    void Shutdown();

    Messaging::ExchangeManager * GetExchangeManager(void) const { return mpExchangeMgr; };

    /**
     * Tears down an active subscription.
     *
     * @retval #CHIP_ERROR_KEY_NOT_FOUND If the subscription is not found.
     * @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR ShutdownSubscription(uint64_t aSubscriptionId);

    /**
     * Tears down active subscriptions for a given peer node ID.
     *
     * @retval #CHIP_ERROR_KEY_NOT_FOUND If no active subscription is found.
     * @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR ShutdownSubscriptions(FabricIndex aFabricIndex, NodeId aPeerNodeId);

    uint32_t GetNumActiveReadHandlers() const;
    uint32_t GetNumActiveReadHandlers(ReadHandler::InteractionType type) const;

    uint32_t GetNumActiveWriteHandlers() const;

    /**
     * Returns the handler at a particular index within the active handler list.
     */
    ReadHandler * ActiveHandlerAt(unsigned int aIndex);

    /**
     * The Magic number of this InteractionModelEngine, the magic number is set during Init()
     */
    uint32_t GetMagicNumber() { return mMagic; }

    reporting::Engine & GetReportingEngine() { return mReportingEngine; }

    void ReleaseClusterInfoList(ClusterInfo *& aClusterInfo);
    CHIP_ERROR PushFront(ClusterInfo *& aClusterInfoLisst, ClusterInfo & aClusterInfo);
    bool IsOverlappedAttributePath(ClusterInfo & aAttributePath);

    CHIP_ERROR RegisterCommandHandler(CommandHandlerInterface * handler);
    CHIP_ERROR UnregisterCommandHandler(CommandHandlerInterface * handler);
    CommandHandlerInterface * FindCommandHandler(EndpointId endpointId, ClusterId clusterId);
    void UnregisterCommandHandlers(EndpointId endpointId);

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
     * Returns whether the write operation to the given path is conflict with another write operations. (i.e. another write
     * transaction is in the middle of processing the chunked value of the given path.)
     */
    bool HasConflictWriteRequests(const WriteHandler * apWriteHandler, const ConcreteAttributePath & aPath);

#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    //
    // Get direct access to the underlying read handler pool
    //
    auto & GetReadHandlerPool() { return mReadHandlers; }

    //
    // Override the maximal capacity of the underlying read handler pool to mimic
    // out of memory scenarios in unit-tests.
    //
    // If -1 is passed in, no override is instituted and default behavior resumes.
    //
    void SetHandlerCapacity(int32_t sz) { mReadHandlerCapacityOverride = sz; }

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
     * the Read Request are handled entirely within this function. The caller pre-sets status to failure and the callee is
     * expected to set it to success if it does not want an automatic status response message to be sent.
     */

    CHIP_ERROR OnReadInitialRequest(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                    System::PacketBufferHandle && aPayload, ReadHandler::InteractionType aInteractionType,
                                    Protocols::InteractionModel::Status & aStatus);

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

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;

    CommandHandlerInterface * mCommandHandlerList = nullptr;

    ObjectPool<CommandHandler, CHIP_IM_MAX_NUM_COMMAND_HANDLER> mCommandHandlerObjs;
    ObjectPool<TimedHandler, CHIP_IM_MAX_NUM_TIMED_HANDLER> mTimedHandlers;
    ObjectPool<ReadHandler, CHIP_IM_MAX_NUM_READ_HANDLER> mReadHandlers;
    WriteHandler mWriteHandlers[CHIP_IM_MAX_NUM_WRITE_HANDLER];
    reporting::Engine mReportingEngine;
    ObjectPool<ClusterInfo, CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS> mClusterInfoPool;

    ReadClient * mpActiveReadClientList = nullptr;

#if CONFIG_IM_BUILD_FOR_UNIT_TEST
    int mReadHandlerCapacityOverride = -1;
#endif

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
 *  When the endpoint / cluster / attribute / event data specified by aClusterInfo does not exist, corresponding interaction model
 * error code will be put into the writer, and CHIP_NO_ERROR will be returned.
 *  If the data exists on the server, the data (with tag kData) and the data version (with tag kDataVersion) will be put
 * into the TLVWriter. TLVWriter error will be returned if any error occurred during encoding
 * these values.
 *  This function is implemented by CHIP as a part of cluster data storage & management.
 * The apWriter and apDataExists can be nullptr.
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
 * TODO: Document.
 */
CHIP_ERROR WriteSingleClusterData(const Access::SubjectDescriptor & aSubjectDescriptor,
                                  const ConcreteDataAttributePath & aAttributePath, TLV::TLVReader & aReader,
                                  WriteHandler * apWriteHandler);

/**
 * Check if the given cluster has the given DataVersion.
 */
bool IsClusterDataVersionEqual(const ConcreteClusterPath & aConcreteClusterPath, DataVersion aRequiredVersion);
} // namespace app
} // namespace chip
