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
    mSecureSessionMgr = nullptr;
    mTransport        = nullptr;
}

void TransportMgrBase::HandleMessageReceived(const Transport::PeerAddress & peerAddress, System::PacketBufferHandle && msg)
{
    if (mSecureSessionMgr != nullptr)
    {
        mSecureSessionMgr->OnMessageReceived(peerAddress, std::move(msg));
    }
    else
    {
        char addrBuffer[Transport::PeerAddress::kMaxToStringSize];
        peerAddress.ToString(addrBuffer);
        ChipLogError(Inet, "message from %s is dropped since no corresponding handler is set in TransportMgr.", addrBuffer);
    }
}

} // namespace chip
