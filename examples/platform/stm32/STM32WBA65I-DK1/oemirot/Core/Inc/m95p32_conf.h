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
/**
 ******************************************************************************
 * @file    m95p32_conf.h
 * @author  MCD Application Team
 * @brief   M95P32 memory configuration template file.
 *          This file should be copied to the application folder and renamed
 *          to m95p32_conf.h
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef M95P32_CONF_H
#define M95P32_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32wbaxx_hal.h"

/** @addtogroup BSP
 * @{
 */
/* Uncomment one of the lines according the way to use the component */
/*#define USE_QUADSPI*/
#define USE_SPI

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* M95P32_CONF_H */
