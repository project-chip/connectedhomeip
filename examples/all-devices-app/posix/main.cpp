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
#include <AppRootNode.h>
#include <LinuxCommissionableDataProvider.h>
#include <TracingCommandLineArgument.h>
#include <app/DeviceLoadStatusProvider.h>
#include <app/InteractionModelEngine.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app_options/AppOptions.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <devices/device-factory/DeviceFactory.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <string>
#include <system/SystemLayer.h>

#include <TermHandling.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Platform;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::ArgParser;

namespace {
AppMainLoopImplementation * gMainLoopImplementation = nullptr;

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
        // Usage of VerifyOrDie in the nested lambda instead of SuccessOrDie is intentional:
        // The SuccessOrDie macro uses a `__err` assignment and some compilers complain about
        // variable shadowing.
        SuccessOrDie(SystemLayer().ScheduleLambda([]() { VerifyOrDie(PlatformMgr().StopEventLoopTask() == CHIP_NO_ERROR); }));
    }
}

class CodeDrivenDataModelDevices
{
public:
    struct Context
    {
        chip::PersistentStorageDelegate & storageDelegate;
        CommissioningWindowManager & commissioningWindowManager;
        DeviceLayer::ConfigurationManager & configurationManager;
        DeviceLayer::DeviceControlServer & deviceControlServer;
        FabricTable & fabricTable;
        Access::AccessControl & accessControl;
        PersistentStorageDelegate & persistentStorage;
        FailSafeContext & failSafeContext;
        DeviceLayer::DeviceInstanceInfoProvider & deviceInstanceInfoProvider;
        DeviceLayer::PlatformManager & platformManager;
        Credentials::GroupDataProvider & groupDataProvider;
        SessionManager & sessionManager;
        DnssdServer & dnssdServer;
        DeviceLoadStatusProvider & deviceLoadStatusProvider;
        DeviceLayer::DiagnosticDataProvider & diagnosticDataProvider;
        TestEventTriggerDelegate * testEventTriggerDelegate;

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        TermsAndConditionsProvider & termsAndConditionsProvider;
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    };

    CodeDrivenDataModelDevices(const Context & context) :
        mContext(context), mDataModelProvider(mContext.storageDelegate, mAttributePersistence),
        mRootNode(
            {
                .commissioningWindowManager     = mContext.commissioningWindowManager, //
                    .configurationManager       = mContext.configurationManager,       //
                    .deviceControlServer        = mContext.deviceControlServer,        //
                    .fabricTable                = mContext.fabricTable,                //
                    .accessControl              = mContext.accessControl,              //
                    .persistentStorage          = mContext.persistentStorage,          //
                    .failSafeContext            = mContext.failSafeContext,            //
                    .deviceInstanceInfoProvider = mContext.deviceInstanceInfoProvider, //
                    .platformManager            = mContext.platformManager,            //
                    .groupDataProvider          = mContext.groupDataProvider,          //
                    .sessionManager             = mContext.sessionManager,             //
                    .dnssdServer                = mContext.dnssdServer,                //
                    .deviceLoadStatusProvider   = mContext.deviceLoadStatusProvider,   //
                    .diagnosticDataProvider     = mContext.diagnosticDataProvider,     //
                    .testEventTriggerDelegate   = mContext.testEventTriggerDelegate,   //

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
                    .termsAndConditionsProvider = mContext.termsAndConditionsProvider,
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
            },
            []() {
                BitFlags<AppRootNode::EnabledFeatures> features;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
                features.Set(AppRootNode::EnabledFeatures::kWiFi, AppOptions::EnableWiFi());
#endif
                return features;
            }())
    {}

    CHIP_ERROR Startup()
    {
        ReturnErrorOnFailure(mAttributePersistence.Init(&mContext.storageDelegate));
        ReturnErrorOnFailure(mRootNode.RootDevice().Register(kRootEndpointId, mDataModelProvider, kInvalidEndpointId));

        mConstructedDevice = DeviceFactory::GetInstance().Create(AppOptions::GetDeviceType());
        VerifyOrReturnError(mConstructedDevice, CHIP_ERROR_NO_MEMORY);
        ReturnErrorOnFailure(mConstructedDevice->Register(AppOptions::GetDeviceEndpoint(), mDataModelProvider, kInvalidEndpointId));

        return CHIP_NO_ERROR;
    }

    void Shutdown()
    {
        if (mConstructedDevice)
        {
            mConstructedDevice->UnRegister(mDataModelProvider);
            mConstructedDevice.reset();
        }
        mRootNode.RootDevice().UnRegister(mDataModelProvider);
    }

