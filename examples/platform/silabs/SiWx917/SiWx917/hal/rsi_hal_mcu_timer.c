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

#include "rsi_driver.h"
#ifdef RSI_WITH_OS
/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "StackMacros.h"
#include "task.h"
#include "timers.h"

#if defined(SysTick)
#undef SysTick_Handler
/* FreeRTOS SysTick interrupt handler prototype */
extern void SysTick_Handler(void);
/* FreeRTOS tick timer interrupt handler prototype */
extern void xPortSysTickHandler(void);
#endif /* SysTick */
#endif

static volatile uint32_t _dwTickCount; // systick cout variable

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

    //! Initialise the timer

    //! register the call back

    //! Start timer

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

    //! Stop the timer

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

    //! read the timer and return timer value

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

    //! call the API for delay in micro seconds

    return;
}

/*===================================================*/
/**
 * @fn           void rsi_delay_ms(uint32_t delay)
 * @brief        create delay in micro seconds
 * @param[in]    delay, timer delay in micro seconds
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to create delay in micro seconds
 *
 */
void rsi_delay_ms1(uint32_t delay_ms)
{

    //! call the API for delay in milli seconds

    return;
}

/*===================================================*/
/**
 * @fn           void SysTick_Handler(void);
 * @brief        systick cout variable
 * @param[in]    none
 * @param[out]   none
 * @return       none
 * @description  This HAL API is systick cout variable
 *
 */

void SysTick_Handler(void)
{
    _dwTickCount++;
#ifdef RSI_WITH_OS
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED)
    {
        xPortSysTickHandler();
    }
#endif
}
/*===================================================*/
/**
 * @fn           uint32_t GetTickCount( void )
 * @brief        gets the tick count from systic ISR
 * @param[in]    delay, timer delay in micro seconds
 * @param[out]   none
 * @return       Returns the systick current tick count
 * @description  This HAL API gets the tick count from systic ISR
 *
 */

uint32_t GetTickCount(void)
{
    return _dwTickCount; // gets the tick count from systic ISR
}

/*===================================================*/
/**
 * @fn           void rsi_delay_ms(uint32_t delay)
 * @brief        create delay in milli seconds
 * @param[in]    delay, timer delay in milli seconds
 * @param[out]   none
 * @return       none
 * @description  This HAL API should contain the code to create delay in milli seconds
 *
 */

void rsi_delay_ms(uint32_t delay_ms)
{
    uint64_t waitTime = rsi_hal_gettickcount() + delay_ms;

    while (waitTime > rsi_hal_gettickcount())
    {
        // we busy wait
    }
}

/*===================================================*/
/**
 * @fn           uint32_t rsi_hal_gettickcount()
 * @brief        provides a tick value in milliseconds
 * @return       tick value
 * @description  This HAL API should contain the code to read the timer tick count value in milliseconds
 *
 */

uint32_t rsi_hal_gettickcount(void)
{
    return GetTickCount();
}
