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
#include <app/server/Dnssd.h>
#include <app/server/EchoHandler.h>
#include <app/util/DataModelHandler.h>

#include <ble/BLEEndPoint.h>
#include <inet/IPAddress.h>
#include <inet/InetError.h>
#include <inet/InetLayer.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <lib/dnssd/Advertiser.h>
#include <lib/dnssd/ServiceNaming.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <lib/support/logging/CHIPLogging.h>
#include <messaging/ExchangeMgr.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/KeyValueStoreManager.h>
#include <protocols/secure_channel/CASEServer.h>
#include <protocols/secure_channel/MessageCounterManager.h>
#include <setup_payload/SetupPayload.h>
#include <sys/param.h>
#include <system/SystemPacketBuffer.h>
#include <system/TLVPacketBufferBackingStore.h>
#include <transport/SessionManager.h>

using chip::kMinValidFabricIndex;
using chip::RendezvousInformationFlag;
using chip::DeviceLayer::PersistedStorage::KeyValueStoreMgr;
using chip::Inet::IPAddressType;
using chip::Transport::BleListenParameters;
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

} // namespace

namespace chip {

Server Server::sServer;

CHIP_ERROR Server::Init(AppDelegate * delegate, uint16_t secureServicePort, uint16_t unsecureServicePort)
{
    mSecuredServicePort   = secureServicePort;
    mUnsecuredServicePort = unsecureServicePort;

    CHIP_ERROR err = CHIP_NO_ERROR;

    chip::Platform::MemoryInit();

    mCommissioningWindowManager.SetAppDelegate(delegate);
    mCommissioningWindowManager.SetSessionIDAllocator(&mSessionIDAllocator);
    InitDataModelHandler(&mExchangeMgr);

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
    err = DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init("chip.store");
    SuccessOrExit(err);
#elif CHIP_DEVICE_LAYER_TARGET_LINUX
    DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH);
#endif

    err = mFabrics.Init(&mServerStorage);
    SuccessOrExit(err);

    // Init transport before operations with secure session mgr.
    err = mTransports.Init(
        UdpListenParameters(&DeviceLayer::InetLayer()).SetAddressType(IPAddressType::kIPv6).SetListenPort(mSecuredServicePort)

#if INET_CONFIG_ENABLE_IPV4
            ,
        UdpListenParameters(&DeviceLayer::InetLayer()).SetAddressType(IPAddressType::kIPv4).SetListenPort(mSecuredServicePort)
#endif
#if CONFIG_NETWORK_LAYER_BLE
            ,
        BleListenParameters(DeviceLayer::ConnectivityMgr().GetBleLayer())
#endif
    );

#if CONFIG_NETWORK_LAYER_BLE
    mBleLayer = DeviceLayer::ConnectivityMgr().GetBleLayer();
#endif
    SuccessOrExit(err);

    err = mSessions.Init(&DeviceLayer::SystemLayer(), &mTransports, &mMessageCounterManager);
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
    else if ((DeviceLayer::ConnectivityMgr().IsWiFiStationProvisioned() || DeviceLayer::ConnectivityMgr().IsThreadProvisioned()) &&
             (GetFabricTable().FabricCount() != 0))
    {
        // The device is already commissioned, proactively disable BLE advertisement.
        ChipLogProgress(AppServer, "Fabric already commissioned. Disabling BLE advertisement");
        chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(false);
    }
    else
    {
#if CHIP_DEVICE_CONFIG_ENABLE_PAIRING_AUTOSTART
        GetFabricTable().DeleteAllFabrics();
        SuccessOrExit(err = mCommissioningWindowManager.OpenBasicCommissioningWindow());
#endif
    }

#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD
    app::DnssdServer::Instance().SetSecuredPort(mSecuredServicePort);
    app::DnssdServer::Instance().SetUnsecuredPort(mUnsecuredServicePort);
#endif // CHIP_DEVICE_CONFIG_ENABLE_DNSSD

    // TODO @bzbarsky-apple @cecille Move to examples
    // ESP32 and Mbed OS examples have a custom logic for enabling DNS-SD
#if CHIP_DEVICE_CONFIG_ENABLE_DNSSD && !CHIP_DEVICE_LAYER_TARGET_ESP32 && !CHIP_DEVICE_LAYER_TARGET_MBED &&                        \
    (!CHIP_DEVICE_LAYER_TARGET_AMEBA || !CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE)
    // StartServer only enables commissioning mode if device has not been commissioned
    app::DnssdServer::Instance().StartServer();
#endif

    err = mCASEServer.ListenForSessionEstablishment(&mExchangeMgr, &mTransports, chip::DeviceLayer::ConnectivityMgr().GetBleLayer(),
                                                    &mSessions, &mFabrics, &mSessionIDAllocator);
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

void Server::Shutdown()
{
    chip::Dnssd::ServiceAdvertiser::Instance().Shutdown();
    chip::app::InteractionModelEngine::GetInstance()->Shutdown();
    mExchangeMgr.Shutdown();
    mSessions.Shutdown();
    mTransports.Close();
    mCommissioningWindowManager.Shutdown();
    chip::Platform::MemoryShutdown();
}

#if CHIP_DEVICE_CONFIG_ENABLE_COMMISSIONER_DISCOVERY_CLIENT
// NOTE: UDC client is located in Server.cpp because it really only makes sense
// to send UDC from a Matter device. The UDC message payload needs to include the device's
// randomly generated service name.
CHIP_ERROR Server::SendUserDirectedCommissioningRequest(chip::Transport::PeerAddress commissioner)
{
    ChipLogDetail(AppServer, "SendUserDirectedCommissioningRequest2");

    CHIP_ERROR err;
    char nameBuffer[chip::Dnssd::Commissionable::kInstanceNameMaxLength + 1];
    err = app::DnssdServer::Instance().GetCommissionableInstanceName(nameBuffer, sizeof(nameBuffer));
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

CHIP_ERROR Server::AddTestCommissioning()
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    PASESession * testSession = nullptr;
    PASESessionSerializable serializedTestSession;

    mTestPairing.ToSerializable(serializedTestSession);

    testSession = chip::Platform::New<PASESession>();
    testSession->FromSerializable(serializedTestSession);
    SuccessOrExit(err = mSessions.NewPairing(Optional<PeerAddress>{ PeerAddress::Uninitialized() }, chip::kTestControllerNodeId,
                                             testSession, CryptoContext::SessionRole::kResponder, kMinValidFabricIndex));

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

} // namespace chip
