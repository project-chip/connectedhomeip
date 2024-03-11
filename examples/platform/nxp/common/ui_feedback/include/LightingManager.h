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
#include "LedOnOff.h"
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
 * @brief Specifies the Application cluster namespace used to set/get a boolean value.
 *
 * Some examples:
 * - chip::app::Clusters::OnOff::Attributes::OnOff - for lighting app, this is the namespace
 *   of the OnOff attribute.
 * - app::Clusters::BooleanState::Attributes::StateValue - for contact sensor app, this is the
 *   namespace of the StateValue attribute.
 *
 * This is used by the LED animation logic to set the LEDs or synchronize the cluster state.
 */
#if defined(MATTER_DM_PLUGIN_ON_OFF)
#ifndef LIGHTING_MANAGER_APP_CLUSTER_ATTRIBUTE
#define LIGHTING_MANAGER_APP_CLUSTER_ATTRIBUTE chip::app::Clusters::OnOff::Attributes::OnOff
#endif
#elif defined(MATTER_DM_PLUGIN_BOOLEAN_STATE)
#ifndef LIGHTING_MANAGER_APP_CLUSTER_ATTRIBUTE
#define LIGHTING_MANAGER_APP_CLUSTER_ATTRIBUTE chip::app::Clusters::BooleanState::Attributes::StateValue
#endif
#endif

/**
 * @brief Specifies the array index of the status LED.
 *
 * The status LED is usually used to indicate additional information
 * related to the connectivity status of the application.
 */
#ifndef LIGHTING_MANAGER_STATUS_LED_INDEX
#define LIGHTING_MANAGER_STATUS_LED_INDEX 0
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
 * @brief Enable status LED.
 *
 */
#ifndef LIGHTING_MANAGER_ENABLE_STATUS_LED
#define LIGHTING_MANAGER_ENABLE_STATUS_LED 0
#endif

/**
 * @brief Manager of LedWidgetInterface concrete classes.
 *
 * It implements the UserInterfaceFeedback abstract interface.
 */
class LightingManager : public UserInterfaceFeedback
{
public:
    void Init() override;
    void DisplayInLoop() override;
    void DisplayOnAction(Action action) override;
    void RestoreState() override;
    void UpdateState() override;

private:
    void ApplyTurnOn();
    void ApplyTurnOff();
    void AnimateOnAction(uint32_t onTimeMS, uint32_t offTimeMS);

#if LIGHTING_MANAGER_ENABLE_STATUS_LED
    void UpdateStatus();

    LedOnOff statusLed;
#endif
    LedOnOff lightLed;

    friend LightingManager & LightingMgr(void);
    static LightingManager sLightingManager;
};

inline LightingManager & LightingMgr(void)
{
    return LightingManager::sLightingManager;
}
