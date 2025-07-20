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
#include "AppTaskBase.h"
#include "AppEvent.h"
#include "CHIPDeviceManager.h"
#include "CommonDeviceCallbacks.h"

#include <app/server/Dnssd.h>
#include <lib/dnssd/Advertiser.h>

#include <data-model-providers/codegen/Instance.h>
#include <setup_payload/OnboardingCodesUtil.h>

#include <app/clusters/network-commissioning/network-commissioning.h>

#include <platform/CommissionableDataProvider.h>

#include <lib/core/ErrorStr.h>

#include <platform/DeviceInstanceInfoProvider.h>

#include <DeviceInfoProviderImpl.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/Zephyr/DeviceInstanceInfoProviderImpl.h>

#if CONFIG_NET_L2_OPENTHREAD
#include <inet/EndPointStateOpenThread.h>
#include <lib/support/ThreadOperationalDataset.h>
#endif

#if CONFIG_CHIP_APP_WIFI_CONNECT_AT_BOOT
#include "WifiConnect.h"
#endif

#if CONFIG_OPERATIONAL_KEYSTORE
#include "OperationalKeystore.h"
#endif

#if CONFIG_CHIP_OTA_PROVIDER
#include <OTAProvider.h>
#endif

#if CONFIG_DIAG_LOGS_DEMO
#include "DiagnosticLogsDemo.h"
#endif

#if CONFIG_LOW_POWER
#include "LowPower.h"
#include "PWR_Interface.h"
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OTARequestorInitiator.h"
#endif

#if CONFIG_CHIP_TEST_EVENT && CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/OTATestEventTriggerDelegate.h>
#endif

#ifdef ENABLE_CHIP_SHELL
#include <lib/shell/commands/WiFi.h>
#endif

#ifdef SMOKE_CO_ALARM
#include <app/TestEventTriggerDelegate.h>
#include <app/clusters/smoke-co-alarm-server/SmokeCOTestEventTriggerHandler.h>
#endif

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/TestEventTriggerDelegate.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_TBR
#include "platform/OpenThread/GenericThreadBorderRouterDelegate.h"
#include <app/clusters/thread-border-router-management-server/thread-border-router-management-server.h>
#endif

#ifndef CONFIG_THREAD_DEVICE_TYPE
#define CONFIG_THREAD_DEVICE_TYPE kThreadDeviceType_Router
#endif

extern "C" uint64_t settingsNVSLoadTimeMs;
extern "C" uint64_t settingsNVSLoadCount;

using namespace chip;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::app::Clusters;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

#if CONFIG_CHIP_WIFI || CHIP_DEVICE_CONFIG_ENABLE_WPA
app::Clusters::NetworkCommissioning::Instance
    sNetworkCommissioningInstance(0, chip::Zephyr::App::GetAppTask().GetWifiDriverInstance());
#endif
#if CHIP_CONFIG_ENABLE_ICD_SERVER || (CONFIG_CHIP_TEST_EVENT && CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR)
static uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                                                                          0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                                                                          0xcc, 0xdd, 0xee, 0xff };
#endif

#if CONFIG_NET_L2_OPENTHREAD
void LockOpenThreadTask(void)
{
    chip::Zephyr::App::GetAppTask().AppMatter_DisallowDeviceToSleep();
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
    chip::Zephyr::App::GetAppTask().AppMatter_AllowDeviceToSleep();
}
#endif

