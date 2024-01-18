/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
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

#include "AppTaskCommon.h"
#include "AppTask.h"

#include "BLEManagerImpl.h"
#include "ButtonManager.h"

#include "ThreadUtil.h"

#include <DeviceInfoProviderImpl.h>
#include <app/clusters/identify-server/identify-server.h>
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <app/util/attribute-storage.h>

#if CONFIG_BOOTLOADER_MCUBOOT
#include <OTAUtil.h>
#endif

#if CONFIG_CHIP_OTA_REQUESTOR
#include <app/clusters/ota-requestor/OTARequestorInterface.h>
#endif

#include <zephyr/fs/nvs.h>
#include <zephyr/settings/settings.h>

using namespace chip::app;

LOG_MODULE_DECLARE(app, CONFIG_CHIP_APP_LOG_LEVEL);

namespace {
constexpr int kFactoryResetCalcTimeout = 3000;
constexpr int kFactoryResetTriggerCntr = 3;
constexpr int kAppEventQueueSize       = 10;

#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
const struct gpio_dt_spec sFactoryResetButtonDt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_1), gpios);
#if APP_USE_BLE_START_BUTTON
const struct gpio_dt_spec sBleStartButtonDt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_2), gpios);
#endif
#if APP_USE_THREAD_START_BUTTON
const struct gpio_dt_spec sThreadStartButtonDt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_3), gpios);
#endif
#if APP_USE_EXAMPLE_START_BUTTON
const struct gpio_dt_spec sExampleActionButtonDt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_4), gpios);
#endif
#else
const struct gpio_dt_spec sButtonCol1Dt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_matrix_col1), gpios);
const struct gpio_dt_spec sButtonCol2Dt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_matrix_col2), gpios);
const struct gpio_dt_spec sButtonRow1Dt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_matrix_row1), gpios);
const struct gpio_dt_spec sButtonRow2Dt = GPIO_DT_SPEC_GET(DT_NODELABEL(key_matrix_row2), gpios);
#endif

#ifdef APP_USE_IDENTIFY_PWM
constexpr uint32_t kIdentifyBlinkRateMs         = 200;
constexpr uint32_t kIdentifyOkayOnRateMs        = 50;
constexpr uint32_t kIdentifyOkayOffRateMs       = 950;
constexpr uint32_t kIdentifyFinishOnRateMs      = 950;
constexpr uint32_t kIdentifyFinishOffRateMs     = 50;
constexpr uint32_t kIdentifyChannelChangeRateMs = 1000;
constexpr uint32_t kIdentifyBreatheRateMs       = 1000;

const struct pwm_dt_spec sPwmIdentifySpecGreenLed = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led3));
#endif

#if APP_SET_NETWORK_COMM_ENDPOINT_SEC
constexpr EndpointId kNetworkCommissioningEndpointSecondary = 0xFFFE;
#endif

K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), kAppEventQueueSize, alignof(AppEvent));

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
LEDWidget sStatusLED;
#endif

Button sFactoryResetButton;
#if APP_USE_BLE_START_BUTTON
Button sBleAdvStartButton;
#endif
#if APP_USE_EXAMPLE_START_BUTTON
Button sExampleActionButton;
#endif
#if APP_USE_THREAD_START_BUTTON
Button sThreadStartButton;
#endif

k_timer sFactoryResetTimer;
uint8_t sFactoryResetCntr = 0;

bool sIsCommissioningFailed = false;
bool sIsThreadProvisioned   = false;
bool sIsThreadEnabled       = false;
bool sIsThreadAttached      = false;
bool sHaveBLEConnections    = false;

#if APP_SET_DEVICE_INFO_PROVIDER
chip::DeviceLayer::DeviceInfoProviderImpl gExampleDeviceInfoProvider;
#endif

#ifdef APP_USE_IDENTIFY_PWM
void OnIdentifyTriggerEffect(Identify * identify)
{
    AppTaskCommon::IdentifyEffectHandler(identify->mCurrentEffectIdentifier);
}

