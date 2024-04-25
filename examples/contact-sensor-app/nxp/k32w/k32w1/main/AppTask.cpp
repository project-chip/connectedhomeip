/*
 *
 *    Copyright (c) 2022-2023 Project CHIP Authors
 *    Copyright (c) 2022 Google LLC.
 *    Copyright (c) 2023 NXP
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
#include <app/server/Server.h>
#include <lib/core/ErrorStr.h>

#include <DeviceInfoProviderImpl.h>
#include <app/server/OnboardingCodesUtil.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <inet/EndPointStateOpenThread.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/internal/DeviceNetworkInfo.h>
#if CONFIG_DIAG_LOGS_DEMO
#include "DiagnosticLogsProviderDelegateImpl.h"
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

#include <src/platform/nxp/k32w/k32w1/BLEManagerImpl.h>

#include "K32W1PersistentStorageOpKeystore.h"

#include "LEDWidget.h"
#include "app.h"
#include "app_config.h"
#include "fsl_component_button.h"
#include "fwk_platform.h"

#define FACTORY_RESET_TRIGGER_TIMEOUT 6000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_TASK_PRIORITY 2
#define APP_EVENT_QUEUE_SIZE 10

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

static QueueHandle_t sAppEventQueue;

#if !defined(chip_with_low_power) || (chip_with_low_power == 0)
/*
 * The status LED and the external flash CS pin are wired together.
 * The OTA image writing may fail if used together.
 */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static LEDWidget sStatusLED;
#endif
static LEDWidget sContactSensorLED;
#endif

static bool sIsThreadProvisioned = false;
static bool sHaveBLEConnections  = false;
#if CHIP_ENABLE_LIT
static bool sIsDeviceCommissioned = false;
#endif

static uint32_t eventMask = 0;

#if CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
extern "C" void otPlatUartProcess(void);
#endif

extern "C" void PWR_DisallowDeviceToSleep(void);
extern "C" void PWR_AllowDeviceToSleep(void);

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip;
using namespace chip::app;
#if CONFIG_DIAG_LOGS_DEMO
using namespace chip::app::Clusters::DiagnosticLogs;
#endif

AppTask AppTask::sAppTask;
#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
static AppTask::FactoryDataProvider sFactoryDataProvider;
#endif

static Identify gIdentify = { chip::EndpointId{ 1 }, AppTask::OnIdentifyStart, AppTask::OnIdentifyStop,
                              Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator };

/* OTA related variables */
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static DefaultOTARequestor gRequestorCore __attribute__((section(".data")));
static DefaultOTARequestorStorage gRequestorStorage __attribute__((section(".data")));
static DeviceLayer::DefaultOTARequestorDriver gRequestorUser __attribute__((section(".data")));
static BDXDownloader gDownloader __attribute__((section(".data")));

constexpr uint16_t requestedOtaBlockSize = 1024;
#endif

static pm_notify_element_t appNotifyElement = {
    .notifyCallback = AppTask::LowPowerCallback,
    .data           = NULL,
};

static void app_gap_callback(gapGenericEvent_t * event)
{
    /* This callback is called in the context of BLE task, so event processing
     * should be posted to app task. */
}

static void app_gatt_callback(deviceId_t id, gattServerEvent_t * event)
{
    /* This callback is called in the context of BLE task, so event processing
     * should be posted to app task. */
}

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

    if (ContactSensorMgr().Init() != 0)
    {
        K32W_LOG("ContactSensorMgr().Init() failed");
        assert(0);
    }

    // Register enter/exit low power application callback.
    status_t status = PM_RegisterNotify(kPM_NotifyGroup2, &appNotifyElement);
    if (status != kStatus_Success)
    {
        K32W_LOG("Failed to register low power app callback.")
        return APP_ERROR_PM_REGISTER_LP_CALLBACK_FAILED;
    }

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

#if !defined(chip_with_low_power) || (chip_with_low_power == 0)
    /* start with all LEDS turnedd off */
