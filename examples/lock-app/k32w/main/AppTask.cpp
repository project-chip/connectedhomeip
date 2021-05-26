/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Google LLC.
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
#include "support/ErrorStr.h"
#include <app/server/Server.h>

#include <app/server/OnboardingCodesUtil.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/internal/DeviceNetworkInfo.h>
#include <support/ThreadOperationalDataset.h>

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>
#include <app/util/attribute-storage.h>

#include "Keyboard.h"
#include "LED.h"
#include "LEDWidget.h"
#include "TimersManager.h"
#include "app_config.h"

constexpr uint32_t kFactoryResetTriggerTimeout = 6000;
constexpr uint8_t kAppEventQueueSize           = 10;

TimerHandle_t sFunctionTimer; // FreeRTOS app sw timer.

static SemaphoreHandle_t sCHIPEventLock;
static QueueHandle_t sAppEventQueue;

static LEDWidget sStatusLED;
static LEDWidget sLockLED;

static bool sIsThreadProvisioned     = false;
static bool sIsThreadEnabled         = false;
static bool sHaveBLEConnections      = false;
static bool sHaveServiceConnectivity = false;

static uint32_t eventMask = 0;

using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

int AppTask::StartAppTask()
{
    int err = CHIP_NO_ERROR;

    sAppEventQueue = xQueueCreate(kAppEventQueueSize, sizeof(AppEvent));
    if (sAppEventQueue == NULL)
    {
        K32W_LOG("Failed to allocate app event queue");
        assert(false);
    }

    return err;
}

int AppTask::Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Init ZCL Data Model and start server
    InitServer();

    // QR code will be used with CHIP Tool
    PrintOnboardingCodes(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));

    TMR_Init();

    /* HW init leds */
    LED_Init();

    /* start with all LEDS turnedd off */
    sStatusLED.Init(SYSTEM_STATE_LED);

    sLockLED.Init(LOCK_STATE_LED);
    sLockLED.Set(!BoltLockMgr().IsUnlocked());

    /* intialize the Keyboard and button press calback */
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
        K32W_LOG("app_timer_create() failed");
        assert(err == CHIP_NO_ERROR);
    }

    err = BoltLockMgr().Init();
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("BoltLockMgr().Init() failed");
        assert(err == CHIP_NO_ERROR);
    }

    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    sCHIPEventLock = xSemaphoreCreateMutex();
    if (sCHIPEventLock == NULL)
    {
        K32W_LOG("xSemaphoreCreateMutex() failed");
        assert(err == CHIP_NO_ERROR);
    }

    // Print the current software version
    char currentFirmwareRev[ConfigurationManager::kMaxFirmwareRevisionLength + 1] = { 0 };
    err = ConfigurationMgr().GetFirmwareRevisionString(currentFirmwareRev, sizeof(currentFirmwareRev));
    if (err != CHIP_NO_ERROR)
    {
        K32W_LOG("Get version error");
        assert(err == CHIP_NO_ERROR);
    }

#ifdef CONFIG_CHIP_NFC_COMMISSIONING
    PlatformMgr().AddEventHandler(ThreadProvisioningHandler, 0);
#endif

    K32W_LOG("Current Firmware Version: %s", currentFirmwareRev);

    return err;
}

void AppTask::AppTaskMain(void * pvParameter)
{
    int err;
    AppEvent event;

    err = sAppTask.Init();
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
            sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
            sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
            sHaveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }

        // Update the status LED if factory reset has not been initiated.
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
            if (sHaveServiceConnectivity)
            {
                sStatusLED.Set(true);
            }
            else if (sIsThreadProvisioned && sIsThreadEnabled)
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
        sLockLED.Animate();

        HandleKeyboard();
    }
}

