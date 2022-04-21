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
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <crypto/CHIPCryptoPAL.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/support/logging/CHIPLogging.h>

#include <credentials/GroupDataProviderImpl.h>
#include <credentials/attestation_verifier/DefaultDeviceAttestationVerifier.h>
#include <credentials/attestation_verifier/DeviceAttestationVerifier.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <platform/CommissionableDataProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/KvsPersistentStorageDelegate.h>
#include <platform/TestOnlyCommissionableDataProvider.h>

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
#include <CommonRpc.h>
#endif

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "TraceHandlers.h"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

#include <signal.h>

#include "AppMain.h"
#include "LinuxCommissionableDataProvider.h"

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

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
chip::trace::TraceStream * gTraceStream = nullptr;
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

void EventHandler(const DeviceLayer::ChipDeviceEvent * event, intptr_t arg)
{
    (void) arg;
    if (event->Type == DeviceLayer::DeviceEventType::kCHIPoBLEConnectionEstablished)
    {
        ChipLogProgress(DeviceLayer, "Receive kCHIPoBLEConnectionEstablished");
    }
}

void OnSignalHandler(int signum)
{
    ChipLogDetail(DeviceLayer, "Caught signal %d", signum);

    // The BootReason attribute SHALL indicate the reason for the Node’s most recent boot, the real usecase
    // for this attribute is embedded system. In Linux simulation, we use different signals to tell the current
    // running process to terminate with different reasons.
    BootReasonType bootReason = BootReasonType::kUnspecified;
    switch (signum)
    {
    case SIGVTALRM:
        bootReason = BootReasonType::kPowerOnReboot;
        break;
    case SIGALRM:
        bootReason = BootReasonType::kBrownOutReset;
        break;
    case SIGILL:
        bootReason = BootReasonType::kSoftwareWatchdogReset;
        break;
    case SIGTRAP:
        bootReason = BootReasonType::kHardwareWatchdogReset;
        break;
    case SIGIO:
        bootReason = BootReasonType::kSoftwareUpdateCompleted;
        break;
    case SIGINT:
        bootReason = BootReasonType::kSoftwareReset;
        break;
    default:
        IgnoreUnusedVariable(bootReason);
        ChipLogError(NotSpecified, "Unhandled signal: Should never happens");
        chipDie();
        break;
    }

    Server::GetInstance().DispatchShutDownAndStopEventLoop();
}

void SetupSignalHandlers()
{
    // sigaction is not used here because Tsan interceptors seems to
    // never dispatch the signals on darwin.
    signal(SIGALRM, OnSignalHandler);
    signal(SIGVTALRM, OnSignalHandler);
    signal(SIGILL, OnSignalHandler);
    signal(SIGTRAP, OnSignalHandler);
    signal(SIGTERM, OnSignalHandler);
    signal(SIGIO, OnSignalHandler);
    signal(SIGINT, OnSignalHandler);
}

CHIP_ERROR InitCommissionableDataProvider(LinuxCommissionableDataProvider & provider, LinuxDeviceOptions & options)
{
    chip::Optional<uint32_t> setupPasscode;

    if (options.payload.setUpPINCode != 0)
    {
        setupPasscode.SetValue(options.payload.setUpPINCode);
    }
    else if (!options.spake2pVerifier.HasValue())
    {
        uint32_t defaultTestPasscode = 0;
        chip::DeviceLayer::TestOnlyCommissionableDataProvider TestOnlyCommissionableDataProvider;
        VerifyOrDie(TestOnlyCommissionableDataProvider.GetSetupPasscode(defaultTestPasscode) == CHIP_NO_ERROR);

        ChipLogError(Support,
                     "*** WARNING: Using temporary passcode %u due to no neither --passcode or --spake2p-verifier-base64 "
                     "given on command line. This is temporary and will disappear. Please update your scripts "
                     "to explicitly configure onboarding credentials. ***",
                     static_cast<unsigned>(defaultTestPasscode));
        setupPasscode.SetValue(defaultTestPasscode);
        options.payload.setUpPINCode = defaultTestPasscode;
    }
    else
    {
        // Passcode is 0, so will be ignored, and verifier will take over. Onboarding payload
        // printed for debug will be invalid, but if the onboarding payload had been given
        // properly to the commissioner later, PASE will succeed.
    }

    if (options.discriminator.HasValue())
    {
        options.payload.discriminator = options.discriminator.Value();
    }
    else
    {
        uint16_t defaultTestDiscriminator = 0;
        chip::DeviceLayer::TestOnlyCommissionableDataProvider TestOnlyCommissionableDataProvider;
        VerifyOrDie(TestOnlyCommissionableDataProvider.GetSetupDiscriminator(defaultTestDiscriminator) == CHIP_NO_ERROR);

        ChipLogError(Support,
                     "*** WARNING: Using temporary test discriminator %u due to --discriminator not "
                     "given on command line. This is temporary and will disappear. Please update your scripts "
                     "to explicitly configure discriminator. ***",
                     static_cast<unsigned>(defaultTestDiscriminator));
        options.payload.discriminator = defaultTestDiscriminator;
    }

    // Default to minimum PBKDF iterations
    uint32_t spake2pIterationCount = chip::Crypto::kSpake2p_Min_PBKDF_Iterations;
    if (options.spake2pIterations != 0)
    {
        spake2pIterationCount = options.spake2pIterations;
    }
    ChipLogError(Support, "PASE PBKDF iterations set to %u", static_cast<unsigned>(spake2pIterationCount));

    return provider.Init(options.spake2pVerifier, options.spake2pSalt, spake2pIterationCount, setupPasscode,
                         options.payload.discriminator);
}

