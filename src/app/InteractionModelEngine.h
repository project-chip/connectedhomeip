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

#pragma once

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
#include <app/InteractionModelDelegate.h>
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
class InteractionModelEngine : public Messaging::ExchangeDelegate, public CommandHandler::Callback
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
     *  @param[in]    apDelegate       InteractionModelDelegate set by application.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If the state is not equal to
     *          kState_NotInitialized.
     *  @retval #CHIP_NO_ERROR On success.
     *
     */
    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, InteractionModelDelegate * apDelegate);

    void Shutdown();

    Messaging::ExchangeManager * GetExchangeManager(void) const { return mpExchangeMgr; };

    /**
     *  Creates a new read client and send ReadRequest message to the node using the read client,
     *  shutdown if fail to send it out
     *
     *  @retval #CHIP_ERROR_NO_MEMORY If there is no ReadClient available
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR SendReadRequest(ReadPrepareParams & aReadPrepareParams, ReadClient::Callback * aCallback);

    /**
     *  Creates a new read client and sends SubscribeRequest message to the node using the read client.
     *  Shuts down on transmission failure.
     *
     *  @retval #CHIP_ERROR_NO_MEMORY If there is no ReadClient available
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR SendSubscribeRequest(ReadPrepareParams & aReadPrepareParams, ReadClient::Callback * aCallback);

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

    /**
     *  Retrieve a WriteClient that the SDK consumer can use to send a write.  If the call succeeds,
     *  see WriteClient documentation for lifetime handling.
     *
     *  The Write interaction is more like Invoke interaction (cluster specific commands) since it will include cluster specific
     * payload, and may have the need to encode non-scalar values (like structs and arrays). Thus we use WriteClientHandle to
     * prevent user's code from leaking WriteClients.
     *
     *  @param[out]    apWriteClient    A pointer to the WriteClient object.
     *
     *  @retval #CHIP_ERROR_NO_MEMORY If there is no WriteClient available
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR NewWriteClient(WriteClientHandle & apWriteClient, WriteClient::Callback * callback,
                              const Optional<uint16_t> & aTimedWriteTimeoutMs = NullOptional);

    /**
     *  Allocate a ReadClient that can be used to do a read interaction.  If the call succeeds, the consumer
     *  is responsible for calling Shutdown() on the ReadClient once it's done using it.
     *
     *  @param[in,out] 	apReadClient	      A double pointer to a ReadClient that is updated to point to a valid ReadClient
     *                                      on successful completion of this function. On failure, it will be updated to point to
     *                                      nullptr.
     *  @param[in]      aInteractionType    Type of interaction (read or subscription) that the requested ReadClient should execute.
     *  @param[in]      aCallback           If not-null, permits overriding the default delegate registered with the
     *                                      InteractionModelEngine that will be used by the ReadClient.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If there is no ReadClient available
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR NewReadClient(ReadClient ** const apReadClient, ReadClient::InteractionType aInteractionType,
                             ReadClient::Callback * aCallback);

    uint32_t GetNumActiveReadHandlers() const;
    uint32_t GetNumActiveReadClients() const;

    uint32_t GetNumActiveWriteHandlers() const;
    uint32_t GetNumActiveWriteClients() const;

    /**
     *  Get read client index in mReadClients
     *
     *  @param[in]    apReadClient    A pointer to a read client object.
     *
     *  @retval  the index in mReadClients array
     */
    uint16_t GetReadClientArrayIndex(const ReadClient * const apReadClient) const;

    uint16_t GetWriteClientArrayIndex(const WriteClient * const apWriteClient) const;

    uint16_t GetReadHandlerArrayIndex(const ReadHandler * const apReadHandler) const;
    /**
     * The Magic number of this InteractionModelEngine, the magic number is set during Init()
     */
    uint32_t GetMagicNumber() { return mMagic; }

    reporting::Engine & GetReportingEngine() { return mReportingEngine; }

    void ReleaseClusterInfoList(ClusterInfo *& aClusterInfo);
    CHIP_ERROR PushFront(ClusterInfo *& aClusterInfoLisst, ClusterInfo & aClusterInfo);
    // Merges aAttributePath inside apAttributePathList if current path is overlapped with existing path in apAttributePathList
    // Overlap means the path is superset or subset of another path
    bool MergeOverlappedAttributePath(ClusterInfo * apAttributePathList, ClusterInfo & aAttributePath);
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

private:
    friend class reporting::Engine;
    friend class TestCommandInteraction;
    using Status = Protocols::InteractionModel::Status;

    void OnDone(CommandHandler & apCommandObj) override;

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
    bool CommandExists(const ConcreteCommandPath & aCommandPath) override;

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    InteractionModelDelegate * mpDelegate      = nullptr;

    CommandHandlerInterface * mCommandHandlerList = nullptr;

    // TODO(#8006): investgate if we can disable some IM functions on some compact accessories.
    // TODO(#8006): investgate if we can provide more flexible object management on devices with more resources.
    BitMapObjectPool<CommandHandler, CHIP_IM_MAX_NUM_COMMAND_HANDLER> mCommandHandlerObjs;
    BitMapObjectPool<TimedHandler, CHIP_IM_MAX_NUM_TIMED_HANDLER, OnObjectPoolDestruction::IgnoreUnsafeDoNotUseInNewCode>
        mTimedHandlers;
    ReadClient mReadClients[CHIP_IM_MAX_NUM_READ_CLIENT];
    ReadHandler mReadHandlers[CHIP_IM_MAX_NUM_READ_HANDLER];
    WriteClient mWriteClients[CHIP_IM_MAX_NUM_WRITE_CLIENT];
    WriteHandler mWriteHandlers[CHIP_IM_MAX_NUM_WRITE_HANDLER];
    reporting::Engine mReportingEngine;
    ClusterInfo mClusterInfoPool[CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS];
    ClusterInfo * mpNextAvailableClusterInfo = nullptr;

    // A magic number for tracking values between stack Shutdown()-s and Init()-s.
    // An ObjectHandle is valid iff. its magic equals to this one.
    uint32_t mMagic = 0;
};

void DispatchSingleClusterCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                  CommandHandler * apCommandObj);
void DispatchSingleClusterResponseCommand(const ConcreteCommandPath & aCommandPath, chip::TLV::TLVReader & aReader,
                                          CommandSender * apCommandObj);

/**
 *  Check whether the given cluster exists on the given endpoint and supports the given command.
 *  TODO: The implementation lives in ember-compatibility-functions.cpp, this should be replaced by IM command catalog look up
 * function after we have a cluster catalog in interaction model engine.
 *  TODO: The endpoint id on response command (client side command) is unclear, so we don't have a ClientClusterCommandExists
 * function. (Spec#3258)
 *
 *  @retval  True if the endpoint contains the server side of the given cluster and that cluster implements the given command, false
 * otherwise.
 */
bool ServerClusterCommandExists(const ConcreteCommandPath & aCommandPath);

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
 *  @param[in]    aAccessingFabricIndex The accessing fabric index for the read.
 *  @param[in]    aPath             The concrete path of the data being read.
 *  @param[in]    aAttributeReport  The TLV Builder for Cluter attribute builder.
 *
 *  @retval  CHIP_NO_ERROR on success
 */
CHIP_ERROR ReadSingleClusterData(FabricIndex aAccessingFabricIndex, const ConcreteReadAttributePath & aPath,
                                 AttributeReportIBs::Builder & aAttributeReports,
                                 AttributeValueEncoder::AttributeEncodeState * apEncoderState);

/**
 * TODO: Document.
 */
CHIP_ERROR WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler * apWriteHandler);
} // namespace app
} // namespace chip
