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

#include <messaging/ExchangeMessageDispatch.h>
#include <transport/TransportMgr.h>

namespace chip {

class SessionEstablishmentExchangeDispatch : public Messaging::ExchangeMessageDispatch
{
public:
    SessionEstablishmentExchangeDispatch() {}

    virtual ~SessionEstablishmentExchangeDispatch() {}

    CHIP_ERROR Init(TransportMgrBase * transportMgr)
    {
        ReturnErrorCodeIf(transportMgr == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
        mTransportMgr = transportMgr;
        return CHIP_NO_ERROR;
    }

    CHIP_ERROR OnMessageReceived(const PayloadHeader & payloadHeader, uint32_t messageId,
                                 const Transport::PeerAddress & peerAddress,
                                 Messaging::ReliableMessageContext * reliableMessageContext) override;

    const Transport::PeerAddress & GetPeerAddress() const { return mPeerAddress; }

    void SetPeerAddress(const Transport::PeerAddress & address) { mPeerAddress = address; }

protected:
    CHIP_ERROR SendMessageImpl(SecureSessionHandle session, PayloadHeader & payloadHeader, System::PacketBufferHandle && message,
                               EncryptedPacketBufferHandle * retainedMessage) override;

    bool MessagePermitted(uint16_t protocol, uint8_t type) override;

    bool IsTransportReliable() override
    {
        // If we are not using BLE as the transport, the underlying transport is UDP based.
        // (return true only if BLE is being used as the transport)
        return (mTransportMgr == nullptr);
    }

private:
    TransportMgrBase * mTransportMgr = nullptr;
    Transport::PeerAddress mPeerAddress;
};

} // namespace chip
