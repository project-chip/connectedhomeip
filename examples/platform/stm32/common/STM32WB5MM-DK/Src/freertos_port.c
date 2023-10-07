/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos_port.c
 * Description        : Custom porting of FreeRTOS functionalities
 *
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_common.h"

#include "FreeRTOS.h"
#include "stm32_lpm.h"
#include "task.h"
#include <limits.h>

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    uint32_t LpTimeLeftOnEntry;
    uint8_t LpTimerFreeRTOS_Id;
} LpTimerContext_t;

/* Private defines -----------------------------------------------------------*/
#ifndef configSYSTICK_CLOCK_HZ
#define configSYSTICK_CLOCK_HZ configCPU_CLOCK_HZ
/* Ensure the SysTick is clocked at the same frequency as the core. */
#define portNVIC_SYSTICK_CLK_BIT (1UL << 2UL)
#else
/* The way the SysTick is clocked is not modified in case it is not the same
        as the core. */
#define portNVIC_SYSTICK_CLK_BIT (0)
#endif

#define CPU_CLOCK_KHZ (configCPU_CLOCK_HZ / 1000)

/* Constants required to manipulate the core.  Registers first... */
#define portNVIC_SYSTICK_CTRL_REG (*((volatile uint32_t *) 0xe000e010))
#define portNVIC_SYSTICK_LOAD_REG (*((volatile uint32_t *) 0xe000e014))
#define portNVIC_SYSTICK_CURRENT_VALUE_REG (*((volatile uint32_t *) 0xe000e018))
#define portNVIC_SYSTICK_INT_BIT (1UL << 1UL)
#define portNVIC_SYSTICK_ENABLE_BIT (1UL << 0UL)
#define portNVIC_SYSTICK_COUNT_FLAG_BIT (1UL << 16UL)

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*
 * The number of SysTick increments that make up one tick period.
 */
#if (CFG_LPM_SUPPORTED != 0)
static uint32_t ulTimerCountsForOneTick;

static LpTimerContext_t LpTimerContext;
#endif
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if (CFG_LPM_SUPPORTED != 0)
static void LpTimerInit(void);
static void LpTimerCb(void);
static void LpTimerStart(uint32_t time_to_sleep);
static void LpEnter(void);
static uint32_t LpGetElapsedTime(void);
void vPortSetupTimerInterrupt(void);
#endif

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief Implement the tickless feature
 *
 *
 * @param: xExpectedIdleTime is given in number of FreeRTOS Ticks
 * @retval: None
 */
void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    /* If low power is not used, do not stop the SysTick and continue execution */
#if (CFG_LPM_SUPPORTED != 0)
    /**
     * Although this is not documented as such, when xExpectedIdleTime = 0xFFFFFFFF = (~0),
     * it likely means the system may enter low power for ever ( from a FreeRTOS point of view ).
     * Otherwise, for a FreeRTOS tick set to 1ms, that would mean it is requested to wakeup in 8 years from now.
     * When the system may enter low power mode for ever, FreeRTOS is not really interested to maintain a
     * systick count and when the system exits from low power mode, there is no need to update the count with
     * the time spent in low power mode
     */
    uint32_t ulCompleteTickPeriods;

    /* Stop the SysTick  to avoid the interrupt to occur while in the critical section.
     * Otherwise, this will prevent the device to enter low power mode
     * At this time, an update of the systick will not be considered
     *
     */
    portNVIC_SYSTICK_CTRL_REG &= ~portNVIC_SYSTICK_ENABLE_BIT;

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
          method as that will mask interrupts that should exit sleep mode. */
    __disable_irq();
    __DSB();
    __ISB();

    /* If a context switch is pending or a task is waiting for the scheduler
          to be unsuspended then abandon the low power entry. */
    if (eTaskConfirmSleepModeStatus() == eAbortSleep)
    {
        /* Restart SysTick. */
        portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;

        /* Re-enable interrupts - see comments above __disable_interrupt()
                    call above. */
        __enable_irq();
    }
    else
    {
        if (xExpectedIdleTime != (~0))
        {
            /* Remove one tick to wake up before the event occurs */
            xExpectedIdleTime--;
            /* Start the low power timer */
            LpTimerStart(xExpectedIdleTime);
        }

        /* Enter low power mode */
        LpEnter();

        if (xExpectedIdleTime != (~0))
        {
            /**
             * Get the number of FreeRTOS ticks that has been suppressed
             * In the current implementation, this shall be kept in critical section
             * so that the timer server return the correct elapsed time
             */
            ulCompleteTickPeriods = LpGetElapsedTime();
            vTaskStepTick(ulCompleteTickPeriods);
        }

        /* Restart SysTick */
        portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
        portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;
        portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;

        /* Exit with interrUpts enabled. */
        __enable_irq();
    }
#endif
}

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency and initialize a low power timer
 * The current implementation is kept as close as possible to the default tickless
 * mode provided.
 * The systick is still used when there is no need to go in low power mode.
 * When the system needs to enter low power mode, the tick is suppressed and a low power timer
 * is used over that time
 * Note that in sleep mode, the system clock is still running and the default tickless implementation
 * using systick could have been kept.
 * However, as at that time, it is not yet known whereas the low power mode that will be used is stop mode or
 * sleep mode, it is easier and simpler to go with a low power timer as soon as the tick need to be
 * suppressed.
 */
