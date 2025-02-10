/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *    Copyright (c) 2022 Google LLC.
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
#include <data-model-providers/codegen/Instance.h>
#include <lib/core/ErrorStr.h>

#include <DeviceInfoProviderImpl.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <inet/EndPointStateOpenThread.h>
#include <lib/support/ThreadOperationalDataset.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <setup_payload/OnboardingCodesUtil.h>

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
#include <src/platform/nxp/common/ota/OTAImageProcessorImpl.h>
#endif

#include <src/platform/nxp/k32w0/BLEManagerImpl.h>

#include "DefaultTestEventTriggerDelegate.h"
#include "Keyboard.h"
#include "LED.h"
#include "LEDWidget.h"
#include "PWR_Interface.h"
#include "app_config.h"

#ifdef ENABLE_HSM_DEVICE_ATTESTATION
#include "DeviceAttestationSe05xCredsExample.h"
#endif

#define FACTORY_RESET_TRIGGER_TIMEOUT 6000
#define APP_EVENT_QUEUE_SIZE 10

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

static QueueHandle_t sAppEventQueue;

#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
static LEDWidget sStatusLED;
static LEDWidget sContactSensorLED;
#endif

static bool sIsThreadProvisioned = false;
static bool sHaveBLEConnections  = false;
#if CHIP_ENABLE_LIT
static bool sIsDeviceCommissioned = false;
#endif

static uint32_t eventMask = 0;

#if CHIP_DEVICE_CONFIG_THREAD_ENABLE_CLI
extern "C" void K32WUartProcess(void);
#endif

using namespace ::chip::Credentials;
using namespace ::chip::DeviceLayer;
using namespace chip;
using namespace chip::app;

AppTask AppTask::sAppTask;
#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
static chip::DeviceLayer::FactoryDataProviderImpl sFactoryDataProvider;
#if CHIP_DEVICE_CONFIG_USE_CUSTOM_PROVIDER
static chip::DeviceLayer::CustomFactoryDataProvider sCustomFactoryDataProvider;
#endif
#endif

// This key is for testing/certification only and should not be used in production devices.
// For production devices this key must be provided from factory data.
uint8_t sTestEventTriggerEnableKey[TestEventTriggerDelegate::kEnableKeyLength] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
                                                                                   0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff };

static Identify gIdentify = { chip::EndpointId{ 1 }, AppTask::OnIdentifyStart, AppTask::OnIdentifyStop,
                              Clusters::Identify::IdentifyTypeEnum::kVisibleIndicator };

/* OTA related variables */
#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static DefaultOTARequestor gRequestorCore;
static DefaultOTARequestorStorage gRequestorStorage;
static DeviceLayer::DefaultOTARequestorDriver gRequestorUser;
static BDXDownloader gDownloader;

constexpr uint16_t requestedOtaBlockSize = 1024;
#endif

#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA && CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
CHIP_ERROR CustomFactoryDataRestoreMechanism(void)
{
    K32W_LOG("This is a custom factory data restore mechanism.");

    return CHIP_NO_ERROR;
}
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

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
static void CheckOtaEntry()
{
    K32W_LOG("Current OTA_ENTRY_TOP_ADDR: 0x%x", OTA_ENTRY_TOP_ADDR);

    CustomOtaEntries_t ota_entries;
    if (gOtaSuccess_c == OTA_GetCustomEntries(&ota_entries) && ota_entries.ota_state != otaNoImage)
    {
        if (ota_entries.ota_state == otaApplied)
        {
            K32W_LOG("OTA successfully applied");
#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA && CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
            // If this point is reached, it means OTA_CommitCustomEntries was successfully called.
            // Delete the factory data backup to stop doing a restore when the factory data provider
            // is initialized. This ensures that both the factory data and app were updated, otherwise
            // revert to the backed up factory data.
            PDM_vDeleteDataRecord(kNvmId_FactoryDataBackup);
#endif
        }
        else
        {
            K32W_LOG("OTA failed with status %d", ota_entries.ota_state);
        }

        // Clear the entry
        OTA_ResetCustomEntries();
    }
    else
    {
        K32W_LOG("Unable to access OTA entries structure");
    }
}
#endif

CHIP_ERROR AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (ContactSensorMgr().Init() != 0)
    {
        K32W_LOG("ContactSensorMgr().Init() failed");
        assert(0);
    }

    PlatformMgr().AddEventHandler(MatterEventHandler, 0);

    // Init ZCL Data Model and start server
    PlatformMgr().ScheduleWork(InitServer, 0);

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    CheckOtaEntry();
#endif

