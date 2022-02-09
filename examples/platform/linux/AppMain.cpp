/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
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

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/DeviceAttestationVerifier.h>
#include <credentials/examples/DefaultDeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/ScopedBuffer.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#include <ControllerShellCommands.h>
#include <controller/CHIPDeviceControllerFactory.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <lib/core/CHIPPersistentStorageDelegate.h>
#include <platform/KeyValueStoreManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#if defined(ENABLE_CHIP_SHELL)
#include <CommissioneeShellCommands.h>
#include <lib/shell/Engine.h>
#include <thread>
#endif

#if defined(PW_RPC_ENABLED)
#include <CommonRpc.h>
#endif

#include "AppMain.h"

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::app::Clusters;

#if defined(ENABLE_CHIP_SHELL)
using chip::Shell::Engine;
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
/*
 * The device shall check every kWiFiStartCheckTimeUsec whether Wi-Fi management
 * has been fully initialized. If after kWiFiStartCheckAttempts Wi-Fi management
 * still hasn't been initialized, the device configuration is reset, and device
 * needs to be paired again.
 */
static constexpr useconds_t kWiFiStartCheckTimeUsec = 100 * 1000; // 100 ms
static constexpr uint8_t kWiFiStartCheckAttempts    = 5;
#endif

namespace {
void EventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
    }
}
} // namespace

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
static bool EnsureWiFiIsStarted()
{
    for (int cnt = 0; cnt < kWiFiStartCheckAttempts; cnt++)
    {
        if (DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted())
        {
            return true;
        }

        usleep(kWiFiStartCheckTimeUsec);
    }

    return DeviceLayer::ConnectivityMgrImpl().IsWiFiManagementStarted();
}
#endif

int ChipLinuxAppInit(int argc, char ** argv, OptionSet * customOptions)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
#if CONFIG_NETWORK_LAYER_BLE
    RendezvousInformationFlags rendezvousFlags = RendezvousInformationFlag::kBLE;
#else  // CONFIG_NETWORK_LAYER_BLE
    RendezvousInformationFlag rendezvousFlags = RendezvousInformationFlag::kOnNetwork;
#endif // CONFIG_NETWORK_LAYER_BLE

#ifdef CONFIG_RENDEZVOUS_MODE
    rendezvousFlags = static_cast<RendezvousInformationFlags>(CONFIG_RENDEZVOUS_MODE);
#endif

    err = Platform::MemoryInit();
    SuccessOrExit(err);

    err = ParseArguments(argc, argv, customOptions);
    SuccessOrExit(err);

#ifdef CHIP_CONFIG_KVS_PATH
    if (LinuxDeviceOptions::GetInstance().KVS == nullptr)
    {
        err = DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH);
    }
    else
    {
        err = DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(LinuxDeviceOptions::GetInstance().KVS);
    }
    SuccessOrExit(err);
#endif

    err = DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    if (0 != LinuxDeviceOptions::GetInstance().payload.discriminator)
    {
        uint16_t discriminator = LinuxDeviceOptions::GetInstance().payload.discriminator;
        err                    = DeviceLayer::ConfigurationMgr().StoreSetupDiscriminator(discriminator);
        SuccessOrExit(err);
    }

    err = GetSetupPayload(LinuxDeviceOptions::GetInstance().payload, rendezvousFlags);
    SuccessOrExit(err);

    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);

#if defined(PW_RPC_ENABLED)
    rpc::Init();
    ChipLogProgress(NotSpecified, "PW_RPC initialized.");
#endif // defined(PW_RPC_ENABLED)

    DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

#if CONFIG_NETWORK_LAYER_BLE
    DeviceLayer::ConnectivityMgr().SetBLEDeviceName(nullptr); // Use default device name (CHIP-XXXX)
    DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(LinuxDeviceOptions::GetInstance().mBleDevice, false);
    DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    if (LinuxDeviceOptions::GetInstance().mWiFi)
    {
        DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
        if (!EnsureWiFiIsStarted())
        {
            ChipLogError(NotSpecified, "Wi-Fi Management taking too long to start - device configuration will be reset.");
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

#if CHIP_ENABLE_OPENTHREAD
    if (LinuxDeviceOptions::GetInstance().mThread)
    {
        SuccessOrExit(err = DeviceLayer::ThreadStackMgrImpl().InitThreadStack());
        ChipLogProgress(NotSpecified, "Thread initialized.");
    }
#endif // CHIP_ENABLE_OPENTHREAD

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Failed to init Linux App: %s ", ErrorStr(err));
        // End the program with non zero error code to indicate a error.
        return 1;
    }
    return 0;
}

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::Transport;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Messaging;
using namespace ::chip::Controller;

