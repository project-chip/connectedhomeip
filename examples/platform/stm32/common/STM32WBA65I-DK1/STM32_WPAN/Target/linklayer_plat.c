/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    linklayer_plat.c
  * @author  MCD Application Team
  * @brief   Source file for the linklayer plateform adaptation layer
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_common.h"
#include "stm32wbaxx_hal.h"
#include "linklayer_plat.h"
#include "stm32wbaxx_hal_conf.h"
#include "stm32wbaxx_ll_rcc.h"
#include "app_conf.h"
#include "scm.h"
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
#include "adc_ctrl.h"
#endif /* (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1) */

#include "stm32_lpm.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

#define max(a,b) ((a) > (b) ? a : b)

/* 2.4GHz RADIO ISR callbacks */
void (*radio_callback)(void) = NULL;
void (*low_isr_callback)(void) = NULL;

/* RNG handle */
extern RNG_HandleTypeDef hrng;

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
/* Link Layer temperature request from background */
extern void ll_sys_bg_temperature_measurement(void);
#endif /* (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1) */

/* Radio critical sections */
static uint32_t primask_bit = 0;
volatile int32_t prio_high_isr_counter = 0;
volatile int32_t prio_low_isr_counter = 0;
volatile int32_t prio_sys_isr_counter = 0;
volatile int32_t irq_counter = 0;
volatile uint32_t local_basepri_value = 0;

/* Radio SW low ISR global variable */
volatile uint8_t radio_sw_low_isr_is_running_high_prio = 0;

/* Radio bus clock control variables */
uint8_t AHB5_SwitchedOff = 0;
uint32_t radio_sleep_timer_val = 0;

/**
  * @brief  Configure the necessary clock sources for the radio.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_ClockInit()
{
  uint32_t linklayer_slp_clk_src = LL_RCC_RADIOSLEEPSOURCE_NONE;

  /* Get the Link Layer sleep timer clock source */
  linklayer_slp_clk_src = LL_RCC_RADIO_GetSleepTimerClockSource();
  if(linklayer_slp_clk_src == LL_RCC_RADIOSLEEPSOURCE_NONE)
  {
    /* If there is no clock source defined, should be selected before */
    assert_param(0);
  }

  /* Enable AHB5ENR peripheral clock (bus CLK) */
  __HAL_RCC_RADIO_CLK_ENABLE();
}

/**
  * @brief  Link Layer active waiting loop.
  * @param  delay: delay in us
  * @retval None
  */
void LINKLAYER_PLAT_DelayUs(uint32_t delay)
{
__IO register uint32_t Delay = delay * (SystemCoreClock / 1000000U);
	do
	{
		__NOP();
	}
	while (Delay --);
}

/**
  * @brief  Link Layer assertion API
  * @param  condition: conditional statement to be checked.
  * @retval None
  */
void LINKLAYER_PLAT_Assert(uint8_t condition)
{
  assert_param(condition);
}

/**
  * @brief  Enable/disable the Link Layer active clock (baseband clock).
  * @param  enable: boolean value to enable (1) or disable (0) the clock.
  * @retval None
  */
void LINKLAYER_PLAT_WaitHclkRdy(void)
{
  /* Wait on radio bus clock readiness if it has been turned of */
  if (AHB5_SwitchedOff == 1)
  {
    AHB5_SwitchedOff = 0;
    while (radio_sleep_timer_val == ll_intf_cmn_get_slptmr_value());
  }
}

/**
  * @brief  Active wait on bus clock readiness.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_AclkCtrl(uint8_t enable)
{
  if(enable != 0u)
  {
#if (CFG_SCM_SUPPORTED == 1)
    /* SCM HSE BEGIN */
    /* Polling on HSE32 activation */
    SCM_HSE_WaitUntilReady();
    /* Enable RADIO baseband clock (active CLK) */
    HAL_RCCEx_EnableRadioBBClock();
	/* SCM HSE END */
#else
    /* Enable RADIO baseband clock (active CLK) */
    HAL_RCCEx_EnableRadioBBClock();
    /* Polling on HSE32 activation */
    while ( LL_RCC_HSE_IsReady() == 0);
#endif /* CFG_SCM_SUPPORTED */
  }
  else
  {
    /* Disable RADIO baseband clock (active CLK) */
    HAL_RCCEx_DisableRadioBBClock();
  }
}