void chip::Zephyr::App::AppTaskBase::InitServer(intptr_t arg)
{
    GetAppTask().PreInitMatterServerInstance();

#if CONFIG_CHIP_TEST_EVENT && CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    static OTATestEventTriggerDelegate testEventTriggerDelegate{ ByteSpan(sTestEventTriggerEnableKey) };
    initParams.testEventTriggerDelegate = &testEventTriggerDelegate;
#endif

#ifdef SMOKE_CO_ALARM
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    static SmokeCOTestEventTriggerHandler sSmokeCOTestEventTriggerHandler{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
    VerifyOrDie(sTestEventTriggerDelegate.AddHandler(&sSmokeCOTestEventTriggerHandler) == CHIP_NO_ERROR);
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
#endif

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    static SimpleTestEventTriggerDelegate sTestEventTriggerDelegate{};
    VerifyOrDie(sTestEventTriggerDelegate.Init(ByteSpan(sTestEventTriggerEnableKey)) == CHIP_NO_ERROR);
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
#endif

#if CONFIG_OPERATIONAL_KEYSTORE
    initParams.operationalKeystore = chip::Zephyr::App::OperationalKeystore::GetInstance();
#endif
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.dataModelProvider = app::CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

#if CONFIG_NET_L2_OPENTHREAD
    // Init ZCL Data Model and start server
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
#endif

    VerifyOrDie((chip::Server::GetInstance().Init(initParams)) == CHIP_NO_ERROR);
    auto * persistentStorage = &Server::GetInstance().GetPersistentStorage();
#if CONFIG_OPERATIONAL_KEYSTORE
    chip::Zephyr::App::OperationalKeystore::Init(persistentStorage);
#endif

    gExampleDeviceInfoProvider.SetStorageDelegate(persistentStorage);
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    GetAppTask().PostInitMatterServerInstance();
    ChipLogDetail(DeviceLayer, "finishing init");

#if CONFIG_DIAG_LOGS_DEMO
    chip::Zephyr::App::DiagnosticLogsDemo::DisplayUsage();
#endif

#if CONFIG_CHIP_OTA_PROVIDER
    InitOTAServer();
#endif

#if CONFIG_CHIP_APP_WIFI_CONNECT_AT_BOOT
    VerifyOrDie(WifiConnectAtboot(chip::Zephyr::App::GetAppTask().GetWifiDriverInstance()) == CHIP_NO_ERROR);
#endif
}

CHIP_ERROR chip::Zephyr::App::AppTaskBase::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Init Chip memory management before the stack */
    err = chip::Platform::MemoryInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "chip::Platform::MemoryInit() failed: %s", ErrorStr(err));
        goto exit;
    }

#if CONFIG_LOW_POWER
    chip::Zephyr::App::LowPower::Init();
#endif

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Pre Factory Data Provider init failed");
        goto exit;
    }

    /*
     * Initialize the CHIP stack.
     * Would also initialize all required platform modules
     */
    err = PlatformMgr().InitChipStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "PlatformMgr().InitChipStack() failed: %s", ErrorStr(err));
        goto exit;
    }

    /* Initialize Matter factory data after initializing the Matter stack */
    err = InitFactoryDataProvider();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Factory Data Provider init failed");
        goto exit;
    }

    /*
     * Register all application callbacks allowing to be informed of stack events
     */
    err = CHIPDeviceManager::GetInstance().Init(&GetDeviceCallbacks());
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "CHIPDeviceManager.Init() failed: %s", ErrorStr(err));
        goto exit;
    }
    /* Make sure to initialize the Matter CLI which will include the ot-cli first.
     * In fact it is mandatory to enable first the ot-cli before initializing the Matter openthread layer
     * which would modify some contexts of the openthread instance.
     */
    err = AppMatter_Register();
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "Error during APP features registration"));

#if CONFIG_NET_L2_OPENTHREAD
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during ThreadStackMgr().InitThreadStack()");
        return err;
    }

    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::CONFIG_THREAD_DEVICE_TYPE);
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }
#endif

    /*
     * Schedule an event to the Matter stack to initialize
     * the ZCL Data Model and start server
     */
    PlatformMgr().ScheduleWork(InitServer, 0);

#if CONFIG_CHIP_WIFI || CHIP_DEVICE_CONFIG_ENABLE_WPA
    sNetworkCommissioningInstance.Init();
#ifdef ENABLE_CHIP_SHELL
    Shell::SetWiFiDriver(chip::Zephyr::App::GetAppTask().GetWifiDriverInstance());
#endif
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    if (err == CHIP_NO_ERROR)
    {
        /* If an update is under test make it permanent */
        OTARequestorInitiator::Instance().HandleSelfTest();
    }
#endif

    ConfigurationMgr().LogDeviceConfig();

    // QR code will be used with CHIP Tool
    PrintOnboardingInfo();

    PrintCurrentVersion();

    /* Start a task to run the CHIP Device event loop. */
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during PlatformMgr().StartEventLoopTask()");
        goto exit;
    }

#if CONFIG_NET_L2_OPENTHREAD
    // Start OpenThread task
    err = ThreadStackMgrImpl().StartThreadTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during ThreadStackMgrImpl().StartThreadTask()");
    }
#endif

exit:
    return err;
}

void chip::Zephyr::App::AppTaskBase::StartCommissioning(intptr_t arg)
{
    /* Check the status of the commissioning */
    if (ConfigurationMgr().IsFullyProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Device already commissioned");
    }
    else if (chip::Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen())
    {
        ChipLogProgress(DeviceLayer, "Commissioning window already opened");
    }
    else
    {
        chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    }
}

