
/**
 ******************************************************************************
 * @file    stm_logging.c
 * @author  MCD Application Team
 * @brief   This file contains all the defines and functions used for logging
 *          on Application examples.
 *
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/**
 * @file
 *   This file implements logging functions to used in Application examples.
 *
 */

#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "app_conf.h"
#include "stm_logging.h"

#define LOG_PARSE_BUFFER_SIZE 256U

#define LOG_TIMESTAMP_ENABLE 0
#define LOG_REGION_ENABLE 1U
#define LOG_RTT_COLOR_ENABLE 1U

#if (LOG_RTT_COLOR_ENABLE == 1U)
#define RTT_COLOR_CODE_DEFAULT "\x1b[0m"
#define RTT_COLOR_CODE_RED "\x1b[0;91m"
#define RTT_COLOR_CODE_GREEN "\x1b[0;92m"
#define RTT_COLOR_CODE_YELLOW "\x1b[0;93m"
#define RTT_COLOR_CODE_CYAN "\x1b[0;96m"

#else /* LOG_RTT_COLOR_ENABLE == 1 */
#define RTT_COLOR_CODE_DEFAULT ""
#define RTT_COLOR_CODE_RED ""
#define RTT_COLOR_CODE_GREEN ""
#define RTT_COLOR_CODE_YELLOW ""
#define RTT_COLOR_CODE_CYAN ""
#endif /* LOG_RTT_COLOR_ENABLE == 1 */

#if (CFG_DEBUG_TRACE != 0)
/**
 * Function for outputting code region string.
 *
 * @param[inout]  aLogString  Pointer to log buffer.
 * @param[in]     aMaxSize    Maximum size of log buffer.
 * @param[in]     otLogRegion The region ID.
 *
 * @returns  String with a log level color value.
 */
static inline uint16_t logRegion(char * aLogString, uint16_t aMaxSize, appliLogRegion_t aLogRegion)
{
    char logRegionString[30U];

    switch (aLogRegion)
    {
    case APPLI_LOG_REGION_GENERAL:
        strcpy(logRegionString, "[M4 APPLICATION]");
        break;
    case APPLI_LOG_REGION_OPENTHREAD_API:
        strcpy(logRegionString, "[M4 OPENTHREAD API]");
        break;
    case APPLI_LOG_REGION_OT_API_LINK:
        strcpy(logRegionString, "[M4 LINK API]");
        break;
    case APPLI_LOG_REGION_OT_API_INSTANCE:
        strcpy(logRegionString, "[M4 INSTANCE API]");
        break;
    case APPLI_LOG_REGION_OT_API_MESSAGE:
        strcpy(logRegionString, "[M4 MESSAGE API]");
        break;
    default:
        strcpy(logRegionString, "[M4]");
        break;
    }

    return snprintf(aLogString, aMaxSize, "%s ", logRegionString);
}
#endif /* CFG_DEBUG_TRACE */

#if (LOG_RTT_COLOR_ENABLE == 1U)
#if (CFG_DEBUG_TRACE != 0)
/**
 * Function for getting color of a given level log.
 *
 * @param[in]  aLogLevel The log level.
 *
 * @returns  String with a log level color value.
 */
static inline const char * levelToString(appliLogLevel_t aLogLevel)
{
    switch (aLogLevel)
    {
    case LOG_LEVEL_CRIT:
        return RTT_COLOR_CODE_RED;

    case LOG_LEVEL_WARN:
        return RTT_COLOR_CODE_YELLOW;

    case LOG_LEVEL_INFO:
        return RTT_COLOR_CODE_GREEN;

    case LOG_LEVEL_DEBG:
    default:
        return RTT_COLOR_CODE_DEFAULT;
    }
}
#endif /* CFG_DEBUG_TRACE */

#if (CFG_DEBUG_TRACE != 0)
/**
 * Function for printing log level.
 *
 * @param[inout]  aLogString  Pointer to log buffer.
 * @param[in]     aMaxSize    Maximum size of log buffer.
 * @param[in]     aLogLevel   Log level.
 *
 * @returns  Number of bytes successfully written to the log buffer.
 */
static inline uint16_t logLevel(char * aLogString, uint16_t aMaxSize, appliLogLevel_t aLogLevel)
{
    return snprintf(aLogString, aMaxSize, "%s", levelToString(aLogLevel));
}
#endif /* CFG_DEBUG_TRACE */
#endif /* LOG_RTT_COLOR_ENABLE */

#if (LOG_TIMESTAMP_ENABLE == 1U)
/**
 * Function for printing actual timestamp.
 *
 * @param[inout]  aLogString Pointer to the log buffer.
 * @param[in]     aMaxSize   Maximum size of the log buffer.
 *
 * @returns  Number of bytes successfully written to the log buffer.
 */
static inline uint16_t logTimestamp(char * aLogString, uint16_t aMaxSize)
{
    return snprintf(aLogString, aMaxSize, "%s[%010ld]", RTT_COLOR_CODE_DEFAULT, otPlatAlarmMilliGetNow());
}
#endif /* LOG_TIMESTAMP_ENABLE */

/**
 * Function for printing application log
 *
 * @param[in]     aLogLevel   Log level.
 * @param[in]     aLogRegion  The region ID.
 * @param[in]     aFormat     User string format.
 *
 * @returns  Number of bytes successfully written to the log buffer.
 */
void logApplication(appliLogLevel_t aLogLevel, appliLogRegion_t aLogRegion, const char * aFormat, ...)
{
#if (CFG_DEBUG_TRACE != 0) /* Since the traces are disabled, there is nothing to print */
    uint16_t length = 0;
    char logString[LOG_PARSE_BUFFER_SIZE + 1U];

#if (LOG_TIMESTAMP_ENABLE == 1U)
    length += logTimestamp(logString, LOG_PARSE_BUFFER_SIZE);
#endif

#if (LOG_RTT_COLOR_ENABLE == 1U)
    /* Add level information */
    length += logLevel(&logString[length], (LOG_PARSE_BUFFER_SIZE - length), aLogLevel);
#endif

#if (LOG_REGION_ENABLE == 1U)
    /* Add Region information */
    length += logRegion(&logString[length], (LOG_PARSE_BUFFER_SIZE - length), aLogRegion);
#endif

    /* Parse user string */
    va_list paramList;
    va_start(paramList, aFormat);
    length += vsnprintf(&logString[length], (LOG_PARSE_BUFFER_SIZE - length), aFormat, paramList);
    logString[length++] = '\r';
    logString[length++] = '\n';
    logString[length++] = 0;
    va_end(paramList);

    if (aLogLevel <= APPLI_CONFIG_LOG_LEVEL)
    {
        printf("%s", logString);
    }
    else
    {
        /* Print nothing */
    }
#endif /* CFG_DEBUG_TRACE */
}
