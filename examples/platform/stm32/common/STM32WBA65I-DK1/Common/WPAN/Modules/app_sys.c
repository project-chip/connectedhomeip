/**
  ******************************************************************************
  * @file    app_sys.c
  * @author  MCD Application Team
  * @brief   Application system for STM32WPAN Middleware.
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

#include "app_sys.h"
#include "app_conf.h"
#include "timer_if.h"
#include "stm32_lpm.h"
#include "ll_intf.h"
#include "ll_sys.h"
#if (UTIL_LPM_LEGACY_ENABLED == 0)  
#include "stm32_lpm_if.h"
#endif

#if MAC
#include "ral.h"
#endif 

/* External functions --------------------------------------------------------*/
extern uint32_t             llhwc_cmn_is_dp_slp_enabled(void);

/* External variables --------------------------------------------------------*/

/* Private variables  --------------------------------------------------------*/
static uint32_t wakeup_offset = RADIO_DEEPSLEEP_WAKEUP_TIME_US;

/* Functions Definition ------------------------------------------------------*/
/**
 *
 */
#if SUPPORT_BLE
void APP_SYS_BLE_EnterDeepSleep(void)
{
  ble_stat_t cmd_status;
  uint32_t radio_remaining_time = 0;

  if (ll_sys_dp_slp_get_state() == LL_SYS_DP_SLP_DISABLED)
  {
    /* Enable radio to retrieve next radio activity */

    /* Getting next radio event time if any */
    cmd_status = ll_intf_le_get_remaining_time_for_next_event(&radio_remaining_time);
    UNUSED(cmd_status);
    assert_param(cmd_status == SUCCESS);

    if (radio_remaining_time == LL_DP_SLP_NO_WAKEUP)
    {
      /* No next radio event scheduled */
      (void)ll_sys_dp_slp_enter(LL_DP_SLP_NO_WAKEUP);
    }
    else if (radio_remaining_time > wakeup_offset)
    {
      /* No event in a "near" futur */
      (void)ll_sys_dp_slp_enter(radio_remaining_time - wakeup_offset);
    }
    else
    {
#if (UTIL_LPM_LEGACY_ENABLED == 1)      
      UTIL_LPM_SetOffMode(1U << CFG_LPM_LL_DEEPSLEEP, UTIL_LPM_DISABLE);
#else /*  (UTIL_LPM_LEGACY_ENABLED == 1) */
#if (CFG_LPM_STOP1_SUPPORTED == 1)
      UTIL_LPM_SetMaxMode(1U << CFG_LPM_LL_DEEPSLEEP, UTIL_LPM_STOP1_MODE);
#else /* (CFG_LPM_STOP1_SUPPORTED == 1) */
      UTIL_LPM_SetMaxMode(1U << CFG_LPM_LL_DEEPSLEEP, UTIL_LPM_SLEEP_MODE);
#endif /* (CFG_LPM_STOP1_SUPPORTED == 1) */
#endif /*  (UTIL_LPM_LEGACY_ENABLED == 1) */      
    }
  }
}

#else /* SUPPORT_BLE */

/**
 *
 */
void APP_SYS_LPM_EnterLowPowerMode(void)
{
  ral_instance_t radio_instance;
  uint8_t channel;
  uint64_t next_radio_evt;

  /* Ensure there are no radio events (implement bsp function like bsp_is_radio_idle() ) */
  ral_event_state_enum_t radio_state = ral_get_current_event_state( &radio_instance, &channel );
  LL_UNUSED(radio_instance);
  LL_UNUSED(channel);
  if (radio_state != RAL_IDLE)
  {
    return;
  }

  next_radio_evt = os_timer_get_earliest_time();
  if ( llhwc_cmn_is_dp_slp_enabled() == 0 )
  {
    if ( next_radio_evt > wakeup_offset )
    {
      /* No event in a "near" futur */
      ll_sys_dp_slp_enter( next_radio_evt - wakeup_offset );
    }
  }
}

#endif /* SUPPORT_BLE */

void APP_SYS_SetWakeupOffset(uint32_t wakeup_offset_us)
{
  wakeup_offset = wakeup_offset_us;
}
