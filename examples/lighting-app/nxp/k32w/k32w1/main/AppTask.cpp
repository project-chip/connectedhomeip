/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2021 Google LLC.
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
#include "AppEvent.h"
#include <app/server/OnboardingCodesUtil.h>
#include <app/server/Server.h>
#include <lib/core/ErrorStr.h>

#include <app/server/OnboardingCodesUtil.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <inet/EndPointStateOpenThread.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <src/platform/nxp/k32w/k32w1/DefaultTestEventTriggerDelegate.h>
#if defined(USE_SMU2_DYNAMIC)
#include <src/platform/nxp/k32w/k32w1/SMU2Manager.h>
#endif

#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/attribute-storage.h>

/* OTA related includes */
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
#include "OtaSupport.h"
#include <app/clusters/ota-requestor/BDXDownloader.h>
#include <app/clusters/ota-requestor/DefaultOTARequestor.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorDriver.h>
#include <app/clusters/ota-requestor/DefaultOTARequestorStorage.h>
#include <src/platform/nxp/k32w/common/OTAImageProcessorImpl.h>
#endif

#include "K32W1PersistentStorageOpKeystore.h"

#include "LEDWidget.h"
#include "app.h"
#include "app_config.h"
#include "fsl_component_button.h"
#include "fwk_platform.h"

#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
#include "LED_Dimmer.h"
#endif

#define FACTORY_RESET_TRIGGER_TIMEOUT 6000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

static QueueHandle_t sAppEventQueue;

/*
 * The status LED and the external flash CS pin are wired together.
 * The OTA image writing may fail if used together.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static LEDWidget sStatusLED;
#endif
static LEDWidget sLightLED;

static bool sIsThreadProvisioned = false;
#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static bool sHaveFullConnectivity = false;
#endif
static bool sHaveBLEConnections = false;

#if CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
extern "C" void otPlatUartProcess(void);
#endif

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip;
using namespace chip::app;

AppTask AppTask::sAppTask;
#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
static AppTask::FactoryDataProvider sFactoryDataProvider;
#endif

// This key is for testing/certification only and should not be used in production devices.
// For production devices this key must be provided from factory data.
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

static Identify gIdentify = { chip::EndpointId{ 1 }, AppTask::OnIdentifyStart, AppTask::OnIdentifyStop,
                              Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator, AppTask::OnTriggerEffect,
                              // Use invalid value for identifiers to enable TriggerEffect command
                              // to stop Identify command for each effect
                              Clusters::Identify::EffectIdentifierEnum::kUnknownEnumValue,
                              Clusters::Identify::EffectVariantEnum::kDefault };

/* OTA related variables */
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static DefaultOTARequestor gRequestorCore;
static DefaultOTARequestorStorage gRequestorStorage;
static DeviceLayer::DefaultOTARequestorDriver gRequestorUser;
static BDXDownloader gDownloader;

constexpr uint16_t requestedOtaBlockSize = 1024;
#endif

CHIP_ERROR AppTask::StartAppTask()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    sAppEventQueue = xQueueCreate(APP_EVENT_QUEUE_SIZE, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        err = APP_ERROR_EVENT_QUEUE_FAILED;
        K32W_LOG("Failed to allocate app event queue");
        assert(err == CHIP_NO_ERROR);
    }

    return err;
}

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    PlatformMgr().AddEventHandler(MatterEventHandler, 0);

    // Init ZCL Data Model and start server
    PlatformMgr().ScheduleWork(InitServer, 0);

#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
    ReturnErrorOnFailure(sFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);
    SetCommissionableDataProvider(&sFactoryDataProvider);
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif // CONFIG_CHIP_LOAD_REAL_FACTORY_DATA

    // QR code will be used with CHIP Tool
    AppTask::PrintOnboardingInfo();

    if (LightingMgr().Init() != 0)
    {
        K32W_LOG("LightingMgr().Init() failed");
        assert(0);
    }

    LightingMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    /* start with all LEDS turnedd off */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    sStatusLED.Init(SYSTEM_STATE_LED, false);
#endif

#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
    init_dimmable();
#else
    sLightLED.Init(LIGHT_STATE_LED, false);
