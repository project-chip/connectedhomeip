/**
 ******************************************************************************
 * @file    common_wpan_conf_template.h
 * @author  MCD Application Team
 * @brief   Template of configuration file for STM32 WPAN Common Utilities.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

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
    TOTAL_CLIENT_NUM, /* To be at the end of the enum */
} scm_user_id_t;

#ifdef SCM_DEBUG
#define SCM_DBG_PRINTF(...)                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        {                                                                                                                          \
            UTIL_ADV_TRACE_COND_FSend(VLEVEL_ALWAYS, T_REG_OFF, TS_OFF, __VA_ARGS__);                                              \
        }                                                                                                                          \
    } while (0);
#else
#define SCM_DBG_PRINTF(...)
#endif /* SCM_DEBUG */

/******************************************************************************
 * Timer Interface configuration
 ******************************************************************************/
#define RTC_N_PREDIV_S (10U)
#define RTC_PREDIV_S ((1U << RTC_N_PREDIV_S) - 1U)
#define RTC_PREDIV_A ((1U << (15U - RTC_N_PREDIV_S)) - 1U)

#ifdef TIF_DEBUG
#define TIMER_IF_DBG_PRINTF(...)                                                                                                   \
    do                                                                                                                             \
    {                                                                                                                              \
        {                                                                                                                          \
            UTIL_ADV_TRACE_COND_FSend(VLEVEL_ALWAYS, T_REG_OFF, TS_OFF, __VA_ARGS__);                                              \
        }                                                                                                                          \
    } while (0);

#else
#define TIMER_IF_DBG_PRINTF(...)
#endif /* TIF_DEBUG */

/******************************************************************************
 * Other module configuration
 ******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* COMMON_WPAN_CONF_H */
