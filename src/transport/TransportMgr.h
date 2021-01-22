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
 */

/**
 * @file
 *   This file implements a stateless TransportMgr, it will took a raw message
 * buffer from transports, and then extract the message header without decode it.
 * For secure messages, it will pass it to the SecureSessionMgr, and for unsecure
 * messages (rendezvous messages), it will pass it to RendezvousSession.
 *   When sending messages, it will encode the packet header, and pass it to the
 * transports.
 *   The whole process is fully stateless.
 */

#pragma once

#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <transport/raw/Base.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>
#include <transport/raw/Tuple.h>

namespace chip {

class TransportMgrBase;

class TransportMgrDelegate
{
public:
    virtual ~TransportMgrDelegate() = default;
    /**
     * @brief
     *   Handle received secure message.
     *
     * @param header    the received message header
     * @param source    the source address of the package
     * @param msgBuf    the buffer of (encrypted) payload
     */
    virtual void OnMessageReceived(const PacketHeader & header, const Transport::PeerAddress & source,
                                   System::PacketBufferHandle msgBuf) = 0;
};

class TransportMgrBase
{
public:
    CHIP_ERROR Init(Transport::Base * transport);

    CHIP_ERROR SendMessage(const PacketHeader & header, const Transport::PeerAddress & address,
                           System::PacketBufferHandle && msgBuf)
    {
        return mTransport->SendMessage(header, address, std::move(msgBuf));
    }

    void Disconnect(const Transport::PeerAddress & address) { mTransport->Disconnect(address); }

    void SetSecureSessionMgr(TransportMgrDelegate * secureSessionMgr) { mSecureSessionMgr = secureSessionMgr; }

    void SetRendezvousSession(TransportMgrDelegate * rendezvousSessionMgr) { mRendezvous = rendezvousSessionMgr; }

private:
    static void HandleMessageReceived(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                      System::PacketBufferHandle msg, TransportMgrBase * dispatcher);

    TransportMgrDelegate * mSecureSessionMgr = nullptr;
    TransportMgrDelegate * mRendezvous       = nullptr;
    Transport::Base * mTransport             = nullptr;
};

template <typename... TransportTypes>
class TransportMgr : public TransportMgrBase
{
public:
    template <typename... Args>
    CHIP_ERROR Init(Args &&... transportInitArgs)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        err = mTransport.Init(std::forward<Args>(transportInitArgs)...);
        SuccessOrExit(err);
        err = TransportMgrBase::Init(&mTransport);
    exit:
        return err;
    }

    template <typename... Args>
    CHIP_ERROR ResetTransport(Args &&... transportInitArgs)
    {
        return mTransport.Init(std::forward<Args>(transportInitArgs)...);
    }

private:
    Transport::Tuple<TransportTypes...> mTransport;
};

} // namespace chip
