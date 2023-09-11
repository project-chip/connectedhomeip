/*
 *
 * SPDX-FileCopyrightText: 2023 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

class LEDWidget;

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_Button = 0,
        kEventType_Timer,
        kEventType_UpdateLedState,
        kEventType_IdentifyStart,
        kEventType_IdentifyStop,
        kEventType_Lighting,
    };

    uint16_t Type;

    union
    {
        struct
        {
            uint8_t Action;
        } ButtonEvent;
        struct
        {
            void * Context;
        } TimerEvent;
        struct
        {
            uint8_t Action;
            int32_t Actor;
        } LightingEvent;
        struct
        {
            LEDWidget * LedWidget;
        } UpdateLedStateEvent;
    };

    EventHandler Handler;
};
