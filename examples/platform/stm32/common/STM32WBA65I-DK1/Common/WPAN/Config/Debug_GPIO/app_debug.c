/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_debug.c
  * @author  MCD Application Team
  * @brief   Real Time Debug module application side APIs
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
#include "app_debug.h"
#include "RTDebug_dtb.h"

/****************************/
/** Application debug APIs **/
/****************************/

void APP_DEBUG_SIGNAL_SET(app_debug_signal_t signal)
{
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
  GENERIC_DEBUG_GPIO_SET(signal, app_debug_table);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */
}
void APP_DEBUG_SIGNAL_RESET(app_debug_signal_t signal)
{
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
  GENERIC_DEBUG_GPIO_RESET(signal, app_debug_table);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */
}

void APP_DEBUG_SIGNAL_TOGGLE(app_debug_signal_t signal)
{
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
  GENERIC_DEBUG_GPIO_TOGGLE(signal, app_debug_table);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */
}

/*******************************/
/** Debug GPIO Initialization **/
/*******************************/

#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
static uint32_t GPIO_PORT_TO_PWR_NUM(GPIO_TypeDef* gpio_port);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */

void RT_DEBUG_GPIO_Init(void)
{
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  uint32_t pwr_gpio_port = 0;
  uint32_t general_table_size = sizeof(general_debug_table)/sizeof(general_debug_table[0]);

  GPIO_InitStruct.Mode      = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull      = GPIO_NOPULL;
  GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = 0;

  for(unsigned int cpt = 0; cpt<general_table_size ; cpt++)
  {
    if(general_debug_table[cpt].GPIO_pin != RT_DEBUG_SIGNAL_UNUSED)
    {
      GPIO_InitStruct.Pin = general_debug_table[cpt].GPIO_pin;
      pwr_gpio_port = GPIO_PORT_TO_PWR_NUM(general_debug_table[cpt].GPIO_port);
      HAL_GPIO_Init(general_debug_table[cpt].GPIO_port, &GPIO_InitStruct);
      HAL_PWREx_EnableStandbyIORetention(pwr_gpio_port, general_debug_table[cpt].GPIO_pin);
    }
  }
#endif /* CFG_RT_DEBUG_GPIO_MODULE */

#if(CFG_RT_DEBUG_DTB == 1)
  /* DTB initialization and configuration */
  RT_DEBUG_DTBInit();
  RT_DEBUG_DTBConfig();
#endif /* CFG_RT_DEBUG_DTB */
}

#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
static uint32_t GPIO_PORT_TO_PWR_NUM(GPIO_TypeDef* gpio_port)
{
  uint32_t pwr_gpio_port = 0;
  if(gpio_port == GPIOA){pwr_gpio_port = PWR_GPIO_A;}
  else if(gpio_port == GPIOB){pwr_gpio_port = PWR_GPIO_B;}
  else if(gpio_port == GPIOC){pwr_gpio_port = PWR_GPIO_C;}
  else if(gpio_port == GPIOH){pwr_gpio_port = PWR_GPIO_H;}
  return pwr_gpio_port;
}
#endif /* CFG_RT_DEBUG_GPIO_MODULE */
