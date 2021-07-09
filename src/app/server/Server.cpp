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

#include <inttypes.h>

#include <app/server/Server.h>

#include <app/InteractionModelEngine.h>
#include <app/server/EchoHandler.h>
#include <app/server/RendezvousServer.h>
#include <app/server/StorablePeerConnection.h>
#include <app/util/DataModelHandler.h>

#include <ble/BLEEndPoint.h>
#include <core/CHIPPersistentStorageDelegate.h>
#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <setup_payload/SetupPayload.h>
#include <support/CodeUtils.h>
#include <support/ErrorStr.h>
#include <support/logging/CHIPLogging.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SecureSessionMgr.h>

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
#include <app/server/Mdns.h>
#endif

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
    return static_cast<RendezvousInformationFlag>(CONFIG_RENDEZVOUS_MODE) == RendezvousInformationFlag::kNone;
#else
    return false;
#endif
}

constexpr bool useTestPairing()
{
    // Use the test pairing whenever rendezvous is bypassed. Otherwise, there wouldn't be
    // any way to communicate with the device using CHIP protocol.
    // This is used to bypass BLE in the cirque test.
    // Only in the cirque test this is enabled with --args='bypass_rendezvous=true'.
    return isRendezvousBypassed();
}

class ServerStorageDelegate : public PersistentStorageDelegate
{
    CHIP_ERROR SyncGetKeyValue(const char * key, void * buffer, uint16_t & size) override
    {
        ChipLogProgress(AppServer, "Retrieved value from server storage.");
        return PersistedStorage::KeyValueStoreMgr().Get(key, buffer, size);
    }

    CHIP_ERROR SyncSetKeyValue(const char * key, const void * value, uint16_t size) override
    {
        ChipLogProgress(AppServer, "Stored value in server storage");
        return PersistedStorage::KeyValueStoreMgr().Put(key, value, size);
    }

    CHIP_ERROR SyncDeleteKeyValue(const char * key) override
    {
        ChipLogProgress(AppServer, "Delete value in server storage");
        return PersistedStorage::KeyValueStoreMgr().Delete(key);
    }
};

ServerStorageDelegate gServerStorage;
SessionIDAllocator gSessionIDAllocator;

CHIP_ERROR PersistAdminPairingToKVS(AdminPairingInfo * admin, AdminId nextAvailableId)
{
    ReturnErrorCodeIf(admin == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ChipLogProgress(AppServer, "Persisting admin ID %d, next available %d", admin->GetAdminId(), nextAvailableId);

    ReturnErrorOnFailure(GetGlobalAdminPairingTable().Store(admin->GetAdminId()));
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
        // Recreate the binding if one exists in persistent storage. Else skip to the next ID
        if (adminPairings.LoadFromStorage(id) == CHIP_NO_ERROR)
        {
            AdminPairingInfo * admin = adminPairings.FindAdminWithId(id);
            if (admin != nullptr)
            {
                ChipLogProgress(AppServer, "Found admin pairing for %d, node ID 0x" ChipLogFormatX64, admin->GetAdminId(),
                                ChipLogValueX64(admin->GetNodeId()));
            }
        }
    }
    ChipLogProgress(AppServer, "Restored all admin pairings from KVS.");

    return CHIP_NO_ERROR;
}

void EraseAllAdminPairingsUpTo(AdminId nextAvailableId)
{
    PersistedStorage::KeyValueStoreMgr().Delete(kAdminTableCountKey);

    for (AdminId id = 0; id < nextAvailableId; id++)
    {
        GetGlobalAdminPairingTable().Delete(id);
    }
}

static CHIP_ERROR RestoreAllSessionsFromKVS(SecureSessionMgr & sessionMgr)
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

            ChipLogProgress(AppServer, "Fetched the session information: from 0x" ChipLogFormatX64,
                            ChipLogValueX64(session->PeerConnection().GetPeerNodeId()));
            if (gSessionIDAllocator.Reserve(keyId) == CHIP_NO_ERROR)
            {
                sessionMgr.NewPairing(Optional<Transport::PeerAddress>::Value(session->PeerConnection().GetPeerAddress()),
                                      session->PeerConnection().GetPeerNodeId(), session, SecureSession::SessionRole::kResponder,
                                      connection.GetAdminId());
            }
            else
            {
                ChipLogProgress(AppServer, "Session Key ID  %" PRIu16 " cannot be used. Skipping over this session", keyId);
            }
            session->Clear();
        }
    }

    chip::Platform::Delete(session);

    return CHIP_NO_ERROR;
}

