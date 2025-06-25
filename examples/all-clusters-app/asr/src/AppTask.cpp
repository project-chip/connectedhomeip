/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    Copyright 2021, Cypress Semiconductor Corporation (an Infineon company)
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
#include "AppEvent.h"
#include "ButtonHandler.h"
#include "CHIPDeviceManager.h"
#include "DeviceCallbacks.h"
#include "LEDWidget.h"
#include "init_Matter.h"
#include "qrcodegen.h"
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <app/server/Dnssd.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>
#include <app/util/endpoint-config-api.h>
#include <assert.h>
#include <lega_rtos_api.h>
#include <platform/ASR/NetworkCommissioningDriver.h>
#include <platform/CHIPDeviceLayer.h>
#include <protocols/interaction_model/StatusCode.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <static-supported-modes-manager.h>
#include <static-supported-temperature-levels.h>

using chip::Protocols::InteractionModel::Status;

using namespace ::chip;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceManager;
using namespace ::chip::DeviceLayer;
using namespace ::chip::System;

LEDWidget sLightLED;

namespace {
lega_thread_t sAppTaskHandle;
lega_queue_t sAppEventQueue;
lega_timer_t sAppTimer;

constexpr EndpointId kNetworkCommissioningEndpointMain      = 0;
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;

app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(kNetworkCommissioningEndpointMain /* Endpoint Id */,
                                      &(NetworkCommissioning::ASRWiFiDriver::GetInstance()));
app::Clusters::TemperatureControl::AppSupportedTemperatureLevelsDelegate sAppSupportedTemperatureLevelsDelegate;
app::Clusters::ModeSelect::StaticSupportedModesManager sStaticSupportedModesManager;
} // namespace

AppTask AppTask::sAppTask;

void NetWorkCommissioningInstInit()
{
    sWiFiNetworkCommissioningInstance.Init();

    // We only have network commissioning on endpoint 0.
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
}

static DeviceCallbacks EchoCallbacks;

CHIP_ERROR AppTask::StartAppTask()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    OSStatus status;

    status = lega_rtos_init_queue(&sAppEventQueue, "EventQueue", sizeof(AppEvent), APP_EVENT_QUEUE_SIZE);

    VerifyOrExit(status == kNoErr, err = CHIP_ERROR_NO_MEMORY);

    status = lega_rtos_init_timer(&sAppTimer, 3000, (timer_handler_t) AppTimerCallback, (void *) this);

    VerifyOrExit(status == kNoErr, err = CHIP_ERROR_NO_MEMORY);

    status = lega_rtos_create_thread(&sAppTaskHandle, 2, APP_TASK_NAME, (lega_thread_function_t) AppTaskMain, APP_TASK_STACK_SIZE,
                                     (lega_thread_arg_t) this);

    VerifyOrExit(status == kNoErr, err = APP_ERROR_CREATE_TASK_FAILED);

exit:
    return err;
}

CHIP_ERROR AppTask::Init()
{
    ASR_LOG("App Task started");

    if (MatterInitializer::Init_Matter_Stack(MATTER_DEVICE_NAME) != CHIP_NO_ERROR)
        return CHIP_ERROR_INTERNAL;

    CHIPDeviceManager & deviceMgr = CHIPDeviceManager::GetInstance();

    if (deviceMgr.Init(&EchoCallbacks) != CHIP_NO_ERROR)
        return CHIP_ERROR_INTERNAL;

    if (MatterInitializer::Init_Matter_Server() != CHIP_NO_ERROR)
        return CHIP_ERROR_INTERNAL;

    NetWorkCommissioningInstInit();

    ASR_LOG("Current Software Version: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    ConfigurationMgr().LogDeviceConfig();

    // Print setup info
#if CONFIG_NETWORK_LAYER_BLE
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
#else
    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kOnNetwork));
