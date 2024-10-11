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

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app/clusters/identify-server/identify-server.h>

#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <platform/bouffalolab/common/DiagnosticDataProviderImpl.h>
#include <system/SystemClock.h>

#if HEAP_MONITORING
#include "MemMonitoring.h"
#endif

#if CHIP_ENABLE_OPENTHREAD
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/bouffalolab/common/ThreadStackManagerImpl.h>
#include <utils_list.h>
#endif

#if CONFIG_ENABLE_CHIP_SHELL
#include <ChipShellCollection.h>
#include <lib/shell/Engine.h>
#endif

#include <LEDWidget.h>
#include <plat.h>

extern "C" {
#include <bl_gpio.h>
#include <hal_gpio.h>
#include <hosal_gpio.h>
}

#include "AppTask.h"
#include "mboard.h"

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#if CONFIG_ENABLE_CHIP_SHELL
using namespace chip::Shell;
#endif

namespace {

#if defined(BL706_NIGHT_LIGHT) || defined(BL602_NIGHT_LIGHT)
ColorLEDWidget sLightLED;
#else
DimmableLEDWidget sLightLED;
#endif

Identify sIdentify = {
    APP_LIGHT_ENDPOINT_ID,
    AppTask::IdentifyStartHandler,
    AppTask::IdentifyStopHandler,
    Clusters::Identify::IdentifyTypeEnum::kLightOutput,
};

} // namespace

AppTask AppTask::sAppTask;
StackType_t AppTask::appStack[APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t AppTask::appTaskStruct;

void StartAppTask(void)
{
    GetAppTask().sAppTaskHandle = xTaskCreateStatic(GetAppTask().AppTaskMain, APP_TASK_NAME, ArraySize(GetAppTask().appStack), NULL,
                                                    APP_TASK_PRIORITY, GetAppTask().appStack, &GetAppTask().appTaskStruct);
    if (GetAppTask().sAppTaskHandle == NULL)
    {
        ChipLogError(NotSpecified, "Failed to create app task");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }
}

#if CONFIG_ENABLE_CHIP_SHELL
void AppTask::AppShellTask(void * args)
{
    Engine::Root().RunMainLoop();
}

CHIP_ERROR AppTask::StartAppShellTask()
{
    static TaskHandle_t shellTask;

    Engine::Root().Init();

    cmd_misc_init();

    xTaskCreate(AppTask::AppShellTask, "chip_shell", 1024 / sizeof(configSTACK_DEPTH_TYPE), NULL, APP_TASK_PRIORITY, &shellTask);

    return CHIP_NO_ERROR;
}
#endif

void AppTask::PostEvent(app_event_t event)
{
    if (xPortIsInsideInterrupt())
    {
        BaseType_t higherPrioTaskWoken = pdFALSE;
        xTaskNotifyFromISR(sAppTaskHandle, event, eSetBits, &higherPrioTaskWoken);
    }
    else
    {
        xTaskNotify(sAppTaskHandle, event, eSetBits);
    }
}

void AppTask::AppTaskMain(void * pvParameter)
{
    app_event_t appEvent;
    bool onoff               = false;
    uint64_t currentHeapFree = 0;

#if !(CHIP_DEVICE_LAYER_TARGET_BL702 && CHIP_DEVICE_CONFIG_ENABLE_ETHERNET)
    sLightLED.Init();
#endif

#ifdef BOOT_PIN_RESET
    ButtonInit();
#else
    /** Without RESET PIN defined, factory reset will be executed if power cycle count(resetCnt) >= APP_REBOOT_RESET_COUNT */
    uint32_t resetCnt      = 0;
    size_t saved_value_len = 0;
    ef_get_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt), &saved_value_len);

    if (resetCnt > APP_REBOOT_RESET_COUNT)
    {
        resetCnt = 0;
        /** To share with RESET PIN logic, mButtonPressedTime is used to recorded resetCnt increased.
         * +1 makes sure mButtonPressedTime is not zero;
         * a power cycle during factory reset confirm time APP_BUTTON_PRESS_LONG will cancel factoryreset */
        GetAppTask().mButtonPressedTime = System::SystemClock().GetMonotonicMilliseconds64().count() + 1;
    }
    else
    {
        resetCnt++;
        GetAppTask().mButtonPressedTime = 0;
    }
    ef_set_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt));