    chip::app::CodeDrivenDataModelProvider & DataModelProvider() { return mDataModelProvider; }

private:
    Context mContext;
    chip::app::DefaultAttributePersistenceProvider mAttributePersistence;

    chip::app::CodeDrivenDataModelProvider mDataModelProvider;

    AppRootNode mRootNode;
    std::unique_ptr<DeviceInterface> mConstructedDevice;
};

void RunApplication(AppMainLoopImplementation * mainLoop = nullptr)
{
    gMainLoopImplementation = mainLoop;

    static DefaultTimerDelegate timerDelegate;
    DeviceFactory::GetInstance().Init(DeviceFactory::Context{
        .groupDataProvider = gGroupDataProvider,                     //
        .fabricTable       = Server::GetInstance().GetFabricTable(), //
        .timerDelegate     = timerDelegate,                          //

    });

    static chip::CommonCaseDeviceServerInitParams initParams;

    SuccessOrDie(initParams.InitializeStaticResourcesBeforeServerInit());

    gGroupDataProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    Credentials::SetGroupDataProvider(&gGroupDataProvider);

    DeviceLayer::DeviceInstanceInfoProvider * provider = DeviceLayer::GetDeviceInstanceInfoProvider();
    if (provider == nullptr)
    {
        ChipLogError(AppServer, "Failed to get the DeviceInstanceInfoProvifer.");
        chipDie();
    }

    static CodeDrivenDataModelDevices devices({
        .storageDelegate                = *initParams.persistentStorageDelegate,                 //
            .commissioningWindowManager = Server::GetInstance().GetCommissioningWindowManager(), //
            .configurationManager       = DeviceLayer::ConfigurationMgr(),                       //
            .deviceControlServer        = DeviceLayer::DeviceControlServer::DeviceControlSvr(),  //
            .fabricTable                = Server::GetInstance().GetFabricTable(),                //
            .accessControl              = Server::GetInstance().GetAccessControl(),              //
            .persistentStorage          = Server::GetInstance().GetPersistentStorage(),          //
            .failSafeContext            = Server::GetInstance().GetFailSafeContext(),            //
            .deviceInstanceInfoProvider = *provider,                                             //
            .platformManager            = DeviceLayer::PlatformMgr(),                            //
            .groupDataProvider          = gGroupDataProvider,                                    //
            .sessionManager             = Server::GetInstance().GetSecureSessionManager(),       //
            .dnssdServer                = DnssdServer::Instance(),                               //
            .deviceLoadStatusProvider   = *InteractionModelEngine::GetInstance(),                //
            .diagnosticDataProvider     = DeviceLayer::GetDiagnosticDataProvider(),              //
            .testEventTriggerDelegate   = initParams.testEventTriggerDelegate,                   //

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
            .termsAndConditionsProvider = TermsAndConditionsManager::GetInstance(),
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    });

    SuccessOrDie(devices.Startup());

    initParams.dataModelProvider             = &devices.DataModelProvider();
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
    SuccessOrDie(GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));
    payload.discriminator.SetLongValue(discriminator);

    SuccessOrDie(chip::DeviceLayer::GetDeviceInstanceInfoProvider()->GetVendorId(payload.vendorID));
    SuccessOrDie(chip::DeviceLayer::GetDeviceInstanceInfoProvider()->GetProductId(payload.productID));
    PrintOnboardingCodes(payload);

    SetDeviceAttestationCredentialsProvider(Credentials::Examples::GetExampleDACProvider());

    chip::app::SetTerminateHandler(StopSignalHandler);

    // This message is used as a marker for when the application process has started.
    // See: scripts/tests/chiptest/test_definition.py
    // TODO: A cleaner and more generic mechanism needs to be developed as a follow-up.
    // Currently other places (OTA, TV) also scrape logs for information and a better way should be
    // possible.
    ChipLogProgress(DeviceLayer, "===== APP STATUS: Starting event loop =====");

    if (mainLoop != nullptr)
    {
        mainLoop->RunMainLoop();
    }
    else
    {
        DeviceLayer::PlatformMgr().RunEventLoop();
    }
    gMainLoopImplementation = nullptr;

    devices.Shutdown();
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

#if CONFIG_NETWORK_LAYER_BLE
    ReturnErrorOnFailure(DeviceLayer::ConnectivityMgr().SetBLEDeviceName(nullptr));
    ReturnErrorOnFailure(DeviceLayer::Internal::BLEMgrImpl().ConfigureBle(0, false));
    ReturnErrorOnFailure(DeviceLayer::ConnectivityMgr().SetBLEAdvertisingEnabled(true));
#endif

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
