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

#pragma once

#include <lib/support/CodeUtils.h>
#include <system/SystemPacketBuffer.h>
#include <transport/raw/Base.h>
#include <transport/raw/MessageHeader.h>
#include <transport/raw/PeerAddress.h>

namespace chip {

namespace Transport {
class Base;
}

class TransportMgrDelegate;

class TransportMgrBase : public Transport::RawTransportDelegate
{
public:
    CHIP_ERROR Init(Transport::Base * transport);

    CHIP_ERROR SendMessage(const Transport::PeerAddress & address, System::PacketBufferHandle && msgBuf);

    void Close();

    void Disconnect(const Transport::PeerAddress & address);

    void SetSessionManager(TransportMgrDelegate * sessionManager) { mSessionManager = sessionManager; }

    CHIP_ERROR MulticastGroupJoinLeave(const Transport::PeerAddress & address, bool join);

    void HandleMessageReceived(const Transport::PeerAddress & peerAddress, System::PacketBufferHandle && msg) override;

private:
    TransportMgrDelegate * mSessionManager = nullptr;
    Transport::Base * mTransport           = nullptr;
};

} // namespace chip
