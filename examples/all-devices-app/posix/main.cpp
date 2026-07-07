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

#include <AppMainLoop.h>
#include <AppRootNode.h>
#include <DeviceFactoryPlatformOverride.h>
#include <LinuxCommissionableDataProvider.h>
#include <PosixAudioManager.h>
#include <TracingCommandLineArgument.h>
#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
#include <TraceDecoder.h>
#include <TraceHandlers.h>
#endif
#include <app/DefaultSafeAttributePersistenceProvider.h>
#include <app/DeviceLoadStatusProvider.h>
#include <app/InteractionModelEngine.h>
#include <app/SafeAttributePersistenceProvider.h>
#include <app/TestEventTriggerDelegate.h>
#include <app/persistence/DefaultAttributePersistenceProvider.h>
#include <app/server-cluster/ServerClusterInterfaceRegistry.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <providers/AllDevicesExampleDeviceInfoProviderImpl.h>
#include <providers/AllDevicesExampleDeviceInstanceInfoProviderImpl.h>

#include <app_options/AppOptions.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <device-factory/DeviceFactory.h>
#include <devices/device-type-parser/DeviceTypeParser.h>
#include <devices/endpoint-id-allocator/DynamicEndpointIdAllocator.h>
#include <oob-accessors/OOBAccessor.h>
#include <oob-accessors/OOBAccessorRegistry.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/DiagnosticDataProvider.h>
#include <platform/PlatformManager.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <system/SystemLayer.h>

#include <AppCommandDelegate.h>
#include <BleInit.h>
#include <TermHandling.h>
#if PW_RPC_ENABLED
#include <Rpc.h>
#include <oob-accessors/pigweed/PigweedAttributeAccessor.h>
#include <pigweed/rpc_services/AccessInterceptorRegistry.h>
#endif // PW_RPC_ENABLED
#include <devices/boolean-state-sensor/BooleanStateSensorDevice.h>
#include <devices/interface/SingleEndpointDevice.h>
#include <devices/occupancy-sensor/OccupancySensorDevice.h>
#include <devices/on-off-light/impl/LoggingOnOffLightDevice.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Platform;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;
using namespace chip::ArgParser;

void ApplicationShutdown();

