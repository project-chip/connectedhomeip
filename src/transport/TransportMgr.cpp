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

#include <transport/TransportMgr.h>

#include <transport/RendezvousSession.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/Base.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

void TransportMgrBase::HandleMessageReceived(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                             System::PacketBuffer * msg, TransportMgrBase * dispatcher)
{
    if (packetHeader.GetFlags().Has(Header::FlagValues::kSecure))
    {
        if (dispatcher->mSecureSessionMgr != nullptr)
        {
            SecureSessionMgr::HandleSecureMessageReceived(packetHeader, peerAddress, msg, dispatcher->mSecureSessionMgr);
        }
    }
    else
    {
        if (dispatcher->mRendezvous != nullptr)
        {
            dispatcher->mRendezvous->OnRendezvousMessageReceived(packetHeader, peerAddress, msg);
            // Unsecure message received, pass it to Rendezvous Session Mgr directly.
        }
    }
}
} // namespace chip
