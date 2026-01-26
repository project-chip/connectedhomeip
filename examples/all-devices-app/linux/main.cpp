/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
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

#include <AllDevicesExampleDeviceInfoProviderImpl.h>
#include <AppMainLoop.h>
#include <LinuxCommissionableDataProvider.h>
#include <TracingCommandLineArgument.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server/Dnssd.h>
#include <app_options/AppOptions.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <devices/device-factory/DeviceFactory.h>
#include <devices/root-node/RootNodeDevice.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/Linux/NetworkCommissioningDriver.h>
#include <platform/PlatformManager.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <string>
#include <system/SystemLayer.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Platform;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::ArgParser;

namespace {
AppMainLoopImplementation * gMainLoopImplementation = nullptr;

DeviceLayer::NetworkCommissioning::LinuxWiFiDriver sWiFiDriver;

AllDevicesExampleDeviceInfoProviderImpl gExampleDeviceInfoProvider;

Credentials::GroupDataProviderImpl gGroupDataProvider;

// To hold SPAKE2+ verifier, discriminator, passcode
LinuxCommissionableDataProvider gCommissionableDataProvider;

void StopSignalHandler(int /* signal */)
{
    if (gMainLoopImplementation != nullptr)
    {
        gMainLoopImplementation->SignalSafeStopMainLoop();
    }
    else
    {
        Server::GetInstance().GenerateShutDownEvent();
        TEMPORARY_RETURN_IGNORED SystemLayer().ScheduleLambda([]() { TEMPORARY_RETURN_IGNORED PlatformMgr().StopEventLoopTask(); });
    }
}

chip::app::DataModel::Provider * PopulateCodeDrivenDataModelProvider(PersistentStorageDelegate * delegate)
{
    static chip::app::DefaultAttributePersistenceProvider attributePersistenceProvider;
    SuccessOrDie(attributePersistenceProvider.Init(delegate));

    static chip::app::CodeDrivenDataModelProvider dataModelProvider =
        chip::app::CodeDrivenDataModelProvider(*delegate, attributePersistenceProvider);

    static WifiRootNodeDevice rootNodeDevice(
        {
            .commissioningWindowManager = Server::GetInstance().GetCommissioningWindowManager(),
            .configurationManager       = DeviceLayer::ConfigurationMgr(),
            .deviceControlServer        = DeviceLayer::DeviceControlServer::DeviceControlSvr(),
            .fabricTable = Server::GetInstance().GetFabricTable(), .failsafeContext = Server::GetInstance().GetFailSafeContext(),
            .platformManager = DeviceLayer::PlatformMgr(), .groupDataProvider = gGroupDataProvider,
            .sessionManager = Server::GetInstance().GetSecureSessionManager(), .dnssdServer = DnssdServer::Instance(),

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
            .termsAndConditionsProvider = TermsAndConditionsManager::GetInstance(),
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        },
        {
            .wifiDriver = sWiFiDriver,
        });
    static std::unique_ptr<DeviceInterface> constructedDevice;

    TEMPORARY_RETURN_IGNORED rootNodeDevice.Register(kRootEndpointId, dataModelProvider, kInvalidEndpointId);
    constructedDevice = DeviceFactory::GetInstance().Create(AppOptions::GetDeviceType());
    TEMPORARY_RETURN_IGNORED constructedDevice->Register(AppOptions::GetDeviceEndpoint(), dataModelProvider, kInvalidEndpointId);

    return &dataModelProvider;
}

void RunApplication(AppMainLoopImplementation * mainLoop = nullptr)
{
    gMainLoopImplementation = mainLoop;

    static chip::CommonCaseDeviceServerInitParams initParams;
    VerifyOrDie(initParams.InitializeStaticResourcesBeforeServerInit() == CHIP_NO_ERROR);

    gGroupDataProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    Credentials::SetGroupDataProvider(&gGroupDataProvider);

    initParams.dataModelProvider             = PopulateCodeDrivenDataModelProvider(initParams.persistentStorageDelegate);
    initParams.groupDataProvider             = &gGroupDataProvider;
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;
    initParams.interfaceId                   = Inet::InterfaceId::Null();

    chip::CommandLineApp::TracingSetup tracing_setup;
    tracing_setup.EnableTracingFor("json:log");

    // Init ZCL Data Model and CHIP App Server
    CHIP_ERROR err = Server::GetInstance().Init(initParams);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Server init failed: %" CHIP_ERROR_FORMAT, err.Format());
        chipDie();
    }

