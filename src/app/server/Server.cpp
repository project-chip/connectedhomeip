/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include <app/server/EchoHandler.h>
#include <app/server/Mdns.h>
#include <app/server/RendezvousServer.h>
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

using chip::RendezvousInformationFlag;
using chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr;
using chip::Inet::IPAddressType;
using chip::Transport::BleListenParameters;
using chip::Transport::kMinValidFabricIndex;
using chip::Transport::PeerAddress;
using chip::Transport::UdpListenParameters;

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

void HandlePairingWindowTimeout(chip::System::Layer * aSystemLayer, void * aAppState)
{
    chip::Server * server = static_cast<chip::Server *>(aAppState);
    server->ClosePairingWindow();
}

} // namespace

namespace chip {

Server Server::sServer;

CHIP_ERROR Server::Init(AppDelegate * delegate, uint16_t secureServicePort, uint16_t unsecureServicePort)
{
    mAppDelgate           = delegate;
    mSecuredServicePort   = secureServicePort;
    mUnsecuredServicePort = unsecureServicePort;

    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Platform::MemoryInit();

    InitDataModelHandler(&mExchangeMgr);

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    err = DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init("chip.store");
    SuccessOrExit(err);
#elif CHIP_DEVICE_LAYER_TARGET_LINUX
    DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH);
#endif

    err = mRendezvousServer.Init(mAppDelgate, &mSessionIDAllocator);
    SuccessOrExit(err);

    err = mFabrics.Init(this);
    SuccessOrExit(err);

    // Init transport before operations with secure session mgr.
    err = mTransports.Init(UdpListenParameters(&DeviceLayer::InetLayer)
                               .SetAddressType(IPAddressType::kIPAddressType_IPv6)
                               .SetListenPort(mSecuredServicePort)

#if INET_CONFIG_ENABLE_IPV4
                               ,
                           UdpListenParameters(&DeviceLayer::InetLayer)
                               .SetAddressType(IPAddressType::kIPAddressType_IPv4)
                               .SetListenPort(mSecuredServicePort)
#endif
#if CONFIG_NETWORK_LAYER_BLE
                               ,
                           BleListenParameters(DeviceLayer::ConnectivityMgr().GetBleLayer())
#endif
    );

    SuccessOrExit(err);

    err = mSessions.Init(&DeviceLayer::SystemLayer, &mTransports, &mFabrics, &mMessageCounterManager);
    SuccessOrExit(err);

    err = mExchangeMgr.Init(&mSessions);
    SuccessOrExit(err);
    err = mMessageCounterManager.Init(&mExchangeMgr);
    SuccessOrExit(err);

    err = chip::app::InteractionModelEngine::GetInstance()->Init(&mExchangeMgr, nullptr);
    SuccessOrExit(err);

#if defined(CHIP_APP_USE_ECHO)
    err = InitEchoHandler(&gExchangeMgr);
    SuccessOrExit(err);
#endif

    if (isRendezvousBypassed())
    {
        ChipLogProgress(AppServer, "Rendezvous and secure pairing skipped");
        SuccessOrExit(err = AddTestCommissioning());
    }
    else if (DeviceLayer::ConnectivityMgr().IsWiFiStationProvisioned() || DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        // If the network is already provisioned, proactively disable BLE advertisement.
        ChipLogProgress(AppServer, "Network already provisioned. Disabling BLE advertisement");
        chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false);
    }
    else
    {
#if CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART
        SuccessOrExit(err = OpenBasicCommissioningWindow(ResetFabrics::kYes));
#endif
    }

#if CHIP_DEVICE_CONFIG_ENABLE_MDNS
    app::Mdns::SetSecuredPort(mSecuredServicePort);
    app::Mdns::SetUnsecuredPort(mUnsecuredServicePort);
#endif // CHIP_DEVICE_CONFIG_ENABLE_MDNS
// ESP32 and Mbed OS examples have a custom logic for enabling DNS-SD
#if CHIP_DEVICE_CONFIG_ENABLE_MDNS && !CHIP_DEVICE_LAYER_TARGET_ESP32 && !CHIP_DEVICE_LAYER_TARGET_MBED
    // StartServer only enables commissioning mode if device has not been commissioned
    app::Mdns::StartServer();