/**
  * @brief  Link Layer RNG request.
  * @param  ptr_rnd: pointer to the variable that hosts the number.
  * @param  len: number of byte of anthropy to get.
  * @retval None
  */
void LINKLAYER_PLAT_GetRNG(uint8_t *ptr_rnd, uint32_t len)
{
  uint32_t nb_remaining_rng = len;
  uint32_t generated_rng;

  /* Get the requested RNGs (4 bytes by 4bytes) */
  while(nb_remaining_rng >= 4)
  {
    generated_rng = 0;
    HW_RNG_Get(1, &generated_rng);
    memcpy((ptr_rnd+(len-nb_remaining_rng)), &generated_rng, 4);
    nb_remaining_rng -=4;
  }

  /* Get the remaining number of RNGs */
  if(nb_remaining_rng>0){
    generated_rng = 0;
    HW_RNG_Get(1, &generated_rng);
    memcpy((ptr_rnd+(len-nb_remaining_rng)), &generated_rng, nb_remaining_rng);
  }
}

/**
  * @brief  Initialize Link Layer radio high priority interrupt.
  * @param  intr_cb: function pointer to assign for the radio high priority ISR routine.
  * @retval None
  */
void LINKLAYER_PLAT_SetupRadioIT(void (*intr_cb)())
{
  radio_callback = intr_cb;
  HAL_NVIC_SetPriority((IRQn_Type) RADIO_INTR_NUM, RADIO_INTR_PRIO_HIGH, 0);
  HAL_NVIC_EnableIRQ((IRQn_Type) RADIO_INTR_NUM);
}

/**
  * @brief  Initialize Link Layer SW low priority interrupt.
  * @param  intr_cb: function pointer to assign for the SW low priority ISR routine.
  * @retval None
  */
void LINKLAYER_PLAT_SetupSwLowIT(void (*intr_cb)())
{
  low_isr_callback = intr_cb;

  HAL_NVIC_SetPriority((IRQn_Type) RADIO_SW_LOW_INTR_NUM, RADIO_SW_LOW_INTR_PRIO, 0);
  HAL_NVIC_EnableIRQ((IRQn_Type) RADIO_SW_LOW_INTR_NUM);
}

/**
  * @brief  Trigger the link layer SW low interrupt.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_TriggerSwLowIT(uint8_t priority)
{
  uint8_t low_isr_priority = RADIO_INTR_PRIO_LOW;

  /* Check if a SW low interrupt as already been raised.
   * Nested call far radio low isr are not supported
   **/

  if(NVIC_GetActive(RADIO_SW_LOW_INTR_NUM) == 0)
  {
    /* No nested SW low ISR, default behavior */

    if(priority == 0)
    {
      low_isr_priority = RADIO_SW_LOW_INTR_PRIO;
    }

    HAL_NVIC_SetPriority((IRQn_Type) RADIO_SW_LOW_INTR_NUM, low_isr_priority, 0);
  }
  else
  {
    /* Nested call detected */
    /* No change for SW radio low interrupt priority for the moment */

    if(priority != 0)
    {
      /* At the end of current SW radio low ISR, this pending SW low interrupt
       * will run with RADIO_INTR_PRIO_LOW priority
       **/
      radio_sw_low_isr_is_running_high_prio = 1;
    }
  }

  HAL_NVIC_SetPendingIRQ((IRQn_Type) RADIO_SW_LOW_INTR_NUM);
}

/**
  * @brief  Enable interrupts.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_EnableIRQ(void)
{
  irq_counter = max(0,irq_counter-1);

  if(irq_counter == 0)
  {
    /* When irq_counter reaches 0, restore primask bit */
    __set_PRIMASK(primask_bit);
  }
}

/**
  * @brief  Disable interrupts.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_DisableIRQ(void)
{
  if(irq_counter == 0)
  {
    /* Save primask bit at first interrupt disablement */
    primask_bit= __get_PRIMASK();
  }
  __disable_irq();
  irq_counter ++;
}

/**
  * @brief  Enable specific interrupt group.
  * @param  isr_type: mask for interrupt group to enable.
  *         This parameter can be one of the following:
  *         @arg LL_HIGH_ISR_ONLY: enable link layer high priority ISR.
  *         @arg LL_LOW_ISR_ONLY: enable link layer SW low priority ISR.
  *         @arg SYS_LOW_ISR: mask interrupts for all the other system ISR with
  *              lower priority that link layer SW low interrupt.
  * @retval None
  */
