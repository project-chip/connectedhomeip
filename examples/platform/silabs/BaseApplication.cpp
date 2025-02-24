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

/**********************************************************
 * Includes
 *********************************************************/

#include "AppConfig.h"
#include "AppEvent.h"
#include "AppTask.h"
#include "OTAConfig.h"
#include <app/server/Dnssd.h>
#include <app/server/Server.h>

#define APP_ACTION_BUTTON 1

#ifdef DISPLAY_ENABLED
#include "lcd.h"
#ifdef QR_CODE_ENABLED
#include "qrcodegen.h"
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <app/icd/server/ICDNotifier.h> // nogncheck
#ifdef ENABLE_CHIP_SHELL
#include <ICDShellCommands.h>
#endif // ENABLE_CHIP_SHELL
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#include <app/util/attribute-storage.h>
#include <assert.h>
#include <headers/ProvisionManager.h>
#include <lib/support/CodeUtils.h>
#include <platform/CHIPDeviceLayer.h>
#include <setup_payload/OnboardingCodesUtil.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <sl_cmsis_os2_common.h>

#if CHIP_ENABLE_OPENTHREAD
#include <platform/OpenThread/OpenThreadUtils.h>
#include <platform/ThreadStackManager.h>
#include <platform/silabs/ConfigurationManagerImpl.h>
#include <platform/silabs/ThreadStackManagerImpl.h>
#endif // CHIP_ENABLE_OPENTHREAD

#include <platform/silabs/platformAbstraction/SilabsPlatform.h>

#ifdef SL_WIFI
#include <app/clusters/network-commissioning/network-commissioning.h>
#include <platform/silabs/NetworkCommissioningWiFiDriver.h>
#include <platform/silabs/wifi/WifiInterface.h>

#if CHIP_CONFIG_ENABLE_ICD_SERVER
#include <platform/silabs/wifi/icd/WifiSleepManager.h>
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#endif // SL_WIFI

#ifdef DIC_ENABLE
#include "dic.h"
#include "dic_control.h"
#endif // DIC_ENABLE

#ifdef PERFORMANCE_TEST_ENABLED
#include <performance_test_commands.h>
#endif // PERFORMANCE_TEST_ENABLED

/**********************************************************
 * Defines and Constants
 *********************************************************/

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#ifndef APP_TASK_STACK_SIZE
#define APP_TASK_STACK_SIZE (4096)
#endif
#ifndef APP_EVENT_QUEUE_SIZE // Allow apps to define a different app queue size
#define APP_EVENT_QUEUE_SIZE 10
#endif
#define EXAMPLE_VENDOR_ID 0xcafe

#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
#define SYSTEM_STATE_LED 0
#endif // ENABLE_WSTK_LEDS
#define APP_FUNCTION_BUTTON 0

using namespace chip;
using namespace chip::app;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Silabs;

