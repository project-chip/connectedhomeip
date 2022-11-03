/*
 * SPDX-FileCopyrightText: (c) 2022 Project CHIP Authors
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

class LEDWidget;

struct AppEvent
{
    using EventHandler = void (*)(AppEvent *);

    enum class Type : uint8_t
    {
        None,
        Button,
        Timer,
        UpdateLedState,
    };

    Type Type{ Type::None };

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
            LEDWidget * LedWidget;
        } UpdateLedStateEvent;
    };

    EventHandler Handler;
};
