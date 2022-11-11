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

#include "AppConfig.h"
#include "LEDWidget.h"

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
#ifdef OTA_ENABLED
#include "OTAConfig.h"
#endif // OTA_ENABLED

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
#include <NetworkCommissioningDriver.h>
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <route_hook/bl_route_hook.h>
#endif
#include <PlatformManagerImpl.h>

#if HEAP_MONITORING
#include "MemMonitoring.h"
#endif

#if CHIP_ENABLE_OPENTHREAD
#include <ThreadStackManagerImpl.h>
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
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
#include <easyflash.h>
#include <hal_gpio.h>
#include <hosal_gpio.h>
}

#include "AppTask.h"

namespace {

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
chip::app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::BLWiFiDriver::GetInstance()));
#endif

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

    chip::DeviceLayer::ConnectivityMgr().SetBLEDeviceName("MatterLight");

#if CHIP_ENABLE_OPENTHREAD
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
    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
#endif
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "ConnectivityMgr().SetThreadDeviceType() failed");
        appError(ret);
    }
#elif CHIP_DEVICE_CONFIG_ENABLE_WIFI

    ret = sWiFiNetworkCommissioningInstance.Init();
    if (CHIP_NO_ERROR != ret)
    {
        ChipLogError(NotSpecified, "sWiFiNetworkCommissioningInstance.Init() failed");
    }
#endif

    chip::DeviceLayer::PlatformMgr().LockChipStack();

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

#if CHIP_DEVICE_CONFIG_ENABLE_EXTENDED_DISCOVERY
    chip::app::DnssdServer::Instance().SetExtendedDiscoveryTimeoutSecs(EXT_DISCOVERY_TIMEOUT_SECS);
#endif

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

#if CHIP_ENABLE_OPENTHREAD
    ChipLogProgress(NotSpecified, "Starting OpenThread task");
    // Start OpenThread task
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "ThreadStackMgr().StartThreadTask() failed");
        appError(ret);
    }
#endif

    ConfigurationMgr().LogDeviceConfig();

    PrintOnboardingCodes(chip::RendezvousInformationFlag(chip::RendezvousInformationFlag::kBLE));
    PlatformMgr().AddEventHandler(AppTask::ChipEventHandler, 0);

#ifdef OTA_ENABLED
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    OTAConfig::Init();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();
#endif // OTA_ENABLED

#if PW_RPC_ENABLED
    chip::rpc::Init();
#endif

    GetAppTask().PostEvent(AppTask::APP_EVENT_TIMER);

    vTaskResume(GetAppTask().sAppTaskHandle);
}

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
    bool isStateReady = false;

    sLightLED.Init();

#ifdef LED_BTN_RESET
    ButtonInit();
#else

    uint32_t resetCnt      = 0;
    size_t saved_value_len = 0;
    ef_get_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt), &saved_value_len);
    resetCnt++;
    ef_set_env_blob(APP_REBOOT_RESET_COUNT_KEY, &resetCnt, sizeof(resetCnt));
#endif

    ChipLogProgress(NotSpecified, "Starting Platform Manager Event Loop");
    CHIP_ERROR ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().StartEventLoopTask() failed");
        appError(ret);
    }
    vTaskSuspend(NULL);

#ifndef LED_BTN_RESET
    GetAppTask().mButtonPressedTime = chip::System::SystemClock().GetMonotonicMilliseconds64().count() + 1;
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        GetAppTask().PostEvent(APP_EVENT_SYS_PROVISIONED);
    }
