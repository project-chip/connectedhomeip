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

#include "Button.h"

#include <lib/core/CHIPError.h>

namespace chip::NXP::App {

/**
 * @brief This class describes a manager over Button operations.
 *
 */
class ButtonManager
{
public:
    static ButtonManager sInstance;

    CHIP_ERROR Init();

    /**
     * @brief API for registering a Button object.
     *
     */
    CHIP_ERROR RegisterButton(Button & button);

    /**
     * @brief Generic callback for all registered buttons.
     *
     * Event handling is delegated to Button objects.
     *
     */
    static button_status_t HandleCallbacks(void * buttonHandle, button_callback_message_t * message, void * callbackParam);
};

inline ButtonManager & ButtonMgr()
{
    return ButtonManager::sInstance;
}

} // namespace chip::NXP::App