namespace {

/**********************************************************
 * Variable declarations
 *********************************************************/

osTimerId_t sFunctionTimer;
osTimerId_t sLightTimer;
osThreadId_t sAppTaskHandle;
osMessageQueueId_t sAppEventQueue;

#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
LEDWidget sStatusLED;
#endif // ENABLE_WSTK_LEDS

#ifdef SL_WIFI
app::Clusters::NetworkCommissioning::Instance
    sWiFiNetworkCommissioningInstance(0 /* Endpoint Id */, &(NetworkCommissioning::SlWiFiDriver::GetInstance()));
#endif /* SL_WIFI */

bool sIsEnabled  = false;
bool sIsAttached = false;

#if !(CHIP_CONFIG_ENABLE_ICD_SERVER)
bool sHaveBLEConnections = false;
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

constexpr uint32_t kLightTimerPeriod = static_cast<uint32_t>(pdMS_TO_TICKS(10));

uint8_t sAppEventQueueBuffer[APP_EVENT_QUEUE_SIZE * sizeof(AppEvent)];
osMessageQueue_t sAppEventQueueStruct;
constexpr osMessageQueueAttr_t appEventQueueAttr = { .cb_mem  = &sAppEventQueueStruct,
                                                     .cb_size = osMessageQueueCbSize,
                                                     .mq_mem  = sAppEventQueueBuffer,
                                                     .mq_size = sizeof(sAppEventQueueBuffer) };

uint8_t appStack[APP_TASK_STACK_SIZE];
osThread_t appTaskControlBlock;
constexpr osThreadAttr_t appTaskAttr = { .name       = APP_TASK_NAME,
                                         .attr_bits  = osThreadDetached,
                                         .cb_mem     = &appTaskControlBlock,
                                         .cb_size    = osThreadCbSize,
                                         .stack_mem  = appStack,
                                         .stack_size = APP_TASK_STACK_SIZE,
                                         .priority   = osPriorityNormal };

#ifdef DISPLAY_ENABLED
SilabsLCD slLCD;
#endif

#ifdef MATTER_DM_PLUGIN_IDENTIFY_SERVER
Clusters::Identify::EffectIdentifierEnum sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;

Identify gIdentify = {
    chip::EndpointId{ 1 },
    BaseApplication::OnIdentifyStart,
    BaseApplication::OnIdentifyStop,
    Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator,
    BaseApplication::OnTriggerIdentifyEffect,
};

#endif // MATTER_DM_PLUGIN_IDENTIFY_SERVER

} // namespace

bool BaseApplication::sIsProvisioned                  = false;
bool BaseApplication::sIsFactoryResetTriggered        = false;
LEDWidget * BaseApplication::sAppActionLed            = nullptr;
BaseApplicationDelegate BaseApplication::sAppDelegate = BaseApplicationDelegate();

void BaseApplicationDelegate::OnCommissioningSessionStarted()
{
    isComissioningStarted = true;

#if SL_WIFI && CHIP_CONFIG_ENABLE_ICD_SERVER
    WifiSleepManager::GetInstance().HandleCommissioningSessionStarted();
#endif // SL_WIFI && CHIP_CONFIG_ENABLE_ICD_SERVER
}

void BaseApplicationDelegate::OnCommissioningSessionStopped()
{
    isComissioningStarted = false;

#if SL_WIFI && CHIP_CONFIG_ENABLE_ICD_SERVER
    WifiSleepManager::GetInstance().HandleCommissioningSessionStopped();
#endif // SL_WIFI && CHIP_CONFIG_ENABLE_ICD_SERVER
}

void BaseApplicationDelegate::OnCommissioningSessionEstablishmentError(CHIP_ERROR err)
{
    isComissioningStarted = false;

#if SL_WIFI && CHIP_CONFIG_ENABLE_ICD_SERVER
    WifiSleepManager::GetInstance().HandleCommissioningSessionStopped();
#endif // SL_WIFI && CHIP_CONFIG_ENABLE_ICD_SERVER
}

void BaseApplicationDelegate::OnCommissioningWindowClosed()
{
    if (BaseApplication::GetProvisionStatus())
    {
        // After the device is provisioned and the commissioning passed
        // resetting the isCommissioningStarted to false
        isComissioningStarted = false;
#ifdef DISPLAY_ENABLED
#ifdef QR_CODE_ENABLED
        SilabsLCD::Screen_e screen;
        slLCD.GetScreen(screen);
        VerifyOrReturn(screen == SilabsLCD::Screen_e::QRCodeScreen);
        slLCD.SetScreen(SilabsLCD::Screen_e::DemoScreen);
#endif // QR_CODE_ENABLED
#endif // DISPLAY_ENABLED
    }
}

void BaseApplicationDelegate::OnFabricCommitted(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    // If we commissioned our first fabric, Update the commissioned status of the App
    if (fabricTable.FabricCount() == 1)
    {
        BaseApplication::UpdateCommissioningStatus(true);
    }
}

void BaseApplicationDelegate::OnFabricRemoved(const FabricTable & fabricTable, FabricIndex fabricIndex)
{
    if (fabricTable.FabricCount() == 0)
    {
        BaseApplication::UpdateCommissioningStatus(false);

        BaseApplication::DoProvisioningReset();
    }
}

