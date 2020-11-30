/******************************************************************************

 Group: CMCU, LPC
 Target Device: cc13x2_26x2

 ******************************************************************************
 
 Copyright (c) 2017-2020, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 
 
 *****************************************************************************/


/**
 * @file
 *   This file implements an alarm for openthread based on FreeRTOS timers.
 *
 */

#include <openthread/config.h>

#include <openthread/platform/alarm-milli.h>

#include "system.h"

#include <FreeRTOS.h>
#include <timers.h>
#include <task.h>
#include <projdefs.h>
#include <portmacro.h>

/*
 *  The maximum number of ticks before the tick count rolls over.  We use
 *  0xFFFFFFFF instead of 0x100000000 to avoid 64-bit math.
 */
#define MAX_TICKS 0xFFFFFFFF

/* The integral number of seconds in a period of MAX_TICKS */
#define MAX_SECONDS (MAX_TICKS / configTICK_RATE_HZ)

/* The total number of system ticks in MAX_SECONDS seconds */
#define MAX_SECONDS_TICKS (MAX_SECONDS * configTICK_RATE_HZ)

TimerHandle_t   sTimer;
static uint32_t sTime0   = 0;
static uint32_t sTimeout = 0;
static bool     sRunning = false;

void otPlatAlarmCallbackFunction(TimerHandle_t xTimer)
{
    platformAlarmSignal();
}

void platformAlarmInit(void)
{
    sTimer = xTimerCreate("ot_alarm", (TickType_t)-1, pdFALSE, NULL,
            otPlatAlarmCallbackFunction);
}

uint32_t otPlatAlarmMilliGetNow(void)
{
    unsigned long   secs;
    TickType_t      ticks;
    TickType_t      remTicks;
    unsigned long   remSecs;
    unsigned long   totSecs;
    unsigned long   totMsecs;
    uint32_t        numRollovers;
    TimeOut_t       timeout;

    /*
     *  This gets the number of tick count overflows as well as the tick
     *  count.  We use the internal API (new in FreeRTOS 10), since it can be
     *  called from an ISR.  Calling the public API, vTaskSetTimeOutState,
     *  will cause an exception if called from an ISR.
     */
    vTaskInternalSetTimeOutState(&timeout);

    ticks           = timeout.xTimeOnEntering;
    numRollovers    = timeout.xOverflowCount;

    /* Number of seconds in tick count */
    secs = ticks / configTICK_RATE_HZ;

    /* Remaining ticks after seconds are subtracted from tick count */
    remTicks = ticks - (secs * configTICK_RATE_HZ);

    /* Add contribution of remaining ticks from tick count rollovers */
    remTicks = remTicks + (((MAX_TICKS - MAX_SECONDS_TICKS) + 1) * numRollovers);

    remSecs = remTicks / configTICK_RATE_HZ;
    remTicks = remTicks - (remSecs * configTICK_RATE_HZ);

    totSecs  = (unsigned long)secs + remSecs + (MAX_SECONDS * numRollovers);
    totMsecs = (unsigned long)(remTicks * (1000 / configTICK_RATE_HZ));

    return (totSecs * 1000U) + (totMsecs % 1000);
}

void otPlatAlarmMilliStartAt(otInstance *aInstance, uint32_t aT0, uint32_t aDt)
{
    (void)aInstance;
    uint32_t          delta     = (otPlatAlarmMilliGetNow() - aT0);

    sTime0   = aT0;
    sTimeout    = aDt;
    sRunning = true;

    if (delta >= aDt)
    {
        // alarm is in the past
        platformAlarmSignal();
    }
    else
    {
        TickType_t timeoutTicks;
        BaseType_t status;

        timeoutTicks = ((aDt - delta) / 1000U) * configTICK_RATE_HZ +
                ((aDt - delta) % 1000U) * (uint64_t)configTICK_RATE_HZ / 1000L;

        status = xTimerChangePeriod(sTimer, timeoutTicks, (TickType_t)-1);
        if (status == pdPASS) {
            xTimerStart(sTimer, (TickType_t)-1);
        }

    }
}

void otPlatAlarmMilliStop(otInstance *aInstance)
{
    (void)aInstance;
    BaseType_t status;

    status = xTimerStop(sTimer, (TickType_t)-1);
    if (status == pdPASS) {
        sRunning = false;
    }
}

void platformAlarmProcess(otInstance *aInstance)
{
    if (sRunning)
    {
        uint32_t offsetTime = otPlatAlarmMilliGetNow() - sTime0;

        if (sTimeout <= offsetTime)
        {
            sRunning = false;
#if OPENTHREAD_CONFIG_DIAG_ENABLE

            if (otPlatDiagModeGet())
            {
                otPlatDiagAlarmFired(aInstance);
            }
            else
#endif /* OPENTHREAD_CONFIG_DIAG_ENABLE */
            {
                otPlatAlarmMilliFired(aInstance);
            }
        }
        else
        {
            if (pdFALSE == xTimerIsTimerActive(sTimer))
            {
                /* Timer fired a bit early, notify we still need processing. */
                platformAlarmSignal();
            }
        }
    }
}
