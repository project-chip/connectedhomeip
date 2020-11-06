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
 *      This file defines the classes corresponding to CHIP Exchange Context.
 *
 */

#pragma once

#include <system/SystemPacketBuffer.h>
#include <transport/raw/MessageHeader.h>

namespace chip {

class ExchangeContext;

/**
 * @brief
 *   This class provides a skeleton for the callback functions. The functions will be
 *   called by ExchangeContext object on specific events. If the user of ExchangeContext
 *   is interested in receiving these callbacks, they can specialize this class and handle
 *   each trigger in their implementation of this class.
 */
class DLL_EXPORT ExchangeContextDelegate
{
public:
    virtual ~ExchangeContextDelegate() {}

    /**
     * @brief
     *   This function is the protocol callback for handling a received CHIP message.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     *  @param[in]    packetHeader  A reference to the PacketHeader object.
     *  @param[in]    protocolId    The protocol identifier of the received message.
     *  @param[in]    msgType       The message type of the corresponding protocol.
     *  @param[in]    payload       A pointer to the PacketBuffer object holding the message payload.
     */
    virtual void OnMessageReceived(ExchangeContext * ec, const PacketHeader & packetHeader, uint32_t protocolId, uint8_t msgType,
                                   System::PacketBuffer * payload) = 0;

    /**
     * @brief
     *   This function is the protocol callback to invoke when the timeout for the receipt
     *   of a response message has expired.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     */
    virtual void OnResponseTimeout(ExchangeContext * ec) = 0;
};

/**
 * @brief
 *   This class is the interface registered into UnsolicitedMessageHandler.
 */
class DLL_EXPORT ExchangeContextDelegateFactory
{
public:
    virtual ~ExchangeContextDelegateFactory() {}

    /**
     * @brief
     *   This function is the protocol callback for UnsolicitedMessageHandler
     *   to create a context to handle following incoming messages. The first
     *   initiator message will trigger a creation of the Delegate. All
     *   messages (both first and following messages) will be passed to the
     *   created delegate.
     *
     *  @param[in]    ec            A pointer to the ExchangeContext object.
     */
    virtual ExchangeContextDelegate * CreateDelegate() = 0;
};

/**
 * @brief
 *   This is a helper class for protocols where that its Delegate and
 *   DelegateFactory are the same object. In that case, derive from this object
 *   and implementation the pure virtual function OnMessageReceived and
 *   OnResponseTimeout. Then register the derived class into
 *   UnsolicitedMessageHandler
 */
class DLL_EXPORT SimpleExchangeContextDelegate : public ExchangeContextDelegateFactory, public ExchangeContextDelegate
{
public:
    virtual ~SimpleExchangeContextDelegate() override {}
    virtual ExchangeContextDelegate * CreateDelegate() override { return this; }
};

} // namespace chip
