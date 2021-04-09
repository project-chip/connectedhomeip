/*
 *
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

#pragma once

#include "FreeRTOS.h"
#include <stdint.h>
#include <timers.h> // FreeRTOS timer support

struct AppTimer
{
    typedef void (*TimerCallback_t)(AppTimer & timer, void * context);

    const char * mTag         = "*";
    bool mIsActive            = false;
    void * mContext           = nullptr;
    TimerHandle_t mHandler    = nullptr;
    TimerCallback_t mCallback = nullptr;

    AppTimer() = default;
    void Init(void * context, const char * tag, uint32_t timeoutInMs, TimerCallback_t callback);
    void * Context(TimerHandle_t mHandler);
    void Start();
    void Start(uint32_t timeoutInMs);
    void IsrStart();
    void Stop();
    bool IsActive();

private:
    static void TimerCallback(TimerHandle_t xTimer);
};
