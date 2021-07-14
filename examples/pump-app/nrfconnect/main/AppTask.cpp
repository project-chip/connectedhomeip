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

#include "AppTask.h"
#include "AppConfig.h"
#include "LEDWidget.h"
#include "OnboardingCodesUtil.h"
#include "PumpManager.h"
#include "Server.h"
#include "Service.h"
#include "ThreadUtil.h"

#ifdef CONFIG_CHIP_NFC_COMMISSIONING
#include "NFCWidget.h"
#endif

#include "attribute-storage.h"

#include <app/common/gen/attribute-id.h>
#include <app/common/gen/attribute-type.h>
#include <app/common/gen/cluster-id.h>

#include <platform/CHIPDeviceLayer.h>

#include <dk_buttons_and_leds.h>
#include <logging/log.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>
#include <zephyr.h>

#include <algorithm>

#define FACTORY_RESET_TRIGGER_TIMEOUT 3000
#define FACTORY_RESET_CANCEL_WINDOW_TIMEOUT 3000
#define APP_EVENT_QUEUE_SIZE 10
#define BUTTON_PUSH_EVENT 1
#define BUTTON_RELEASE_EVENT 0

LOG_MODULE_DECLARE(app);
K_MSGQ_DEFINE(sAppEventQueue, sizeof(AppEvent), APP_EVENT_QUEUE_SIZE, alignof(AppEvent));

constexpr uint32_t kPublishServicePeriodUs = 5000000;

static LEDWidget sStatusLED;
static LEDWidget sLockLED;
static LEDWidget sUnusedLED;
static LEDWidget sUnusedLED_1;

#ifdef CONFIG_CHIP_NFC_COMMISSIONING
static NFCWidget sNFC;
#endif

static bool sIsThreadProvisioned     = false;
static bool sIsThreadEnabled         = false;
static bool sHaveBLEConnections      = false;
static bool sHaveServiceConnectivity = false;

static k_timer sFunctionTimer;

using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

int AppTask::Init()
{
    // Initialize LEDs
    LEDWidget::InitGpio();

    sStatusLED.Init(SYSTEM_STATE_LED);
    sLockLED.Init(LOCK_STATE_LED);
    sLockLED.Set(!PumpMgr().IsUnlocked());

    sUnusedLED.Init(DK_LED3);
    sUnusedLED_1.Init(DK_LED4);

    // Initialize buttons
    int ret = dk_buttons_init(ButtonEventHandler);
    if (ret)
    {
        LOG_ERR("dk_buttons_init() failed");
        return ret;
    }

    // Initialize timer user data
    k_timer_init(&sFunctionTimer, &AppTask::TimerEventHandler, nullptr);
    k_timer_user_data_set(&sFunctionTimer, this);

    PumpMgr().Init();
    PumpMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    // Init ZCL Data Model and start server
    InitServer();
    ConfigurationMgr().LogDeviceConfig();
    PrintOnboardingCodes(chip::RendezvousInformationFlags::kBLE);

#ifdef CONFIG_CHIP_NFC_COMMISSIONING
    ret = sNFC.Init(ConnectivityMgr());
    if (ret)
    {
        LOG_ERR("NFC initialization failed");
        return ret;
    }

    PlatformMgr().AddEventHandler(AppTask::ThreadProvisioningHandler, 0);
#endif

    return 0;
}

