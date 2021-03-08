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
#include <app/server/EchoHandler.h>
#include <app/server/RendezvousServer.h>
#include <app/server/SessionManager.h>

#include <ble/BLEEndPoint.h>
#include <core/CHIPPersistentStorageDelegate.h>
#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <setup_payload/SetupPayload.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/ReturnMacros.h>
#include <support/logging/CHIPLogging.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>
#include <transport/StorablePeerConnection.h>

#include "Mdns.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Messaging;

namespace {

constexpr bool isRendezvousBypassed()
{
#if defined(CHIP_BYPASS_RENDEZVOUS) && CHIP_BYPASS_RENDEZVOUS
    return true;
#elif defined(CONFIG_RENDEZVOUS_MODE)
    return static_cast<RendezvousInformationFlags>(CONFIG_RENDEZVOUS_MODE) == RendezvousInformationFlags::kNone;
#else
    return false;
#endif
}

constexpr bool useTestPairing()
{
    // Use the test pairing whenever rendezvous is bypassed. Otherwise, there wouldn't be
    // any way to communicate with the device using CHIP protocol.
    return isRendezvousBypassed();
}

class ServerStorageDelegate : public PersistentStorageDelegate
{
    void SetDelegate(PersistentStorageResultDelegate * delegate) override
    {
        ChipLogError(AppServer, "ServerStorageDelegate does not support async operations");
        chipDie();
    }

    void GetKeyValue(const char * key) override
    {
        ChipLogError(AppServer, "ServerStorageDelegate does not support async operations");
        chipDie();
    }

    void SetKeyValue(const char * key, const char * value) override
    {
        ChipLogError(AppServer, "ServerStorageDelegate does not support async operations");
        chipDie();
    }

    CHIP_ERROR GetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        return PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size);
    }

    CHIP_ERROR SetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        return PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
    }

    void DeleteKeyValue(const char * key) override { PersistedStorage::KeyValueStoreMgr().Delete(key); }
};

ServerStorageDelegate gServerStorage;

CHIP_ERROR PersistAdminPairingToKVS(AdminPairingInfo * admin, AdminId nextAvailableId)
{
    ReturnErrorCodeIf(admin == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ChipLogProgress(AppServer, "Persisting admin ID %d, next available %d", admin->GetAdminId(), nextAvailableId);

    ReturnErrorOnFailure(admin->StoreIntoKVS(gServerStorage));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kAdminTableCountKey, &nextAvailableId, sizeof(nextAvailableId)));

    ChipLogProgress(AppServer, "Persisting admin ID successfully");
    return CHIP_NO_ERROR;
}

