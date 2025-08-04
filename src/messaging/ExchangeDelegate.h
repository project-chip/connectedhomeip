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
 *
 *   For consumers who use an ExchangeContext to send/receive protocol messages, there are specific
 *   expectations around who manages the exchange w.r.t clean-up and destruction:
 *     1. When you allocate an exchange, you own the exchange. Until you send a message successfully, it's on you
 *        to release that ownership by calling Close or Abort on the exchange.
 *
 *     2. If you send a message successfully that doesn't require a response, the ownership transfers to
 *        the ExchangeMgr, and it will close the exchange for you automatically.
 *
 *     3. If you send a message successfully that does require a response and desire to close it before
 *        you get any notifications on that exchange from the ExchangeMgr, you should call Close or Abort on that exchange.
 *
 *     4. On reception of a message on an exchange, the ownership transfers to the OnMessageReceived callee.
 *        If you return from OnMessageReceived and no messages were sent on that exchange, the exchange will transfer back
 *        to the ExchangeMgr and it will automatically close it.
 *
 *     5. If you call WillSendMessage on the exchange in OnMessageReceived indicating a desire to send a message later
 *        on the exchange, then the exchange remains with you, and it's your responsibility to either send a message on it,
 *        or Close/Abort if you no longer wish to have the exchange around.
 *
 *     6. If you get a call to OnExchangeClosing, you should null out your reference to the exchange UNLESS you still
 *        hold ownership of the exchange (i.e due to a prior call to WillSendMessage). In that case, you should call Abort/Close
 *        whenever you're done with using the exchange. Those calls can be made synchronously within the OnExchangeClosing
 *        callback.
 *
 *     7. If you get a call to OnResponseTimeout, you should null out your reference to the exchange since the exchange layer
 *        owns the exchange and will handle releasing the ref later. A call to OnExchangeClosing will follow after.
 *
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
     *   exchange context is being closed.
     *
     *   If the exchange was in a state where it was expecting a message to be
     *   sent due to an earlier WillSendMessage call or because the exchange has
     *   just been created as an initiator, the consumer is holding a reference
     *   to the exchange and it's the consumer's responsibility to call
     *   Release() on the exchange at some point.  The usual way this happens is
     *   that the consumer tries to send its message, that fails, and the
     *   consumer calls Close() on the exchange.  Calling Close() after an
     *   OnExchangeClosing() notification is allowed in this situation.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     */
    virtual void OnExchangeClosing(ExchangeContext * ec) {}

    virtual ExchangeMessageDispatch & GetMessageDispatch() { return ApplicationExchangeDispatch::Instance(); }
};

/**
 * @brief
 *   This class handles unsolicited messages. The implementation can select an exchange delegate to use based on the payload header
 * of the incoming message or its session.
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
     *  @param[in]  session       A reference to the session where unsolicited message was received.
     *  @param[out] newDelegate   A new exchange delegate to be used by the new exchange created to handle the message.
     */
    virtual CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, const SessionHandle & session,
                                                    ExchangeDelegate *& newDelegate)
    {
        return OnUnsolicitedMessageReceived(payloadHeader, newDelegate);
    }

    virtual CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, ExchangeDelegate *& newDelegate)
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

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
