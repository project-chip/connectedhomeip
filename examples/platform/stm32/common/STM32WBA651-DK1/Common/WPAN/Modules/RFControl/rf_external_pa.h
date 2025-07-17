/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rf_external_pa.h
  * @author  MCD Application Team
  * @brief   RF related module to handle dedictated GPIOs for external PA
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
#ifndef RF_EXTERNAL_PA_H
#define RF_EXTERNAL_PA_H

#include "stm32wbaxx.h"

#define RF_EPA_CSD     {GPIOA, GPIO_PIN_9, GPIO_MODE_OUTPUT_PP, GPIO_AF15_EVENTOUT}
#define RF_EPA_CTX     {GPIOB, GPIO_PIN_15, GPIO_MODE_AF_PP, GPIO_AF11_RF_IO1}

typedef struct {
  GPIO_TypeDef* GPIO_port;
  uint16_t GPIO_pin;
  uint32_t GPIO_mode;
  uint8_t  GPIO_alternate;
} st_gpio_epa_t;

typedef enum {
  RF_EPA_DISABLE = 0,
  RF_EPA_ENABLE
} rf_external_pa_state_t;

typedef enum {
  RF_EPA_SIGNAL_CSD = 0,
  RF_EPA_SIGNAL_CTX
} rf_external_pa_signal_t;

static const st_gpio_epa_t rf_external_pa_gpio_table[] =
{
  [RF_EPA_SIGNAL_CSD] = RF_EPA_CSD,
  [RF_EPA_SIGNAL_CTX] = RF_EPA_CTX
};

void RF_CONTROL_ExternalPA(rf_external_pa_state_t state);

#endif /* RF_EXTERNAL_PA_H */
