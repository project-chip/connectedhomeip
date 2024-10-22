/*
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2021 Nest Labs, Inc.
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

struct AppEvent
{
    enum AppEventTypes
    {
        kEventType_Timer = 0,
        kEventType_TurnOn,
        kEventType_Install,
    };

    uint16_t Type;

    union
    {
        struct
        {
            void * Context;
        } TimerEvent;
        struct
        {
            uint8_t Action;
            int32_t Actor;
        } ClusterEvent;
    };

    EventHandler Handler;
    void * extra;
};
