/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
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

#include "AppConfig.h"
#include "LedDimmer.h"
#include "LedWidgetInterface.h"
#include "UserInterfaceFeedback.h"

#include <app/util/attribute-storage.h>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::NXP::App;

/* These flags can be overwritten in AppConfig.h */
/**
 * @brief Specifies the endpoint on which Application Device Type cluster is present.
 *
 * For example, this should be 1 for Contact Sensor and Lighting App reference apps.
 */
#ifndef LIGHTING_MANAGER_APP_DEVICE_TYPE_ENDPOINT
#define LIGHTING_MANAGER_APP_DEVICE_TYPE_ENDPOINT 1
#endif

/**
 * @brief Specifies the array index of the light LED.
 *
 * The light LED is usually used to indicate the state of some cluster
 * attribute: e.g OnOff attribute from OnOff cluster.
 */
#ifndef LIGHTING_MANAGER_LIGHT_LED_INDEX
#define LIGHTING_MANAGER_LIGHT_LED_INDEX 1
#endif

/**
 * @brief Enable dimmable LED instead of a simple on/off LED.
 *
 */
#ifndef LIGHTING_MANAGER_ENABLE_DIMMABLE_LED
#define LIGHTING_MANAGER_ENABLE_DIMMABLE_LED 1
#endif

/**
 * @brief Manager of LedDimmer instance.
 *
 * It implements the UserInterfaceFeedback abstract interface.
 */
class LightingManagerDimmable : public UserInterfaceFeedback
{
public:
    void Init() override;
    void DisplayInLoop() override;
    void DisplayOnAction(Action action) override;
    void RestoreState() override;
    void UpdateState() override;

private:
    void ApplyDim(uint8_t value);

    LedDimmer lightLed;

    friend LightingManagerDimmable & LightingMgr(void);
    static LightingManagerDimmable sLightingManager;
};

inline LightingManagerDimmable & LightingMgr(void)
{
    return LightingManagerDimmable::sLightingManager;
}
