/*
 *
 *    Copyright (c) 2021-2023 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
 *    Copyright 2023 NXP
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
#include "AppTask.h"
#include "AppEvent.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "binding-handler.h"
#include "lib/core/ErrorStr.h"
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <lib/dnssd/Advertiser.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/nxp/common/NetworkCommissioningDriver.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

#include <DeviceInfoProviderImpl.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/OnboardingCodesUtil.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/internal/DeviceNetworkInfo.h>

#include <app/util/attribute-storage.h>

#include "AppFactoryData.h"
#include "AppMatterButton.h"
#include "AppMatterCli.h"

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OTARequestorInitiator.h"
#endif

#if ENABLE_OTA_PROVIDER
#include <OTAProvider.h>
#endif

#if CHIP_ENABLE_OPENTHREAD
#include <inet/EndPointStateOpenThread.h>
#endif

#if TCP_DOWNLOAD
#include "TcpDownload.h"
#endif

#ifndef APP_TASK_STACK_SIZE
#define APP_TASK_STACK_SIZE ((configSTACK_DEPTH_TYPE) 6144 / sizeof(portSTACK_TYPE))
#endif
#ifndef APP_TASK_PRIORITY
#define APP_TASK_PRIORITY 2
#endif
#define APP_EVENT_QUEUE_SIZE 10

static QueueHandle_t sAppEventQueue;

using namespace chip;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::app::Clusters;

chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
namespace {
chip::app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */,
                                      &(::chip::DeviceLayer::NetworkCommissioning::NXPWiFiDriver::GetInstance()));
} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

AppTask AppTask::sAppTask;

CHIP_ERROR AppTask::StartAppTask()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TaskHandle_t taskHandle;

    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        err = CHIP_ERROR_NO_MEMORY;
        ChipLogError(DeviceLayer, "Failed to allocate app event queue");
        assert(err == CHIP_NO_ERROR);
    }

    if (xTaskCreate(&AppTask::AppTaskMain, "AppTaskMain", APP_TASK_STACK_SIZE, &sAppTask, APP_TASK_PRIORITY, &taskHandle) != pdPASS)
    {
        err = CHIP_ERROR_NO_MEMORY;
        ChipLogError(DeviceLayer, "Failed to start app task");
        assert(err == CHIP_NO_ERROR);
    }

    return err;
}

#if CHIP_ENABLE_OPENTHREAD
void LockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}
#endif

void AppTask::InitServer(intptr_t arg)
{
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

#if CHIP_ENABLE_OPENTHREAD
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

#ifdef DEVICE_TYPE_ALL_CLUSTERS
    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);
#endif /* DEVICE_TYPE_ALL_CLUSTERS */

#if ENABLE_OTA_PROVIDER
    InitOTAServer();
#endif
}

CHIP_ERROR AppTask::Init()
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
    err = CHIPDeviceManager::GetInstance().Init(&deviceCallbacks);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "CHIPDeviceManager.Init() failed: %s", ErrorStr(err));
        goto exit;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    ConnectivityMgrImpl().StartWiFiManagement();
#endif

#if CHIP_ENABLE_OPENTHREAD
    err = ThreadStackMgr().InitThreadStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during ThreadStackMgr().InitThreadStack()");
        goto exit;
    }

    err = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
    if (err != CHIP_NO_ERROR)
    {
        goto exit;
    }
#endif

    /*
     * Schedule an event to the Matter stack to initialize
     * the ZCL Data Model and start server
     */
    PlatformMgr().ScheduleWork(InitServer, 0);

    /* Init binding handlers */
    err = InitBindingHandlers();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "InitBindingHandlers failed: %s", ErrorStr(err));
        goto exit;
    }

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
    NetWorkCommissioningInstInit();
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    /* If an update is under test make it permanent */
    OTARequestorInitiator::Instance().HandleSelfTest();