void EraseAllSessionsUpTo(uint16_t nextSessionKeyId)
{
    PersistedStorage::KeyValueStoreMgr().Delete(kStorablePeerConnectionCountKey);

    for (uint16_t keyId = 0; keyId < nextSessionKeyId; keyId++)
    {
        gSessionIDAllocator.Free(keyId);
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
        if (isBLE)
        {
            ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true));
        }
        if (mDelegate != nullptr)
        {
            mDelegate->OnPairingWindowOpened();
        }
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR StopAdvertisement() const override
    {
        gDeviceDiscriminatorCache.RestoreDiscriminator();

        if (isBLE)
        {
            ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false));
        }

        if (mDelegate != nullptr)
        {
            mDelegate->OnPairingWindowClosed();
        }

        AdminPairingInfo * admin = gAdminPairings.FindAdminWithId(mAdmin);
        if (admin != nullptr)
        {
            ReturnErrorOnFailure(PersistAdminPairingToKVS(admin, gNextAvailableAdminId));
        }

        return CHIP_NO_ERROR;
    }

    void SetDelegate(AppDelegate * delegate) { mDelegate = delegate; }
    void SetBLE(bool ble) { isBLE = ble; }
    void SetAdminId(AdminId id) { mAdmin = id; }

private:
    AppDelegate * mDelegate = nullptr;
    AdminId mAdmin;
    bool isBLE = true;
};

DemoTransportMgr gTransports;
SecureSessionMgr gSessions;
RendezvousServer gRendezvousServer;
CASEServer gCASEServer;
Messaging::ExchangeManager gExchangeMgr;
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

    return gRendezvousServer.WaitForPairing(std::move(params), &gExchangeMgr, &gTransports, &gSessions, adminInfo);
}

class ServerCallback : public ExchangeDelegate
{
public:
    CHIP_ERROR OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                 const PayloadHeader & payloadHeader, System::PacketBufferHandle && buffer) override
    {
        CHIP_ERROR err = CHIP_NO_ERROR;
        // as soon as a client connects, assume it is connected
        VerifyOrExit(!buffer.IsNull(), ChipLogError(AppServer, "Received data but couldn't process it..."));
        VerifyOrExit(packetHeader.GetSourceNodeId().HasValue(), ChipLogError(AppServer, "Unknown source for received message"));

        VerifyOrExit(mSessionMgr != nullptr, ChipLogError(AppServer, "SecureSessionMgr is not initilized yet"));

        VerifyOrExit(packetHeader.GetSourceNodeId().Value() != kUndefinedNodeId,
                     ChipLogError(AppServer, "Unknown source for received message"));

        ChipLogProgress(AppServer, "Packet received from Node 0x" ChipLogFormatX64 ": %u bytes",
                        ChipLogValueX64(packetHeader.GetSourceNodeId().Value()), buffer->DataLength());

        // TODO: This code is temporary, and must be updated to use the Cluster API.
        // Issue: https://github.com/project-chip/connectedhomeip/issues/4725
        if (payloadHeader.HasProtocol(chip::Protocols::ServiceProvisioning::Id))
        {
            uint32_t timeout;
            uint16_t discriminator;
            PASEVerifier verifier;

            ChipLogProgress(AppServer, "Received service provisioning message. Treating it as OpenPairingWindow request");
            chip::System::PacketBufferTLVReader reader;
            reader.Init(std::move(buffer));
            reader.ImplicitProfileId = chip::Protocols::ServiceProvisioning::Id.ToTLVProfileId();

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

            ChipLogProgress(AppServer, "Pairing Window timeout %" PRIu32 " seconds", timeout);

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
            HandleDataModelMessage(exchangeContext, std::move(buffer));
        }

    exit:
        exchangeContext->Close();
        return err;
    }

    void OnResponseTimeout(ExchangeContext * ec) override
    {
        ChipLogProgress(AppServer, "Failed to receive response");
        if (mDelegate != nullptr)
        {
            mDelegate->OnReceiveError();
        }
    }

    void SetDelegate(AppDelegate * delegate) { mDelegate = delegate; }
    void SetSessionMgr(SecureSessionMgr * mgr) { mSessionMgr = mgr; }

