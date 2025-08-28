/**
  ******************************************************************************
  * @file    RTDebug.h
  * @author  MCD Application Team
  * @brief   Real Time Debug module API declaration
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

#ifndef SYSTEM_DEBUG_H
#define SYSTEM_DEBUG_H

#include "debug_config.h"

#if(CFG_RT_DEBUG_GPIO_MODULE == 1)

/**************************************************************/
/** Generic macros for local signal table index recuperation **/
/** and global signal table GPIO manipulation                **/
/**************************************************************/

#define GENERIC_DEBUG_GPIO_SET(signal, table) do {                    \
  uint32_t debug_table_idx = 0;                                       \
  if(signal >= sizeof(table))                                         \
  {                                                                   \
    return;                                                           \
  }                                                                   \
  debug_table_idx = table[signal];                                    \
  if(debug_table_idx != RT_DEBUG_SIGNAL_UNUSED)                       \
  {                                                                   \
    HAL_GPIO_WritePin(general_debug_table[debug_table_idx].GPIO_port, \
                      general_debug_table[debug_table_idx].GPIO_pin,  \
                      GPIO_PIN_SET);                                  \
  }                                                                   \
} while(0)

#define GENERIC_DEBUG_GPIO_RESET(signal, table) do {                  \
  uint32_t debug_table_idx = 0;                                       \
  if(signal >= sizeof(table))                                         \
  {                                                                   \
    return;                                                           \
  }                                                                   \
  debug_table_idx = table[signal];                                    \
  if(debug_table_idx != RT_DEBUG_SIGNAL_UNUSED)                       \
  {                                                                   \
    HAL_GPIO_WritePin(general_debug_table[debug_table_idx].GPIO_port, \
                      general_debug_table[debug_table_idx].GPIO_pin,  \
                      GPIO_PIN_RESET);                                \
  }                                                                   \
} while(0)

#define GENERIC_DEBUG_GPIO_TOGGLE(signal, table) do {                  \
  uint32_t debug_table_idx = 0;                                        \
  if(signal >= sizeof(table))                                          \
  {                                                                    \
    return;                                                            \
  }                                                                    \
  debug_table_idx = table[signal];                                     \
  if(debug_table_idx != RT_DEBUG_SIGNAL_UNUSED)                        \
  {                                                                    \
    HAL_GPIO_TogglePin(general_debug_table[debug_table_idx].GPIO_port, \
                       general_debug_table[debug_table_idx].GPIO_pin); \
  }                                                                    \
} while(0)

#endif /* CFG_RT_DEBUG_GPIO_MODULE */

/* System debug API definition */
void SYSTEM_DEBUG_SIGNAL_SET(system_debug_signal_t signal);
void SYSTEM_DEBUG_SIGNAL_RESET(system_debug_signal_t signal);
void SYSTEM_DEBUG_SIGNAL_TOGGLE(system_debug_signal_t signal);

/* Link Layer debug API definition */
void LINKLAYER_DEBUG_SIGNAL_SET(linklayer_debug_signal_t signal);
void LINKLAYER_DEBUG_SIGNAL_RESET(linklayer_debug_signal_t signal);
void LINKLAYER_DEBUG_SIGNAL_TOGGLE(linklayer_debug_signal_t signal);

#endif /* SYSTEM_DEBUG_H */
