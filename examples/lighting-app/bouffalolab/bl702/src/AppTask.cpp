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

#include "AppTask.h"
#include "AppConfig.h"
#include "LEDWidget.h"
#include <easyflash.h>

#include <app-common/zap-generated/attribute-id.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-id.h>
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
#ifdef BL702_OTA_ENABLED
#include "OTAConfig.h"
#endif // BL702_OTA_ENABLED

#include <platform/bouffalolab/BL702/PlatformManagerImpl.h>

#if HEAP_MONITORING
#include "MemMonitoring.h"
#endif

#if CHIP_ENABLE_OPENTHREAD
#include <openthread_port.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/bouffalolab/BL702/ThreadStackManagerImpl.h>
#include <utils_list.h>
#endif

#if PW_RPC_ENABLED
#include "PigweedLogger.h"
#include "Rpc.h"
#endif

#if CONFIG_ENABLE_CHIP_SHELL
#include <ChipShellCollection.h>
#include <lib/shell/Engine.h>
#endif

#if CONFIG_ENABLE_CHIP_SHELL || PW_RPC_ENABLED
#include "uart.h"
#endif

extern "C" {
#include "board.h"
#include <bl_gpio.h>
#include <hal_gpio.h>
#include <hosal_gpio.h>
}

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_PRIORITY 2
#define EXT_DISCOVERY_TIMEOUT_SECS 20
#define APP_LIGHT_ENDPOINT_ID 1
#define APP_REBOOT_RESET_COUNT 3
#define APP_REBOOT_RESET_COUNT_KEY "app_reset_cnt"
namespace {

#if APP_BOARD_LED_STATUS
LEDWidget sStatusLED;
#endif

#ifdef BL706_NIGHT_LIGHT
ColorLEDWidget sLightLED;
#else
DimmableLEDWidget sLightLED;
#endif

Identify sIdentify = {
    APP_LIGHT_ENDPOINT_ID,
    AppTask::IdentifyStartHandler,
    AppTask::IdentifyStopHandler,
#if APP_BOARD_LED_STATUS
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LED,
#else
    EMBER_ZCL_IDENTIFY_IDENTIFY_TYPE_VISIBLE_LIGHT,
#endif
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

void PlatformManagerImpl::PlatformInit(void)
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
    CHIP_ERROR ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().InitChipStack() failed");
        appError(ret);
    }

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("BL702_LIGHT");

    ot_alarmInit();
    ot_radioInit();
#if CONFIG_ENABLE_CHIP_SHELL
    cmd_otcli_init();
#endif
    ChipLogProgress(NotSpecified, "Initializing OpenThread stack");
    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "ThreadStackMgr().InitThreadStack() failed");
        appError(ret);
    }

#if CHIP_DEVICE_CONFIG_THREAD_FTD
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_Router);
#else
    ret                    = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "ConnectivityMgr().SetThreadDeviceType() failed");
        appError(ret);
    }

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(EXT_DISCOVERY_TIMEOUT_SECS);

    // Init ZCL Data Model
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

    ret = chip::Server::GetInstance().Init(initParams);
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "chip::Server::GetInstance().Init(initParams) failed");
        appError(ret);
    }
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    ChipLogProgress(NotSpecified, "Starting OpenThread task");
    // Start OpenThread task
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "ThreadStackMgr().StartThreadTask() failed");
        appError(ret);
    }

    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    PlatformMgr().AddEventHandler(AppTask::ChipEventHandler, 0);

#ifdef BL702_OTA_ENABLED
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    OTAConfig::Init();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
#endif // BL702_OTA_ENABLED

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

#if APP_BOARD_LED_STATUS || APP_BOARD_BTN
    GetAppTask().PostEvent(AppTask::APP_EVENT_TIMER);
#endif

    vTaskResume(GetAppTask().sAppTaskHandle);
}

void StartAppTask(void)
{
    ChipLogProgress(NotSpecified, "Initializing APP task");

    easyflash_init();

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
    static uint32_t taskDelay = portMAX_DELAY;
    bool isStateReady         = false;

#if APP_BOARD_LED_STATUS
    sStatusLED.Init();
#endif

    sLightLED.Init();

#if APP_BOARD_BTN
    ButtonInit();
#else
    uint32_t resetCnt      = 0;
    size_t saved_value_len = 0;

    ef_get_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt), &saved_value_len);
    if (resetCnt >= APP_REBOOT_RESET_COUNT)
    {
        GetAppTask().PostEvent(APP_EVENT_FACTORY_RESET);
    }
    else
    {
        resetCnt++;
        ef_set_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt));
    }