namespace {
AppMainLoopImplementation * gMainLoopImplementation = nullptr;

Credentials::GroupDataProviderImpl gGroupDataProvider;
chip::app::DefaultSafeAttributePersistenceProvider gSafeAttributePersistenceProvider;
DefaultTimerDelegate gTimerDelegate;
chip::app::PosixAudioManager gAudioManager;

// To hold SPAKE2+ verifier, discriminator, passcode
LinuxCommissionableDataProvider gCommissionableDataProvider;

AllDevicesAppCommandDelegate gAllDevicesAppCommandDelegate;
NamedPipeCommands gNamedPipeCommands;

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
        Credentials::DeviceAttestationCredentialsProvider & dacProvider;
        EventManagement & eventManagement;
        SafeAttributePersistenceProvider & safeAttributePersistenceProvider;
        TimerDelegate & timerDelegate;
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
        TermsAndConditionsProvider & termsAndConditionsProvider;
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    };

    CodeDrivenDataModelDevices(const Context & context) :
        mContext(context), mDataModelProvider(mContext.storageDelegate, mAttributePersistence),
        mRootNode(
            {
                .commissioningWindowManager           = mContext.commissioningWindowManager,       //
                    .configurationManager             = mContext.configurationManager,             //
                    .deviceControlServer              = mContext.deviceControlServer,              //
                    .fabricTable                      = mContext.fabricTable,                      //
                    .accessControl                    = mContext.accessControl,                    //
                    .persistentStorage                = mContext.persistentStorage,                //
                    .failSafeContext                  = mContext.failSafeContext,                  //
                    .deviceInstanceInfoProvider       = mContext.deviceInstanceInfoProvider,       //
                    .platformManager                  = mContext.platformManager,                  //
                    .groupDataProvider                = mContext.groupDataProvider,                //
                    .sessionManager                   = mContext.sessionManager,                   //
                    .dnssdServer                      = mContext.dnssdServer,                      //
                    .deviceLoadStatusProvider         = mContext.deviceLoadStatusProvider,         //
                    .diagnosticDataProvider           = mContext.diagnosticDataProvider,           //
                    .testEventTriggerDelegate         = mContext.testEventTriggerDelegate,         //
                    .dacProvider                      = mContext.dacProvider,                      //
                    .eventManagement                  = mContext.eventManagement,                  //
                    .safeAttributePersistenceProvider = mContext.safeAttributePersistenceProvider, //
                    .timerDelegate                    = mContext.timerDelegate,                    //
#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
                    .termsAndConditionsProvider = mContext.termsAndConditionsProvider,
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
            },
            []() {
                BitFlags<AppRootNode::EnabledFeatures> features;
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
                features.Set(AppRootNode::EnabledFeatures::kWiFi, AppOptions::GetConfig().enableWiFi);
#endif
                return features;
            }())
    {}

    std::set<EndpointId> GetReservedEndpointIds() const
    {
        std::set<EndpointId> usedIds;
        usedIds.insert(kRootEndpointId);

        for (const auto & entry : AppOptions::GetDeviceTypeEntries())
        {
            if (entry.endpoint != kInvalidEndpointId)
            {
                usedIds.insert(entry.endpoint);
            }
        }
        return usedIds;
    }

    CHIP_ERROR Startup()
    {
        ReturnErrorOnFailure(mAttributePersistence.Init(&mContext.storageDelegate));

        DynamicEndpointIdAllocator endpointIdAllocator(GetReservedEndpointIds());
        endpointIdAllocator.ForceNext(kRootEndpointId);
        ReturnErrorOnFailure(mRootNode.RootDevice().Register(endpointIdAllocator, mDataModelProvider));

        for (const auto & entry : AppOptions::GetDeviceTypeEntries())
        {
            auto device = DeviceFactory::GetInstance().Create(entry.type, entry.label);

            VerifyOrReturnError(device, CHIP_ERROR_NO_MEMORY);
            ChipLogProgress(AppServer, "Registering device %s on endpoint %u with parent 0x%04X", entry.type.c_str(),
                            entry.endpoint, entry.parentId);
            if (entry.endpoint != kInvalidEndpointId)
            {
                endpointIdAllocator.ForceNext(entry.endpoint);
            }
            ReturnErrorOnFailure(
                device->Register(endpointIdAllocator, mDataModelProvider, EndpointComposition::WithParent(entry.parentId)));
            auto oobAccessor = DeviceFactory::GetInstance().CreateAccessor(entry.type, *device);
            if (oobAccessor)
            {
                OOBAccessorRegistry::Instance().Register(*oobAccessor);
                mConstructedAccessors.push_back(std::move(oobAccessor));
            }
            mConstructedDevices.push_back(std::move(device));
        }

        return CHIP_NO_ERROR;
    }

    void Shutdown()
    {
        mConstructedAccessors.clear();
        for (auto & device : mConstructedDevices)
        {
            device->Unregister(mDataModelProvider);
        }
        mConstructedDevices.clear();
        mRootNode.RootDevice().Unregister(mDataModelProvider);
    }

    chip::app::CodeDrivenDataModelProvider & DataModelProvider() { return mDataModelProvider; }

    AppRootNode & RootNode() { return mRootNode; }

    const std::vector<std::unique_ptr<DeviceInterface>> & GetConstructedDevices() const { return mConstructedDevices; }

private:
    Context mContext;
    chip::app::DefaultAttributePersistenceProvider mAttributePersistence;
    chip::app::CodeDrivenDataModelProvider mDataModelProvider;

    AppRootNode mRootNode;
    std::vector<std::unique_ptr<DeviceInterface>> mConstructedDevices;

    std::vector<std::unique_ptr<chip::app::OOBAccessor>> mConstructedAccessors;
};

