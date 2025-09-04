/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    app_debug.h
  * @author  MCD Application Team
  * @brief   Real Time Debug module application APIs and signal table
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
#ifndef APP_DEBUG_H
#define APP_DEBUG_H

#include "RTDebug.h"

/***************************************************/
/** Specific application debug signals definition **/
/***************************************************/
typedef enum {
  APP_APPE_INIT,
} app_debug_signal_t;

#if(CFG_RT_DEBUG_GPIO_MODULE == 1)

/************************************/
/** Application local signal table **/
/************************************/
static const rt_debug_signal_t app_debug_table[] = {
#if (USE_RT_DEBUG_APP_APPE_INIT == 1)
  [APP_APPE_INIT] = RT_DEBUG_APP_APPE_INIT,
#else
  [APP_APPE_INIT] = RT_DEBUG_SIGNAL_UNUSED,
#endif /* USE_RT_DEBUG_APP_APPE_INIT */
};

#endif /* CFG_RT_DEBUG_GPIO_MODULE */

/**************************************/
/** Application debug API definition **/
/**************************************/
void APP_DEBUG_SIGNAL_SET(app_debug_signal_t signal);
void APP_DEBUG_SIGNAL_RESET(app_debug_signal_t signal);
void APP_DEBUG_SIGNAL_TOGGLE(app_debug_signal_t signal);

/*******************************/
/** Debug GPIO Initialization **/
/*******************************/
void RT_DEBUG_GPIO_Init(void);

#endif /* APP_DEBUG_H */
