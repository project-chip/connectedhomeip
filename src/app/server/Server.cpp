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
#include <mdns/ServiceNaming.h>
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
#include <transport/FabricTable.h>
#include <transport/SecureSessionMgr.h>

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT || CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#include <protocols/user_directed_commissioning/UserDirectedCommissioning.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT || CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

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

CHIP_ERROR PersistFabricToKVS(FabricInfo * fabric, FabricIndex nextAvailableId)
{
    ReturnErrorCodeIf(fabric == nullptr, CHIP_ERROR_INVALID_ARGUMENT);
    ChipLogProgress(AppServer, "Persisting fabric ID %d, next available %d", fabric->GetFabricIndex(), nextAvailableId);

    ReturnErrorOnFailure(GetGlobalFabricTable().Store(fabric->GetFabricIndex()));
    ReturnErrorOnFailure(PersistedStorage::KeyValueStoreMgr().Put(kFabricTableCountKey, &nextAvailableId, sizeof(nextAvailableId)));

    ChipLogProgress(AppServer, "Persisting fabric ID successfully");
    return CHIP_NO_ERROR;
}

CHIP_ERROR RestoreAllFabricsFromKVS(FabricTable & fabrics, FabricIndex & nextAvailableId)
{
    // It's not an error if the key doesn't exist. Just return right away.
    VerifyOrReturnError(PersistedStorage::KeyValueStoreMgr().Get(kFabricTableCountKey, &nextAvailableId) == CHIP_NO_ERROR,
                        CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Next available fabric ID is %d", nextAvailableId);

    // TODO: The fabric ID space allocation should be re-evaluated. With the current approach, the space could be
    //       exhausted while IDs are still available (e.g. if the fabric IDs are allocated and freed over a period of time).
    //       Also, the current approach can make ID lookup slower as more IDs are allocated and freed.
    for (FabricIndex id = 0; id < nextAvailableId; id++)
    {
        // Recreate the binding if one exists in persistent storage. Else skip to the next ID
        if (fabrics.LoadFromStorage(id) == CHIP_NO_ERROR)
        {
            FabricInfo * fabric = fabrics.FindFabricWithIndex(id);
            if (fabric != nullptr)
            {
                ChipLogProgress(AppServer, "Found fabric pairing for %d, node ID 0x" ChipLogFormatX64, fabric->GetFabricIndex(),
                                ChipLogValueX64(fabric->GetNodeId()));
            }
        }
    }
    ChipLogProgress(AppServer, "Restored all fabric pairings from KVS.");

    return CHIP_NO_ERROR;
}

void EraseAllFabricsUpTo(FabricIndex nextAvailableId)
{
    PersistedStorage::KeyValueStoreMgr().Delete(kFabricTableCountKey);

    for (FabricIndex id = 0; id < nextAvailableId; id++)
    {
        GetGlobalFabricTable().Delete(id);
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
                                      connection.GetFabricIndex());
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
FabricTable gFabrics;
FabricIndex gNextAvailableFabricIndex = 0;
bool gPairingWindowOpen               = false;

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
        gPairingWindowOpen = true;
        return CHIP_NO_ERROR;
    }
    CHIP_ERROR StopAdvertisement() const override
    {
        gDeviceDiscriminatorCache.RestoreDiscriminator();

        gPairingWindowOpen = false;

        if (isBLE)
        {
            ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false));
        }

        if (mDelegate != nullptr)
        {
            mDelegate->OnPairingWindowClosed();
        }

        FabricInfo * fabric = gFabrics.FindFabricWithIndex(mFabric);
        if (fabric != nullptr)
        {
            ReturnErrorOnFailure(PersistFabricToKVS(fabric, gNextAvailableFabricIndex));
        }

        return CHIP_NO_ERROR;
    }

    void SetDelegate(AppDelegate * delegate) { mDelegate = delegate; }
    void SetBLE(bool ble) { isBLE = ble; }
    void SetFabricIndex(FabricIndex id) { mFabric = id; }

private:
    AppDelegate * mDelegate = nullptr;
    FabricIndex mFabric;
    bool isBLE = true;
};

DemoTransportMgr gTransports;
SecureSessionMgr gSessions;
RendezvousServer gRendezvousServer;
CASEServer gCASEServer;
Messaging::ExchangeManager gExchangeMgr;
ServerRendezvousAdvertisementDelegate gAdvDelegate;

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

        HandleDataModelMessage(exchangeContext, std::move(buffer));

    exit:
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

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

