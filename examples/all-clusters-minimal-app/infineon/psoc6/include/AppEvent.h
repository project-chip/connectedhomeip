/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 * SPDX-FileCopyrightText: 2021 Cypress Semiconductor Corporation (an Infineon company)
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
        kEventType_Button = 0,
        kEventType_Timer,
        kEventType_Light,
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
    };

    EventHandler Handler;
};
