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
#include "fwk_platform.h"

#include <lib/core/CHIPError.h>
#include <lib/support/CodeUtils.h>

chip::NXP::App::ButtonManager chip::NXP::App::ButtonManager::sInstance;

CHIP_ERROR chip::NXP::App::ButtonManager::Init()
{
    VerifyOrReturnError(PLATFORM_InitTimerManager() >= 0, CHIP_ERROR_UNEXPECTED_EVENT);

    return CHIP_NO_ERROR;
}

CHIP_ERROR chip::NXP::App::ButtonManager::RegisterButton(Button & button)
{
    ReturnErrorOnFailure(button.Init());
    VerifyOrReturnError(button.handle != nullptr, CHIP_ERROR_INVALID_ARGUMENT);

    auto status = BUTTON_InstallCallback(button.handle, ButtonManager::HandleCallbacks, &button);
    VerifyOrReturnError(status == kStatus_BUTTON_Success, CHIP_ERROR_UNEXPECTED_EVENT);

    return CHIP_NO_ERROR;
}

button_status_t chip::NXP::App::ButtonManager::HandleCallbacks(void * buttonHandle, button_callback_message_t * message,
                                                               void * callbackParam)
{
    auto * button = static_cast<Button *>(callbackParam);

    switch (message->event)
    {
    case kBUTTON_EventOneClick:
    case kBUTTON_EventShortPress:
        button->HandleShortPress();
        break;
    case kBUTTON_EventLongPress:
        button->HandleLongPress();
        break;
    case kBUTTON_EventDoubleClick:
        button->HandleDoubleClick();
        break;
    default:
        break;
    }

    return kStatus_BUTTON_Success;
}