#endif

    GetAppTask().mIsConnected = false;

    GetAppTask().sTimer = xTimerCreate("lightTmr", pdMS_TO_TICKS(1000), false, NULL, AppTask::TimerCallback);
    if (GetAppTask().sTimer == NULL)
    {
        ChipLogError(NotSpecified, "Failed to create timer task");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }

    ChipLogProgress(NotSpecified, "App Task started, with heap %d left\r\n", xPortGetFreeHeapSize());

    StartTimer();

    while (true)
    {
        appEvent                 = APP_EVENT_NONE;
        BaseType_t eventReceived = xTaskNotifyWait(0, APP_EVENT_ALL_MASK, (uint32_t *) &appEvent, portMAX_DELAY);

        if (eventReceived)
        {
            PlatformMgr().LockChipStack();
            if (APP_EVENT_SYS_BLE_ADV & appEvent)
            {
                LightingSetStatus(APP_EVENT_SYS_BLE_ADV);
                LightingUpdate(APP_EVENT_LIGHTING_GO_THROUGH);

                isStateReady = false;
            }

            if (APP_EVENT_SYS_PROVISIONED & appEvent)
            {
                LightingSetStatus(APP_EVENT_SYS_PROVISIONED);
                LightingUpdate(APP_EVENT_LIGHTING_GO_THROUGH);

                isStateReady = true;
            }

            if (APP_EVENT_BTN_SHORT & appEvent)
            {
                LightingSetStatus(APP_EVENT_SYS_LIGHT_TOGGLE);
                LightingUpdate(APP_EVENT_LIGHTING_GO_THROUGH);
            }

            if ((APP_EVENT_LIGHTING_MASK & appEvent) && isStateReady)
            {
                LightingUpdate((app_event_t)(APP_EVENT_LIGHTING_MASK & appEvent));
            }

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

void AppTask::ChipEventHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:

#ifndef LED_BTN_RESET
        if (ConnectivityMgr().IsThreadProvisioned())
        {
            GetAppTask().PostEvent(APP_EVENT_SYS_PROVISIONED);
            break;
        }
#endif

        if (ConnectivityMgr().NumBLEConnections())
        {
            GetAppTask().PostEvent(APP_EVENT_SYS_BLE_CONN);
        }
        else
        {
            GetAppTask().PostEvent(APP_EVENT_SYS_BLE_ADV);

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
            GetAppTask().mIsConnected = ConnectivityMgr().IsWiFiStationConnected();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
            GetAppTask().mIsConnected = ConnectivityMgr().IsThreadAttached();
#endif
        }
        ChipLogProgress(NotSpecified, "BLE adv changed, connection number: %d\r\n", ConnectivityMgr().NumBLEConnections());
        break;
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    case DeviceEventType::kThreadStateChange:

        ChipLogProgress(NotSpecified, "Thread state changed, IsThreadAttached: %d\r\n", ConnectivityMgr().IsThreadAttached());
        if (!GetAppTask().mIsConnected && ConnectivityMgr().IsThreadAttached())
        {
            GetAppTask().PostEvent(APP_EVENT_SYS_PROVISIONED);
            GetAppTask().mIsConnected = true;
        }
        break;
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
    case DeviceEventType::kWiFiConnectivityChange:

        ChipLogProgress(NotSpecified, "Wi-Fi state changed\r\n", ConnectivityMgr().IsWiFiStationConnected());

        chip::app::DnssdServer::Instance().StartServer();
        NetworkCommissioning::BLWiFiDriver::GetInstance().SaveConfiguration();
        if (!GetAppTask().mIsConnected && ConnectivityMgr().IsWiFiStationConnected())
        {
            GetAppTask().PostEvent(APP_EVENT_SYS_PROVISIONED);
            GetAppTask().mIsConnected = true;
        }
        break;

    case DeviceEventType::kInterfaceIpAddressChanged:
        if ((event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV4_Assigned) ||
            (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned))
        {
            // MDNS server restart on any ip assignment: if link local ipv6 is configured, that
            // will not trigger a 'internet connectivity change' as there is no internet
            // connectivity. MDNS still wants to refresh its listening interfaces to include the
            // newly selected address.
            chip::app::DnssdServer::Instance().StartServer();
        }

        if (event->InterfaceIpAddressChanged.Type == InterfaceIpChangeType::kIpV6_Assigned)
        {
            ChipLogProgress(NotSpecified, "Initializing route hook...");
            bl_route_hook_init();
        }
        break;
#endif
    case DeviceEventType::kFailSafeTimerExpired:

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        GetAppTask().mIsConnected = ConnectivityMgr().IsWiFiStationConnected();
#endif
#if CHIP_DEVICE_CONFIG_ENABLE_WIFI
        GetAppTask().mIsConnected = ConnectivityMgr().IsThreadAttached();
#endif

        break;
    default:
        break;
    }
}

void AppTask::LightingUpdate(app_event_t event)
{
    uint8_t v, onoff, hue, sat;
    EndpointId endpoint = GetAppTask().GetEndpointId();

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
#if defined(BL706_NIGHT_LIGHT) || defined(BL602_NIGHT_LIGHT)
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
    uint8_t onoff             = 1, level, hue, sat;
    EndpointId endpoint       = GetAppTask().GetEndpointId();
    static bool isProvisioned = false;

    if (APP_EVENT_SYS_LIGHT_TOGGLE == status)
    {
        emberAfReadAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, (uint8_t *) &onoff,
                             ZCL_BOOLEAN_ATTRIBUTE_TYPE);
        onoff = 1 - onoff;
    }
    else if (APP_EVENT_SYS_BLE_ADV == status)
    {
        hue = 35;
        emberAfWriteAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_HUE_ATTRIBUTE_ID, (uint8_t *) &hue,
                              ZCL_INT8U_ATTRIBUTE_TYPE);
        sat = 254;
        emberAfWriteAttribute(endpoint, ZCL_COLOR_CONTROL_CLUSTER_ID, ZCL_COLOR_CONTROL_CURRENT_SATURATION_ATTRIBUTE_ID,
                              (uint8_t *) &sat, ZCL_INT8U_ATTRIBUTE_TYPE);
        level = 254;
        emberAfWriteAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, (uint8_t *) &level,
                              ZCL_INT8U_ATTRIBUTE_TYPE);

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
        level = 254;
        emberAfWriteAttribute(endpoint, ZCL_LEVEL_CONTROL_CLUSTER_ID, ZCL_CURRENT_LEVEL_ATTRIBUTE_ID, (uint8_t *) &level,
                              ZCL_INT8U_ATTRIBUTE_TYPE);
    }

    emberAfWriteAttribute(endpoint, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, (uint8_t *) &onoff, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
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
            LightingUpdate(APP_EVENT_LIGHTING_GO_THROUGH);
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

#ifdef BL706_NIGHT_LIGHT

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
#ifdef LED_BTN_RESET
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
        LightingUpdate(APP_EVENT_LIGHTING_GO_THROUGH);
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
        hosal_gpio_irq_set(&gpio_key, HOSAL_IRQ_TRIG_NEG_LEVEL, GetAppTask().ButtonEventHandler, NULL);

        GetAppTask().mButtonPressedTime = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
        GetAppTask().PostEvent(APP_EVENT_BTN_FACTORY_RESET_PRESS);
    }
    else
    {
        hosal_gpio_irq_set(&gpio_key, HOSAL_IRQ_TRIG_POS_PULSE, GetAppTask().ButtonEventHandler, NULL);

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
