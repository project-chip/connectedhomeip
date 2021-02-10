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

#include <app/InteractionModelEngine.h>
#include <app/server/DataModelHandler.h>
#include <app/server/RendezvousServer.h>
#include <app/server/SessionManager.h>

#include <ble/BLEEndPoint.h>
#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <mdns/Advertiser.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/SetupPayload.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/ReturnMacros.h>
#include <support/logging/CHIPLogging.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/AdminPairingTable.h>
#include <transport/SecureSessionMgr.h>

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Messaging;

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

// TODO: The following class is setting the discriminator in Persistent Storage. This is
//       is needed since BLE reads the discriminator using ConfigurationMgr APIs. The
//       better solution will be to pass the discriminator to BLE without changing it
//       in the persistent storage.
//       https://github.com/project-chip/connectedhomeip/issues/4767
class DeviceDiscriminatorCache
{
public:
    CHIP_ERROR UpdateDiscriminator(uint16_t discriminator)
    {
        if (!mOriginalDiscriminatorCached)
        {
            // Cache the original discriminator
            ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(mOriginalDiscriminator));
            mOriginalDiscriminatorCached = true;
        }

        return DeviceLayer::ConfigurationMgr().StoreSetupDiscriminator(discriminator);
    }

    CHIP_ERROR RestoreDiscriminator()
    {
        if (mOriginalDiscriminatorCached)
        {
            // Restore the original discriminator
            ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().StoreSetupDiscriminator(mOriginalDiscriminator));
            mOriginalDiscriminatorCached = false;
        }

        return CHIP_NO_ERROR;
    }

private:
    bool mOriginalDiscriminatorCached = false;
    uint16_t mOriginalDiscriminator   = 0;
};

DeviceDiscriminatorCache gDeviceDiscriminatorCache;

class ServerRendezvousAdvertisementDelegate : public RendezvousAdvertisementDelegate
{
public:
    CHIP_ERROR StartAdvertisement() const override
    {
        ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true));
        if (mDelegate != nullptr)
        {
            mDelegate->OnPairingWindowOpened();
        }
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR StopAdvertisement() const override
    {
        gDeviceDiscriminatorCache.RestoreDiscriminator();

        ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false));
        if (mDelegate != nullptr)
        {
            mDelegate->OnPairingWindowClosed();
        }
        return CHIP_NO_ERROR;
    }

    void SetDelegate(AppDelegate * delegate) { mDelegate = delegate; }

private:
    AppDelegate * mDelegate = nullptr;
};

DemoTransportMgr gTransports;
SecureSessionMgr gSessions;
RendezvousServer gRendezvousServer;
AdminPairingTable gAdminPairings;
AdminId gNextAvailableAdminId = 0;

ServerRendezvousAdvertisementDelegate gAdvDelegate;

static CHIP_ERROR OpenPairingWindowUsingVerifier(uint16_t discriminator, PASEVerifier & verifier)
{
    RendezvousParameters params;

    ReturnErrorOnFailure(gDeviceDiscriminatorCache.UpdateDiscriminator(discriminator));

#if CONFIG_NETWORK_LAYER_BLE
    params.SetPASEVerifier(verifier)
        .SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer())
        .SetPeerAddress(Transport::PeerAddress::BLE())
        .SetAdvertisementDelegate(&gAdvDelegate);
#else
    params.SetPASEVerifier(verifier);
