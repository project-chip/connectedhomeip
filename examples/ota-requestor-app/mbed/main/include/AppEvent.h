/*
 *
 * SPDX-FileCopyrightText: 2021 Project CHIP Authors
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

struct AppEvent;
typedef void (*EventHandler)(AppEvent *);

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_Button = 0,
        kEventType_Timer
    };

    uint16_t Type;

    union
    {
        struct
        {
            uint8_t button;
            uint8_t action;
        } ButtonEvent;

        struct
        {
            uint8_t index;
        } TimerEvent;
    };

    EventHandler Handler;
};
