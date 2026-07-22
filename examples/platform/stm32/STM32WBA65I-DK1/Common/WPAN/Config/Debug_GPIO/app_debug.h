/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/* USER CODE END Header */
#ifndef APP_DEBUG_H
#define APP_DEBUG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "RTDebug.h"

/***************************************************/
/** Specific application debug signals definition **/
/***************************************************/
typedef enum
{
    APP_APPE_INIT,
} app_debug_signal_t;

#if (CFG_RT_DEBUG_GPIO_MODULE == 1)

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

#ifdef __cplusplus
}
#endif

#endif /* APP_DEBUG_H */
