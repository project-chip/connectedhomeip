/*
 *
 * SPDX-FileCopyrightText: 2020 Project CHIP Authors
 * SPDX-FileCopyrightText: 2019 Google LLC.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

/**********************************************************
 * Includes
 *********************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "BaseApplication.h"
#include "FreeRTOS.h"
#include "timers.h" // provides FreeRTOS timer support
#include <ble/BLEEndPoint.h>
#include <lib/core/CHIPError.h>
#include <platform/CHIPDeviceLayer.h>

/**********************************************************
 * AppTask Declaration
 *********************************************************/

class AppTask : public BaseApplication
{

public:
    AppTask() = default;

    static AppTask & GetAppTask() { return sAppTask; }

    /**
     * @brief AppTask task main loop function
     *
     * @param pvParameter FreeRTOS task parameter
     */
    static void AppTaskMain(void * pvParameter);

    CHIP_ERROR StartAppTask();

    static void ButtonEventHandler(uint8_t button, uint8_t btnAction);

private:
    static AppTask sAppTask;

    /**
     * @brief AppTask initialisation function
     *
     * @return CHIP_ERROR
     */
    CHIP_ERROR Init();
};
