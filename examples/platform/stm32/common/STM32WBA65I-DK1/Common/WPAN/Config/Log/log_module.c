/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    log_module.c
  * @author  MCD Application Team
  * @brief   Source file of the log module.
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h> /* vsnprintf */

#include "app_conf.h"
#include "log_module.h"
#include "stm32_adv_trace.h"
#include "utilities_conf.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* Definition of 'End Of Line' */
#define ENDOFLINE_SIZE          0x01u
#define ENDOFLINE_CHAR          '\n'
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Exported constants --------------------------------------------------------*/
/* Global const struct variables to make the life of the user easier */
const Log_Module_t LOG_MODULE_DEFAULT_CONFIGURATION =
{
  .verbose_level = LOG_VERBOSE_ERROR,
  .region = LOG_REGION_ALL_REGIONS
};
const Log_Verbose_Level_t LOG_VERBOSE_DEFAULT = LOG_VERBOSE_ERROR;
const Log_Region_t LOG_REGION_MASK_DEFAULT = LOG_REGION_ALL_REGIONS;
const Log_Color_t LOG_COLOR_DEFAULT_CONFIGURATION[] =
{
  LOG_COLOR_CODE_DEFAULT,   // For Region BLE
  LOG_COLOR_CODE_DEFAULT,   // For Region System
  LOG_COLOR_CODE_DEFAULT,   // For Region APP
  LOG_COLOR_CODE_RED,       // For Region LinkLayer
  LOG_COLOR_CODE_YELLOW,    // For Region MAC
  LOG_COLOR_CODE_GREEN,     // For Region Zigbee
  LOG_COLOR_CODE_GREEN,     // For Region Thread
  LOG_COLOR_CODE_DEFAULT,   // For Region RTOS
};

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Private variables ---------------------------------------------------------*/
uint32_t              lLogCurrentRegionMask;
Log_Verbose_Level_t   eLogCurrentVerboseLevel;
Log_Color_t           eLogCurrentColorList[32];
CallBack_TimeStamp    * pLogTimeStampFunc;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static uint32_t Get_Region_Mask(Log_Region_t region);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/

#if ( LOG_INSERT_COLOR_INSIDE_THE_TRACE != 0 )

/**
 * @brief Add the color (in function of Region) on the start of Log sentence.
 *
 * @param szBuffer    Pointer on Log buffer
 * @param eRegion     Region of Log (listed in Log_Region_t)
 *
 * @return Length of the new Log.
 */
static uint16_t RegionToColor( char * szBuffer, Log_Region_t eRegion )
{
  uint16_t            iLength = 0;
  Log_Color_t         eColor;
  static Log_Color_t  ePreviousColor = LOG_COLOR_NONE;

  if ( eRegion != LOG_MODULE_ALL_REGION_MASK )
  {
    eColor = eLogCurrentColorList[eRegion];
  }
  else
  {
    eColor = LOG_COLOR_CODE_DEFAULT;
  }

  /* Insert Color code only if previous is not the same */
  if ( eColor != ePreviousColor )
  {
    if ( eColor == LOG_COLOR_CODE_DEFAULT )
      { sprintf( szBuffer, "\x1b[0m" ); }
    else
      { sprintf( szBuffer, "\x1b[0;%02dm", eColor ); }

    ePreviousColor = eColor;
    iLength = strlen( szBuffer );
  }

  return( iLength );
}

#endif /* LOG_INSERT_COLOR_INSIDE_THE_TRACE  */

/**
 *
 */
