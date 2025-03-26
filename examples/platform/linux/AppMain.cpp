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

#include <app/InteractionModelEngine.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/endpoint-config-api.h>
#include <crypto/CHIPCryptoPAL.h>
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/CHIPError.h>
#include <lib/core/NodeId.h>
#include <lib/core/Optional.h>
#include <lib/support/logging/CHIPLogging.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <credentials/DeviceAttestationCredsProvider.h>

#include <lib/support/CHIPMem.h>
#include <lib/support/ScopedBuffer.h>
#include <lib/support/TestGroupData.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <platform/CommissionableDataProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/RuntimeOptionsProvider.h>

#include <DeviceInfoProviderImpl.h>

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
#include "CommissionerMain.h"
#include <ControllerShellCommands.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#if defined(ENABLE_CHIP_SHELL)
#include <CommissioneeShellCommands.h>
#include <lib/shell/Engine.h> // nogncheck
#include <thread>
#endif

#if defined(PW_RPC_ENABLED)
#include <Rpc.h>
#endif

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include "TraceDecoder.h"
#include "TraceHandlers.h"
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

#if ENABLE_TRACING
#include <TracingCommandLineArgument.h> // nogncheck
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_DIAGNOSTIC_TRIGGER
#include <app/clusters/software-diagnostics-server/SoftwareDiagnosticsTestEventTriggerHandler.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_DIAGNOSTIC_TRIGGER
#include <app/clusters/wifi-network-diagnostics-server/WiFiDiagnosticsTestEventTriggerHandler.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/OTATestEventTriggerHandler.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_SMOKE_CO_TRIGGER
#include <app/clusters/smoke-co-alarm-server/SmokeCOTestEventTriggerHandler.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_BOOLEAN_STATE_CONFIGURATION_TRIGGER
#include <app/clusters/boolean-state-configuration-server/BooleanStateConfigurationTestEventTriggerHandler.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_ENERGY_EVSE_TRIGGER
#include <app/clusters/energy-evse-server/EnergyEvseTestEventTriggerHandler.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_ENERGY_REPORTING_TRIGGER
#include <app/clusters/electrical-energy-measurement-server/EnergyReportingTestEventTriggerHandler.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WATER_HEATER_MANAGEMENT_TRIGGER
#include <app/clusters/water-heater-management-server/WaterHeaterManagementTestEventTriggerHandler.h>
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_DEVICE_ENERGY_MANAGEMENT_TRIGGER
#include <app/clusters/device-energy-management-server/DeviceEnergyManagementTestEventTriggerHandler.h>
#endif
#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDManager.h> // nogncheck
#endif
#include <app/TestEventTriggerDelegate.h>

#include <signal.h>

#include "AppMain.h"
#include "CommissionableInit.h"

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
#include "ExampleAccessRestrictionProvider.h"
#endif

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
#include <app/server/TermsAndConditionsManager.h> // nogncheck
#endif

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#include <platform/Darwin/NetworkCommissioningDriver.h>
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <platform/Darwin/WiFi/NetworkCommissioningWiFiDriver.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

#if CHIP_DEVICE_LAYER_TARGET_LINUX
#include <platform/Linux/NetworkCommissioningDriver.h>
#endif // CHIP_DEVICE_LAYER_TARGET_LINUX

using namespace chip;
using namespace chip::ArgParser;
using namespace chip::Credentials;
using namespace chip::DeviceLayer;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::app::Clusters;
using namespace chip::Access;