#endif // CONFIG_NETWORK_LAYER_BLE

    AdminId admin                = gNextAvailableAdminId;
    AdminPairingInfo * adminInfo = gAdminPairings.AssignAdminId(admin);
    VerifyOrReturnError(adminInfo != nullptr, CHIP_ERROR_NO_MEMORY);
    gNextAvailableAdminId++;

    return gRendezvousServer.Init(std::move(params), &gTransports, &gSessions, adminInfo);
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

        // TODO: This code is temporary, and must be updated to use the Cluster API.
        // Issue: https://github.com/project-chip/connectedhomeip/issues/4725
        if (payloadHeader.GetProtocolID() == chip::Protocols::kProtocol_ServiceProvisioning)
        {
            CHIP_ERROR err = CHIP_NO_ERROR;
            uint32_t timeout;
            uint16_t discriminator;
            PASEVerifier verifier;

            ChipLogProgress(AppServer, "Received service provisioning message. Treating it as OpenPairingWindow request");
            chip::System::PacketBufferTLVReader reader;
            reader.Init(std::move(buffer));
            reader.ImplicitProfileId = chip::Protocols::kProtocol_ServiceProvisioning;

            SuccessOrExit(reader.Next(kTLVType_UnsignedInteger, TLV::ProfileTag(reader.ImplicitProfileId, 1)));
            SuccessOrExit(reader.Get(timeout));

            err = reader.Next(kTLVType_UnsignedInteger, TLV::ProfileTag(reader.ImplicitProfileId, 2));
            if (err == CHIP_NO_ERROR)
            {
                SuccessOrExit(reader.Get(discriminator));

                err = reader.Next(kTLVType_ByteString, TLV::ProfileTag(reader.ImplicitProfileId, 3));
                if (err == CHIP_NO_ERROR)
                {
                    SuccessOrExit(reader.GetBytes(reinterpret_cast<uint8_t *>(verifier), sizeof(verifier)));
                }
            }

            ChipLogProgress(AppServer, "Pairing Window timeout %d seconds", timeout);

            if (err != CHIP_NO_ERROR)
            {
                SuccessOrExit(err = OpenDefaultPairingWindow(ResetAdmins::kNo));
            }
            else
            {
                ChipLogProgress(AppServer, "Pairing Window discriminator %d", discriminator);
                err = OpenPairingWindowUsingVerifier(discriminator, verifier);
                SuccessOrExit(err);
            }
            ChipLogProgress(AppServer, "Opened the pairing window");
        }
        else
        {
            HandleDataModelMessage(header.GetSourceNodeId().Value(), std::move(buffer));
        }

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

#ifdef CHIP_APP_USE_INTERACTION_MODEL
Messaging::ExchangeManager gExchange;
#endif
ServerCallback gCallbacks;
SecurePairingUsingTestSecret gTestPairing;

} // namespace

SecureSessionMgr & chip::SessionManager()
{
    return gSessions;
}

CHIP_ERROR OpenDefaultPairingWindow(ResetAdmins resetAdmins)
{
    gDeviceDiscriminatorCache.RestoreDiscriminator();

    uint32_t pinCode;
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSetupPinCode(pinCode));

    RendezvousParameters params;

#if CONFIG_NETWORK_LAYER_BLE
    params.SetSetupPINCode(pinCode)
        .SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer())
        .SetPeerAddress(Transport::PeerAddress::BLE())
        .SetAdvertisementDelegate(&gAdvDelegate);
#else
    params.SetSetupPINCode(pinCode);
#endif // CONFIG_NETWORK_LAYER_BLE

    if (resetAdmins == ResetAdmins::kYes)
    {
        gNextAvailableAdminId = 0;
        gAdminPairings.Reset();
    }

    AdminId admin                = gNextAvailableAdminId;
    AdminPairingInfo * adminInfo = gAdminPairings.AssignAdminId(admin);
    VerifyOrReturnError(adminInfo != nullptr, CHIP_ERROR_NO_MEMORY);
    gNextAvailableAdminId++;

    return gRendezvousServer.Init(std::move(params), &gTransports, &gSessions, adminInfo);
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
    gAdvDelegate.SetDelegate(delegate);

    // Init transport before operations with secure session mgr.
#if INET_CONFIG_ENABLE_IPV4
    err = gTransports.Init(UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6),
                           UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv4));
#else
    err = gTransports.Init(UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6));
#endif
    SuccessOrExit(err);

    err = gSessions.Init(chip::kTestDeviceNodeId, &DeviceLayer::SystemLayer, &gTransports, &gAdminPairings);
    SuccessOrExit(err);

#ifdef CHIP_APP_USE_INTERACTION_MODEL
    err = gExchange.Init(&gSessions);
    SuccessOrExit(err);
    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchange);
    SuccessOrExit(err);
#else
    gSessions.SetDelegate(&gCallbacks);
#endif

    // This flag is used to bypass BLE in the cirque test
    // Only in the cirque test this is enabled with --args='bypass_rendezvous=true'
    if (isRendezvousBypassed())
    {
        AdminPairingInfo * adminInfo = gAdminPairings.AssignAdminId(gNextAvailableAdminId);
        VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);
        adminInfo->SetNodeId(chip::kTestDeviceNodeId);
        ChipLogProgress(AppServer, "Rendezvous and Secure Pairing skipped. Using test secret.");
        err = gSessions.NewPairing(peer, chip::kTestControllerNodeId, &gTestPairing, gNextAvailableAdminId);
        SuccessOrExit(err);
    }
    else if (DeviceLayer::ConnectivityMgr().IsWiFiStationProvisioned() || DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        // If the network is already provisioned, proactively disable BLE advertisement.
        ChipLogProgress(AppServer, "Network already provisioned. Disabling BLE advertisement");
        chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false);
    }
    else
    {
        SuccessOrExit(err = OpenDefaultPairingWindow(ResetAdmins::kYes));
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
