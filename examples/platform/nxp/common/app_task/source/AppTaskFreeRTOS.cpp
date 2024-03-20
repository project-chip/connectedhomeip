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

#include "AppMatterButton.h"
#include "AppMatterCli.h"

#include "CHIPDeviceManager.h"
#include <app/server/Server.h>

#if CHIP_DEVICE_CONFIG_ENABLE_WPA
#include <platform/nxp/common/NetworkCommissioningDriver.h>
#endif // CHIP_DEVICE_CONFIG_ENABLE_WPA

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
#define APP_EVENT_QUEUE_SIZE 10

static QueueHandle_t sAppEventQueue;

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
    err = AppMatterCli_RegisterCommands();
    AppMatter_RegisterCustomCliCommands();
    AppMatterCli_StartTask();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during AppMatterCli_RegisterCommands");
        return err;
    }
    /* Register Matter buttons */
    err = AppMatterButton_registerButtons();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error during AppMatterButton_registerButtons");
        return err;
    }
    return err;
}

CHIP_ERROR chip::NXP::App::AppTaskFreeRTOS::Start()
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

    /* AppTaskMain function will loss actual object instance, give it as parameter */
    if (xTaskCreate(&AppTaskFreeRTOS::AppTaskMain, "AppTaskMain", APP_TASK_STACK_SIZE, this, APP_TASK_PRIORITY, &taskHandle) !=
        pdPASS)
    {
        err = CHIP_ERROR_NO_MEMORY;
        ChipLogError(DeviceLayer, "Failed to start app task");
        assert(err == CHIP_NO_ERROR);
    }

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
    sAppTask->PostInitMatterStack();
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
            sAppTask->DispatchEvent(event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }
    }
}

void chip::NXP::App::AppTaskFreeRTOS::PostEvent(const AppEvent & event)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, &event, 0))
        {
            ChipLogError(DeviceLayer, "Failed to post event to app task event queue");
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

void chip::NXP::App::AppTaskFreeRTOS::StartCommissioning(intptr_t arg)
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

void chip::NXP::App::AppTaskFreeRTOS::StopCommissioning(intptr_t arg)
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

void chip::NXP::App::AppTaskFreeRTOS::SwitchCommissioningState(intptr_t arg)
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

void chip::NXP::App::AppTaskFreeRTOS::StartCommissioningHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(StartCommissioning, 0);
}

void chip::NXP::App::AppTaskFreeRTOS::StopCommissioningHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(StopCommissioning, 0);
}

void chip::NXP::App::AppTaskFreeRTOS::SwitchCommissioningStateHandler(void)
{
    /* Publish an event to the Matter task to always set the commissioning state in the Matter task context */
    PlatformMgr().ScheduleWork(SwitchCommissioningState, 0);
}

void chip::NXP::App::AppTaskFreeRTOS::FactoryResetHandler(void)
{
    /* Emit the ShutDown event before factory reset */
    chip::Server::GetInstance().GenerateShutDownEvent();
    chip::Server::GetInstance().ScheduleFactoryReset();
}
