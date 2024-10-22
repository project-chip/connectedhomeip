/*
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

#include "LowPower.h"
#include "fsl_pm_core.h"

#include <lib/support/CodeUtils.h>

#define APP_ERROR_PM_REGISTER_LP_CALLBACK_FAILED CHIP_APPLICATION_ERROR(0x01)

static status_t LowPowerCallback(pm_event_type_t type, uint8_t state, void * data);

static pm_notify_element_t appNotifyElement = {
    .notifyCallback = LowPowerCallback,
    .data           = NULL,
};

static status_t LowPowerCallback(pm_event_type_t type, uint8_t state, void * data)
{
    // This is just an example. It does nothing by default.
    return kStatus_Success;
}

namespace chip::NXP::App::LowPower {

CHIP_ERROR Init()
{
    status_t status = PM_RegisterNotify(kPM_NotifyGroup2, &appNotifyElement);
    VerifyOrReturnError(status == kStatus_Success, APP_ERROR_PM_REGISTER_LP_CALLBACK_FAILED);

    return CHIP_NO_ERROR;
}

} // namespace chip::NXP::App::LowPower
