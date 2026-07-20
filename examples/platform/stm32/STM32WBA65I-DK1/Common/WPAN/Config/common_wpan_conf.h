/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    common_wpan_conf.h
 * @author  MCD Application Team
 * @brief   Configuration file for STM32 WPAN Common Utilities.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef COMMON_WPAN_CONF_H
#define COMMON_WPAN_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32_adv_trace.h"

/******************************************************************************
 * System Clock Manager (SCM) configuration
 ******************************************************************************/
typedef enum
{
    SCM_USER_APP,
    SCM_USER_LL_FW,
    SCM_USER_LL_HW_RCO_CLBR,
    /* USER CODE BEGIN SCM_E */

    /* USER CODE END SCM_E */
    TOTAL_CLIENT_NUM, /* To be at the end of the enum */
} scm_user_id_t;

/* USER CODE BEGIN SCM_DBG1 */

/* USER CODE END SCM_DBG1 */

#ifdef SCM_DEBUG
#define SCM_DBG_PRINTF(...)                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        {                                                                                                                          \
            UTIL_ADV_TRACE_COND_FSend(VLEVEL_ALWAYS, T_REG_OFF, TS_OFF, __VA_ARGS__);                                              \
        }                                                                                                                          \
    } while (0);

/* USER CODE BEGIN SCM_DBG2 */

/* USER CODE END SCM_DBG2 */
#else
#define SCM_DBG_PRINTF(...)
#endif /* SCM_DEBUG */

/******************************************************************************
 * Timer Interface configuration
 ******************************************************************************/
#define RTC_N_PREDIV_S (10U)
#define RTC_PREDIV_S ((1U << RTC_N_PREDIV_S) - 1U)
#define RTC_PREDIV_A ((1U << (15U - RTC_N_PREDIV_S)) - 1U)

/* USER CODE BEGIN TIF_RTC */

/* USER CODE END TIF_RTC */

/* USER CODE BEGIN TIF_DBG1 */

/* USER CODE END TIF_DBG1 */

#ifdef TIF_DEBUG
#define TIMER_IF_DBG_PRINTF(...)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        {                                                                                                                          \
            UTIL_ADV_TRACE_COND_FSend(VLEVEL_ALWAYS, T_REG_OFF, TS_OFF, __VA_ARGS__);                                              \
        }                                                                                                                          \
    } while (0);

/* USER CODE BEGIN TIF_DBG2 */

/* USER CODE END TIF_DBG2 */
#else
#define TIMER_IF_DBG_PRINTF(...)
#endif /* TIF_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* COMMON_WPAN_CONF_H */