void AppTask::ButtonEventHandler(uint8_t pin_no, uint8_t button_action)
{
    if ((pin_no != RESET_BUTTON) && (pin_no != LOCK_BUTTON) && (pin_no != JOIN_BUTTON) && (pin_no != BLE_BUTTON))
    {
        return;
    }

    AppEvent button_event;
    button_event.Type               = AppEvent::kEventType_Button;
    button_event.ButtonEvent.PinNo  = pin_no;
    button_event.ButtonEvent.Action = button_action;

    if (pin_no == RESET_BUTTON)
    {
        button_event.Handler = ResetActionEventHandler;
    }
    else if (pin_no == LOCK_BUTTON)
    {
        button_event.Handler = LockActionEventHandler;
    }
    else if (pin_no == JOIN_BUTTON)
    {
        button_event.Handler = JoinHandler;
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
    }
    sAppTask.PostEvent(&button_event);
}

void AppTask::KBD_Callback(uint8_t events)
{
    eventMask = eventMask | (uint32_t)(1 << events);
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
#if (defined OM15082)
            ButtonEventHandler(RESET_BUTTON, RESET_BUTTON_PUSH);
            break;
#else
            ButtonEventHandler(BLE_BUTTON, BLE_BUTTON_PUSH);
            break;
#endif
        case gKBD_EventPB2_c:
            ButtonEventHandler(LOCK_BUTTON, LOCK_BUTTON_PUSH);
            break;
        case gKBD_EventPB3_c:
            ButtonEventHandler(JOIN_BUTTON, JOIN_BUTTON_PUSH);
            break;
        case gKBD_EventPB4_c:
            ButtonEventHandler(BLE_BUTTON, BLE_BUTTON_PUSH);
            break;
#if !(defined OM15082)
        case gKBD_EventLongPB1_c:
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
    ConfigurationMgr().InitiateFactoryReset();
}

void AppTask::ResetActionEventHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != RESET_BUTTON && aEvent->ButtonEvent.PinNo != BLE_BUTTON)
        return;

    if (sAppTask.mResetTimerActive)
    {
        sAppTask.CancelTimer();
        sAppTask.mFunction = kFunction_NoneSelected;

        /* restore initial state for the LED indicating Lock state */
        if (BoltLockMgr().IsUnlocked())
        {
            sLockLED.Set(false);
        }
        else
        {
            sLockLED.Set(true);
        }

        K32W_LOG("Factory Reset was cancelled!");
    }
    else
    {
        uint32_t resetTimeout = kFactoryResetTriggerTimeout;

        if (sAppTask.mFunction != kFunction_NoneSelected)
        {
            K32W_LOG("Another function is scheduled. Could not initiate Factory Reset!");
            return;
        }

        K32W_LOG("Factory Reset Triggered. Push the RESET button within %u ms to cancel!", resetTimeout);
        sAppTask.mFunction = kFunction_FactoryReset;

        /* LEDs will start blinking to signal that a Factory Reset was scheduled */
        sStatusLED.Set(false);
        sLockLED.Set(false);

        sStatusLED.Blink(500);
        sLockLED.Blink(500);

        sAppTask.StartTimer(kFactoryResetTriggerTimeout);
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    BoltLockManager::Action_t action;
    int err        = CHIP_NO_ERROR;
    int32_t actor  = 0;
    bool initiated = false;

    if (sAppTask.mFunction != kFunction_NoneSelected)
    {
        K32W_LOG("Another function is scheduled. Could not initiate Lock/Unlock!");
        return;
    }

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<BoltLockManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        if (BoltLockMgr().IsUnlocked())
        {
            action = BoltLockManager::LOCK_ACTION;
        }
        else
        {
            action = BoltLockManager::UNLOCK_ACTION;
        }
    }
    else
    {
        err = CHIP_ERROR_MAX;
    }

    if (err == CHIP_NO_ERROR)
    {
        initiated = BoltLockMgr().InitiateAction(actor, action);

        if (!initiated)
        {
            K32W_LOG("Action is already in progress or active.");
        }
    }
}

