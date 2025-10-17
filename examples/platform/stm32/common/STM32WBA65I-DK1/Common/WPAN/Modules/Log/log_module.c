/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    log_module.c
  * @author  MCD Application Team
  * @brief   Source file of the log module.
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

/* Includes ------------------------------------------------------------------*/
#include <stdio.h> /* vsnprintf */

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
#define ENDOFLINE_SIZE          (0x01u)
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
  .region_mask = (LOG_REGION_ALL_REGIONS)
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
  /* USER CODE BEGIN LOG_COLOR_DEFAULT_CONFIGURATION */

  /* USER CODE END LOG_COLOR_DEFAULT_CONFIGURATION */
};
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Private variables ---------------------------------------------------------*/
static uint32_t                 current_region_mask;
static Log_Verbose_Level_t      current_verbose_level;
static Log_Color_t              current_color_list[32];
CallBack_TimeStamp *            log_timestamp_function;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static uint32_t Get_Region_Mask(Log_Region_t Region);

#if (LOG_INSERT_COLOR_INSIDE_THE_TRACE != 0)
static uint16_t RegionToColor(char * TextBuffer, uint16_t SizeMax, Log_Region_t Region);
#endif /* LOG_INSERT_COLOR_INSIDE_THE_TRACE != 0  */
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
#if (LOG_INSERT_COLOR_INSIDE_THE_TRACE != 0)
/**
 * @brief Add the color (in function of Region) on the start of Log sentence.
 *
 * @param TextBuffer    Pointer on the log buffer
 * @param SizeMax       The maximum number of bytes that will be written to the buffer.
 * @param Region        Region of the log to apply its corresponding color.
 *
 * @return Length of the new Log.
 */
static uint16_t RegionToColor(char * TextBuffer, uint16_t SizeMax, Log_Region_t Region)
{
  uint16_t              text_length = 0;
  Log_Color_t           color;
  static Log_Color_t    previous_color = LOG_COLOR_NONE;

  if (Region != LOG_MODULE_ALL_REGION_MASK)
  {
    color = current_color_list[Region];
  }
  else
  {
    color = LOG_COLOR_CODE_DEFAULT;
  }

  /* Insert Color code only if previous is not the same */
  if (color != previous_color)
  {
    if (color == LOG_COLOR_CODE_DEFAULT)
    {
      snprintf(TextBuffer, SizeMax, "\x1b[0m");
    }
    else
    {
      snprintf(TextBuffer, SizeMax, "\x1b[0;%02dm", color);
    }

    previous_color = color;
    text_length = strlen(TextBuffer);
  }

  return text_length;
}
#endif /* LOG_INSERT_COLOR_INSIDE_THE_TRACE != 0  */

void Log_Module_PrintWithArg(Log_Verbose_Level_t VerboseLevel, Log_Region_t Region, const char * Text, va_list Args)
{
  uint16_t tmp_size = 0;
  uint16_t buffer_size = 0;
  char full_text[UTIL_ADV_TRACE_TMP_BUF_SIZE + 1u];

  /* USER CODE BEGIN Log_Module_PrintWithArg_1 */

  /* USER CODE END Log_Module_PrintWithArg_1 */

  /* If the verbose level of the given log is not enabled, then we do not print the log */
  if (VerboseLevel > current_verbose_level)
  {
    return;
  }

  /* If the region for the given log is not enabled, then we do not print the log */
  if ((Get_Region_Mask(Region) & current_region_mask) == 0u)
  {
    return;
  }

#if (LOG_INSERT_COLOR_INSIDE_THE_TRACE != 0)
  /* Add to full_text the color matching the region */
  tmp_size = RegionToColor(&full_text[buffer_size], (UTIL_ADV_TRACE_TMP_BUF_SIZE - buffer_size), Region);
  buffer_size += tmp_size;
#endif /* LOG_INSERT_COLOR_INSIDE_THE_TRACE != 0 */

#if (LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE != 0)
  if (log_timestamp_function != NULL)
  {
     tmp_size = UTIL_ADV_TRACE_TMP_BUF_SIZE - buffer_size;
     log_timestamp_function(&full_text[buffer_size], tmp_size, &tmp_size);
     buffer_size += tmp_size;
  }
#endif /* LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE != 0 */

  /* Copy the data */
  tmp_size = (uint16_t)vsnprintf(&full_text[buffer_size], (UTIL_ADV_TRACE_TMP_BUF_SIZE - buffer_size), Text, Args);
  buffer_size += tmp_size;

  /* USER CODE BEGIN Log_Module_PrintWithArg_2 */

  /* USER CODE END Log_Module_PrintWithArg_2 */

#if (LOG_INSERT_EOL_INSIDE_THE_TRACE != 0)
  /* Add End Of Line if needed */
  if (buffer_size > 1)
  {
    if ((full_text[buffer_size - 1] != ENDOFLINE_CHAR) && (full_text[buffer_size - 2] != ENDOFLINE_CHAR))
    {
      full_text[buffer_size++] = ENDOFLINE_CHAR;
      full_text[buffer_size] = 0;
    }
  }
#endif /* LOG_INSERT_EOL_INSIDE_THE_TRACE != 0 */

  /* USER CODE BEGIN Log_Module_PrintWithArg_3 */

  /* USER CODE END Log_Module_PrintWithArg_3 */

  /* Send full_text to ADV Traces */
  UTIL_ADV_TRACE_Send((const uint8_t *)full_text, buffer_size);
}