#endif

    ChipLogProgress(NotSpecified, "Starting Platform Manager Event Loop");
    CHIP_ERROR ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }
    vTaskSuspend(NULL);

#if APP_BOARD_LED_STATUS || APP_BOARD_BTN
    GetAppTask().sTimer = xTimerCreate("lightTmr", pdMS_TO_TICKS(1000), false, NULL, AppTask::TimerCallback);
    if (GetAppTask().sTimer == NULL)
    {
        ChipLogError(NotSpecified, "Failed to create timer task");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }
#endif

    ChipLogProgress(NotSpecified, "App Task started, with heap %d left\r\n", xPortGetFreeHeapSize());

    while (true)
    {
        appEvent                 = APP_EVENT_NONE;
        BaseType_t eventReceived = xTaskNotifyWait(0, APP_EVENT_ALL_MASK, (uint32_t *) &appEvent, taskDelay);

        if (eventReceived)
        {
#if APP_BOARD_LED_STATUS || APP_BOARD_BTN
            TimerDutyCycle(appEvent);
#endif

            PlatformMgr().LockChipStack();
            if (APP_EVENT_SYS_BLE_ADV & appEvent)
            {
                LightingSetStatus(APP_EVENT_SYS_BLE_ADV);
                isStateReady = false;
            }

            if (APP_EVENT_SYS_PROVISIONED & appEvent)
            {
                LightingSetStatus(APP_EVENT_SYS_PROVISIONED);
                isStateReady = true;
            }

            if ((APP_EVENT_LIGHTING_MASK & appEvent) && isStateReady)
            {
#if APP_BOARD_LED_STATUS || APP_BOARD_BTN
                if (APP_EVENT_LIGHTING_CHECK & appEvent)
                {
                    TimerDutyCycle(APP_EVENT_SYS_PROVISIONED);
                }
#endif
                LightingUpdate((app_event_t)(APP_EVENT_LIGHTING_MASK & appEvent));
            }

            if (APP_EVENT_IDENTIFY_MASK & appEvent)
            {
                IdentifyHandleOp(appEvent);
            }

            if (APP_EVENT_FACTORY_RESET & appEvent)
            {
#if !APP_BOARD_LED_STATUS
                LightingSetStatus(APP_EVENT_FACTORY_RESET);
                LightingUpdate(APP_EVENT_LIGHTING_GO_THROUGH);
                vTaskDelay(FACTORY_RESET_TRIGGER_TIMEOUT);
#endif
                DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
            }

            if (APP_EVENT_BTN_FACTORY_RESET_PW_PRC & appEvent)
            {
                vTaskDelay(FACTORY_RESET_TRIGGER_TIMEOUT);
                DeviceLayer::ConfigurationMgr().InitiateFactoryReset();
            }

            PlatformMgr().UnlockChipStack();
#if APP_BOARD_LED_STATUS || APP_BOARD_BTN
            if (APP_EVENT_TIMER & appEvent)
            {
                TimerEventHandler();
            }
#endif
        }
    }
}

void AppTask::ChipEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        if (ConnectivityMgr().NumBLEConnections())
        {
            GetAppTask().PostEvent(APP_EVENT_SYS_BLE_CONN);
        }
        else
        {
            GetAppTask().PostEvent(APP_EVENT_SYS_BLE_ADV);
        }
        ChipLogProgress(NotSpecified, "Thread state, ble conn %d\r\n", ConnectivityMgr().NumBLEConnections());
        break;
    case DeviceEventType::kThreadStateChange:

        if (ConnectivityMgr().IsThreadProvisioned() && ConnectivityMgr().IsThreadEnabled())
        {
            GetAppTask().PostEvent(APP_EVENT_SYS_PROVISIONED);
        }

        ChipLogProgress(NotSpecified, "Thread state, prov %d, enabled %d, attached %d\r\n", ConnectivityMgr().IsThreadProvisioned(),
                        ConnectivityMgr().IsThreadEnabled(), ConnectivityMgr().IsThreadAttached());
        break;
    case DeviceEventType::kWiFiConnectivityChange:
        GetAppTask().PostEvent(APP_EVENT_SYS_PROVISIONED);
        break;
    default:
        break;
    }
}