void AppTask::ThreadStart()
{
    chip::Thread::OperationalDataset dataset{};

    constexpr uint8_t xpanid[]    = { 0xde, 0xad, 0x00, 0xbe, 0xef, 0x00, 0xca, 0xfe };
    constexpr uint8_t masterkey[] = {
        0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
    };
    constexpr uint16_t panid   = 0xabcd;
    constexpr uint16_t channel = 15;

    dataset.SetNetworkName("OpenThread");
    dataset.SetExtendedPanId(xpanid);
    dataset.SetMasterKey(masterkey);
    dataset.SetPanId(panid);
    dataset.SetChannel(channel);

    ThreadStackMgr().SetThreadEnabled(false);
    ThreadStackMgr().SetThreadProvision(dataset.AsByteSpan());
    ThreadStackMgr().SetThreadEnabled(true);
}

void AppTask::JoinHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != JOIN_BUTTON)
        return;

    if (sAppTask.mFunction != kFunction_NoneSelected)
    {
        K32W_LOG("Another function is scheduled. Could not initiate Thread Join!");
        return;
    }

    /* hard-code Thread Commissioning Parameters for the moment.
     * In a future PR, these parameters will be sent via BLE.
     */
    ThreadStart();
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

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        ConnectivityMgr().SetBLEAdvertisingEnabled(false);
        K32W_LOG("Stopped BLE Advertising!");
    }
    else
    {
        ConnectivityMgr().SetBLEAdvertisingEnabled(true);

        if (OpenDefaultPairingWindow(chip::ResetAdmins::kNo) == CHIP_NO_ERROR)
        {
            K32W_LOG("Started BLE Advertising!");
        }
        else
        {
            K32W_LOG("OpenDefaultPairingWindow() failed");
        }
    }
}

#ifdef CONFIG_CHIP_NFC_COMMISSIONING
void AppTask::ThreadProvisioningHandler(const ChipDeviceEvent * event, intptr_t)
{
    if (event->Type == DeviceEventType::kCHIPoBLEAdvertisingChange && event->CHIPoBLEAdvertisingChange.Result == kActivity_Stopped)
    {
        if (!NFCMgr().IsTagEmulationStarted())
        {
            K32W_LOG("NFC Tag emulation is already stopped!");
        }
        else
        {
            NFCMgr().StopTagEmulation();
            K32W_LOG("Stopped NFC Tag Emulation!");
        }
    }
    else if (event->Type == DeviceEventType::kCHIPoBLEAdvertisingChange &&
             event->CHIPoBLEAdvertisingChange.Result == kActivity_Started)
    {
        if (NFCMgr().IsTagEmulationStarted())
        {
            K32W_LOG("NFC Tag emulation is already started!");
        }
        else
        {
            ShareQRCodeOverNFC(chip::RendezvousInformationFlags(chip::RendezvousInformationFlag::kBLE));
            K32W_LOG("Started NFC Tag Emulation!");
        }
    }
}
#endif

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

void AppTask::ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        K32W_LOG("Lock Action has been initiated")
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        K32W_LOG("Unlock Action has been initiated")
    }

    sAppTask.mFunction = kFunctionLockUnlock;
    sLockLED.Blink(50, 50);
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        K32W_LOG("Lock Action has been completed")
        sLockLED.Set(true);
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        K32W_LOG("Unlock Action has been completed")
        sLockLED.Set(false);
    }

    sAppTask.mFunction = kFunction_NoneSelected;
}

void AppTask::PostLockActionRequest(int32_t aActor, BoltLockManager::Action_t aAction)
{
    AppEvent event;
    event.Type             = AppEvent::kEventType_Lock;
    event.LockEvent.Actor  = aActor;
    event.LockEvent.Action = aAction;
    event.Handler          = LockActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(const AppEvent * aEvent)
{
    if (sAppEventQueue != NULL)
    {
        if (!xQueueSend(sAppEventQueue, aEvent, 1))
        {
            K32W_LOG("Failed to post event to app task event queue");
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
    uint8_t newValue = !BoltLockMgr().IsUnlocked();

    // write the new on/off value
    EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER,
                                                 (uint8_t *) &newValue, ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        ChipLogError(NotSpecified, "ERR: updating on/off %x", status);
    }
}
