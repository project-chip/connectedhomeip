/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2022 Silabs.
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

#include "silabs_utils.h"
#include "SoftwareFaultReports.h"
#include <MatterConfig.h>

void appError(int err)
{
    char faultMessage[30] = { 0 }; // App Critical Error: + 10 digit + null character
    snprintf(faultMessage, sizeof faultMessage, "App Critical Error:%d", err);
    SILABS_LOG("!!!!!!!!!!!! %s !!!!!!!!!!!", faultMessage);
    chip::DeviceLayer::Silabs::OnSoftwareFaultEventHandler(faultMessage);
    vTaskDelay(pdMS_TO_TICKS(1000));
    /* Force an assert. */
    chipAbort();
}

void appError(CHIP_ERROR error)
{
    appError(static_cast<int>(error.AsInteger()));
}
