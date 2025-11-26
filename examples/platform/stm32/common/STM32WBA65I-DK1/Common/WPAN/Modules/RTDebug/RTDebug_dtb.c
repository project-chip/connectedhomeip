/**
  ******************************************************************************
  * @file    RTDebug_dtb.c
  * @author  MCD Application Team
  * @brief   Real Time Debug module API definition for DTB usage
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

/**
 * The DEBUG DTB interface is INTENDED TO BE USED ONLY ON REQUEST FROM ST SUPPORT.
 * It provides HW signals from RF PHY activity.
 */

/* Includes ------------------------------------------------------------------*/
#include "RTDebug_dtb.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

#if(CFG_RT_DEBUG_DTB == 1)
/* Private function prototypes -----------------------------------------------*/
static void RT_DEBUG_SetDTBMode(uint8_t dtb, uint8_t mode);

/* Private functions ---------------------------------------------------------*/
static void RT_DEBUG_SetDTBMode(uint8_t dtb, uint8_t mode)
{
  /* Set SYSCFG for DTB */
  uint32_t addr = (uint32_t)(&SYSCFG->SECCFGR);
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  addr += 0x100; /* contains SYSCFG->DBTCR address */

  /* Keep others DTB in current modes */
  *(uint32_t*)addr |= (0xF<<(dtb*4)); /* Selected DTB unmask mode */
  *(uint32_t*)addr ^= (0xF<<(dtb*4)); /* Selected DTB unset mode */

  /* Selected DTB set to mode */
  *(uint32_t*)addr |= (mode<<(dtb*4));
}

void RT_DEBUG_DTBInit(void)
{
  /** access_match PA7 DTB[7] on TL_10 **/
/*
  LL_GPIO_SetPinSpeed(GPIOA,LL_GPIO_PIN_7,LL_GPIO_SPEED_FREQ_HIGH);
  LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_7,LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(GPIOA,LL_GPIO_PIN_7,LL_GPIO_AF_15);
  HAL_PWREx_EnableStandbyIORetention(PWR_GPIO_A, GPIO_PIN_7);
*/

  /** long_range_iq_vld PA6 DTB[6] on TL_9 **/
  LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_6,LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(GPIOA,LL_GPIO_PIN_6,LL_GPIO_AF_15);


  /** tx_data PA5 DTB[5] on TL_8 **/
/*
  LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_5,LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(GPIOA,LL_GPIO_PIN_5,LL_GPIO_AF_15);
*/

  /** rx_data PB9 DTB[4] on TL_31 **/
/*
  LL_GPIO_SetPinMode(GPIOB,LL_GPIO_PIN_9,LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_8_15(GPIOB,LL_GPIO_PIN_9,LL_GPIO_AF_15);
  HAL_PWREx_EnableStandbyIORetention(PWR_GPIO_B, GPIO_PIN_9);
*/

  /** tx_data_clk PB8 DTB[3] on TL_30 **/
/*
  LL_GPIO_SetPinMode(GPIOB,LL_GPIO_PIN_8,LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_8_15(GPIOB,LL_GPIO_PIN_8,LL_GPIO_AF_15);
*/

  /** rx_data_clk PA2 DTB[2] on TL_3 **/
/*
  LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_2,LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(GPIOA,LL_GPIO_PIN_2,LL_GPIO_AF_15);
  HAL_PWREx_EnableStandbyIORetention(PWR_GPIO_A, GPIO_PIN_2);
*/

  /** tx_on PA1 DTB[1] on TL_2 **/
  LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_1,LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(GPIOA,LL_GPIO_PIN_1,LL_GPIO_AF_15);
  HAL_PWREx_EnableStandbyIORetention(PWR_GPIO_A, GPIO_PIN_1);

  /** rx_on PA0 DTB[0] on TL_1 **/
/*
  LL_GPIO_SetPinMode(GPIOA,LL_GPIO_PIN_0,LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(GPIOA,LL_GPIO_PIN_0,LL_GPIO_AF_15);
  HAL_PWREx_EnableStandbyIORetention(PWR_GPIO_A, GPIO_PIN_0);
*/

  /* Set SYSCFG for DTB */
  uint32_t addr = (uint32_t)(&SYSCFG->SECCFGR);
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  addr += 0x100; /* contains SYSCFG->DBTCR address */

  /* Default DTB in test mode 0 */
  *(uint32_t*)addr = 0x00000000;
}

/* Public functions ----------------------------------------------------------*/
void RT_DEBUG_DTBConfig(void)
{

/*
 ** Monitor DTB **
 * A0  o PA0 AF15 DTB[0] mode 1 rx_on
 * A1  o PA1 AF15 DTB[1] mode 1 tx_on
 * A2  o PB9 AF15 DTB[4] mode 5 radio_bbclk
 * A3  o PA7 AF15 DTB[7] mode 3 slptmr_irq
 * A4  o PA2 AF15 DTB[2] mode 5 CM33_sleepdeep
 * A5  o PA6 AF00 PWR_CSTOP
 * A6  o PA8 as SYSclk source
 * A7  o PB1 Monitor wakeup GPIO
 * A8  o PB0 Debug PIN
 * A9  o PC0 o_pm_sleep_sw
 * A10 o PC1 o_pm_act_com_sw
 **/

  RT_DEBUG_SetDTBMode(6, 3) ; /* Radio IRQ */
  RT_DEBUG_SetDTBMode(1, 1); /* tx_on */

  //RT_DEBUG_SetDTBMode(0, 1); /* rx_on */
  //RT_DEBUG_SetDTBMode(1, 3); /* sleep timer irq */
  //RT_DEBUG_SetDTBMode(2, 5); /* CM33_sleepdeep */
  //RT_DEBUG_SetDTBMode(4, 5); /* radio_bbclk */
  //RT_DEBUG_SetDTBMode(7, 3); /* slptm_irq */
  //RT_DEBUG_SetDTBMode(7, 5); /* radio_bus_clk */
  //RT_DEBUG_SetDTBMode(7, 6); /* test tm 00 */
  //RT_DEBUG_SetDTBMode(6, 6); /* test tm 01 */
}

#endif /* CFG_RT_DEBUG_DTB */