private:
    AppDelegate * mDelegate        = nullptr;
    SecureSessionMgr * mSessionMgr = nullptr;
};

secure_channel::MessageCounterManager gMessageCounterManager;
ServerCallback gCallbacks;
SecurePairingUsingTestSecret gTestPairing;

} // namespace

CHIP_ERROR OpenDefaultPairingWindow(ResetAdmins resetAdmins, chip::PairingWindowAdvertisement advertisementMode)
{
    // TODO(cecille): If this is re-called when the window is already open, what should happen?
    gDeviceDiscriminatorCache.RestoreDiscriminator();

    uint32_t pinCode;
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSetupPinCode(pinCode));

    RendezvousParameters params;

    params.SetSetupPINCode(pinCode);
#if CONFIG_NETWORK_LAYER_BLE
    gAdvDelegate.SetBLE(advertisementMode == chip::PairingWindowAdvertisement::kBle);
    params.SetAdvertisementDelegate(&gAdvDelegate);
    if (advertisementMode == chip::PairingWindowAdvertisement::kBle)
    {
        params.SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer()).SetPeerAddress(Transport::PeerAddress::BLE());
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    if (resetAdmins == ResetAdmins::kYes)
    {
        EraseAllAdminPairingsUpTo(gNextAvailableAdminId);
        EraseAllSessionsUpTo(gSessionIDAllocator.Peek());
        // Only resetting gNextAvailableAdminId at reboot otherwise previously paired device with adminID 0
        // can continue sending messages to accessory as next available admin will also be 0.
        // This logic is not up to spec, will be implemented up to spec once AddOptCert is implemented.
        gAdminPairings.Reset();
    }

    AdminId admin                = gNextAvailableAdminId;
    AdminPairingInfo * adminInfo = gAdminPairings.AssignAdminId(admin);
    VerifyOrReturnError(adminInfo != nullptr, CHIP_ERROR_NO_MEMORY);
    gNextAvailableAdminId++;

    return gRendezvousServer.WaitForPairing(std::move(params), &gExchangeMgr, &gTransports, &gSessions, adminInfo);
}

// The function will initialize datamodel handler and then start the server
// The server assumes the platform's networking has been setup already
void InitServer(AppDelegate * delegate)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Platform::MemoryInit();

    InitDataModelHandler(&gExchangeMgr);
    gCallbacks.SetDelegate(delegate);

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    err = PersistedStorage::KeyValueStoreMgrImpl().Init("chip.store");
    SuccessOrExit(err);
#elif CHIP_DEVICE_LAYER_TARGET_LINUX
    PersistedStorage::KeyValueStoreMgrImpl().Init("/tmp/chip_server_kvs");
#endif

    err = gRendezvousServer.Init(delegate, &gServerStorage, &gSessionIDAllocator);
    SuccessOrExit(err);

    gAdvDelegate.SetDelegate(delegate);

    err = gAdminPairings.Init(&gServerStorage);
    SuccessOrExit(err);

    // Init transport before operations with secure session mgr.
    err = gTransports.Init(UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv6)

#if INET_CONFIG_ENABLE_IPV4
                               ,
                           UdpListenParameters(&DeviceLayer::InetLayer).SetAddressType(kIPAddressType_IPv4)
#endif
#if CONFIG_NETWORK_LAYER_BLE
                               ,
                           BleListenParameters(DeviceLayer::ConnectivityMgr().GetBleLayer())