/**********************************************************
 * AppTask Definitions
 *********************************************************/

CHIP_ERROR BaseApplication::StartAppTask(osThreadFunc_t taskFunction)
{
    sAppEventQueue = osMessageQueueNew(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent), &appEventQueueAttr);
    if (sAppEventQueue == NULL)
    {
        ChipLogError(AppServer, "Failed to allocate app event queue");
        appError(APP_ERROR_EVENT_QUEUE_FAILED);
    }

    // Start App task.
    sAppTaskHandle = osThreadNew(taskFunction, &sAppEventQueue, &appTaskAttr);
    if (sAppTaskHandle == nullptr)
    {
        ChipLogError(AppServer, "Failed to create app task");
        appError(APP_ERROR_CREATE_TASK_FAILED);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BaseApplication::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#ifdef SL_WIFI
    /*
     * Wait for the WiFi to be initialized
     */
    ChipLogProgress(AppServer, "APP: Wait WiFi Init");
    while (!IsStationReady())
    {
        osDelay(pdMS_TO_TICKS(10));
    }
    ChipLogProgress(AppServer, "APP: Done WiFi Init");
    /* We will init server when we get IP */

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    sWiFiNetworkCommissioningInstance.Init();
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

#endif

    // Create cmsis os sw timer for Function Selection.
    sFunctionTimer = osTimerNew(FunctionTimerEventHandler, // timer callback handler
                                osTimerOnce,               // no timer reload (one-shot timer)
                                (void *) this,             // pass the app task obj context
                                NULL                       // No osTimerAttr_t to provide.
    );
    if (sFunctionTimer == NULL)
    {
        ChipLogError(AppServer, "funct timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    // Create cmsis os sw timer for LED Management.
    sLightTimer = osTimerNew(LightTimerEventHandler, // Timer callback handler"LightTmr",
                             osTimerPeriodic,        // timer repeats automatically
                             (void *) this,          // pass the app task obj context
                             NULL                    // No osTimerAttr_t to provide.
    );
    if (sLightTimer == NULL)
    {
        ChipLogError(AppServer, "Light Timer create failed");
        appError(APP_ERROR_CREATE_TIMER_FAILED);
    }

    ChipLogProgress(AppServer, "Current Software Version String: %s", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION_STRING);
    ChipLogProgress(AppServer, "Current Software Version: %d", CHIP_DEVICE_CONFIG_DEVICE_SOFTWARE_VERSION);

    ConfigurationMgr().LogDeviceConfig();

    OutputQrCode(true /*refreshLCD at init*/);
#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
    LEDWidget::InitGpio();
    sStatusLED.Init(SYSTEM_STATE_LED);
#endif // ENABLE_WSTK_LEDS

#ifdef ENABLE_CHIP_SHELL
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    ICDCommands::RegisterCommands();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#endif // ENABLE_CHIP_SHELL

#ifdef PERFORMANCE_TEST_ENABLED
    RegisterPerfTestCommands();
#endif // PERFORMANCE_TEST_ENABLED

    PlatformMgr().AddEventHandler(OnPlatformEvent, 0);
#ifdef SL_WIFI
    BaseApplication::sIsProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
#endif /* SL_WIFI */
#if CHIP_ENABLE_OPENTHREAD
    BaseApplication::sIsProvisioned = ConnectivityMgr().IsThreadProvisioned();
#endif

    err = chip::Server::GetInstance().GetFabricTable().AddFabricDelegate(&sAppDelegate);
    return err;
}

void BaseApplication::FunctionTimerEventHandler(void * timerCbArg)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = timerCbArg;
    event.Handler            = FunctionEventHandler;
    PostEvent(&event);
}

void BaseApplication::FunctionEventHandler(AppEvent * aEvent)
{
    VerifyOrReturn(aEvent->Type == AppEvent::kEventType_Timer);
    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT,
    if (!sIsFactoryResetTriggered)
    {
        StartFactoryResetSequence();
    }
    else
    {
        // The factory reset sequence was in motion. The cancellation window expired.
        // Factory Reset the device now.
#if CHIP_CONFIG_ENABLE_ICD_SERVER
        StopStatusLEDTimer();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

        ScheduleFactoryReset();
    }
}

bool BaseApplication::ActivateStatusLedPatterns()
{
    bool isPatternSet = false;
#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
#ifdef MATTER_DM_PLUGIN_IDENTIFY_SERVER
    if (gIdentify.mActive)
    {
        // Identify in progress
        // Do a steady blink on the status led
        sStatusLED.Blink(250, 250);
        isPatternSet = true;
    }
    else if (sIdentifyEffect != Clusters::Identify::EffectIdentifierEnum::kStopEffect)
    {
        // Identify trigger effect received. Do some on/off patterns on the status led
        if (sIdentifyEffect == Clusters::Identify::EffectIdentifierEnum::kBlink)
        {
            // Fast blink
            sStatusLED.Blink(50, 50);
        }
        else if (sIdentifyEffect == Clusters::Identify::EffectIdentifierEnum::kBreathe)
        {
            // Slow blink
            sStatusLED.Blink(1000, 1000);
        }
        else if (sIdentifyEffect == Clusters::Identify::EffectIdentifierEnum::kOkay)
        {
            // Pulse effect
            sStatusLED.Blink(300, 700);
        }
        else if (sIdentifyEffect == Clusters::Identify::EffectIdentifierEnum::kChannelChange)
        {
            // Alternate between Short and Long pulses effect
            static uint64_t mLastChangeTimeMS = 0;
            static bool alternatePattern      = false;
            uint32_t onTimeMS                 = alternatePattern ? 50 : 700;
            uint32_t offTimeMS                = alternatePattern ? 950 : 300;

            uint64_t nowMS = chip::System::SystemClock().GetMonotonicMilliseconds64().count();
            if (nowMS >= mLastChangeTimeMS + 1000) // each pattern is done over a 1 second period
            {
                mLastChangeTimeMS = nowMS;
                alternatePattern  = !alternatePattern;
                sStatusLED.Blink(onTimeMS, offTimeMS);
            }
        }
        isPatternSet = true;
    }
#endif // MATTER_DM_PLUGIN_IDENTIFY_SERVER

#if !(CHIP_CONFIG_ENABLE_ICD_SERVER)
    // Identify Patterns have priority over Status patterns
    if (!isPatternSet)
    {
        // Apply different status feedbacks
        if (BaseApplication::sIsProvisioned && sIsEnabled)
        {
            if (sIsAttached)
            {
                sStatusLED.Set(true);
            }
            else
            {
                sStatusLED.Blink(950, 50);
            }
        }
        else if (sHaveBLEConnections)
        {
            sStatusLED.Blink(100, 100);
        }
        else
        {
            sStatusLED.Blink(50, 950);
        }
        isPatternSet = true;
    }
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#endif // ENABLE_WSTK_LEDS) && SL_CATALOG_SIMPLE_LED_LED1_PRESENT
    return isPatternSet;
}

void BaseApplication::UpdateCommissioningStatus(bool newState)
{
#ifdef SL_WIFI
    BaseApplication::sIsProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
    sIsEnabled                      = ConnectivityMgr().IsWiFiStationEnabled();
    sIsAttached                     = ConnectivityMgr().IsWiFiStationConnected();
#endif /* SL_WIFI */
#if CHIP_ENABLE_OPENTHREAD
    // TODO: This is a temporary solution until we can read Thread provisioning status from RAM instead of NVM.
    BaseApplication::sIsProvisioned = newState;
    sIsEnabled                      = ConnectivityMgr().IsThreadEnabled();
    sIsAttached                     = ConnectivityMgr().IsThreadAttached();
#endif /* CHIP_ENABLE_OPENTHREAD */

    ActivateStatusLedPatterns();
}

// TODO Move State Monitoring elsewhere
void BaseApplication::LightEventHandler()
{
    // Collect connectivity and configuration state from the CHIP stack. Because
    // the CHIP event loop is being run in a separate task, the stack must be
    // locked while these values are queried.  However we use a non-blocking
    // lock request (TryLockCHIPStack()) to avoid blocking other UI activities
    // when the CHIP task is busy (e.g. with a long crypto operation).
#if !(CHIP_CONFIG_ENABLE_ICD_SERVER)
    if (PlatformMgr().TryLockChipStack())
    {
#ifdef SL_WIFI
        BaseApplication::sIsProvisioned = ConnectivityMgr().IsWiFiStationProvisioned();
        sIsEnabled                      = ConnectivityMgr().IsWiFiStationEnabled();
        sIsAttached                     = ConnectivityMgr().IsWiFiStationConnected();
#endif /* SL_WIFI */
#if CHIP_ENABLE_OPENTHREAD
        sIsEnabled  = ConnectivityMgr().IsThreadEnabled();
        sIsAttached = ConnectivityMgr().IsThreadAttached();
#endif /* CHIP_ENABLE_OPENTHREAD */
        sHaveBLEConnections = (ConnectivityMgr().NumBLEConnections() != 0);

        PlatformMgr().UnlockChipStack();
    }

#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#if defined(ENABLE_WSTK_LEDS)
#ifdef SL_CATALOG_SIMPLE_LED_LED1_PRESENT
    // Update the status LED if factory reset has not been initiated.
    //
    // If system has "full connectivity", keep the LED On constantly.
    //
    // If thread and service provisioned, but not attached to the thread network
    // yet OR no connectivity to the service OR subscriptions are not fully
    // established THEN blink the LED Off for a short period of time.
    //
    // If the system has ble connection(s) uptill the stage above, THEN blink
    // the LEDs at an even rate of 100ms.
    //
    // Otherwise, blink the LED ON for a very short time.
    if (!sIsFactoryResetTriggered)
    {
        ActivateStatusLedPatterns();
    }

    sStatusLED.Animate();
#endif // SL_CATALOG_SIMPLE_LED_LED1_PRESENT
    if (sAppActionLed)
    {
        sAppActionLed->Animate();
    }
#endif // ENABLE_WSTK_LEDS
}

void BaseApplication::ButtonHandler(AppEvent * aEvent)
{
    // To trigger software update: press the APP_FUNCTION_BUTTON button briefly (<
    // FACTORY_RESET_TRIGGER_TIMEOUT) To initiate factory reset: press the
    // APP_FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT +
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT All LEDs start blinking after
    // FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the APP_FUNCTION_BUTTON once all LEDs
    // start blinking within the FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == static_cast<uint8_t>(SilabsPlatform::ButtonAction::ButtonPressed))
    {
        StartFunctionTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
    }
    else
    {
        if (sIsFactoryResetTriggered)
        {
            CancelFactoryResetSequence();
        }
        else
        {
            // The factory reset sequence was not initiated,
            // Press and Release:
            // - Open the commissioning window and start BLE advertisement in fast mode when not  commissioned
            // - Output qr code in logs
            // - Cycle LCD screen
            CancelFunctionTimer();

            AppTask::GetAppTask().UpdateDisplay();

#ifdef SL_WIFI
            if (!ConnectivityMgr().IsWiFiStationProvisioned())
#else
            if (!BaseApplication::sIsProvisioned)
#endif /* !SL_WIFI */
            {
                // Open Basic CommissioningWindow. Will start BLE advertisements
                chip::DeviceLayer::PlatformMgr().LockChipStack();
                CHIP_ERROR err = chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
                chip::DeviceLayer::PlatformMgr().UnlockChipStack();
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(AppServer, "Failed to open the Basic Commissioning Window");
                }
            }
            else
            {
                ChipLogProgress(AppServer, "Network is already provisioned, Ble advertisement not enabled");
#if CHIP_CONFIG_ENABLE_ICD_SERVER
                // Temporarily claim network activity, until we implement a "user trigger" reason for ICD wakeups.
                PlatformMgr().ScheduleWork([](intptr_t) { ICDNotifier::GetInstance().NotifyNetworkActivityNotification(); });
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
            }
        }
    }
}