Identify sIdentify = {
    kExampleEndpointId,
    [](Identify *) { ChipLogProgress(Zcl, "OnIdentifyStart"); },
    [](Identify *) { ChipLogProgress(Zcl, "OnIdentifyStop"); },
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    OnIdentifyTriggerEffect,
};
#endif

#if CONFIG_CHIP_FACTORY_DATA
// NOTE! This key is for test/certification only and should not be available in production devices!
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };
#endif

class AppCallbacks : public AppDelegate
{
    bool isComissioningStarted;

public:
    void OnCommissioningSessionEstablishmentStarted() override { sIsCommissioningFailed = false; }
    void OnCommissioningSessionStarted() override { isComissioningStarted = true; }
    void OnCommissioningSessionStopped() override { isComissioningStarted = false; }
    void OnCommissioningSessionEstablishmentError(CHIP_ERROR err) override { sIsCommissioningFailed = true; }
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    void OnCommissioningWindowClosed() override
    {
        if (!isComissioningStarted)
            chip::DeviceLayer::Internal::BLEMgr().Shutdown();
    }
#endif
};

AppCallbacks sCallbacks;
} // namespace

class AppFabricTableDelegate : public FabricTable::Delegate
{
    void OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
    {
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
        {
            ChipLogProgress(DeviceLayer, "Performing erasing of settings partition");

            // Do FactoryReset in case of failed commissioning to allow new pairing via BLE
            if (sIsCommissioningFailed)
            {
                chip::Server::GetInstance().ScheduleFactoryReset();
            }
            // TC-OPCREDS-3.6 (device doesn't need to reboot automatically after the last fabric is removed) can't use FactoryReset
            else
            {
                void * storage = nullptr;
                int status     = settings_storage_get(&storage);

                if (!status)
                {
                    status = nvs_clear(static_cast<nvs_fs *>(storage));
                }

                if (!status)
                {
                    status = nvs_mount(static_cast<nvs_fs *>(storage));
                }

                if (status)
                {
                    ChipLogError(DeviceLayer, "Storage clearance failed: %d", status);
                }
            }
        }
    }
};

class PlatformMgrDelegate : public DeviceLayer::PlatformManagerDelegate
{
    // Disable openthread before reset to prevent writing to NVS
    void OnShutDown() override
    {
        if (ThreadStackManagerImpl().IsThreadEnabled())
        {
            ThreadStackManagerImpl().Finalize();
        }
    }
};

#if CONFIG_CHIP_LIB_SHELL
#include <zephyr/shell/shell.h>
#include <zephyr/sys/reboot.h>

static int cmd_telink_reboot(const struct shell * shell, size_t argc, char ** argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    shell_print(shell, "Performing board reboot...");
    sys_reboot(0);

    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_telink, SHELL_CMD(reboot, NULL, "Reboot board command", cmd_telink_reboot),
                               SHELL_SUBCMD_SET_END);
SHELL_CMD_REGISTER(telink, &sub_telink, "Telink commands", NULL);
#endif // CONFIG_CHIP_LIB_SHELL

#ifdef CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET
void AppTaskCommon::PowerOnFactoryReset(void)
{
    LOG_INF("schedule factory reset");
    chip::Server::GetInstance().ScheduleFactoryReset();
}
#endif /* CONFIG_CHIP_ENABLE_POWER_ON_FACTORY_RESET */

CHIP_ERROR AppTaskCommon::StartApp(void)
{
    CHIP_ERROR err = GetAppTask().Init();

    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AppTask Init fail");
        return err;
    }

    AppEvent event = {};

#if !CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
    StartThreadButtonEventHandler();
#endif

#ifdef CONFIG_BOOTLOADER_MCUBOOT
    if (!chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        LOG_INF("Confirm image.");
        OtaConfirmNewImage();
    }