#endif
    );

    SuccessOrExit(err);

    err =
        gSessions.Init(chip::kTestDeviceNodeId, &DeviceLayer::SystemLayer, &gTransports, &gAdminPairings, &gMessageCounterManager);
    SuccessOrExit(err);

    err = gExchangeMgr.Init(&gSessions);
    SuccessOrExit(err);
    err = gMessageCounterManager.Init(&gExchangeMgr);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&gExchangeMgr, nullptr);
    SuccessOrExit(err);

#if defined(CHIP_APP_USE_ECHO)
    err = InitEchoHandler(&gExchangeMgr);
    SuccessOrExit(err);
#endif

    if (useTestPairing())
    {
        ChipLogProgress(AppServer, "Rendezvous and secure pairing skipped");
        SuccessOrExit(err = AddTestPairing());
    }
    else if (DeviceLayer::ConnectivityMgr().IsWiFiStationProvisioned() || DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        // If the network is already provisioned, proactively disable BLE advertisement.
        ChipLogProgress(AppServer, "Network already provisioned. Disabling BLE advertisement");
        chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false);

        // Restore any previous admin pairings
        VerifyOrExit(CHIP_NO_ERROR == RestoreAllAdminPairingsFromKVS(gAdminPairings, gNextAvailableAdminId),
                     ChipLogError(AppServer, "Could not restore admin table"));

        VerifyOrExit(CHIP_NO_ERROR == RestoreAllSessionsFromKVS(gSessions),
                     ChipLogError(AppServer, "Could not restore previous sessions"));
    }
    else
    {
#if CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART
        SuccessOrExit(err = OpenDefaultPairingWindow(ResetAdmins::kYes));
#endif
    }

// ESP32 and Mbed OS examples have a custom logic for enabling DNS-SD
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !CHIP_DEVICE_LAYER_TARGET_ESP32 && !CHIP_DEVICE_LAYER_TARGET_MBED
    app::Mdns::StartServer();
#endif

    gCallbacks.SetSessionMgr(&gSessions);

    // Register to receive unsolicited legacy ZCL messages from the exchange manager.
    err = gExchangeMgr.RegisterUnsolicitedMessageHandlerForProtocol(Protocols::TempZCL::Id, &gCallbacks);
    VerifyOrExit(err == CHIP_NO_ERROR, err = CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER);

    // Register to receive unsolicited Service Provisioning messages from the exchange manager.
    err = gExchangeMgr.RegisterUnsolicitedMessageHandlerForProtocol(Protocols::ServiceProvisioning::Id, &gCallbacks);
    VerifyOrExit(err == CHIP_NO_ERROR, err = CHIP_ERROR_NO_UNSOLICITED_MESSAGE_HANDLER);

    err = gCASEServer.ListenForSessionEstablishment(&gExchangeMgr, &gTransports, &gSessions, &GetGlobalAdminPairingTable(),
                                                    &gSessionIDAllocator);
    SuccessOrExit(err);

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
    PASESession * testSession    = nullptr;
    PASESessionSerializable serializedTestSession;

    for (const AdminPairingInfo & admin : gAdminPairings)
        if (admin.IsInitialized() && admin.GetNodeId() == chip::kTestDeviceNodeId)
            ExitNow();

    adminInfo = gAdminPairings.AssignAdminId(gNextAvailableAdminId);
    VerifyOrExit(adminInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

    adminInfo->SetNodeId(chip::kTestDeviceNodeId);
    gTestPairing.ToSerializable(serializedTestSession);

    testSession = chip::Platform::New<PASESession>();
    testSession->FromSerializable(serializedTestSession);
    SuccessOrExit(err = gSessions.NewPairing(Optional<PeerAddress>{ PeerAddress::Uninitialized() }, chip::kTestControllerNodeId,
                                             testSession, SecureSession::SessionRole::kResponder, gNextAvailableAdminId));
    ++gNextAvailableAdminId;

exit:
    if (testSession)
    {
        testSession->Clear();
        chip::Platform::Delete(testSession);
    }

    if (err != CHIP_NO_ERROR && adminInfo != nullptr)
        gAdminPairings.ReleaseAdminId(gNextAvailableAdminId);

    return err;
}

AdminPairingTable & GetGlobalAdminPairingTable()
{
    return gAdminPairings;
}