#endif

    // Register to receive unsolicited legacy ZCL messages from the exchange manager.
    err = mExchangeMgr.RegisterUnsolicitedMessageHandlerForProtocol(Protocols::TempZCL::Id, this);
    SuccessOrExit(err);

    // Register to receive unsolicited Service Provisioning messages from the exchange manager.
    err = mExchangeMgr.RegisterUnsolicitedMessageHandlerForProtocol(Protocols::ServiceProvisioning::Id, this);
    SuccessOrExit(err);

    err = mCASEServer.ListenForSessionEstablishment(&mExchangeMgr, &mTransports, &mSessions, &mFabrics, &mSessionIDAllocator);
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
    return err;
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
// NOTE: UDC client is located in Server.cpp because it really only makes sense
// to send UDC from a Matter device. The UDC message payload needs to include the device's
// randomly generated service name.
CHIP_ERROR Server::SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner)
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

    chip::System::PacketBufferHandle payloadBuf = chip::MessagePacketBuffer::NewWithData(nameBuffer, strlen(nameBuffer));
    if (payloadBuf.IsNull())
    {
        ChipLogError(AppServer, "Unable to allocate packet buffer\n");
        return CHIP_ERROR_NO_MEMORY;
    }

    err = gUDCClient.SendUDCMessage(&mTransports, std::move(payloadBuf), commissioner);
    if (err == CHIP_NO_ERROR)
    {
        ChipLogDetail(AppServer, "Send UDC request success");
    }
    else
    {
        ChipLogError(AppServer, "Send UDC request failed, err: %s\n", chip::ErrorStr(err));
    }
    return err;
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT

CHIP_ERROR Server::OpenBasicCommissioningWindow(ResetFabrics resetFabrics, uint16_t commissioningTimeoutSeconds,
                                                PairingWindowAdvertisement advertisementMode)
{
    // TODO(cecille): If this is re-called when the window is already open, what should happen?
    mDeviceDiscriminatorCache.RestoreDiscriminator();

    uint32_t pinCode;
    ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSetupPinCode(pinCode));

    RendezvousParameters params;

    params.SetSetupPINCode(pinCode);
#if CONFIG_NETWORK_LAYER_BLE
    SetBLE(advertisementMode == chip::PairingWindowAdvertisement::kBle);
    params.SetAdvertisementDelegate(this);
    if (advertisementMode == chip::PairingWindowAdvertisement::kBle)
    {
        params.SetBleLayer(DeviceLayer::ConnectivityMgr().GetBleLayer()).SetPeerAddress(Transport::PeerAddress::BLE());
    }
#endif // CONFIG_NETWORK_LAYER_BLE

    if (resetFabrics == ResetFabrics::kYes)
    {
        mFabrics.DeleteAllFabrics();
        // Only resetting gNextAvailableFabricIndex at reboot otherwise previously paired device with fabricID 0
        // can continue sending messages to accessory as next available fabric will also be 0.
        // This logic is not up to spec, will be implemented up to spec once AddOptCert is implemented.
        mFabrics.Reset();
    }

    ReturnErrorOnFailure(mRendezvousServer.WaitForPairing(
        std::move(params), kSpake2p_Iteration_Count,
        ByteSpan(reinterpret_cast<const uint8_t *>(kSpake2pKeyExchangeSalt), strlen(kSpake2pKeyExchangeSalt)), 0, &mExchangeMgr,
        &mTransports, &mSessions));

    if (commissioningTimeoutSeconds != kNoCommissioningTimeout)
    {
        ReturnErrorOnFailure(
            DeviceLayer::SystemLayer.StartTimer(commissioningTimeoutSeconds * 1000, HandlePairingWindowTimeout, this));
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Server::OpenEnhancedCommissioningWindow(uint16_t commissioningTimeoutSeconds, uint16_t discriminator,
                                                   PASEVerifier & verifier, uint32_t iterations, ByteSpan salt, uint16_t passcodeID)
{
    RendezvousParameters params;

    ReturnErrorOnFailure(mDeviceDiscriminatorCache.UpdateDiscriminator(discriminator));

    SetBLE(false);
    params.SetPASEVerifier(verifier).SetAdvertisementDelegate(this);

    ReturnErrorOnFailure(
        mRendezvousServer.WaitForPairing(std::move(params), iterations, salt, passcodeID, &mExchangeMgr, &mTransports, &mSessions));

    if (commissioningTimeoutSeconds != kNoCommissioningTimeout)
    {
        ReturnErrorOnFailure(
            DeviceLayer::SystemLayer.StartTimer(commissioningTimeoutSeconds * 1000, HandlePairingWindowTimeout, this));
    }

    return CHIP_NO_ERROR;
}

void Server::ClosePairingWindow()
{
    if (mPairingWindowOpen)
    {
        ChipLogProgress(AppServer, "Closing pairing window");
        mRendezvousServer.Cleanup();
    }
}

bool Server::IsPairingWindowOpen()
{
    return mPairingWindowOpen;
}

CHIP_ERROR Server::AddTestCommissioning()
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    PASESession * testSession = nullptr;
    PASESessionSerializable serializedTestSession;

    mTestPairing.ToSerializable(serializedTestSession);

    testSession = chip::Platform::New<PASESession>();
    testSession->FromSerializable(serializedTestSession);
    SuccessOrExit(err = mSessions.NewPairing(Optional<PeerAddress>{ PeerAddress::Uninitialized() }, chip::kTestControllerNodeId,
                                             testSession, SecureSession::SessionRole::kResponder, kMinValidFabricIndex));

exit:
    if (testSession)
    {
        testSession->Clear();
        chip::Platform::Delete(testSession);
    }

    if (err != CHIP_NO_ERROR)
    {
        mFabrics.ReleaseFabricIndex(kMinValidFabricIndex);
    }
    return err;
}

