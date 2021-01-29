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

#include <app/MessageDef/MessageDef.h>
#include <core/CHIPCore.h>
#include <map>
#include <messaging/ExchangeContext.h>
#include <messaging/ExchangeMgr.h>
#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <support/CodeUtils.h>
#include <support/DLLUtil.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemPacketBuffer.h>

#include <app/Command.h>
#include <app/CommandHandler.h>
#include <app/CommandSender.h>

#define CHIP_MAX_NUM_COMMAND_HANDLER_OBJECTS 1
#define CHIP_MAX_NUM_COMMAND_SENDER_OBJECTS 1

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
            0, ///< Called when an incoming invoke command request has arrived before applying commands..
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
            const PacketHeader * mpPacketHeader; ///< A pointer to the message information for the request
        } mIncomingInvokeCommandRequest;
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
            bool mShouldContinueProcessing; ///< Set to true if update is allowed.
        } mIncomingInvokeCommandRequest;
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
     * @brief Retrieve the singleton DataManagement Engine. Note this function should be implemented by the
     *  adoption layer.
     *
     *  @return  A pointer to the shared InteractionModel Engine
     *
     */
    static InteractionModelEngine * GetInstance(void);

    InteractionModelEngine(void);

    CHIP_ERROR Init(Messaging::ExchangeManager * apExchangeMgr);

    void Shutdown();

    CHIP_ERROR DeregisterClusterCommandHandler(chip::ClusterId aClusterId, chip::CommandId aCommandId,
                                               Command::CommandRoleId aCommandRoleId);
    CHIP_ERROR RegisterClusterCommandHandler(chip::ClusterId aClusterId, chip::CommandId aCommandId,
                                             Command::CommandRoleId aCommandRoleId, CommandCbFunct aDispatcher);
    void ProcessCommand(chip::ClusterId aClusterId, chip::CommandId aCommandId, chip::TLV::TLVReader & aReader,
                        Command * apCommandObj, Command::CommandRoleId aCommandRoleId);

    Messaging::ExchangeManager * GetExchangeManager(void) const { return mpExchangeMgr; };

    CHIP_ERROR NewCommandSender(CommandSender ** const apComandSender);

private:
    void OnUnknownMsgType(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader, uint32_t aProtocolId,
                          uint8_t aMsgType, System::PacketBufferHandle aPayload);
    void OnInvokeCommandRequest(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader, uint32_t aProtocolId,
                                uint8_t aMsgType, System::PacketBufferHandle aPayload);
    void OnMessageReceived(Messaging::ExchangeContext * apEc, const PacketHeader & aPacketHeader, uint32_t aProtocolId,
                           uint8_t aMsgType, System::PacketBufferHandle aPayload);
    void OnResponseTimeout(Messaging::ExchangeContext * ec);

    struct HandlerKey
    {
        HandlerKey(chip::ClusterId aClusterId, chip::CommandId aCommandId, Command::CommandRoleId aCommandRoleId);

        chip::ClusterId mClusterId;
        chip::CommandId mCommandId;
        Command::CommandRoleId mCommandRoleId;
        bool operator<(const HandlerKey & aOtherKey) const;
    };

    typedef std::map<HandlerKey, CommandCbFunct> HandlersMapType;

    HandlersMapType mHandlersMap;
    Messaging::ExchangeManager * mpExchangeMgr = nullptr;
    void * mpAppState                          = nullptr;
    EventCallback mEventCallback;
    CommandHandler mCommandHandlerObjs[CHIP_MAX_NUM_COMMAND_HANDLER_OBJECTS];
    CommandSender mCommandSenderObjs[CHIP_MAX_NUM_COMMAND_SENDER_OBJECTS];
};

} // namespace app
} // namespace chip

#endif //_CHIP_INTERACTION_MODEL_ENGINE_H
