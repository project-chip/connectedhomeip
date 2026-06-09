/**
  ******************************************************************************
  * @file    simple_nvm_arbiter_common.h
  * @author  MCD Application Team
  * @brief   Common header of simple_nvm_arbiter.c module
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
#ifndef SIMPLE_NVM_ARBITER_COMMON_H
#define SIMPLE_NVM_ARBITER_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

/* Exported constants --------------------------------------------------------*/
/* Maximum number of different NVM Identifiers */
#define SNVMA_MAX_NUMBER_NVM              32u

/* Maximum number of buffer per NVM */
#define SNVMA_MAX_NUMBER_BUFFER           4u

/* Minimum number of bank per NVM */
#define SNVMA_MIN_NUMBER_BANK             2u

/* Exported types ------------------------------------------------------------*/

/* SNVMA command status */
typedef enum SNVMA_Cmd_Status
{
  SNVMA_ERROR_OK,
  SNVMA_ERROR_NOK,
  SNVMA_ERROR_NOT_INIT,
  SNVMA_ERROR_ALREADY_INIT,
  SNVMA_ERROR_CMD_PENDING,
  SNVMA_ERROR_BANK_OP_ONGOING,
  SNVMA_ERROR_NVM_NULL,
  SNVMA_ERROR_NVM_NOT_ALIGNED,
  SNVMA_ERROR_NVM_OVERLAP_FLASH,
  SNVMA_ERROR_NVM_BUFFER_FULL,
  SNVMA_ERROR_NVM_BANK_EMPTY,
  SNVMA_ERROR_NVM_BANK_CORRUPTED,
  SNVMA_ERROR_CRC_INIT,
  SNVMA_ERROR_BANK_NUMBER,
  SNVMA_ERROR_BANK_SIZE,
  SNVMA_ERROR_BUFFERID_NOT_KNOWN,
  SNVMA_ERROR_BUFFERID_NOT_REGISTERED,
  SNVMA_ERROR_BUFFER_NULL,
  SNVMA_ERROR_BUFFER_NOT_ALIGNED,
  SNVMA_ERROR_BUFFER_SIZE,
  SNVMA_ERROR_BUFFER_CONFIG_MISSMATCH,
  SNVMA_ERROR_FLASH_ERROR,
  SNVMA_ERROR_UNKNOWN,
} SNVMA_Cmd_Status_t;

/* SNVMA callback status */
typedef enum SNVMA_Callback_Status
{
  SNVMA_OPERATION_COMPLETE,
  SNVMA_OPERATION_FAILED
}SNVMA_Callback_Status_t;

/* Buffer Element */
typedef struct SNVMA_BufferElt
{
  /* Buffer address */
  uint32_t * p_Addr;
  /* Buffer size */
  uint32_t Size;
}SNVMA_BufferElt_t;

/* Bank Element */
typedef struct SNVMA_BankElt
{
  /* Pointer onto the start address of the bank */
  uint32_t * p_StartAddr;
  /* Pointer onto the buffer address in the bank */
  uint32_t * ap_BufferAddr[SNVMA_MAX_NUMBER_BUFFER];
}SNVMA_BankElt_t;

/* NVM Element */
typedef struct SNVMA_NvmElt
{
  /* Bank number */
  uint8_t BankNumber;
  /* Bank size */
  uint8_t BankSize;
  /*
   *  Pending buffer write operations bit mask
   *
   *  ----------------------------------------
   *  +      4 bits      |      4 bits       +
   *  + ------------------------------------ +
   *  +  Active Request  |    New Request    +
   *  ----------------------------------------
   *
   */
  uint8_t PendingBufferWriteOp;
  /* Pointer onto the bank list */
  SNVMA_BankElt_t * p_BankList;
  /* Pointer onto the bank being used for write operation */
  SNVMA_BankElt_t * p_BankForWrite;
  /* Pointer onto the bank being used for restore operation */
  SNVMA_BankElt_t * p_BankForRestore;
  /* Callback pointer array */
  void (* a_Callback [SNVMA_MAX_NUMBER_BUFFER]) (SNVMA_Callback_Status_t);
  /* Array of buffers in use */
  SNVMA_BufferElt_t a_Buffers [SNVMA_MAX_NUMBER_BUFFER];
}SNVMA_NvmElt_t;

/* Exported variables --------------------------------------------------------*/

/* Exported macros -----------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /*SIMPLE_NVM_ARBITER_COMMON_H */