#endif

    UpdateDeviceState();

    /* intialize the Keyboard and button press callback */
    BUTTON_InstallCallback((button_handle_t) g_buttonHandle[0], KBD_Callback, (void *) BLE_BUTTON);
    BUTTON_InstallCallback((button_handle_t) g_buttonHandle[1], KBD_Callback, (void *) LIGHT_BUTTON);

    // Create FreeRTOS sw timer for Function Selection.
    sFunctionTimer = xTimerCreate("FnTmr",          // Just a text name, not used by the RTOS kernel
                                  1,                // == default timer period (mS)
                                  false,            // no timer reload (==one-shot)
                                  (void *) this,    // init timer id = app task obj context
                                  TimerEventHandler // timer callback handler
    );

    if (sFunctionTimer == NULL)
    {
        err = APP_ERROR_CREATE_TIMER_FAILED;
        K32W_LOG("app_timer_create() failed");
        assert(err == CHIP_NO_ERROR);
    }

    // Print the current software version
    char currentSoftwareVer[ConfigurationManager::kMaxSoftwareVersionStringLength + 1] = { 0 };
    err = ConfigurationMgr().GetSoftwareVersionString(currentSoftwareVer, sizeof(currentSoftwareVer));
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("Get version error");
        assert(err == CHIP_NO_ERROR);
    }

    uint32_t currentVersion;
    err = ConfigurationMgr().GetSoftwareVersion(currentVersion);

    K32W_LOG("Current Software Version: %s, %d", currentSoftwareVer, currentVersion);

    return err;
}

void LockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
}

void AppTask::InitServer(intptr_t arg)
{
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

#if CHIP_CRYPTO_PLATFORM
    static chip::K32W1PersistentStorageOpKeystore sK32W1PersistentStorageOpKeystore;
    VerifyOrDie((sK32W1PersistentStorageOpKeystore.Init(initParams.persistentStorageDelegate)) == CHIP_NO_ERROR);
    initParams.operationalKeystore = &sK32W1PersistentStorageOpKeystore;
#endif

#if defined(USE_SMU2_DYNAMIC)
    VerifyOrDie(SMU2::Init(initParams.persistentStorageDelegate) == CHIP_NO_ERROR);
#endif

    // Init ZCL Data Model and start server
    static DefaultTestEventTriggerDelegate sTestEventTriggerDelegate{ ByteSpan(sTestEventTriggerEnableKey) };
    initParams.testEventTriggerDelegate = &sTestEventTriggerDelegate;
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
    VerifyOrDie((chip::Server::GetInstance().Init(initParams)) == CHIP_NO_ERROR);
}

void AppTask::PrintOnboardingInfo()
{
    chip::PayloadContents payload;
    CHIP_ERROR err = GetPayloadContents(payload, chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(AppServer, "GetPayloadContents() failed: %" CHIP_ERROR_FORMAT, err.Format());
    }
    payload.commissioningFlow = chip::CommissioningFlow::kUserActionRequired;
    PrintOnboardingCodes(payload);
}

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
void AppTask::InitOTA(intptr_t arg)
{
    // Initialize and interconnect the Requestor and Image Processor objects -- START
    SetRequestorInstance(&gRequestorCore);

    gRequestorStorage.Init(chip::Server::GetInstance().GetPersistentStorage());
    gRequestorCore.Init(chip::Server::GetInstance(), gRequestorStorage, gRequestorUser, gDownloader);
    gRequestorUser.SetMaxDownloadBlockSize(requestedOtaBlockSize);
    auto & imageProcessor = OTAImageProcessorImpl::GetDefaultInstance();
    gRequestorUser.Init(&gRequestorCore, &imageProcessor);
    CHIP_ERROR err = imageProcessor.Init(&gDownloader);
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("Image processor init failed");
        assert(err == CHIP_NO_ERROR);
    }

    // Connect the gDownloader and Image Processor objects
    gDownloader.SetImageProcessorDelegate(&imageProcessor);
    // Initialize and interconnect the Requestor and Image Processor objects -- END
}
#endif