void BaseApplication::UpdateDisplay()
{
    OutputQrCode(false);
#ifdef DISPLAY_ENABLED
    UpdateLCDStatusScreen();
    slLCD.CycleScreens();
#endif
}

void BaseApplication::CancelFunctionTimer()
{
    if (osTimerStop(sFunctionTimer) == osError)
    {
        ChipLogError(AppServer, "app timer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

void BaseApplication::StartFunctionTimer(uint32_t aTimeoutInMs)
{
    // Starts or restarts the function timer
    if (osTimerStart(sFunctionTimer, pdMS_TO_TICKS(aTimeoutInMs)) != osOK)
    {
        ChipLogError(AppServer, "app timer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void BaseApplication::StartFactoryResetSequence()
{
    // Initiate the factory reset sequence
    ChipLogProgress(AppServer, "Factory Reset Triggered. Release button within %ums to cancel.",
                    FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

    // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to
    // cancel, if required.
    StartFunctionTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);

    sIsFactoryResetTriggered = true;
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    StartStatusLEDTimer();
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER

#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
    // Turn off all LEDs before starting blink to make sure blink is
    // co-ordinated.
    sStatusLED.Set(false);
    sStatusLED.Blink(500);
#endif // ENABLE_WSTK_LEDS
}

void BaseApplication::CancelFactoryResetSequence()
{
    CancelFunctionTimer();

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    StopStatusLEDTimer();
#endif
    if (sIsFactoryResetTriggered)
    {
        sIsFactoryResetTriggered = false;
        ChipLogProgress(AppServer, "Factory Reset has been cancelled");
    }
}

void BaseApplication::StartStatusLEDTimer()
{
    if (osTimerStart(sLightTimer, kLightTimerPeriod) != osOK)
    {
        ChipLogError(AppServer, "Light Time start failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }
}

void BaseApplication::StopStatusLEDTimer()
{
#if (defined(ENABLE_WSTK_LEDS) && (defined(SL_CATALOG_SIMPLE_LED_LED1_PRESENT)))
    sStatusLED.Set(false);
#endif // ENABLE_WSTK_LEDS

    if (osTimerStop(sLightTimer) == osError)
    {
        ChipLogError(AppServer, "Light Time start failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

#ifdef MATTER_DM_PLUGIN_IDENTIFY_SERVER
void BaseApplication::OnIdentifyStart(Identify * identify)
{
    ChipLogProgress(Zcl, "onIdentifyStart");

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    StartStatusLEDTimer();
#endif
}

void BaseApplication::OnIdentifyStop(Identify * identify)
{
    ChipLogProgress(Zcl, "onIdentifyStop");

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    StopStatusLEDTimer();
#endif
}

void BaseApplication::OnTriggerIdentifyEffectCompleted(chip::System::Layer * systemLayer, void * appState)
{
    ChipLogProgress(Zcl, "Trigger Identify Complete");
    sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    StopStatusLEDTimer();
#endif
}

void BaseApplication::OnTriggerIdentifyEffect(Identify * identify)
{
    sIdentifyEffect = identify->mCurrentEffectIdentifier;

    if (identify->mEffectVariant != Clusters::Identify::EffectVariantEnum::kDefault)
    {
        ChipLogDetail(AppServer, "Identify Effect Variant unsupported. Using default");
    }

#if CHIP_CONFIG_ENABLE_ICD_SERVER
    StartStatusLEDTimer();
#endif

    switch (sIdentifyEffect)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(5), OnTriggerIdentifyEffectCompleted,
                                                           identify);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(10), OnTriggerIdentifyEffectCompleted,
                                                           identify);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
        (void) chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(1), OnTriggerIdentifyEffectCompleted,
                                                           identify);
        break;
    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        (void) chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerIdentifyEffectCompleted, identify);
        break;
    default:
        sIdentifyEffect = Clusters::Identify::EffectIdentifierEnum::kStopEffect;
        ChipLogProgress(Zcl, "No identifier effect");
    }
}
#endif // MATTER_DM_PLUGIN_IDENTIFY_SERVER

void BaseApplication::LightTimerEventHandler(void * timerCbArg)
{
    LightEventHandler();
}

#ifdef DISPLAY_ENABLED
SilabsLCD & BaseApplication::GetLCD(void)
{
    return slLCD;
}

void BaseApplication::UpdateLCDStatusScreen(bool withChipStackLock)
{
    SilabsLCD::DisplayStatus_t status;
    bool enabled, attached;
    if (withChipStackLock)
    {
        chip::DeviceLayer::PlatformMgr().LockChipStack();
    }
#ifdef SL_WIFI
    enabled  = ConnectivityMgr().IsWiFiStationEnabled();
    attached = ConnectivityMgr().IsWiFiStationConnected();
    chip::DeviceLayer::NetworkCommissioning::Network network;
    memset(reinterpret_cast<void *>(&network), 0, sizeof(network));
    chip::DeviceLayer::NetworkCommissioning::GetConnectedNetwork(network);
    if (network.networkIDLen)
    {
        chip::Platform::CopyString(status.networkName, sizeof(status.networkName),
                                   reinterpret_cast<const char *>(network.networkID));
    }
#endif /* SL_WIFI */
#if CHIP_ENABLE_OPENTHREAD
    enabled  = ConnectivityMgr().IsThreadEnabled();
    attached = ConnectivityMgr().IsThreadAttached();
#endif /* CHIP_ENABLE_OPENTHREAD */
    status.connected   = enabled && attached;
    status.advertising = chip::Server::GetInstance().GetCommissioningWindowManager().IsCommissioningWindowOpen();
    status.nbFabric    = chip::Server::GetInstance().GetFabricTable().FabricCount();
#if CHIP_CONFIG_ENABLE_ICD_SERVER
    status.icdMode = (ICDConfigurationData::GetInstance().GetICDMode() == ICDConfigurationData::ICDMode::SIT)
        ? SilabsLCD::ICDMode_e::SIT
        : SilabsLCD::ICDMode_e::LIT;
#endif
    if (withChipStackLock)
    {
        chip::DeviceLayer::PlatformMgr().UnlockChipStack();
    }
    slLCD.SetStatus(status);
}
#endif

void BaseApplication::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != nullptr)
    {
        if (osMessageQueuePut(sAppEventQueue, aEvent, osPriorityNormal, 0) != osOK)
        {
            ChipLogError(AppServer, "Failed to post event to app task event queue");
        }
    }
    else
    {
        ChipLogError(AppServer, "App Event Queue is uninitialized");
    }
}

void BaseApplication::DispatchEvent(AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(aEvent);
    }
    else
    {
        ChipLogProgress(AppServer, "Event received with no handler. Dropping event.");
    }
}

void BaseApplication::ScheduleFactoryReset()
{
    PlatformMgr().ScheduleWork([](intptr_t) {
        // Press both buttons to request provisioning
        if (GetPlatform().GetButtonState(APP_ACTION_BUTTON))
        {
            Provision::Manager::GetInstance().SetProvisionRequired(true);
        }
#if SL_WIFI
        // Removing the matter services on factory reset
        chip::Dnssd::ServiceAdvertiser::Instance().RemoveServices();
#endif
        PlatformMgr().HandleServerShuttingDown(); // HandleServerShuttingDown calls OnShutdown() which is only implemented for the
                                                  // basic information cluster it seems. And triggers and Event flush, which is not
                                                  // relevant when there are no fabrics left
        ConfigurationMgr().InitiateFactoryReset();
    });
}

void BaseApplication::DoProvisioningReset()
{
    PlatformMgr().ScheduleWork([](intptr_t) {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        ConfigurationManagerImpl::GetDefaultInstance().ClearThreadStack();
        ThreadStackMgrImpl().FactoryResetThreadStack();
        ThreadStackMgr().InitThreadStack();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

#if CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
        ChipLogProgress(DeviceLayer, "Clearing WiFi provision");
        chip::DeviceLayer::ConnectivityMgr().ClearWiFiStationProvision();
#endif // CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION

        CHIP_ERROR err = Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(AppServer, "Failed to open the Basic Commissioning Window");
        }
    });
}

#if SILABS_OTA_ENABLED
void BaseApplication::InitOTARequestorHandler(System::Layer * systemLayer, void * appState)
{
    OTAConfig::Init();
}
#endif

void BaseApplication::OnPlatformEvent(const ChipDeviceEvent * event, intptr_t)
{
    switch (event->Type)
    {
    case DeviceEventType::kServiceProvisioningChange: {
        // Note: This is only called on Attach, we need to add a method to detect Thread Network Detach
        BaseApplication::sIsProvisioned = event->ServiceProvisioningChange.IsServiceProvisioned;
    }
    break;

    case DeviceEventType::kThreadConnectivityChange:
    case DeviceEventType::kInternetConnectivityChange: {
#ifdef DIC_ENABLE
        if (event->InternetConnectivityChange.IPv4 == kConnectivity_Established)
        {
            if (DIC_OK != dic_init(dic::control::subscribeCB))
            {
                ChipLogError(AppServer, "dic_init failed");
            }
        }
#endif // DIC_ENABLE
#ifdef DISPLAY_ENABLED
        SilabsLCD::Screen_e screen;
        AppTask::GetLCD().GetScreen(screen);
        // Update the LCD screen with SSID and connected state
        if (screen == SilabsLCD::Screen_e::StatusScreen)
        {
            BaseApplication::UpdateLCDStatusScreen(false);
            AppTask::GetLCD().SetScreen(screen);
        }
#endif // DISPLAY_ENABLED
        if ((event->ThreadConnectivityChange.Result == kConnectivity_Established) ||
            (event->InternetConnectivityChange.IPv6 == kConnectivity_Established))
        {
#if SL_WIFI
            chip::app::DnssdServer::Instance().StartServer();

#if CHIP_CONFIG_ENABLE_ICD_SERVER
            WifiSleepManager::GetInstance().VerifyAndTransitionToLowPowerMode(WifiSleepManager::PowerEvent::kConnectivityChange);
#endif // CHIP_CONFIG_ENABLE_ICD_SERVER
#endif // SL_WIFI

#if SILABS_OTA_ENABLED
            ChipLogProgress(AppServer, "Scheduling OTA Requestor initialization");
            chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds32(OTAConfig::kInitOTARequestorDelaySec),
                                                        InitOTARequestorHandler, nullptr);
#endif // SILABS_OTA_ENABLED
        }
    }
    break;

    case DeviceEventType::kCommissioningComplete: {
#if SL_WIFI && CHIP_CONFIG_ENABLE_ICD_SERVER
        WifiSleepManager::GetInstance().VerifyAndTransitionToLowPowerMode(WifiSleepManager::PowerEvent::kCommissioningComplete);
#endif // SL_WIFI && CHIP_CONFIG_ENABLE_ICD_SERVER
    }
    break;
    default:
        break;
    }
}

void BaseApplication::OutputQrCode(bool refreshLCD)
{
    (void) refreshLCD; // could be unused

    // Create buffer for the Qr code setup payload that can fit max size and null terminator.
    char setupPayloadBuffer[chip::QRCodeBasicSetupPayloadGenerator::kMaxQRCodeBase38RepresentationLength + 1];
    chip::MutableCharSpan setupPayload(setupPayloadBuffer);

    CHIP_ERROR err = Provision::Manager::GetInstance().GetStorage().GetSetupPayload(setupPayload);
    if (CHIP_NO_ERROR == err)
    {
        // Print setup info on LCD if available
#ifdef QR_CODE_ENABLED
        if (refreshLCD)
        {
            slLCD.SetQRCode((uint8_t *) setupPayload.data(), setupPayload.size());
            slLCD.ShowQRCode(true);
        }
#endif // QR_CODE_ENABLED

        PrintQrCodeURL(setupPayload);
    }
    else
    {
        ChipLogError(AppServer, "Getting QR code failed!");
    }
}

bool BaseApplication::GetProvisionStatus()
{
    return BaseApplication::sIsProvisioned;
}
