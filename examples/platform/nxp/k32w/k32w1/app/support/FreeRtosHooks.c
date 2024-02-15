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

#include "NVM_Interface.h"
#include "PWR_Interface.h"
#include "board.h"
#include "fsl_os_abstraction.h"

/* Bluetooth Low Energy */
#include "ble_config.h"
#include "l2ca_cb_interface.h"

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

#if (configUSE_TICKLESS_IDLE != 0)

/*! *********************************************************************************
 *\private
 *\fn           void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
 *\brief        This function will try to put the MCU into a deep sleep mode for at
 *              most the maximum OS idle time specified. Else the MCU will enter a
 *              sleep mode until the first IRQ.
 *
 *\param  [in]  xExpectedIdleTime    The idle time in OS ticks.
 *
 *\retval       none.
 *
 *\remarks      This feature is available only for FreeRTOS.
 ********************************************************************************** */
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    bool abortIdle = false;
    uint64_t actualIdleTimeUs, expectedIdleTimeUs;

    /* The OSA_InterruptDisable() API will prevent us to wakeup so we use
     * OSA_DisableIRQGlobal() */
    OSA_DisableIRQGlobal();

    /* Disable and prepare systicks for low power */
    abortIdle = PWR_SysticksPreProcess((uint32_t) xExpectedIdleTime, &expectedIdleTimeUs);

    if (abortIdle == false)
    {
        /* Enter low power with a maximal timeout */
        actualIdleTimeUs = PWR_EnterLowPower(expectedIdleTimeUs);

        /* Re enable systicks and compensate systick timebase */
        PWR_SysticksPostProcess(expectedIdleTimeUs, actualIdleTimeUs);
    }

    /* Exit from critical section */
    OSA_EnableIRQGlobal();
}
#endif

extern void OTAIdleActivities(void);

void vApplicationIdleHook(void)
{
    // Data queued by PDM will be written to external flash
    // when PDM_vIdleTask is called. Interrupts are disabled
    // to ensure there is no context switch during the actual
    // writing, thus avoiding race conditions.
    OSA_InterruptDisable();
#if CHIP_PLAT_NVM_SUPPORT
    NvIdle();
#endif
    OSA_InterruptEnable();

    OTAIdleActivities();
}