CHIP_ERROR InitConfigurationManager(ConfigurationManagerImpl & configManager, LinuxDeviceOptions & options)
{
    if (options.payload.vendorID != 0)
    {
        configManager.StoreVendorId(options.payload.vendorID);
    }

    if (options.payload.productID != 0)
    {
        configManager.StoreProductId(options.payload.productID);
    }

    return CHIP_NO_ERROR;
}

void Cleanup()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    if (gTraceStream != nullptr)
    {
        delete gTraceStream;
        gTraceStream = nullptr;
    }
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

    // Init the commissionable data provider based on command line options
    // to handle custom verifiers, discriminators, etc.
    err = InitCommissionableDataProvider(gCommissionableDataProvider, LinuxDeviceOptions::GetInstance());
    SuccessOrExit(err);
    DeviceLayer::SetCommissionableDataProvider(&gCommissionableDataProvider);

    err = InitConfigurationManager(reinterpret_cast<ConfigurationManagerImpl &>(ConfigurationMgr()),
                                   LinuxDeviceOptions::GetInstance());
    SuccessOrExit(err);

    err = GetSetupPayload(LinuxDeviceOptions::GetInstance().payload, rendezvousFlags);
    SuccessOrExit(err);

    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);

    // For testing of manual pairing code with custom commissioning flow
    err = GetSetupPayload(LinuxDeviceOptions::GetInstance().payload, rendezvousFlags);
    SuccessOrExit(err);

    LinuxDeviceOptions::GetInstance().payload.commissioningFlow = chip::CommissioningFlow::kCustom;

    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);

#if defined(PW_RPC_ENABLED)
    rpc::Init();
    ChipLogProgress(NotSpecified, "PW_RPC initialized.");
#endif // defined(PW_RPC_ENABLED)

    DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0);

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    if (LinuxDeviceOptions::GetInstance().traceStreamFilename.HasValue())
    {
        const char * traceFilename = LinuxDeviceOptions::GetInstance().traceStreamFilename.Value().c_str();
        gTraceStream               = new chip::trace::TraceStreamFile(traceFilename);
    }
    else if (LinuxDeviceOptions::GetInstance().traceStreamToLogEnabled)
    {
        gTraceStream = new chip::trace::TraceStreamLog();
    }
    chip::trace::InitTrace();
    if (gTraceStream != nullptr)
    {
        chip::trace::SetTraceStream(gTraceStream);
    }
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

void ChipLinuxAppMainLoop(DeviceAttestationCredentialsProvider * dacProvider)
{
    static chip::CommonCaseDeviceServerInitParams initParams;
    VerifyOrDie(initParams.InitializeStaticResourcesBeforeServerInit() == CHIP_NO_ERROR);

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
    ;
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    initParams.interfaceId = LinuxDeviceOptions::GetInstance().interfaceId;

    // Init ZCL Data Model and CHIP App Server
    Server::GetInstance().Init(initParams);

    // Now that the server has started and we are done with our startup logging,
    // log our discovery/onboarding information again so it's not lost in the
    // noise.
    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(dacProvider == nullptr ? Examples::GetExampleDACProvider() : dacProvider);

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    ChipLogProgress(AppServer, "Starting commissioner");
    VerifyOrReturn(InitCommissioner(LinuxDeviceOptions::GetInstance().securedCommissionerPort + 10,
                                    LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort) == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Started commissioner");
#if defined(ENABLE_CHIP_SHELL)
    Shell::RegisterControllerCommands();
#endif // defined(ENABLE_CHIP_SHELL)
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    SetupSignalHandlers();

    ApplicationInit();

    DeviceLayer::PlatformMgr().RunEventLoop();

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
