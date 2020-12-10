/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
/*
 * Copyright (c) 2020, Qorvo Inc
 *
 *
 *
 * This software is owned by Qorvo Inc
 * and protected under applicable copyright laws.
 * It is delivered under the terms of the license
 * and is intended and supplied for use solely and
 * exclusively with products manufactured by
 * Qorvo Inc.
 *
 *
 * THIS SOFTWARE IS PROVIDED IN AN "AS IS"
 * CONDITION. NO WARRANTIES, WHETHER EXPRESS,
 * IMPLIED OR STATUTORY, INCLUDING, BUT NOT
 * LIMITED TO, IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 * QORVO INC. SHALL NOT, IN ANY
 * CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
 * INCIDENTAL OR CONSEQUENTIAL DAMAGES,
 * FOR ANY REASON WHATSOEVER.
 *
 * $Change: 154416 $
 * $DateTime: 2020/08/21 09:47:42 $
 *
 */

/** @file "main.cpp"
 *
 * Main application.
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/

// FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// Qorvo CHIP library
#include "qvCHIP.h"

// CHIP includes
#include <platform/CHIPDeviceLayer.h>
#include <support/logging/CHIPLogging.h>

// Application level logic
#include "AppTask.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/

#define APP_NAME "Lock-app"

/*****************************************************************************
 *                    External Function Definitions
 *****************************************************************************/

/*****************************************************************************
 *                    Application Function Definitions
 *****************************************************************************/

int Application_Init(void)
{
    int ret = CHIP_ERROR_MAX;

    /* Launch application task */
    ChipLogProgress(NotSpecified, "============================");
    ChipLogProgress(NotSpecified, "Qorvo " APP_NAME " Launching");
    ChipLogProgress(NotSpecified, "============================");

    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "GetAppTask().Init() failed");
        return -1;
    }

    return 0;
}

int CHIP_Init(void)
{
    int ret = CHIP_ERROR_MAX;

    ChipLogProgress(NotSpecified, "Init CHIP Stack");
    ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().InitChipStack() failed");
        goto exit;
    }

    ChipLogProgress(NotSpecified, "Starting Platform Manager Event Loop");
    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        ChipLogError(NotSpecified, "PlatformMgr().StartEventLoopTask() failed");
        goto exit;
    }

exit:
    return ret;
}

/*****************************************************************************
 * --- Main
 *****************************************************************************/

int main(void)
{
    int result;

    /* Initialize Qorvo stack */
    result = qvCHIP_init();
    if (result < 0)
    {
        goto exit;
    }

    /* Initialize CHIP stack */
    result = CHIP_Init();
    if (result != CHIP_NO_ERROR)
    {
        goto exit;
    }

    /* Application task */
    result = Application_Init();
    if (result < 0)
    {
        goto exit;
    }

    /* Start FreeRTOS */
    vTaskStartScheduler();

exit:
    return 0;
}