#endif /* CONFIG_BOOTLOADER_MCUBOOT */

    while (true)
    {
        GetEvent(&event);
        DispatchEvent(&event);
    }
}

CHIP_ERROR AppTaskCommon::InitCommonParts(void)
{
    CHIP_ERROR err;
    LOG_INF("SW Version: %u, %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION, CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);

    // Initialize status LED
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
    LEDWidget::SetStateUpdateCallback(LEDStateUpdateHandler);
    sStatusLED.Init(GPIO_DT_SPEC_GET_OR(DT_ALIAS(system_state_led), gpios, {}));

    UpdateStatusLED();
#endif

    InitButtons();

    // Initialize function button timer
    k_timer_init(&sFactoryResetTimer, &AppTask::FactoryResetTimerTimeoutCallback, nullptr);
    k_timer_user_data_set(&sFactoryResetTimer, this);

#ifdef APP_USE_IDENTIFY_PWM
    // Initialize PWM Identify led
    err = GetAppTask().mPwmIdentifyLed.Init(&sPwmIdentifySpecGreenLed, kDefaultMinLevel, kDefaultMaxLevel, kDefaultMaxLevel);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("Green IDENTIFY PWM Device Init fail");
        return err;
    }

    GetAppTask().mPwmIdentifyLed.SetCallbacks(nullptr, nullptr, ActionIdentifyStateUpdateHandler);
#endif

    // Initialize CHIP server
#if CONFIG_CHIP_FACTORY_DATA
    ReturnErrorOnFailure(mFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&mFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&mFactoryDataProvider);
    SetCommissionableDataProvider(&mFactoryDataProvider);
    // Read EnableKey from the factory data.
    MutableByteSpan enableKey(sTestEventTriggerEnableKey);
    err = mFactoryDataProvider.GetEnableKey(enableKey);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("mFactoryDataProvider.GetEnableKey() failed. Could not delegate a test event trigger");
        memset(sTestEventTriggerEnableKey, 0, sizeof(sTestEventTriggerEnableKey));
    }
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif

    // Init ZCL Data Model and start server
    static CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();
    initParams.appDelegate = &sCallbacks;
    ReturnErrorOnFailure(chip::Server::GetInstance().Init(initParams));

#if APP_SET_DEVICE_INFO_PROVIDER
    gExampleDeviceInfoProvider.SetStorageDelegate(&Server::GetInstance().GetPersistentStorage());
    chip::DeviceLayer::SetDeviceInfoProvider(&gExampleDeviceInfoProvider);
#endif

    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

#if APP_SET_NETWORK_COMM_ENDPOINT_SEC
    // We only have network commissioning on endpoint 0.
    // Set up a valid Network Commissioning cluster on endpoint 0 is done in
    // src/platform/OpenThread/GenericThreadStackManagerImpl_OpenThread.hpp
    emberAfEndpointEnableDisable(kNetworkCommissioningEndpointSecondary, false);
#endif

    // We need to disable OpenThread to prevent writing to the NVS storage when factory reset occurs
    // The OpenThread thread is running during factory reset. The nvs_clear function is called during
    // factory reset, which makes the NVS storage innaccessible, but the OpenThread knows nothing
    // about this and tries to store the parameters to NVS. Because of this the OpenThread need to be
    // shut down before NVS. This delegate fixes the issue "Failed to store setting , ret -13",
    // which means that the NVS is already disabled.
    // For this the OnShutdown function is used
    PlatformMgr().SetDelegate(new PlatformMgrDelegate);

    // Add CHIP event handler and start CHIP thread.
    // Note that all the initialization code should happen prior to this point to avoid data races
    // between the main and the CHIP threads.
    PlatformMgr().AddEventHandler(ChipEventHandler, 0);

    err = chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(new AppFabricTableDelegate);
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERR("AppFabricTableDelegate fail");
        return err;
    }

    return CHIP_NO_ERROR;
}

