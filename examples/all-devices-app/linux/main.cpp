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
Credentials::GroupDataProviderImpl gGroupDataProvider;
AllDevicesExampleDeviceInfoProviderImpl gExampleDeviceInfoProvider;

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
        // NOTE: using VerifyOrDie instead of SuccessOrDie to not have nested `__err` declarations because
        //       that causes compilers to complain about shadowing.
        VerifyOrDie(SystemLayer().ScheduleLambda([]() { SuccessOrDie(PlatformMgr().StopEventLoopTask()); }) == CHIP_NO_ERROR);
    }
}

class CodeDrivenDataModelDevices
{
public:
    CodeDrivenDataModelDevices(chip::PersistentStorageDelegate & storageDelegate) :
        mStorageDelegate(storageDelegate), mDataModelProvider(storageDelegate, mAttributePersistence), mRootNode(&mWifiDriver)
    {
        mWifiDriver.Set5gSupport(true);
    }

    CHIP_ERROR Init()
    {
        ReturnErrorOnFailure(mAttributePersistence.Init(&mStorageDelegate));
        ReturnErrorOnFailure(mRootNode.Register(kRootEndpointId, mDataModelProvider, kInvalidEndpointId));

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
        mRootNode.UnRegister(mDataModelProvider);
    }

    chip::app::CodeDrivenDataModelProvider & DataModelProvider() { return mDataModelProvider; }

private:
    chip::PersistentStorageDelegate & mStorageDelegate;
    chip::app::DefaultAttributePersistenceProvider mAttributePersistence;
    DeviceLayer::NetworkCommissioning::LinuxWiFiDriver mWifiDriver;
    chip::app::CodeDrivenDataModelProvider mDataModelProvider;

    WifiRootNodeDevice mRootNode;
    std::unique_ptr<DeviceInterface> mConstructedDevice;
};

void RunApplication(AppMainLoopImplementation * mainLoop = nullptr)
{
    gMainLoopImplementation = mainLoop;

    static DefaultTimerDelegate timerDelegate;
    DeviceFactory::GetInstance().Init(DeviceFactory::Context{
        .timerDelegate     = timerDelegate,
        .groupDataProvider = gGroupDataProvider,
        .fabricTable       = Server::GetInstance().GetFabricTable(),
    });

    static chip::CommonCaseDeviceServerInitParams initParams;
    SuccessOrDie(initParams.InitializeStaticResourcesBeforeServerInit());

    gGroupDataProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    gGroupDataProvider.SetSessionKeystore(initParams.sessionKeystore);
    SuccessOrDie(gGroupDataProvider.Init());

    static CodeDrivenDataModelDevices devices(*initParams.persistentStorageDelegate);
    SuccessOrDie(devices.Init());

    initParams.dataModelProvider             = &devices.DataModelProvider();
    initParams.operationalServicePort        = CHIP_PORT;
    initParams.userDirectedCommissioningPort = CHIP_UDC_PORT;
    initParams.interfaceId                   = Inet::InterfaceId::Null();
    initParams.groupDataProvider             = &gGroupDataProvider;

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

    struct sigaction sa = {};
    sa.sa_handler       = StopSignalHandler;
    sa.sa_flags         = SA_RESETHAND;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    // This message is used as a marker for when the application process has started.
    // See: scripts/tests/chiptest/test_definition.py
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