int AppTask::StartApp()
{
    int ret                            = Init();
    uint64_t mLastPublishServiceTimeUS = 0;

    if (ret)
    {
        LOG_ERR("AppTask.Init() failed");
        return ret;
    }

    AppEvent event = {};

    while (true)
    {
        ret = k_msgq_get(&sAppEventQueue, &event, K_MSEC(10));

        while (!ret)
        {
            DispatchEvent(&event);
            ret = k_msgq_get(&sAppEventQueue, &event, K_NO_WAIT);
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
        sUnusedLED.Animate();
        sUnusedLED_1.Animate();

        uint64_t nowUS            = chip::System::Clock::GetMonotonicMicroseconds();
        uint64_t nextChangeTimeUS = mLastPublishServiceTimeUS + kPublishServicePeriodUs;

        if (nowUS > nextChangeTimeUS)
        {
            PublishService();
            mLastPublishServiceTimeUS = nowUS;
        }
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    PumpManager::Action_t action = PumpManager::INVALID_ACTION;
    int32_t actor                = 0;

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<PumpManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = PumpMgr().IsUnlocked() ? PumpManager::LOCK_ACTION : PumpManager::UNLOCK_ACTION;
        actor  = AppEvent::kEventType_Button;
    }

    if (action != PumpManager::INVALID_ACTION && !PumpMgr().InitiateAction(actor, action))
        LOG_INF("Action is already in progress or active.");
}

void AppTask::ButtonEventHandler(uint32_t button_state, uint32_t has_changed)
{
    AppEvent button_event;
    button_event.Type = AppEvent::kEventType_Button;

    if (LOCK_BUTTON_MASK & button_state & has_changed)
    {
        button_event.ButtonEvent.PinNo  = LOCK_BUTTON;
        button_event.ButtonEvent.Action = BUTTON_PUSH_EVENT;
        button_event.Handler            = LockActionEventHandler;
        sAppTask.PostEvent(&button_event);
    }

    if (FUNCTION_BUTTON_MASK & has_changed)
    {
        button_event.ButtonEvent.PinNo  = FUNCTION_BUTTON;
        button_event.ButtonEvent.Action = (FUNCTION_BUTTON_MASK & button_state) ? BUTTON_PUSH_EVENT : BUTTON_RELEASE_EVENT;
        button_event.Handler            = FunctionHandler;
        sAppTask.PostEvent(&button_event);
    }

    if (THREAD_START_BUTTON_MASK & button_state & has_changed)
    {
        button_event.ButtonEvent.PinNo  = THREAD_START_BUTTON;
        button_event.ButtonEvent.Action = BUTTON_PUSH_EVENT;
        button_event.Handler            = StartThreadHandler;
        sAppTask.PostEvent(&button_event);
    }

    if (BLE_ADVERTISEMENT_START_BUTTON_MASK & button_state & has_changed)
    {
        button_event.ButtonEvent.PinNo  = BLE_ADVERTISEMENT_START_BUTTON;
        button_event.ButtonEvent.Action = BUTTON_PUSH_EVENT;
        button_event.Handler            = StartBLEAdvertisementHandler;
        sAppTask.PostEvent(&button_event);
    }
}

void AppTask::TimerEventHandler(k_timer * timer)
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = k_timer_user_data_get(timer);
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;

    // If we reached here, the button was held past FACTORY_RESET_TRIGGER_TIMEOUT, initiate factory reset
    if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
    {
        LOG_INF("Factory Reset Triggered. Release button within %ums to cancel.", FACTORY_RESET_TRIGGER_TIMEOUT);

        // Start timer for FACTORY_RESET_CANCEL_WINDOW_TIMEOUT to allow user to cancel, if required.
        sAppTask.StartTimer(FACTORY_RESET_CANCEL_WINDOW_TIMEOUT);
        sAppTask.mFunction = kFunction_FactoryReset;

        // Turn off all LEDs before starting blink to make sure blink is co-ordinated.
        sStatusLED.Set(false);
        sLockLED.Set(false);
        sUnusedLED_1.Set(false);
        sUnusedLED.Set(false);

        sStatusLED.Blink(500);
        sLockLED.Blink(500);
        sUnusedLED.Blink(500);
        sUnusedLED_1.Blink(500);
    }
    else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
    {
        // Actually trigger Factory Reset
        sAppTask.mFunction = kFunction_NoneSelected;
        ConfigurationMgr().InitiateFactoryReset();
    }
}

void AppTask::FunctionHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != FUNCTION_BUTTON)
        return;

    // To trigger software update: press the FUNCTION_BUTTON button briefly (< FACTORY_RESET_TRIGGER_TIMEOUT)
    // To initiate factory reset: press the FUNCTION_BUTTON for FACTORY_RESET_TRIGGER_TIMEOUT + FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    // All LEDs start blinking after FACTORY_RESET_TRIGGER_TIMEOUT to signal factory reset has been initiated.
    // To cancel factory reset: release the FUNCTION_BUTTON once all LEDs start blinking within the
    // FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    if (aEvent->ButtonEvent.Action == BUTTON_PUSH_EVENT)
    {
        if (!sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_NoneSelected)
        {
            sAppTask.StartTimer(FACTORY_RESET_TRIGGER_TIMEOUT);

            sAppTask.mFunction = kFunction_SoftwareUpdate;
        }
    }
    else
    {
        // If the button was released before factory reset got initiated, trigger a software update.
        if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_SoftwareUpdate)
        {
            sAppTask.CancelTimer();
            sAppTask.mFunction = kFunction_NoneSelected;
            LOG_INF("Software update is not implemented");
        }
        else if (sAppTask.mFunctionTimerActive && sAppTask.mFunction == kFunction_FactoryReset)
        {
            sUnusedLED.Set(false);
            sUnusedLED_1.Set(false);

            // Set lock status LED back to show state of lock.
            sLockLED.Set(!PumpMgr().IsUnlocked());

            sAppTask.CancelTimer();

            // Change the function to none selected since factory reset has been canceled.
            sAppTask.mFunction = kFunction_NoneSelected;

            LOG_INF("Factory Reset has been Canceled");
        }
    }
}

void AppTask::StartThreadHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != THREAD_START_BUTTON)
        return;

    if (AddTestPairing() != CHIP_NO_ERROR)
    {
        LOG_ERR("Failed to add test pairing");
    }

    if (!chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        StartDefaultThreadNetwork();
        LOG_INF("Device is not commissioned to a Thread network. Starting with the default configuration.");
    }
    else
    {
        LOG_INF("Device is commissioned to a Thread network.");
    }
}

