/*
 *
 * SPDX-FileCopyrightText: 2020 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

struct AppEvent;
typedef void (*EventHandler)(void *);

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_Button = 0,
        kEventType_Timer,
        kEventType_Lock,
        kEventType_Install,
#if defined(chip_with_low_power) && (chip_with_low_power == 1)
        kEventType_Lp,
#endif
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
        } LockEvent;
    };

    EventHandler Handler;

#if defined(chip_with_low_power) && (chip_with_low_power == 1)
    void * param;
#endif
};
