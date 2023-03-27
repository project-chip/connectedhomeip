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

#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <system/SystemClock.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <NetworkCommissioningDriver.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <route_hook/bl_route_hook.h>
#endif
#include <PlatformManagerImpl.h>

#if HEAP_MONITORING
#include <MemMonitoring.h>
#include <lib/support/CHIPMem.h>
#endif

#if CHIP_ENABLE_OPENTHREAD
#include <ThreadStackManagerImpl.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <utils_list.h>
#endif

#ifdef OTA_ENABLED
#include "OTAConfig.h"
#endif // OTA_ENABLED

#if CONFIG_ENABLE_CHIP_SHELL
#include <ChipShellCollection.h>
#include <lib/shell/Engine.h>
#endif

#if PW_RPC_ENABLED
#include "PigweedLogger.h"
#include "Rpc.h"
#endif

#include <DeviceInfoProviderImpl.h>

#if CONFIG_ENABLE_CHIP_SHELL || PW_RPC_ENABLED
#include "uart.h"
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

void ChipEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        GetAppTask().mIsConnected = ConnectivityMgr().IsWiFiStationConnected();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        GetAppTask().mIsConnected = ConnectivityMgr().IsThreadAttached();
#endif

        if (ConnectivityMgr().NumBLEConnections())
        {
            GetAppTask().PostEvent(AppTask::APP_EVENT_SYS_BLE_CONN);
        }
        else
        {
            GetAppTask().PostEvent(AppTask::APP_EVENT_SYS_BLE_ADV);
        }
        ChipLogProgress(NotSpecified, "BLE adv changed, connection number: %d", ConnectivityMgr().NumBLEConnections());
        break;
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    case DeviceEventType::kThreadStateChange:

        ChipLogProgress(NotSpecified, "Thread state changed, IsThreadAttached: %d", ConnectivityMgr().IsThreadAttached());
        if (!GetAppTask().mIsConnected && ConnectivityMgr().IsThreadAttached())
        {
            GetAppTask().PostEvent(AppTask::APP_EVENT_SYS_PROVISIONED);
            GetAppTask().mIsConnected = true;
        }
        break;
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    case DeviceEventType::kWiFiConnectivityChange:

        ChipLogProgress(NotSpecified, "Wi-Fi state changed to %s.",
                        ConnectivityMgr().IsWiFiStationConnected() ? "connected" : "disconnected");

        chip::app::DnssdServer::Instance().StartServer();
        NetworkCommissioning::BLWiFiDriver::GetInstance().SaveConfiguration();
        if (!GetAppTask().mIsConnected && ConnectivityMgr().IsWiFiStationConnected())
        {
            GetAppTask().PostEvent(AppTask::APP_EVENT_SYS_PROVISIONED);
            GetAppTask().mIsConnected = true;
        }
        break;

    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();
        }

        if (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned)
        {
            ChipLogProgress(NotSpecified, "Initializing route hook...");
            bl_route_hook_init();
        }
        break;
#endif

        break;
    default:
        break;
    }
}

CHIP_ERROR PlatformManagerImpl::PlatformInit(void)
{
#if CONFIG_ENABLE_CHIP_SHELL || PW_RPC_ENABLED
    uartInit();
#endif

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

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

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

    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

#ifdef OTA_ENABLED
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    OTAConfig::Init();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
#endif // OTA_ENABLED

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

    vTaskResume(GetAppTask().sAppTaskHandle);

    return CHIP_NO_ERROR;
}
