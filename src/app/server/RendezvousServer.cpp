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

#include <app/server/RendezvousServer.h>

#include <app/server/SessionManager.h>
#include <core/CHIPError.h>
#include <support/CodeUtils.h>
#include <transport/SecureSessionMgr.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/ThreadStackManager.h>
#endif
#include <mdns/Advertiser.h>

using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;

namespace chip {

RendezvousServer::RendezvousServer() : mRendezvousSession(this) {}

CHIP_ERROR RendezvousServer::Init(const RendezvousParameters & params, TransportMgrBase * transportMgr,
                                  SecureSessionMgr * sessionMgr)
{
    return mRendezvousSession.Init(params, transportMgr, sessionMgr);
}

void RendezvousServer::OnRendezvousError(CHIP_ERROR err)
{
    ChipLogProgress(AppServer, "OnRendezvousError: %s", ErrorStr(err));
}

void RendezvousServer::OnRendezvousConnectionOpened()
{
    ChipLogProgress(AppServer, "OnRendezvousConnectionOpened");
}

void RendezvousServer::OnRendezvousConnectionClosed()
{
    ChipLogProgress(AppServer, "OnRendezvousConnectionClosed");
}

void RendezvousServer::OnRendezvousMessageReceived(const PacketHeader & packetHeader, const PeerAddress & peerAddress,
                                                   System::PacketBufferHandle buffer)
{}

void RendezvousServer::OnRendezvousComplete()
{
    ChipLogProgress(AppServer, "Device completed Rendezvous process");
}

void RendezvousServer::OnRendezvousStatusUpdate(Status status, CHIP_ERROR err)
{
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(AppServer, "OnRendezvousStatusUpdate: %s", chip::ErrorStr(err)));

    switch (status)
    {
    case RendezvousSessionDelegate::SecurePairingSuccess:
        ChipLogProgress(AppServer, "Device completed SPAKE2+ handshake");
        if (mDelegate != nullptr)
        {
            mDelegate->OnRendezvousStarted();
        }
        break;

    case RendezvousSessionDelegate::SecurePairingFailed:
        ChipLogProgress(AppServer, "Failed in SPAKE2+ handshake");
        if (mDelegate != nullptr)
        {
            mDelegate->OnRendezvousStopped();
        }
        break;

    case RendezvousSessionDelegate::NetworkProvisioningSuccess:
        ChipLogProgress(AppServer, "Device was assigned network credentials");
        if (chip::Mdns::ServiceAdvertiser::Instance().Start(&DeviceLayer::InetLayer, chip::Mdns::kMdnsPort) != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to start mDNS advertisement");
        }
        if (mDelegate != nullptr)
        {
            mDelegate->OnRendezvousStopped();
        }
        break;

    case RendezvousSessionDelegate::NetworkProvisioningFailed:
        ChipLogProgress(AppServer, "Failed in network provisioning");
        if (mDelegate != nullptr)
        {
            mDelegate->OnRendezvousStopped();
        }
        break;

    default:
        break;
    };

exit:
    return;
}
} // namespace chip
