/**
  ******************************************************************************
  * @file    m95p32_conf.h
  * @author  MCD Application Team
  * @brief   M95P32 memory configuration template file.
  *          This file should be copied to the application folder and renamed
  *          to m95p32_conf.h
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
