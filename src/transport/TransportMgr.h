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

#include <support/logging/CHIPLogging.h>
#include <transport/RendezvousSession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/Base.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

class TransportMgrBase
{
public:
    CHIP_ERROR SendMessage(const PacketHeader & header, Header::Flags payloadFlags, const Transport::PeerAddress & address,
                           System::PacketBuffer * msgBuf)
    {
        return mTransport->SendMessage(header, payloadFlags, address, msgBuf);
    }

    void Disconnect(const Transport::PeerAddress & address) { mTransport->Disconnect(address); }

    void SetSecureSessionMgr(SecureSessionMgr * secureSessionMgr)
    {
        mSecureSessionMgr = secureSessionMgr;
        if (mSecureSessionMgr != nullptr)
        {
            mSecureSessionMgr->SetTransportMgr(this);
        }
    }

    void SetRendezvousSession(RendezvousSession * rendezvous)
    {
        mRendezvous = rendezvous;
        if (mRendezvous != nullptr)
        {
            mRendezvous->SetTransportMgr(this);
        }
    }

protected:
    void InitInternal(SecureSessionMgr * secureMgr, RendezvousSession * rendezvous, Transport::Base * transport)
    {
        mTransport        = transport;
        mTransport->SetMessageReceiveHandler(HandleMessageReceived, this);
        SetSecureSessionMgr(secureMgr);
        SetRendezvousSession(rendezvous);
        ChipLogDetail(Inet, "TransportMgr initialized");
    }

private:
    static void HandleMessageReceived(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                      System::PacketBuffer * msg, TransportMgrBase * dispatcher);

    SecureSessionMgr * mSecureSessionMgr = nullptr;
    RendezvousSession * mRendezvous      = nullptr;
    Transport::Base * mTransport         = nullptr;
};

template <typename... TransportTypes>
class TransportMgr : public TransportMgrBase
{
public:
    template <typename... Args>
    CHIP_ERROR Init(SecureSessionMgr * secureMgr, RendezvousSession * rendezvous, Args &&... transportInitArgs)
    {
        CHIP_ERROR err = CHIP_NO_ERROR;

        err = mTransport.Init(std::forward<Args>(transportInitArgs)...);
        SuccessOrExit(err);
        InitInternal(secureMgr, rendezvous, &mTransport);
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