    // Now that the server has started and we are done with our startup logging,
    // log our discovery/onboarding information again so it's not lost in the
    // noise.
    ConfigurationMgr().LogDeviceConfig();

    chip::PayloadContents payload;

    payload.version = 0;
    payload.rendezvousInformation.SetValue(RendezvousInformationFlag::kBLE);

    if (GetCommissionableDataProvider()->GetSetupPasscode(payload.setUpPINCode) != CHIP_NO_ERROR)
    {
        payload.setUpPINCode = CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE;
    }

    uint16_t discriminator = 0;
    VerifyOrDie(GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator) == CHIP_NO_ERROR);
    payload.discriminator.SetLongValue(discriminator);

    VerifyOrDie(chip::DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(payload.vendorID) == CHIP_NO_ERROR);
    VerifyOrDie(chip::DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(payload.productID) == CHIP_NO_ERROR);
    PrintOnboardingCodes(payload);

    SetDeviceAttestationCredentialsProvider(Credentials::Examples::GetExampleDACProvider());

    sWiFiDriver.Set5gSupport(true);

    struct sigaction sa = {};
    sa.sa_handler       = StopSignalHandler;
    sa.sa_flags         = SA_RESETHAND;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    if (mainLoop != nullptr)
    {
        mainLoop->RunMainLoop();
    }
    else
    {
        DeviceLayer::PlatformMgr().RunEventLoop();
    }
    gMainLoopImplementation = nullptr;

    Server::GetInstance().Shutdown();
    DeviceLayer::PlatformMgr().Shutdown();
    tracing_setup.StopTracing();
}

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
        DnssdServer::Instance().StartServer();
    }
}

CHIP_ERROR InitCommissionableDataProvider(LinuxCommissionableDataProvider & provider)
{
    auto discriminator                              = static_cast<uint16_t>(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR);
    chip::Optional<uint16_t> discriminatorFromParam = LinuxDeviceOptions::GetInstance().discriminator;
    if (discriminatorFromParam.HasValue())
    {
        discriminator = discriminatorFromParam.Value();
    }

    const auto setupPasscode             = MakeOptional(static_cast<uint32_t>(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_PIN_CODE));
    const uint32_t spake2pIterationCount = Crypto::kSpake2p_Min_PBKDF_Iterations;

    Optional<std::vector<uint8_t>> serializedSpake2pVerifier = NullOptional;
    Optional<std::vector<uint8_t>> spake2pSalt               = NullOptional;

    return provider.Init(          //
        serializedSpake2pVerifier, //
        spake2pSalt,               //
        spake2pIterationCount,     //
        setupPasscode,             //
        discriminator              //
    );
}

CHIP_ERROR Initialize(int argc, char * argv[])
{
    ChipLogProgress(AppServer, "Initializing...");
    ReturnErrorOnFailure(Platform::MemoryInit());
    ReturnErrorOnFailure(ParseArguments(argc, argv, AppOptions::GetOptions()));
    ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(CHIP_CONFIG_KVS_PATH));
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().InitChipStack());

    ReturnErrorOnFailure(InitCommissionableDataProvider(gCommissionableDataProvider));
    DeviceLayer::SetCommissionableDataProvider(&gCommissionableDataProvider);
    DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);
    ConfigurationMgr().LogDeviceConfig();

    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0));
    ReturnErrorOnFailure(DeviceLayer::ConnectivityMgr().SetBLEDeviceName(nullptr));
    ReturnErrorOnFailure(DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(0, false));
    ReturnErrorOnFailure(DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true));

    return CHIP_NO_ERROR;
}

} // namespace

void ApplicationShutdown() {}

int main(int argc, char * argv[])
{
    ChipLogProgress(AppServer, "Initializing");

    if (CHIP_ERROR err = Initialize(argc, argv); err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Initialize() failed: %" CHIP_ERROR_FORMAT, err.Format());
        chipDie();
    }

    ChipLogProgress(AppServer, "Hello from all-devices-app!");
    RunApplication();

    return 0;
}