// Network comissioning implementation
namespace {
// If secondaryNetworkCommissioningEndpoint has a value and both Thread and WiFi
// are enabled, we put the WiFi network commissioning cluster on
// secondaryNetworkCommissioningEndpoint.
Optional<EndpointId> sSecondaryNetworkCommissioningEndpoint;

#if CHIP_DEVICE_LAYER_TARGET_LINUX
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define CHIP_APP_MAIN_HAS_THREAD_DRIVER 1
DeviceLayer::NetworkCommissioning::LinuxThreadDriver sThreadDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#define CHIP_APP_MAIN_HAS_WIFI_DRIVER 1
DeviceLayer::NetworkCommissioning::LinuxWiFiDriver sWiFiDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#define CHIP_APP_MAIN_HAS_ETHERNET_DRIVER 1
DeviceLayer::NetworkCommissioning::LinuxEthernetDriver sEthernetDriver;
#endif // CHIP_DEVICE_LAYER_TARGET_LINUX

#if CHIP_DEVICE_LAYER_TARGET_DARWIN
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#define CHIP_APP_MAIN_HAS_WIFI_DRIVER 1
DeviceLayer::NetworkCommissioning::DarwinWiFiDriver sWiFiDriver;
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI

#define CHIP_APP_MAIN_HAS_ETHERNET_DRIVER 1
DeviceLayer::NetworkCommissioning::DarwinEthernetDriver sEthernetDriver;
#endif // CHIP_DEVICE_LAYER_TARGET_DARWIN

#ifndef CHIP_APP_MAIN_HAS_THREAD_DRIVER
#define CHIP_APP_MAIN_HAS_THREAD_DRIVER 0
#endif // CHIP_APP_MAIN_HAS_THREAD_DRIVER

#ifndef CHIP_APP_MAIN_HAS_WIFI_DRIVER
#define CHIP_APP_MAIN_HAS_WIFI_DRIVER 0
#endif // CHIP_APP_MAIN_HAS_WIFI_DRIVER

#ifndef CHIP_APP_MAIN_HAS_ETHERNET_DRIVER
#define CHIP_APP_MAIN_HAS_ETHERNET_DRIVER 0
#endif // CHIP_APP_MAIN_HAS_ETHERNET_DRIVER

#if CHIP_APP_MAIN_HAS_THREAD_DRIVER
app::Clusters::NetworkCommissioning::Instance sThreadNetworkCommissioningInstance(kRootEndpointId, &sThreadDriver);
#endif // CHIP_APP_MAIN_HAS_THREAD_DRIVER

#if CHIP_APP_MAIN_HAS_WIFI_DRIVER
// The WiFi network commissioning instance cannot be constructed until we know
// whether we have an sSecondaryNetworkCommissioningEndpoint.
Optional<app::Clusters::NetworkCommissioning::Instance> sWiFiNetworkCommissioningInstance;
#endif // CHIP_APP_MAIN_HAS_WIFI_DRIVER

#if CHIP_APP_MAIN_HAS_ETHERNET_DRIVER
app::Clusters::NetworkCommissioning::Instance sEthernetNetworkCommissioningInstance(kRootEndpointId, &sEthernetDriver);
#endif // CHIP_APP_MAIN_HAS_ETHERNET_DRIVER

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
auto exampleAccessRestrictionProvider = std::make_unique<ExampleAccessRestrictionProvider>();
#endif

void EnableThreadNetworkCommissioning()
{
#if CHIP_APP_MAIN_HAS_THREAD_DRIVER
    sThreadNetworkCommissioningInstance.Init();
#endif // CHIP_APP_MAIN_HAS_THREAD_DRIVER
}

void EnableWiFiNetworkCommissioning(EndpointId endpoint)
{
#if CHIP_APP_MAIN_HAS_WIFI_DRIVER
    sWiFiNetworkCommissioningInstance.Emplace(endpoint, &sWiFiDriver);
    sWiFiNetworkCommissioningInstance.Value().Init();
#endif // CHIP_APP_MAIN_HAS_WIFI_DRIVER
}

void InitNetworkCommissioning()
{
    if (sSecondaryNetworkCommissioningEndpoint.HasValue())
    {
        // Enable secondary endpoint only when we need it, this should be applied to all platforms.
        emberAfEndpointEnableDisable(sSecondaryNetworkCommissioningEndpoint.Value(), false);
    }

    bool isThreadEnabled = false;
#if CHIP_APP_MAIN_HAS_THREAD_DRIVER
    isThreadEnabled = LinuxDeviceOptions::GetInstance().mThread;
#endif // CHIP_APP_MAIN_HAS_THREAD_DRIVER

    bool isWiFiEnabled = false;
#if CHIP_APP_MAIN_HAS_WIFI_DRIVER
    isWiFiEnabled = LinuxDeviceOptions::GetInstance().mWiFi;

    // On Linux, command-line indicates whether Wi-Fi is supported since determining it from
    // the OS level is not easily portable.
#if CHIP_DEVICE_LAYER_TARGET_LINUX
    sWiFiDriver.Set5gSupport(LinuxDeviceOptions::GetInstance().wifiSupports5g);
#endif // CHIP_DEVICE_LAYER_TARGET_LINUX

#endif // CHIP_APP_MAIN_HAS_WIFI_DRIVER

    if (isThreadEnabled && isWiFiEnabled)
    {
        if (sSecondaryNetworkCommissioningEndpoint.HasValue())
        {
            EnableThreadNetworkCommissioning();
            EnableWiFiNetworkCommissioning(sSecondaryNetworkCommissioningEndpoint.Value());
            // Only enable secondary endpoint for network commissioning cluster when both WiFi and Thread are enabled.
            emberAfEndpointEnableDisable(sSecondaryNetworkCommissioningEndpoint.Value(), true);
        }
        else
        {
            // Just use the Thread one.
            EnableThreadNetworkCommissioning();
        }
    }
    else if (isThreadEnabled)
    {
        EnableThreadNetworkCommissioning();
    }
    else if (isWiFiEnabled)
    {
        EnableWiFiNetworkCommissioning(kRootEndpointId);
    }
    else
    {
#if CHIP_APP_MAIN_HAS_ETHERNET_DRIVER
        sEthernetNetworkCommissioningInstance.Init();
#endif // CHIP_APP_MAIN_HAS_ETHERNET_DRIVER
    }
}

} // anonymous namespace

