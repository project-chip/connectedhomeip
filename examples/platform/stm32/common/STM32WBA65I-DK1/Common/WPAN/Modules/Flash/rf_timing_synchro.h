/**
  ******************************************************************************
  * @file    rf_timing_synchro.h
  * @author  MCD Application Team
  * @brief   Header for rf_timing_synchro.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef RF_TIMING_SYNCHRO_H
#define RF_TIMING_SYNCHRO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

/* Exported types ------------------------------------------------------------*/

/* RFTS command status */
typedef enum
{
  RFTS_CMD_OK,              /* The RF Timing synchronization command was successfully executed */
  RFTS_WINDOW_REQ_FAILED,   /* The RF Timing synchronization module failed to register the window request */
  RFTS_WINDOW_REL_ERROR     /* An error occurred during the window release procedure */
} RFTS_Cmd_Status_t;

/* Exported constants --------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
RFTS_Cmd_Status_t RFTS_ReqWindow(uint32_t Duration, void (*Callback)(void));
RFTS_Cmd_Status_t RFTS_RelWindow(void);

#ifdef __cplusplus
}
#endif

#endif /*RF_TIMING_SYNCHRO_H */
