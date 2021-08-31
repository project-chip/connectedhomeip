/*
 *
 *    Copyright (c) 2020 Google LLC.
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
 *
 */

#include "FreeRtosHooks.h"

#include "FreeRTOS.h"
#include "semphr.h"

#include <assert.h>

#include <stdio.h>
#include <string.h>

#include "PWR_Interface.h"
#include "TimersManager.h"
#include "board.h"

/* Bluetooth Low Energy */
#include "ble_config.h"
#include "l2ca_cb_interface.h"

#include "controller_interface.h"

#if defined gLoggingActive_d && (gLoggingActive_d > 0)
#include "dbg_logging.h"
#ifndef DBG_APP
#define DBG_APP 0
#endif
#define APP_DBG_LOG(fmt, ...)                                                                                                      \
    if (DBG_APP)                                                                                                                   \
        do                                                                                                                         \
        {                                                                                                                          \
            DbgLogAdd(__FUNCTION__, fmt, VA_NUM_ARGS(__VA_ARGS__), ##__VA_ARGS__);                                                 \
        } while (0);
#else
#define APP_DBG_LOG(...)
#endif

static inline void mutex_init(mbedtls_threading_mutex_t * p_mutex)
{
    assert(p_mutex != NULL);
    *p_mutex = xSemaphoreCreateMutex();
    assert(*p_mutex != NULL);
}

static inline void mutex_free(mbedtls_threading_mutex_t * p_mutex)
{
    assert(p_mutex != NULL);
    assert(*p_mutex != NULL);
    vSemaphoreDelete(*p_mutex);
}

static inline int mutex_lock(mbedtls_threading_mutex_t * p_mutex)
{
    assert(p_mutex != NULL);
    assert(*p_mutex != NULL);
    return xSemaphoreTake(*p_mutex, portMAX_DELAY) != pdTRUE;
}

static inline int mutex_unlock(mbedtls_threading_mutex_t * p_mutex)
{
    assert(p_mutex != NULL);
    assert(*p_mutex != NULL);
    return xSemaphoreGive(*p_mutex) != pdTRUE;
}

void freertos_mbedtls_mutex_init(void)
{
    mbedtls_threading_set_alt(mutex_init, mutex_free, mutex_lock, mutex_unlock);
}

void freertos_mbedtls_mutex_free(void)
{
    mbedtls_threading_free_alt();
}

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode) && (configUSE_TICKLESS_IDLE != 0)

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void vPortSetupTimerInterrupt(void)
{
    /* Stop and clear the SysTick. */
    SysTick->CTRL = 0UL;
    SysTick->VAL  = 0UL;

#if DBG_PostStepTickAssess
    tickless_SystickCheckDriftInit();
#endif

    /* configure module for tickless mode */
    tickless_init();

#if gApp_SystemClockCalibration
    /* calibration on the internal FRO48MHz clock - this clock is inaccurate (2%),
     * so it needs calibration if we want the systick to be accurate to less
     * than 500pp. */
    tickless_StartFroCalibration();
#endif

    /* Configure SysTick to interrupt at the requested rate. */
    SysTick_Config(CLOCK_GetFreq(kCLOCK_CoreSysClk) / configTICK_RATE_HZ);
}

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    tmrlp_tickless_systick_t lp_ctx;
    APP_DBG_LOG("xExpectedIdleTime = %d ticks %d ms", xExpectedIdleTime, xExpectedIdleTime * portTICK_PERIOD_MS);
    OSA_InterruptDisable();

    /* Do not go to sleep, lowpower or WFI if there is a pending task to schedule
     * Scheduler is suspended when calling vPortSuppressTicksAndSleep,
     * as interrupt are disabled if one is pending, eTaskConfirmSleepModeStatus
     * will prevent sleep.
     */
    if (eTaskConfirmSleepModeStatus() == eAbortSleep)
    {
        APP_DBG_LOG("task to schedule");
        /* Do nothing */
    }
#if gApp_SystemClockCalibration
    /* Prevent lowpower / tickless mode if cal ongoing - get estimated core freq */
    else if (tickless_EstimateCoreClockFreq())
    {
        /* can eventually enter sleep or re evaluate on next idle loop, the calibration shall take 4 ms average */
        // PWR_EnterSleep();
    }
#endif

    /* Do not go to power down if:
     * - the RTOS is expecting a wake-up too close to the current date
     * To worth a power down the xExpectedIdleTime should be
     * > to 2 rtos tick (which includes residual_count worst margin + carry after wake-up worst margin)
     * + enter/exist power down duration converted in RTOS tick
     * - power down is not allowed
     */
    else if ((xExpectedIdleTime >
              (2 + ((PWR_SYSTEM_EXIT_LOW_POWER_DURATION_MS + PWR_SYSTEM_ENTER_LOW_POWER_DURATION_MS) / portTICK_PERIOD_MS) + 1)))
    {
        int result = PWR_CheckIfDeviceCanGoToSleepExt();
        if (result >= kPmPowerDown0)
        {
            PWR_WakeupReason_t wakeupReason;
            lp_ctx.exitTicklessDuration32kTick = MILLISECONDS_TO_TICKS32K(PWR_SYSTEM_EXIT_LOW_POWER_DURATION_MS);
            /* Tickless pre processing */
            tickless_PreProcessing(&lp_ctx, xExpectedIdleTime);

            /* Enter power down */
            wakeupReason = PWR_EnterPowerDown();

            APP_DBG_LOG("wakeReason=%x", (uint16_t) wakeupReason.AllBits);
            (void) wakeupReason;

            /* Tickless post processing */
            tickless_PostProcessing(&lp_ctx);

#if DBG_PostStepTickAssess
            if (wakeupReason.Bits.FromTMR == 1)
                configASSERT(lp_ctx.idle_tick_jump == xExpectedIdleTime);
            if (wakeupReason.Bits.DidPowerDown == 1)
                configASSERT((wakeupReason.AllBits & ~0x8000U) != 0);
#endif
        }
        else if ((result == kPmSleep) || (result < 0))
        {
#if App_SuppressTickInStopMode
            lp_ctx.exitTicklessDuration32kTick = 0;
            /* Tickless pre processing */
            tickless_PreProcessing(&lp_ctx, xExpectedIdleTime);
#endif

            PWR_EnterSleep();

#if App_SuppressTickInStopMode
            /* Tickless post processing */
            tickless_PostProcessing(&lp_ctx);
#endif
        }
    }
    else
    {
#if App_SuppressTickInStopMode
        lp_ctx.exitTicklessDuration32kTick = 0;
        /* Tickless pre processing */
        tickless_PreProcessing(&lp_ctx, xExpectedIdleTime);
#endif

        PWR_EnterSleep();

#if App_SuppressTickInStopMode
        /* Tickless post processing */
        tickless_PostProcessing(&lp_ctx);
#endif
    }

#if DBG_PostStepTickAssess
    tickless_SystickCheckDrift();
#endif

    OSA_InterruptEnable();
}

static void BOARD_ActionOnIdle(void)
{
#if ((defined gTcxo32k_ModeEn_c) && (gTcxo32k_ModeEn_c != 0))
    BOARD_tcxo32k_compensation_run(2, 0);
#endif
#if ((defined gTcxo32M_ModeEn_c) && (gTcxo32M_ModeEn_c != 0))
    BOARD_tcxo32M_compensation_run(2, 10); /* 2 degrees - wait for 10us */
#endif
}

void vApplicationIdleHook(void)
{
    BOARD_ActionOnIdle();
}

#endif /*  (cPWR_UsePowerDownMode) && (configUSE_TICKLESS_IDLE != 0) */