void AppTask::AppTaskMain(void * pvParameter)
{
    AppEvent event;

    CHIP_ERROR err = sAppTask.Init();
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("AppTask.Init() failed");
        assert(err == CHIP_NO_ERROR);
    }

    while (true)
    {
        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, pdMS_TO_TICKS(10));
        while (eventReceived == pdTRUE)
        {
            sAppTask.DispatchEvent(&event);
            eventReceived = xQueueReceive(sAppEventQueue, &event, 0);
        }

        // Collect connectivity and configuration state from the CHIP stack.  Because the
        // CHIP event loop is being run in a separate task, the stack must be locked
        // while these values are queried.  However we use a non-blocking lock request
        // (TryLockChipStack()) to avoid blocking other UI activities when the CHIP
        // task is busy (e.g. with a long crypto operation).
        if (PlatformMgr().TryLockChipStack())
        {
#if CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
            otPlatUartProcess();
#endif

            sIsThreadProvisioned = ConnectivityMgr().IsThreadProvisioned();
            sHaveBLEConnections  = (ConnectivityMgr().NumBLEConnections() != 0);
            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset or identify process have not been initiated.
        //
        // If system has "full connectivity", keep the LED On constantly.
        //
        // If thread and service provisioned, but not attached to the thread network yet OR no
        // connectivity to the service OR subscriptions are not fully established
        // THEN blink the LED Off for a short period of time.
        //
        // If the system has ble connection(s) uptill the stage above, THEN blink the LEDs at an even
        // rate of 100ms.
        //
        // Otherwise, blink the LED ON for a very short time.
        if (sAppTask.mFunction != kFunction_FactoryReset)
        {
#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
            if (sHaveFullConnectivity)
            {
                sStatusLED.Set(true);
            }
            else if (sIsThreadProvisioned)
            {
                sStatusLED.Blink(950, 50);
            }
            else if (sHaveBLEConnections)
            {
                sStatusLED.Blink(100, 100);
            }
            else
            {
                sStatusLED.Blink(50, 950);
            }
#endif
        }

#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        sStatusLED.Animate();
#endif
        sLightLED.Animate();
    }
}

void AppTask::ButtonEventHandler(uint8_t pin_no, uint8_t button_action)
{
    if ((pin_no != RESET_BUTTON) && (pin_no != LIGHT_BUTTON) && (pin_no != SOFT_RESET_BUTTON) && (pin_no != BLE_BUTTON))
    {
        return;
    }

    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.PinNo  = pin_no;
    button_event.ButtonEvent.Action = button_action;

    if (pin_no == LIGHT_BUTTON)
    {
        button_event.Handler = LightActionEventHandler;
    }
    else if (pin_no == SOFT_RESET_BUTTON)
    {
        // Soft reset ensures that platform manager shutdown procedure is called.
        button_event.Handler = SoftResetHandler;
    }
    else if (pin_no == BLE_BUTTON)
    {
        button_event.Handler = BleHandler;

        if (button_action == RESET_BUTTON_PUSH)
        {
            button_event.Handler = ResetActionEventHandler;
        }
    }
    sAppTask.PostEvent(&button_event);
}

button_status_t AppTask::KBD_Callback(void * buttonHandle, button_callback_message_t * message, void * callbackParam)
{
    uint32_t pinNb = (uint32_t) callbackParam;
    switch (message->event)
    {
    case kBUTTON_EventOneClick:
    case kBUTTON_EventShortPress:
        switch (pinNb)
        {
        case BLE_BUTTON:
            // K32W_LOG("pb1 short press");
            if (sAppTask.mResetTimerActive)
            {
                ButtonEventHandler(BLE_BUTTON, RESET_BUTTON_PUSH);
            }
            else
            {
                ButtonEventHandler(BLE_BUTTON, BLE_BUTTON_PUSH);
            }
            break;

        case LIGHT_BUTTON:
            // K32W_LOG("pb2 short press");
            ButtonEventHandler(LIGHT_BUTTON, LIGHT_BUTTON_PUSH);
            break;
        }
        break;

    case kBUTTON_EventLongPress:
        switch (pinNb)
        {
        case BLE_BUTTON:
            // K32W_LOG("pb1 long press");
            ButtonEventHandler(BLE_BUTTON, RESET_BUTTON_PUSH);
            break;

        case LIGHT_BUTTON:
            // K32W_LOG("pb2 long press");
            ButtonEventHandler(SOFT_RESET_BUTTON, SOFT_RESET_BUTTON_PUSH);
            break;
        }
        break;

    default:
        /* No action required */
        break;
    }
    return kStatus_BUTTON_Success;
}