chip::Protocols::UserDirectedCommissioning::UserDirectedCommissioningClient gUDCClient;

#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

void HandlePairingWindowTimeout(System::Layer * aSystemLayer, void * aAppState, CHIP_ERROR aError)
{
    ClosePairingWindow();
}

} // namespace

CHIP_ERROR OpenDefaultPairingWindow(ResetFabrics resetFabrics, uint16_t commissioningTimeoutSeconds,
                                    chip::PairingWindowAdvertisement advertisementMode)
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

    if (resetFabrics == ResetFabrics::kYes)
    {
        EraseAllFabricsUpTo(gNextAvailableFabricIndex);
        EraseAllSessionsUpTo(gSessionIDAllocator.Peek());
        // Only resetting gNextAvailableFabricIndex at reboot otherwise previously paired device with fabricID 0
        // can continue sending messages to accessory as next available fabric will also be 0.
        // This logic is not up to spec, will be implemented up to spec once AddOptCert is implemented.
        gFabrics.Reset();
    }

    FabricIndex fabricIndex = gNextAvailableFabricIndex;
    FabricInfo * fabricInfo = gFabrics.AssignFabricIndex(fabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_NO_MEMORY);
    gNextAvailableFabricIndex++;

    ReturnErrorOnFailure(gRendezvousServer.WaitForPairing(
        std::move(params), kSpake2p_Iteration_Count,
        ByteSpan(reinterpret_cast<const uint8_t *>(kSpake2pKeyExchangeSalt), strlen(kSpake2pKeyExchangeSalt)), 0, &gExchangeMgr,
        &gTransports, &gSessions, fabricInfo));

    if (commissioningTimeoutSeconds != kNoCommissioningTimeout)
    {
        ReturnErrorOnFailure(
            DeviceLayer::SystemLayer.StartTimer(commissioningTimeoutSeconds * 1000, HandlePairingWindowTimeout, nullptr));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR OpenPairingWindowUsingVerifier(uint16_t commissioningTimeoutSeconds, uint16_t discriminator, PASEVerifier & verifier,
                                          uint32_t iterations, ByteSpan salt, uint16_t passcodeID)
{
    RendezvousParameters params;

    ReturnErrorOnFailure(gDeviceDiscriminatorCache.UpdateDiscriminator(discriminator));

    gAdvDelegate.SetBLE(false);
    params.SetPASEVerifier(verifier).SetAdvertisementDelegate(&gAdvDelegate);

    FabricIndex fabricIndex = gNextAvailableFabricIndex;
    FabricInfo * fabricInfo = gFabrics.AssignFabricIndex(fabricIndex);
    VerifyOrReturnError(fabricInfo != nullptr, CHIP_ERROR_NO_MEMORY);
    gNextAvailableFabricIndex++;

    ReturnErrorOnFailure(gRendezvousServer.WaitForPairing(std::move(params), iterations, salt, passcodeID, &gExchangeMgr,
                                                          &gTransports, &gSessions, fabricInfo));

    if (commissioningTimeoutSeconds != kNoCommissioningTimeout)
    {
        ReturnErrorOnFailure(
            DeviceLayer::SystemLayer.StartTimer(commissioningTimeoutSeconds * 1000, HandlePairingWindowTimeout, nullptr));
    }

    return CHIP_NO_ERROR;
}

void ClosePairingWindow()
{
    if (gPairingWindowOpen)
    {
        ChipLogProgress(AppServer, "Closing pairing window");
        gRendezvousServer.Cleanup();
    }
}

bool IsPairingWindowOpen()
{
    return gPairingWindowOpen;
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
    PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH);
#endif

    err = gRendezvousServer.Init(delegate, &gServerStorage, &gSessionIDAllocator);
    SuccessOrExit(err);

    gAdvDelegate.SetDelegate(delegate);

    err = gFabrics.Init(&gServerStorage);
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

    err = gSessions.Init(&DeviceLayer::SystemLayer, &gTransports, &gFabrics, &gMessageCounterManager);
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

        // Restore any previous fabric pairings
        VerifyOrExit(CHIP_NO_ERROR == RestoreAllFabricsFromKVS(gFabrics, gNextAvailableFabricIndex),
                     ChipLogError(AppServer, "Could not restore fabric table"));

        VerifyOrExit(CHIP_NO_ERROR == RestoreAllSessionsFromKVS(gSessions),
                     ChipLogError(AppServer, "Could not restore previous sessions"));
    }
    else
    {
#if CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART
        SuccessOrExit(err = OpenDefaultPairingWindow(ResetFabrics::kYes));
#endif
    }