#if (CFG_LPM_SUPPORTED != 0)
void vPortSetupTimerInterrupt(void)
{
    LpTimerInit();

    /* Calculate the constants required to configure the tick interrupt. */
    ulTimerCountsForOneTick = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ);

    /* Stop and clear the SysTick. */
    portNVIC_SYSTICK_CTRL_REG          = 0UL;
    portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;

    /* Configure SysTick to interrupt at the requested rate. */
    portNVIC_SYSTICK_LOAD_REG = (configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;
    portNVIC_SYSTICK_CTRL_REG = (portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT);
}
#endif

/**
 * @brief The current implementation uses the hw_timerserver to provide a low power timer
 *        This may be replaced by another low power timer.
 *
 * @param  None
 * @retval None
 */
#if (CFG_LPM_SUPPORTED != 0)
static void LpTimerInit(void)
{
    (void) HW_TS_Create(CFG_TIM_PROC_ID_ISR, &(LpTimerContext.LpTimerFreeRTOS_Id), hw_ts_SingleShot, LpTimerCb);

    return;
}
#endif

/**
 * @brief Low power timer callback
 *
 * @param  None
 * @retval None
 */
#if (CFG_LPM_SUPPORTED != 0)
static void LpTimerCb(void)
{
    /**
     * Nothing to be done
     */

    return;
}
#endif

/**
 * @brief  Request to start a low power timer ( running is stop mode )
 *
 * @param  time_to_sleep : Number of FreeRTOS ticks
 * @retval None
 */
#if (CFG_LPM_SUPPORTED != 0)
static void LpTimerStart(uint32_t time_to_sleep)
{
    uint64_t time;

    /* Converts the number of FreeRTOS ticks into hw timer tick */
    if (time_to_sleep > (ULLONG_MAX / 1e12)) /* Prevent overflow in else statement */
    {
        time = 0xFFFF0000; /* Maximum value equal to 24 days */
    }
    else
    {
        /* The result always fits in uint32_t and is always less than 0xFFFF0000 */
        time = time_to_sleep * 1000000000000ULL;
        time = (uint64_t) (time / (CFG_TS_TICK_VAL_PS * configTICK_RATE_HZ));
    }

    HW_TS_Start(LpTimerContext.LpTimerFreeRTOS_Id, (uint32_t) time);

    /**
     * There might be other timers already running in the timer server that may elapse
     * before this one.
     * Store how long before the next event so that on wakeup, it will be possible to calculate
     * how long the tick has been suppressed
     */
    LpTimerContext.LpTimeLeftOnEntry = HW_TS_RTC_ReadLeftTicksToCount();

    return;
}
#endif

/**
 * @brief  Enter low power mode
 *
 * @param  None
 * @retval None
 */
#if (CFG_LPM_SUPPORTED != 0)
static void LpEnter(void)
{
#if (CFG_LPM_SUPPORTED == 1)
    UTIL_LPM_EnterLowPower();
#endif
    return;
}
#endif

/**
 * @brief  Read how long the tick has been suppressed
 *
 * @param  None
 * @retval The number of tick rate (FreeRTOS tick)
 */
#if (CFG_LPM_SUPPORTED != 0)
static uint32_t LpGetElapsedTime(void)
{
    uint64_t val_ticks, time_ps;
    uint32_t LpTimeLeftOnExit;

    LpTimeLeftOnExit = HW_TS_RTC_ReadLeftTicksToCount();
    /* This cannot overflow. Max result is ~ 1.6e13 */
    time_ps = (uint64_t) ((CFG_TS_TICK_VAL_PS) * (uint64_t) (LpTimerContext.LpTimeLeftOnEntry - LpTimeLeftOnExit));

    /* time_ps can be less than 1 RTOS tick in following situations
     * a) MCU didn't go to STOP2 due to wake-up unrelated to Timer Server or woke up from STOP2 very shortly after.
     *    Advancing RTOS clock by 1 FreeRTOS tick doesn't hurt in this case.
     * b) vPortSuppressTicksAndSleep(xExpectedIdleTime) was called with xExpectedIdleTime = 2 which is minimum value defined by
     * configEXPECTED_IDLE_TIME_BEFORE_SLEEP. The xExpectedIdleTime is decremented by one RTOS tick to wake-up in advance. Ex: RTOS
     * tick is 1ms, the timer Server wakes the MCU in ~977 us. RTOS clock should be advanced by 1 ms.
     * */
    if (time_ps <= (1e12 / configTICK_RATE_HZ)) /* time_ps < RTOS tick */
    {
        val_ticks = 1;
    }
    else
    {
        /* Convert pS time into OS ticks */
        val_ticks = time_ps * configTICK_RATE_HZ;    /* This cannot overflow. Max result is ~ 1.6e16 */
        val_ticks = (uint64_t) (val_ticks / (1e12)); /* The result always fits in uint32_t */
    }

    /**
     * The system may have been out from another reason than the timer
     * Stop the timer after the elapsed time is calculated other wise, HW_TS_RTC_ReadLeftTicksToCount()
     * may return 0xFFFF ( TIMER LIST EMPTY )
     * It does not hurt stopping a timer that exists but is not running.
     */
    HW_TS_Stop(LpTimerContext.LpTimerFreeRTOS_Id);

    return (uint32_t) val_ticks;
}
#endif