void AppTask::TimerEventHandler(TimerHandle_t xTimer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = (void *) xTimer;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;

    K32W_LOG("Device will factory reset...");

    // Actually trigger Factory Reset
    chip::Server::GetInstance().ScheduleFactoryReset();
}

void AppTask::ResetActionEventHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != RESET_BUTTON && aEvent->ButtonEvent.PinNo != BLE_BUTTON)
        return;

    if (sAppTask.mResetTimerActive)
    {
        sAppTask.CancelTimer();
        sAppTask.mFunction = kFunction_NoneSelected;

        RestoreLightingState();

        K32W_LOG("Factory Reset was cancelled!");
    }
    else
    {
        uint32_t resetTimeout = FACTORY_RESET_TRIGGER_TIMEOUT;

        if (sAppTask.mFunction != kFunction_NoneSelected)
        {
            K32W_LOG("Another function is scheduled. Could not initiate Factory Reset!");
            return;
        }

        K32W_LOG("Factory Reset Triggered. Push the RESET button within %lu ms to cancel!", resetTimeout);
        sAppTask.mFunction = kFunction_FactoryReset;

        /* LEDs will start blinking to signal that a Factory Reset was scheduled */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        sStatusLED.Set(false);
#endif

#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
        sLightLED.SetLevel(0);
#else
        sLightLED.Set(false);
#endif

#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        sStatusLED.Blink(500);
#endif
        sLightLED.Blink(500);

        sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
    }
}

void AppTask::LightActionEventHandler(AppEvent * aEvent)
{
    LightingManager::Action_t action;
    CHIP_ERROR err = CHIP_NO_ERROR;
    int32_t actor  = 0;
    bool initiated = false;

    if (sAppTask.mFunction != kFunction_NoneSelected)
    {
        K32W_LOG("Another function is scheduled. Could not initiate ON/OFF Light command!");
        return;
    }

    if (aEvent->Type == AppEvent::kEventType_TurnOn)
    {
        action = static_cast<LightingManager::Action_t>(aEvent->LightEvent.Action);
        actor  = aEvent->LightEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        actor = AppEvent::kEventType_Button;

        if (LightingMgr().IsTurnedOff())
        {
            action = LightingManager::TURNON_ACTION;
        }
        else
        {
            action = LightingManager::TURNOFF_ACTION;
        }
    }
    else
    {
        err    = APP_ERROR_UNHANDLED_EVENT;
        action = LightingManager::INVALID_ACTION;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = LightingMgr().InitiateAction(actor, action, LightingMgr().IsTurnedOff() ? 0 : 1);

        if (!initiated)
        {
            K32W_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::SoftResetHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != SOFT_RESET_BUTTON)
        return;

    PlatformMgrImpl().CleanReset();
}

void AppTask::BleHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != BLE_BUTTON)
        return;

    if (sAppTask.mFunction != kFunction_NoneSelected)
    {
        K32W_LOG("Another function is scheduled. Could not toggle BLE state!");
        return;
    }
    PlatformMgr().ScheduleWork(AppTask::BleStartAdvertising, 0);
}

void AppTask::BleStartAdvertising(intptr_t arg)
{
    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        ConnectivityMgr().SetBLEAdvertisingEnabled(false);
        K32W_LOG("Stopped BLE Advertising!");
    }
    else
    {
        ConnectivityMgr().SetBLEAdvertisingEnabled(true);
        if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() == CHIP_NO_ERROR)
        {
            K32W_LOG("Started BLE Advertising!");
        }
        else
        {
            K32W_LOG("OpenBasicCommissioningWindow() failed");
        }
    }
}

void AppTask::MatterEventHandler(const ChipDeviceEvent * event, intptr_t)
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    if (event->Type == DeviceEventType::kDnssdInitialized)
    {
        K32W_LOG("Dnssd platform initialized.");
        PlatformMgr().ScheduleWork(InitOTA, 0);
    }
#else
    if (event->Type == DeviceEventType::kDnssdInitialized)
    {
        sHaveFullConnectivity = TRUE;
    }
