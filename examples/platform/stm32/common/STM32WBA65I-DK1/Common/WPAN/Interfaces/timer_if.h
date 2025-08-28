/**
  ******************************************************************************
  * @file    timer_if.h
  * @author  MCD Application Team
  * @brief  configuration of the timer_if.c instances
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef TIMER_IF_H
#define TIMER_IF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32_timer.h"
#include "stm32_systime.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define RTC_N_PREDIV_S      (10U)
#define RTC_PREDIV_S        ( ( 1U << RTC_N_PREDIV_S ) - 1U )
#define RTC_PREDIV_A        ( ( 1U << ( 15U - RTC_N_PREDIV_S ) ) - 1U )

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* External variables --------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief Init RTC hardware
  * @return Status based on @ref UTIL_TIMER_Status_t
  */
UTIL_TIMER_Status_t TIMER_IF_Init(void);

/**
  * @brief Set the alarm
  * @note The alarm is set at timeout from timer Reference (TimerContext)
  * @param timeout Duration of the Timer in ticks
  * @return Status based on @ref UTIL_TIMER_Status_t
  */
UTIL_TIMER_Status_t TIMER_IF_StartTimer(uint32_t timeout);

/**
  * @brief Stop the Alarm
  * @return Status based on @ref UTIL_TIMER_Status_t
  */
UTIL_TIMER_Status_t TIMER_IF_StopTimer(void);

/**
  * @brief set timer Reference (TimerContext)
  * @return  Timer Reference Value in  Ticks
  */
uint32_t TIMER_IF_SetTimerContext(void);

/**
  * @brief Get the RTC timer Reference
  * @return Timer Value in  Ticks
  */
uint32_t TIMER_IF_GetTimerContext(void);

/**
  * @brief Get the timer elapsed time since timer Reference (TimerContext) was set
  * @return RTC Elapsed time in ticks
  */
uint32_t TIMER_IF_GetTimerElapsedTime(void);

/**
  * @brief Get the timer value
  * @return RTC Timer value in ticks
  */
uint32_t TIMER_IF_GetTimerValue(void);

/**
  * @brief Return the minimum timeout in ticks the RTC is able to handle
  * @return minimum value for a timeout in ticks
  */
uint32_t TIMER_IF_GetMinimumTimeout(void);

/**
  * @brief a delay of delay ms by polling RTC
  * @param delay in ms
  */
void TIMER_IF_DelayMs(uint32_t delay);

/**
  * @brief converts time in ms to time in ticks
  * @param[in] timeMilliSec time in milliseconds
  * @return time in timer ticks
  */
uint32_t TIMER_IF_Convert_ms2Tick(uint32_t timeMilliSec);

/**
  * @brief converts time in ticks to time in ms
  * @param[in] tick time in timer ticks
  * @return time in timer milliseconds
  */
uint32_t TIMER_IF_Convert_Tick2ms(uint32_t tick);

/**
  * @brief Get rtc time
  * @param[out] subSeconds in ticks
  * @return time seconds
  */
uint32_t TIMER_IF_GetTime(uint16_t *subSeconds);

/**
  * @brief write seconds in backUp register
  * @note Used to store seconds difference between RTC time and Unix time
  * @param[in] Seconds time in seconds
  */
void TIMER_IF_BkUp_Write_Seconds(uint32_t Seconds);

/**
  * @brief reads seconds from backUp register
  * @note Used to store seconds difference between RTC time and Unix time
  * @return Time in seconds
  */
uint32_t TIMER_IF_BkUp_Read_Seconds(void);

/**
  * @brief writes SubSeconds in backUp register
  * @note Used to store SubSeconds difference between RTC time and Unix time
  * @param[in] SubSeconds time in SubSeconds
  */
void TIMER_IF_BkUp_Write_SubSeconds(uint32_t SubSeconds);

/**
  * @brief reads SubSeconds from backUp register
  * @note Used to store SubSeconds difference between RTC time and Unix time
  * @return Time in SubSeconds
  */
uint32_t TIMER_IF_BkUp_Read_SubSeconds(void);

#ifdef __cplusplus
}
#endif

#endif /* TIMER_IF_H */