void AppTask::StartBLEAdvertisementHandler(AppEvent * aEvent)
{
    if (aEvent->ButtonEvent.PinNo != BLE_ADVERTISEMENT_START_BUTTON)
        return;

    if (chip::DeviceLayer::ConnectivityMgr().IsThreadProvisioned())
    {
        LOG_INF("NFC Tag emulation and BLE advertisement not started - device is commissioned to a Thread network.");
        return;
    }

    if (!sNFC.IsTagEmulationStarted())
    {
        if (!(GetAppTask().StartNFCTag() < 0))
        {
            LOG_INF("Started NFC Tag emulation");
        }
        else
        {
            LOG_ERR("Starting NFC Tag failed");
        }
    }
    else
    {
        LOG_INF("NFC Tag emulation is already started");
    }

    if (ConnectivityMgr().IsBLEAdvertisingEnabled())
    {
        LOG_INF("BLE Advertisement is already enabled");
        return;
    }

    if (OpenDefaultPairingWindow(chip::ResetAdmins::kNo) == CHIP_NO_ERROR)
    {
        LOG_INF("Enabled BLE Advertisement");
    }
    else
    {
        LOG_ERR("OpenDefaultPairingWindow() failed");
    }
}

#ifdef CONFIG_CHIP_NFC_COMMISSIONING
void AppTask::ThreadProvisioningHandler(const ChipDeviceEvent * event, intptr_t arg)
{
    ARG_UNUSED(arg);
    if ((event->Type == DeviceEventType::kServiceProvisioningChange) && ConnectivityMgr().IsThreadProvisioned())
    {
        const int result = sNFC.StopTagEmulation();
        if (result)
        {
            LOG_ERR("Stopping NFC Tag emulation failed");
        }
    }
}
#endif

void AppTask::CancelTimer()
{
    k_timer_stop(&sFunctionTimer);
    mFunctionTimerActive = false;
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    k_timer_start(&sFunctionTimer, K_MSEC(aTimeoutInMs), K_NO_WAIT);
    mFunctionTimerActive = true;
}

#ifdef CONFIG_CHIP_NFC_COMMISSIONING
int AppTask::StartNFCTag()
{
    // Get QR Code and emulate its content using NFC tag
    std::string QRCode;

    int result = GetQRCode(QRCode, chip::RendezvousInformationFlags::kBLE);
    VerifyOrExit(!result, ChipLogError(AppServer, "Getting QR code payload failed"));

    // TODO: Issue #4504 - Remove replacing spaces with _ after problem described in #415 will be fixed.
    std::replace(QRCode.begin(), QRCode.end(), ' ', '_');

    result = sNFC.StartTagEmulation(QRCode.c_str(), QRCode.size());
    VerifyOrExit(result >= 0, ChipLogError(AppServer, "Starting NFC Tag emulation failed"));

exit:
    return result;
}
#endif

void AppTask::ActionInitiated(PumpManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == PumpManager::LOCK_ACTION)
    {
        LOG_INF("Lock Action has been initiated");
    }
    else if (aAction == PumpManager::UNLOCK_ACTION)
    {
        LOG_INF("Unlock Action has been initiated");
    }

    sLockLED.Blink(50, 50);
}

void AppTask::ActionCompleted(PumpManager::Action_t aAction, int32_t aActor)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == PumpManager::LOCK_ACTION)
    {
        LOG_INF("Lock Action has been completed");
        sLockLED.Set(true);
    }
    else if (aAction == PumpManager::UNLOCK_ACTION)
    {
        LOG_INF("Unlock Action has been completed");
        sLockLED.Set(false);
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        sAppTask.UpdateClusterState();
    }
}

void AppTask::PostLockActionRequest(int32_t aActor, PumpManager::Action_t aAction)
{
    AppEvent event;
    event.Type             = AppEvent::kEventType_Lock;
    event.LockEvent.Actor  = aActor;
    event.LockEvent.Action = aAction;
    event.Handler          = LockActionEventHandler;
    PostEvent(&event);
}

void AppTask::PostEvent(AppEvent * aEvent)
{
    if (k_msgq_put(&sAppEventQueue, aEvent, K_NO_WAIT))
    {
        LOG_INF("Failed to post event to app task event queue");
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
        LOG_INF("Event received with no handler. Dropping event.");
    }
}

void AppTask::UpdateClusterState()
{
    uint8_t newValue = !PumpMgr().IsUnlocked();

    // write the new on/off value
    EmberAfStatus status = emberAfWriteAttribute(1, ZCL_ON_OFF_CLUSTER_ID, ZCL_ON_OFF_ATTRIBUTE_ID, CLUSTER_MASK_SERVER, &newValue,
                                                 ZCL_BOOLEAN_ATTRIBUTE_TYPE);
    if (status != EMBER_ZCL_STATUS_SUCCESS)
    {
        LOG_ERR("Updating on/off %x", status);
    }
}
