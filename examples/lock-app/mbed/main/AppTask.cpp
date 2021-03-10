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

#include <support/logging/CHIPLogging.h>

#include "mbed.h"

static mbed::Ticker sFunctionTimer;

// TODO: change EventQueue default event size
static EventQueue sAppEventQueue;

AppTask AppTask::sAppTask;

int AppTask::Init()
{
    mFunctionTimerActive = false;

    // Timer initialization
    // TODO: timer period to FACTORY_RESET_CANCEL_WINDOW_TIMEOUT
    StartTimer(50);

    return 0;
}

int AppTask::StartApp()
{
    int ret = Init();

    if (ret)
    {
        ChipLogError(NotSpecified, "AppTask.Init() failed");
        return ret;
    }

    while (true)
    {
        sAppEventQueue.dispatch(100);
    }

    return 0;
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
    // Below code is only for debug purpose!
    static DigitalOut led(LED1);
    if (aEvent->Type != AppEvent::kEventType_Timer)
        return;

    led = !led;
}
