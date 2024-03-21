/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

/**
 * Includes
 */

#include "em_cmu.h"
#ifdef RSI_WITH_OS
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "timers.h"
#if defined(SysTick)
#undef SysTick_Handler
/* FreeRTOS SysTick interrupt handler prototype */
extern void SysTick_Handler(void);
/* FreeRTOS tick timer interrupt handler prototype */
extern void xPortSysTickHandler(void);
#endif /* SysTick */
#endif /* RSI_WITH_OS */
#include "wfx_host_events.h"

/* RSI Driver include file */
#include "rsi_driver.h"
/* RSI WLAN Config include file */
#include "rsi_bootup_config.h"
#include "rsi_common_apis.h"
#include "rsi_data_types.h"
#include "rsi_error.h"
#include "rsi_nwk.h"
#include "rsi_socket.h"
#include "rsi_utils.h"
#include "rsi_wlan.h"
#include "rsi_wlan_apis.h"
#include "rsi_wlan_config.h"
#include "wfx_rsi.h"

#ifndef _use_the_rsi_defined_functions

StaticTimer_t sRsiTimerBuffer;

/*
 * We (Matter port) need a few functions out of this file
 * They are at the top
 */
uint32_t rsi_hal_gettickcount(void)
{
    return xTaskGetTickCount();
}
void rsi_delay_ms(uint32_t delay_ms)
{
#ifndef RSI_WITH_OS
    uint32_t start;
#endif
    if (delay_ms == 0) // Check if delay is 0msec
        return;

#ifdef RSI_WITH_OS
    vTaskDelay(pdMS_TO_TICKS(delay_ms));
#else
    start = rsi_hal_gettickcount();
    do
    {
    } while (rsi_hal_gettickcount() - start < delay_ms);
#endif
}
static struct rsi_timer
{
    void (*func)(void);
    TimerHandle_t handle;
    uint8_t id;
    uint8_t name[3];
} rsi_timer[WFX_RSI_NUM_TIMERS];
static void timer_cb(TimerHandle_t thandle)
{
    int x;
    for (x = 0; x < WFX_RSI_NUM_TIMERS; x++)
    {
        if (rsi_timer[x].handle == thandle)
        {
            (*rsi_timer[x].func)();
            break;
        }
    }
}

/*
 * Run a one-shot/periodic timer
 */
int32_t rsi_timer_start(uint8_t timer_node, uint8_t mode, uint8_t type, uint32_t duration, void (*rsi_timer_cb)(void))
{
    int x;
    struct rsi_timer * tp;

    if (mode == RSI_HAL_TIMER_MODE_MILLI)
        return RSI_ERROR_INVALID_OPTION; /* Not supported for now - Fix this later */
    for (x = 0; x < WFX_RSI_NUM_TIMERS; x++)
    {
        tp = &rsi_timer[x];
        if (tp->handle == NULL)
        {
            goto found;
        }
    }
    /* No space */
    return RSI_ERROR_INSUFFICIENT_BUFFER;
found:
    tp->name[0] = 'r';
    tp->name[1] = timer_node;
    tp->name[2] = 0;
    tp->func    = rsi_timer_cb;
    tp->handle =
        xTimerCreateStatic((char *) &tp->name[0], pdMS_TO_TICKS(duration),
                           ((mode == RSI_HAL_TIMER_TYPE_SINGLE_SHOT) ? pdFALSE : pdTRUE), NULL, timer_cb, &sRsiTimerBuffer);

    if (tp->handle == NULL)
    {
        return RSI_ERROR_INSUFFICIENT_BUFFER;
    }

    (void) xTimerStart(tp->handle, TIMER_TICKS_TO_WAIT_0);

    return RSI_ERROR_NONE;
}
/*==============================================*/
/**
 * @fn          uint32_t rsi_ms_to_tick(uint32_t timeout_ms)
 * @brief       Converting ms to os ticks
 * @param[in]   timeout_ms - timeout in ms
 * @return      uint32_t
 */
uint32_t rsi_ms_to_tick(uint32_t timeout_ms)
{
    uint32_t ticks_cnt = 0, ticks_val = 0, ticks = 0;
    ticks_cnt = SysTick->LOAD + 1UL;
    ticks_val = SystemCoreClock / ticks_cnt;
    ticks     = ticks_val / 1000;
    return (timeout_ms * ticks);
}
#else /* _use_the_rsi_defined_functions */

/* Counts 1ms timeTicks */
volatile uint32_t msTicks = 0;

/*===================================================*/
/**
 * @fn           int32_t rsi_timer_start(uint8_t timer_no, uint8_t mode,uint8_t type,uint32_t duration,void (*
 * rsi_timer_expiry_handler)())
 * @brief        Starts and configures timer
 * @param[in]    timer_node, timer node to be configured.
 * @param[in]    mode , mode of the timer
 *               0 - Micro seconds mode
 *               1 - Milli seconds mode
 * @param[in]    type, type of  the timer
 *               0 - single shot type
 *               1 - periodic type
 * @param[in]    duration, timer duration
 * @param[in]    rsi_timer_expiry_handler() ,call back function to handle timer interrupt
 * @param[out]   none
 * @return       0 - success
 *               !0 - Failure
 * @description  This HAL API should contain the code to initialize the timer and start the timer
 *
 */

int32_t rsi_timer_start(uint8_t timer_node, uint8_t mode, uint8_t type, uint32_t duration, void (*rsi_timer_expiry_handler)(void))
{

    // Initialise the timer

    // register the call back

    // Start timer

    return 0;
}

/*===================================================*/
/**
 * @fn           int32_t rsi_timer_stop(uint8_t timer_no)
 * @brief        Stops timer
 * @param[in]    timer_node, timer node to stop
 * @param[out]   none
 * @return       0 - success
 *               !0 - Failure
 * @description  This HAL API should contain the code to stop the timer
 *
 */

int32_t rsi_timer_stop(uint8_t timer_node)
{

    // Stop the timer

    return 0;
}

/*===================================================*/
/**
 * @fn           uint32_t rsi_timer_read(uint8_t timer_node)
 * @brief        read timer
 * @param[in]    timer_node, timer node to read
 * @param[out]   none
 * @return       timer value
 * @description  This HAL API should contain API to  read the timer
 *
 */
uint32_t rsi_timer_read(uint8_t timer_node)
{

    volatile uint32_t timer_val = 0;

    // read the timer and return timer value

    return timer_val;
}

/*===================================================*/
/**
 * @fn           void rsi_delay_us(uint32_t delay)
 * @brief        create delay in micro seconds
 * @param[in]    delay_us, timer delay in micro seconds
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to create delay in micro seconds
 *
 */
void rsi_delay_us(uint32_t delay_us)
{

    // call the API for delay in micro seconds

    return;
}

#ifdef RSI_M4_INTERFACE

extern void SysTick_Handler(void);

void SysTick_Handler(void)
{
    _dwTickCount++;
}

uint32_t GetTickCount(void)
{
    return _dwTickCount; // gets the tick count from systic ISR
}
#endif

/*===================================================*/
/**
 * @fn           void rsi_delay_ms(uint32_t delay)
 * @brief        create delay in milli seconds
 * @param[in]    delay, timer delay in milli seconds
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to create delay in milli seconds
 */
void rsi_delay_ms(uint32_t delay_ms)
{
#ifndef RSI_WITH_OS
    uint32_t start;
#endif
    if (delay_ms == DELAY0)
        return;

#ifdef RSI_WITH_OS
    vTaskDelay(delay_ms);
#else
    start = rsi_hal_gettickcount();
    do
    {
    } while (rsi_hal_gettickcount() - start < delay_ms);
#endif
}

/*===================================================*/
/**
 * @fn           uint32_t rsi_hal_gettickcount()
 * @brief        provides a tick value in milliseconds
 * @return       tick value
 * @description  This HAL API should contain the code to read the timer tick count value in milliseconds
 *
 */

#ifndef RSI_HAL_USE_RTOS_SYSTICK
/*
  SysTick handler implementation that also clears overflow flag.
*/
void SysTick_Handler(void)
{
    /* Increment counter necessary in Delay()*/
    msTicks++;
#ifdef RSI_WITH_OS
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
#endif
}

uint32_t rsi_hal_gettickcount(void)
{
    return msTicks;

#ifdef LINUX_PLATFORM
    // Define your API to get the tick count delay in milli seconds from systic ISR and return the resultant value
    struct rsi_timeval tv1;
    gettimeofday(&tv1, NULL);
    return (tv1.tv_sec * CONVERT_SEC_TO_MSEC + tv1.tv_usec * CONVERT_USEC_TO_MSEC);
#endif
}

#else
uint32_t rsi_hal_gettickcount(void)
{
    return xTaskGetTickCount();
}
#endif /* RSI_HAL_USE_RTOS_SYSTICK */
#endif /* _use_the_rsi_defined_functions */