#if defined(ENABLE_CHIP_SHELL)
using chip::Shell::Engine;
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WPA && CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
/*
 * The device shall check every kWiFiStartCheckTimeUsec whether Wi-Fi management
 * has been fully initialized. If after kWiFiStartCheckAttempts Wi-Fi management
 * still hasn't been initialized, the device configuration is reset, and device
 * needs to be paired again.
 */
static constexpr useconds_t kWiFiStartCheckTimeUsec = WIFI_START_CHECK_TIME_USEC;
static constexpr uint8_t kWiFiStartCheckAttempts    = WIFI_START_CHECK_ATTEMPTS;
#endif

namespace {
AppMainLoopImplementation * gMainLoopImplementation = nullptr;

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
    else if ((event->Type == chip::DeviceLayer::DeviceEventType::kInternetConnectivityChange))
    {
        // Restart the server on connectivity change
        app::DnssdServer::Instance().StartServer();
    }
}

void StopMainEventLoop()
{
    if (gMainLoopImplementation != nullptr)
    {
        gMainLoopImplementation->SignalSafeStopMainLoop();
    }
    else
    {
        Server::GetInstance().GenerateShutDownEvent();
        SystemLayer().ScheduleLambda([]() { PlatformMgr().StopEventLoopTask(); });
    }
}

void Cleanup()
{
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    chip::trace::DeInitTrace();
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

    // TODO(16968): Lifecycle management of storage-using components like GroupDataProvider, etc
}

void StopSignalHandler(int /* signal */)
{
#if defined(ENABLE_CHIP_SHELL)
    Engine::Root().StopMainLoop();
#endif
    StopMainEventLoop();
}

} // namespace

#if CHIP_DEVICE_CONFIG_ENABLE_WPA && CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
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

class SampleTestEventTriggerHandler : public TestEventTriggerHandler
{
    /// NOTE: If you copy this for NON-STANDARD CLUSTERS OR USAGES, please use the reserved range FFFF_FFFF_<VID_HEX>_xxxx for your
    /// trigger codes. NOTE: Standard codes are <CLUSTER_ID_HEX>_xxxx_xxxx_xxxx.
    static constexpr uint64_t kSampleTestEventTriggerAlwaysSuccess = static_cast<uint64_t>(0xFFFF'FFFF'FFF1'0000ull);

public:
    CHIP_ERROR HandleEventTrigger(uint64_t eventTrigger) override
    {
        ChipLogProgress(Support, "Saw TestEventTrigger: " ChipLogFormatX64, ChipLogValueX64(eventTrigger));

        if (eventTrigger == kSampleTestEventTriggerAlwaysSuccess)
        {
            // Do nothing, successfully
            ChipLogProgress(Support, "Handling \"Always success\" internal test event");
            return CHIP_NO_ERROR;
        }

        return CHIP_ERROR_INVALID_ARGUMENT;
    }
};

int ChipLinuxAppInit(int argc, char * const argv[], OptionSet * customOptions,
                     const Optional<EndpointId> secondaryNetworkCommissioningEndpoint)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
#if CONFIG_NETWORK_LAYER_BLE
    RendezvousInformationFlags rendezvousFlags = RendezvousInformationFlag::kBLE;
#else  // CONFIG_NETWORK_LAYER_BLE
    RendezvousInformationFlags rendezvousFlags = RendezvousInformationFlag::kOnNetwork;
#endif // CONFIG_NETWORK_LAYER_BLE

#ifdef CONFIG_RENDEZVOUS_MODE
    rendezvousFlags = static_cast<RendezvousInformationFlags>(CONFIG_RENDEZVOUS_MODE);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    rendezvousFlags.Set(RendezvousInformationFlag::kWiFiPAF);
#endif