class MyServerStorageDelegate : public PersistentStorageDelegate
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

class MyCommissionerCallback : public CommissionerCallback
{
    void ReadyForCommissioning(uint32_t pincode, uint16_t longDiscriminator, PeerAddress peerAddress) override
    {
        CommissionerPairOnNetwork(pincode, longDiscriminator, peerAddress);
    }
};

DeviceCommissioner gCommissioner;
CommissionerDiscoveryController gCommissionerDiscoveryController;
MyCommissionerCallback gCommissionerCallback;
MyServerStorageDelegate gServerStorage;
SimpleFabricStorage gFabricStorage;
ExampleOperationalCredentialsIssuer gOpCredsIssuer;
NodeId gLocalId = kMaxOperationalNodeId;

CHIP_ERROR InitCommissioner()
{
    Controller::FactoryInitParams factoryParams;
    Controller::SetupParams params;

    ReturnErrorOnFailure(gFabricStorage.Initialize(&gServerStorage));

    factoryParams.fabricStorage = &gFabricStorage;
    // use a different listen port for the commissioner than the default used by chip-tool.
    factoryParams.listenPort              = LinuxDeviceOptions::GetInstance().securedCommissionerPort + 10;
    params.storageDelegate                = &gServerStorage;
    params.deviceAddressUpdateDelegate    = nullptr;
    params.operationalCredentialsDelegate = &gOpCredsIssuer;

    ReturnErrorOnFailure(gOpCredsIssuer.Initialize(gServerStorage));

    // No need to explicitly set the UDC port since we will use default
    ReturnErrorOnFailure(gCommissioner.SetUdcListenPort(LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort));

    // Initialize device attestation verifier
    // TODO: Replace testingRootStore with a AttestationTrustStore that has the necessary official PAA roots available
    const Credentials::AttestationTrustStore * testingRootStore = Credentials::GetTestAttestationTrustStore();
    SetDeviceAttestationVerifier(GetDefaultDACVerifier(testingRootStore));

    Platform::ScopedMemoryBuffer<uint8_t> noc;
    VerifyOrReturnError(noc.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan nocSpan(noc.Get(), Controller::kMaxCHIPDERCertLength);

    Platform::ScopedMemoryBuffer<uint8_t> icac;
    VerifyOrReturnError(icac.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan icacSpan(icac.Get(), Controller::kMaxCHIPDERCertLength);

    Platform::ScopedMemoryBuffer<uint8_t> rcac;
    VerifyOrReturnError(rcac.Alloc(Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    MutableByteSpan rcacSpan(rcac.Get(), Controller::kMaxCHIPDERCertLength);

    Crypto::P256Keypair ephemeralKey;
    ReturnErrorOnFailure(ephemeralKey.Initialize());

    ReturnErrorOnFailure(
        gOpCredsIssuer.GenerateNOCChainAfterValidation(gLocalId, 0, ephemeralKey.Pubkey(), rcacSpan, icacSpan, nocSpan));

    params.operationalKeypair = &ephemeralKey;
    params.controllerRCAC     = rcacSpan;
    params.controllerICAC     = icacSpan;
    params.controllerNOC      = nocSpan;

    auto & factory = Controller::DeviceControllerFactory::GetInstance();
    ReturnErrorOnFailure(factory.Init(factoryParams));
    ReturnErrorOnFailure(factory.SetupCommissioner(params, gCommissioner));
    gCommissionerDiscoveryController.SetUserDirectedCommissioningServer(gCommissioner.GetUserDirectedCommissioningServer());
    gCommissionerDiscoveryController.SetCommissionerCallback(&gCommissionerCallback);

    return CHIP_NO_ERROR;
}

CHIP_ERROR ShutdownCommissioner()
{
    UserDirectedCommissioningServer * udcServer = gCommissioner.GetUserDirectedCommissioningServer();
    if (udcServer != nullptr)
    {
        udcServer->SetUserConfirmationProvider(nullptr);
    }

    gCommissioner.Shutdown();
    return CHIP_NO_ERROR;
}

class PairingCommand : public Controller::DevicePairingDelegate, public Controller::DeviceAddressUpdateDelegate
{
public:
    PairingCommand() :
        mOnDeviceConnectedCallback(OnDeviceConnectedFn, this),
        mOnDeviceConnectionFailureCallback(OnDeviceConnectionFailureFn, this){};

    /////////// DevicePairingDelegate Interface /////////
    void OnStatusUpdate(Controller::DevicePairingDelegate::Status status) override;
    void OnPairingComplete(CHIP_ERROR error) override;
    void OnPairingDeleted(CHIP_ERROR error) override;
    void OnCommissioningComplete(NodeId deviceId, CHIP_ERROR error) override;

    /////////// DeviceAddressUpdateDelegate Interface /////////
    void OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR error) override;

    CHIP_ERROR UpdateNetworkAddress();

private:
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    static void OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device);
    static void OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR error);

    chip::Callback::Callback<chip::OnDeviceConnected> mOnDeviceConnectedCallback;
    chip::Callback::Callback<chip::OnDeviceConnectionFailure> mOnDeviceConnectionFailureCallback;
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
};

PairingCommand gPairingCommand;
NodeId gRemoteId = kTestDeviceNodeId;

CHIP_ERROR PairingCommand::UpdateNetworkAddress()
{
    ChipLogProgress(AppServer, "Mdns: Updating NodeId: %" PRIx64 " ...", gRemoteId);
    return gCommissioner.UpdateDevice(gRemoteId);
}

void PairingCommand::OnAddressUpdateComplete(NodeId nodeId, CHIP_ERROR err)
{
    ChipLogProgress(AppServer, "OnAddressUpdateComplete: %s", ErrorStr(err));
}

void PairingCommand::OnStatusUpdate(DevicePairingDelegate::Status status)
{
    switch (status)
    {
    case DevicePairingDelegate::Status::SecurePairingSuccess:
        ChipLogProgress(AppServer, "Secure Pairing Success");
        break;
    case DevicePairingDelegate::Status::SecurePairingFailed:
        ChipLogError(AppServer, "Secure Pairing Failed");
        break;
    }
}

void PairingCommand::OnPairingComplete(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Pairing Success");
    }
    else
    {
        ChipLogProgress(AppServer, "Pairing Failure: %s", ErrorStr(err));
        // For some devices, it may take more time to appear on the network and become discoverable
        // over DNS-SD, so don't give up on failure and restart the address update. Note that this
        // will not be repeated endlessly as each chip-tool command has a timeout (in the case of
        // the `pairing` command it equals 120s).
        // UpdateNetworkAddress();
    }
}

