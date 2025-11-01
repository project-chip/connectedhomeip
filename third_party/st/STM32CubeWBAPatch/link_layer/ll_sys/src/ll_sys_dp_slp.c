/**
  ******************************************************************************
  * @file    ll_sys_dp_slp.c
  * @author  MCD Application Team
  * @brief   Link Layer IP system interface deep sleep management
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

#include "linklayer_plat.h"
#include "ll_sys.h"
#include "ll_intf_cmn.h"

/* Link Layer deep sleep timer */
static os_timer_id radio_dp_slp_tmr_id = NULL;

/* Link Layer deep sleep state */
static ll_sys_dp_slp_state_t linklayer_dp_slp_state = LL_SYS_DP_SLP_DISABLED;

/**
  * @brief  Initialize resources to handle deep sleep entry/exit
  * @param  None
  * @retval LL_SYS status
  */
ll_sys_status_t ll_sys_dp_slp_init(void)
{
  ll_sys_status_t return_status = LL_SYS_ERROR;

  /* Create link layer timer for handling IP DEEP SLEEP mode */
  radio_dp_slp_tmr_id = os_timer_create((t_timer_callbk)ll_sys_dp_slp_wakeup_evt_clbk, os_timer_once, NULL);

  /* Set priority of deep sleep timer */
  os_timer_set_prio(radio_dp_slp_tmr_id, hg_prio_tmr);

  if (radio_dp_slp_tmr_id != NULL)
  {
    return_status = LL_SYS_OK;
  }

  return return_status;
}

/**
  * @brief  Link Layer deep sleep status getter
  * @param  None
  * @retval Link Layer deep sleep state
  */
ll_sys_dp_slp_state_t ll_sys_dp_slp_get_state(void)
{
  return linklayer_dp_slp_state;
}

/**
  * @brief  The Link Layer IP enters deep sleep mode
  * @param  dp_slp_duration    deep sleep duration in us
  * @retval LL_SYS status
  */
ll_sys_status_t ll_sys_dp_slp_enter(uint32_t dp_slp_duration){
  ble_stat_t cmd_status;
  int32_t os_status = GENERAL_FAILURE;
  ll_sys_status_t return_status = LL_SYS_ERROR;

  /* Check if deep sleep timer has to be started */
  if(dp_slp_duration < LL_DP_SLP_NO_WAKEUP)
  {
    /* Start deep sleep timer */
    os_status = os_timer_start(radio_dp_slp_tmr_id, LL_INTERNAL_TMR_US_TO_STEPS(dp_slp_duration));
  }

  else
  {
    /* No timer started */
    os_status = SUCCESS;
  }

  if(os_status == SUCCESS)
  {
    /* Switch Link Layer IP to DEEP SLEEP mode */
    cmd_status = ll_intf_cmn_le_set_dp_slp_mode(DEEP_SLEEP_ENABLE);
    if(cmd_status == SUCCESS){
      linklayer_dp_slp_state = LL_SYS_DP_SLP_ENABLED;
      return_status = LL_SYS_OK;
    }
  }

  return return_status;
}

/**
  * @brief  The Link Layer IP exits deep sleep mode
  * @param  None
  * @retval LL_SYS status
  */
ll_sys_status_t ll_sys_dp_slp_exit(void){
  ble_stat_t cmd_status;
  ll_sys_status_t return_status = LL_SYS_ERROR;

  /* Disable radio interrupt */
  LINKLAYER_PLAT_DisableRadioIT();

  if(linklayer_dp_slp_state == LL_SYS_DP_SLP_DISABLED)
  {
    /* Radio not in sleep mode */
    return_status = LL_SYS_OK;
  }
  else
  {
    /* Switch Link Layer IP to SLEEP mode (by deactivate DEEP SLEEP mode) */
    cmd_status = ll_intf_cmn_le_set_dp_slp_mode(DEEP_SLEEP_DISABLE);
    if(cmd_status == SUCCESS)
    {
      linklayer_dp_slp_state = LL_SYS_DP_SLP_DISABLED;
      return_status = LL_SYS_OK;
    }

    /* Stop the deep sleep wake-up timer if running */
    if(os_get_tmr_state(radio_dp_slp_tmr_id) != (os_timer_state)osTimerStopped)
    {
      os_timer_stop(radio_dp_slp_tmr_id);
    }
  }

  /* Re-enable radio interrupt */
  LINKLAYER_PLAT_EnableRadioIT();

  return return_status;
}

/**
  * @brief  Link Layer deep sleep wake-up timer callback
  * @param  ptr_arg    pointer passed through the callback
  * @retval LL_SYS status
  */
void ll_sys_dp_slp_wakeup_evt_clbk(void const *ptr_arg){

  /* Link Layer IP exits from DEEP SLEEP mode */
  ll_sys_dp_slp_exit();

}