void LINKLAYER_PLAT_EnableSpecificIRQ(uint8_t isr_type)
{
  if( (isr_type & LL_HIGH_ISR_ONLY) != 0 )
  {
    prio_high_isr_counter--;
    if(prio_high_isr_counter == 0)
    {
      /* When specific counter for link layer high ISR reaches 0, interrupt is enabled */
      HAL_NVIC_EnableIRQ(RADIO_INTR_NUM);
      /* USER CODE BEGIN LINKLAYER_PLAT_EnableSpecificIRQ_1*/

      /* USER CODE END LINKLAYER_PLAT_EnableSpecificIRQ_1*/
    }
  }

  if( (isr_type & LL_LOW_ISR_ONLY) != 0 )
  {
    prio_low_isr_counter--;
    if(prio_low_isr_counter == 0)
    {
      /* When specific counter for link layer SW low ISR reaches 0, interrupt is enabled */
      HAL_NVIC_EnableIRQ(RADIO_SW_LOW_INTR_NUM);
    }

  }

  if( (isr_type & SYS_LOW_ISR) != 0 )
  {
    prio_sys_isr_counter--;
    if(prio_sys_isr_counter == 0)
    {
      /* Restore basepri value */
      __set_BASEPRI(local_basepri_value);
    }
  }
}

/**
  * @brief  Disable specific interrupt group.
  * @param  isr_type: mask for interrupt group to disable.
  *         This parameter can be one of the following:
  *         @arg LL_HIGH_ISR_ONLY: disable link layer high priority ISR.
  *         @arg LL_LOW_ISR_ONLY: disable link layer SW low priority ISR.
  *         @arg SYS_LOW_ISR: unmask interrupts for all the other system ISR with
  *              lower priority that link layer SW low interrupt.
  * @retval None
  */
void LINKLAYER_PLAT_DisableSpecificIRQ(uint8_t isr_type)
{
  if( (isr_type & LL_HIGH_ISR_ONLY) != 0 )
  {
    prio_high_isr_counter++;
    if(prio_high_isr_counter == 1)
    {
      /* USER CODE BEGIN LINKLAYER_PLAT_DisableSpecificIRQ_1*/

      /* USER CODE END LINKLAYER_PLAT_DisableSpecificIRQ_1*/
      /* When specific counter for link layer high ISR value is 1, interrupt is disabled */
      HAL_NVIC_DisableIRQ(RADIO_INTR_NUM);
    }
  }

  if( (isr_type & LL_LOW_ISR_ONLY) != 0 )
  {
    prio_low_isr_counter++;
    if(prio_low_isr_counter == 1)
    {
      /* When specific counter for link layer SW low ISR value is 1, interrupt is disabled */
      HAL_NVIC_DisableIRQ(RADIO_SW_LOW_INTR_NUM);
    }
  }

  if( (isr_type & SYS_LOW_ISR) != 0 )
  {
    prio_sys_isr_counter++;
    if(prio_sys_isr_counter == 1)
    {
      /* Save basepri register value */
      local_basepri_value = __get_BASEPRI();

      /* Mask all other interrupts with lower priority that link layer SW low ISR */
      __set_BASEPRI_MAX(RADIO_INTR_PRIO_LOW<<4);
    }
  }
}

/**
  * @brief  Enable link layer high priority ISR only.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_EnableRadioIT(void)
{
  /* USER CODE BEGIN LINKLAYER_PLAT_EnableRadioIT_1*/

  /* USER CODE END LINKLAYER_PLAT_EnableRadioIT_1*/

  HAL_NVIC_EnableIRQ((IRQn_Type) RADIO_INTR_NUM);

  /* USER CODE BEGIN LINKLAYER_PLAT_EnableRadioIT_2*/

  /* USER CODE END LINKLAYER_PLAT_EnableRadioIT_2*/
}

/**
  * @brief  Disable link layer high priority ISR only.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_DisableRadioIT(void)
{
  /* USER CODE BEGIN LINKLAYER_PLAT_DisableRadioIT_1*/

  /* USER CODE END LINKLAYER_PLAT_DisableRadioIT_1*/

  HAL_NVIC_DisableIRQ((IRQn_Type) RADIO_INTR_NUM);

  /* USER CODE BEGIN LINKLAYER_PLAT_DisableRadioIT_2*/

  /* USER CODE END LINKLAYER_PLAT_DisableRadioIT_2*/
}

