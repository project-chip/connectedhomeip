// /*
//  *
//  *    Copyright (c) 2021 Project CHIP Authors
//  *    All rights reserved.
//  *
//  *    Licensed under the Apache License, Version 2.0 (the "License");
//  *    you may not use this file except in compliance with the License.
//  *    You may obtain a copy of the License at
//  *
//  *        http://www.apache.org/licenses/LICENSE-2.0
//  *
//  *    Unless required by applicable law or agreed to in writing, software
//  *    distributed under the License is distributed on an "AS IS" BASIS,
//  *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  *    See the License for the specific language governing permissions and
//  *    limitations under the License.
//  */
// #pragma once

// #include <stdbool.h>
// #include <stdint.h>

// #include <functional>

// #include "AppEvent.h"

// #include "FreeRTOS.h"
// #include "timers.h" // provides FreeRTOS timer support

// #include <lib/core/CHIPError.h>

// #include <app-common/zap-generated/attributes/Accessors.h>
// #include <app-common/zap-generated/cluster-objects.h>
// #include <app/clusters/switch-server/switch-server.h>
// #include <app/data-model/DecodableList.h>
// #include <app/util/im-client-callbacks.h>

// class SwitchManager
// {
// public:
//     void Init(void);
//     static void GenericSwitchInitialPress(void);
//     static void GenericSwitchReleasePress(void);
//     static void ToggleHandler(AppEvent * aEvent);

// private:
//     friend SwitchManager & SwitchMgr(void);
//     static SwitchManager sSwitch;
// };

// inline SwitchManager & SwitchMgr(void)
// {
//     return SwitchManager::sSwitch;
// }

/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

#include <app/util/basic-types.h>
#include <lib/core/CHIPError.h>

using namespace ::chip;

#if CONFIG_DEFAULT_ZAP
#define MAX_SUPPORTED_LIGHT_NUM 1
constexpr EndpointId kLightSwitchForGroupEndpointId                  = 1;
constexpr EndpointId kLightGenericSwitchEndpointId                   = 2;
constexpr EndpointId kLightSwitchEndpointId[MAX_SUPPORTED_LIGHT_NUM] = { 1 };
#elif CONFIG_1_TO_2_ZAP
#define MAX_SUPPORTED_LIGHT_NUM 2
constexpr EndpointId kLightSwitchForGroupEndpointId                  = 1;
constexpr EndpointId kLightGenericSwitchEndpointId                   = 4;
constexpr EndpointId kLightSwitchEndpointId[MAX_SUPPORTED_LIGHT_NUM] = { 2, 3 };
#elif CONFIG_1_TO_8_ZAP
#define MAX_SUPPORTED_LIGHT_NUM 8
constexpr EndpointId kLightSwitchForGroupEndpointId                  = 1;
constexpr EndpointId kLightGenericSwitchEndpointId                   = 10;
constexpr EndpointId kLightSwitchEndpointId[MAX_SUPPORTED_LIGHT_NUM] = { 2, 3, 4, 5, 6, 7, 8, 9 };
#elif CONFIG_1_TO_11_ZAP
#define MAX_SUPPORTED_LIGHT_NUM 11
constexpr EndpointId kLightSwitchForGroupEndpointId                  = 1;
constexpr EndpointId kLightGenericSwitchEndpointId                   = 10;
constexpr EndpointId kLightSwitchEndpointId[MAX_SUPPORTED_LIGHT_NUM] = { 2, 3, 4, 5, 6, 7, 8, 9, 11, 12, 13 };
#endif

enum Action : uint8_t
{
    Toggle, /// Switch state on lighting-app device
    On,     /// Turn on light on lighting-app device
    Off     /// Turn off light on lighting-app device
};

class LightSwitch
{
public:
    void Init();
    void InitiateActionSwitch(chip::EndpointId endpointId, uint8_t action);
    void GenericSwitchInitialPress();
    void GenericSwitchReleasePress();

#if CONFIG_ENABLE_ATTRIBUTE_SUBSCRIBE
    void SubscribeRequestForOneNode(chip::EndpointId endpointId);
    void ShutdownSubscribeRequestForOneNode(chip::EndpointId endpointId);
#endif

    static LightSwitch & GetInstance()
    {
        static LightSwitch sLightSwitch;
        return sLightSwitch;
    }

private:
};
