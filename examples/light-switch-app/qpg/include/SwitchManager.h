/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

#include <stdbool.h>
#include <stdint.h>

#include <functional>

#include "AppEvent.h"

#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support

#include <lib/core/CHIPError.h>

#include <app-common/zap-generated/attributes/Accessors.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app/clusters/switch-server/switch-server.h>
#include <app/data-model/DecodableList.h>
#include <app/util/im-client-callbacks.h>

#define SWITCH_ENDPOINT_ID 1
#define GENERICSWITCH_ENDPOINT_ID 2

class SwitchManager
{
public:
    enum Level_t
    {
        MIN_LEVEL = 0x40,
        MAX_LEVEL = 0xfe,
    };

    enum Color_t
    {
        COLOR_AMBER = 1,
        COLOR_GREEN,
    };

    void Init(void);
    static void GenericSwitchInitialPressHandler(AppEvent * aEvent);
    static void GenericSwitchShortReleaseHandler(AppEvent * aEvent);
    static void GenericSwitchLongReleaseHandler(AppEvent * aEvent);
    static void GenericSwitchLongPressHandler(AppEvent * aEvent);
    static void GenericSwitchMultipressCompleteHandler(AppEvent * aEvent);
    static void GenericSwitchMultipressOngoingHandler(AppEvent * aEvent);
    static void ToggleHandler(AppEvent * aEvent);
    static void LevelHandler(AppEvent * aEvent);
    static void ColorHandler(AppEvent * aEvent);

private:
    friend SwitchManager & SwitchMgr(void);

    static SwitchManager sSwitch;
};

inline SwitchManager & SwitchMgr(void)
{
    return SwitchManager::sSwitch;
}