#if CONFIG_CHIP_LOAD_REAL_FACTORY_DATA
#if CONFIG_CHIP_OTA_FACTORY_DATA_PROCESSOR
    sFactoryDataProvider.RegisterRestoreMechanism(CustomFactoryDataRestoreMechanism);
#endif
    ReturnErrorOnFailure(sFactoryDataProvider.Init());
    SetDeviceInstanceInfoProvider(&sFactoryDataProvider);
    SetDeviceAttestationCredentialsProvider(&sFactoryDataProvider);
    SetCommissionableDataProvider(&sFactoryDataProvider);
#if CHIP_DEVICE_CONFIG_USE_CUSTOM_PROVIDER
    sCustomFactoryDataProvider.ParseFunctionExample();
#endif
#else
#ifdef ENABLE_HSM_DEVICE_ATTESTATION
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleSe05xDACProvider());
#else
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());
#endif
#endif // CONFIG_CHIP_LOAD_REAL_FACTORY_DATA

    // QR code will be used with CHIP Tool
    AppTask::PrintOnboardingInfo();

    /* HW init leds */
#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
    LED_Init();

    /* start with all LEDS turnedd off */
    sStatusLED.Init(SYSTEM_STATE_LED);

    sContactSensorLED.Init(CONTACT_SENSOR_STATE_LED);
    sContactSensorLED.Set(ContactSensorMgr().IsContactClosed());
#endif
    UpdateDeviceState();

    /* intialize the Keyboard and button press callback */
    KBD_Init(KBD_Callback);

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

    K32W_LOG("Current Software Version: %s, %" PRIu32, currentSoftwareVer, currentVersion);

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    /* SSBL will always be seen as booting from address 0, thanks to the remapping mechanism.
     * This means the SSBL version will always offset from address 0. */
    extern uint32_t __MATTER_SSBL_VERSION_START[];
    K32W_LOG("Current SSBL Version: %ld. Found at address 0x%lx", *((uint32_t *) __MATTER_SSBL_VERSION_START),
             (uint32_t) __MATTER_SSBL_VERSION_START);
#endif

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
    initParams.dataModelProvider = CodegenDataModelProviderInstance(initParams.persistentStorageDelegate);

    auto & infoProvider = chip::DeviceLayer::DeviceInfoProviderImpl::GetDefaultInstance();
    infoProvider.SetStorageDelegate(initParams.persistentStorageDelegate);
    chip::DeviceLayer::SetDeviceInfoProvider(&infoProvider);

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
        TickType_t xTicksToWait = pdMS_TO_TICKS(10);

#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
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
            K32WUartProcess();
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

#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
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
        sContactSensorLED.Animate();
#endif
    }
}

void AppTask::ButtonEventHandler(uint8_t pin_no, uint8_t button_action)
{
    if ((pin_no != RESET_BUTTON) && (pin_no != CONTACT_SENSOR_BUTTON) && (pin_no != OTA_BUTTON) && (pin_no != BLE_BUTTON))
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
    else if (pin_no == OTA_BUTTON)
    {
        button_event.Handler = OTAHandler;
    }
    else if (pin_no == BLE_BUTTON)
    {
        button_event.Handler = BleHandler;
#if !(defined OM15082)
        if (button_action == RESET_BUTTON_PUSH)
        {
            button_event.Handler = ResetActionEventHandler;
        }
#endif

#if CHIP_ENABLE_LIT
        if (button_action == USER_ACTIVE_MODE_TRIGGER_PUSH)
        {
            button_event.Handler = UserActiveModeHandler;
        }
#endif
    }

    sAppTask.PostEvent(&button_event);
}

void AppTask::KBD_Callback(uint8_t events)
{
    eventMask = eventMask | (uint32_t) (1 << events);

    HandleKeyboard();
}

