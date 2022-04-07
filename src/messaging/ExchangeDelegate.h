/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *      This file defines the classes corresponding to CHIP Exchange Context Delegate.
 *
 */

#pragma once

#include <lib/support/CHIPMem.h>
#include <messaging/ApplicationExchangeDispatch.h>
#include <messaging/ExchangeMessageDispatch.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SessionManager.h>
#include <transport/raw/MessageHeader.h>

namespace chip {
namespace Messaging {

class ExchangeContext;

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by ExchangeContext object on specific events. If the user of ExchangeContext
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each trigger in their implementation of this class.
 */
class DLL_EXPORT ExchangeDelegate
{
public:
    virtual ~ExchangeDelegate() {}

    /**
     * @brief
     *   This function is the protocol callback for handling a received CHIP
     *   message.
     *
     *   After calling this method an exchange will close itself unless one of
     *   two things happens:
     *
     *   1) A call to SendMessage on the exchange with the kExpectResponse flag
     *      set.
     *   2) A call to WillSendMessage on the exchange.
     *
     *   Consumers that don't do one of those things MUST NOT retain a pointer
     *   to the exchange.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *  @param[in]    payloadHeader A reference to the PayloadHeader object.
     *  @param[in]    payload       A handle to the PacketBuffer object holding the message payload.
     */
    virtual CHIP_ERROR OnMessageReceived(ExchangeContext * ec, const PayloadHeader & payloadHeader,
                                         System::PacketBufferHandle && payload) = 0;

    /**
     * @brief
     *   This function is the protocol callback to invoke when the timeout for the receipt
     *   of a response message has expired.
     *
     *   After calling this method an exchange will close itself unless one of
     *   two things happens:
     *
     *   1) A call to SendMessage on the exchange with the kExpectResponse flag
     *      set.
     *   2) A call to WillSendMessage on the exchange.
     *
     *   Consumers that don't do one of those things MUST NOT retain a pointer
     *   to the exchange.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     */
    virtual void OnResponseTimeout(ExchangeContext * ec) = 0;

    /**
     * @brief
     *   This function is the protocol callback to invoke when the associated
     *   exchange context is being closed
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     */
    virtual void OnExchangeClosing(ExchangeContext * ec) {}

    virtual ExchangeMessageDispatch & GetMessageDispatch() { return ApplicationExchangeDispatch::Instance(); }
};

/**
 * @brief
 *   This class handles unsolicited messages. The implementation can select an exchange delegate to use based on the payload header
 * of the incoming message.
 */
class DLL_EXPORT UnsolicitedMessageHandler
{
public:
    virtual ~UnsolicitedMessageHandler() {}

    /**
     * @brief
     *   This function handles an unsolicited CHIP message.
     *
     *   If the implementation returns CHIP_NO_ERROR, it is expected to set newDelegate to the delegate to use for the exchange
     *   handling the message.  The message layer will handle creating the exchange with this delegate.
     *
     *   If the implementation returns an error, message processing will be aborted for this message.
     *
     *  @param[in]  payloadHeader A reference to the PayloadHeader object for the unsolicited message.  The protocol and message
     *                            type of this header match the UnsolicitedMessageHandler.
     *  @param[out] newDelegate   A new exchange delegate to be used by the new exchange created to handle the message.
     */
    virtual CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate) = 0;

    /**
     * @brief
     *   This function is called when OnUnsolicitedMessageReceived successfully returns a new delegate, but the session manager
     *   fails to assign the delegate to a new exchange.  It can be used to free the delegate as needed.
     *
     *   Once an exchange is created with the delegate, the OnExchangeClosing notification can be used to free the delegate as
     *   needed.
     *
     *  @param[in] delegate   The exchange delegate to be released.
     */
    virtual void OnExchangeCreationFailed(ExchangeDelegate * delegate) {}
};

} // namespace Messaging
} // namespace chip