void AppTask::LightingUpdate(app_event_t event)
{
    static uint32_t updateOpCnt = 0;
    uint8_t v, onoff, hue, sat;
    EmberAfAttributeType dataType;
    EndpointId endpoint = GetAppTask().GetEndpointId();

    if (APP_EVENT_LIGHTING_CHECK & event)
    {
        if (updateOpCnt++ < 2)
        {
            GetAppTask().PostEvent(APP_EVENT_LIGHTING_CHECK);
            return;
        }
        updateOpCnt = 0;
    }
    else if (APP_EVENT_LIGHTING_MASK & event)
    {
        updateOpCnt = 0;
        GetAppTask().PostEvent(APP_EVENT_LIGHTING_CHECK);
        return;
    }

    do
    {
        if (EMBER_ZCL_STATUS_SUCCESS !=
            emberAfReadAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, &onoff, sizeof(onoff)))
        {
            break;
        }

        if (EMBER_ZCL_STATUS_SUCCESS !=
            emberAfReadAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, &v, sizeof(v)))
        {
            break;
        }

        if (EMBER_ZCL_STATUS_SUCCESS !=
            emberAfReadAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, &hue,
                                 sizeof(v)))
        {
            break;
        }

        if (EMBER_ZCL_STATUS_SUCCESS !=
            emberAfReadAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID, &sat,
                                 sizeof(v)))
        {
            break;
        }

        if (0 == onoff)
        {
            sLightLED.SetLevel(0);
        }
        else
        {
#ifdef BL706_NIGHT_LIGHT
            sLightLED.SetColor(v, hue, sat);
#else
            sLightLED.SetLevel(v);
#endif
        }

    } while (0);
}

void AppTask::LightingSetOnoff(uint8_t bonoff)
{
    uint8_t newValue    = bonoff;
    EndpointId endpoint = GetAppTask().GetEndpointId();

    // write the new on/off value
    emberAfWriteAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, (uint8_t *) &newValue,
                          ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    newValue = 254;
    emberAfWriteAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, (uint8_t *) &newValue,
                          ZCL_INT8U_ATTRIBUTE_TYPE);
}

void AppTask::LightingSetStatus(app_event_t status)
{
    uint8_t level, hue, sat;
    EndpointId endpoint       = GetAppTask().GetEndpointId();
    static bool isProvisioned = false;

#if !APP_BOARD_BTN
    uint32_t resetCnt      = 0;
    size_t saved_value_len = 0;

    if ((APP_EVENT_SYS_BLE_ADV | APP_EVENT_SYS_PROVISIONED) & status)
    {

        if (0 == GetAppTask().buttonPressedTimeout)
        {

            GetAppTask().buttonPressedTimeout = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
            ef_get_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt), &saved_value_len);
            if (resetCnt >= APP_REBOOT_RESET_COUNT)
            {
                GetAppTask().PostEvent(APP_EVENT_FACTORY_RESET);
            }
            else
            {
                resetCnt = 0;
                ef_set_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt));
            }
        }
    }
    else if (APP_EVENT_FACTORY_RESET & status)
    {
        /** reset resetCnt to zero before factory reset, just in case to cancel factory reset */
        resetCnt = 0;
        ef_set_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt));
    }
#endif

    if (APP_EVENT_SYS_BLE_ADV == status)
    {
        hue = 35;
        emberAfWriteAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, (uint8_t *) &hue,
                              ZCL_INT8U_ATTRIBUTE_TYPE);
        sat = 254;
        emberAfWriteAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                              (uint8_t *) &sat, ZCL_INT8U_ATTRIBUTE_TYPE);
        isProvisioned = false;
    }
    else if (APP_EVENT_SYS_PROVISIONED == status)
    {
        if (isProvisioned)
        {
            return;
        }
        isProvisioned = true;
        sat           = 0;
        emberAfWriteAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                              (uint8_t *) &sat, ZCL_INT8U_ATTRIBUTE_TYPE);
    }
    else if (APP_EVENT_FACTORY_RESET == status)
    {
        hue = 84;
        emberAfWriteAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, (uint8_t *) &hue,
                              ZCL_INT8U_ATTRIBUTE_TYPE);
        sat = 254;
        emberAfWriteAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                              (uint8_t *) &sat, ZCL_INT8U_ATTRIBUTE_TYPE);
    }

    level = 254;
    emberAfWriteAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, (uint8_t *) &level,
                          ZCL_INT8U_ATTRIBUTE_TYPE);

    level = 1;
    emberAfWriteAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, (uint8_t *) &level, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
}