#endif

    /* Register Matter CLI cmds */
    err = AppMatterCli_RegisterCommands();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during AppMatterCli_RegisterCommands");
        goto exit;
    }
    /* Register Matter buttons */
    err = AppMatterButton_registerButtons();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during AppMatterButton_registerButtons");
        goto exit;
    }

    err = DisplayDeviceInformation();
    if (err != CHIP_NO_ERROR)
        goto exit;

    /* Start a task to run the CHIP Device event loop. */
    err = PlatformMgr().StartEventLoopTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during PlatformMgr().StartEventLoopTask()");
        goto exit;
    }

#if CHIP_ENABLE_OPENTHREAD
    // Start OpenThread task
    err = ThreadStackMgrImpl().StartThreadTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during ThreadStackMgrImpl().StartThreadTask()");
        goto exit;
    }
#endif

#if TCP_DOWNLOAD
    EnableTcpDownloadComponent();
#endif

exit:
    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppTask * task = (AppTask *) pvParameter;
    CHIP_ERROR err;
    AppEvent event;

    ChipLogProgress(DeviceLayer, "Welcome to NXP All Clusters Demo App");

    err = task->Init();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "AppTask.Init() failed");
        assert(err == CHIP_NO_ERROR);
    }

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, portMAX_DELAY);
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
    }
}

CHIP_ERROR AppTask::DisplayDeviceInformation(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint16_t discriminator;
    uint32_t setupPasscode;
    uint16_t vendorId;
    uint16_t productId;
    char currentSoftwareVer[ConfigurationManager::kMaxSoftwareVersionStringLength + 1];

    err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Couldn't get discriminator: %s", ErrorStr(err));
        goto exit;
    }
    ChipLogProgress(DeviceLayer, "Setup discriminator: %u (0x%x)", discriminator, discriminator);

    err = GetCommissionableDataProvider()->GetSetupPasscode(setupPasscode);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Couldn't get setupPasscode: %s", ErrorStr(err));
        goto exit;
    }
    ChipLogProgress(DeviceLayer, "Setup passcode: %lu (0x%lx)", setupPasscode, setupPasscode);

    err = GetDeviceInstanceInfoProvider()->GetVendorId(vendorId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Couldn't get vendorId: %s", ErrorStr(err));
        goto exit;
    }
    ChipLogProgress(DeviceLayer, "Vendor ID: %u (0x%x)", vendorId, vendorId);

    err = GetDeviceInstanceInfoProvider()->GetProductId(productId);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Couldn't get productId: %s", ErrorStr(err));
        goto exit;
    }
    ChipLogProgress(DeviceLayer, "nProduct ID: %u (0x%x)", productId, productId);

    // QR code will be used with CHIP Tool
#if CONFIG_NETWORK_LAYER_BLE
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kOnNetwork));
#endif /* CONFIG_NETWORK_LAYER_BLE */

    err = ConfigurationMgr().GetSoftwareVersionString(currentSoftwareVer, sizeof(currentSoftwareVer));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Get current software version error");
        goto exit;
    }

    ChipLogProgress(DeviceLayer, "Current Software Version: %s", currentSoftwareVer);

exit:
    return err;
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 0))
        {
            ChipLogError(DeviceLayer, "Failed to post event to app task event queue");
        }
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Event received with no handler. Dropping event.");
    }
}

void AppTask::StartCommissioning(intptr_t arg)
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

void AppTask::StopCommissioning(intptr_t arg)
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

void AppTask::SwitchCommissioningState(intptr_t arg)
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

void AppTask::StartCommissioningHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(StartCommissioning, 0);
}

void AppTask::StopCommissioningHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(StopCommissioning, 0);
}

void AppTask::SwitchCommissioningStateHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(SwitchCommissioningState, 0);
}

void AppTask::FactoryResetHandler(void)
{
    /* Emit the ShutDown event before factory reset */
    chip::Server::GetInstance().GenerateShutDownEvent();
    chip::Server::GetInstance().ScheduleFactoryReset();
}
