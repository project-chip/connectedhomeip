/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#include <LEDWidget.h>
#include <plat.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/util/attribute-storage.h>

#include <app/server/Dnssd.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/ErrorStr.h>
#include <system/SystemClock.h>

#if HEAP_MONITORING
#include "MemMonitoring.h"
#endif

#if CHIP_ENABLE_OPENTHREAD
#include <ThreadStackManagerImpl.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <utils_list.h>
#endif

#if CONFIG_ENABLE_CHIP_SHELL
#include <ChipShellCollection.h>
#include <lib/shell/Engine.h>
#endif

extern "C" {
#include "board.h"
#include <bl_gpio.h>
#include <easyflash.h>
#include <hal_gpio.h>
#include <hosal_gpio.h>
}

#include "AppTask.h"

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
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LIGHT,
};

} // namespace

using namespace ::chip;
using namespace ::chip::app;
using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;

#if CONFIG_ENABLE_CHIP_SHELL
using namespace chip::Shell;
#endif

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
    bool onoff = false;

    sLightLED.Init();

#ifdef LED_BTN_RESET
    ButtonInit();
#else
    /** Without RESET PIN defined, factory reset will be executed if power cycle count(resetCnt) >= APP_REBOOT_RESET_COUNT */
    uint32_t resetCnt      = 0;
    size_t saved_value_len = 0;
    ef_get_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt), &saved_value_len);
    resetCnt++;
    ef_set_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt));

    /** To share with RESET PIN logic, mButtonPressedTime is used to recorded resetCnt increased.
     * +1 makes sure mButtonPressedTime is not zero */
    GetAppTask().mButtonPressedTime = chip::System::SystemClock().GetMonotonicMilliseconds64().count() + 1;