void Log_Module_Print(Log_Verbose_Level_t VerboseLevel, Log_Region_t Region, const char * Text, ...)
{
#if (CFG_LOG_SUPPORTED != 0)
  va_list variadic_args;

  va_start(variadic_args, Text);
  Log_Module_PrintWithArg(VerboseLevel, Region, Text, variadic_args);
  va_end(variadic_args);
#else /* (CFG_LOG_SUPPORTED != 0) */
  UNUSED(VerboseLevel);
  UNUSED(Region);
  UNUSED(Text);
#endif /* (CFG_LOG_SUPPORTED != 0)  */
}

void Log_Module_Init(Log_Module_t LogConfiguration)
{
  UTIL_ADV_TRACE_Init();

  memcpy(&current_color_list, &LOG_COLOR_DEFAULT_CONFIGURATION, sizeof(LOG_COLOR_DEFAULT_CONFIGURATION));
  Log_Module_Set_Verbose_Level(LogConfiguration.verbose_level);
  Log_Module_Set_Multiple_Regions(LogConfiguration.region_mask);
  log_timestamp_function = NULL;
}

void Log_Module_DeInit(void)
{
  UTIL_ADV_TRACE_DeInit();
}

void Log_Module_Set_Verbose_Level(Log_Verbose_Level_t NewVerboseLevel)
{
  current_verbose_level = NewVerboseLevel;
}

void Log_Module_Set_Region(Log_Region_t NewRegion)
{
  current_region_mask = Get_Region_Mask(NewRegion);
}

void Log_Module_Add_Region(Log_Region_t NewRegion)
{
  current_region_mask |= Get_Region_Mask(NewRegion);
}

void Log_Module_Remove_Region(Log_Region_t Region)
{
  current_region_mask &= ~Get_Region_Mask(Region);
}

void Log_Module_Enable_All_Regions(void)
{
  Log_Module_Set_Region(LOG_REGION_ALL_REGIONS);
}

static uint32_t Get_Region_Mask(Log_Region_t Region)
{
  if (Region == LOG_REGION_ALL_REGIONS)
  {
    /* Return the full mask */
    return ((uint32_t)LOG_MODULE_ALL_REGION_MASK);
  }
  else
  {
    /* Return the bit matching the region */
    return ((uint32_t)(1U << ((uint32_t)Region)));
  }
}

void Log_Module_Set_Multiple_Regions(uint32_t NewRegionMask)
{
  current_region_mask = NewRegionMask;
}

void Log_Module_Set_Color(Log_Region_t Region, Log_Color_t Color)
{
  if ( Region != LOG_MODULE_ALL_REGION_MASK )
  {
    current_color_list[Region] = Color;
  }
}

void Log_Module_RegisterTimeStampFunction(CallBack_TimeStamp * TimeStampFunction)
{
  log_timestamp_function = TimeStampFunction;
}

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */
