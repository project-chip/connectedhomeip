/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    utilities_conf.h
  * @author  MCD Application Team
  * @brief   Header for configuration file for STM32 Utilities.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#ifndef UTILITIES_CONF_H
#define UTILITIES_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "cmsis_compiler.h"
#include "app_conf.h"
/* definitions to be provided to "sequencer" utility */
#include "stm32_mem.h"
/* definition and callback for tiny_vsnprintf */
#include "stm32_tiny_vsnprintf.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/

#define VLEVEL_OFF    0  /*!< used to set UTIL_ADV_TRACE_SetVerboseLevel() (not as message param) */
#define VLEVEL_ALWAYS 0  /*!< used as message params, if this level is given
                              trace will be printed even when UTIL_ADV_TRACE_SetVerboseLevel(OFF) */
#define VLEVEL_L 1       /*!< just essential traces */
#define VLEVEL_M 2       /*!< functional traces */
#define VLEVEL_H 3       /*!< all traces */

#define TS_OFF 0         /*!< Log without TimeStamp */
#define TS_ON 1          /*!< Log with TimeStamp */

#define T_REG_OFF  0     /*!< Log without bitmask */

/* USER CODE BEGIN EC */

/* USER CODE END EC */
/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macros -----------------------------------------------------------*/
/**
  * @brief Memory placement macro
  */
#if defined(__CC_ARM)
#define UTIL_PLACE_IN_SECTION( __x__ )  __attribute__((section (__x__), zero_init))
#elif defined(__ICCARM__)
#define UTIL_PLACE_IN_SECTION( __x__ )  __attribute__((section (__x__)))
#else  /* __GNUC__ */
#define UTIL_PLACE_IN_SECTION( __x__ )  __attribute__((section (__x__)))
#endif /* __CC_ARM | __ICCARM__ | __GNUC__ */

/**
  * @brief Memory alignment macro
  */
#undef ALIGN
#ifdef WIN32
#define ALIGN(n)
#else
#define ALIGN(n)             __attribute__((aligned(n)))
#endif /* WIN32 */

/**
  * @brief macro used to initialize the critical section
  */
#define UTIL_SEQ_INIT_CRITICAL_SECTION( )    UTILS_INIT_CRITICAL_SECTION()

/**
  * @brief macro used to enter the critical section
  */
#define UTIL_SEQ_ENTER_CRITICAL_SECTION( )   UTILS_ENTER_CRITICAL_SECTION()

/**
  * @brief macro used to exit the critical section
  */
#define UTIL_SEQ_EXIT_CRITICAL_SECTION( )    UTILS_EXIT_CRITICAL_SECTION()

/**
  * @brief Memset utilities interface to application
  */
#define UTIL_SEQ_MEMSET8( dest, value, size )   UTIL_MEM_set_8( dest, value, size )

/**
  * @brief macro used to initialize the critical section
  */
#define UTILS_INIT_CRITICAL_SECTION()

/**
  * @brief macro used to enter the critical section
  */
#define UTILS_ENTER_CRITICAL_SECTION() uint32_t primask_bit= __get_PRIMASK();\
  __disable_irq()

/**
  * @brief macro used to exit the critical section
  */
#define UTILS_EXIT_CRITICAL_SECTION()  __set_PRIMASK(primask_bit)

/**
  * @brief macro used to enter the critical section
  */
#define UTILS_ENTER_LIMITED_CRITICAL_SECTION(x) uint32_t basepri_value= __get_BASEPRI();\
  __set_BASEPRI_MAX(x)

/**
  * @brief macro used to exit the critical section
  */
#define UTILS_EXIT_LIMITED_CRITICAL_SECTION()  __set_BASEPRI(basepri_value)

/******************************************************************************
  * trace\advanced
  * the define option
  *    UTIL_ADV_TRACE_CONDITIONNAL shall be defined if you want use conditional function
  *    UTIL_ADV_TRACE_UNCHUNK_MODE shall be defined if you want use the unchunk mode
  *
  ******************************************************************************/

#define UTIL_ADV_TRACE_CONDITIONNAL                                                      /*!< not used */
#define UTIL_ADV_TRACE_UNCHUNK_MODE                                                      /*!< not used */
#define UTIL_ADV_TRACE_DEBUG(...)                                                        /*!< not used */
#define UTIL_ADV_TRACE_INIT_CRITICAL_SECTION( )    UTILS_INIT_CRITICAL_SECTION()         /*!< init the critical section in trace feature */
#define UTIL_ADV_TRACE_ENTER_CRITICAL_SECTION( )   UTILS_ENTER_CRITICAL_SECTION()        /*!< enter the critical section in trace feature */
#define UTIL_ADV_TRACE_EXIT_CRITICAL_SECTION( )    UTILS_EXIT_CRITICAL_SECTION()         /*!< exit the critical section in trace feature */
#define UTIL_ADV_TRACE_TMP_BUF_SIZE                (CFG_LOG_TRACE_BUF_SIZE)              /*!< trace buffer size */
#define UTIL_ADV_TRACE_TMP_MAX_TIMESTMAP_SIZE      (15U)                                 /*!< trace timestamp size */
#define UTIL_ADV_TRACE_FIFO_SIZE                   (CFG_LOG_TRACE_FIFO_SIZE)             /*!< trace fifo size */
#define UTIL_ADV_TRACE_MEMSET8( dest, value, size) UTIL_MEM_set_8((dest),(value),(size)) /*!< memset utilities interface to trace feature */
#define UTIL_ADV_TRACE_VSNPRINTF(...)              vsnprintf(__VA_ARGS__)      /*!< vsnprintf utilities interface to trace feature */

/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /*UTILITIES_CONF_H */
