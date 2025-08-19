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

#include "fsl_component_button.h"

#include <lib/core/CHIPError.h>

namespace chip::NXP::App {

/**
 * @brief This class is an abstraction over an SDK button.
 *
 */
class Button
{
public:
    using Callback = button_status_t (*)(void * handle, button_callback_message_t * message, void * param);

    virtual ~Button() = default;

    virtual CHIP_ERROR Init()        = 0;
    virtual void HandleShortPress()  = 0;
    virtual void HandleLongPress()   = 0;
    virtual void HandleDoubleClick() = 0;

    /**
     * @brief This is an SDK handle for a button.
     *
     * It should be set in the Init method, based on a newly defined
     * handle or an already defined handle owned by the SDK.
     */
    button_handle_t handle = nullptr;
};

} // namespace chip::NXP::App
