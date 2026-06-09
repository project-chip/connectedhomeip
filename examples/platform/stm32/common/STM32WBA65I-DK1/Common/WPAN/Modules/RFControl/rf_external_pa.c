/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rf_external_pa.c
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

#include "rf_external_pa.h"
#include "power_table.h"
#include "ll_intf.h"
#include "app_conf.h"

static void RF_CONTROL_ExternalPA_Enable(void);
static void RF_CONTROL_ExternalPA_Disable(void);
static uint8_t RF_CONTROL_ExternalPA_Enable_cb(uint8_t epa_enable);

static void RF_CONTROL_ExternalPA_Enable(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  uint32_t table_size = sizeof(rf_external_pa_gpio_table)/sizeof(rf_external_pa_gpio_table[0]);

  for(unsigned int cpt = 0; cpt<table_size; cpt++)
  {
    GPIO_InitStruct.Pin    = rf_external_pa_gpio_table[cpt].GPIO_pin;
    GPIO_InitStruct.Mode   = rf_external_pa_gpio_table[cpt].GPIO_mode;
    GPIO_InitStruct.Pull   = GPIO_NOPULL;
    GPIO_InitStruct.Speed  = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = rf_external_pa_gpio_table[cpt].GPIO_alternate;
    HAL_GPIO_Init(rf_external_pa_gpio_table[cpt].GPIO_port, &GPIO_InitStruct);
  }
}

static void RF_CONTROL_ExternalPA_Disable(void)
{
  uint32_t table_size = sizeof(rf_external_pa_gpio_table)/sizeof(rf_external_pa_gpio_table[0]);

  for(unsigned int cpt = 0; cpt<table_size; cpt++)
  {
    HAL_GPIO_DeInit(rf_external_pa_gpio_table[cpt].GPIO_port, rf_external_pa_gpio_table[cpt].GPIO_pin);
  }
}

static uint8_t RF_CONTROL_ExternalPA_Enable_cb(uint8_t epa_enable)
{
  if(epa_enable == 1)
  {
    HAL_GPIO_WritePin(rf_external_pa_gpio_table[RF_EPA_SIGNAL_CSD].GPIO_port, rf_external_pa_gpio_table[RF_EPA_SIGNAL_CSD].GPIO_pin, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(rf_external_pa_gpio_table[RF_EPA_SIGNAL_CSD].GPIO_port, rf_external_pa_gpio_table[RF_EPA_SIGNAL_CSD].GPIO_pin, GPIO_PIN_RESET);
  }
  
  return 0;
}

void RF_CONTROL_ExternalPA(rf_external_pa_state_t state)
{
  ble_stat_t status = GENERAL_FAILURE;

  if(state == RF_EPA_ENABLE)
  {
    status = ll_tx_pwr_if_epa_init(1, RF_CONTROL_ExternalPA_Enable_cb);
    if(status != SUCCESS)
    {
      /* EPA callback is not defined */
      assert_param(0);
      return;
    }
    RF_CONTROL_ExternalPA_Enable();
  }
  else
  {
    RF_CONTROL_ExternalPA_Disable();
  }
}