#endif
}

void AppTask::CancelTimer()
{
    if (xTimerStop(sFunctionTimer, 0) == pdFAIL)
    {
        K32W_LOG("app timer stop() failed");
    }

    mResetTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sFunctionTimer))
    {
        K32W_LOG("app timer already started!");
        CancelTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sFunctionTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        K32W_LOG("app timer start() failed");
    }

    mResetTimerActive = true;
}

void AppTask::ActionInitiated(LightingManager::Action_t aAction, int32_t aActor)
{
    // start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == LightingManager::TURNON_ACTION)
    {
        K32W_LOG("Turn on Action has been initiated")
    }
    else if (aAction == LightingManager::TURNOFF_ACTION)
    {
        K32W_LOG("Turn off Action has been initiated")
    }
    else if (aAction == LightingManager::DIM_ACTION)
    {
        K32W_LOG("Dim Action has been initiated");
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.mSyncClusterToButtonAction = true;
    }

    sAppTask.mFunction = kFunctionTurnOnTurnOff;
}

void AppTask::ActionCompleted(LightingManager::Action_t aAction, uint8_t level)
{
    // Turn on the light LED if in a TURNON state OR
    // Turn off the light LED if in a TURNOFF state.
    if (aAction == LightingManager::TURNON_ACTION)
    {
        K32W_LOG("Turn on action has been completed")
#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
#else
        sLightLED.Set(true);
#endif
    }
    else if (aAction == LightingManager::TURNOFF_ACTION)
    {
        K32W_LOG("Turn off action has been completed")
#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
#else
        sLightLED.Set(false);
#endif
    }
    else if (aAction == LightingManager::DIM_ACTION)
    {
        K32W_LOG("Move to level %d completed", level);
    }
#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
    sLightLED.SetLevel(LightingMgr().IsTurnedOff() ? 1 : LightingMgr().GetDimLevel());
#endif

    if (sAppTask.mSyncClusterToButtonAction)
    {
        sAppTask.UpdateClusterState();
        sAppTask.mSyncClusterToButtonAction = false;
    }

    sAppTask.mFunction = kFunction_NoneSelected;
}

void AppTask::RestoreLightingState(void)
{
#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
    LightingMgr().SetState(!LightingMgr().IsTurnedOff());
#else
    /* restore initial state for the LED indicating Lighting state */
    if (LightingMgr().IsTurnedOff())
    {
        sLightLED.Set(false);
    }
    else
    {
        sLightLED.Set(true);
    }
#endif
}

void AppTask::OnIdentifyStart(Identify * identify)
{
    if ((kFunction_NoneSelected != sAppTask.mFunction) && (kFunction_TriggerEffect != sAppTask.mFunction))
    {
        K32W_LOG("Another function is scheduled. Could not initiate Identify process!");
        return;
    }

    if (kFunction_TriggerEffect == sAppTask.mFunction)
    {
        chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerEffectComplete, identify);
        OnTriggerEffectComplete(&chip::DeviceLayer::SystemLayer(), identify);
    }

    ChipLogProgress(Zcl, "Identify process has started. Status LED should blink with a period of 0.5 seconds.");
    sAppTask.mFunction = kFunction_Identify;
#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
    sLightLED.SetLevel(0);
#else
    sLightLED.Set(false);
#endif
    sLightLED.Blink(250);
}

void AppTask::OnIdentifyStop(Identify * identify)
{
    if (kFunction_Identify == sAppTask.mFunction)
    {
        ChipLogProgress(Zcl, "Identify process has stopped.");
        sAppTask.mFunction = kFunction_NoneSelected;

        RestoreLightingState();
    }
}

void AppTask::OnTriggerEffectComplete(chip::System::Layer * systemLayer, void * appState)
{
    // Let Identify command take over if called during TriggerEffect already running
    if (kFunction_TriggerEffect == sAppTask.mFunction)
    {
        ChipLogProgress(Zcl, "TriggerEffect has stopped.");
        sAppTask.mFunction = kFunction_NoneSelected;

        // TriggerEffect finished - reset identifiers
        // Use invalid value for identifiers to enable TriggerEffect command
        // to stop Identify command for each effect
        gIdentify.mCurrentEffectIdentifier = Clusters::Identify::EffectIdentifierEnum::kUnknownEnumValue;
        gIdentify.mTargetEffectIdentifier  = Clusters::Identify::EffectIdentifierEnum::kUnknownEnumValue;
        gIdentify.mEffectVariant           = Clusters::Identify::EffectVariantEnum::kDefault;

        RestoreLightingState();
    }
}

