/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    log_module_conf.h
  * @author  MCD Application Team
  * @brief   Header file of the log module.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef LOG_MODULE_CONF_H
#define LOG_MODULE_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "app_conf.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Module configuration ------------------------------------------------------*/
/**
 * @brief  When this define is set to 0, there is no time stamp added to the trace data.
 *         When this define is set to 1, the time stamp is added to the trace data,
 *         according to the function registered with Log_Module_RegisterTimeStampFunction.
 */
#define LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE    CFG_LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE

/**
 * @brief  When this define is set to 0, the color of the trace data remains the same for all regions.
 *         When this define is set to 1, the color added to the trace data is based on LOG_COLOR_DEFAULT_CONFIGURATION.
 */
#define LOG_INSERT_COLOR_INSIDE_THE_TRACE         CFG_LOG_INSERT_COLOR_INSIDE_THE_TRACE

/**
 * @brief  When this define is set to 0, the trace data is not modified.
 *         When this define is set to 1, if there is no ENDOFLINE_CHAR as last
 *         character in the trace data, then one is added.
 */
#define LOG_INSERT_EOL_INSIDE_THE_TRACE           CFG_LOG_INSERT_EOL_INSIDE_THE_TRACE

/* USER CODE BEGIN Module configuration */

/* USER CODE END Module configuration */

/* Private defines -----------------------------------------------------------*/
/* These defines are related to the UTIL_ADV_TRACE. Do not modify them please. */
#define LOG_MODULE_MIN_VERBOSE_LEVEL    (0)
#define LOG_MODULE_MAX_VERBOSE_LEVEL    (0xFFFFFFFF)
#define LOG_MODULE_MIN_REGION_VALUE     (0)
#define LOG_MODULE_ALL_REGION_MASK      (0xFFFFFFFF)

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Exported types ------------------------------------------------------------*/
/* Log module types */
/**
 * @brief  Customizable enum describing the verbose levels used by the log module.
 *         The levels include the lower levels in the logs.
 *
 *         E.g. LOG_VERBOSE_ERROR means LOG_VERBOSE_ERROR logs will be printed,
 *         as well as LOG_VERBOSE_INFO, but not the others with higher values.
 *
 *         The min and max ranges are defined by LOG_MODULE_MIN_VERBOSE_LEVEL
 *         and LOG_MODULE_MAX_VERBOSE_LEVEL.
 *
 *         The user can add its own levels but must NOT add a value to the said
 *         levels. Verbose levels are handled by the UTIL_ADV_TRACE.
 */
typedef enum
{
  LOG_VERBOSE_INFO = LOG_MODULE_MIN_VERBOSE_LEVEL,
  /* USER CODE BEGIN Log_Verbose_Level_t_0 */

  /* USER CODE END Log_Verbose_Level_t_0 */
  LOG_VERBOSE_ERROR,
  /* USER CODE BEGIN Log_Verbose_Level_t_1 */

  /* USER CODE END Log_Verbose_Level_t_1 */
  LOG_VERBOSE_WARNING,
  /* USER CODE BEGIN Log_Verbose_Level_t_2 */

  /* USER CODE END Log_Verbose_Level_t_2 */
  LOG_VERBOSE_DEBUG,
  /* USER CODE BEGIN Log_Verbose_Level_t_3 */

  /* USER CODE END Log_Verbose_Level_t_3 */
  LOG_VERBOSE_ALL_LOGS = LOG_MODULE_MAX_VERBOSE_LEVEL,
} Log_Verbose_Level_t;

/**
 * @brief  Customizable enum describing the regions used by the log module.
 *         Regions are used to separate the logs into different places.
 *
 *         Let's say you have a Task 1 and a Task 2.
 *         Both of them have Info and Debug logs.
 *
 *         By using them as such, i.e. with the same regions, you'll
 *         print the logs of the 2 tasks as long as the verbose is Info or Debug.
 *
 *         If you create a region for Task 1 and another for Task 2, you can
 *         split the logs between them, and, if needed, only print the Debug
 *         logs for Task 1 only (i.e. Task 1 logs for Info and Debug).
 *
 *         Behind the scenes is a mask into which each region is a bit.
 *         The user can add its own regions but must NOT add a value to them.
 *         The log module handles the mask on its own.
 */
typedef enum
{
  LOG_REGION_BLE = LOG_MODULE_MIN_REGION_VALUE,
  LOG_REGION_SYSTEM,
  LOG_REGION_APP,
  LOG_REGION_LINKLAYER,
  LOG_REGION_MAC,
  LOG_REGION_ZIGBEE,
  LOG_REGION_THREAD,
  LOG_REGION_RTOS,
  /* USER CODE BEGIN Log_Region_t */

  /* USER CODE END Log_Region_t */
  LOG_REGION_ALL_REGIONS = LOG_MODULE_ALL_REGION_MASK,
} Log_Region_t;