CHIP_ERROR RestoreAllAdminPairingsFromKVS(AdminPairingTable & adminPairings, AdminId & nextAvailableId)
{
    // It's not an error if the key doesn't exist. Just return right away.
    VerifyOrReturnError(PersistedStorage::KeyValueStoreMgr().Get(kAdminTableCountKey, &nextAvailableId) == CHIP_NO_ERROR,
                        CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Next available admin ID is %d", nextAvailableId);

    // TODO: The admin ID space allocation should be re-evaluated. With the current approach, the space could be
    //       exhausted while IDs are still available (e.g. if the admin IDs are allocated and freed over a period of time).
    //       Also, the current approach can make ID lookup slower as more IDs are allocated and freed.
    for (AdminId id = 0; id < nextAvailableId; id++)
    {
        AdminPairingInfo * admin = adminPairings.AssignAdminId(id);
        // Recreate the binding if one exists in persistent storage. Else skip to the next ID
        if (admin->FetchFromKVS(gServerStorage) != CHIP_NO_ERROR)
        {
            adminPairings.ReleaseAdminId(id);
        }
        else
        {
            ChipLogProgress(AppServer, "Found admin pairing for %d, node ID %llu", admin->GetAdminId(), admin->GetNodeId());
        }
    }

    return CHIP_NO_ERROR;
}

void EraseAllAdminPairingsUpTo(AdminId nextAvailableId)
{
    PersistedStorage::KeyValueStoreMgr().Delete(kAdminTableCountKey);

    for (AdminId id = 0; id < nextAvailableId; id++)
    {
        AdminPairingInfo::DeleteFromKVS(gServerStorage, id);
    }
}

static CHIP_ERROR RestoreAllSessionsFromKVS(SecureSessionMgr & sessionMgr, RendezvousServer & server)
{
    uint16_t nextSessionKeyId = 0;
    // It's not an error if the key doesn't exist. Just return right away.
    VerifyOrReturnError(PersistedStorage::KeyValueStoreMgr().Get(kStorablePeerConnectionCountKey, &nextSessionKeyId) ==
                            CHIP_NO_ERROR,
                        CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Found %d stored connections", nextSessionKeyId);

    PASESession * session = chip::Platform::New<PASESession>();
    VerifyOrReturnError(session != nullptr, CHIP_ERROR_NO_MEMORY);

    for (uint16_t keyId = 0; keyId < nextSessionKeyId; keyId++)
    {
        StorablePeerConnection connection;
        if (CHIP_NO_ERROR == connection.FetchFromKVS(gServerStorage, keyId))
        {
            connection.GetPASESession(session);

            ChipLogProgress(AppServer, "Fetched the session information: from %llu", session->PeerConnection().GetPeerNodeId());
            sessionMgr.NewPairing(Optional<Transport::PeerAddress>::Value(session->PeerConnection().GetPeerAddress()),
                                  session->PeerConnection().GetPeerNodeId(), session,
                                  SecureSessionMgr::PairingDirection::kResponder, connection.GetAdminId(), nullptr);
            session->Clear();
        }
    }

    chip::Platform::Delete(session);

    server.GetRendezvousSession()->SetNextKeyId(nextSessionKeyId);
    return CHIP_NO_ERROR;
}

void EraseAllSessionsUpTo(uint16_t nextSessionKeyId)
{
    PersistedStorage::KeyValueStoreMgr().Delete(kStorablePeerConnectionCountKey);

    for (uint16_t keyId = 0; keyId < nextSessionKeyId; keyId++)
    {
        StorablePeerConnection::DeleteFromKVS(gServerStorage, keyId);
    }
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
AdminPairingTable gAdminPairings;
AdminId gNextAvailableAdminId = 0;

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
        {
            if (mDelegate != nullptr)
                mDelegate->OnPairingWindowClosed();
        }

        AdminPairingInfo * admin = gAdminPairings.FindAdmin(mAdmin);
        if (admin != nullptr)
        {
            ReturnErrorOnFailure(PersistAdminPairingToKVS(admin, gNextAvailableAdminId));
        }

        return CHIP_NO_ERROR;
    }

    void RendezvousComplete() const override
    {
        // Once rendezvous completed, assume we are operational
        if (app::Mdns::AdvertiseOperational() != CHIP_NO_ERROR)
        {
            ChipLogError(Discovery, "Failed to start advertising operational state at rendezvous completion time.");
        }
    }

    void SetDelegate(AppDelegate * delegate) { mDelegate = delegate; }

    void SetAdminId(AdminId id) { mAdmin = id; }

private:
    AppDelegate * mDelegate = nullptr;
    AdminId mAdmin;
};

DemoTransportMgr gTransports;
SecureSessionMgr gSessions;
RendezvousServer gRendezvousServer;

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

    return gRendezvousServer.WaitForPairing(std::move(params), &gTransports, &gSessions, adminInfo);
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

        state->GetPeerAddress().ToString(src_addr);

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

#if defined(CHIP_APP_USE_INTERACTION_MODEL) || defined(CHIP_APP_USE_ECHO)
Messaging::ExchangeManager gExchangeMgr;
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
        uint16_t nextKeyId = gRendezvousServer.GetRendezvousSession()->GetNextKeyId();
        EraseAllAdminPairingsUpTo(gNextAvailableAdminId);
        EraseAllSessionsUpTo(nextKeyId);
        gNextAvailableAdminId = 0;
        gAdminPairings.Reset();
    }

    AdminId admin                = gNextAvailableAdminId;
    AdminPairingInfo * adminInfo = gAdminPairings.AssignAdminId(admin);
    VerifyOrReturnError(adminInfo != nullptr, CHIP_ERROR_NO_MEMORY);
    gNextAvailableAdminId++;

    return gRendezvousServer.WaitForPairing(std::move(params), &gTransports, &gSessions, adminInfo);
}

