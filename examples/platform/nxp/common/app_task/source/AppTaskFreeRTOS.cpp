/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
#include "AppTaskFreeRTOS.h"
#include <app/server/Dnssd.h>
#include <lib/dnssd/Advertiser.h>

#include "ButtonRegistration.h"

#include "CHIPDeviceManager.h"
#include <app/server/Server.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/nxp/common/NetworkCommissioningDriver.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

#ifdef ENABLE_CHIP_SHELL
#include "AppCLIBase.h"
#endif

#if CONFIG_ENABLE_FEEDBACK
#include "UserInterfaceFeedback.h"
#endif

#if CONFIG_ENABLE_PW_RPC
#include "AppRpc.h"
#endif

#include <platform/CHIPDeviceLayer.h>

#include <platform/internal/DeviceNetworkInfo.h>

#if WIFI_CONNECT_TASK
#include "WifiConnect.h"
#endif

#ifndef APP_TASK_STACK_SIZE
#define APP_TASK_STACK_SIZE ((configSTACK_DEPTH_TYPE) 6144 / sizeof(portSTACK_TYPE))
#endif

#ifndef APP_TASK_PRIORITY
#define APP_TASK_PRIORITY 2
#endif

#ifndef APP_EVENT_QUEUE_SIZE
#define APP_EVENT_QUEUE_SIZE 10
#endif

#ifndef APP_QUEUE_TICKS_TO_WAIT
#define APP_QUEUE_TICKS_TO_WAIT portMAX_DELAY
#endif

using namespace chip;
using namespace chip::TLV;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceManager;
using namespace ::chip::app::Clusters;

#if CHIP_DEVICE_CONFIG_ENABLE_WPA

chip::DeviceLayer::NetworkCommissioning::WiFiDriver * chip::NXP::App::AppTaskFreeRTOS::GetWifiDriverInstance()
{
    return static_cast<chip::DeviceLayer::NetworkCommissioning::WiFiDriver *>(
        &(::chip::DeviceLayer::NetworkCommissioning::NXPWiFiDriver::GetInstance()));
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

CHIP_ERROR chip::NXP::App::AppTaskFreeRTOS::AppMatter_Register()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    /* Register Matter CLI cmds */
#ifdef ENABLE_CHIP_SHELL
    err = chip::NXP::App::GetAppCLI().Init();
    VerifyOrReturnError(err == CHIP_NO_ERROR, err, ChipLogError(DeviceLayer, "Error during CLI init"));
    AppMatter_RegisterCustomCliCommands();
#endif

#if CONFIG_ENABLE_FEEDBACK
    FeedbackMgr().Init();
#endif

    /* Register Matter buttons */
    err = chip::NXP::App::RegisterButtons();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during button registration");
        return err;
    }
    return err;
}

CHIP_ERROR chip::NXP::App::AppTaskFreeRTOS::Start()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    TaskHandle_t taskHandle;

#if CONFIG_ENABLE_PW_RPC
    chip::NXP::App::Rpc::Init();
#endif

    appEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (appEventQueue == NULL)
    {
        err = CHIP_ERROR_NO_MEMORY;
        ChipLogError(DeviceLayer, "Failed to allocate app event queue");
        assert(err == CHIP_NO_ERROR);
    }

    ticksToWait = APP_QUEUE_TICKS_TO_WAIT;

#if FSL_OSA_MAIN_FUNC_ENABLE
    /* When OSA is used, this code will be called from within the startup_task
     * and the scheduler will be started at this point. Just call AppTaskMain to
     * start the main loop instead of creating a task, since we are already in it.
     * Task parameters are configured through SDK flags:
     *  - gMainThreadPriority_c
     *  - gMainThreadStackSize_c
     */
    AppTaskFreeRTOS::AppTaskMain(this);
#else
    /* AppTaskMain function will loss actual object instance, give it as parameter */
    if (xTaskCreate(&AppTaskFreeRTOS::AppTaskMain, "AppTaskMain", APP_TASK_STACK_SIZE, this, APP_TASK_PRIORITY, &taskHandle) !=
        pdPASS)
    {
        err = CHIP_ERROR_NO_MEMORY;
        ChipLogError(DeviceLayer, "Failed to start app task");
        assert(err == CHIP_NO_ERROR);
    }
#endif // FSL_OSA_TASK_ENABLE

    return err;
}

void chip::NXP::App::AppTaskFreeRTOS::AppTaskMain(void * pvParameter)
{
    CHIP_ERROR err;
    AppEvent event;

    /* AppTaskMain function will loss AppTask object instance (FreeRTOS task context), AppTask instance is given as parameter */
    AppTaskFreeRTOS * sAppTask = static_cast<AppTaskFreeRTOS *>(pvParameter);

    sAppTask->PreInitMatterStack();
    err = sAppTask->Init();
    VerifyOrDieWithMsg(err == CHIP_NO_ERROR, DeviceLayer, "AppTask.Init() failed");
    sAppTask->PostInitMatterStack();

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppTask->appEventQueue, &event, sAppTask->ticksToWait);
        while (eventReceived == pdTRUE)
        {
            sAppTask->DispatchEvent(event);
            eventReceived = xQueueReceive(sAppTask->appEventQueue, &event, 0);
        }
#if CONFIG_ENABLE_FEEDBACK
        FeedbackMgr().DisplayInLoop();
#endif
    }
}

void chip::NXP::App::AppTaskFreeRTOS::PostEvent(const AppEvent & event)
{
    if (appEventQueue != NULL)
    {
        if (__get_IPSR())
        {
            portBASE_TYPE taskToWake = pdFALSE;
            if (!xQueueSendToFrontFromISR(appEventQueue, &event, &taskToWake))
            {
                ChipLogError(DeviceLayer, "Failed to post event to app task event queue from ISR");
            }

            portYIELD_FROM_ISR(taskToWake);
        }
        else
        {
            if (!xQueueSend(appEventQueue, &event, 0))
            {
                ChipLogError(DeviceLayer, "Failed to post event to app task event queue");
            }
        }
    }
}

void chip::NXP::App::AppTaskFreeRTOS::DispatchEvent(const AppEvent & event)
{
    if (event.Handler)
    {
        event.Handler(event);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Event received with no handler. Dropping event.");
    }
}
