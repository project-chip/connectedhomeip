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
 *      This file defines ExchangeTransport class. The object of this
 *      class can be used by CHIP protocols to send and receive messages.
 */

#pragma once

#include <transport/SecureSessionMgr.h>

namespace chip {
namespace Messaging {

class ReliableMessageMgr;
class ReliableMessageContext;

class ExchangeTransport
{
public:
    struct ExchangeInfo
    {
        bool mInitiator;
        uint16_t mExchangeId;
    };

    struct MessageReliabilityInfo
    {
        uint32_t mMessageId;
        bool mHasAck;
        uint32_t mAckId;
        bool mNeedsAck;
    };

    ExchangeTransport() {}
    virtual ~ExchangeTransport() {}

    CHIP_ERROR Init(ReliableMessageMgr * rmMgr)
    {
        mReliableMessageMgr = rmMgr;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR SendMessage(SecureSessionHandle session, ExchangeTransport::ExchangeInfo & exchangeInfo,
                           ReliableMessageContext & rmCtxt, bool isReliableTransmission, Protocols::Id protocol, uint8_t type,
                           System::PacketBufferHandle message);

    virtual CHIP_ERROR ResendMessage(SecureSessionHandle session, EncryptedPacketBufferHandle message,
                                     EncryptedPacketBufferHandle * retainedMessage) const
    {
        return CHIP_ERROR_NOT_IMPLEMENTED;
    }

    CHIP_ERROR OnMessageReceived(uint16_t protocol, uint8_t type, ReliableMessageContext & rmCtxt, MessageReliabilityInfo & rmInfo);

protected:
    virtual bool MessagePermitted(uint16_t protocol, uint8_t type) = 0;

    virtual CHIP_ERROR SendMessageImpl(SecureSessionHandle session, PayloadHeader & payloadHeader,
                                       System::PacketBufferHandle && message, EncryptedPacketBufferHandle * retainedMessage) = 0;

    virtual bool IsTransportReliable() { return false; }

private:
    ReliableMessageMgr * mReliableMessageMgr = nullptr;
};

} // namespace Messaging
} // namespace chip
