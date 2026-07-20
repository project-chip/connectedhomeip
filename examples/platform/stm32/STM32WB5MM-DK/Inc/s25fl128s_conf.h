/**
 ******************************************************************************
 * @file    s25fl128s_conf_template.h
 * @author  MCD Application Team
 * @brief   This file contains the configurations of the S25FL128S QSPI memory.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
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
