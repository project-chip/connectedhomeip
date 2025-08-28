/**
  ******************************************************************************
  * @file    timer_if.c
  * @author  MCD Application Team
  * @brief   Configure RTC Alarm, Tick and Calendar manager
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

/* Includes ------------------------------------------------------------------*/
#include <math.h>

#include "stm32wbaxx_hal.h"
#include "stm32wbaxx_hal_conf.h"
#include "stm32wbaxx_ll_rtc.h"

#include "timer_if.h"

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
/**
  * @brief Minimum timeout delay of Alarm in ticks
  */
#define MIN_ALARM_DELAY    3

/**
  * @brief Backup seconds register
  */
#define RTC_BKP_SECONDS    RTC_BKP_DR0

/**
  * @brief Backup subseconds register
  */
#define RTC_BKP_SUBSECONDS RTC_BKP_DR1

/**
  * @brief Backup msbticks register
  */
#define RTC_BKP_MSBTICKS   RTC_BKP_DR2

/* #define RTIF_DEBUG */

/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/**
  * @brief Timer driver callbacks handler
  */
const UTIL_TIMER_Driver_s UTIL_TimerDriver =
{
  TIMER_IF_Init,
  NULL,

  TIMER_IF_StartTimer,
  TIMER_IF_StopTimer,

  TIMER_IF_SetTimerContext,
  TIMER_IF_GetTimerContext,

  TIMER_IF_GetTimerElapsedTime,
  TIMER_IF_GetTimerValue,
  TIMER_IF_GetMinimumTimeout,

  TIMER_IF_Convert_ms2Tick,
  TIMER_IF_Convert_Tick2ms,
};

/**
  * @brief SysTime driver callbacks handler
  */
const UTIL_SYSTIM_Driver_s UTIL_SYSTIMDriver =
{
  TIMER_IF_BkUp_Write_Seconds,
  TIMER_IF_BkUp_Read_Seconds,
  TIMER_IF_BkUp_Write_SubSeconds,
  TIMER_IF_BkUp_Read_SubSeconds,
  TIMER_IF_GetTime,
};

/* External variables --------------------------------------------------------*/
/**
  * @brief RTC handle
  */
extern RTC_HandleTypeDef hrtc;

/* Private variables ---------------------------------------------------------*/
/**
  * @brief Indicates if the RTC is already Initialized or not
  */
static bool RTC_Initialized = 0;

/**
  * @brief RtcTimerContext
  */
static uint32_t RtcTimerContext = 0;

/* Exported macro ------------------------------------------------------------*/
#ifdef RTIF_DEBUG
#include "sys_app.h" /*for app_log*/
/**
  * @brief Post the RTC log string format to the circular queue for printing in using the polling mode
  */
#define TIMER_IF_DBG_PRINTF(...) do{ {UTIL_ADV_TRACE_COND_FSend(VLEVEL_ALWAYS, T_REG_OFF, TS_OFF, __VA_ARGS__);} }while(0);
#else
/**
  * @brief not used
  */
#define TIMER_IF_DBG_PRINTF(...)
#endif /* RTIF_DEBUG */

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief Get rtc timer Value in rtc tick
  * @return val the rtc timer value (upcounting)
  */
static inline uint32_t GetTimerTicks(void);

/**
  * @brief Writes MSBticks to backup register
  * Absolute RTC time in tick is (MSBticks)<<32 + (32bits binary counter)
  * @note MSBticks incremented every time the 32bits RTC timer wraps around (~44days)
  * @param[in] MSBticks
  * @return none
  */
static void TIMER_IF_BkUp_Write_MSBticks(uint32_t MSBticks);

/**
  * @brief Reads MSBticks from backup register
  * Absolute RTC time in tick is (MSBticks)<<32 + (32bits binary counter)
  * @note MSBticks incremented every time the 32bits RTC timer wraps around (~44days)
  * @retval MSBticks
  */
static uint32_t TIMER_IF_BkUp_Read_MSBticks(void);

/* Private user code ---------------------------------------------------------*/

UTIL_TIMER_Status_t TIMER_IF_Init(void)
{
  UTIL_TIMER_Status_t ret = UTIL_TIMER_OK;

  if (RTC_Initialized == false)
  {
    /* Stop Timer */
    TIMER_IF_StopTimer();

    /* DeActivate the Alarm A enabled by MX during MX_RTC_Init() */
    HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);

    /* Enable Direct Read of the calendar registers (not through Shadow) */
    HAL_RTCEx_EnableBypassShadow(&hrtc);

    /* Initialise MSB ticks */
    TIMER_IF_BkUp_Write_MSBticks(0);

    TIMER_IF_SetTimerContext();

    RTC_Initialized = true;
  }

  return ret;
}

