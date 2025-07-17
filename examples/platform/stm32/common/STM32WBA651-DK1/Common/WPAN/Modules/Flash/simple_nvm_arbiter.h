/**
  ******************************************************************************
  * @file    simple_nvm_arbiter.h
  * @author  MCD Application Team
  * @brief   Header for simple_nvm_arbiter.c module
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
#ifndef SIMPLE_NVM_ARBITER_H
#define SIMPLE_NVM_ARBITER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "simple_nvm_arbiter_common.h"
#include "simple_nvm_arbiter_conf.h"

#include "utilities_common.h"

/* Exported constants --------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/**
 * @brief  Initialize the Simple NVM Arbiter
 *
 * @param p_NvmStartAddress: Start address of the NVM to work with - Shall be aligned 128 bits
 *
 * @return Status of the command
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_OK
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NOK
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_ALREADY_INIT
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NVM_NULL
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NVM_NOT_ALIGNED
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NVM_OVERLAP_FLASH
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_CRC_INIT
 */
SNVMA_Cmd_Status_t SNVMA_Init (const uint32_t * p_NvmStartAddress);

/**
 * @brief  Register a user buffer to a NVM
 *
 * @details Buffer IDs are hardcoded, please refer to SNVMA_BufferId_t enumeration
 *
 * @param BufferId: Id of the user which ask for buffer registration
 * @param p_BufferAddress: Address of the buffer to be registered - Shall be aligned 32 bits
 * @param BufferSize: Size of the buffer to be registered - Shall be a multiple of 32bits
 *
 * @return Status of the command
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_OK
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NOK
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NOT_INIT
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_CMD_PENDING
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_BUFFERID_NOT_KNOWN
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_BUFFER_NULL
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_BUFFER_NOT_ALIGNED
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_BUFFER_SIZE
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NVM_BUFFER_FULL
 */
SNVMA_Cmd_Status_t SNVMA_Register (const SNVMA_BufferId_t BufferId,
                                   const uint32_t * p_BufferAddress,
                                   const uint32_t BufferSize);

/**
 * @brief  Restore a user buffer from a NVM
 *
 * @details The user buffer information shall first be provided by calling SNVMA_Register
 *
 * @details Buffer IDs are hardcoded, please refer to SNVMA_BufferId_t enumeration
 *
 * @param BufferId: Id of the user which ask for buffer registration
 *
 * @return Status of the command
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_OK
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NOK
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NOT_INIT
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_CMD_PENDING
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_BUFFERID_NOT_KNOWN
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_BUFFERID_NOT_REGISTERED
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NVM_BANK_EMPTY
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NVM_BANK_CORRUPTED
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_BUFFER_CONFIG_MISSMATCH
 */
SNVMA_Cmd_Status_t SNVMA_Restore (const SNVMA_BufferId_t BufferId);

/**
 * @brief  Register a user buffer to a NVM
 *
 * @details The user buffer information shall first be provided by calling SNVMA_Register
 *
 * @details Buffer IDs are hardcoded, please refer to SNVMA_BufferId_t enumeration
 *
 * @details A buffer write request cannot be scheduled once its NVM is already on a write operation. This will lead
 *          to a SNVMA_OPERATION_FAILED callback status.
 *
 * @param BufferId: Id of the user which ask for buffer registration
 * @param Callback: Callback function for operation status return - Can be NULL
 *
 * @return Status of the command
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_OK
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NOK
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_NOT_INIT
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_BUFFERID_NOT_KNOWN
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_BUFFERID_NOT_REGISTERED
 * @retval SNVMA_Cmd_Status_t::SNVMA_ERROR_FLASH_ERROR
 */
SNVMA_Cmd_Status_t SNVMA_Write (const SNVMA_BufferId_t BufferId,
                                void (* Callback) (SNVMA_Callback_Status_t));

#ifdef __cplusplus
}
#endif

#endif /*SIMPLE_NVM_ARBITER_H */
