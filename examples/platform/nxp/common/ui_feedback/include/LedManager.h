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

/**
 * @brief Specifies the array index of the status LED.
 *
 * The status LED is usually used to indicate additional information
 * related to the connectivity status of the application.
 */
#ifndef LED_MANAGER_STATUS_LED_INDEX
#define LED_MANAGER_STATUS_LED_INDEX 0
#endif

/**
 * @brief Specifies the array index of the light LED.
 *
 * The light LED is usually used to indicate the state of some cluster
 * attribute: e.g OnOff attribute from OnOff cluster.
 */
#ifndef LED_MANAGER_LIGHT_LED_INDEX
#define LED_MANAGER_LIGHT_LED_INDEX 1
#endif

/**
 * @brief Enable status LED.
 *
 */
#ifndef LED_MANAGER_ENABLE_STATUS_LED
#define LED_MANAGER_ENABLE_STATUS_LED 0
#endif

/**
 * @brief Manager of LedWidgetInterface concrete classes.
 *
 * It implements the UserInterfaceFeedback abstract interface.
 */
class LedManager : public UserInterfaceFeedback
{
public:
    void Init() override;
    void DisplayInLoop() override;
    void DisplayOnAction(Action action) override;
    void RestoreState() override;

private:
    void ApplyTurnOn();
    void ApplyTurnOff();
    void AnimateOnAction(uint32_t onTimeMS, uint32_t offTimeMS);

#if LED_MANAGER_ENABLE_STATUS_LED
    void UpdateStatus();

    LedOnOff statusLed;
#endif
    LedOnOff lightLed;

    friend LedManager & LightingMgr(void);
    static LedManager sLedManager;
};

inline LedManager & LightingMgr(void)
{
    return LedManager::sLedManager;
}
