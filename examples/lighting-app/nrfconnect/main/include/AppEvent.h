/*
 *
 * SPDX-FileCopyrightText: 2018 Nest Labs, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <cstdint>

#include "EventTypes.h"

class LEDWidget;

enum class AppEventType : uint8_t
{
    None = 0,
    Button,
    ButtonPushed,
    ButtonReleased,
    Timer,
    UpdateLedState,
    Lighting,
    IdentifyStart,
    IdentifyStop,
};

enum class FunctionEvent : uint8_t
{
    NoneSelected   = 0,
    SoftwareUpdate = 0,
    FactoryReset,
    AdvertisingStart
};

struct AppEvent
{
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
        } LightingEvent;
        struct
        {
            LEDWidget * LedWidget;
        } UpdateLedStateEvent;
    };

    AppEventType Type{ AppEventType::None };
    EventHandler Handler;
};