// ESP32 and Mbed OS examples have a custom logic for enabling DNS-SD
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !CHIP_DEVICE_LAYER_TARGET_ESP32 && !CHIP_DEVICE_LAYER_TARGET_MBED
    app::Mdns::StartServer();
#endif

    gCallbacks.SetSessionMgr(&gSessions);

    // Register to receive unsolicited legacy ZCL messages from the exchange manager.
    err = gExchangeMgr.RegisterUnsolicitedMessageHandlerForProtocol(Protocols::TempZCL::Id, &gCallbacks);
    SuccessOrExit(err);

    // Register to receive unsolicited Service Provisioning messages from the exchange manager.
    err = gExchangeMgr.RegisterUnsolicitedMessageHandlerForProtocol(Protocols::ServiceProvisioning::Id, &gCallbacks);
    SuccessOrExit(err);

    err = gCASEServer.ListenForSessionEstablishment(&gExchangeMgr, &gTransports, &gSessions, &GetGlobalFabricTable(),
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

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
// NOTE: UDC client is located in Server.cpp because it really only makes sense
// to send UDC from a Matter device. The UDC message payload needs to include the device's
// randomly generated service name.

CHIP_ERROR SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner)
{
    ChipLogDetail(AppServer, "SendUserDirectedCommissioningRequest2");

    CHIP_ERROR err;
    char nameBuffer[chip::Mdns::kMaxInstanceNameSize + 1];
    err = app::Mdns::GetCommissionableInstanceName(nameBuffer, sizeof(nameBuffer));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to get mdns instance name error: %s", ErrorStr(err));
        return err;
    }
    ChipLogDetail(AppServer, "instanceName=%s", nameBuffer);

    // send UDC message 5 times per spec (no ACK on this message)
    for (unsigned int i = 0; i < 5; i++)
    {
        chip::System::PacketBufferHandle payloadBuf = chip::MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));
        if (payloadBuf.IsNull())
        {
            ChipLogError(AppServer, "Unable to allocate packet buffer\n");
            return CHIP_ERROR_NO_MEMORY;
        }

        err = gUDCClient.SendUDCMessage(&gTransports, std::move(payloadBuf), commissioner);
        if (err == CHIP_NO_ERROR)
        {
            ChipLogDetail(AppServer, "Send UDC request success");
        }
        else
        {
            ChipLogError(AppServer, "Send UDC request failed, err: %s\n", chip::ErrorStr(err));
        }

        sleep(1);
    }
    return err;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

CHIP_ERROR AddTestPairing()
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    FabricInfo * fabricInfo   = nullptr;
    PASESession * testSession = nullptr;
    PASESessionSerializable serializedTestSession;

    for (const FabricInfo & fabric : gFabrics)
        if (fabric.IsInitialized() && fabric.GetNodeId() == chip::kTestDeviceNodeId)
            ExitNow();

    fabricInfo = gFabrics.AssignFabricIndex(gNextAvailableFabricIndex);
    VerifyOrExit(fabricInfo != nullptr, err = CHIP_ERROR_NO_MEMORY);

    fabricInfo->SetNodeId(chip::kTestDeviceNodeId);
    gTestPairing.ToSerializable(serializedTestSession);

    testSession = chip::Platform::New<PASESession>();
    testSession->FromSerializable(serializedTestSession);
    SuccessOrExit(err = gSessions.NewPairing(Optional<PeerAddress>{ PeerAddress::Uninitialized() }, chip::kTestControllerNodeId,
                                             testSession, SecureSession::SessionRole::kResponder, gNextAvailableFabricIndex));
    ++gNextAvailableFabricIndex;

exit:
    if (testSession)
    {
        testSession->Clear();
        chip::Platform::Delete(testSession);
    }

    if (err != CHIP_NO_ERROR && fabricInfo != nullptr)
        gFabrics.ReleaseFabricIndex(gNextAvailableFabricIndex);

    return err;
}

FabricTable & GetGlobalFabricTable()
{
    return gFabrics;
}
