/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
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
#include <app/server/Server.h>
#include <crypto/CHIPCryptoPAL.h>
#include <crypto/DefaultSessionKeystore.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/support/logging/CHIPLogging.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/GroupDataProviderImpl.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <platform/CommissionableDataProvider.h>
#include <platform/DiagnosticDataProvider.h>

#include <DeviceInfoProviderImpl.h>

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#include "CommissionerMain.h"
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
#include <Rpc.h>
#endif

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "TraceDecoder.h"
#include "TraceHandlers.h"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#endif

#include <signal.h>

#include "AppMain.h"
#include "CommissionableInit.h"

#include "DeviceAttestationSe05xCredsExample.h"
#include <CHIPCryptoPALHsm_se05x_config.h>

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
// To hold SPAKE2+ verifier, discriminator, passcode
LinuxCommissionableDataProvider gCommissionableDataProvider;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

void EventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
    }
}

void Cleanup()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::DeInitTrace();
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

    // TODO(16968): Lifecycle management of storage-using components like GroupDataProvider, etc
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

int ChipLinuxAppInit(int argc, char * const argv[], OptionSet * customOptions)
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

    // Init the commissionable data provider based on command line options
    // to handle custom verifiers, discriminators, etc.
    err = chip::examples::InitCommissionableDataProvider(gCommissionableDataProvider, LinuxDeviceOptions::GetInstance());
    SuccessOrExit(err);
    DeviceLayer::SetCommissionableDataProvider(&gCommissionableDataProvider);

    err = chip::examples::InitConfigurationManager(reinterpret_cast<ConfigurationManagerImpl &>(ConfigurationMgr()),
                                                   LinuxDeviceOptions::GetInstance());
    SuccessOrExit(err);

    if (LinuxDeviceOptions::GetInstance().payload.rendezvousInformation.HasValue())
    {
        rendezvousFlags = LinuxDeviceOptions::GetInstance().payload.rendezvousInformation.Value();
    }

    err = GetPayloadContents(LinuxDeviceOptions::GetInstance().payload, rendezvousFlags);
    SuccessOrExit(err);

    ConfigurationMgr().LogDeviceConfig();

    {
        ChipLogProgress(NotSpecified, "==== Onboarding payload for Standard Commissioning Flow ====");
        PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);
    }

    {
        // For testing of manual pairing code with custom commissioning flow
        ChipLogProgress(NotSpecified, "==== Onboarding payload for Custom Commissioning Flows ====");
        err = GetPayloadContents(LinuxDeviceOptions::GetInstance().payload, rendezvousFlags);
        SuccessOrExit(err);

        LinuxDeviceOptions::GetInstance().payload.commissioningFlow = chip::CommissioningFlow::kCustom;

        PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);
    }

#if defined(PW_RPC_ENABLED)
    rpc::Init();
    ChipLogProgress(NotSpecified, "PW_RPC initialized.");
#endif // defined(PW_RPC_ENABLED)

    DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    if (LinuxDeviceOptions::GetInstance().traceStreamFilename.HasValue())
    {
        const char * traceFilename = LinuxDeviceOptions::GetInstance().traceStreamFilename.Value().c_str();
        auto traceStream           = new chip::trace::TraceStreamFile(traceFilename);
        chip::trace::AddTraceStream(traceStream);
    }
    else if (LinuxDeviceOptions::GetInstance().traceStreamToLogEnabled)
    {
        auto traceStream = new chip::trace::TraceStreamLog();
        chip::trace::AddTraceStream(traceStream);
    }

    if (LinuxDeviceOptions::GetInstance().traceStreamDecodeEnabled)
    {
        chip::trace::TraceDecoderOptions options;
        options.mEnableProtocolInteractionModelResponse = false;

        chip::trace::TraceDecoder * decoder = new chip::trace::TraceDecoder();
        decoder->SetOptions(options);
        chip::trace::AddTraceStream(decoder);
    }
    chip::trace::InitTrace();
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

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
        Cleanup();

        // End the program with non zero error code to indicate a error.
        return 1;
    }
    return 0;
}

void ChipLinuxAppMainLoop(AppMainLoopImplementation * impl)
{
    static chip::CommonCaseDeviceServerInitParams initParams;

    VerifyOrDie(initParams.InitializeStaticResourcesBeforeServerInit() == CHIP_NO_ERROR);
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

#if defined(ENABLE_CHIP_SHELL)
    Engine::Root().Init();
    std::thread shellThread([]() { Engine::Root().RunMainLoop(); });
    Shell::RegisterCommissioneeCommands();
#endif
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE || CHIP_DEVICE_ENABLE_PORT_PARAMS
    // use a different service port to make testing possible with other sample devices running on same host
    initParams.operationalServicePort        = LinuxDeviceOptions::GetInstance().securedDevicePort;
    initParams.userDirectedCommissioningPort = LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort;
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    initParams.interfaceId = LinuxDeviceOptions::GetInstance().interfaceId;

    if (LinuxDeviceOptions::GetInstance().mCSRResponseOptions.csrExistingKeyPair)
    {
        LinuxDeviceOptions::GetInstance().mCSRResponseOptions.badCsrOperationalKeyStoreForTest.Init(
            initParams.persistentStorageDelegate);
        initParams.operationalKeystore = &LinuxDeviceOptions::GetInstance().mCSRResponseOptions.badCsrOperationalKeyStoreForTest;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    static OTATestEventTriggerHandler sOtaTestEventTriggerHandler{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(LinuxDeviceOptions::GetInstance().testEventTriggerEnableKey)) ==
                CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sOtaTestEventTriggerHandler) == CHIP_NO_ERROR);
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
#endif

    // We need to set DeviceInfoProvider before Server::Init to setup the storage of DeviceInfoProvider properly.
    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    // Init ZCL Data Model and CHIP App Server
    Server::GetInstance().Init(initParams);

    // Now that the server has started and we are done with our startup logging,
    // log our discovery/onboarding information again so it's not lost in the
    // noise.
    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);

    // Initialize device attestation config
#if ENABLE_SE05X_DEVICE_ATTESTATION
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleSe05xDACProvider());
#else
    SetDeviceAttestationCredentialsProvider(LinuxDeviceOptions::GetInstance().dacProvider);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    ChipLogProgress(AppServer, "Starting commissioner");
    VerifyOrReturn(InitCommissioner(LinuxDeviceOptions::GetInstance().securedCommissionerPort,
                                    LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort) == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Started commissioner");
#if defined(ENABLE_CHIP_SHELL)
    Shell::RegisterControllerCommands();
#endif // defined(ENABLE_CHIP_SHELL)
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    ApplicationInit();

    if (impl != nullptr)
    {
        impl->RunMainLoop();
    }
    else
    {
        DeviceLayer::PlatformMgr().RunEventLoop();
    }

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    ShutdownCommissioner();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#if defined(ENABLE_CHIP_SHELL)
    shellThread.join();
#endif

    Server::GetInstance().Shutdown();

    DeviceLayer::PlatformMgr().Shutdown();

    Cleanup();
}
