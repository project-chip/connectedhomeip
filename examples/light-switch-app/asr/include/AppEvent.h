/*
 *
 * SPDX-FileCopyrightText: 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once
#include <cstdint>

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

struct AppEvent
{
    constexpr static uint8_t kButtonPushEvent    = 1;
    constexpr static uint8_t kButtonReleaseEvent = 0;

    enum AppEventTypes
    {
        kEventType_StartBLEAdvertising,
        kEventType_Button,
        kEventType_Timer,
        kEventType_UpdateLedState,
        kEventType_IdentifyStart,
        kEventType_IdentifyStop,
        kEventType_Light,
        kEventType_Install,
    };

    uint16_t Type;

    union
    {
        struct
        {
            uint8_t PinNo;
            uint8_t Action;
        } ButtonEvent;
        struct
        {
            uint8_t TimerType;
            void * Context;
        } TimerEvent;
        struct
        {
            uint8_t Action;
            int32_t Actor;
        } LightEvent;
    };

    EventHandler Handler;
};
