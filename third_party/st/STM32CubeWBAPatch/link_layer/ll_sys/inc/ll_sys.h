/**
  ******************************************************************************
  * @file    ll_sys.h
  * @author  MCD Application Team
  * @brief   Header for Link Layer system interface module
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

#ifndef LL_SYS_H
#define LL_SYS_H

#include "ll_intf.h"
#include "hci.h"
#include "cmsis_compiler.h"
#include <stdint.h>

#define LL_DP_SLP_NO_WAKEUP                 ((uint32_t) ~(0) )
#define LL_INTERNAL_TMR_US_TO_STEPS(US)     (((US)*4)/125)

/**
  * @brief  Link Layer Status structure definition
  */
typedef enum
{
  LL_SYS_OK       = 0x00,
  LL_SYS_ERROR    = 0x01,
  LL_SYS_BUSY     = 0x02,
} ll_sys_status_t;

/**
  * @brief  Link Layer radio bus clock clients definition
  */
typedef enum
{
  LL_SYS_RADIO_HCLK_RADIO_ISR,    /* Radio interrupt event */
  LL_SYS_RADIO_HCLK_LL_EVT,       /* Link Layer event depending on radio activity */
  LL_SYS_RADIO_HCLK_LL_BG,        /* Link Layer background task */
  LL_SYS_RADIO_HCLK_BLEHOST_BG,   /* BLE Host background task */
  LL_SYS_RADIO_HCLK_PREIDLE,      /* Link Layer PRE IDLE task */
} ll_sys_radio_hclk_client_t;

/**
  * @brief  Link Layer deep sleep state
  */
typedef enum
{
  LL_SYS_DP_SLP_DISABLED = 0x00,
  LL_SYS_DP_SLP_ENABLED,
} ll_sys_dp_slp_state_t;

/* Link Layer system interface general module functions  ************************************************/
void ll_sys_init(void);
void ll_sys_reset(void);
void ll_sys_delay_us(uint32_t delay);
void ll_sys_assert(uint8_t condition);
void ll_sys_get_rng(uint8_t *ptr_rnd, uint32_t len);
void ll_sys_radio_ack_ctrl(uint8_t enable);
void ll_sys_radio_wait_for_busclkrdy(void);
void ll_sys_setup_radio_intr(void (*intr_cb)());
void ll_sys_setup_radio_sw_low_intr(void (*intr_cb)());
void ll_sys_radio_sw_low_intr_trigger(uint8_t priority);
void ll_sys_radio_evt_not(uint8_t start);
void ll_sys_rco_clbr_not(uint8_t start);
void ll_sys_request_temperature(void);
void ll_sys_schldr_timing_update_not(Evnt_timing_t * p_evnt_timing);

extern int ll_intf_is_ptr_in_ble_mem(void* inp_ptr);
void HostStack_Process(void);

/* Link Layer system interface synchronisation module functions  ************************************************/
void ll_sys_bg_process(void);
void ll_sys_bg_process_init(void);
void ll_sys_schedule_bg_process(void);
void ll_sys_schedule_bg_process_isr(void);
void ll_sys_config_params(void);

/* Link Layer system interface critical sections module functions  ************************************************/
void ll_sys_enable_irq(void);
void ll_sys_disable_irq(void);
void ll_sys_enable_specific_irq(uint8_t isr_type);
void ll_sys_disable_specific_irq(uint8_t isr_type);
void ll_sys_enable_os_context_switch(void);
void ll_sys_disable_os_context_switch(void);

/* Link Layer system interface deep sleep module functions  ************************************************/
ll_sys_status_t ll_sys_dp_slp_init(void);
ll_sys_status_t ll_sys_dp_slp_enter(uint32_t dp_slp_duration);
ll_sys_status_t ll_sys_dp_slp_exit(void);
ll_sys_dp_slp_state_t ll_sys_dp_slp_get_state(void);
void ll_sys_dp_slp_wakeup_evt_clbk(void const *ptr_arg);

/**
  * @brief  Get the number of concurrent state machines for the Link Layer
  * @param  None
  * @retval Supported number of concurrent state machines
  */
uint8_t ll_sys_get_concurrent_state_machines_num(void);

#if BLE
/**
  * @brief  Updating Link Layer BLE timings
  * @param  drift_time[in]: number of Link Layer sleep timer cycles (1 cycle = 31us) for the DRIFT TIME timing.
  * @param  exec_time[in]: number of Link Layer sleep timer cycles (1 cycle = 31us)  for the EXEC TIME timing.
  * @note   This interface needs to be called after system initialization
  *         and before starting any radio activity.
  * @retval None
  */
void ll_sys_config_BLE_schldr_timings(uint8_t drift_time, uint8_t exec_time);
#endif /* BLE */

uint32_t ll_intf_cmn_get_slptmr_value(void);

#endif /* LL_SYS_H */
