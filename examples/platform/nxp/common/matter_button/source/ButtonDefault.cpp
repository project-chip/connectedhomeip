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

#include "ButtonDefault.h"
#include "AppTaskBase.h"
#include "board_comp.h"

BUTTON_HANDLE_DEFINE(sHandle);

CHIP_ERROR chip::NXP::App::ButtonDefault::Init()
{
    handle = sHandle;

    auto status = BOARD_InitButton(handle);
    VerifyOrReturnError(status == kStatus_BUTTON_Success, CHIP_ERROR_UNEXPECTED_EVENT);

    return CHIP_NO_ERROR;
}

void chip::NXP::App::ButtonDefault::HandleShortPress()
{
    chip::NXP::App::GetAppTask().SwitchCommissioningStateHandler();
}

void chip::NXP::App::ButtonDefault::HandleLongPress()
{
    chip::NXP::App::GetAppTask().FactoryResetHandler();
}

void chip::NXP::App::ButtonDefault::HandleDoubleClick()
{
    /* Currently not mapped to any action */
}