#endif

    GetAppTask().sTimer =
        xTimerCreate("lightTmr", pdMS_TO_TICKS(APP_TIMER_EVENT_DEFAULT_ITVL), false, NULL, AppTask::TimerCallback);
    if (GetAppTask().sTimer == NULL)
    {
        ChipLogError(NotSpecified, "Failed to create timer task");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }

    ChipLogProgress(NotSpecified, "Starting Platform Manager Event Loop");
    CHIP_ERROR ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }

    GetAppTask().PostEvent(APP_EVENT_TIMER);
    GetAppTask().PostEvent(APP_EVENT_LIGHTING_MASK);

    vTaskSuspend(NULL);

    DiagnosticDataProviderImpl::GetDefaultInstance().GetCurrentHeapFree(currentHeapFree);
    ChipLogProgress(NotSpecified, "App Task started, with SRAM heap %lld left\r\n", currentHeapFree);

    while (true)
    {
        appEvent                 = APP_EVENT_NONE;
        BaseType_t eventReceived = xTaskNotifyWait(0, APP_EVENT_ALL_MASK, (uint32_t *) &appEvent, portMAX_DELAY);

        if (eventReceived)
        {
            PlatformMgr().LockChipStack();

            if (APP_EVENT_LIGHTING_MASK & appEvent)
            {
                LightingUpdate(appEvent);
            }

            if (APP_EVENT_BTN_SHORT & appEvent)
            {
                if (Server::GetInstance().GetFabricTable().FabricCount())
                {
                    Clusters::OnOff::Attributes::OnOff::Get(GetAppTask().GetEndpointId(), &onoff);
                    onoff = !onoff;
                    Clusters::OnOff::Attributes::OnOff::Set(GetAppTask().GetEndpointId(), onoff);
                }
                else
                {
                    sLightLED.Toggle();
                }
            }

#ifdef BOOT_PIN_RESET
            if (APP_EVENT_BTN_LONG & appEvent)
            {
                /** Turn off light to indicate button long press for factory reset is confirmed */
                sLightLED.SetOnoff(false);
            }
#endif
            if (APP_EVENT_IDENTIFY_MASK & appEvent)
            {
                IdentifyHandleOp(appEvent);
            }

            if (APP_EVENT_FACTORY_RESET & appEvent)
            {
                DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
            }

            TimerEventHandler(appEvent);

            PlatformMgr().UnlockChipStack();
        }
    }
}

void AppTask::LightingUpdate(app_event_t status)
{
    uint8_t hue, sat;
    bool onoff;
    DataModel::Nullable<uint8_t> v(0);
    EndpointId endpoint = GetAppTask().GetEndpointId();

    if (APP_EVENT_LIGHTING_MASK & status)
    {

        if (Server::GetInstance().GetFabricTable().FabricCount())
        {
            do
            {
                if (Protocols::InteractionModel::Status::Success != Clusters::OnOff::Attributes::OnOff::Get(endpoint, &onoff))
                {
                    break;
                }

                if (Protocols::InteractionModel::Status::Success !=
                    Clusters::LevelControl::Attributes::CurrentLevel::Get(endpoint, v))
                {
                    break;
                }

                if (Protocols::InteractionModel::Status::Success !=
                    Clusters::ColorControl::Attributes::CurrentHue::Get(endpoint, &hue))
                {
                    break;
                }

                if (Protocols::InteractionModel::Status::Success !=
                    Clusters::ColorControl::Attributes::CurrentSaturation::Get(endpoint, &sat))
                {
                    break;
                }

                if (!onoff)
                {
                    sLightLED.SetLevel(0);
                }
                else
                {
                    if (v.IsNull())
                    {
                        v.SetNonNull(254);
                    }
#if defined(BL706_NIGHT_LIGHT) || defined(BL602_NIGHT_LIGHT)
                    sLightLED.SetColor(v.Value(), hue, sat);
#else
                    sLightLED.SetLevel(v.Value());
#endif
                }
            } while (0);
        }
        else
        {
#if defined(BL706_NIGHT_LIGHT) || defined(BL602_NIGHT_LIGHT)
            /** show yellow to indicate not-provision state for extended color light */
            sLightLED.SetColor(254, 35, 254);
#else
            /** show 30% brightness to indicate not-provision state */
            sLightLED.SetLevel(25);
#endif
        }
    }
}

bool AppTask::StartTimer(void)
{
    if (xTimerIsTimerActive(GetAppTask().sTimer))
    {
        CancelTimer();
    }

    if (GetAppTask().mTimerIntvl == 0)
    {
        GetAppTask().mTimerIntvl = APP_TIMER_EVENT_DEFAULT_ITVL;
    }

    if (xTimerChangePeriod(GetAppTask().sTimer, pdMS_TO_TICKS(GetAppTask().mTimerIntvl), pdMS_TO_TICKS(100)) != pdPASS)
    {
        ChipLogProgress(NotSpecified, "Failed to access timer with 100 ms delay.");
    }

    return true;
}

void AppTask::CancelTimer(void)
{
    xTimerStop(GetAppTask().sTimer, 0);
}