#ifdef CONFIG_CHIP_PW_RPC
void AppTaskCommon::ButtonEventHandler(ButtonId_t btnId, bool btnPressed)
{
    if (!btnPressed)
    {
        return;
    }

    switch (btnId)
    {
#if APP_USE_EXAMPLE_START_BUTTON
    case kButtonId_ExampleAction:
        ExampleActionButtonEventHandler();
        break;
#endif
    case kButtonId_FactoryReset:
        FactoryResetButtonEventHandler();
        break;
#if APP_USE_THREAD_START_BUTTON
    case kButtonId_StartThread:
        StartThreadButtonEventHandler();
        break;
#endif
#if APP_USE_BLE_START_BUTTON
    case kButtonId_StartBleAdv:
        StartBleAdvButtonEventHandler();
        break;
#endif
    }
}
#endif

void AppTaskCommon::InitButtons(void)
{
#if CONFIG_CHIP_BUTTON_MANAGER_IRQ_MODE
    sFactoryResetButton.Configure(&sFactoryResetButtonDt, FactoryResetButtonEventHandler);
#if APP_USE_BLE_START_BUTTON
    sBleAdvStartButton.Configure(&sBleStartButtonDt, StartBleAdvButtonEventHandler);
#endif
#if APP_USE_EXAMPLE_START_BUTTON
    if (ExampleActionEventHandler)
    {
        sExampleActionButton.Configure(&sExampleActionButtonDt, ExampleActionButtonEventHandler);
    }
#endif
#if APP_USE_THREAD_START_BUTTON
    sThreadStartButton.Configure(&sThreadStartButtonDt, StartThreadButtonEventHandler);
#endif
#else
    sFactoryResetButton.Configure(&sButtonRow1Dt, &sButtonCol1Dt, FactoryResetButtonEventHandler);
#if APP_USE_BLE_START_BUTTON
    sBleAdvStartButton.Configure(&sButtonRow2Dt, &sButtonCol2Dt, StartBleAdvButtonEventHandler);
#endif
#if APP_USE_EXAMPLE_START_BUTTON
    if (ExampleActionEventHandler)
    {
        sExampleActionButton.Configure(&sButtonRow1Dt, &sButtonCol2Dt, ExampleActionButtonEventHandler);
    }
#endif
#if APP_USE_THREAD_START_BUTTON
    sThreadStartButton.Configure(&sButtonRow2Dt, &sButtonCol1Dt, StartThreadButtonEventHandler);
#endif
#endif

    ButtonManagerInst().AddButton(sFactoryResetButton);
#if APP_USE_BLE_START_BUTTON
    ButtonManagerInst().AddButton(sBleAdvStartButton);
#endif
#if APP_USE_THREAD_START_BUTTON
    ButtonManagerInst().AddButton(sThreadStartButton);
#endif
#if APP_USE_EXAMPLE_START_BUTTON
    if (ExampleActionEventHandler)
    {
        ButtonManagerInst().AddButton(sExampleActionButton);
    }
#endif
}

#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
void AppTaskCommon::UpdateLedStateEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type == AppEvent::kEventType_UpdateLedState)
    {
        aEvent->UpdateLedStateEvent.LedWidget->UpdateState();
    }
}