void AppTask::OnTriggerEffect(Identify * identify)
{
    // Allow overlapping TriggerEffect calls
    if ((kFunction_NoneSelected != sAppTask.mFunction) && (kFunction_TriggerEffect != sAppTask.mFunction))
    {
        K32W_LOG("Another function is scheduled. Could not initiate Identify process!");
        return;
    }

    sAppTask.mFunction  = kFunction_TriggerEffect;
    uint16_t timerDelay = 0;

    ChipLogProgress(Zcl, "TriggerEffect has started.");

    switch (identify->mCurrentEffectIdentifier)
    {
    case Clusters::Identify::EffectIdentifierEnum::kBlink:
        timerDelay = 2;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kBreathe:
        timerDelay = 15;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kOkay:
        timerDelay = 4;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kChannelChange:
        ChipLogProgress(Zcl, "Channel Change effect not supported, using effect %d",
                        to_underlying(Clusters::Identify::EffectIdentifierEnum::kBlink));
        timerDelay = 2;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kFinishEffect:
        chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerEffectComplete, identify);
        timerDelay = 1;
        break;

    case Clusters::Identify::EffectIdentifierEnum::kStopEffect:
        chip::DeviceLayer::SystemLayer().CancelTimer(OnTriggerEffectComplete, identify);
        OnTriggerEffectComplete(&chip::DeviceLayer::SystemLayer(), identify);
        break;

    default:
        ChipLogProgress(Zcl, "Invalid effect identifier.");
    }

    if (timerDelay)
    {
#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
        sLightLED.SetLevel(0);
#else
        sLightLED.Set(false);
#endif
        sLightLED.Blink(500);

        chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Seconds16(timerDelay), OnTriggerEffectComplete, identify);
    }
}

void AppTask::PostTurnOnActionRequest(int32_t aActor, LightingManager::Action_t aAction)
{
    AppEvent event;
    event.Type              = AppEvent::kEventType_TurnOn;
    event.LightEvent.Actor  = aActor;
    event.LightEvent.Action = aAction;
    event.Handler           = LightActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    portBASE_TYPE taskToWake = pdFALSE;
    if (sAppEventQueue != NULL)
    {
        if (__get_IPSR())
        {
            if (!xQueueSendToFrontFromISR(sAppEventQueue, aEvent, &taskToWake))
            {
                K32W_LOG("Failed to post event to app task event queue");
            }

            portYIELD_FROM_ISR(taskToWake);
        }
        else
        {
            if (!xQueueSend(sAppEventQueue, aEvent, 1))
            {
                K32W_LOG("Failed to post event to app task event queue");
            }
        }
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
        K32W_LOG("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState(void)
{
    PlatformMgr().ScheduleWork(UpdateClusterStateInternal, 0);
}

void AppTask::UpdateClusterStateInternal(intptr_t arg)
{
    uint8_t newValue = !LightingMgr().IsTurnedOff();

    // write the new on/off value
    Protocols::InteractionModel::Status status = app::Clusters::OnOff::Attributes::OnOff::Set(1, newValue);
    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: updating on/off %x", to_underlying(status));
    }
}

void AppTask::UpdateDeviceState(void)
{
    PlatformMgr().ScheduleWork(UpdateDeviceStateInternal, 0);
}

void AppTask::UpdateDeviceStateInternal(intptr_t arg)
{
    bool onoffAttrValue = 0;

    /* get onoff attribute value */
    (void) app::Clusters::OnOff::Attributes::OnOff::Get(1, &onoffAttrValue);

    /* set the device state */
#if CHIP_CONFIG_ENABLE_DIMMABLE_LED
#else
    sLightLED.Set(onoffAttrValue);
#endif
    LightingMgr().SetState(onoffAttrValue);
}

extern "C" void OTAIdleActivities(void)
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    OTA_TransactionResume();
#endif
}
