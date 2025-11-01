/**
  ******************************************************************************
  * @file    linklayer_plat.h
  * @author  MCD Application Team
  * @brief   Header for linklayer_plat.c interface module
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

#ifndef LINKLAYER_PLAT_H
#define LINKLAYER_PLAT_H

#include "bsp.h"
#include <stdint.h>

/**
  * @brief  Configure the necessary clock sources for the radio.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_ClockInit(void);

/**
  * @brief  Link Layer active waiting loop.
  * @param  delay: delay in us
  * @retval None
  */
extern void LINKLAYER_PLAT_DelayUs(uint32_t delay);

/**
  * @brief  Link Layer assertion API
  * @param  condition: conditional statement to be checked.
  * @retval None
  */
extern void LINKLAYER_PLAT_Assert(uint8_t condition);

/**
  * @brief  Enable/disable the Link Layer active clock (baseband clock).
  * @param  enable: boolean value to enable (1) or disable (0) the clock.
  * @retval None
  */
extern void LINKLAYER_PLAT_AclkCtrl(uint8_t enable);

/**
  * @brief  Notify the Link Layer platform layer the system will enter in WFI
  *         and AHB5 clock may be turned of regarding the 2.4Ghz radio state.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_NotifyWFIEnter(void);

/**
  * @brief  Notify the Link Layer platform layer the system exited WFI and AHB5
  *         clock may be resynchronized as is may have been turned of during
  *         low power mode entry.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_NotifyWFIExit(void);


/**
  * @brief  Active wait on bus clock readiness.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_WaitHclkRdy(void);

/**
  * @brief  Link Layer RNG request.
  * @param  ptr_rnd: pointer to the variable that hosts the number.
  * @param  len: number of byte of anthropy to get.
  * @retval None
  */
extern void LINKLAYER_PLAT_GetRNG(uint8_t *ptr_rnd, uint32_t len);

/**
  * @brief  Initialize Link Layer radio high priority interrupt.
  * @param  intr_cb: function pointer to assign for the radio high priority ISR routine.
  * @retval None
  */
extern void LINKLAYER_PLAT_SetupRadioIT(void (*intr_cb)());

/**
  * @brief  Initialize Link Layer SW low priority interrupt.
  * @param  intr_cb: function pointer to assign for the SW low priority ISR routine.
  * @retval None
  */
extern void LINKLAYER_PLAT_SetupSwLowIT(void (*intr_cb)());

/**
  * @brief  Trigger the link layer SW low interrupt.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_TriggerSwLowIT(uint8_t priority);

/**
  * @brief  Enable interrupts.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_EnableIRQ(void);

/**
  * @brief  Disable interrupts.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_DisableIRQ(void);

/**
  * @brief  Enable specific interrupt group.
  * @param  isr_type: mask for interrupt group to enable.
  *         This parameter can be one of the following:
  *         @arg LL_HIGH_ISR_ONLY: enable link layer high priority ISR.
  *         @arg LL_LOW_ISR_ONLY: enable link layer SW low priority ISR.
  *         @arg SYS_LOW_ISR: unmask interrupts for all the other system ISR with
  *              lower priority that link layer SW low interrupt.
  * @retval None
  */
extern void LINKLAYER_PLAT_EnableSpecificIRQ(uint8_t isr_type);

/**
  * @brief  Disable specific interrupt group.
  * @param  isr_type: mask for interrupt group to disable.
  *         This parameter can be one of the following:
  *         @arg LL_HIGH_ISR_ONLY: disable link layer high priority ISR.
  *         @arg LL_LOW_ISR_ONLY: disable link layer SW low priority ISR.
  *         @arg SYS_LOW_ISR: mask interrupts for all the other system ISR with
  *              lower priority that link layer SW low interrupt.
  * @retval None
  */
extern void LINKLAYER_PLAT_DisableSpecificIRQ(uint8_t isr_type);

/**
  * @brief  Enable link layer high priority ISR only.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_EnableRadioIT(void);

/**
  * @brief  Disable link layer high priority ISR only.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_DisableRadioIT(void);

/**
  * @brief  Link Layer notification for radio activity start.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_StartRadioEvt(void);

/**
  * @brief  Link Layer notification for radio activity end.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_StopRadioEvt(void);

/**
  * @brief  Link Layer notification for RCO calibration start.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_RCOStartClbr(void);

/**
  * @brief  Link Layer notification for RCO calibration end.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_RCOStopClbr(void);

/**
  * @brief  Link Layer requests temperature.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_RequestTemperature(void);

/**
  * @brief  Enable RTOS context switch.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_EnableOSContextSwitch(void);

/**
  * @brief  Disable RTOS context switch.
  * @param  None
  * @retval None
  */
extern void LINKLAYER_PLAT_DisableOSContextSwitch(void);

/**
 * @brief Notify the upper layer that new Link Layer timings have been applied.
 * @param evnt_timing[in]: Evnt_timing_t pointer to structure contains drift time , execution time and scheduling time
 * @retval None.
 */
extern void LINKLAYER_PLAT_SCHLDR_TIMING_UPDATE_NOT(Evnt_timing_t * p_evnt_timing);

/**
  * @brief  Get the ST company ID.
  * @param  None
  * @retval Company ID
  */
extern uint32_t LINKLAYER_PLAT_GetSTCompanyID(void);

/**
  * @brief  Get the Unique Device Number (UDN).
  * @param  None
  * @retval UDN
  */
extern uint32_t LINKLAYER_PLAT_GetUDN(void);

#endif /* LINKLAYER_PLAT_H */