void PairingCommand::OnPairingDeleted(CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
        ChipLogProgress(AppServer, "Pairing Deleted Success");
    }
    else
    {
        ChipLogProgress(AppServer, "Pairing Deleted Failure: %s", ErrorStr(err));
    }
}

void PairingCommand::OnCommissioningComplete(NodeId nodeId, CHIP_ERROR err)
{
    if (err == CHIP_NO_ERROR)
    {
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        ChipLogProgress(AppServer, "Device commissioning completed with success - getting OperationalDeviceProxy");

        gCommissioner.GetConnectedDevice(nodeId, &mOnDeviceConnectedCallback, &mOnDeviceConnectionFailureCallback);
#else  // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        ChipLogProgress(AppServer, "Device commissioning completed with success");
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    }
    else
    {
        ChipLogProgress(AppServer, "Device commissioning Failure: %s", ErrorStr(err));
#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
        CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
        if (cdc != nullptr)
        {
            cdc->CommissioningFailed(err);
        }
#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED
    }
}

#if CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

void PairingCommand::OnDeviceConnectedFn(void * context, chip::OperationalDeviceProxy * device)
{
    ChipLogProgress(Controller, "OnDeviceConnectedFn");
    CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();

    if (device == nullptr)
    {
        ChipLogProgress(AppServer, "No OperationalDeviceProxy returned from OnDeviceConnectedFn");
        if (cdc != nullptr)
        {
            cdc->CommissioningFailed(CHIP_ERROR_INCORRECT_STATE);
        }
        return;
    }

    if (cdc != nullptr)
    {
        // TODO: get from DAC!
        UDCClientState * udc = cdc->GetUDCClientState();
        uint16_t vendorId    = (udc == nullptr ? 0 : udc->GetVendorId());
        uint16_t productId   = (udc == nullptr ? 0 : udc->GetProductId());
        cdc->CommissioningSucceeded(vendorId, productId, gRemoteId, device);
    }
}

