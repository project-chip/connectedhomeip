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
#include <core/CHIPCore.h>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <protocols/interaction_model/Constants.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

#include <app/ClusterInfo.h>
#include <app/Command.h>
#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/InteractionModelDelegate.h>
#include <app/ReadClient.h>
#include <app/ReadHandler.h>
#include <app/WriteClient.h>
#include <app/WriteHandler.h>
#include <app/reporting/Engine.h>
#include <app/util/basic-types.h>

// TODO: Make number of command handler and command sender configurable
#define CHIP_MAX_NUM_COMMAND_HANDLER 4
#define CHIP_MAX_NUM_COMMAND_SENDER 4
#define CHIP_MAX_NUM_READ_CLIENT 1
#define CHIP_MAX_NUM_READ_HANDLER 1
#define CHIP_MAX_REPORTS_IN_FLIGHT 1
#define IM_SERVER_MAX_NUM_PATH_GROUPS 8
#define CHIP_MAX_NUM_WRITE_CLIENT 1
#define CHIP_MAX_NUM_WRITE_HANDLER 1

namespace chip {
namespace app {

constexpr size_t kMaxSecureSduLengthBytes = 1024;
/* TODO: https://github.com/project-chip/connectedhomeip/issues/7489 */
constexpr uint32_t kImMessageTimeoutMsec = 12000;
constexpr FieldId kRootFieldId           = 0;

/**
 * @class InteractionModelEngine
 *
 * @brief This is a singleton hosting all CHIP unsolicited message processing and managing interaction model related clients and
 * handlers
 *
 */
class InteractionModelEngine : public Messaging::ExchangeDelegate
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
     *  Retrieve a CommandSender that the SDK consumer can use to send a set of commands.  If the call succeeds,
     *  see CommandSender documentation for lifetime handling.
     *
     *  @param[out]    apCommandSender    A pointer to the CommandSender object.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If there is no CommandSender available
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR NewCommandSender(CommandSender ** const apCommandSender);

    /**
     *  Retrieve a ReadClient that the SDK consumer can use to send do a read.  If the call succeeds, the consumer
     *  is responsible for calling Shutdown() on the ReadClient once it's done using it.
     *
     *  @param[out]    apReadClient    A pointer to the ReadClient object.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If there is no ReadClient available
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR NewReadClient(ReadClient ** const apReadClient);

    /**
     *  Retrieve a WriteClient that the SDK consumer can use to send do a write.  If the call succeeds, the consumer
     *  is responsible for calling Shutdown() on the WriteClient once it's done using it.
     *
     *  @param[out]    apWriteClient    A pointer to the WriteClient object.
     *
     *  @retval #CHIP_ERROR_INCORRECT_STATE If there is no WriteClient available
     *  @retval #CHIP_NO_ERROR On success.
     */
    CHIP_ERROR NewWriteClient(WriteClient ** const apWriteClient);

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

private:
    friend class reporting::Engine;
    CHIP_ERROR OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload);
    CHIP_ERROR OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                      const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload);
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                 const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload);
    void OnResponseTimeout(Messaging::ExchangeContext * ec);

    /**
     * Called when Interaction Model receives a Read Request message.  Errors processing
     * the Read Request are handled entirely within this function.
     */
    CHIP_ERROR OnReadRequest(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                             const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload);

    /**
     * Called when Interaction Model receives a Write Request message.  Errors processing
     * the Write Request are handled entirely within this function.
     */
    CHIP_ERROR OnWriteRequest(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                              const PayloadHeader & aPayloadHeader, System::PacketBufferHandle && aPayload);

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    InteractionModelDelegate * mpDelegate      = nullptr;
    CommandHandler mCommandHandlerObjs[CHIP_MAX_NUM_COMMAND_HANDLER];
    CommandSender mCommandSenderObjs[CHIP_MAX_NUM_COMMAND_SENDER];
    ReadClient mReadClients[CHIP_MAX_NUM_READ_CLIENT];
    ReadHandler mReadHandlers[CHIP_MAX_NUM_READ_HANDLER];
    WriteClient mWriteClients[CHIP_MAX_NUM_WRITE_CLIENT];
    WriteHandler mWriteHandlers[CHIP_MAX_NUM_WRITE_HANDLER];
    reporting::Engine mReportingEngine;
    ClusterInfo mClusterInfoPool[IM_SERVER_MAX_NUM_PATH_GROUPS];
    ClusterInfo * mpNextAvailableClusterInfo = nullptr;
};

void DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                  chip::TLV::TLVReader & aReader, Command * apCommandObj);

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
bool ServerClusterCommandExists(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId);
CHIP_ERROR ReadSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVWriter & aWriter);
CHIP_ERROR WriteSingleClusterData(ClusterInfo & aClusterInfo, TLV::TLVReader & aReader, WriteHandler * apWriteHandler);
} // namespace app
} // namespace chip
