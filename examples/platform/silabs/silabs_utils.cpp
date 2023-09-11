/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2022 Silabs
 *
 * SPDX-License-Identifier: Apache-2.0
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
