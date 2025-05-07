/*
 *
 *    Copyright (c) 2025 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <cstdint>
#ifdef DISPLAY_ENABLED
#include "lcd.h"
#endif

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

struct BaseAppEvent
{
    enum AppEventTypes
    {
        kEventType_Button = 0,
        kEventType_LCD,
        kEventType_Timer,
        kEventType_Max,
    };

    uint16_t Type;

    union
    {
        struct
        {
            uint8_t Action;
        } ButtonEvent;
#ifdef DISPLAY_ENABLED
        struct
        {
            SilabsLCD::Screen_e screen;
        } LCDEvent;
#endif
        struct
        {
            void * Context;
        } TimerEvent;
    };

    EventHandler Handler;
};