typedef enum
{
  LOG_COLOR_NONE          = 0,     /* Initialization */
  LOG_COLOR_CODE_DEFAULT  = 37,    /* White */
  LOG_COLOR_CODE_RED      = 91,
  LOG_COLOR_CODE_GREEN    = 92,
  LOG_COLOR_CODE_YELLOW   = 93,
  LOG_COLOR_CODE_CYAN     = 96,
  /* USER CODE BEGIN Log_Color_t */

  /* USER CODE END Log_Color_t */
} Log_Color_t;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported macro ------------------------------------------------------------*/
/* Display 64 bits number for all compiler. */
/* Example : LOG_INFO_APP( "New Device : " LOG_DISPLAY64() " installed in %d seconds", LOG_NUMBER64( dlDevice ), iTime ); */
#define LOG_DISPLAY64()             "0x%08X%08X"
#define LOG_NUMBER64( number )      (uint32_t)( number >> 32u ), (uint32_t)( number )

/* Module API - Log macros for each region */
/* LOG_REGION_BLE */
#if (CFG_LOG_SUPPORTED != 0)
#define LOG_INFO_BLE(...)         Log_Module_Print( LOG_VERBOSE_INFO, LOG_REGION_BLE, __VA_ARGS__)
#define LOG_ERROR_BLE(...)        Log_Module_Print( LOG_VERBOSE_ERROR, LOG_REGION_BLE, __VA_ARGS__)
#define LOG_WARNING_BLE(...)      Log_Module_Print( LOG_VERBOSE_WARNING, LOG_REGION_BLE, __VA_ARGS__)
#define LOG_DEBUG_BLE(...)        Log_Module_Print( LOG_VERBOSE_DEBUG, LOG_REGION_BLE, __VA_ARGS__)
#else /* (CFG_LOG_SUPPORTED != 0) */
#define LOG_INFO_BLE(...)         do {} while(0)
#define LOG_ERROR_BLE(...)        do {} while(0)
#define LOG_WARNING_BLE(...)      do {} while(0)
#define LOG_DEBUG_BLE(...)        do {} while(0)
#endif /* (CFG_LOG_SUPPORTED != 0) */

/* USER CODE BEGIN LOG_REGION_BLE */
/**
 * Add inside this user section your defines to match the new verbose levels you
 * created into Log_Verbose_Level_t.
 * Example :
 * #define LOG_CUSTOM_BLE(...)      Log_Module_Print( LOG_VERBOSE_CUSTOM, LOG_REGION_BLE, __VA_ARGS__);
 *
 * You don't need to update all regions with your custom values.
 * Do it accordingly to your needs. E.g you might not need LOG_VERBOSE_CUSTOM for a System region.
 */

/* USER CODE END LOG_REGION_BLE */

/* LOG_REGION_SYSTEM */
#if (CFG_LOG_SUPPORTED != 0)
#define LOG_INFO_SYSTEM(...)      Log_Module_Print( LOG_VERBOSE_INFO, LOG_REGION_SYSTEM, __VA_ARGS__)
#define LOG_ERROR_SYSTEM(...)     Log_Module_Print( LOG_VERBOSE_ERROR, LOG_REGION_SYSTEM, __VA_ARGS__)
#define LOG_WARNING_SYSTEM(...)   Log_Module_Print( LOG_VERBOSE_WARNING, LOG_REGION_SYSTEM, __VA_ARGS__)
#define LOG_DEBUG_SYSTEM(...)     Log_Module_Print( LOG_VERBOSE_DEBUG, LOG_REGION_SYSTEM, __VA_ARGS__)
#else /* (CFG_LOG_SUPPORTED != 0) */
#define LOG_INFO_SYSTEM(...)      do {} while(0)
#define LOG_ERROR_SYSTEM(...)     do {} while(0)
#define LOG_WARNING_SYSTEM(...)   do {} while(0)
#define LOG_DEBUG_SYSTEM(...)     do {} while(0)
#endif /* (CFG_LOG_SUPPORTED != 0) */

/* USER CODE BEGIN LOG_REGION_SYSTEM */
/**
 * Add inside this user section your defines to match the new verbose levels you
 * created into Log_Verbose_Level_t.
 * Example :
 * #define LOG_CUSTOM_SYSTEM(...)      Log_Module_Print( LOG_VERBOSE_CUSTOM, LOG_REGION_SYSTEM, __VA_ARGS__);
 *
 * You don't need to update all regions with your custom values.
 * Do it accordingly to your needs. E.g you might not need LOG_VERBOSE_CUSTOM for a System region.
 */

/* USER CODE END LOG_REGION_SYSTEM */