/**
  * @brief  Link Layer notification for radio activity start.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_StartRadioEvt(void)
{
  __HAL_RCC_RADIO_CLK_SLEEP_ENABLE();
  NVIC_SetPriority(RADIO_INTR_NUM, RADIO_INTR_PRIO_HIGH);
#if (CFG_SCM_SUPPORTED == 1)
  scm_notifyradiostate(SCM_RADIO_ACTIVE);
#endif /* CFG_SCM_SUPPORTED */
}

/**
  * @brief  Link Layer notification for radio activity end.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_StopRadioEvt(void)
{
  __HAL_RCC_RADIO_CLK_SLEEP_DISABLE();
  NVIC_SetPriority(RADIO_INTR_NUM, RADIO_INTR_PRIO_LOW);
#if (CFG_SCM_SUPPORTED == 1)
  scm_notifyradiostate(SCM_RADIO_NOT_ACTIVE);
#endif /* CFG_SCM_SUPPORTED */
}

/**
  * @brief  Link Layer notification for RCO calibration start.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_RCOStartClbr(void)
{
#if (CFG_SCM_SUPPORTED == 1)
#if (CFG_LPM_LEVEL != 0)
#if (CFG_LPM_STDBY_SUPPORTED == 1)
  UTIL_LPM_SetOffMode(1U << CFG_LPM_APP, UTIL_LPM_DISABLE);
#endif /* (CFG_LPM_STDBY_SUPPORTED == 1) */
  UTIL_LPM_SetStopMode(1U << CFG_LPM_APP, UTIL_LPM_DISABLE);
#endif /* (CFG_LPM_LEVEL != 0) */
  scm_setsystemclock(SCM_USER_LL_HW_RCO_CLBR, HSE_32MHZ); 
  while (LL_PWR_IsActiveFlag_VOS() == 0);
#endif /* CFG_SCM_SUPPORTED */
}

/**
  * @brief  Link Layer notification for RCO calibration end.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_RCOStopClbr(void)
{
#if (CFG_SCM_SUPPORTED == 1)
#if (CFG_LPM_LEVEL != 0)
#if (CFG_LPM_STDBY_SUPPORTED == 1)
  UTIL_LPM_SetOffMode(1U << CFG_LPM_APP, UTIL_LPM_ENABLE);
#endif /* (CFG_LPM_STDBY_SUPPORTED == 1) */
  UTIL_LPM_SetStopMode(1U << CFG_LPM_APP, UTIL_LPM_ENABLE);
#endif /* (CFG_LPM_LEVEL != 0) */
  scm_setsystemclock(SCM_USER_LL_HW_RCO_CLBR, HSE_16MHZ); 
  while (LL_PWR_IsActiveFlag_VOS() == 0);
#endif /* CFG_SCM_SUPPORTED */
}

/**
  * @brief  Link Layer requests temperature.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_RequestTemperature(void)
{
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
  ll_sys_bg_temperature_measurement();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
}

/**
  * @brief  Enable RTOS context switch.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_EnableOSContextSwitch(void)
{
}

/**
  * @brief  Disable RTOS context switch.
  * @param  None
  * @retval None
  */
void LINKLAYER_PLAT_DisableOSContextSwitch(void)
{
}

/**
 * @brief Notify the upper layer that new Link Layer timings have been applied.
 * @param evnt_timing[in]: Evnt_timing_t pointer to structure contains drift time , execution time and scheduling time
 * @retval None.
 */
void LINKLAYER_PLAT_SCHLDR_TIMING_UPDATE_NOT(Evnt_timing_t * p_evnt_timing)
{
}

/**
  * @brief  Get the ST company ID.
  * @param  None
  * @retval Company ID
  */
uint32_t LINKLAYER_PLAT_GetSTCompanyID(void)
{
  return LL_FLASH_GetSTCompanyID();
}

/**
  * @brief  Get the Unique Device Number (UDN).
  * @param  None
  * @retval UDN
  */
uint32_t LINKLAYER_PLAT_GetUDN(void)
{
  return LL_FLASH_GetUDN();
}
