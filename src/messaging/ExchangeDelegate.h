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
 *   This class handles unsolicited messages. The implementation can peek the message and creating a new exchange for it or not.
 */
class DLL_EXPORT ExchangeAcceptor
{
public:
    virtual ~ExchangeAcceptor() {}

    /**
     * @brief
     *   This function handles a unsolicited CHIP message.
     *
     *   If the implmentation returns CHIP_NO_ERROR, the message layer will create a new exchange with newDelegate, associated to
     * the message, and using the new exchange to handle future messages.
     *
     *   The acceptor peek the message here, and the message payload will be sent to the new exchange regardless.
     *
     *  @param[in]  payloadHeader A reference to the PayloadHeader object.
     *  @param[in]  payload       A handle to the PacketBuffer object holding the message payload.
     *  @param[out] newDelegate   A new exchange delegate being used by the new exchange to handle future messages.
     */
    virtual CHIP_ERROR OnUnsolicitedMessageReceived(const PayloadHeader & payloadHeader, System::PacketBufferHandle & payload,
                                                    ExchangeDelegate *& newDelegate) = 0;
};

} // namespace Messaging
} // namespace chip