CHIP_ERROR Server::SyncGetKeyValue(const char * key, void * buffer, uint16_t & size)
{
    ChipLogDetail(AppServer, "Retrieved value from server storage.");
    return KeyValueStoreMgr().Get(key, buffer, size);
}

CHIP_ERROR Server::SyncSetKeyValue(const char * key, const void * value, uint16_t size)
{
    ChipLogDetail(AppServer, "Stored value in server storage");
    return KeyValueStoreMgr().Put(key, value, size);
}

CHIP_ERROR Server::SyncDeleteKeyValue(const char * key)
{
    ChipLogDetail(AppServer, "Delete value in server storage");
    return KeyValueStoreMgr().Delete(key);
}

CHIP_ERROR Server::StartAdvertisement()
{
    if (mIsBLE)
    {
        ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true));
    }
    if (mAppDelgate != nullptr)
    {
        mAppDelgate->OnPairingWindowOpened();
    }
    mPairingWindowOpen = true;
    return CHIP_NO_ERROR;
}

CHIP_ERROR Server::StopAdvertisement()
{
    mDeviceDiscriminatorCache.RestoreDiscriminator();

    mPairingWindowOpen = false;

    if (mIsBLE)
    {
        ReturnErrorOnFailure(chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false));
    }

    if (mAppDelgate != nullptr)
    {
        mAppDelgate->OnPairingWindowClosed();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR Server::OnMessageReceived(Messaging::ExchangeContext * exchangeContext, const PacketHeader & packetHeader,
                                     const PayloadHeader & payloadHeader, System::PacketBufferHandle && buffer)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // as soon as a client connects, assume it is connected
    VerifyOrExit(!buffer.IsNull(), ChipLogError(AppServer, "Received data but couldn't process it..."));
    HandleDataModelMessage(exchangeContext, std::move(buffer));

exit:
    return err;
}

void Server::OnResponseTimeout(Messaging::ExchangeContext * ec)
{
    ChipLogProgress(AppServer, "Failed to receive response");
    if (mAppDelgate != nullptr)
    {
        mAppDelgate->OnReceiveError();
    }
}

CHIP_ERROR Server::DeviceDiscriminatorCache::UpdateDiscriminator(uint16_t discriminator)
{
    if (!mOriginalDiscriminatorCached)
    {
        // Cache the original discriminator
        ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().GetSetupDiscriminator(mOriginalDiscriminator));
        mOriginalDiscriminatorCached = true;
    }

    return DeviceLayer::ConfigurationMgr().StoreSetupDiscriminator(discriminator);
}

CHIP_ERROR Server::DeviceDiscriminatorCache::RestoreDiscriminator()
{
    if (mOriginalDiscriminatorCached)
    {
        // Restore the original discriminator
        ReturnErrorOnFailure(DeviceLayer::ConfigurationMgr().StoreSetupDiscriminator(mOriginalDiscriminator));
        mOriginalDiscriminatorCached = false;
    }

    return CHIP_NO_ERROR;
}

} // namespace chip
