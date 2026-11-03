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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef S25FL128S_CONF_H
#define S25FL128S_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx_hal.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Components
 * @{
 */

/** @addtogroup S25FL128S
 * @brief     This file provides a set of definitions for the Spansion
 *            S25FL128S memory configuration.
 * @{
 */

/** @addtogroup S25FL128S_Exported_Constants
 * @{
 */

#define CONF_S25FL128S_READ_ENHANCE 0 /* MMP performance enhance read enable/disable */
#define CONF_QSPI_DUMMY_CLOCK 8U

/* Dummy cycles for STR read mode */
#define S25FL128S_DUMMY_CYCLES_READ_QUAD 8U
#define S25FL128S_DUMMY_CYCLES_READ 8U
#define S25FL128S_DUMMY_CYCLES_READ_DUAL_INOUT 4U
#define S25FL128S_DUMMY_CYCLES_READ_QUAD_INOUT 6U

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* S25FL128S_CONF_H */
