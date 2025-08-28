/**
  ******************************************************************************
  * @file    flash_manager.h
  * @author  MCD Application Team
  * @brief   Header for flash_manager.c module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"
#include "stm_list.h"

/* Exported types ------------------------------------------------------------*/

/* Flash Manager command status */
typedef enum
{
  FM_OK,    /* The Flash Manager is available and a window request is scheduled */
  FM_BUSY,  /* The Flash Manager is busy and the caller will be called back when it is available */
  FM_ERROR  /* An error occurred while processing the command */
} FM_Cmd_Status_t;

/* Flash operation status */
typedef enum
{
  FM_OPERATION_COMPLETE,  /* The requested flash operation is complete */
  FM_OPERATION_AVAILABLE  /* A flash operation can be requested */
} FM_FlashOp_Status_t;

/**
 * @brief  Flash Manager callback node type to store them in a chained list
 */
typedef struct FM_CallbackNode
{
  tListNode NodeList;  /* Next and previous nodes in the list */
  void (*Callback)(FM_FlashOp_Status_t Status);  /* Callback function pointer for Flash Manager caller */
}FM_CallbackNode_t;

/* Exported constants --------------------------------------------------------*/

#define TIME_WINDOW_ERASE_DURATION 4000U  /* Duration in us of the time window requested for Flash Erase */
#define TIME_WINDOW_WRITE_DURATION 1000U  /* Duration in us of the time window requested for Flash Write */
#define TIME_WINDOW_MARGIN   100U  /* Time margin added so to ensure timeout protection before actual closure */
                                   /* As timers use ms, amount below 1000 is removed at conversion */
#define TIME_WINDOW_ERASE_REQUEST  (TIME_WINDOW_ERASE_DURATION + TIME_WINDOW_MARGIN)
#define TIME_WINDOW_WRITE_REQUEST  (TIME_WINDOW_WRITE_DURATION + TIME_WINDOW_MARGIN)

/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
FM_Cmd_Status_t FM_Write(uint32_t *Src, uint32_t *Dest, int32_t Size, FM_CallbackNode_t *CallbackNode);
FM_Cmd_Status_t FM_Write_ext(uint32_t *Src, uint32_t *Dest, int32_t Size, FM_CallbackNode_t *CallbackNode);
FM_Cmd_Status_t FM_Erase(uint32_t FirstSect, uint32_t NbrSect, FM_CallbackNode_t *CallbackNode);
void FM_BackgroundProcess (void);
void FM_ProcessRequest (void);

#ifdef __cplusplus
}
#endif

#endif /*FLASH_MANAGER_H */
