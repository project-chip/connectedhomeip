/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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
#include "AppConfig.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "init_Matter.h"
#include "qrcodegen.h"

#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <assert.h>
#include <platform/ASR/NetworkCommissioningDriver.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

namespace {
TaskHandle_t sAppTaskHandle;
QueueHandle_t sAppEventQueue;

constexpr EndpointId kThermostatEndpointId = 1;
} // namespace

namespace {
constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointMain /* Endpoint Id */,
                                      &(NetworkCommissioning::ASRWiFiDriver::GetInstance()));
} // namespace

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
}

AppTask AppTask::sAppTask;
static DeviceCallbacks EchoCallbacks;

CHIP_ERROR AppTask::StartAppTask()
{
    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        ASR_LOG("Failed to allocate app event queue");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }
    // Start App task.
    xTaskCreate(AppTaskMain, APP_TASK_NAME, APP_TASK_STACK_SIZE, 0, 2, &sAppTaskHandle);
    return (sAppTaskHandle == nullptr) ? CHIP_APPLICATION_ERROR(0x02) : CHIP_NO_ERROR;
}

CHIP_ERROR AppTask::Init()
{
    if (MatterInitializer::Init_Matter_Stack(MATTER_DEVICE_NAME) != CHIP_NO_ERROR)
        appError(CHIP_ERROR_INTERNAL);

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    if (deviceMgr.Init(&EchoCallbacks) != CHIP_NO_ERROR)
        appError(CHIP_ERROR_INTERNAL);

    if (MatterInitializer::Init_Matter_Server() != CHIP_NO_ERROR)
        appError(CHIP_ERROR_INTERNAL);

    NetWorkCommissioningInstInit();

    ASR_LOG("Current Firmware Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    ConfigurationMgr().LogDeviceConfig();

    // Print setup info
#if CONFIG_NETWORK_LAYER_BLE
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kOnNetwork));
#endif /* CONFIG_NETWORK_LAYER_BLE */

    CHIP_ERROR err;
    err = SensorMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ASR_LOG("SensorMgr Init fail");
        appError(err);
    }
    err = TempMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        ASR_LOG("TempMgr Init fail");
        appError(err);
    }

    return CHIP_NO_ERROR;
}

void AppTask::AppTaskMain(void * pvParameter)
{

    AppEvent event;
    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        ASR_LOG("AppTask.Init() failed");
        appError(err);
    }

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        if (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
        }
    }

    /* Delete task */
    vTaskDelete(NULL);
}

void AppTask::UpdateThermoStatUI(void)
{
    ASR_LOG("Thermostat Status - M:%d T:%d'C H:%d'C C:%d'C", TempMgr().GetMode(), TempMgr().GetCurrentTemp(),
            TempMgr().GetHeatingSetPoint(), TempMgr().GetCoolingSetPoint());
}

void AppTask::UpdateClusterState() {}

void AppTask::PostEvent(const AppEvent * event)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, event, 1))
        {
            ASR_LOG("Failed to post event to app task event queue");
        }
    }
}

void AppTask::DispatchEvent(AppEvent * event)
{
    if (event->Handler)
    {
        event->Handler(event);
    }
    else
    {
        ASR_LOG("Event received with no handler. Dropping event.");
    }
}