void AppTask::TimerCallback(TimerHandle_t xTimer)
{
    GetAppTask().PostEvent(APP_EVENT_TIMER);
}

void AppTask::TimerEventHandler(app_event_t event)
{
    uint32_t pressedTime = 0;

    if (GetAppTask().mButtonPressedTime)
    {
        pressedTime = System::SystemClock().GetMonotonicMilliseconds64().count() - GetAppTask().mButtonPressedTime;
#ifdef BOOT_PIN_RESET
        if (ButtonPressed())
        {
            if (pressedTime > APP_BUTTON_PRESS_LONG)
            {
                GetAppTask().PostEvent(APP_EVENT_BTN_LONG);
            }
            else if (pressedTime >= APP_BUTTON_PRESS_SHORT)
            {
#if defined(BL602_NIGHT_LIGHT) || defined(BL706_NIGHT_LIGHT)
                /** change color to indicate to wait factory reset confirm */
                sLightLED.SetColor(254, 0, 210);
#else
                /** toggle led to indicate to wait factory reset confirm */
                sLightLED.Toggle();
#endif
            }
        }
        else
        {
            if (pressedTime >= APP_BUTTON_PRESS_LONG)
            {
                GetAppTask().PostEvent(APP_EVENT_FACTORY_RESET);
            }
            else if (APP_BUTTON_PRESS_SHORT >= pressedTime && pressedTime >= APP_BUTTON_PRESS_JITTER)
            {
                GetAppTask().PostEvent(APP_EVENT_BTN_SHORT);
            }
            else
            {
                GetAppTask().PostEvent(APP_EVENT_LIGHTING_MASK);
            }

            GetAppTask().mTimerIntvl        = APP_BUTTON_PRESSED_ITVL;
            GetAppTask().mButtonPressedTime = 0;
        }
#else
        if (pressedTime > APP_BUTTON_PRESS_LONG)
        {
            /** factory reset confirm timeout */
            GetAppTask().mButtonPressedTime = 0;
            GetAppTask().PostEvent(APP_EVENT_FACTORY_RESET);
        }
        else
        {
#if defined(BL602_NIGHT_LIGHT) || defined(BL706_NIGHT_LIGHT)
            /** change color to indicate to wait factory reset confirm */
            sLightLED.SetColor(254, 0, 210);
#else
            /** toggle led to indicate to wait factory reset confirm */
            sLightLED.Toggle();
#endif
        }
#endif
    }
#ifdef BOOT_PIN_RESET
    else
    {
        if (ButtonPressed())
        {
            GetAppTask().mTimerIntvl        = APP_BUTTON_PRESSED_ITVL;
            GetAppTask().mButtonPressedTime = System::SystemClock().GetMonotonicMilliseconds64().count();
        }
    }
#endif

    StartTimer();
}

void AppTask::IdentifyStartHandler(Identify *)
{
    GetAppTask().PostEvent(APP_EVENT_IDENTIFY_START);
}

void AppTask::IdentifyStopHandler(Identify *)
{
    GetAppTask().PostEvent(APP_EVENT_IDENTIFY_STOP);
}

void AppTask::IdentifyHandleOp(app_event_t event)
{
    static uint32_t identifyState = 0;

    if (APP_EVENT_IDENTIFY_START & event)
    {
        identifyState = 1;
        ChipLogProgress(NotSpecified, "identify start");
    }

    if ((APP_EVENT_IDENTIFY_IDENTIFY & event) && identifyState)
    {
        sLightLED.Toggle();
        ChipLogProgress(NotSpecified, "identify");
    }

    if (APP_EVENT_IDENTIFY_STOP & event)
    {
        identifyState = 0;
        GetAppTask().PostEvent(APP_EVENT_LIGHTING_MASK);
        ChipLogProgress(NotSpecified, "identify stop");
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
    GetAppTask().PostEvent(APP_EVENT_FACTORY_RESET);
}

#ifdef BOOT_PIN_RESET
static hosal_gpio_dev_t gpio_key = { .port = BOOT_PIN_RESET, .config = INPUT_HIGH_IMPEDANCE, .priv = NULL };

void AppTask::ButtonInit(void)
{
    GetAppTask().mButtonPressedTime = 0;

    hosal_gpio_init(&gpio_key);
    hosal_gpio_irq_set(&gpio_key, HOSAL_IRQ_TRIG_POS_PULSE, GetAppTask().ButtonEventHandler, NULL);
}

bool AppTask::ButtonPressed(void)
{
    uint8_t val = 1;

    hosal_gpio_input_get(&gpio_key, &val);

    return val == 1;
}

void AppTask::ButtonEventHandler(void * arg)
{
    if (ButtonPressed())
    {
        GetAppTask().PostEvent(APP_EVENT_BTN_ISR);
    }
}
#endif
