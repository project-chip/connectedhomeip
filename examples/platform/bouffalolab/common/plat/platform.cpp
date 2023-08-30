/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include <DeviceInfoProviderImpl.h>
#include <OTAConfig.h>
#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/bouffalolab/common/PlatformManagerImpl.h>
#include <system/SystemClock.h>

#if HEAP_MONITORING
#include <MemMonitoring.h>
#endif

#if CONFIG_ENABLE_CHIP_SHELL
#include <ChipShellCollection.h>
#include <lib/shell/Engine.h>
#endif

#if PW_RPC_ENABLED
#include <PigweedLogger.h>
#include <Rpc.h>
#endif
#if CONFIG_ENABLE_CHIP_SHELL || PW_RPC_ENABLED
#include <uart.h>
#endif

#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE || defined(CONFIG_BOUFFALOLAB_FACTORY_DATA_TEST)
#include <platform/bouffalolab/common/FactoryDataProvider.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <NetworkCommissioningDriver.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#endif

#if CHIP_ENABLE_OPENTHREAD
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/bouffalolab/common/ThreadStackManagerImpl.h>
#include <utils_list.h>
#endif

#if !CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <lwip/netif.h>
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#if defined(BL602_ENABLE)
#include <wifi_mgmr_ext.h>
#else
#include <platform/bouffalolab/BL702/WiFiInterface.h>
#endif
#else
#include <platform/bouffalolab/BL702/EthernetInterface.h>
#endif
#include <bl_route_hook.h>
#endif

#include <AppTask.h>
#include <plat.h>

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
namespace {
chip::app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::BLWiFiDriver::GetInstance()));
}
#endif

static chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE || defined(CONFIG_BOUFFALOLAB_FACTORY_DATA_TEST)
namespace {
FactoryDataProvider sFactoryDataProvider;
}
#endif

void ChipEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        ChipLogProgress(NotSpecified, "BLE adv changed, connection number: %d", ConnectivityMgr().NumBLEConnections());
        break;
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    case DeviceEventType::kThreadStateChange:

        if (ConnectivityMgr().IsThreadAttached())
        {
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(OTAConfig::kInitOTARequestorDelaySec),
                                                        OTAConfig::InitOTARequestorHandler, nullptr);
        }
        break;
#else
    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.

            chip::app::DnssdServer::Instance().StartServer();

            bl_route_hook_init();

            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(OTAConfig::kInitOTARequestorDelaySec),
                                                        OTAConfig::InitOTARequestorHandler, nullptr);
        }

        break;
#endif
    case DeviceEventType::kInternetConnectivityChange:
        if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
        {
            ChipLogProgress(NotSpecified, "IPv4 connectivity ready...");
        }
        else if (event->InternetConnectivityChange.IPv4 == kConnectivity_Lost)
        {
            ChipLogProgress(NotSpecified, "Lost IPv4 connectivity...");
        }
        if (event->InternetConnectivityChange.IPv6 == kConnectivity_Established)
        {
            ChipLogProgress(NotSpecified, "IPv6 connectivity ready...");
        }
        else if (event->InternetConnectivityChange.IPv6 == kConnectivity_Lost)
        {
            ChipLogProgress(NotSpecified, "Lost IPv6 connectivity...");
        }
        break;
    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        ChipLogProgress(NotSpecified, "BLE connection established");
        break;
    case DeviceEventType::kCHIPoBLEConnectionClosed:
        ChipLogProgress(NotSpecified, "BLE disconnected");
        break;
    case DeviceEventType::kCommissioningComplete:
        ChipLogProgress(NotSpecified, "Commissioning complete");
        GetAppTask().PostEvent(AppTask::APP_EVENT_LIGHTING_MASK);
        break;
    default:
        break;
    }
}

CHIP_ERROR PlatformManagerImpl::PlatformInit(void)
{
#if PW_RPC_ENABLED
    PigweedLogger::pw_init();
#elif CONFIG_ENABLE_CHIP_SHELL
    AppTask::StartAppShellTask();
#endif

#if HEAP_MONITORING
    MemMonitoring::startHeapMonitoring();
#endif

    ChipLogProgress(NotSpecified, "Initializing CHIP stack");
    ReturnLogErrorOnFailure(PlatformMgr().InitChipStack());

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName(CHIP_BLE_DEVICE_NAME);

#if CHIP_ENABLE_OPENTHREAD

#if CONFIG_ENABLE_CHIP_SHELL
    cmd_otcli_init();
#endif

    ChipLogProgress(NotSpecified, "Initializing OpenThread stack");
    ReturnLogErrorOnFailure(ThreadStackMgr().InitThreadStack());

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    ReturnLogErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router));
#else
    ReturnLogErrorOnFailure(ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice));
#endif

#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI
    ReturnLogErrorOnFailure(sWiFiNetworkCommissioningInstance.Init());
#endif

    // Initialize device attestation config
#if CONFIG_BOUFFALOLAB_FACTORY_DATA_ENABLE || defined(CONFIG_BOUFFALOLAB_FACTORY_DATA_TEST)
    if (CHIP_NO_ERROR == sFactoryDataProvider.Init())
    {
        SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
        SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);
        SetCommissionableDataProvider(&sFactoryDataProvider);
    }
    else
    {
        ChipLogError(NotSpecified, "sFactoryDataProvider.Init() failed");
    }
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

    chip::DeviceLayer::PlatformMgr().LockChipStack();

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(EXT_DISCOVERY_TIMEOUT_SECS);
#endif

    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    static CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

    ReturnLogErrorOnFailure(chip::Server::GetInstance().Init(initParams));

    gExampleDeviceInfoProvider.SetStorageDelegate(&chip::Server::GetInstance().GetPersistentStorage());

    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

#if CHIP_ENABLE_OPENTHREAD
    ChipLogProgress(NotSpecified, "Starting OpenThread task");
    // Start OpenThread task
    ReturnLogErrorOnFailure(ThreadStackMgrImpl().StartThreadTask());
#endif

    ConfigurationMgr().LogDeviceConfig();

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kOnNetwork));
#endif

    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

    vTaskResume(GetAppTask().sAppTaskHandle);

    return CHIP_NO_ERROR;
}