#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    sStatusLED.Init(SYSTEM_STATE_LED, false);
#endif

    sContactSensorLED.Init(CONTACT_SENSOR_STATE_LED, false);
    sContactSensorLED.Set(ContactSensorMgr().IsContactClosed());
#endif

    UpdateDeviceState();

    /* intialize the Keyboard and button press callback */
    BUTTON_InstallCallback((button_handle_t) g_buttonHandle[0], KBD_Callback, (void *) BLE_BUTTON);
    BUTTON_InstallCallback((button_handle_t) g_buttonHandle[1], KBD_Callback, (void *) CONTACT_SENSOR_BUTTON);

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

    ContactSensorMgr().SetCallback(OnStateChanged);

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

    auto & bleManager = chip::DeviceLayer::Internal::BLEMgrImpl();
    bleManager.RegisterAppCallbacks(app_gap_callback, app_gatt_callback);

    return err;
}

void LockOpenThreadTask(void)
{
    PWR_DisallowDeviceToSleep();
    chip::DeviceLayer::ThreadStackMgr().LockThreadStack();
}

void UnlockOpenThreadTask(void)
{
    chip::DeviceLayer::ThreadStackMgr().UnlockThreadStack();
    PWR_AllowDeviceToSleep();
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

    // Init ZCL Data Model and start server
    chip::Inet::EndPointStateOpenThread::OpenThreadEndpointInitParam nativeParams;
    nativeParams.lockCb                = LockOpenThreadTask;
    nativeParams.unlockCb              = UnlockOpenThreadTask;
    nativeParams.openThreadInstancePtr = chip::DeviceLayer::ThreadStackMgrImpl().OTInstance();
    initParams.endpointNativeParams    = static_cast<void *>(&nativeParams);
    VerifyOrDie((chip::Server::GetInstance().Init(initParams)) == CHIP_NO_ERROR);

#if CONFIG_DIAG_LOGS_DEMO
    char diagLog[CHIP_DEVICE_CONFIG_MAX_DIAG_LOG_SIZE];
    uint16_t diagLogSize = CHIP_DEVICE_CONFIG_MAX_DIAG_LOG_SIZE;

    StorageKeyName keyUser  = LogProvider::GetKeyDiagUserSupport();
    StorageKeyName keyNwk   = LogProvider::GetKeyDiagNetwork();
    StorageKeyName keyCrash = LogProvider::GetKeyDiagCrashLog();

    memset(diagLog, 0, diagLogSize);
    Server::GetInstance().GetPersistentStorage().SyncSetKeyValue(keyUser.KeyName(), diagLog, diagLogSize);

    memset(diagLog, 1, diagLogSize);
    Server::GetInstance().GetPersistentStorage().SyncSetKeyValue(keyNwk.KeyName(), diagLog, diagLogSize);

    memset(diagLog, 2, diagLogSize);
    Server::GetInstance().GetPersistentStorage().SyncSetKeyValue(keyCrash.KeyName(), diagLog, diagLogSize);
#endif
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
        TickType_t xTicksToWait = pdMS_TO_TICKS(10);

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
        xTicksToWait = portMAX_DELAY;
#endif

        BaseType_t eventReceived = xQueueReceive(sAppEventQueue, &event, xTicksToWait);
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

            sHaveBLEConnections = (ConnectivityMgr().NumBLEConnections() != 0);
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

#if !defined(chip_with_low_power) || (chip_with_low_power == 0)
#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        if (sAppTask.mFunction != Function::kFactoryReset && sAppTask.mFunction != Function::kIdentify)
        {
            if (sIsThreadProvisioned)
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
        }

        sStatusLED.Animate();
#endif

        sContactSensorLED.Animate();
#endif
    }
}

