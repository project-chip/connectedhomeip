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

#include <app/MessageDef/ReportData.h>
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
#include <app/Command.h>
#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/ConcreteAttributePath.h>
#include <app/ConcreteCommandPath.h>
#include <app/InteractionModelDelegate.h>
#include <app/ReadClient.h>
#include <app/ReadHandler.h>
#include <app/WriteClient.h>
#include <app/WriteHandler.h>
#include <app/reporting/Engine.h>
#include <app/util/basic-types.h>

namespace chip {
namespace app {

static constexpr size_t kMaxSecureSduLengthBytes = 1024;

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
    CHIP_ERROR SendReadRequest(ReadPrepareParams & aReadPrepareParams, uint64_t aAppIdentifier = 0);

    /**
     *  Creates a new read client and sends SubscribeRequest message to the node using the read client.
     *  Shuts down on transmission failure.
     *
     *  @retval #CHIP_ERROR_NO_MEMORY If there is no ReadClient available
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR SendSubscribeRequest(ReadPrepareParams & aReadPrepareParams, uint64_t aAppIdentifier = 0);

    /**
     * Tears down an active subscription.
     *
     * @retval #CHIP_ERROR_KEY_NOT_FOUND If the subscription is not found.
     * @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR ShutdownSubscription(uint64_t aSubscriptionId);
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
    CHIP_ERROR NewWriteClient(WriteClientHandle & apWriteClient, WriteClient::Callback * callback);

    /**
     *  Allocate a ReadClient that can be used to do a read interaction.  If the call succeeds, the consumer
     *  is responsible for calling Shutdown() on the ReadClient once it's done using it.
     *
     *  @param[inout] 	apReadClient	    A double pointer to a ReadClient that is updated to point to a valid ReadClient
     *                                      on successful completion of this function. On failure, it will be updated to point to
     *                                      nullptr.
     *  @param[in]      aInteractionType    Type of interaction (read or subscription) that the requested ReadClient should execute.
     *  @param[in]      aAppIdentifier      A unique token that can be attached to the returned ReadClient object that will be
     *                                      passed through some of the methods in the registered InteractionModelDelegate.
     *  @param[in]      apDelegateOverride  If not-null, permits overriding the default delegate registered with the
     *                                      InteractionModelEngine that will be used by the ReadClient.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If there is no ReadClient available
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR NewReadClient(ReadClient ** const apReadClient, ReadClient::InteractionType aInteractionType,
                             uint64_t aAppIdentifier, InteractionModelDelegate * apDelegateOverride = nullptr);

    uint32_t GetNumActiveReadHandlers() const;
    uint32_t GetNumActiveReadClients() const;

    /**
     *  Get read client index in mReadClients
     *
     *  @param[in]    apReadClient    A pointer to a read client object.
     *
     *  @retval  the index in mReadClients array
     */
    uint16_t GetReadClientArrayIndex(const ReadClient * const apReadClient) const;

    uint16_t GetWriteClientArrayIndex(const WriteClient * const apWriteClient) const;

    reporting::Engine & GetReportingEngine() { return mReportingEngine; }

    void ReleaseClusterInfoList(ClusterInfo *& aClusterInfo);
    CHIP_ERROR PushFront(ClusterInfo *& aClusterInfoLisst, ClusterInfo & aClusterInfo);
    // Merges aAttributePath inside apAttributePathList if current path is overlapped with existing path in apAttributePathList
    // Overlap means the path is superset or subset of another path
    bool MergeOverlappedAttributePath(ClusterInfo * apAttributePathList, ClusterInfo & aAttributePath);
    bool IsOverlappedAttributePath(ClusterInfo & aAttributePath);

private:
    friend class reporting::Engine;
    friend class TestCommandInteraction;

    void OnDone(CommandHandler * apCommandObj);

    CHIP_ERROR OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                System::PacketBufferHandle && aPayload);
    CHIP_ERROR OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                      System::PacketBufferHandle && aPayload);
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                 System::PacketBufferHandle && aPayload);
    void OnResponseTimeout(Messaging::ExchangeContext * ec);

    /**
     * Called when Interaction Model receives a Read Request message.  Errors processing
     * the Read Request are handled entirely within this function.
     */

    CHIP_ERROR OnReadInitialRequest(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                    System::PacketBufferHandle && aPayload, ReadHandler::InteractionType aInteractionType);

    /**
     * Called when Interaction Model receives a Write Request message.  Errors processing
     * the Write Request are handled entirely within this function.
     */
    CHIP_ERROR OnWriteRequest(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                              System::PacketBufferHandle && aPayload);

    /**This function handles processing of un-solicited ReportData messages on the client, which can
     * only occur post subscription establishment
     */
    CHIP_ERROR OnUnsolicitedReportData(Messaging::ExchangeContext * apExchangeContext, const PayloadHeader & aPayloadHeader,
                                       System::PacketBufferHandle && aPayload);

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    InteractionModelDelegate * mpDelegate      = nullptr;

    // TODO(#8006): investgate if we can disable some IM functions on some compact accessories.
    // TODO(#8006): investgate if we can provide more flexible object management on devices with more resources.
    BitMapObjectPool<CommandHandler, CHIP_IM_MAX_NUM_COMMAND_HANDLER> mCommandHandlerObjs;
    ReadClient mReadClients[CHIP_IM_MAX_NUM_READ_CLIENT];
    ReadHandler mReadHandlers[CHIP_IM_MAX_NUM_READ_HANDLER];
    WriteClient mWriteClients[CHIP_IM_MAX_NUM_WRITE_CLIENT];
    WriteHandler mWriteHandlers[CHIP_IM_MAX_NUM_WRITE_HANDLER];
    reporting::Engine mReportingEngine;
    ClusterInfo mClusterInfoPool[CHIP_IM_SERVER_MAX_NUM_PATH_GROUPS];
    ClusterInfo * mpNextAvailableClusterInfo = nullptr;
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
 *  Fetch attribute value and version info and write to the TLVWriter provided.
 *  When the endpoint / cluster / attribute / event data specified by aClusterInfo does not exist, corresponding interaction model
 * error code will be put into the writer, and CHIP_NO_ERROR will be returned and apDataExists will be set to false.
 *  If the data exists on the server, the data (with tag kCsTag_Data) and the data version (with tag kCsTag_DataVersion) will be put
 * into the TLVWriter and apDataExists will be set to true. TLVWriter error will be returned if any error occurred during encoding
 * these values.
 *  This function is implemented by CHIP as a part of cluster data storage & management.
 * The apWriter and apDataExists can be nullptr.
 *
 *  @param[in]    aPath             The concrete path of the data being read.
 *  @param[in]    apWriter          The TLVWriter for holding cluster data. Can be a nullptr if the caller does not care
 *                                  the exact value of the attribute.
 *  @param[out]   apDataExists      Tell whether the cluster data exist on server. Can be a nullptr if the caller does not care
 *                                  whether the data exists.
 *
 *  @retval  CHIP_NO_ERROR on success
 */
CHIP_ERROR ReadSingleClusterData(const ConcreteAttributePath & aPath, TLV::TLVWriter * apWriter, bool * apDataExists);

/**
 * TODO: Document.
 */
CHIP_ERROR WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler * apWriteHandler);
} // namespace app
} // namespace chip