void AppTask::HandleKeyboard(void)
{
    uint8_t keyEvent = 0xFF;
    uint8_t pos      = 0;

    while (eventMask)
    {
        for (pos = 0; pos < (8 * sizeof(eventMask)); pos++)
        {
            if (eventMask & (1 << pos))
            {
                keyEvent  = pos;
                eventMask = eventMask & ~(1 << pos);
                break;
            }
        }

        switch (keyEvent)
        {
        case gKBD_EventPB1_c:
            K32W_LOG("pb1 short press");

#if (defined OM15082)
            ButtonEventHandler(RESET_BUTTON, RESET_BUTTON_PUSH);
            break;
#elif CHIP_ENABLE_LIT
            if (sIsDeviceCommissioned)
            {
                ButtonEventHandler(BLE_BUTTON, USER_ACTIVE_MODE_TRIGGER_PUSH);
            }
            else
            {
                ButtonEventHandler(BLE_BUTTON, BLE_BUTTON_PUSH);
            }
            break;
#else
            ButtonEventHandler(BLE_BUTTON, BLE_BUTTON_PUSH);
            break;
#endif
        case gKBD_EventPB2_c:
            ButtonEventHandler(CONTACT_SENSOR_BUTTON, CONTACT_SENSOR_BUTTON_PUSH);
            break;
        case gKBD_EventPB3_c:
            ButtonEventHandler(OTA_BUTTON, OTA_BUTTON_PUSH);
            break;
        case gKBD_EventPB4_c:
#if CHIP_ENABLE_LIT
            if (sIsDeviceCommissioned)
            {
                ButtonEventHandler(BLE_BUTTON, USER_ACTIVE_MODE_TRIGGER_PUSH);
            }
            else
#endif

                ButtonEventHandler(BLE_BUTTON, BLE_BUTTON_PUSH);
            break;
#if !(defined OM15082)
        case gKBD_EventLongPB1_c:
            K32W_LOG("pb1 long press");
            ButtonEventHandler(BLE_BUTTON, RESET_BUTTON_PUSH);
            break;
#endif
        default:
            break;
        }
    }
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

#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
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
#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
        sStatusLED.Set(false);
        sContactSensorLED.Set(false);

        sStatusLED.Blink(500);
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

void AppTask::OTAHandler(void * aGenericEvent)
{
    AppEvent * aEvent = (AppEvent *) aGenericEvent;
    if (aEvent->ButtonEvent.PinNo != OTA_BUTTON)
        return;

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
    if (sAppTask.mFunction != Function::kNoneSelected)
    {
        K32W_LOG("Another function is scheduled. Could not initiate OTA!");
        return;
    }

    PlatformMgr().ScheduleWork(StartOTAQuery, 0);
#endif
}

#if CHIP_DEVICE_CONFIG_ENABLE_OTA_REQUESTOR
void AppTask::StartOTAQuery(intptr_t arg)
{
    GetRequestorInstance()->TriggerImmediateQuery();
}
#endif

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
#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
        sStatusLED.Set(false);
#endif
        K32W_LOG("Stopped BLE Advertising!");
    }
    else
    {
        ConnectivityMgr().SetBLEAdvertisingEnabled(true);

        if (chip::Server::GetInstance().GetCommissioningWindowManager().OpenBasicCommissioningWindow() == CHIP_NO_ERROR)
        {
#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
            sStatusLED.Set(true);
#endif
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
        PlatformMgr().ScheduleWork(AppTask::InitOTA, 0);
    }
#endif

#if CONFIG_CHIP_NFC_ONBOARDING_PAYLOAD
    if (event->Type == DeviceEventType::kCHIPoBLEAdvertisingChange && event->CHIPoBLEAdvertisingChange.Result == kActivity_Stopped)
    {
        if (!NFCOnboardingPayloadMgr().IsTagEmulationStarted())
        {
            K32W_LOG("NFC Tag emulation is already stopped!");
        }
        else
        {
            NFCOnboardingPayloadMgr().StopTagEmulation();
            K32W_LOG("Stopped NFC Tag Emulation!");
        }
    }
    else if (event->Type == DeviceEventType::kCHIPoBLEAdvertisingChange &&
             event->CHIPoBLEAdvertisingChange.Result == kActivity_Started)
    {
        if (NFCOnboardingPayloadMgr().IsTagEmulationStarted())
        {
            K32W_LOG("NFC Tag emulation is already started!");
        }
        else
        {
            ShareQRCodeOverNFC(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
            K32W_LOG("Started NFC Tag Emulation!");
        }
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
#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
        sContactSensorLED.Set(true);
#endif
    }
    else if (ContactSensorManager::State::kContactOpened == aState)
    {
        K32W_LOG("Contact state changed to opened.")
#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
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
#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
        sStatusLED.Set(false);
        sStatusLED.Blink(500);
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
#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
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
#if !defined(nxp_use_low_power) || (nxp_use_low_power == 0)
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

extern "C" bool AppHaveBLEConnections(void)
{
    return sHaveBLEConnections;
}
