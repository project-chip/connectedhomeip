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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef UTILITIES_CONF_H
#define UTILITIES_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_compiler.h"
#include "string.h"

/******************************************************************************
 * common
 ******************************************************************************/
#define UTILS_ENTER_CRITICAL_SECTION()                                                                                             \
    uint32_t primask_bit = __get_PRIMASK();                                                                                        \
    __disable_irq()

#define UTILS_EXIT_CRITICAL_SECTION() __set_PRIMASK(primask_bit)

#define UTILS_MEMSET8(dest, value, size) memset(dest, value, size);

/******************************************************************************
 * tiny low power manager
 * (any macro that does not need to be modified can be removed)
 ******************************************************************************/
#define UTIL_LPM_INIT_CRITICAL_SECTION()
#define UTIL_LPM_ENTER_CRITICAL_SECTION() UTILS_ENTER_CRITICAL_SECTION()
#define UTIL_LPM_EXIT_CRITICAL_SECTION() UTILS_EXIT_CRITICAL_SECTION()

/******************************************************************************
 * sequencer
 * (any macro that does not need to be modified can be removed)
 ******************************************************************************/
#define UTIL_SEQ_INIT_CRITICAL_SECTION()
#define UTIL_SEQ_ENTER_CRITICAL_SECTION() UTILS_ENTER_CRITICAL_SECTION()
#define UTIL_SEQ_EXIT_CRITICAL_SECTION() UTILS_EXIT_CRITICAL_SECTION()
#define UTIL_SEQ_CONF_TASK_NBR (32)
#define UTIL_SEQ_CONF_PRIO_NBR (2)
#define UTIL_SEQ_MEMSET8(dest, value, size) UTILS_MEMSET8(dest, value, size)

#ifdef __cplusplus
}
#endif

#endif /*UTILITIES_CONF_H */
