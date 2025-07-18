/**
  ******************************************************************************
  * @file    ll_sys_cs.c
  * @author  MCD Application Team
  * @brief   Link Layer IP system interface critical sections management
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
#include <stdint.h>

/**
  * @brief  Enable interrupts
  * @param  None
  * @retval None
  */
void ll_sys_enable_irq(void) {
  LINKLAYER_PLAT_EnableIRQ();
}

/**
  * @brief  Disable interrupts
  * @param  None
  * @retval None
  */
void ll_sys_disable_irq(void) {
  LINKLAYER_PLAT_DisableIRQ();
}

/**
  * @brief  Set the Current Interrupt Priority Mask. 
  *         All interrupts with low priority level will be masked.
  * @param  None
  * @retval None
  */
void ll_sys_enable_specific_irq(uint8_t isr_type)
{
  LINKLAYER_PLAT_EnableSpecificIRQ(isr_type);
}

/**
  * @brief  Restore the previous interrupt priority level
  * @param  None
  * @retval None
  */
void ll_sys_disable_specific_irq(uint8_t isr_type)
{
  LINKLAYER_PLAT_DisableSpecificIRQ(isr_type);
}

void ll_sys_enable_os_context_switch(void)
{
  LINKLAYER_PLAT_EnableOSContextSwitch();
}

void ll_sys_disable_os_context_switch(void)
{
  LINKLAYER_PLAT_DisableOSContextSwitch();
}