void PairingCommand::OnDeviceConnectionFailureFn(void * context, PeerId peerId, CHIP_ERROR err)
{
    ChipLogProgress(Controller, "OnDeviceConnectionFailureFn - attempt to get OperationalDeviceProxy failed");
    CommissionerDiscoveryController * cdc = GetCommissionerDiscoveryController();
    {
        cdc->CommissioningFailed(err);
    }
}

#endif // CHIP_DEVICE_CONFIG_APP_PLATFORM_ENABLED

CHIP_ERROR CommissionerPairOnNetwork(uint32_t pincode, uint16_t disc, Transport::PeerAddress address)
{
    RendezvousParameters params = RendezvousParameters().SetSetupPINCode(pincode).SetDiscriminator(disc).SetPeerAddress(address);

    gCommissioner.RegisterDeviceAddressUpdateDelegate(&gPairingCommand);
    gCommissioner.RegisterPairingDelegate(&gPairingCommand);
    gCommissioner.PairDevice(gRemoteId, params);

    return CHIP_NO_ERROR;
}

CHIP_ERROR CommissionerPairUDC(uint32_t pincode, size_t index)
{
    UDCClientState * state = gCommissioner.GetUserDirectedCommissioningServer()->GetUDCClients().GetUDCClientState(index);
    if (state == nullptr)
    {
        ChipLogProgress(AppServer, "udc client[%ld] null \r\n", index);
        return CHIP_ERROR_KEY_NOT_FOUND;
    }
    else
    {
        Transport::PeerAddress peerAddress = state->GetPeerAddress();

        state->SetUDCClientProcessingState(UDCClientProcessingState::kCommissioningNode);

        return CommissionerPairOnNetwork(pincode, state->GetLongDiscriminator(), peerAddress);
    }
}

DeviceCommissioner * GetDeviceCommissioner()
{
    return &gCommissioner;
}

CommissionerDiscoveryController * GetCommissionerDiscoveryController()
{
    return &gCommissionerDiscoveryController;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

void ChipLinuxAppMainLoop()
{
#if defined(ENABLE_CHIP_SHELL)
    Engine::Root().Init();
    std::thread shellThread([]() { Engine::Root().RunMainLoop(); });
    Shell::RegisterCommissioneeCommands();
#endif
    uint16_t securePort   = CHIP_PORT;
    uint16_t unsecurePort = CHIP_UDC_PORT;

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE || CHIP_DEVICE_ENABLE_PORT_PARAMS
    // use a different service port to make testing possible with other sample devices running on same host
    securePort   = LinuxDeviceOptions::GetInstance().securedDevicePort;
    unsecurePort = LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort;
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    // Init ZCL Data Model and CHIP App Server
    Server::GetInstance().Init(nullptr, securePort, unsecurePort);

    // Now that the server has started and we are done with our startup logging,
    // log our discovery/onboarding information again so it's not lost in the
    // noise.
    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    InitCommissioner();
#if defined(ENABLE_CHIP_SHELL)
    Shell::RegisterControllerCommands();
#endif // defined(ENABLE_CHIP_SHELL)
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    ApplicationInit();

    DeviceLayer::PlatformMgr().RunEventLoop();

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    ShutdownCommissioner();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#if defined(ENABLE_CHIP_SHELL)
    shellThread.join();
#endif
}
