/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2024 NXP
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
#include "AppFactoryData.h"
#include "CHIPDeviceManager.h"
#include "CommonDeviceCallbacks.h"

#include <app/server/Dnssd.h>
#include <lib/dnssd/Advertiser.h>

#include <DeviceInfoProviderImpl.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/util/attribute-storage.h>

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/CommissionableDataProvider.h>

#include "lib/core/ErrorStr.h"

#include <platform/DeviceInstanceInfoProvider.h>

#ifdef EMBER_AF_PLUGIN_BINDING
#include "binding-handler.h"
#endif

#if CONFIG_NET_L2_OPENTHREAD
#include <inet/EndPointStateOpenThread.h>
#include <lib/support/ThreadOperationalDataset.h>
#endif

#if CONFIG_CHIP_TCP_DOWNLOAD
#include "TcpDownload.h"
#endif

#if CONFIG_CHIP_OTA_PROVIDER
#include <OTAProvider.h>
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OTARequestorInitiator.h"
#endif

#if CONFIG_CHIP_TEST_EVENT && CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include <app/clusters/ota-requestor/OTATestEventTriggerDelegate.h>
#endif

using namespace chip;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::app::Clusters;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

#if CONFIG_CHIP_WIFI || CHIP_DEVICE_CONFIG_ENABLE_WPA
app::Clusters::NetworkCommissioning::Instance sNetworkCommissioningInstance(0,
                                                                            chip::NXP::App::GetAppTask().GetWifiDriverInstance());
#endif

#if CONFIG_CHIP_TEST_EVENT && CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                                                                                          0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb,
                                                                                          0xcc, 0xdd, 0xee, 0xff };
#endif

#if CONFIG_NET_L2_OPENTHREAD
void LockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}
#endif

void chip::NXP::App::AppTaskBase::InitServer(intptr_t arg)
{
    GetAppTask().PreInitMatterServerInstance();

#if CONFIG_CHIP_TEST_EVENT && CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    static OTATestEventTriggerDelegate testEventTriggerDelegate{ ByteSpan(sTestEventTriggerEnableKey) };
    initParams.testEventTriggerDelegate = &testEventTriggerDelegate;
#endif
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
#if CONFIG_NET_L2_OPENTHREAD
    // Init ZCL Data Model and start server
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
#endif

    VerifyOrDie((chip::Server::GetInstance().Init(initParams)) == CHIP_NO_ERROR);

    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);

    GetAppTask().PostInitMatterServerInstance();

#if CONFIG_CHIP_OTA_PROVIDER
    InitOTAServer();
#endif
}

CHIP_ERROR chip::NXP::App::AppTaskBase::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Init Chip memory management before the stack */
    chip::Platform::MemoryInit();

    /* Initialize Matter factory data before initializing the Matter stack */
    err = AppFactoryData_PreMatterStackInit();

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
    err = AppFactoryData_PostMatterStackInit();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Post Factory Data Provider init failed");
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

#if CONFIG_NET_L2_OPENTHREAD
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during ThreadStackMgr().InitThreadStack()");
        return err;
    }

    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
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

#ifdef EMBER_AF_PLUGIN_BINDING
    /* Init binding handlers */
    err = InitBindingHandlers();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "InitBindingHandlers failed: %s", ErrorStr(err));
        goto exit;
    }
#endif

#if CONFIG_CHIP_WIFI || CHIP_DEVICE_CONFIG_ENABLE_WPA
    sNetworkCommissioningInstance.Init();
#endif
#if CONFIG_CHIP_OTA_REQUESTOR
    if (err == CHIP_NO_ERROR)
    {
        /* If an update is under test make it permanent */
        OTARequestorInitiator::Instance().HandleSelfTest();
    }
#endif

    err = AppMatter_Register();
    if (err != CHIP_NO_ERROR)
    {
        goto exit;
    }

    ConfigurationMgr().LogDeviceConfig();

    // QR code will be used with CHIP Tool
#if CONFIG_NETWORK_LAYER_BLE
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kOnNetwork));
#endif /* CONFIG_NETWORK_LAYER_BLE */

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

#if CONFIG_CHIP_TCP_DOWNLOAD
    EnableTcpDownloadComponent();
#endif

exit:
    return err;
}
