/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
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