void Log_Module_PrintWithArg( Log_Verbose_Level_t eVerboseLevel, Log_Region_t eRegion, const char * pText, va_list args )
{
  uint16_t  iTempSize, iBuffSize = 0u;
  char      szFullText[UTIL_ADV_TRACE_TMP_BUF_SIZE];

  /**
   * This user section can be used to insert a guard clauses design pattern
   * if you want to modify how is handled the verbose.
   * E.g. By sewwlecting ERROR, you only want ERROR logs and not ERROR + INFO logs.
   *
   * Example of how to do it :
   *
   * // If the log Level isn't matching the configured one, then we don't log,
   * // excepted for LOG_VERBOSE_ALL_LOGS where we log in all cases.
   * uint8_t current_verbose = UTIL_ADV_TRACE_GetVerboseLevel();
   * if ((log_configuration.verbose_level != current_verbose) && (current_verbose != (uint8_t)LOG_VERBOSE_ALL_LOGS)) { return; }
   */
  /* USER CODE BEGIN Log_Module_PrintWithArg_1 */

  /* USER CODE END Log_Module_PrintWithArg_1 */

  /* Check verbose level */
  if ( eVerboseLevel > eLogCurrentVerboseLevel )
  {
    return;
  }

  /* Check Region */
  if ( ( Get_Region_Mask( eRegion ) & lLogCurrentRegionMask ) == 0u )
  {
    return;
  }

#if ( LOG_INSERT_COLOR_INSIDE_THE_TRACE != 0 )
  /* Add Color in function of Region */
  iTempSize = RegionToColor( &szFullText[iBuffSize], eRegion );
  iBuffSize += iTempSize;
#endif /* LOG_INSERT_COLOR_INSIDE_THE_TRACE */

#if ( LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE != 0 )
  if ( pLogTimeStampFunc != NULL )
  {
     pLogTimeStampFunc( &szFullText[iBuffSize], &iTempSize );
     iBuffSize += iTempSize;
  }
#endif /* LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE */

  /* Copy the data */
  iTempSize = (uint16_t)UTIL_ADV_TRACE_VSNPRINTF( &szFullText[iBuffSize], ( UTIL_ADV_TRACE_TMP_BUF_SIZE - iBuffSize ), pText, args );
  iBuffSize += iTempSize;

  /* USER CODE BEGIN Log_Module_PrintWithArg_2 */

  /* USER CODE END Log_Module_PrintWithArg_2 */

#if ( LOG_INSERT_EOL_INSIDE_THE_TRACE != 0 )
  /* Add End Of Line if needed */
  if ( ( szFullText[iBuffSize - 1] != ENDOFLINE_CHAR ) && ( szFullText[iBuffSize - 2] != ENDOFLINE_CHAR ) )
  {
    szFullText[iBuffSize++] = ENDOFLINE_CHAR;
    szFullText[iBuffSize] = 0;
  }
#endif /* LOG_INSERT_EOL_INSIDE_THE_TRACE */

  /* Send full_text to ADV Traces */
  UTIL_ADV_TRACE_Send( (const uint8_t *)szFullText, iBuffSize );
}

/**
 *
 */
void Log_Module_Print( Log_Verbose_Level_t eVerboseLevel, Log_Region_t eRegion, const char * pText, ...)
{
#if (CFG_LOG_SUPPORTED != 0)
  va_list   variadic_args;

  va_start( variadic_args, pText );
  Log_Module_PrintWithArg( eVerboseLevel, eRegion, pText, variadic_args );
  va_end( variadic_args );
#else /* (CFG_LOG_SUPPORTED != 0) */
  UNUSED(eVerboseLevel);
  UNUSED(eRegion);
  UNUSED(pText);
#endif /* (CFG_LOG_SUPPORTED != 0)  */
}

/**
 *
 */
void Log_Module_Init(Log_Module_t log_configuration)
{
  UTIL_ADV_TRACE_Init();

  memcpy( &eLogCurrentColorList, &LOG_COLOR_DEFAULT_CONFIGURATION, sizeof(LOG_COLOR_DEFAULT_CONFIGURATION) );
  Log_Module_Set_Verbose_Level(log_configuration.verbose_level);
  Log_Module_Set_Region(log_configuration.region);
  pLogTimeStampFunc = NULL;
}

/**
 *
 */
void Log_Module_DeInit(void)
{
  UTIL_ADV_TRACE_DeInit();
}

/**
 *
 */
void Log_Module_Set_Verbose_Level(Log_Verbose_Level_t new_verbose_level)
{
  eLogCurrentVerboseLevel = new_verbose_level;
}

/**
 *
 */
void Log_Module_Set_Region(Log_Region_t new_region)
{
  lLogCurrentRegionMask = Get_Region_Mask(new_region);
}

/**
 *
 */
void Log_Module_Add_Region(Log_Region_t new_region)
{
  lLogCurrentRegionMask |= Get_Region_Mask(new_region);
}

/**
 *
 */
void Log_Module_Enable_All_Regions(void)
{
  Log_Module_Set_Region(LOG_REGION_ALL_REGIONS);
}

/**
 *
 */
static uint32_t Get_Region_Mask(Log_Region_t region)
{
  if (region == LOG_REGION_ALL_REGIONS)
  {
    /* Return the full mask */
    return ((uint32_t)LOG_MODULE_ALL_REGION_MASK);
  }
  else
  {
    /* Return the bit matching the region */
    return ((uint32_t)(1U << ((uint32_t)region - 1U)));
  }
}

/**
 *
 */
void Log_Module_Set_Color(Log_Region_t eRegion, Log_Color_t eNewColor )
{
  if ( eRegion != LOG_MODULE_ALL_REGION_MASK )
  {
    eLogCurrentColorList[eRegion] = eNewColor;
  }
}

/**
 *
 */
void Log_Module_RegisterTimeStampFunction( CallBack_TimeStamp * pCallbackFunc )
{
  pLogTimeStampFunc = pCallbackFunc;
}

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
