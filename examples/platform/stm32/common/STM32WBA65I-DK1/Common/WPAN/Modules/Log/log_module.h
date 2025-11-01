/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    log_module.h
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
#ifndef LOG_MODULE_H
#define LOG_MODULE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "log_module_conf.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* Log module types */
/**
 * @brief  Data type to initialize the module by calling Log_Module_Init.
 *         verbose_level : A value of type Log_Verbose_Level_t.
 *         region_mask   : A mask based on Log_Region_t.
 *                         You can directly assign it to LOG_REGION_ALL_REGIONS,
 *                         or select only some regions :
 *                         (1U << LOG_REGION_BLE | 1U << LOG_REGION_APP)
 */
typedef struct
{
  Log_Verbose_Level_t   verbose_level;
  uint32_t              region_mask;
} Log_Module_t;

/**
 * @brief  Callback function to insert Time Stamp.
 *
 * @param  Data                 The data into which to insert the TimeStamp.
 * @param  SizeMax              The maximum size for the TimeStamp insert.
 * @param  TimeStampSize        Pointer to update with the size of the TimeStamp inserted into data.
 */
typedef void CallBack_TimeStamp(char * Data, uint16_t SizeMax, uint16_t * TimeStampSize);

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
 *
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
 *
 * @param  LogConfiguration     The configuration of the log module, of type Log_Module_t.
 * @return None.
 */
void Log_Module_Init(Log_Module_t LogConfiguration);

/**
 * @brief  DeInitialization of the log module.
 *
 * @param  None.
 * @return None.
 */
void Log_Module_DeInit(void);

/**
 * @brief  Set the verbose level of the log.
 *         The levels include the lower levels in the logs.
 *
 *         E.g. LOG_VERBOSE_ERROR means LOG_VERBOSE_ERROR logs will be printed,
 *         as well as LOG_VERBOSE_INFO, but not the others with higher values.
 *
 * @param  NewVerboseLevel      The new verbose level to be set, of type Log_Verbose_Level_t
 * @return None
 */
void Log_Module_Set_Verbose_Level(Log_Verbose_Level_t NewVerboseLevel);

/**
 * @brief  Replace the current regions in use and only set the given region.
 *
 * @param  NewRegion            The new region to use, of type Log_Region_t.
 * @return None.
 */
void Log_Module_Set_Region(Log_Region_t NewRegion);

/**
 * @brief  Replace the current regions in use and set one or several as replacement.
 *
 * @param  NewRegionMask        A mask, of type uint32_t, where each bit corresponds to a region.
 *                              You can directly assign it to LOG_REGION_ALL_REGIONS to enable all of them,
 *                              or select only some regions, e.g. (1U << LOG_REGION_BLE | 1U << LOG_REGION_APP)
 * @return None.
 */
void Log_Module_Set_Multiple_Regions(uint32_t NewRegionMask);

/**
 * @brief  Add to the current region mask the given region.
 *
 * @param  NewRegion            The new region to use, alongside the others, of type Log_Region_t.
 * @return None.
 */
void Log_Module_Add_Region(Log_Region_t NewRegion);

/**
 * @brief  Remove from the current region mask the given region.
 *
 * @param  Region               The region to remove, of type Log_Region_t.
 * @return None.
 */
void Log_Module_Remove_Region(Log_Region_t Region);

/**
 * @brief  Enable all the regions.
 *
 * @param  None.
 * @return None.
 */
void Log_Module_Enable_All_Regions(void);

/**
 * @brief  Set the color for a region.
 *
 * @param  Region               The region where apply the color, type Log_Region_t.
 * @param  Color                The color to apply to selected region, of type Log_Color_t.
 * @return None.
 */
void Log_Module_Set_Color(Log_Region_t Region, Log_Color_t Color);

/**
 * @brief  Register a callback function to insert the TimeStamp into the data.
 *
 * @param  TimeStampFunction    Callback function to insert TimeStamp.
 *                              This function is typedef void (char * data, uint16_t size_max, uint16_t * timestamp_size);
 *                              Where data is the location where to insert the new TimeStamp, and timestamp_size is the size of this insertion.
 * @return None.
 */
void Log_Module_RegisterTimeStampFunction(CallBack_TimeStamp * TimeStampFunction);

/* Module API - Wrapper function */
/**
 * @brief  Underlying function of all the LOG_xxx macros.
 *
 * @param  VerboseLevel         The level of verbose used for this Log, of type Log_Verbose_Level_t.
 * @param  Region               The region set for this log, of type Log_Region_t.
 * @param  Text                 The text to be printed.
 * @param  ...                  Any other parameters to be printed with the text.
 *                              E.g. an int variable. as 3rd parameter, as long as %d is in text.
 *
 * @return None.
 */
void Log_Module_Print(Log_Verbose_Level_t VerboseLevel, Log_Region_t Region, const char * Text, ...);

/**
 * @brief  Function of log with already a arg list.
 *
 * @param  VerboseLevel         The level of verbose used for this Log, of type Log_Verbose_Level_t.
 * @param  Region               The region set for this log, of type Log_Region_t.
 * @param  Text                 The text to be printed
 * @param  Args                 Arguments list, of type va_list.
 *
 * @return None.
 */
void Log_Module_PrintWithArg(Log_Verbose_Level_t VerboseLevel, Log_Region_t Region, const char * Text, va_list Args);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* LOG_MODULE_H */
