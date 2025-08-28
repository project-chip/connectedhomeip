/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    log_module.h
  * @author  MCD Application Team
  * @brief   Header file of the log module.
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
#ifndef LOG_MODULE_H
#define LOG_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Module configuration ------------------------------------------------------*/
/**
 * @brief  When this define is set to 0, there is no time stamp insertion inside
 *         the trace data. When set to 1, there is.
 */
#define LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE    CFG_LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE

/**
 * @brief  When this define is set to 1, a color in function or region is inserted on
 *         the trace data. When set to 0, color is always the same.
 */
#define LOG_INSERT_COLOR_INSIDE_THE_TRACE         CFG_LOG_INSERT_COLOR_INSIDE_THE_TRACE

/**
 * @brief  When this define is set to 1, a End Of File is inserted at the end of
 *         the trace data. When set to 0, not EOF is inserted.
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

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

/* Exported types ------------------------------------------------------------*/
/* Log module types */
/**
 * @brief  Customizable enum describing the verbose levels used by the log module.
 *         The levels include the lower levels in the logs.
 *         E.g. LOG_VERBOSE_ERROR means LOG_VERBOSE_ERROR logs will be printed,
 *         as well as LOG_VERBOSE_INFO, but not the others with higher values.
 *         The min and max ranges are defined by LOG_MODULE_MIN_VERBOSE_LEVEL
 *         and LOG_MODULE_MAX_VERBOSE_LEVEL.
 *         The user can add its own levels but must NOT add a value to the said
 *         levels. Verbose levels are handled by ADV Trace.
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
 *         Let's say you have a Task 1 and a Task 2. Both of them have Info and
 *         Debug logs. By using them as such, i.e. with the same regions, you'll
 *         print the logs of the 2 tasks as long as the verbose is Info or Debug.
 *         If you create a region for Task 1 and another for Task 2, you can
 *         split the logs between them, and, if needed, only print the Debug
 *         logs for Task 1 only (i.e. Task 1 logs for Info and Debug).
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
  LOG_COLOR_NONE          = 0,     // Initialization.
  LOG_COLOR_CODE_DEFAULT  = 37,    // White
  LOG_COLOR_CODE_RED      = 91,
  LOG_COLOR_CODE_GREEN    = 92,
  LOG_COLOR_CODE_YELLOW   = 93,
  LOG_COLOR_CODE_CYAN     = 96,
  /* USER CODE BEGIN Log_Color_t */

  /* USER CODE END Log_Color_t */
} Log_Color_t;

/**
 * @brief  Data type to initialize the module by calling Log_Module_Init.
 */
typedef struct
{
  Log_Verbose_Level_t verbose_level;
  Log_Region_t region;
} Log_Module_t;

/**
 * @brief Callback function to insert Time Stamp.
 *
 * @param  pData    The location where insert the new TimeStamp
 * @param  piSize   The size of the TimeStamp insert.
 */
typedef void CallBack_TimeStamp( uint8_t * pData, uint16_t * piSize );

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* Global const struct variables to make the life of the user easier */
/**
 * @brief  A const struct with default parameters for the log module.
 *         Its main use is to pass it as parameter to Log_Module_Init.
 */
extern const Log_Module_t LOG_MODULE_DEFAULT_CONFIGURATION;

/**
 * @brief  A const enum variable with the verbose level set to LOG_VERBOSE_ERROR.
 *         The levels include the lower levels in the logs.
 *         E.g. LOG_VERBOSE_ERROR means LOG_VERBOSE_ERROR logs will be printed,
 *         as well as LOG_VERBOSE_INFO, but not the others with higher values.
 */
extern const Log_Verbose_Level_t LOG_VERBOSE_DEFAULT;

/**
 * @brief  A const enum variable to include all regions in the logs.
 */
extern const Log_Region_t LOG_REGION_MASK_DEFAULT;

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported functions prototypes ---------------------------------------------*/
/* Module API - Module configuration */
/**
 * @brief  Initialization of the log module.
 * @param  The configuration of the log module, of type Log_Module_t.
 * @return None.
 */
void Log_Module_Init(Log_Module_t log_configuration);

/**
 * @brief  DeInitialization of the log module.
 * @param  None.
 * @return None.
 */
void Log_Module_DeInit(void);

