/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    peripheral_init.h
  * @author  MCD Application Team
  * @brief   Header for peripheral init module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef PERIPHERAL_INIT_H
#define PERIPHERAL_INIT_H

#if (CFG_LPM_STANDBY_SUPPORTED == 1)
/**
  * @brief  Configure the SoC peripherals at Standby mode exit.
  * @param  None
  * @retval None
  */
void MX_StandbyExit_PeripheralInit(void);
#endif
#if (CFG_LPM_STOP2_SUPPORTED == 1)
/**
  * @brief  Configure the SoC peripherals at Stop2 mode exit.
  * @param  None
  * @retval None
  */
void MX_Stop2Exit_PeripheralInit(void);
#endif

#endif /* PERIPHERAL_INIT_H */
