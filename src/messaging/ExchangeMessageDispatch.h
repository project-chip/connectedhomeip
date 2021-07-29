/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *      This file defines ExchangeMessageDispatch class. The object of this
 *      class can be used by CHIP protocols to send and receive messages.
 */

#pragma once

#include <lib/core/ReferenceCounted.h>
#include <messaging/Flags.h>
#include <transport/SecureSessionMgr.h>

namespace chip {
namespace Messaging {

class ReliableMessageMgr;
class ReliableMessageContext;

class ExchangeMessageDispatch : public ReferenceCounted<ExchangeMessageDispatch>
{
public:
    ExchangeMessageDispatch() {}
    virtual ~ExchangeMessageDispatch() {}

    CHIP_ERROR Init() { return CHIP_NO_ERROR; }

    CHIP_ERROR SendMessage(SecureSessionHandle session, uint16_t exchangeId, bool isInitiator,
                           ReliableMessageContext * reliableMessageContext, bool isReliableTransmission, Protocols::Id protocol,
                           uint8_t type, System::PacketBufferHandle && message);

    /**
     * @brief
     *   This interface takes the payload and returns the prepared message which can be send multiple times.
     *
     * @param session         Peer node to which the payload to be sent
     * @param payloadHeader   The payloadHeader to be encoded into the packet
     * @param message         The payload to be sent
     * @param preparedMessage The handle to hold the prepared message
     */
    virtual CHIP_ERROR PrepareMessage(SecureSessionHandle session, PayloadHeader & payloadHeader,
                                      System::PacketBufferHandle && message, EncryptedPacketBufferHandle & preparedMessage) = 0;
    virtual CHIP_ERROR SendPreparedMessage(SecureSessionHandle session,
                                           const EncryptedPacketBufferHandle & preparedMessage) const                       = 0;

    virtual CHIP_ERROR OnMessageReceived(const PayloadHeader & payloadHeader, uint32_t messageId,
                                         const Transport::PeerAddress & peerAddress, MessageFlags msgFlags,
                                         ReliableMessageContext * reliableMessageContext);

protected:
    virtual bool MessagePermitted(uint16_t protocol, uint8_t type) = 0;
    virtual bool IsReliableTransmissionAllowed() const { return true; }
};

} // namespace Messaging
} // namespace chip