/**
 * @brief  Set the verbose level of the log.
 *         The levels include the lower levels in the logs.
 *         E.g. LOG_VERBOSE_ERROR means LOG_VERBOSE_ERROR logs will be printed,
 *         as well as LOG_VERBOSE_INFO, but not the others with higher values.
 * @param  The new verbose level to be set, of type Log_Verbose_Level_t
 * @return None
 */
void Log_Module_Set_Verbose_Level(Log_Verbose_Level_t new_verbose_level);

/**
 * @brief  Replace the current region mask to use and set only the given region.
 * @param  The new region to use, of type Log_Region_t.
 * @return None.
 */
void Log_Module_Set_Region(Log_Region_t new_region);

/**
 * @brief  Add to the current region mask the given region.
 * @param  The new region to use, alongside the others, of type Log_Region_t.
 * @return None.
 */
void Log_Module_Add_Region(Log_Region_t new_region);

/**
 * @brief  Enable all the regions.
 * @param  None.
 * @return None.
 */
void Log_Module_Enable_All_Regions(void);

/**
 * @brief  Set/Replace the color for a region.
 * @param  eRegion    The region where apply the color, type Log_Region_t.
 * @param  eNewColor  The color to apply to selected region, type Log_Color_t.
 * @return None.
 */
void Log_Module_Set_Color(Log_Region_t eRegion, Log_Color_t eNewColor );

/**
 * @brief   Register a callback function to insert the 'TimeStamp' to the log.
 *
 * @param  pCallbackFunc    Callback function to insert Time Stamp.
 *                          This function is typedef void ( uint8_t * pData, uint16_t * piSize );
 *                          Where pData is the location where insert the new TimeStamp and piSize is the size of insert.
 * @return None.
 */
void Log_Module_RegisterTimeStampFunction( CallBack_TimeStamp * pCallbackFunc );

/* Module API - Wrapper function */
/**
 * @brief  Underlying function of all the LOG macros.
 *
 * @param  eVerboseLevel  The level of verbose for this Log.
 * @param  eRegion        The stack where the log is issued
 * @param  pText          Pointer to the text to be printed.
 * @param  Any other parameters to be printed with the text.
 *         E.g. an int variable. as 3rd parameter, as long as %d is in text.
 *
 * @return None.
 */
void Log_Module_Print( Log_Verbose_Level_t eVerboseLevel, Log_Region_t eRegion, const char * pText, ...);

/**
 * @brief  Function of log with already a arg list.
 *
 * @param  eVerboseLevel  The level of verbose for this Log.
 * @param  eRegion        The stack where the log is issued
 * @param  pText          Pointer to the text to be printed.
 * @param  args           Arguments list.
 *
 * @return None.
 */
void Log_Module_PrintWithArg( Log_Verbose_Level_t eVerboseLevel, Log_Region_t eRegion, const char * pText, va_list args );

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Exported macro ------------------------------------------------------------*/
/* Module API - Log macros for each region */
/* LOG_REGION_BLE */
#define LOG_INFO_BLE(...)         Log_Module_Print( LOG_VERBOSE_INFO, LOG_REGION_BLE, __VA_ARGS__)
#define LOG_ERROR_BLE(...)        Log_Module_Print( LOG_VERBOSE_ERROR, LOG_REGION_BLE, __VA_ARGS__)
#define LOG_WARNING_BLE(...)      Log_Module_Print( LOG_VERBOSE_WARNING, LOG_REGION_BLE, __VA_ARGS__)
#define LOG_DEBUG_BLE(...)        Log_Module_Print( LOG_VERBOSE_DEBUG, LOG_REGION_BLE, __VA_ARGS__)

/* USER CODE BEGIN LOG_REGION_BLE */
/**
 * Add inside this user section your defines to match the new verbose levels you
 * created into Log_Verbose_Level_t.
 * Example :
 * #define LOG_CUSTOM_BLE(...)      { Log_Module_t _tmp = { .verbose_level = LOG_VERBOSE_CUSTOM, .region = LOG_REGION_BLE }; _Log(_tmp, __VA_ARGS__);         }
 *
 * You don't need to update all regions with your custom values.
 * Do it accordingly to your needs. E.g you might not need LOG_VERBOSE_CUSTOM
 * for a System region.
 */