void SetupNamedPipe(CodeDrivenDataModelDevices & devices, const char * namedPipePath)
{
    auto deviceConfigs              = AppOptions::GetDeviceTypeEntries();
    const auto & constructedDevices = devices.GetConstructedDevices();

    // Calling code already checked that deviceConfigs.size() == constructedDevices.size().
    VerifyOrDie(deviceConfigs.size() == constructedDevices.size());

    // TODO(#72638): The hardcoded type references to specific device implementations below prevent those
    // classes from being selectively compiled out or stripped by LTO when they are disabled.
    // A more generic and pluggable registration mechanism (e.g., via DeviceFactory or an interface)
    // should be developed to allow true conditional compilation of devices.
    for (size_t i = 0; i < deviceConfigs.size(); i++)
    {
        const auto & config = deviceConfigs[i];
        auto * device       = constructedDevices[i].get();

        if (config.type == "occupancy-sensor")
        {
            auto * occupancyDevice = static_cast<OccupancySensorDevice *>(device);
            gAllDevicesAppCommandDelegate.GetClusterImplementationRegistry()
                .RegisterClusterInstance<chip::app::Clusters::OccupancySensingCluster>(&occupancyDevice->OccupancySensingCluster());
        }
        else if (config.type == "contact-sensor" || config.type == "water-leak-detector")
        {
            auto * booleanStateDevice = static_cast<BooleanStateSensorDevice *>(device);
            gAllDevicesAppCommandDelegate.GetClusterImplementationRegistry()
                .RegisterClusterInstance<chip::app::Clusters::BooleanStateCluster>(&booleanStateDevice->BooleanState());
        }
        else if (config.type == "on-off-light")
        {
            auto * lightDevice = static_cast<LoggingOnOffLightDevice *>(device);
            gAllDevicesAppCommandDelegate.GetClusterImplementationRegistry()
                .RegisterClusterInstance<chip::app::Clusters::OnOffCluster>(&lightDevice->OnOffCluster());
        }
    }

    gAllDevicesAppCommandDelegate.GetClusterImplementationRegistry()
        .RegisterClusterInstance<chip::app::Clusters::BasicInformationCluster>(
            &devices.RootNode().GetRootNodeDevice().BasicInformation());
    gAllDevicesAppCommandDelegate.RegisterCommandHandlers();

    CHIP_ERROR err = gNamedPipeCommands.Start(namedPipePath, &gAllDevicesAppCommandDelegate);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "Failed to start named pipe at %s: %" CHIP_ERROR_FORMAT, namedPipePath, err.Format());
        LogErrorOnFailure(gNamedPipeCommands.Stop());
    }
}

