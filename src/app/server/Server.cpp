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
#include <mdns/Advertiser.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/SetupPayload.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/ReturnMacros.h>
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
    void OnMessageReceived(const PacketHeader & header, const PayloadHeader & payloadHeader, SecureSessionHandle session,
                           System::PacketBufferHandle buffer, SecureSessionMgr * mgr) override
    {
        auto state            = mgr->GetPeerConnectionState(session);
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

    void OnNewConnection(SecureSessionHandle session, SecureSessionMgr * mgr) override
    {
        ChipLogProgress(AppServer, "Received a new connection.");
    }

    void SetDelegate(AppDelegate * delegate) { mDelegate = delegate; }

private:
    AppDelegate * mDelegate = nullptr;
};

#if CHIP_ENABLE_MDNS

CHIP_ERROR InitMdns()
{
    auto & mdnsAdvertiser = Mdns::ServiceAdvertiser::Instance();

    // TODO: advertise this only when really operational once we support both
    // operational and commisioning advertising is supported.
    if (ConfigurationMgr().IsFullyProvisioned())
    {
        uint64_t fabricId;

        if (ConfigurationMgr().GetFabricId(fabricId) != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Fabric ID not known. Using a default");
            fabricId = 5544332211;
        }

        const auto advertiseParameters = Mdns::OperationalAdvertisingParameters()
                                             .SetFabricId(fabricId)
                                             .SetNodeId(chip::kTestDeviceNodeId)
                                             .SetPort(CHIP_PORT)
                                             .EnableIpV4(true);

        ReturnErrorOnFailure(mdnsAdvertiser.Advertise(advertiseParameters));
    }
    else
    {
        auto advertiseParameters = Mdns::CommissionAdvertisingParameters().SetPort(CHIP_PORT).EnableIpV4(true);

        uint16_t value;
        if (ConfigurationMgr().GetVendorId(value) != CHIP_NO_ERROR)
        {
            ChipLogProgress(Discovery, "Vendor ID not known");
        }
        else
        {
            advertiseParameters.SetVendorId(chip::Optional<uint16_t>::Value(value));
        }

        if (ConfigurationMgr().GetProductId(value) != CHIP_NO_ERROR)
        {
            ChipLogProgress(Discovery, "Product ID not known");
        }
        else
        {
            advertiseParameters.SetProductId(chip::Optional<uint16_t>::Value(value));
        }

        if (ConfigurationMgr().GetSetupDiscriminator(value) != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Setup discriminator not known. Using a default.");
            value = 840;
        }
        advertiseParameters.SetShortDiscriminator(static_cast<uint8_t>(value & 0xFF)).SetLongDiscrimininator(value);

        ReturnErrorOnFailure(mdnsAdvertiser.Advertise(advertiseParameters));
    }

    return mdnsAdvertiser.Start(&DeviceLayer::InetLayer, chip::Mdns::kMdnsPort);
}
#endif

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

    gSessions.SetDelegate(&gCallbacks);

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
#if CONFIG_NETWORK_LAYER_BLE
        params.SetSetupPINCode(pinCode)
            .SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer())
            .SetPeerAddress(Transport::PeerAddress::BLE());
#else
        params.SetSetupPINCode(pinCode);
#endif // CONFIG_NETWORK_LAYER_BLE
        SuccessOrExit(err = gRendezvousServer.Init(params, &gTransports, &gSessions));
    }

#if CHIP_ENABLE_MDNS
    err = InitMdns();
    SuccessOrExit(err);
#endif

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