#endif

    GetAppTask().sTimer = xTimerCreate("lightTmr", pdMS_TO_TICKS(1000), false, NULL, AppTask::TimerCallback);
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

    GetAppTask().PostEvent(AppTask::APP_EVENT_TIMER);
    vTaskSuspend(NULL);

    GetAppTask().mIsConnected = false;

    ChipLogProgress(NotSpecified, "App Task started, with heap %d left\r\n", xPortGetFreeHeapSize());

    while (true)
    {
        appEvent                 = APP_EVENT_NONE;
        BaseType_t eventReceived = xTaskNotifyWait(0, APP_EVENT_ALL_MASK, (uint32_t *) &appEvent, portMAX_DELAY);

        if (eventReceived)
        {
            PlatformMgr().LockChipStack();

            if (APP_EVENT_SYS_PROVISIONED & appEvent)
            {
                LightingUpdate(APP_EVENT_LIGHTING_MASK);
            }

            if (APP_EVENT_BTN_SHORT & appEvent)
            {
                Clusters::OnOff::Attributes::OnOff::Get(GetAppTask().GetEndpointId(), &onoff);
                onoff = !onoff;
                Clusters::OnOff::Attributes::OnOff::Set(GetAppTask().GetEndpointId(), onoff);
                LightingUpdate((app_event_t)(APP_EVENT_LIGHTING_MASK & appEvent));
            }

            if (APP_EVENT_LIGHTING_MASK & appEvent)
            {
                LightingUpdate((app_event_t)(APP_EVENT_LIGHTING_MASK & appEvent));
            }

            if (APP_EVENT_IDENTIFY_MASK & appEvent)
            {
                IdentifyHandleOp(appEvent);
            }

            if (APP_EVENT_SYS_BLE_ADV & appEvent)
            {
                LightingUpdate(APP_EVENT_SYS_BLE_ADV);
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
        do
        {
            if (EMBER_ZCL_STATUS_SUCCESS != Clusters::OnOff::Attributes::OnOff::Get(endpoint, &onoff))
            {
                break;
            }

            if (EMBER_ZCL_STATUS_SUCCESS != Clusters::LevelControl::Attributes::CurrentLevel::Get(endpoint, v))
            {
                break;
            }

            if (EMBER_ZCL_STATUS_SUCCESS != Clusters::ColorControl::Attributes::CurrentHue::Get(endpoint, &hue))
            {
                break;
            }

            if (EMBER_ZCL_STATUS_SUCCESS != Clusters::ColorControl::Attributes::CurrentSaturation::Get(endpoint, &sat))
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
                    // Just pick something.
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
    else if (APP_EVENT_SYS_BLE_ADV & status)
    {
#if defined(BL706_NIGHT_LIGHT) || defined(BL602_NIGHT_LIGHT)
        /** show yellow to indicate BLE advertisement */
        sLightLED.SetColor(254, 35, 254);
#else
        /** show 30% brightness to indicate BLE advertisement */
        sLightLED.SetLevel(25);
#endif
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
        GetAppTask().mTimerIntvl = 1000;
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
    if (event & APP_EVENT_BTN_FACTORY_RESET_PRESS)
    {
        GetAppTask().mTimerIntvl = APP_BUTTON_PRESS_JITTER;
        StartTimer();
    }
    else if (event & APP_EVENT_BTN_FACTORY_RESET_IND)
    {
        if (GetAppTask().mButtonPressedTime)
        {
            GetAppTask().mIsFactoryResetIndicat = true;
#if defined(BL706_NIGHT_LIGHT) || defined(BL602_NIGHT_LIGHT)
            sLightLED.SetColor(254, 0, 210);
#ifndef LED_BTN_RESET
            uint32_t resetCnt               = 0;
            GetAppTask().mButtonPressedTime = 0;
            ef_set_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt));

            vTaskDelay(APP_BUTTON_PRESS_LONG);
            GetAppTask().PostEvent(APP_EVENT_FACTORY_RESET);
#endif
#else
            sLightLED.Toggle();
            GetAppTask().mTimerIntvl = 100;
            StartTimer();
#endif
        }
    }

    if (event & APP_EVENT_BTN_FACTORY_RESET_CANCEL)
    {
        GetAppTask().mTimerIntvl = 1000;
        StartTimer();
        if (GetAppTask().mIsFactoryResetIndicat)
        {
            if (GetAppTask().mIsConnected)
            {
                LightingUpdate(APP_EVENT_LIGHTING_MASK);
            }
            else
            {
                LightingUpdate(APP_EVENT_SYS_BLE_ADV);
            }
        }
        GetAppTask().mIsFactoryResetIndicat = false;
        GetAppTask().mButtonPressedTime     = 0;
    }

    if (APP_EVENT_TIMER & event)
    {
        if (GetAppTask().mButtonPressedTime)
        {
            if (GetAppTask().mIsFactoryResetIndicat)
            {
                if (chip::System::SystemClock().GetMonotonicMilliseconds64().count() - GetAppTask().mButtonPressedTime >=
                    APP_BUTTON_PRESS_LONG)
                {
                    /** factory reset indicat done. */
                    sLightLED.SetOnoff(false);
                    GetAppTask().mTimerIntvl = 1000;
                }
                else
                {
#if !(defined(BL706_NIGHT_LIGHT) || defined(BL602_NIGHT_LIGHT))
                    sLightLED.Toggle();
#endif
                }
            }
            else
            {

#if defined(BL706_NIGHT_LIGHT) && !defined(LED_BTN_RESET)

                if (GetAppTask().mButtonPressedTime)
                {
                    uint32_t resetCnt = 0;

                    if (chip::System::SystemClock().GetMonotonicMilliseconds64().count() - GetAppTask().mButtonPressedTime >
                        APP_BUTTON_PRESS_LONG * 2)
                    {
                        GetAppTask().mButtonPressedTime = 0;
                    }
                    else if (chip::System::SystemClock().GetMonotonicMilliseconds64().count() - GetAppTask().mButtonPressedTime >
                             APP_BUTTON_PRESS_LONG)
                    {

                        size_t saved_value_len = 0;

                        ef_get_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt), &saved_value_len);
                        if (resetCnt >= APP_REBOOT_RESET_COUNT)
                        {
                            GetAppTask().PostEvent(APP_EVENT_BTN_FACTORY_RESET_IND);
                        }

                        /** a power cycle during factory reset indication state will quit factory reset operation */
                        resetCnt = 0;
                        ef_set_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt));
                    }
                }
#else
                if (ButtonPressed())
                {
                    if (!GetAppTask().mIsFactoryResetIndicat &&
                        chip::System::SystemClock().GetMonotonicMilliseconds64().count() - GetAppTask().mButtonPressedTime >=
                            APP_BUTTON_PRESS_SHORT)
                    {
                        GetAppTask().PostEvent(APP_EVENT_BTN_FACTORY_RESET_IND);
                    }
                }
                else
                {
                    GetAppTask().PostEvent(APP_EVENT_BTN_FACTORY_RESET_CANCEL);
                }
#endif
            }
        }

        StartTimer();
    }
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
        LightingUpdate(APP_EVENT_LIGHTING_MASK);
        ChipLogProgress(NotSpecified, "identify stop");
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
    GetAppTask().PostEvent(APP_EVENT_FACTORY_RESET);
}

#ifdef LED_BTN_RESET
hosal_gpio_dev_t gpio_key = { .port = LED_BTN_RESET, .config = INPUT_HIGH_IMPEDANCE, .priv = NULL };

void AppTask::ButtonInit(void)
{
    GetAppTask().mButtonPressedTime     = 0;
    GetAppTask().mIsFactoryResetIndicat = false;

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
    uint32_t presstime;
    if (ButtonPressed())
    {
        bl_set_gpio_intmod(gpio_key.port, 1, HOSAL_IRQ_TRIG_NEG_LEVEL);

        GetAppTask().mButtonPressedTime = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
        GetAppTask().PostEvent(APP_EVENT_BTN_FACTORY_RESET_PRESS);
    }
    else
    {
        bl_set_gpio_intmod(gpio_key.port, 1, HOSAL_IRQ_TRIG_POS_PULSE);

        if (GetAppTask().mButtonPressedTime)
        {
            presstime = chip::System::SystemClock().GetMonotonicMilliseconds64().count() - GetAppTask().mButtonPressedTime;
            if (presstime >= APP_BUTTON_PRESS_LONG)
            {
                GetAppTask().PostEvent(APP_EVENT_FACTORY_RESET);
            }
            else if (presstime <= APP_BUTTON_PRESS_SHORT && presstime >= APP_BUTTON_PRESS_JITTER)
            {
                GetAppTask().PostEvent(APP_EVENT_BTN_SHORT);
            }
            else
            {
                GetAppTask().PostEvent(APP_EVENT_BTN_FACTORY_RESET_CANCEL);
            }
        }

        GetAppTask().mButtonPressedTime = 0;
    }
}
#endif