void AppTaskCommon::LEDStateUpdateHandler(LEDWidget * ledWidget)
{
    AppEvent event;
    event.Type                          = AppEvent::kEventType_UpdateLedState;
    event.Handler                       = UpdateLedStateEventHandler;
    event.UpdateLedStateEvent.LedWidget = ledWidget;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::UpdateStatusLED()
{
    if (sIsThreadProvisioned && sIsThreadEnabled)
    {
        if (sIsThreadAttached)
        {
            sStatusLED.Blink(950, 50);
        }
        else
        {
            sStatusLED.Blink(100, 100);
        }
    }
    else
    {
        sStatusLED.Blink(50, 950);
    }
}
#endif

#ifdef APP_USE_IDENTIFY_PWM
void AppTaskCommon::ActionIdentifyStateUpdateHandler(k_timer * timer)
{
    AppEvent event;
    event.Type    = AppEvent::kEventType_UpdateLedState;
    event.Handler = UpdateIdentifyStateEventHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::UpdateIdentifyStateEventHandler(AppEvent * aEvent)
{
    GetAppTask().mPwmIdentifyLed.UpdateAction();
}

void AppTaskCommon::IdentifyEffectHandler(Clusters::Identify::EffectIdentifierEnum aEffect)
{
    AppEvent event;
    event.Type = AppEvent::kEventType_IdentifyStart;

    switch (aEffect)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBlink");
        event.Handler = [](AppEvent *) {
            GetAppTask().mPwmIdentifyLed.InitiateBlinkAction(kIdentifyBlinkRateMs, kIdentifyBlinkRateMs);
        };
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kBreathe");
        event.Handler = [](AppEvent *) {
            GetAppTask().mPwmIdentifyLed.InitiateBreatheAction(PWMDevice::kBreatheType_Both, kIdentifyBreatheRateMs);
        };
        break;
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kOkay");
        event.Handler = [](AppEvent *) {
            GetAppTask().mPwmIdentifyLed.InitiateBlinkAction(kIdentifyOkayOnRateMs, kIdentifyOkayOffRateMs);
        };
        break;
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kChannelChange");
        event.Handler = [](AppEvent *) {
            GetAppTask().mPwmIdentifyLed.InitiateBlinkAction(kIdentifyChannelChangeRateMs, kIdentifyChannelChangeRateMs);
        };
        break;
    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kFinishEffect");
        event.Handler = [](AppEvent *) {
            GetAppTask().mPwmIdentifyLed.InitiateBlinkAction(kIdentifyFinishOnRateMs, kIdentifyFinishOffRateMs);
        };
        break;
    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        ChipLogProgress(Zcl, "Clusters::Identify::EffectIdentifierEnum::kStopEffect");
        event.Handler = [](AppEvent *) { GetAppTask().mPwmIdentifyLed.StopAction(); };
        event.Type    = AppEvent::kEventType_IdentifyStop;
        break;
    default:
        ChipLogProgress(Zcl, "No identifier effect");
        return;
    }

    GetAppTask().PostEvent(&event);
}
#endif

#if APP_USE_BLE_START_BUTTON
void AppTaskCommon::StartBleAdvButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = StartBleAdvHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::StartBleAdvHandler(AppEvent * aEvent)
{
    LOG_INF("StartBleAdvHandler");

    // Don't allow on starting Matter service BLE advertising after Thread provisioning.
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        LOG_INF("Device already commissioned");
        return;
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        LOG_INF("BLE adv already enabled");
        return;
    }

    if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() != CHIP_NO_ERROR)
    {
        LOG_ERR("OpenBasicCommissioningWindow fail");
    }
}
#endif

void AppTaskCommon::FactoryResetButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = FactoryResetHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::FactoryResetHandler(AppEvent * aEvent)
{
    if (sFactoryResetCntr == 0)
    {
        k_timer_start(&sFactoryResetTimer, K_MSEC(kFactoryResetCalcTimeout), K_NO_WAIT);
    }

    sFactoryResetCntr++;
    LOG_INF("Factory Reset Trigger Counter: %d/%d", sFactoryResetCntr, kFactoryResetTriggerCntr);

    if (sFactoryResetCntr == kFactoryResetTriggerCntr)
    {
        k_timer_stop(&sFactoryResetTimer);
        sFactoryResetCntr = 0;

        chip::Server::GetInstance().ScheduleFactoryReset();
    }
}

void AppTaskCommon::FactoryResetTimerTimeoutCallback(k_timer * timer)
{
    if (!timer)
    {
        return;
    }

    AppEvent event;
    event.Type    = AppEvent::kEventType_Timer;
    event.Handler = FactoryResetTimerEventHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::FactoryResetTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
    {
        return;
    }

    sFactoryResetCntr = 0;
    LOG_INF("Factory Reset Trigger Counter is cleared");
}

