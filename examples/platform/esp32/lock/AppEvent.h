/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "esp_system.h"

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_Button = 0,
        kEventType_Timer,
        kEventType_Light,
        kEventType_Lock,
        kEventType_Install,
    };

    uint16_t mType;

    union
    {
        struct
        {
            uint8_t mPinNo;
            uint8_t mAction;
        } mButtonEvent;
        struct
        {
            void * mContext;
        } mTimerEvent;
        struct
        {
            uint8_t mAction;
            int32_t mActor;
        } mLockEvent;
    };

    EventHandler mHandler;
};
