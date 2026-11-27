/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#pragma once

#include <cstdint>

struct AppEvent;
using EventHandler = void (*)(const AppEvent &);

enum class AppEventType : uint8_t
{
    None = 0,
    Button,
    ButtonPushed,
    ButtonReleased,
    Timer,
    IdentifyStart,
    IdentifyStop,
};

enum class FunctionEvent : uint8_t
{
    NoneSelected = 0,
    Reset,            // released before kResetTriggerTimeout
    AdvertisingStart, // held past kResetTriggerTimeout
    FactoryReset,     // held past kBleAdvTriggerTimeout, cancelable for a window
};

struct AppEvent
{
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

    AppEventType Type{ AppEventType::None };
    EventHandler Handler;
};