    err = Platform::MemoryInit();
    SuccessOrExit(err);

    err = ParseArguments(argc, argv, customOptions);
    SuccessOrExit(err);

    sSecondaryNetworkCommissioningEndpoint = secondaryNetworkCommissioningEndpoint;

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

#if defined(ENABLE_CHIP_SHELL)
    /* Block SIGINT and SIGTERM. Other threads created by the main thread
     * will inherit the signal mask. Then we can explicitly unblock signals
     * in the shell thread to handle them, so the read(stdin) call can be
     * interrupted by a signal. */
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &set, nullptr);
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

#if defined(PW_RPC_ENABLED)
    rpc::Init(LinuxDeviceOptions::GetInstance().rpcServerPort);
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

#if CHIP_DEVICE_CONFIG_ENABLE_WPA && CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    if (LinuxDeviceOptions::GetInstance().mWiFi)
    {
        // Start WiFi management in Concurrent mode
        DeviceLayer::ConnectivityMgrImpl().StartWiFiManagement();
        if (!EnsureWiFiIsStarted())
        {
            ChipLogError(NotSpecified, "Wi-Fi Management taking too long to start - device configuration will be reset.");
        }
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA
#if CHIP_DEVICE_CONFIG_ENABLE_WPA && CHIP_DEVICE_CONFIG_ENABLE_WIFIPAF
    ChipLogProgress(NotSpecified, "WiFi-PAF: initialzing");
    if (LinuxDeviceOptions::GetInstance().mWiFi)
    {
        if (EnsureWiFiIsStarted())
        {
            ChipLogProgress(NotSpecified, "Wi-Fi Management started");
            DeviceLayer::ConnectivityManager::WiFiPAFAdvertiseParam args;
            args.enable  = LinuxDeviceOptions::GetInstance().mWiFiPAF;
            args.ExtCmds = LinuxDeviceOptions::GetInstance().mWiFiPAFExtCmds;
            DeviceLayer::ConnectivityMgr().SetWiFiPAFAdvertisingEnabled(args);
        }
    }
#endif

#if CHIP_ENABLE_OPENTHREAD
    if (LinuxDeviceOptions::GetInstance().mThread)
    {
        SuccessOrExit(err = DeviceLayer::ThreadStackMgrImpl().InitThreadStack());
        ChipLogProgress(NotSpecified, "Thread initialized.");
    }
#endif // CHIP_ENABLE_OPENTHREAD

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    if (LinuxDeviceOptions::GetInstance().icdActiveModeDurationMs.HasValue() ||
        LinuxDeviceOptions::GetInstance().icdIdleModeDurationMs.HasValue())
    {
        err = Server::GetInstance().GetICDManager().SetModeDurations(LinuxDeviceOptions::GetInstance().icdActiveModeDurationMs,
                                                                     LinuxDeviceOptions::GetInstance().icdIdleModeDurationMs);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(NotSpecified, "Invalid arguments to set ICD mode durations");
            SuccessOrExit(err);
        }
    }
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

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
    gMainLoopImplementation = impl;

    static chip::CommonCaseDeviceServerInitParams initParams;
    VerifyOrDie(initParams.InitializeStaticResourcesBeforeServerInit() == CHIP_NO_ERROR);
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    if (LinuxDeviceOptions::GetInstance().tcVersion.HasValue() && LinuxDeviceOptions::GetInstance().tcRequired.HasValue())
    {
        uint16_t version  = LinuxDeviceOptions::GetInstance().tcVersion.Value();
        uint16_t required = LinuxDeviceOptions::GetInstance().tcRequired.Value();
        Optional<app::TermsAndConditions> requiredAcknowledgements(app::TermsAndConditions(required, version));
        app::TermsAndConditionsManager::GetInstance()->Init(initParams.persistentStorageDelegate, requiredAcknowledgements);
    }
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED

#if defined(ENABLE_CHIP_SHELL)
    Engine::Root().Init();
    Shell::RegisterCommissioneeCommands();
    std::thread shellThread([]() {
        sigset_t set;
        sigemptyset(&set);
        sigaddset(&set, SIGINT);
        sigaddset(&set, SIGTERM);
        // Unblock SIGINT and SIGTERM, so that the shell thread can handle
        // them - we need read() call to be interrupted.
        pthread_sigmask(SIG_UNBLOCK, &set, nullptr);
        Engine::Root().RunMainLoop();
        StopMainEventLoop();
    });
