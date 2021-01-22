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
 *    limitations under the License.
 */

#pragma once

#include <app/server/AppDelegate.h>
#include <platform/CHIPDeviceLayer.h>
#include <transport/RendezvousSession.h>

namespace chip {

class RendezvousServer : public RendezvousSessionDelegate
{
public:
    RendezvousServer();

    CHIP_ERROR Init(const RendezvousParameters & params, TransportMgrBase * transportMgr, SecureSessionMgr * sessionMgr);
    void SetDelegate(AppDelegate * delegate) { mDelegate = delegate; };

    //////////////// RendezvousSessionDelegate Implementation ///////////////////

    void OnRendezvousConnectionOpened() override;
    void OnRendezvousConnectionClosed() override;
    void OnRendezvousError(CHIP_ERROR err) override;
    void OnRendezvousMessageReceived(const PacketHeader & packetHeader, const Transport::PeerAddress & peerAddress,
                                     System::PacketBufferHandle buffer) override;
    void OnRendezvousComplete() override;
    void OnRendezvousStatusUpdate(Status status, CHIP_ERROR err) override;
    RendezvousSession * GetRendezvousSession() { return &mRendezvousSession; };

private:
    RendezvousSession mRendezvousSession;
    AppDelegate * mDelegate;
};

} // namespace chip