/* LOG_REGION_APP */
#if (CFG_LOG_SUPPORTED != 0)
#define LOG_INFO_APP(...)         Log_Module_Print( LOG_VERBOSE_INFO, LOG_REGION_APP, __VA_ARGS__)
#define LOG_ERROR_APP(...)        Log_Module_Print( LOG_VERBOSE_ERROR, LOG_REGION_APP, __VA_ARGS__)
#define LOG_WARNING_APP(...)      Log_Module_Print( LOG_VERBOSE_WARNING, LOG_REGION_APP, __VA_ARGS__)
#define LOG_DEBUG_APP(...)        Log_Module_Print( LOG_VERBOSE_DEBUG, LOG_REGION_APP, __VA_ARGS__)
#else /* (CFG_LOG_SUPPORTED != 0) */
#define LOG_INFO_APP(...)         do {} while(0)
#define LOG_ERROR_APP(...)        do {} while(0)
#define LOG_WARNING_APP(...)      do {} while(0)
#define LOG_DEBUG_APP(...)        do {} while(0)
#endif /* (CFG_LOG_SUPPORTED != 0) */

/* USER CODE BEGIN LOG_REGION_APP */
/**
 * Add inside this user section your defines to match the new verbose levels you
 * created into Log_Verbose_Level_t.
 * Example :
 * #define LOG_CUSTOM_APP(...)         Log_Module_Print( LOG_VERBOSE_CUSTOM, LOG_REGION_APP, __VA_ARGS__);
 *
 * You don't need to update all regions with your custom values.
 * Do it accordingly to your needs. E.g you might not need LOG_VERBOSE_CUSTOM for a System region.
 */

/* USER CODE END LOG_REGION_APP */

/* LOG_REGION_LINKLAYER */
#if (CFG_LOG_SUPPORTED != 0)
#define LOG_INFO_LINKLAYER(...)   Log_Module_Print( LOG_VERBOSE_INFO, LOG_REGION_LINKLAYER, __VA_ARGS__)
#define LOG_ERROR_LINKLAYER(...)  Log_Module_Print( LOG_VERBOSE_ERROR, LOG_REGION_LINKLAYER, __VA_ARGS__)
#define LOG_WARNING_LINKLAYER(...)Log_Module_Print( LOG_VERBOSE_WARNING, LOG_REGION_LINKLAYER, __VA_ARGS__)
#define LOG_DEBUG_LINKLAYER(...)  Log_Module_Print( LOG_VERBOSE_DEBUG, LOG_REGION_LINKLAYER, __VA_ARGS__)
#else /* (CFG_LOG_SUPPORTED != 0) */
#define LOG_INFO_LINKLAYER(...)   do {} while(0)
#define LOG_ERROR_LINKLAYER(...)  do {} while(0)
#define LOG_WARNING_LINKLAYER(...)do {} while(0)
#define LOG_DEBUG_LINKLAYER(...)  do {} while(0)
#endif /* (CFG_LOG_SUPPORTED != 0) */

/* USER CODE BEGIN LOG_REGION_LINKLAYER */
/**
 * Add inside this user section your defines to match the new verbose levels you
 * created into Log_Verbose_Level_t.
 * Example :
 * #define LOG_CUSTOM_LINKLAYER(...)         Log_Module_Print( LOG_VERBOSE_CUSTOM, LOG_REGION_LINKLAYER, __VA_ARGS__);
 *
 * You don't need to update all regions with your custom values.
 * Do it accordingly to your needs. E.g you might not need LOG_VERBOSE_CUSTOM for a System region.
 */

/* USER CODE END LOG_REGION_LINKLAYER */

/* USER CODE BEGIN APP_LOG_USER_DEFINES */
/**
 * Add inside this user section your defines to match the new regions you
 * created into Log_Region_t.
 * Example :
#if (CFG_LOG_SUPPORTED != 0)
#define LOG_INFO_CUSTOM(...)      Log_Module_Print( LOG_VERBOSE_INFO, LOG_REGION_CUSTOM, __VA_ARGS__)
#define LOG_ERROR_CUSTOM(...)     Log_Module_Print( LOG_VERBOSE_ERROR, LOG_REGION_CUSTOM, __VA_ARGS__)
#define LOG_WARNING_CUSTOM(...)   Log_Module_Print( LOG_VERBOSE_WARNING, LOG_REGION_CUSTOM, __VA_ARGS__)
#define LOG_DEBUG_CUSTOM(...)     Log_Module_Print( LOG_VERBOSE_DEBUG, LOG_REGION_CUSTOM, __VA_ARGS__)
#else
#define LOG_INFO_CUSTOM(...)      do {} while(0)
#define LOG_ERROR_CUSTOM(...)     do {} while(0)
#define LOG_WARNING_CUSTOM(...)   do {} while(0)
#define LOG_DEBUG_CUSTOM(...)     do {} while(0)
#endif
 */
/* USER CODE END APP_LOG_USER_DEFINES */

#ifdef __cplusplus
}
#endif

#endif /* LOG_MODULE_CONF_H */
