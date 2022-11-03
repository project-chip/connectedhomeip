/*
 * SPDX-FileCopyrightText: (c) 2020-2021 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <transport/TransportMgrBase.h>

#include <lib/support/CodeUtils.h>
#include <platform/LockTracker.h>
#include <transport/TransportMgr.h>
#include <transport/raw/Base.h>

namespace chip {

CHIP_ERROR TransportMgrBase::SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf)
{
    return mTransport->SendMessage(address, std::move(msgBuf));
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

CHIP_ERROR TransportMgrBase::MulticastGroupJoinLeave(const Transport::PeerAddress & address, bool join)
{
    return mTransport->MulticastGroupJoinLeave(address, join);
}

void TransportMgrBase::HandleMessageReceived(const Transport::PeerAddress & peerAddress, System::PacketBufferHandle && msg)
{
    // This is the first point all incoming messages funnel through.  Ensure
    // that our message receipts are all synchronized correctly.
    assertChipStackLockedByCurrentThread();

    if (msg->HasChainedBuffer())
    {
        // Something in the lower levels messed up.
        char addrBuffer[Transport::PeerAddress::kMaxToStringSize];
        peerAddress.ToString(addrBuffer);
        ChipLogError(Inet, "message from %s dropped due to lower layers not ensuring a single packet buffer.", addrBuffer);
        return;
    }

    if (mSessionManager != nullptr)
    {
        mSessionManager->OnMessageReceived(peerAddress, std::move(msg));
    }
    else
    {
        char addrBuffer[Transport::PeerAddress::kMaxToStringSize];
        peerAddress.ToString(addrBuffer);
        ChipLogError(Inet, "message from %s is dropped since no corresponding handler is set in TransportMgr.", addrBuffer);
    }
}

} // namespace chip
