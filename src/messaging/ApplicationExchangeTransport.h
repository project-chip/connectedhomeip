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
 *      This file defines Application Channel class. The object of this
 *      class can be used by CHIP data model cluster applications to send
 *      and receive messages. The messages are encrypted using session keys.
 */

#pragma once

#include <messaging/ExchangeTransport.h>
#include <support/CodeUtils.h>
#include <transport/SecureSessionMgr.h>

namespace chip {
namespace Messaging {

class ApplicationExchangeTransport : public ExchangeTransport
{
public:
    ApplicationExchangeTransport() {}

    virtual ~ApplicationExchangeTransport() {}

    CHIP_ERROR Init(ReliableMessageMgr * reliableMessageMgr, SecureSessionMgr * sessionMgr)
    {
        ReturnErrorCodeIf(sessionMgr == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mSessionMgr = sessionMgr;
        return ExchangeTransport::Init(reliableMessageMgr);
    }

    CHIP_ERROR ResendMessage(SecureSessionHandle session, EncryptedPacketBufferHandle message,
                             EncryptedPacketBufferHandle * retainedMessage) const override;

protected:
    CHIP_ERROR SendMessageImpl(SecureSessionHandle session, PayloadHeader & payloadHeader, System::PacketBufferHandle && message,
                               EncryptedPacketBufferHandle * retainedMessage) override;

    bool MessagePermitted(uint16_t protocol, uint8_t type) override;

private:
    SecureSessionMgr * mSessionMgr = nullptr;
};

} // namespace Messaging
} // namespace chip
