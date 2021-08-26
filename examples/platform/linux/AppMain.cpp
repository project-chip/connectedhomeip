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

#include <iostream>
#include <thread>

#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <core/CHIPError.h>
#include <core/NodeId.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <support/CHIPMem.h>
#include <support/RandUtils.h>
#include <support/ScopedBuffer.h>

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#include <ControllerShellCommands.h>
#include <controller/CHIPDeviceController.h>
#include <controller/ExampleOperationalCredentialsIssuer.h>
#include <core/CHIPPersistentStorageDelegate.h>
#include <platform/KeyValueStoreManager.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#if defined(ENABLE_CHIP_SHELL)
#include <CommissioneeShellCommands.h>
#include <lib/shell/Engine.h>
#endif

#if defined(PW_RPC_ENABLED)
#include <CommonRpc.h>
#endif

#include "Options.h"

using namespace chip;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;
using namespace chip::Inet;
using namespace chip::Transport;

#if defined(ENABLE_CHIP_SHELL)
using chip::Shell::Engine;
#endif

namespace {
void EventHandler(const chip::DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == chip::DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
    }
}
} // namespace

int ChipLinuxAppInit(int argc, char ** argv)
{
    CHIP_ERROR err                                   = CHIP_NO_ERROR;
    chip::RendezvousInformationFlags rendezvousFlags = chip::RendezvousInformationFlag::kBLE;

#ifdef CONFIG_RENDEZVOUS_MODE
    rendezvousFlags = static_cast<chip::RendezvousInformationFlags>(CONFIG_RENDEZVOUS_MODE);
#endif

    err = chip::Platform::MemoryInit();
    SuccessOrExit(err);

    err = GetSetupPayload(LinuxDeviceOptions::GetInstance().payload, rendezvousFlags);
    SuccessOrExit(err);

    err = ParseArguments(argc, argv);
    SuccessOrExit(err);

    err = chip::DeviceLayer::PlatformMgr().InitChipStack();
    SuccessOrExit(err);

    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);

#if defined(PW_RPC_ENABLED)
    chip::rpc::Init();
    ChipLogProgress(NotSpecified, "PW_RPC initialized.");
#endif // defined(PW_RPC_ENABLED)

    chip::DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(nullptr); // Use default device name (CHIP-XXXX)

#if CONFIG_NETWORK_LAYER_BLE
    chip::DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(LinuxDeviceOptions::GetInstance().mBleDevice, false);
#endif

    chip::DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true);

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    if (LinuxDeviceOptions::GetInstance().mWiFi)
    {
        chip::DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

#if CHIP_ENABLE_OPENTHREAD
    if (LinuxDeviceOptions::GetInstance().mThread)
    {
        SuccessOrExit(err = chip::DeviceLayer::ThreadStackMgrImpl().InitThreadStack());
        ChipLogProgress(NotSpecified, "Thread initialized.");
    }
#endif // CHIP_ENABLE_OPENTHREAD

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogProgress(NotSpecified, "Failed to run Linux Lighting App: %s ", ErrorStr(err));
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

DeviceCommissioner gCommissioner;
MyServerStorageDelegate gServerStorage;
ExampleOperationalCredentialsIssuer gOpCredsIssuer;

CHIP_ERROR InitCommissioner()
{
    NodeId localId = chip::kPlaceholderNodeId;

    chip::Controller::CommissionerInitParams params;

    params.storageDelegate                = &gServerStorage;
    params.mDeviceAddressUpdateDelegate   = nullptr;
    params.operationalCredentialsDelegate = &gOpCredsIssuer;

    ReturnErrorOnFailure(gOpCredsIssuer.Initialize(gServerStorage));

    // use a different listen port for the commissioner.
    ReturnErrorOnFailure(gCommissioner.SetUdpListenPort(LinuxDeviceOptions::GetInstance().securedCommissionerPort));
    // No need to explicitly set the UDC port since we will use default
    ReturnErrorOnFailure(gCommissioner.SetUdcListenPort(LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort));

    chip::Platform::ScopedMemoryBuffer<uint8_t> noc;
    VerifyOrReturnError(noc.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    chip::MutableByteSpan nocSpan(noc.Get(), chip::Controller::kMaxCHIPDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> icac;
    VerifyOrReturnError(icac.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    chip::MutableByteSpan icacSpan(icac.Get(), chip::Controller::kMaxCHIPDERCertLength);

    chip::Platform::ScopedMemoryBuffer<uint8_t> rcac;
    VerifyOrReturnError(rcac.Alloc(chip::Controller::kMaxCHIPDERCertLength), CHIP_ERROR_NO_MEMORY);
    chip::MutableByteSpan rcacSpan(rcac.Get(), chip::Controller::kMaxCHIPDERCertLength);

    chip::Crypto::P256Keypair ephemeralKey;
    ReturnErrorOnFailure(ephemeralKey.Initialize());

    ReturnErrorOnFailure(
        gOpCredsIssuer.GenerateNOCChainAfterValidation(localId, 0, ephemeralKey.Pubkey(), rcacSpan, icacSpan, nocSpan));

    params.ephemeralKeypair = &ephemeralKey;
    params.controllerRCAC   = rcacSpan;
    params.controllerICAC   = icacSpan;
    params.controllerNOC    = nocSpan;

    ReturnErrorOnFailure(gCommissioner.Init(params));

    return CHIP_NO_ERROR;
}

CHIP_ERROR ShutdownCommissioner()
{
    gCommissioner.Shutdown();
    return CHIP_NO_ERROR;
}

#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

void ChipLinuxAppMainLoop()
{
#if defined(ENABLE_CHIP_SHELL)
    std::thread shellThread([]() { Engine::Root().RunMainLoop(); });
    chip::Shell::RegisterCommissioneeCommands();
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    // use a different service port to make testing possible with other sample devices running on same host
    ServerConfigParams params;
    params.securedServicePort   = LinuxDeviceOptions::GetInstance().securedDevicePort;
    params.unsecuredServicePort = LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort;
    SetServerConfig(params);
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    // Init ZCL Data Model and CHIP App Server
    InitServer();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    InitCommissioner();
#if defined(ENABLE_CHIP_SHELL)
    chip::Shell::RegisterDiscoverCommands(&gCommissioner);
#endif // defined(ENABLE_CHIP_SHELL)
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    chip::DeviceLayer::PlatformMgr().RunEventLoop();

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    ShutdownCommissioner();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#if defined(ENABLE_CHIP_SHELL)
    shellThread.join();
#endif
}