#if APP_BOARD_LED_STATUS || APP_BOARD_BTN
bool AppTask::StartTimer(void)
{
    uint32_t aTimeoutMs = GetAppTask().mBlinkOnTimeMS;

#if APP_BOARD_LED_STATUS
    if (GetAppTask().mBlinkOffTimeMS)
    {
        aTimeoutMs = sStatusLED.GetOnoff() ? GetAppTask().mBlinkOnTimeMS : GetAppTask().mBlinkOffTimeMS;
        sStatusLED.Toggle();
    }
    else
    {
        if (!sStatusLED.GetOnoff())
        {
            sStatusLED.SetOnoff(1);
        }
    }
#endif

    if (xTimerIsTimerActive(GetAppTask().sTimer))
    {
        CancelTimer();
    }
    if (aTimeoutMs == 0)
    {
        aTimeoutMs = 1000;
    }

    if (xTimerChangePeriod(GetAppTask().sTimer, pdMS_TO_TICKS(aTimeoutMs), pdMS_TO_TICKS(100)) != pdPASS)
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

void AppTask::TimerEventHandler(void)
{
#if APP_BOARD_BTN
    if (GetAppTask().buttonPressedTimeout)
    {
        /** device is being in factory reset confirm state*/
        if (ButtonPressed())
        {
            if (GetAppTask().buttonPressedTimeout < chip::System::SystemClock().GetMonotonicMilliseconds64().count())
            {
                GetAppTask().PostEvent(APP_EVENT_FACTORY_RESET);
            }
        }
        else
        {
            /** factory reset cancelled */
            GetAppTask().buttonPressedTimeout = 0;
            GetAppTask().PostEvent(APP_EVENT_BTN_FACTORY_RESET_CANCEL);
        }
    }
#endif

    StartTimer();
}

void AppTask::TimerDutyCycle(app_event_t event)
{
    static uint32_t backup_blinkOnTimeMS, backup_blinkOffTimeMS;

    if (event & APP_EVENT_SYS_PROVISIONED)
    {
        GetAppTask().mBlinkOnTimeMS = 1000, GetAppTask().mBlinkOffTimeMS = 0;
    }
    else if (event & APP_EVENT_SYS_BLE_CONN)
    {
        GetAppTask().mBlinkOnTimeMS = 100, GetAppTask().mBlinkOffTimeMS = 100;
    }
    else if (event & APP_EVENT_SYS_BLE_ADV)
    {
        GetAppTask().mBlinkOnTimeMS = 100, GetAppTask().mBlinkOffTimeMS = 900;
    }
    else if (event & APP_EVENT_BTN_FACTORY_RESET_IND)
    {
        if (500 != backup_blinkOffTimeMS || 500 != backup_blinkOnTimeMS)
        {
            backup_blinkOnTimeMS = GetAppTask().mBlinkOnTimeMS, backup_blinkOffTimeMS = GetAppTask().mBlinkOffTimeMS;
        }
        GetAppTask().mBlinkOnTimeMS = 500, GetAppTask().mBlinkOffTimeMS = 500;
    }
    else if (event & APP_EVENT_BTN_FACTORY_RESET_CANCEL)
    {
        GetAppTask().mBlinkOnTimeMS = backup_blinkOnTimeMS, GetAppTask().mBlinkOffTimeMS = backup_blinkOffTimeMS;
    }
}
#endif

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
        LightingUpdate(APP_EVENT_LIGHTING_GO_THROUGH);
        ChipLogProgress(NotSpecified, "identify stop");
    }
}

void AppTask::ButtonEventHandler(uint8_t btnIdx, uint8_t btnAction)
{
    GetAppTask().PostEvent(APP_EVENT_BTN_FACTORY_RESET_PW_PRC);
    GetAppTask().buttonPressedTimeout =
        chip::System::SystemClock().GetMonotonicMilliseconds64().count() + FACTORY_RESET_TRIGGER_TIMEOUT - 100;
}

#if APP_BOARD_BTN
hosal_gpio_dev_t gpio_key = { .port = LED_BTN_RESET, .config = INPUT_PULL_UP, .priv = NULL };

void AppTask::ButtonInit(void)
{
    GetAppTask().buttonPressedTimeout = 0;

    hosal_gpio_init(&gpio_key);
    hosal_gpio_irq_set(&gpio_key, HOSAL_IRQ_TRIG_NEG_PULSE, GetAppTask().ButtonEventHandler, NULL);
}

bool AppTask::ButtonPressed(void)
{
    uint8_t val = 1;
    hosal_gpio_input_get(&gpio_key, &val);
    return val == 0;
}

void AppTask::ButtonEventHandler(void * arg)
{
    if (ButtonPressed())
    {
        GetAppTask().PostEvent(APP_EVENT_BTN_FACTORY_RESET_IND);
        GetAppTask().buttonPressedTimeout =
            chip::System::SystemClock().GetMonotonicMilliseconds64().count() + FACTORY_RESET_TRIGGER_TIMEOUT - 100;
    }
}
#endif
