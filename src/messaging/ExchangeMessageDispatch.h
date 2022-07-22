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

#include <messaging/Flags.h>
#include <protocols/Protocols.h>
#include <transport/SessionManager.h>

namespace chip {
namespace Messaging {

class ReliableMessageContext;

class ExchangeMessageDispatch
{
public:
    ExchangeMessageDispatch() {}
    virtual ~ExchangeMessageDispatch() {}

    virtual bool IsEncryptionRequired() const { return true; }

    CHIP_ERROR SendMessage(SessionManager * sessionManager, const SessionHandle & session, uint16_t exchangeId, bool isInitiator,
                           ReliableMessageContext * reliableMessageContext, bool isReliableTransmission, Protocols::Id protocol,
                           uint8_t type, System::PacketBufferHandle && message);

    virtual bool MessagePermitted(Protocols::Id protocol, uint8_t type) = 0;

    // TODO: remove IsReliableTransmissionAllowed, this function should be provided over session.
    virtual bool IsReliableTransmissionAllowed() const { return true; }
};

} // namespace Messaging
} // namespace chip
