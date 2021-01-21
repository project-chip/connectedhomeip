/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef _CHIP_INTERACTION_MODEL_ENGINE_H
#define _CHIP_INTERACTION_MODEL_ENGINE_H

#include <app/MessageDef/ReportData.h>
#include <core/CHIPCore.h>
#include <map>
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
#include <app/ReadClient.h>
#include <app/ReadHandler.h>
#include <app/reporting/ReportingEngine.h>

#define CHIP_MAX_NUM_COMMAND_HANDLER_OBJECTS 1
#define CHIP_MAX_NUM_COMMAND_SENDER_OBJECTS 1

#define CHIP_MAX_NUM_READ_CLIENT 1
#define CHIP_MAX_NUM_READ_HANDLER 1

namespace chip {
namespace app {

typedef void (*CommandCbFunct)(chip::TLV::TLVReader & aReader, Command * apCommandObj);

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
    enum EventID
    {
        kEvent_OnIncomingInvokeCommandRequest =
            0, //< Called when an incoming invoke command request has arrived before applying commands.>
        kEvent_OnIncomingReadRequest   = 1, //< Called when a read request has arrived.>
        kEvent_OnIncomingReportRequest = 2, //////< Called when a report data has arrived.>
    };

    /**
     * Incoming parameters sent with events generated directly from this component
     *
     */
    union InEventParam
    {
        void Clear(void) { memset(this, 0, sizeof(*this)); }
        struct
        {
            const PacketHeader * mpPacketHeader; ///< A pointer to the message information for the command request
        } mIncomingInvokeCommandRequest;
        struct
        {
            const PacketHeader * mpPacketHeader; ///< A pointer to the message information for the read request
        } mIncomingReadRequest;
    };

    /**
     * Outgoing parameters sent with events generated directly from this component
     *
     */
    union OutEventParam
    {
        void Clear(void) { memset(this, 0, sizeof(*this)); }

        struct
        {
            bool mShouldContinueProcessing; ///< Set to true if invoke command is allowed.
        } mIncomingInvokeCommandRequest;

        struct
        {
            bool mShouldContinueProcessing; ///< Set to true if read requestt is allowed.
        } mIncomingReadRequest;
    };

    /**
     * @brief Set the event back function and pointer to associated state object for SubscriptionEngine specific call backs
     *
     * @param[in]  apAppState    A pointer to application layer supplied state object
     * @param[in]  aEvent       A function pointer for event call back
     * @param[in]  aInParam     A const reference to the input parameter for this event
     * @param[out] aOutParam    A reference to the output parameter for this event
     */
    typedef void (*EventCallback)(void * apAppState, EventID aEvent, const InEventParam & aInParam, OutEventParam & aOutParam);

    /**
     * @brief Set the event back function and pointer to associated state object for SubscriptionEngine specific call backs
     *
     * @param[in]  apAppState  		A pointer to application layer supplied state object
     * @param[in]  aEventCallback  	A function pointer for event call back
     */
    void SetEventCallback(void * apAppState, EventCallback aEventCallback);

    /**
     * @brief This is the default event handler to be called by application layer for any ignored or unrecognized event
     *
     * @param[in]  aEvent       A function pointer for event call back
     * @param[in]  aInParam     A const reference to the input parameter for this event
     * @param[out] aOutParam    A reference to the output parameter for this event
     */
    static void DefaultEventHandler(EventID aEvent, const InEventParam & aInParam, OutEventParam & aOutParam);

    /**
     * @brief Retrieve the singleton Interaction Model Engine. Note this function should be implemented by the
     *  adoption layer.
     *
     *  @return  A pointer to the shared InteractionModel Engine
     *
     */
    static InteractionModelEngine * GetInstance(void);

    InteractionModelEngine(void);

    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr, void * const apAppState, const EventCallback aEventCallback);

    void Shutdown();

    Messaging::ExchangeManager * GetExchangeManager(void) const { return mpExchangeMgr; };

    CHIP_ERROR NewCommandSender(CommandSender ** const apComandSender);

    CHIP_ERROR NewReadClient(ReadClient ** const apReadClient, chip::app::ReadClient::EventCallback aEventCallback,
                             void * const apAppState);

    reporting::ReportingEngine * GetReportingEngine(void) { return &mReportingEngine; }

    ReadHandler mReadHandlers[CHIP_MAX_NUM_READ_HANDLER];

private:
    void OnUnknownMsgType(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader,
                          const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload);
    void OnInvokeCommandRequest(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader,
                                const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload);
    void OnMessageReceived(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader,
                           const PayloadHeader & aPayloadHeader, System::PacketBufferHandle aPayload);
    void OnResponseTimeout(Messaging::ExchangeContext * ec);

    friend class ReadClient;
    friend class ReportingEngine;

    void OnReadRequest(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader, const PayloadHeader & aPayloadHeader,
                       System::PacketBufferHandle aPayload);

    uint16_t GetCommandSenderId(const CommandSender * const apSender) const;

    uint16_t GetReadClientId(const ReadClient * const apClient) const;

    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    void * mpAppState                          = nullptr;
    EventCallback mEventCallback;
    CommandHandler mCommandHandlerObjs[CHIP_MAX_NUM_COMMAND_HANDLER_OBJECTS];
    CommandSender mCommandSenderObjs[CHIP_MAX_NUM_COMMAND_SENDER_OBJECTS];

    ReadClient mReadClients[CHIP_MAX_NUM_READ_CLIENT];
    reporting::ReportingEngine mReportingEngine;
};

void DispatchSingleClusterCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::EndpointId aEndPointId,
                                  chip::TLV::TLVReader & aReader, Command * apCommandObj);

} // namespace app
} // namespace chip

#endif //_CHIP_INTERACTION_MODEL_ENGINE_H
