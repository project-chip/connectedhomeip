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

#include "Server.h"

#include "DataModelHandler.h"
#include "RendezvousServer.h"
#include "SessionManager.h"

#include <ble/BLEEndPoint.h>
#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <lib/mdns/DiscoveryManager.h>
#include <platform/CHIPDeviceLayer.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>
#include <transport/raw/UDP.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;

namespace {

class ServerCallback : public SecureSessionMgrDelegate
{
public:
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader,
                           const Transport::PeerConnectionState * state, System::PacketBufferHandle buffer,
                           SecureSessionMgr * mgr) override
    {
        const size_t data_len = buffer->DataLength();
        char src_addr[PeerAddress::kMaxToStringSize];

        // as soon as a client connects, assume it is connected
        VerifyOrExit(!buffer.IsNull(), ChipLogProgress(AppServer, "Received data but couldn't process it..."));
        VerifyOrExit(header.GetSourceNodeId().HasValue(), ChipLogProgress(AppServer, "Unknown source for received message"));

        VerifyOrExit(state->GetPeerNodeId() != kUndefinedNodeId, ChipLogProgress(AppServer, "Unknown source for received message"));

        state->GetPeerAddress().ToString(src_addr, sizeof(src_addr));

        ChipLogProgress(AppServer, "Packet received from %s: %zu bytes", src_addr, static_cast<size_t>(data_len));

        HandleDataModelMessage(header, std::move(buffer), mgr);

    exit:;
    }

    void OnNewConnection(const Transport::PeerConnectionState * state, SecureSessionMgr * mgr) override
    {
        ChipLogProgress(AppServer, "Received a new connection.");
    }
};

DemoTransportMgr gTransports;
SecureSessionMgr gSessions;
ServerCallback gCallbacks;
SecurePairingUsingTestSecret gTestPairing;
RendezvousServer gRendezvousServer;

} // namespace

SecureSessionMgr & chip::SessionManager()
{
    return gSessions;
}

// The function will initialize datamodel handler and then start the server
// The server assumes the platform's networking has been setup already
void InitServer()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);

    InitDataModelHandler();

    // Init transport before operations with secure session mgr.
    err = gTransports.Init(UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6));
    SuccessOrExit(err);

    err = gSessions.Init(chip::kTestDeviceNodeId, &DeviceLayer::SystemLayer, &gTransports);
    SuccessOrExit(err);

    // This flag is used to bypass BLE in the cirque test
    // Only in the cirque test this is enabled with --args='bypass_rendezvous=true'
#ifndef CHIP_BYPASS_RENDEZVOUS
    {
        RendezvousParameters params;
        uint32_t pinCode;

        SuccessOrExit(err = DeviceLayer::ConfigurationMgr().GetSetupPinCode(pinCode));
        params.SetSetupPINCode(pinCode)
            .SetLocalNodeId(chip::kTestDeviceNodeId)
            .SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer())
            .SetPeerAddress(Transport::PeerAddress::BLE());
        SuccessOrExit(err = gRendezvousServer.Init(params, &gTransports));
    }
#endif

    err = gSessions.NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing);
    SuccessOrExit(err);

    gSessions.SetDelegate(&gCallbacks);
    chip::Mdns::DiscoveryManager::GetInstance().StartPublishDevice(chip::Inet::kIPAddressType_IPv6);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "ERROR setting up transport: %s", ErrorStr(err));
    }
    else
    {
        ChipLogProgress(AppServer, "Server Listening...");
    }
}
