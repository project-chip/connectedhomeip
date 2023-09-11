/*
 *
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef APP_EVENT_H
#define APP_EVENT_H

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_Button = 0,
        kEventType_Timer,
        kEventType_Level,
        kEventType_Install,
    };

    uint16_t Type;

    union
    {
        struct
        {
            uint8_t ButtonIdx;
            uint8_t Action;
        } ButtonEvent;
        struct
        {
            void * Context;
        } TimerEvent;
        struct
        {
            uint8_t Action;
            int32_t Color;
        } LightingEvent;
    };

    EventHandler Handler;
};

#endif // APP_EVENT_H