void AppTask::ButtonEventHandler(uint8_t pin_no, uint8_t button_action)
{
    if ((pin_no != RESET_BUTTON) && (pin_no != CONTACT_SENSOR_BUTTON) && (pin_no != SOFT_RESET_BUTTON) && (pin_no != BLE_BUTTON))
    {
        return;
    }

    AppEvent button_event;
    button_event.Type               = AppEvent::kButton;
    button_event.ButtonEvent.PinNo  = pin_no;
    button_event.ButtonEvent.Action = button_action;

    if (pin_no == RESET_BUTTON)
    {
        button_event.Handler = ResetActionEventHandler;
    }
    else if (pin_no == CONTACT_SENSOR_BUTTON)
    {
        button_event.Handler = ContactActionEventHandler;
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
#if CHIP_ENABLE_LIT
        else if (button_action == USER_ACTIVE_MODE_TRIGGER_PUSH)
        {
            button_event.Handler = UserActiveModeHandler;
        }
#endif
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
            K32W_LOG("pb1 short press");
            if (sAppTask.mResetTimerActive)
            {
                ButtonEventHandler(BLE_BUTTON, RESET_BUTTON_PUSH);
            }
#if CHIP_ENABLE_LIT
            else if (sIsDeviceCommissioned)
            {
                ButtonEventHandler(BLE_BUTTON, USER_ACTIVE_MODE_TRIGGER_PUSH);
            }
#endif
            else
            {
                ButtonEventHandler(BLE_BUTTON, BLE_BUTTON_PUSH);
            }
            break;

        case CONTACT_SENSOR_BUTTON:
            K32W_LOG("pb2 short press");
            ButtonEventHandler(CONTACT_SENSOR_BUTTON, CONTACT_SENSOR_BUTTON_PUSH);
            break;
        }
        break;

    case kBUTTON_EventLongPress:
        switch (pinNb)
        {
        case BLE_BUTTON:
            K32W_LOG("pb1 long press");
            ButtonEventHandler(BLE_BUTTON, RESET_BUTTON_PUSH);
            break;

        case CONTACT_SENSOR_BUTTON:
            K32W_LOG("pb2 long press");
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
    event.Type               = AppEvent::kTimer;
    event.TimerEvent.Context = (void *) xTimer;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(void * aGenericEvent)
{
    AppEvent * aEvent = (AppEvent *) aGenericEvent;

    if (aEvent->Type != AppEvent::kTimer)
        return;

    K32W_LOG("Device will factory reset...");

    // Actually trigger Factory Reset
    chip::Server::GetInstance().ScheduleFactoryReset();
}

void AppTask::ResetActionEventHandler(void * aGenericEvent)
{
    AppEvent * aEvent = (AppEvent *) aGenericEvent;

    if (aEvent->ButtonEvent.PinNo != RESET_BUTTON && aEvent->ButtonEvent.PinNo != BLE_BUTTON)
        return;

    if (sAppTask.mResetTimerActive)
    {
        sAppTask.CancelTimer();
        sAppTask.mFunction = Function::kNoneSelected;

#if !defined(chip_with_low_power) || (chip_with_low_power == 0)
        /* restore initial state for the LED indicating contact state */
        if (!ContactSensorMgr().IsContactClosed())
        {
            sContactSensorLED.Set(false);
        }
        else
        {
            sContactSensorLED.Set(true);
        }
#endif

        K32W_LOG("Factory Reset was cancelled!");
    }
    else
    {
        uint32_t resetTimeout = FACTORY_RESET_TRIGGER_TIMEOUT;

        if (sAppTask.mFunction != Function::kNoneSelected)
        {
            K32W_LOG("Another function is scheduled. Could not initiate Factory Reset!");
            return;
        }

        K32W_LOG("Factory Reset Triggered. Push the RESET button within %lu ms to cancel!", resetTimeout);
        sAppTask.mFunction = Function::kFactoryReset;

        /* LEDs will start blinking to signal that a Factory Reset was scheduled */
#if !defined(chip_with_low_power) || (chip_with_low_power == 0)
#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        sStatusLED.Set(false);
        sStatusLED.Blink(500);
#endif
        sContactSensorLED.Set(false);
        sContactSensorLED.Blink(500);
#endif

        sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);
    }
}

void AppTask::ContactActionEventHandler(void * aGenericEvent)
{
    AppEvent * aEvent                   = (AppEvent *) aGenericEvent;
    ContactSensorManager::Action action = ContactSensorManager::Action::kInvalid;
    CHIP_ERROR err                      = CHIP_NO_ERROR;
    bool state_changed                  = false;

    if (sAppTask.mFunction != Function::kNoneSelected)
    {
        K32W_LOG("Another function is scheduled. Could not change contact state.");
        return;
    }

    if (aEvent->Type == AppEvent::kContact)
    {
        action = static_cast<ContactSensorManager::Action>(aEvent->ContactEvent.Action);
    }
    else if (aEvent->Type == AppEvent::kButton)
    {
        if (ContactSensorMgr().IsContactClosed())
        {
            action = ContactSensorManager::Action::kSignalLost;
        }
        else
        {
            action = ContactSensorManager::Action::kSignalDetected;
        }

        sAppTask.SetSyncClusterToButtonAction(true);
    }
    else
    {
        err    = APP_ERROR_UNHANDLED_EVENT;
        action = ContactSensorManager::Action::kInvalid;
    }

    if (err == CHIP_NO_ERROR)
    {
        ContactSensorMgr().InitiateAction(action);
    }
}

void AppTask::SoftResetHandler(void * aGenericEvent)
{
    AppEvent * aEvent = (AppEvent *) aGenericEvent;
    if (aEvent->ButtonEvent.PinNo != SOFT_RESET_BUTTON)
        return;

    PlatformMgrImpl().CleanReset();
}

void AppTask::BleHandler(void * aGenericEvent)
{
    AppEvent * aEvent = (AppEvent *) aGenericEvent;

    if (aEvent->ButtonEvent.PinNo != BLE_BUTTON)
        return;

    if (sAppTask.mFunction != Function::kNoneSelected)
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

#if CHIP_ENABLE_LIT
void AppTask::UserActiveModeHandler(void * aGenericEvent)
{
    AppEvent * aEvent = (AppEvent *) aGenericEvent;

    if (aEvent->ButtonEvent.PinNo != BLE_BUTTON)
        return;

    if (sAppTask.mFunction != Function::kNoneSelected)
    {
        K32W_LOG("Another function is scheduled. Could not request ICD Active Mode!");
        return;
    }
    PlatformMgr().ScheduleWork(AppTask::UserActiveModeTrigger, 0);
}

void AppTask::UserActiveModeTrigger(intptr_t arg)
{
    ICDNotifier::GetInstance().NotifyNetworkActivityNotification();
}
#endif

void AppTask::MatterEventHandler(const ChipDeviceEvent * event, intptr_t)
{
    if (event->Type == DeviceEventType::kServiceProvisioningChange && event->ServiceProvisioningChange.IsServiceProvisioned)
    {
        if (event->ServiceProvisioningChange.IsServiceProvisioned)
        {
            sIsThreadProvisioned = TRUE;
        }
        else
        {
            sIsThreadProvisioned = FALSE;
        }
    }
#if CHIP_ENABLE_LIT
    else if (event->Type == DeviceEventType::kCommissioningComplete)
    {
        sIsDeviceCommissioned = TRUE;
    }
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    if (event->Type == DeviceEventType::kDnssdInitialized)
    {
        K32W_LOG("Dnssd platform initialized.");
        PlatformMgr().ScheduleWork(InitOTA, 0);
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

void AppTask::OnStateChanged(ContactSensorManager::State aState)
{
    // If the contact state was changed, update LED state and cluster state (only if button was pressed).
    //  - turn on the contact LED if contact sensor is in closed state.
    //  - turn off the lock LED if contact sensor is in opened state.
    if (ContactSensorManager::State::kContactClosed == aState)
    {
        K32W_LOG("Contact state changed to closed.")
#if !defined(chip_with_low_power) || (chip_with_low_power == 0)
        sContactSensorLED.Set(true);
#endif
    }
    else if (ContactSensorManager::State::kContactOpened == aState)
    {
        K32W_LOG("Contact state changed to opened.")
#if !defined(chip_with_low_power) || (chip_with_low_power == 0)
        sContactSensorLED.Set(false);
#endif
    }

    if (sAppTask.IsSyncClusterToButtonAction())
    {
        sAppTask.UpdateClusterState();
    }

    sAppTask.mFunction = Function::kNoneSelected;
}

void AppTask::OnIdentifyStart(Identify * identify)
{
    if (Clusters::Identify::EffectIdentifierEnum::kBlink == identify->mCurrentEffectIdentifier)
    {
        if (Function::kNoneSelected != sAppTask.mFunction)
        {
            K32W_LOG("Another function is scheduled. Could not initiate Identify process!");
            return;
        }
        K32W_LOG("Identify process has started. Status LED should blink every 0.5 seconds.");
        sAppTask.mFunction = Function::kIdentify;
#if !defined(chip_with_low_power) || (chip_with_low_power == 0)
#ifndef CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
        sStatusLED.Set(false);
        sStatusLED.Blink(500);
#endif
#endif
    }
}

void AppTask::OnIdentifyStop(Identify * identify)
{
    if (Clusters::Identify::EffectIdentifierEnum::kBlink == identify->mCurrentEffectIdentifier)
    {
        K32W_LOG("Identify process has stopped.");
        sAppTask.mFunction = Function::kNoneSelected;
    }
}

status_t AppTask::LowPowerCallback(pm_event_type_t eventType, uint8_t powerState, void * data)
{
    return kStatus_Success;
}

void AppTask::PostContactActionRequest(ContactSensorManager::Action aAction)
{
    AppEvent event;
    event.Type                = AppEvent::kContact;
    event.ContactEvent.Action = static_cast<uint8_t>(aAction);
    event.Handler             = ContactActionEventHandler;
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
            if (taskToWake)
            {
                portYIELD_FROM_ISR(taskToWake);
            }
        }
        else if (!xQueueSend(sAppEventQueue, aEvent, 0))
        {
            K32W_LOG("Failed to post event to app task event queue");
        }
    }
}

void AppTask::DispatchEvent(AppEvent * aEvent)
{
#if defined(chip_with_low_power) && (chip_with_low_power == 1)
    /* specific processing for events sent from App_PostCallbackMessage (see main.cpp) */
    if (aEvent->Type == AppEvent::kEventType_Lp)
    {
        aEvent->Handler(aEvent->param);
    }
    else
#endif

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
extern void logBooleanStateEvent(bool state);
void AppTask::UpdateClusterStateInternal(intptr_t arg)
{
    uint8_t newValue = ContactSensorMgr().IsContactClosed();

    // write the new on/off value
    Protocols::InteractionModel::Status status = app::Clusters::BooleanState::Attributes::StateValue::Set(1, newValue);

    if (status != Protocols::InteractionModel::Status::Success)
    {
        ChipLogError(NotSpecified, "ERR: updating boolean status value %x", to_underlying(status));
    }
    logBooleanStateEvent(newValue);
}

void AppTask::UpdateDeviceState(void)
{
    PlatformMgr().ScheduleWork(UpdateDeviceStateInternal, 0);
}

void AppTask::UpdateDeviceStateInternal(intptr_t arg)
{
    bool stateValueAttrValue = 0;

    /* get onoff attribute value */
    (void) app::Clusters::BooleanState::Attributes::StateValue::Get(1, &stateValueAttrValue);

#if !defined(chip_with_low_power) || (chip_with_low_power == 0)
    /* set the device state */
    sContactSensorLED.Set(stateValueAttrValue);
#endif
}

extern "C" void OTAIdleActivities(void)
{
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    OTA_TransactionResume();
#endif
}