// The function will initialize datamodel handler and then start the server
// The server assumes the platform's networking has been setup already
void InitServer(AppDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Platform::MemoryInit();

    InitDataModelHandler();
    gCallbacks.SetDelegate(delegate);

    err = gRendezvousServer.Init(delegate, &gServerStorage);
    SuccessOrExit(err);

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

#if defined(CHIP_APP_USE_INTERACTION_MODEL) || defined(CHIP_APP_USE_ECHO)
    err = gExchangeMgr.Init(&gSessions);
    SuccessOrExit(err);
#else
    gSessions.SetDelegate(&gCallbacks);
#endif

#if defined(CHIP_APP_USE_INTERACTION_MODEL)
    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeMgr);
    SuccessOrExit(err);
#endif

#if defined(CHIP_APP_USE_ECHO)
    err = InitEchoHandler(&gExchangeMgr);
    SuccessOrExit(err);
#endif

    if (useTestPairing())
    {
        SuccessOrExit(err = AddTestPairing());
    }

    // This flag is used to bypass BLE in the cirque test
    // Only in the cirque test this is enabled with --args='bypass_rendezvous=true'
    if (isRendezvousBypassed())
    {
        ChipLogProgress(AppServer, "Rendezvous and secure pairing skipped");
    }
    else if (DeviceLayer::ConnectivityMgr().IsWiFiStationProvisioned() || DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        // If the network is already provisioned, proactively disable BLE advertisement.
        ChipLogProgress(AppServer, "Network already provisioned. Disabling BLE advertisement");
        chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false);

        // Restore any previous admin pairings
        VerifyOrExit(CHIP_NO_ERROR == RestoreAllAdminPairingsFromKVS(gAdminPairings, gNextAvailableAdminId),
                     ChipLogError(AppServer, "Could not restore admin table"));

        VerifyOrExit(CHIP_NO_ERROR == RestoreAllSessionsFromKVS(gSessions, gRendezvousServer),
                     ChipLogError(AppServer, "Could not restore previous sessions"));
    }
    else
    {
#if CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART
        SuccessOrExit(err = OpenDefaultPairingWindow(ResetAdmins::kYes));
#endif
    }

// Starting mDNS server only for Thread devices due to problem reported in issue #5076.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    app::Mdns::StartServer();
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

CHIP_ERROR AddTestPairing()
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    AdminPairingInfo * adminInfo = nullptr;

    for (const AdminPairingInfo & admin : gAdminPairings)
        if (admin.IsInitialized() && admin.GetNodeId() == chip::kTestDeviceNodeId)
            ExitNow();

    adminInfo = gAdminPairings.AssignAdminId(gNextAvailableAdminId);
    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

    adminInfo->SetNodeId(chip::kTestDeviceNodeId);
    SuccessOrExit(err = gSessions.NewPairing(Optional<PeerAddress>{ PeerAddress::Uninitialized() }, chip::kTestControllerNodeId,
                                             &gTestPairing, SecureSessionMgr::PairingDirection::kResponder, gNextAvailableAdminId));
    ++gNextAvailableAdminId;

exit:
    if (err != CHIP_NO_ERROR && adminInfo != nullptr)
        gAdminPairings.ReleaseAdminId(gNextAvailableAdminId);

    return err;
}

AdminPairingTable & GetGlobalAdminPairingTable()
{
    return gAdminPairings;
}