#endif
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE || CHIP_DEVICE_ENABLE_PORT_PARAMS
    // use a different service port to make testing possible with other sample devices running on same host
    initParams.operationalServicePort        = LinuxDeviceOptions::GetInstance().securedDevicePort;
    initParams.userDirectedCommissioningPort = LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort;
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#if ENABLE_TRACING
    chip::CommandLineApp::TracingSetup tracing_setup;

    for (const auto & trace_destination : LinuxDeviceOptions::GetInstance().traceTo)
    {
        tracing_setup.EnableTracingFor(trace_destination.c_str());
    }
#endif

    initParams.interfaceId = LinuxDeviceOptions::GetInstance().interfaceId;

    if (LinuxDeviceOptions::GetInstance().mCSRResponseOptions.csrExistingKeyPair)
    {
        LinuxDeviceOptions::GetInstance().mCSRResponseOptions.badCsrOperationalKeyStoreForTest.Init(
            initParams.persistentStorageDelegate);
        initParams.operationalKeystore = &LinuxDeviceOptions::GetInstance().mCSRResponseOptions.badCsrOperationalKeyStoreForTest;
    }

    // For general testing of TestEventTrigger, we have a common "core" event trigger delegate.
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate;
    static SampleTestEventTriggerHandler sTestEventTriggerHandler;
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(LinuxDeviceOptions::GetInstance().testEventTriggerEnableKey)) ==
                CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sTestEventTriggerHandler) == CHIP_NO_ERROR);

#if CHIP_DEVICE_CONFIG_ENABLE_SOFTWARE_DIAGNOSTIC_TRIGGER
    static SoftwareDiagnosticsTestEventTriggerHandler sSoftwareDiagnosticsTestEventTriggerHandler;
    sTestEventTriggerDelegate.AddHandler(&sSoftwareDiagnosticsTestEventTriggerHandler);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_DIAGNOSTIC_TRIGGER
    static WiFiDiagnosticsTestEventTriggerHandler sWiFiDiagnosticsTestEventTriggerHandler;
    sTestEventTriggerDelegate.AddHandler(&sWiFiDiagnosticsTestEventTriggerHandler);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    // We want to allow triggering OTA queries if OTA requestor is enabled
    static OTATestEventTriggerHandler sOtaTestEventTriggerHandler;
    sTestEventTriggerDelegate.AddHandler(&sOtaTestEventTriggerHandler);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_SMOKE_CO_TRIGGER
    static SmokeCOTestEventTriggerHandler sSmokeCOTestEventTriggerHandler;
    sTestEventTriggerDelegate.AddHandler(&sSmokeCOTestEventTriggerHandler);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_BOOLEAN_STATE_CONFIGURATION_TRIGGER
    static BooleanStateConfigurationTestEventTriggerHandler sBooleanStateConfigurationTestEventTriggerHandler;
    sTestEventTriggerDelegate.AddHandler(&sBooleanStateConfigurationTestEventTriggerHandler);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_ENERGY_EVSE_TRIGGER
    static EnergyEvseTestEventTriggerHandler sEnergyEvseTestEventTriggerHandler;
    sTestEventTriggerDelegate.AddHandler(&sEnergyEvseTestEventTriggerHandler);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_ENERGY_REPORTING_TRIGGER
    static EnergyReportingTestEventTriggerHandler sEnergyReportingTestEventTriggerHandler;
    sTestEventTriggerDelegate.AddHandler(&sEnergyReportingTestEventTriggerHandler);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WATER_HEATER_MANAGEMENT_TRIGGER
    static WaterHeaterManagementTestEventTriggerHandler sWaterHeaterManagementTestEventTriggerHandler;
    sTestEventTriggerDelegate.AddHandler(&sWaterHeaterManagementTestEventTriggerHandler);
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_DEVICE_ENERGY_MANAGEMENT_TRIGGER
    static DeviceEnergyManagementTestEventTriggerHandler sDeviceEnergyManagementTestEventTriggerHandler;
    sTestEventTriggerDelegate.AddHandler(&sDeviceEnergyManagementTestEventTriggerHandler);
