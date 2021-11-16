/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#include <transport/TransportMgrBase.h>

#include <lib/support/CodeUtils.h>
#include <transport/TransportMgr.h>
#include <transport/raw/Base.h>

namespace chip {

CHIP_ERROR TransportMgrBase::SendMessage(const Transport::PeerAddress & peer, const Transport::PeerAddress & local,
                                         System::PacketBufferHandle && message)
{
    return mTransport->SendMessage(peer, local, std::move(message));
}

void TransportMgrBase::Disconnect(const Transport::PeerAddress & address)
{
    mTransport->Disconnect(address);
}

CHIP_ERROR TransportMgrBase::Init(Transport::Base * transport)
{
    if (mTransport != nullptr)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }
    mTransport = transport;
    mTransport->SetDelegate(this);
    ChipLogDetail(Inet, "TransportMgr initialized");
    return CHIP_NO_ERROR;
}

void TransportMgrBase::Close()
{
    mSessionManager = nullptr;
    mTransport      = nullptr;
}

void TransportMgrBase::HandleMessageReceived(const Transport::PeerAddress & peer, const Transport::PeerAddress & local,
                                             System::PacketBufferHandle && message)
{
    if (message->HasChainedBuffer())
    {
        // Something in the lower levels messed up.
        char peerAddrBuffer[Transport::PeerAddress::kMaxToStringSize];
        char localAddrBuffer[Transport::PeerAddress::kMaxToStringSize];
        peer.ToString(peerAddrBuffer);
        local.ToString(localAddrBuffer);
        ChipLogError(Inet, "message %s -> %s dropped due to lower layers not ensuring a single packet buffer.", peerAddrBuffer,
                     localAddrBuffer);
        return;
    }

    if (mSessionManager != nullptr)
    {

#if CHIP_DETAIL_LOGGING
        char peerAddrBuffer[Transport::PeerAddress::kMaxToStringSize];
        char localAddrBuffer[Transport::PeerAddress::kMaxToStringSize];
        peer.ToString(peerAddrBuffer);
        local.ToString(localAddrBuffer);
        ChipLogProgress(Inet, "message %s -> %s received.", peerAddrBuffer, localAddrBuffer);
#endif // CHIP_DETAIL_LOGGING
        mSessionManager->OnMessageReceived(peer, local, std::move(message));
    }
    else
    {
        char peerAddrBuffer[Transport::PeerAddress::kMaxToStringSize];
        char localAddrBuffer[Transport::PeerAddress::kMaxToStringSize];
        peer.ToString(peerAddrBuffer);
        local.ToString(localAddrBuffer);
        ChipLogError(Inet, "message from %s -> %s is dropped since no corresponding handler is set in TransportMgr.",
                     peerAddrBuffer, localAddrBuffer);
    }
}

} // namespace chip
