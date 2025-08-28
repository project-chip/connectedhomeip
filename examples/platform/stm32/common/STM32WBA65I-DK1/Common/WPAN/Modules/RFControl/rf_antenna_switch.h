/**
  ******************************************************************************
  * @file    rf_antenna_switch.h
  * @author  MCD Application Team
  * @brief   RF related module to handle dedictated GPIOs for antenna switch
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

#ifndef RF_ANTENNA_SWITCH_H
#define RF_ANTENNA_SWITCH_H

#include "stm32wbaxx.h"

#define RF_ANTSW0  {GPIOA, GPIO_PIN_12, GPIO_AF11_RF_ANTSW0}
#define RF_ANTSW1  {GPIOA, GPIO_PIN_11, GPIO_AF11_RF_ANTSW1}
#define RF_ANTSW2  {GPIOB, GPIO_PIN_2, GPIO_AF11_RF_ANTSW2}

typedef struct {
  GPIO_TypeDef* GPIO_port;
  uint16_t GPIO_pin;
  uint8_t  GPIO_alternate;
} st_gpio_antsw_t;

typedef enum {
  RF_ANTSW_DISABLE = 0,
  RF_ANTSW_ENABLE
} rf_antenna_switch_state_t;

static const st_gpio_antsw_t rt_antenna_switch_gpio_table[] =
{
  RF_ANTSW0,
  RF_ANTSW1,
  RF_ANTSW2
};

void RF_CONTROL_AntennaSwitch(rf_antenna_switch_state_t state);

#endif /* RF_ANTENNA_SWITCH_H */