#endif
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    sTestEventTriggerDelegate.AddHandler(&Server::GetInstance().GetICDManager());
#endif

    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;

    // We need to set DeviceInfoProvider before Server::Init to setup the storage of DeviceInfoProvider properly.
    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    chip::app::RuntimeOptionsProvider::Instance().SetSimulateNoInternalTime(
        LinuxDeviceOptions::GetInstance().mSimulateNoInternalTime);

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    initParams.accessRestrictionProvider = exampleAccessRestrictionProvider.get();
#endif

    // Init ZCL Data Model and CHIP App Server
    Server::GetInstance().Init(initParams);

#if CHIP_CONFIG_USE_ACCESS_RESTRICTIONS
    if (LinuxDeviceOptions::GetInstance().commissioningArlEntries.HasValue())
    {
        exampleAccessRestrictionProvider->SetCommissioningEntries(
            LinuxDeviceOptions::GetInstance().commissioningArlEntries.Value());
    }

    if (LinuxDeviceOptions::GetInstance().arlEntries.HasValue())
    {
        // This example use of the ARL feature proactively installs the provided entries on fabric index 1
        exampleAccessRestrictionProvider->SetEntries(1, LinuxDeviceOptions::GetInstance().arlEntries.Value());
    }
#endif

#if CONFIG_BUILD_FOR_HOST_UNIT_TEST
    // Set ReadHandler Capacity for Subscriptions
    chip::app::InteractionModelEngine::GetInstance()->SetHandlerCapacityForSubscriptions(
        LinuxDeviceOptions::GetInstance().subscriptionCapacity);
    chip::app::InteractionModelEngine::GetInstance()->SetForceHandlerQuota(true);
#if CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
    // Set subscription time resumption retry interval seconds
    chip::app::InteractionModelEngine::GetInstance()->SetSubscriptionTimeoutResumptionRetryIntervalSeconds(
        LinuxDeviceOptions::GetInstance().subscriptionResumptionRetryIntervalSec);
#endif // CHIP_CONFIG_PERSIST_SUBSCRIPTIONS && CHIP_CONFIG_SUBSCRIPTION_TIMEOUT_RESUMPTION
#endif // CONFIG_BUILD_FOR_HOST_UNIT_TEST

    // Now that the server has started and we are done with our startup logging,
    // log our discovery/onboarding information again so it's not lost in the
    // noise.
    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(LinuxDeviceOptions::GetInstance().payload);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(LinuxDeviceOptions::GetInstance().dacProvider);

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    ChipLogProgress(AppServer, "Starting commissioner");
    VerifyOrReturn(InitCommissioner(LinuxDeviceOptions::GetInstance().securedCommissionerPort,
                                    LinuxDeviceOptions::GetInstance().unsecuredCommissionerPort,
                                    LinuxDeviceOptions::GetInstance().commissionerFabricId) == CHIP_NO_ERROR);
    ChipLogProgress(AppServer, "Started commissioner");
#if defined(ENABLE_CHIP_SHELL)
    Shell::RegisterControllerCommands();
#endif // defined(ENABLE_CHIP_SHELL)
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

    InitNetworkCommissioning();

    ApplicationInit();

    // NOTE: For some reason, on Darwin, the signal handler is not called if the signal is
    //       registered with sigaction() call and TSAN is enabled. The problem seems to be
    //       related with the dispatch_semaphore_wait() function in the RunEventLoop() method.
    //       If this call is commented out, the signal handler is called as expected...
#if defined(__APPLE__)
    // NOLINTBEGIN(bugprone-signal-handler)
    signal(SIGINT, StopSignalHandler);
    signal(SIGTERM, StopSignalHandler);
    // NOLINTEND(bugprone-signal-handler)
#else
    struct sigaction sa                        = {};
    sa.sa_handler                              = StopSignalHandler;
    sa.sa_flags                                = SA_RESETHAND;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
#endif

    if (impl != nullptr)
    {
        impl->RunMainLoop();
    }
    else
    {
        DeviceLayer::PlatformMgr().RunEventLoop();
    }
    gMainLoopImplementation = nullptr;

    ApplicationShutdown();

#if defined(ENABLE_CHIP_SHELL)
    shellThread.join();
#endif

    Server::GetInstance().Shutdown();

#if CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE
    // Commissioner shutdown call shuts down entire stack, including the platform manager.
    ShutdownCommissioner();
#else
    DeviceLayer::PlatformMgr().Shutdown();
#endif // CHIP_DEVICE_CONFIG_ENABLE_BOTH_COMMISSIONER_AND_COMMISSIONEE

#if ENABLE_TRACING
    tracing_setup.StopTracing();
#endif

    Cleanup();
}