UTIL_TIMER_Status_t TIMER_IF_StartTimer(uint32_t timeout)
{
  UTIL_TIMER_Status_t ret = UTIL_TIMER_OK;

  RTC_AlarmTypeDef sAlarm = {0};

  /* Stop timer if one is already started */
  TIMER_IF_StopTimer();
  timeout += RtcTimerContext;

  TIMER_IF_DBG_PRINTF("Start timer: time=%d, alarm=%d\n\r",  GetTimerTicks(), timeout);

  /* Starts timer */
  sAlarm.BinaryAutoClr        = RTC_ALARMSUBSECONDBIN_AUTOCLR_NO;
  sAlarm.AlarmTime.SubSeconds = UINT32_MAX - timeout;
  sAlarm.AlarmMask            = RTC_ALARMMASK_NONE;
  sAlarm.AlarmSubSecondMask   = RTC_ALARMSUBSECONDBINMASK_NONE;
  sAlarm.Alarm                = RTC_ALARM_A;

  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
  {
    /* Initialization Error */
    while(1);
  }

  return ret;
}

UTIL_TIMER_Status_t TIMER_IF_StopTimer(void)
{
  UTIL_TIMER_Status_t ret = UTIL_TIMER_OK;

  /* Clear RTC Alarm Flag */
  __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);

  /* Disable the Alarm A interrupt */
  HAL_RTC_DeactivateAlarm(&hrtc, RTC_ALARM_A);

  return ret;
}

uint32_t TIMER_IF_SetTimerContext(void)
{
  /* Store time context */
  RtcTimerContext = GetTimerTicks();

  TIMER_IF_DBG_PRINTF("TIMER_IF_SetTimerContext=%d\n\r", RtcTimerContext);

  /* Return time context */
  return RtcTimerContext;
}

uint32_t TIMER_IF_GetTimerContext(void)
{
  TIMER_IF_DBG_PRINTF("TIMER_IF_GetTimerContext=%d\n\r", RtcTimerContext);

  /* Return time context */
  return RtcTimerContext;
}

uint32_t TIMER_IF_GetTimerElapsedTime(void)
{
  return ((uint32_t)(GetTimerTicks() - RtcTimerContext));
}

uint32_t TIMER_IF_GetTimerValue(void)
{
  if (RTC_Initialized == true)
  {
    return GetTimerTicks();
  }
  else
  {
    return 0;
  }
}

uint32_t TIMER_IF_GetMinimumTimeout(void)
{
  return (MIN_ALARM_DELAY);
}

uint32_t TIMER_IF_Convert_ms2Tick(uint32_t timeMilliSec)
{
  return ((uint32_t)( ( ( (uint64_t) timeMilliSec ) << RTC_N_PREDIV_S ) / 1000U ) );
}

uint32_t TIMER_IF_Convert_Tick2ms(uint32_t tick)
{
  return ((uint32_t)( ( ( ( int64_t)( tick ) ) * 1000U ) >> RTC_N_PREDIV_S ) );
}

void TIMER_IF_DelayMs(uint32_t delay)
{
  uint32_t delayTicks = TIMER_IF_Convert_ms2Tick(delay);
  uint32_t timeout = GetTimerTicks();

  /* Wait delay ms */
  while (((GetTimerTicks() - timeout)) < delayTicks)
  {
    __NOP();
  }
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
  UTIL_TIMER_IRQ_Handler();
}

void HAL_RTCEx_SSRUEventCallback(RTC_HandleTypeDef *hrtc)
{
  /* Called every 48 days with 1024 ticks per seconds */
  TIMER_IF_DBG_PRINTF(">>Handler SSRUnderflow at %d\n\r", GetTimerTicks());

  /* Increment MSBticks */
  uint32_t MSB_ticks = TIMER_IF_BkUp_Read_MSBticks();
  TIMER_IF_BkUp_Write_MSBticks(MSB_ticks + 1);
}

uint32_t TIMER_IF_GetTime(uint16_t *mSeconds)
{
  uint64_t ticks;
  uint32_t timerValueLsb = GetTimerTicks();
  uint32_t timerValueMSB = TIMER_IF_BkUp_Read_MSBticks();

  ticks = (((uint64_t) timerValueMSB) << 32) + timerValueLsb;

  uint32_t seconds = (uint32_t)(ticks >> RTC_N_PREDIV_S);

  ticks = (uint32_t) ticks & RTC_PREDIV_S;

  *mSeconds = TIMER_IF_Convert_Tick2ms(ticks);

  return seconds;
}

void TIMER_IF_BkUp_Write_Seconds(uint32_t Seconds)
{
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_SECONDS, Seconds);
}

void TIMER_IF_BkUp_Write_SubSeconds(uint32_t SubSeconds)
{
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_SUBSECONDS, SubSeconds);
}

uint32_t TIMER_IF_BkUp_Read_Seconds(void)
{
  return HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_SECONDS);
}

uint32_t TIMER_IF_BkUp_Read_SubSeconds(void)
{
  return HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_SUBSECONDS);
}

static void TIMER_IF_BkUp_Write_MSBticks(uint32_t MSBticks)
{
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_MSBTICKS, MSBticks);
}

static uint32_t TIMER_IF_BkUp_Read_MSBticks(void)
{
  uint32_t MSBticks;
  MSBticks = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_MSBTICKS);
  return MSBticks;
}

static inline uint32_t GetTimerTicks(void)
{
  return (UINT32_MAX - LL_RTC_TIME_GetSubSecond(RTC));
}