void chip::Zephyr::App::AppTaskBase::StopCommissioning(intptr_t arg)
{
    /* Check the status of the commissioning */
    if (ConfigurationMgr().IsFullyProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Device already commissioned");
    }
    else if (!chip::Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen())
    {
        ChipLogProgress(DeviceLayer, "Commissioning window not opened");
    }
    else
    {
        chip::Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
    }
}

void chip::Zephyr::App::AppTaskBase::SwitchCommissioningState(intptr_t arg)
{
    /* Check the status of the commissioning */
    if (ConfigurationMgr().IsFullyProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Device already commissioned");
    }
    else if (!chip::Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen())
    {
        chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
    }
    else
    {
        chip::Server::GetInstance().GetCommissioningWindowManager().CloseCommissioningWindow();
    }
}

void chip::Zephyr::App::AppTaskBase::StartCommissioningHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(StartCommissioning, 0);
}

void chip::Zephyr::App::AppTaskBase::StopCommissioningHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(StopCommissioning, 0);
}

void chip::Zephyr::App::AppTaskBase::SwitchCommissioningStateHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(SwitchCommissioningState, 0);
}

void chip::Zephyr::App::AppTaskBase::FactoryResetHandler(void)
{
    /* Emit the ShutDown event before factory reset */
    chip::Server::GetInstance().GenerateShutDownEvent();
    chip::Server::GetInstance().ScheduleFactoryReset();
}

void chip::Zephyr::App::AppTaskBase::AppMatter_DisallowDeviceToSleep(void)
{
#if CONFIG_LOW_POWER
    PWR_DisallowDeviceToSleep();
#endif
}

void chip::Zephyr::App::AppTaskBase::AppMatter_AllowDeviceToSleep(void)
{
#if CONFIG_LOW_POWER
    PWR_AllowDeviceToSleep();
#endif
}

void chip::Zephyr::App::AppTaskBase::PrintOnboardingInfo()
{
#if CONFIG_NETWORK_LAYER_BLE
    ChipLogDetail(AppServer, "Retrieving BLE Info");
    auto flags = chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE);
#else
    auto flags = chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kOnNetwork);
#endif /* CONFIG_NETWORK_LAYER_BLE */

    chip::PayloadContents payload;
    CHIP_ERROR err = GetPayloadContents(payload, flags);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "GetPayloadContents() failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
#if CONFIG_USER_ACTION_REQUIRED
    payload.commissioningFlow = chip::CommissioningFlow::kUserActionRequired;
#endif
    PrintOnboardingCodes(payload);
}

void chip::Zephyr::App::AppTaskBase::PrintCurrentVersion()
{
    // Print the current software version
    char currentSoftwareVer[ConfigurationManager::kMaxSoftwareVersionStringLength + 1] = { 0 };
    auto err = ConfigurationMgr().GetSoftwareVersionString(currentSoftwareVer, sizeof(currentSoftwareVer));
    ReturnOnFailure(err);

    uint32_t currentVersion;
    err = ConfigurationMgr().GetSoftwareVersion(currentVersion);
    ReturnOnFailure(err);

    ChipLogProgress(DeviceLayer, "Current Software Version: %s, %d", currentSoftwareVer, static_cast<int>(currentVersion));
}

CHIP_ERROR chip::Zephyr::App::AppTaskBase::InitFactoryDataProvider(void)
{
#if CONFIG_CHIP_FACTORY_DATA
#if CONFIG_CHIP_ENCRYPTED_FACTORY_DATA
    FactoryDataPrvdImpl().SetEncryptionMode(FactoryDataProvider::encrypt_ecb);
    FactoryDataPrvdImpl().SetAes128Key(&aes128TestKey[0]);
#endif /* CONFIG_CHIP_ENCRYPTED_FACTORY_DATA */
    ReturnErrorOnFailure(FactoryDataPrvdImpl().Init());
    SetDeviceInstanceInfoProvider(&FactoryDataPrvd());
    SetDeviceAttestationCredentialsProvider(&FactoryDataPrvd());
    SetCommissionableDataProvider(&FactoryDataPrvd());
#else
    SetDeviceInstanceInfoProvider(&DeviceInstanceInfoProviderMgrImpl());
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif /* CONFIG_CHIP_FACTORY_DATA */
    return CHIP_NO_ERROR;
}