/* USER CODE END LOG_REGION_BLE */

/* LOG_REGION_SYSTEM */
#define LOG_INFO_SYSTEM(...)      Log_Module_Print( LOG_VERBOSE_INFO, LOG_REGION_SYSTEM, __VA_ARGS__)
#define LOG_ERROR_SYSTEM(...)     Log_Module_Print( LOG_VERBOSE_ERROR, LOG_REGION_SYSTEM, __VA_ARGS__)
#define LOG_WARNING_SYSTEM(...)   Log_Module_Print( LOG_VERBOSE_WARNING, LOG_REGION_SYSTEM, __VA_ARGS__)
#define LOG_DEBUG_SYSTEM(...)     Log_Module_Print( LOG_VERBOSE_DEBUG, LOG_REGION_SYSTEM, __VA_ARGS__)

/* USER CODE BEGIN LOG_REGION_SYSTEM */
/**
 * Add inside this user section your defines to match the new verbose levels you
 * created into Log_Verbose_Level_t.
 * Example :
 * #define LOG_CUSTOM_SYSTEM(...)      { Log_Module_t _tmp = { .verbose_level = LOG_VERBOSE_CUSTOM, .region = LOG_REGION_SYSTEM }; _Log(_tmp, __VA_ARGS__);         }
 *
 * You don't need to update all regions with your custom values.
 * Do it accordingly to your needs. E.g you might not need LOG_VERBOSE_CUSTOM
 * for a System region.
 */

/* USER CODE END LOG_REGION_SYSTEM */

/* LOG_REGION_APP */
#define LOG_INFO_APP(...)       Log_Module_Print( LOG_VERBOSE_INFO, LOG_REGION_APP, __VA_ARGS__)
#define LOG_ERROR_APP(...)      Log_Module_Print( LOG_VERBOSE_ERROR, LOG_REGION_APP, __VA_ARGS__)
#define LOG_WARNING_APP(...)    Log_Module_Print( LOG_VERBOSE_WARNING, LOG_REGION_APP, __VA_ARGS__)
#define LOG_DEBUG_APP(...)      Log_Module_Print( LOG_VERBOSE_DEBUG, LOG_REGION_APP, __VA_ARGS__)

/* USER CODE BEGIN LOG_REGION_APP */
/**
 * Add inside this user section your defines to match the new verbose levels you
 * created into Log_Verbose_Level_t.
 * Example :
 * #define LOG_CUSTOM_APP(...)      { Log_Module_t _tmp = { .verbose_level = LOG_VERBOSE_CUSTOM, .region = LOG_REGION_APP }; _Log(_tmp, __VA_ARGS__);         }
 *
 * You don't need to update all regions with your custom values.
 * Do it accordingly to your needs. E.g you might not need LOG_VERBOSE_CUSTOM
 * for a System region.
 */

/* USER CODE END LOG_REGION_APP */

/* USER CODE BEGIN APP_LOG_USER_DEFINES */
/**
 * Add inside this user section your defines to match the new regions you
 * created into Log_Region_t.
 * Example :
 * #define LOG_INFO_CUSTOM(...)       { Log_Module_t _tmp = { .verbose_level = LOG_VERBOSE_INFO, .region = LOG_REGION_CUSTOM }; _Log(_tmp, __VA_ARGS__);          }
 * #define LOG_ERROR_CUSTOM(...)      { Log_Module_t _tmp = { .verbose_level = LOG_VERBOSE_ERROR, .region = LOG_REGION_CUSTOM }; _Log(_tmp, __VA_ARGS__);         }
 * #define LOG_WARNING_CUSTOM(...)    { Log_Module_t _tmp = { .verbose_level = LOG_VERBOSE_WARNING, .region = LOG_REGION_CUSTOM }; _Log(_tmp, __VA_ARGS__);       }
 * #define LOG_DEBUG_CUSTOM(...)      { Log_Module_t _tmp = { .verbose_level = LOG_VERBOSE_DEBUG, .region = LOG_REGION_CUSTOM }; _Log(_tmp, __VA_ARGS__);         }
 */

/* USER CODE END APP_LOG_USER_DEFINES */

#ifdef __cplusplus
}
#endif

#endif /* LOG_MODULE_H */