#if APP_USE_THREAD_START_BUTTON || !CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
void AppTaskCommon::StartThreadButtonEventHandler(void)
{
    AppEvent event;

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = StartThreadHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::StartThreadHandler(AppEvent * aEvent)
{
    LOG_INF("StartThreadHandler");
    if (!chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        // Switch context from BLE to Thread
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
        Internal::BLEManagerImpl sInstance;
        sInstance.SwitchToIeee802154();
#else
        ThreadStackMgrImpl().SetRadioBlocked(false);
        ThreadStackMgrImpl().SetThreadEnabled(true);
#endif
        StartDefaultThreadNetwork();
    }
    else
    {
        LOG_INF("Device already commissioned");
    }
}
#endif

#if APP_USE_EXAMPLE_START_BUTTON
void AppTaskCommon::ExampleActionButtonEventHandler(void)
{
    AppEvent event;

    if (!GetAppTask().ExampleActionEventHandler)
    {
        return;
    }

    event.Type               = AppEvent::kEventType_Button;
    event.ButtonEvent.Action = kButtonPushEvent;
    event.Handler            = GetAppTask().ExampleActionEventHandler;
    GetAppTask().PostEvent(&event);
}

void AppTaskCommon::SetExampleButtonCallbacks(EventHandler aAction_CB)
{
    ExampleActionEventHandler = aAction_CB;
}
#endif

void AppTaskCommon::ChipEventHandler(const ChipDeviceEvent * event, intptr_t /* arg */)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        sHaveBLEConnections = ConnectivityMgr().NumBLEConnections() != 0;
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        UpdateStatusLED();
#endif
#ifdef CONFIG_CHIP_NFC_COMMISSIONING
        if (event->CHIPoBLEAdvertisingChange.Result == kActivity_Started)
        {
            if (NFCMgr().IsTagEmulationStarted())
            {
                LOG_INF("NFC Tag emulation is already started");
            }
            else
            {
                ShareQRCodeOverNFC(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
            }
        }
        else if (event->CHIPoBLEAdvertisingChange.Result == kActivity_Stopped)
        {
            NFCMgr().StopTagEmulation();
        }
#endif
        break;
    case DeviceEventType::kThreadStateChange:
        sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
        sIsThreadEnabled     = ConnectivityMgr().IsThreadEnabled();
        sIsThreadAttached    = ConnectivityMgr().IsThreadAttached();
#if CONFIG_CHIP_ENABLE_APPLICATION_STATUS_LED
        UpdateStatusLED();
#endif
        break;
    case DeviceEventType::kDnssdInitialized:
#if CONFIG_CHIP_OTA_REQUESTOR
        InitBasicOTARequestor();
        if (GetRequestorInstance()->GetCurrentUpdateState() == Clusters::OtaSoftwareUpdateRequestor::OTAUpdateStateEnum::kIdle)
        {
#endif
#ifdef CONFIG_BOOTLOADER_MCUBOOT
            OtaConfirmNewImage();
#endif /* CONFIG_BOOTLOADER_MCUBOOT */
#if CONFIG_CHIP_OTA_REQUESTOR
        }
#endif
        break;
    default:
        break;
    }
}

void AppTaskCommon::PostEvent(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (k_msgq_put(&sAppEventQueue, aEvent, K_NO_WAIT) != 0)
    {
        LOG_INF("PostEvent fail");
    }
}

void AppTaskCommon::DispatchEvent(AppEvent * aEvent)
{
    if (!aEvent)
        return;
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        LOG_INF("Dropping event without handler");
    }
}

void AppTaskCommon::GetEvent(AppEvent * aEvent)
{
    k_msgq_get(&sAppEventQueue, aEvent, K_FOREVER);
}
