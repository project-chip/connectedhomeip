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

#include "ButtonManager.h"
#include "ButtonRegistration.h"

#include "ButtonApp.h"
#include "ButtonBle.h"

#include <lib/support/CodeUtils.h>

/**
 * @brief Flag to configure if the app button is enabled.
 *
 * Enabled by default.
 */
#ifndef CONFIG_APP_BUTTON_ENABLED
#define CONFIG_APP_BUTTON_ENABLED 1
#endif

/**
 * @brief Flag to configure if the BLE button is enabled.
 *
 * Enabled by default.
 */
#ifndef CONFIG_BLE_BUTTON_ENABLED
#define CONFIG_BLE_BUTTON_ENABLED 1
#endif

CHIP_ERROR chip::NXP::App::RegisterButtons(void)
{
    ReturnErrorOnFailure(ButtonMgr().Init());

#if CONFIG_BLE_BUTTON_ENABLED
    static chip::NXP::App::ButtonBle sBleButton;
    ReturnErrorOnFailure(ButtonMgr().RegisterButton(sBleButton));
#endif

#if CONFIG_APP_BUTTON_ENABLED
    static chip::NXP::App::ButtonApp sAppButton;
    ReturnErrorOnFailure(ButtonMgr().RegisterButton(sAppButton));
#endif

    return CHIP_NO_ERROR;
}
