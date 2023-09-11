/*
 *
 * SPDX-FileCopyrightText: 2021 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_None = 0,
        kEventType_Button,
        kEventType_Timer,
        kEventType_TurnOn,
        kEventType_Install,
        kEventType_OTAResume,
    };

    AppEventTypes Type;

    union
    {
        struct
        {
            uint8_t PinNo;
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
        } LightEvent;
    };

    EventHandler Handler;
};