#endif /* CONFIG_NETWORK_LAYER_BLE */

    app::Clusters::TemperatureControl::SetInstance(&sAppSupportedTemperatureLevelsDelegate);
    app::Clusters::ModeSelect::setSupportedModesManager(&sStaticSupportedModesManager);

    return CHIP_NO_ERROR;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;
    CHIP_ERROR err = sAppTask.Init();

    ButtonHandler::Init();

    sLightLED.Init(LIGHT_LED);

    lega_rtos_start_timer(&sAppTimer);

    if (err != CHIP_NO_ERROR)
    {
        ASR_LOG("AppTask.Init() failed");
        appError(err);
    }

    while (true)
    {
        /* Check the event queue. */
        if (lega_rtos_is_queue_empty(&sAppEventQueue))
        {
            lega_rtos_delay_milliseconds(1);
            continue;
        }

        /* Pop one event from the event queue. */
        lega_rtos_pop_from_queue(&sAppEventQueue, &event, LEGA_WAIT_FOREVER);

        sAppTask.DispatchEvent(&event);
    }
}

void AppTask::AppEventHandler(AppEvent * aEvent)
{
    static char lightState;
    int16_t temperature = 2550;
    int16_t humidity    = 5000;
    int16_t pressure    = 1234;

    switch (aEvent->Type)
    {
    case AppEvent::kEventType_Timer: {
        chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Set(
            /* endpoint ID */ 1, /* temperature in 0.01*C */ int16_t(temperature));

        chip::app::Clusters::RelativeHumidityMeasurement::Attributes::MeasuredValue::Set(
            /* endpoint ID */ 1, /* humidity in 0.01% */ int16_t(humidity));

        chip::app::Clusters::PressureMeasurement::Attributes::MeasuredValue::Set(
            /* endpoint ID */ 1, /* pressure in 0.01 */ int16_t(pressure));

        break;
    }

    case AppEvent::kEventType_Button: {
        lightState = !lightState;
        /* ON/OFF Light Led based on Button interrupt */
        if (lightState)
        {
            sLightLED.Set(1);
        }
        else
        {
            sLightLED.Set(0);
        }

        /* Update OnOff Cluster state */
        sAppTask.OnOffUpdateClusterState();
        break;
    }

    default:
        break;
    }
}

void AppTask::AppTimerCallback(void * params)
{
    AppEvent timer_event           = {};
    timer_event.Type               = AppEvent::kEventType_Timer;
    timer_event.TimerEvent.Context = nullptr;

    timer_event.Handler = AppEventHandler;
    sAppTask.PostEvent(&timer_event);
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
    ASR_LOG("%s %s\n", btnIdx == SWITCH1_BUTTON ? "btn1" : "btn2", btnAction == BUTTON_PRESSED ? "Pressed" : "Released");

    if (btnIdx != SWITCH1_BUTTON && btnIdx != SWITCH2_BUTTON)
    {
        return;
    }

    AppEvent button_event              = {};
    button_event.Type                  = AppEvent::kEventType_Button;
    button_event.ButtonEvent.ButtonIdx = btnIdx;
    button_event.ButtonEvent.Action    = btnAction;

    if (btnAction == BUTTON_RELEASED)
    {
        button_event.Handler = AppEventHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    OSStatus status = lega_rtos_push_to_queue(&sAppEventQueue, const_cast<AppEvent *>(aEvent), LEGA_NO_WAIT);
    if (kNoErr != status)
    {
        ASR_LOG("Post event failed.");
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
        ASR_LOG("Event received with no handler. Dropping event.");
    }
}

void AppTask::OnOffUpdateClusterState(void)
{
    uint8_t onoff = sLightLED.Get();

    // write the new on/off value
    Status status = app::Clusters::OnOff::Attributes::OnOff::Set(1, onoff);

    if (status != Status::Success)
    {
        ASR_LOG("ERR: updating on/off %x", to_underlying(status));
    }
}

bool lowPowerClusterSleep()
{
    return true;
}
