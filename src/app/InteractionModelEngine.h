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

#include <app/Command.h>
#include <app/CommandHandler.h>
#include <app/CommandSender.h>
#include <app/InteractionModelDelegate.h>
#include <app/ReadClient.h>
#include <app/ReadHandler.h>
#include <app/reporting/Engine.h>

#define CHIP_MAX_NUM_COMMAND_HANDLER 1
#define CHIP_MAX_NUM_COMMAND_SENDER 1
#define CHIP_MAX_NUM_READ_CLIENT 1
#define CHIP_MAX_NUM_READ_HANDLER 1
#define CHIP_MAX_REPORTS_IN_FLIGHT 1

namespace chip {
namespace app {

constexpr size_t kMaxSecureSduLengthBytes = 1024;
constexpr uint32_t kImMessageTimeoutMsec  = 3000;

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
     *  the consumer is responsible for calling Shutdown() on the CommandSender once it's done using it.
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
     *  Get read client index in mReadClients
     *
     *  @param[in]    apReadClient    A pointer to a read client object.
     *
     *  @retval  the index in mReadClients array
     */
    uint16_t GetReadClientArrayIndex(const ReadClient * const apReadClient) const;

    reporting::Engine & GetReportingEngine() { return mReportingEngine; }

private:
    friend class reporting::Engine;
    void OnUnknownMsgType(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                          const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload);
    void OnInvokeCommandRequest(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                                const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload);
    void OnMessageReceived(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload);
    void OnResponseTimeout(Messaging::ExchangeContext * ec);

    /**
     * Called when Interaction Model receives a Read Request message.  Errors processing
     * the Read Request are handled entirely within this function.
     */
    void OnReadRequest(Messaging::ExchangeContext * apExchangeContext, const PacketHeader & aPacketHeader,
                       const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload);

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    InteractionModelDelegate * mpDelegate      = nullptr;
    CommandHandler mCommandHandlerObjs[CHIP_MAX_NUM_COMMAND_HANDLER];
    CommandSender mCommandSenderObjs[CHIP_MAX_NUM_COMMAND_SENDER];
    ReadClient mReadClients[CHIP_MAX_NUM_READ_CLIENT];
    ReadHandler mReadHandlers[CHIP_MAX_NUM_READ_HANDLER];
    reporting::Engine mReportingEngine;
};

void DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                  chip::TLV::TLVReader & aReader, Command * apCommandObj);

} // namespace app
} // namespace chip