void RunApplication(AppMainLoopImplementation * mainLoop = nullptr)
{
    gMainLoopImplementation = mainLoop;

    static chip::CommonCaseDeviceServerInitParams initParams;
    SuccessOrDie(initParams.InitializeStaticResourcesBeforeServerInit());

    DeviceFactory::GetInstance().Init(DeviceFactory::Context{
        .groupDataProvider = gGroupDataProvider,                     //
        .fabricTable       = Server::GetInstance().GetFabricTable(), //
        .timerDelegate     = gTimerDelegate,                         //
        .storageDelegate   = *initParams.persistentStorageDelegate,  //
    });

    RegisterDeviceFactoryOverrides(gTimerDelegate, initParams.persistentStorageDelegate, gAudioManager);

#if CHIP_CONFIG_ENABLE_GROUPCAST
    // TODO(#72056): Once groupcast is enabled by default, this should not be dependent on the app argument.
    if (AppOptions::GetConfig().enableGroupcast)
    {
        static chip::Access::Examples::GroupAuxiliaryAccessControlDelegateImpl groupAuxDelegate;
        SuccessOrDie(groupAuxDelegate.Initialize(&gGroupDataProvider, &Server::GetInstance().GetFabricTable()));
        initParams.groupAuxiliaryAccessControlDelegate = &groupAuxDelegate;
        gGroupDataProvider.SetGroupcastEnabled(true);
    }
#endif // CHIP_CONFIG_ENABLE_GROUPCAST

    gGroupDataProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    gGroupDataProvider.SetSessionKeystore(initParams.sessionKeystore);
    SuccessOrDie(gGroupDataProvider.Init());
    Credentials::SetGroupDataProvider(&gGroupDataProvider);

    DeviceLayer::DeviceInstanceInfoProvider * provider = DeviceLayer::GetDeviceInstanceInfoProvider();
    if (provider == nullptr)
    {
        ChipLogError(AppServer, "Failed to get the DeviceInstanceInfoProvider.");
        chipDie();
    }

    // Initialize the safe attribute persistence provider
    SuccessOrDie(gSafeAttributePersistenceProvider.Init(initParams.persistentStorageDelegate));
    SetSafeAttributePersistenceProvider(&gSafeAttributePersistenceProvider);

    // Set the global DAC provider before server/cluster init so any integration path that
    // snapshots the provider during construction sees a valid implementation.
    SetDeviceAttestationCredentialsProvider(Credentials::Examples::GetExampleDACProvider());

    static CodeDrivenDataModelDevices devices({
        .storageDelegate                      = *initParams.persistentStorageDelegate,                   //
            .commissioningWindowManager       = Server::GetInstance().GetCommissioningWindowManager(),   //
            .configurationManager             = DeviceLayer::ConfigurationMgr(),                         //
            .deviceControlServer              = DeviceLayer::DeviceControlServer::DeviceControlSvr(),    //
            .fabricTable                      = Server::GetInstance().GetFabricTable(),                  //
            .accessControl                    = Server::GetInstance().GetAccessControl(),                //
            .persistentStorage                = Server::GetInstance().GetPersistentStorage(),            //
            .failSafeContext                  = Server::GetInstance().GetFailSafeContext(),              //
            .deviceInstanceInfoProvider       = *provider,                                               //
            .platformManager                  = DeviceLayer::PlatformMgr(),                              //
            .groupDataProvider                = gGroupDataProvider,                                      //
            .sessionManager                   = Server::GetInstance().GetSecureSessionManager(),         //
            .dnssdServer                      = DnssdServer::Instance(),                                 //
            .deviceLoadStatusProvider         = *InteractionModelEngine::GetInstance(),                  //
            .diagnosticDataProvider           = DeviceLayer::GetDiagnosticDataProvider(),                //
            .testEventTriggerDelegate         = initParams.testEventTriggerDelegate,                     //
            .dacProvider                      = *Credentials::GetDeviceAttestationCredentialsProvider(), //
            .eventManagement                  = EventManagement::GetInstance(),                          //
            .safeAttributePersistenceProvider = gSafeAttributePersistenceProvider,                       //
            .timerDelegate                    = gTimerDelegate,                                          //

#if CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
            .termsAndConditionsProvider = TermsAndConditionsManager::GetInstance(),
#endif // CHIP_CONFIG_TERMS_AND_CONDITIONS_REQUIRED
    });

    SuccessOrDie(devices.Startup());

    if (AppOptions::GetDeviceTypeEntries().size() != devices.GetConstructedDevices().size())
    {
        ChipLogError(AppServer, "Failed to initialize some of the --device entries on command line. Cannot proceed.");
        chipDie();
    }

    // Set up named pipe command handlers against the registered devices.
    const std::string & namedPipePath = AppOptions::GetConfig().appPipePath;
    if (!namedPipePath.empty())
    {
        SetupNamedPipe(devices, namedPipePath.c_str());
    }

    initParams.dataModelProvider      = &devices.DataModelProvider();
    initParams.groupDataProvider      = &gGroupDataProvider;
    initParams.operationalServicePort = AppOptions::GetConfig().port.value_or(CHIP_PORT);
    ChipLogProgress(AppServer, "Using operationalServicePort %u\n", initParams.operationalServicePort);
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;

    if (AppOptions::GetConfig().interfaceId.has_value())
    {
        initParams.interfaceId =
            Inet::InterfaceId(static_cast<Inet::InterfaceId::PlatformType>(AppOptions::GetConfig().interfaceId.value()));
    }
    else
    {
        initParams.interfaceId = Inet::InterfaceId::Null();
    }

#if defined(ENABLE_TRACING) && ENABLE_TRACING
    chip::CommandLineApp::TracingSetup tracing_setup;
    for (const auto & trace_destination : AppOptions::GetConfig().traceTo)
    {
        tracing_setup.EnableTracingFor(trace_destination.c_str());
    }
#endif

#if CHIP_CONFIG_TRANSPORT_TRACE_ENABLED
    // Note: Transport tracing options are currently not supported via the custom AppOptions parser.
    // If transport tracing is required in the future, corresponding options should be added to AppOptions.
    // See examples/platform/linux/Options.cpp and examples/platform/linux/AppMain.cpp for examples
    // of how to parse transport tracing options from the command line.
#endif // CHIP_CONFIG_TRANSPORT_TRACE_ENABLED

#if PW_RPC_ENABLED
    static chip::app::PigweedAttributeAccessor sPwOobAccessor;
    chip::rpc::PigweedDebugAccessInterceptorRegistry::Instance().Register(&sPwOobAccessor);

    chip::rpc::Init(33000); // TODO: Add an arg for Pw port.
    ChipLogProgress(AppServer, "PW_RPC initialized.");
#endif // PW_RPC_ENABLED

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

    LogErrorOnFailure(gNamedPipeCommands.Stop());
    devices.Shutdown();
    Server::GetInstance().Shutdown();
    DeviceLayer::PlatformMgr().Shutdown();

#if defined(ENABLE_TRACING) && ENABLE_TRACING
    tracing_setup.StopTracing();
#endif

    ApplicationShutdown();
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

CHIP_ERROR InitCommissionableDataProvider(LinuxCommissionableDataProvider & provider, const AppOptions::AppConfig & config)
{
    auto discriminator = config.discriminator.value_or(static_cast<uint16_t>(CHIP_DEVICE_CONFIG_USE_TEST_SETUP_DISCRIMINATOR));

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

    static HelpOptions sHelpOptions(argv[0], "Usage: all-devices-app [options]", "1.0");
    static OptionSet * sAppOptionSets[] = { AppOptions::GetOptions(), &sHelpOptions, nullptr };
    if (!ArgParser::ParseArgs(argv[0], argc, argv, sAppOptionSets))
    {
        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    ReturnErrorOnFailure(AppOptions::ValidateConfig());

    const char * kvsPath = AppOptions::GetConfig().kvsPath.empty() ? CHIP_CONFIG_KVS_PATH : AppOptions::GetConfig().kvsPath.c_str();
    ReturnErrorOnFailure(DeviceLayer::PersistedStorage::KeyValueStoreMgrImpl().Init(kvsPath));
    ReturnErrorOnFailure(DeviceLayer::PlatformMgr().InitChipStack());

    ReturnErrorOnFailure(InitCommissionableDataProvider(gCommissionableDataProvider, AppOptions::GetConfig()));
    DeviceLayer::SetCommissionableDataProvider(&gCommissionableDataProvider);

    static AllDevicesExampleDeviceInfoProviderImpl sExampleDeviceInfoProvider;
    DeviceLayer::SetDeviceInfoProvider(&sExampleDeviceInfoProvider);

    const auto & config = AppOptions::GetConfig();
    auto vendorId       = config.vendorId.has_value() ? config.vendorId : std::nullopt;
    auto productId      = config.productId.has_value() ? config.productId : std::nullopt;
    static AllDevicesExampleDeviceInstanceInfoProviderImpl sAppDeviceInstanceInfoProvider(
        DeviceLayer::GetDeviceInstanceInfoProvider(), vendorId, productId);
    DeviceLayer::SetDeviceInstanceInfoProvider(&sAppDeviceInstanceInfoProvider);

    ConfigurationMgr().LogDeviceConfig();

    ReturnErrorOnFailure(DeviceLayer::PlatformMgrImpl().AddEventHandler(EventHandler, 0));

    ReturnErrorOnFailure(chip::app::InitBle(AppOptions::GetConfig().bleController));

    return CHIP_NO_ERROR;
}

} // namespace

void ApplicationShutdown()
{
    gAudioManager.Shutdown();
}

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
