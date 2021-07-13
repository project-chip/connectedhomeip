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

#include <AppConfig.h>
#include <AppTask.h>
#include <AppTimer.h>

void AppTimer::Init(void * context, const char * tag, uint32_t timeoutInMs, TimerCallback_t callback)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mIsActive = false;
    mHandler  = xTimerCreate(tag,           // Just a text name, not used by the RTOS kernel
                            timeoutInMs,   // == default timer period (mS)
                            false,         // no timer reload (==one-shot)
                            (void *) this, // init timer id = app task obj context
                            TimerCallback  // timer callback handler
    );
    if (mHandler == NULL)
    {
        EFR32_LOG("Timer create failed");
        appError(err);
    }
    mContext  = context;
    mCallback = callback;
}

// [static]
void * AppTimer::Context(TimerHandle_t handle)
{
    return pvTimerGetTimerID(handle);
}

void AppTimer::Start()
{
    if (xTimerIsTimerActive(mHandler))
    {
        EFR32_LOG("AppTimer already started!");
        Stop();
    }

    // Timer is not active
    if (xTimerStart(mHandler, 100) != pdPASS)
    {
        EFR32_LOG("AppTimer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mIsActive = true;
}

void AppTimer::Start(uint32_t timeoutInMs)
{
    if (xTimerIsTimerActive(mHandler))
    {
        EFR32_LOG("AppTimer already started!");
        Stop();
    }

    // Timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(mHandler, timeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        EFR32_LOG("AppTimer start() failed");
        appError(APP_ERROR_START_TIMER_FAILED);
    }

    mIsActive = true;
}

void AppTimer::IsrStart()
{
    portBASE_TYPE taskWoken = pdFALSE; // For FreeRTOS timer (below).
    // Start/restart the button debounce timer (Note ISR version of FreeRTOS
    // api call here).
    xTimerStartFromISR(mHandler, &taskWoken);
    if (taskWoken != pdFALSE)
    {
        taskYIELD();
    }
    mIsActive = true;
}

void AppTimer::Stop()
{
    mIsActive = false;
    if (xTimerStop(mHandler, 0) == pdFAIL)
    {
        EFR32_LOG("AppTimer stop() failed");
        appError(APP_ERROR_STOP_TIMER_FAILED);
    }
}

bool AppTimer::IsActive()
{
    return mIsActive;
}

void AppTimer::TimerCallback(TimerHandle_t xTimer)
{
    AppTimer * timer = (AppTimer *) pvTimerGetTimerID(xTimer);
    if (timer)
    {
        timer->mIsActive = false;
        if (timer->mCallback)
        {
            timer->mCallback(*timer, timer->mContext);
        }
    }
}
