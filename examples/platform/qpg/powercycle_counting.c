/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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

/** @file "gpAppFramework_Reset.c"
 *
 *  Application API
 *
 *  Implementation of gpAppFramework Reset
 */

/*****************************************************************************
 *                    Includes Definitions
 *****************************************************************************/
#define GP_COMPONENT_ID_APPFRAMEWORK 56
#define GP_COMPONENT_ID GP_COMPONENT_ID_APPFRAMEWORK

#include "powercycle_counting.h"
#include "global.h"
#include "gpAssert.h"
#include "gpLog.h"
#include "gpReset.h"
#include "gpSched.h"
#include "qvCHIP.h"
#include "qvCHIP_KVS.h"

/*****************************************************************************
 *                    Macro Definitions
 *****************************************************************************/
#define RESET_COUNTING_PERIOD_US 2000000 // 2s

#define KVS_RESET_CYCLES_KEY "qrst"
/*****************************************************************************
 *                    Static Function Prototypes
 *****************************************************************************/

/*****************************************************************************
 *                    Static Function Definitions
 *****************************************************************************/

static void gpAppFramework_HardwareResetTriggered(void)
{
    UInt8 resetCounts;
    qvStatus_t status;
    size_t readBytesSize;

    status = qvCHIP_KvsGet(KVS_RESET_CYCLES_KEY, &resetCounts, 1, &readBytesSize, 0);
    if (status == QV_STATUS_INVALID_DATA)
    {
        // No reset count stored yet - create new key
        resetCounts = 0;
    }
    else if (status != QV_STATUS_NO_ERROR)
    {
        GP_LOG_SYSTEM_PRINTF("got status %d", 0, status);
        GP_ASSERT_SYSTEM(status == QV_STATUS_NO_ERROR); // fixme
    }

    GP_LOG_SYSTEM_PRINTF("ResetCount[%d]", 0, resetCounts);

    resetCounts++;

    status = qvCHIP_KvsPut(KVS_RESET_CYCLES_KEY, &resetCounts, 1);
    if (status != QV_STATUS_NO_ERROR)
    {
        GP_LOG_SYSTEM_PRINTF("got status %d", 0, status);
        GP_ASSERT_SYSTEM(status == QV_STATUS_NO_ERROR); // fixme
    }
}

/*****************************************************************************
 *                    Public Function Definitions
 *****************************************************************************/
UInt8 gpAppFramework_Reset_GetResetCount(void)
{
    UInt8 resetCounts;
    const UInt8 resetCountsCleared = 0;

    qvStatus_t status;
    size_t readBytesSize;
    status = qvCHIP_KvsGet(KVS_RESET_CYCLES_KEY, &resetCounts, 1, &readBytesSize, 0);
    if (status == QV_STATUS_INVALID_DATA || readBytesSize != 1)
    {
        // Reset count was not stored yet
        resetCounts = 0;
    }
    else if (status != QV_STATUS_NO_ERROR)
    {
        GP_LOG_SYSTEM_PRINTF("got status %d", 0, status);
        GP_ASSERT_SYSTEM(status == QV_STATUS_NO_ERROR);
    }

    GP_LOG_PRINTF("Processing reset counts: %u", 0, resetCounts);

    status = qvCHIP_KvsPut(KVS_RESET_CYCLES_KEY, &resetCountsCleared, 1);
    if (status != QV_STATUS_NO_ERROR)
    {
        GP_LOG_SYSTEM_PRINTF("got status %d", 0, status);
        GP_ASSERT_SYSTEM(status == QV_STATUS_NO_ERROR); // fixme
    }

    return resetCounts;
}

void gpAppFramework_Reset_Init(void)
{
    if (gpReset_GetResetReason() == gpReset_ResetReason_HW_Por)
    {
        gpAppFramework_HardwareResetTriggered();
    }

    gpSched_ScheduleEvent(RESET_COUNTING_PERIOD_US, gpAppFramework_Reset_cbTriggerResetCountCompleted);
}
