/**
  ******************************************************************************
  * @file    RTDebug.c
  * @author  MCD Application Team
  * @brief   Real Time Debug module API definition
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

#include "RTDebug.h"
#include "local_debug_tables.h"
#include "stm32wbaxx_hal.h"
#include <assert.h>

#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
static_assert((sizeof(general_debug_table)/sizeof(st_gpio_debug_t)) == RT_DEBUG_SIGNALS_TOTAL_NUM,
              "Debug signals number is different from debug signal table size."
);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */

/***********************/
/** System debug APIs **/
/***********************/

void SYSTEM_DEBUG_SIGNAL_SET(system_debug_signal_t signal)
{
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
  GENERIC_DEBUG_GPIO_SET(signal, system_debug_table);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */
}

void SYSTEM_DEBUG_SIGNAL_RESET(system_debug_signal_t signal)
{
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
  GENERIC_DEBUG_GPIO_RESET(signal, system_debug_table);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */
}

void SYSTEM_DEBUG_SIGNAL_TOGGLE(system_debug_signal_t signal)
{
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
  GENERIC_DEBUG_GPIO_TOGGLE(signal, system_debug_table);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */
}

/***************************/
/** Link Layer debug APIs **/
/***************************/

/* Link Layer debug API definition */
void LINKLAYER_DEBUG_SIGNAL_SET(linklayer_debug_signal_t signal)
{
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
  GENERIC_DEBUG_GPIO_SET(signal, linklayer_debug_table);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */
}

void LINKLAYER_DEBUG_SIGNAL_RESET(linklayer_debug_signal_t signal)
{
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
   GENERIC_DEBUG_GPIO_RESET(signal, linklayer_debug_table);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */
}

void LINKLAYER_DEBUG_SIGNAL_TOGGLE(linklayer_debug_signal_t signal)
{
#if(CFG_RT_DEBUG_GPIO_MODULE == 1)
  GENERIC_DEBUG_GPIO_TOGGLE(signal, linklayer_debug_table);
#endif /* CFG_RT_DEBUG_GPIO_MODULE */
}
