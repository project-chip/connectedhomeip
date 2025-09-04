/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    ble_timer.h
  * @author  MCD Application Team
  * @brief   This header defines the timer functions used by the BLE stack
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

#ifndef BLE_TIMER_H__
#define BLE_TIMER_H__

void BLE_TIMER_Init( void );

uint8_t BLE_TIMER_Start( uint16_t id,
                     uint32_t ms_timeout );

void BLE_TIMER_Stop( uint16_t id );

/* Callback
 */
void BLE_TIMERCB_Expiry( uint16_t id );

#endif /* BLE_TIMER_H__ */
