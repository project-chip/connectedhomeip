/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
#include "BoltLockManager.h"
#include "LEDWidget.h"

#include <platform/CHIPDeviceLayer.h>
#include <support/logging/CHIPLogging.h>

// mbed-os headers
#include "drivers/Ticker.h"
#include "events/EventQueue.h"

constexpr uint32_t kPublishServicePeriodUs = 5000000;

static LEDWidget sStatusLED(MBED_CONF_APP_SYSTEM_STATE_LED);
static LEDWidget sLockLED(MBED_CONF_APP_LOCK_STATE_LED);

static bool sIsThreadProvisioned     = false;
static bool sIsThreadEnabled         = false;
static bool sIsThreadAttached        = false;
static bool sIsPairedToAccount       = false;
static bool sHaveBLEConnections      = false;
static bool sHaveServiceConnectivity = false;

static mbed::Ticker sFunctionTimer;

// TODO: change EventQueue default event size
static events::EventQueue sAppEventQueue;

using namespace ::chip::DeviceLayer;

AppTask AppTask::sAppTask;

int AppTask::Init()
{
    mFunctionTimerActive = false;

    // Initialize LEDs
    sLockLED.Set(!BoltLockMgr().IsUnlocked());

    // Timer initialization
    // TODO: timer period to FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    StartTimer(50);

    BoltLockMgr().Init();
    BoltLockMgr().SetCallbacks(ActionInitiated, ActionCompleted);

    return 0;
}

int AppTask::StartApp()
{
    int ret                            = Init();
    uint64_t mLastPublishServiceTimeUS = 0;

    if (ret)
    {
        ChipLogError(NotSpecified, "AppTask.Init() failed");
        return ret;
    }

    while (true)
    {
        sAppEventQueue.dispatch(100);

        // Collect connectivity and configuration state from the CHIP stack.  Because the
        // CHIP event loop is being run in a separate task, the stack must be locked
        // while these values are queried.  However we use a non-blocking lock request
        // (TryLockChipStack()) to avoid blocking other UI activities when the CHIP
        // task is busy (e.g. with a long crypto operation).

        if (PlatformMgr().TryLockChipStack())
        {
            sIsThreadProvisioned     = ConnectivityMgr().IsThreadProvisioned();
            sIsThreadEnabled         = ConnectivityMgr().IsThreadEnabled();
            sIsThreadAttached        = ConnectivityMgr().IsThreadAttached();
            sHaveBLEConnections      = (ConnectivityMgr().NumBLEConnections() != 0);
            sHaveServiceConnectivity = ConnectivityMgr().HaveServiceConnectivity();
            PlatformMgr().UnlockChipStack();
        }

        // Consider the system to be "fully connected" if it has service
        // connectivity and it is able to interact with the service on a regular basis.
        bool isFullyConnected = sHaveServiceConnectivity;

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
            if (isFullyConnected)
            {
                sStatusLED.Set(true);
            }
            else if (sIsThreadProvisioned && sIsThreadEnabled && sIsPairedToAccount && (!sIsThreadAttached || !isFullyConnected))
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

        uint64_t nowUS            = chip::System::Platform::Layer::GetClock_Monotonic();
        uint64_t nextChangeTimeUS = mLastPublishServiceTimeUS + kPublishServicePeriodUs;

        if (nowUS > nextChangeTimeUS)
        {
            // TODO:
            // PublishService();
            mLastPublishServiceTimeUS = nowUS;
        }
    }
}

void AppTask::LockActionEventHandler(AppEvent * aEvent)
{
    BoltLockManager::Action_t action = BoltLockManager::INVALID_ACTION;
    int32_t actor                    = 0;

    if (aEvent->Type == AppEvent::kEventType_Lock)
    {
        action = static_cast<BoltLockManager::Action_t>(aEvent->LockEvent.Action);
        actor  = aEvent->LockEvent.Actor;
    }
    else if (aEvent->Type == AppEvent::kEventType_Button)
    {
        action = BoltLockMgr().IsUnlocked() ? BoltLockManager::LOCK_ACTION : BoltLockManager::UNLOCK_ACTION;
        actor  = AppEvent::kEventType_Button;
    }

    if (action != BoltLockManager::INVALID_ACTION && !BoltLockMgr().InitiateAction(actor, action))
        ChipLogProgress(NotSpecified, "Action is already in progress or active.");
}

void AppTask::ActionInitiated(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // If the action has been initiated by the lock, update the bolt lock trait
    // and start flashing the LEDs rapidly to indicate action initiation.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Lock Action has been initiated");
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Unlock Action has been initiated");
    }

    sLockLED.Blink(50, 50);
}

void AppTask::ActionCompleted(BoltLockManager::Action_t aAction, int32_t aActor)
{
    // if the action has been completed by the lock, update the bolt lock trait.
    // Turn on the lock LED if in a LOCKED state OR
    // Turn off the lock LED if in an UNLOCKED state.
    if (aAction == BoltLockManager::LOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Lock Action has been completed");
        sLockLED.Set(true);
    }
    else if (aAction == BoltLockManager::UNLOCK_ACTION)
    {
        ChipLogProgress(NotSpecified, "Unlock Action has been completed");
        sLockLED.Set(false);
    }

    if (aActor == AppEvent::kEventType_Button)
    {
        // TODO
        // sAppTask.UpdateClusterState();
    }
}

void AppTask::CancelTimer()
{
    if (mFunctionTimerActive)
    {
        sFunctionTimer.detach();
        mFunctionTimerActive = false;
    }
}

void AppTask::StartTimer(uint32_t aTimeoutInMs)
{
    auto chronoTimeoutMs = std::chrono::duration<uint32_t, std::milli>(aTimeoutInMs);

    if (mFunctionTimerActive)
    {
        ChipLogError(NotSpecified, "App timer already started!");
        CancelTimer();
    }

    sFunctionTimer.attach(&AppTask::TimerEventHandler, chronoTimeoutMs);
    mFunctionTimerActive = true;
}

void AppTask::PostEvent(AppEvent * aEvent)
{
    auto handle = sAppEventQueue.call([event = *aEvent, this] { DispatchEvent(&event); });
    if (!handle)
    {
        ChipLogError(NotSpecified, "Failed to post event to app task event queue: Not enough memory");
    }
}

void AppTask::DispatchEvent(const AppEvent * aEvent)
{
    if (aEvent->Handler)
    {
        aEvent->Handler(const_cast<AppEvent *>(aEvent));
    }
    else
    {
        ChipLogError(NotSpecified, "Event received with no handler. Dropping event.");
    }
}

// static
void AppTask::TimerEventHandler()
{
    AppEvent event;
    event.Type               = AppEvent::kEventType_Timer;
    event.TimerEvent.Context = nullptr;
    event.Handler            = FunctionTimerEventHandler;
    sAppTask.PostEvent(&event);
}

// static
void AppTask::FunctionTimerEventHandler(AppEvent * aEvent)
{
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;
}
