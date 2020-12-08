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

#include <app/server/Server.h>

#include <app/server/DataModelHandler.h>
#include <app/server/RendezvousServer.h>
#include <app/server/SessionManager.h>

#include <ble/BLEEndPoint.h>
#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <lib/mdns/DiscoveryManager.h>
#include <mdns/Advertiser.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/SetupPayload.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <transport/SecureSessionMgr.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;

namespace {

bool isRendezvousBypassed()
{
    RendezvousInformationFlags rendezvousMode = RendezvousInformationFlags::kBLE;

#ifdef CONFIG_RENDEZVOUS_MODE
    rendezvousMode = static_cast<RendezvousInformationFlags>(CONFIG_RENDEZVOUS_MODE);
#endif

#ifdef CHIP_BYPASS_RENDEZVOUS
    rendezvousMode = RendezvousInformationFlags::kNone;
#endif

    return rendezvousMode == RendezvousInformationFlags::kNone;
}

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

    void OnReceiveError(CHIP_ERROR error, const Transport::PeerAddress & source, SecureSessionMgr * mgr) override
    {
        ChipLogProgress(AppServer, "Packet received error: %s", ErrorStr(error));
        if (mDelegate != nullptr)
        {
            mDelegate->OnReceiveError();
        }
    }

    void OnNewConnection(const Transport::PeerConnectionState * state, SecureSessionMgr * mgr) override
    {
        ChipLogProgress(AppServer, "Received a new connection.");
    }

    void SetDelegate(AppDelegate * delegate) { mDelegate = delegate; }

private:
    AppDelegate * mDelegate = nullptr;
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
void InitServer(AppDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    Optional<Transport::PeerAddress> peer(Transport::Type::kUndefined);

    InitDataModelHandler();
    gCallbacks.SetDelegate(delegate);
    gRendezvousServer.SetDelegate(delegate);

    // Init transport before operations with secure session mgr.
#if INET_CONFIG_ENABLE_IPV4
    err = gTransports.Init(UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6),
                           UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv4));
#else
    err = gTransports.Init(UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6));
#endif
    SuccessOrExit(err);

    err = gSessions.Init(chip::kTestDeviceNodeId, &DeviceLayer::SystemLayer, &gTransports);
    SuccessOrExit(err);

    // This flag is used to bypass BLE in the cirque test
    // Only in the cirque test this is enabled with --args='bypass_rendezvous=true'
    if (isRendezvousBypassed())
    {
        ChipLogProgress(AppServer, "Rendezvous and Secure Pairing skipped. Using test secret.");
        err = gSessions.NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing);
        SuccessOrExit(err);
    }
    else
    {
        RendezvousParameters params;
        uint32_t pinCode;

        SuccessOrExit(err = DeviceLayer::ConfigurationMgr().GetSetupPinCode(pinCode));
        params.SetSetupPINCode(pinCode)
            .SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer())
            .SetPeerAddress(Transport::PeerAddress::BLE());
        SuccessOrExit(err = gRendezvousServer.Init(params, &gTransports));
    }

#if CHIP_ENABLE_MDNS
    // TODO: advertise this only when really operational once we support both
    // operational and commisioning advertising is supported.
    {
        constexpr uint64_t kTestFabricId = 5544332211;
        err                              = Mdns::ServiceAdvertiser::Instance().Advertise(Mdns::OperationalAdvertisingParameters()
                                                                .SetFabricId(kTestFabricId)
                                                                .SetNodeId(chip::kTestDeviceNodeId)
                                                                .SetPort(CHIP_PORT)
#if INET_CONFIG_ENABLE_IPV4
                                                                .EnableIpV4(true)
#else
                                                                .EnableIpV4(false)
#endif
        );
        SuccessOrExit(err);
    }

    err = Mdns::ServiceAdvertiser::Instance().Start(&DeviceLayer::InetLayer, chip::Mdns::kMdnsPort);
    SuccessOrExit(err);
#endif

    err = gSessions.NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing);
    SuccessOrExit(err);

    gSessions.SetDelegate(&gCallbacks);

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
