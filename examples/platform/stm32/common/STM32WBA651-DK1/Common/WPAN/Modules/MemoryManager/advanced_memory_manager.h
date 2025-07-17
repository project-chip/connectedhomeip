/**
  ******************************************************************************
  * @file    advance_memory_manager.h
  * @author  MCD Application Team
  * @brief   Header for advance_memory_manager.c module
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
#ifndef ADVANCED_MEMORY_MANAGER_H
#define ADVANCED_MEMORY_MANAGER_H

/* Includes ------------------------------------------------------------------*/
#include "stm_list.h"

#ifdef __cplusplus
extern "C" {
#endif
/* Exported defines -----------------------------------------------------------*/

/* Define for No Virtual Memory ID */
#define AMM_NO_VIRTUAL_ID 0x00u

/**
 * @brief Size of a virtual memory information element in 32bits
 *
 * @details At initialization, the Advance Memory Manager allocate as many VM info element as there
 * is Virtual Memory to manage.
 * Thus user shall provide at the initialization more space for the pool.
 *
 * ie:
 * - sizeOfDesiredPool = 8092
 * - numberOfVirtualMemory = 3
 * - actualSizeOfThePoolToGive = sizeOfDesiredPool + numberOfVirtualMemory * AMM_VIRTUAL_INFO_ELEMENT_SIZE
 */
#define AMM_VIRTUAL_INFO_ELEMENT_SIZE 0x3u

/* Exported types ------------------------------------------------------------*/
/* Redefine the header for chained list */
typedef tListNode AMM_VirtualMemoryCallbackHeader_t;

/* Function error enumeration */
typedef enum AMM_Function_Error
{
  /* No error code */
  AMM_ERROR_OK,
  /* Error that occurred before any check */
  AMM_ERROR_NOK,
  /*Bad pointer error */
  AMM_ERROR_BAD_POINTER,
  /* Bad pool configuration error */
  AMM_ERROR_BAD_POOL_CONFIG,
  /* Bad virtual memory configuration error */
  AMM_ERROR_BAD_VIRTUAL_CONFIG,
  /* Bad BMM registration error */
  AMM_ERROR_BAD_BMM_REGISTRATION,
  /* Bad BMM allocation error */
  AMM_ERROR_BAD_BMM_ALLOCATION,
  /* Not aligned address error */
  AMM_ERROR_NOT_ALIGNED,
  /* Out of range error */
  AMM_ERROR_OUT_OF_RANGE,
  /* Unknown virtual memory manager ID error */
  AMM_ERROR_UNKNOWN_ID,
  /* Bad allocation size error */
  AMM_ERROR_BAD_ALLOCATION_SIZE,
  /* Allocation failed error */
  AMM_ERROR_ALLOCATION_FAILED,
  /* Already initialized AMM error */
  AMM_ERROR_ALREADY_INIT,
  /* Not initialized AMM error */
  AMM_ERROR_NOT_INIT
} AMM_Function_Error_t;

/**
 * @brief   Virtual Memory configuration struct
 */
typedef struct AMM_VirtualMemoryConfig
{
  /* ID of the Virtual Memory */
  uint8_t Id;
  /* Size of the Virtual Memory buffer with a multiple of 32bits.

     ie: BufferSize = 4; TotalSize = BufferSize * 32bits
                                   = 4 * 32bits
                                   = 128 bits */
  uint32_t BufferSize;
}AMM_VirtualMemoryConfig_t;

/**
 * @brief   Basic Memory Manager functions struct
 *
 * @details Structure that handles all the required functions of the Basic Memory Manager.
 */
typedef struct AMM_BasicMemoryManagerFunctions
{
  /* Basic Memory Manager Init function - Shall be in bytes - */
  void (* Init) (uint32_t * const p_PoolAddr, const uint32_t PoolSize);
  /* Basic Memory Manager Allocate function - Shall be in bytes - */
  uint32_t * (* Allocate) (const uint32_t BufferSize);
  /* Basic Memory Manager Free function */
  void (* Free) (uint32_t * const p_BufferAddr);
}AMM_BasicMemoryManagerFunctions_t;

/**
 * @brief   Advance Memory Manager init struct
 *
 * @details Structure that contains all the needed parameters to initialize the
 *          Advance Memory Manager.
 */
typedef struct AMM_InitParameters
{
  /* Address of the pool of memory to manage */
  uint32_t * p_PoolAddr;
  /* Size of the pool with a multiple of 32bits.

     ie: PoolSize = 4; TotalSize = PoolSize * 32bits
                                 = 4 * 32bits
                                 = 128 bits */
  uint32_t PoolSize;
  /* Number of Virtual Memory to create */
  uint32_t VirtualMemoryNumber;
  /* List of the Virtual Memory configurations */
  AMM_VirtualMemoryConfig_t * p_VirtualMemoryConfigList;
}AMM_InitParameters_t;

/**
 * @brief   Virtual Memory Callback function struct
 */
typedef struct AMM_VirtualMemoryCallbackFunction
{
  /* Next and previous callbacks in the list */
  AMM_VirtualMemoryCallbackHeader_t Header;
  /* Callback function pointer to invoke once memory has been freed */
  void (* Callback) (void);
}AMM_VirtualMemoryCallbackFunction_t;

/* Exported constants --------------------------------------------------------*/
/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief  Initialize the Advance Memory Manager Pool
 * @param  p_InitParams: Struct of init parameters
 * @return Status of the initialization
 * @retval AMM_Function_Error_t::AMM_ERROR_OK
 * @retval AMM_Function_Error_t::AMM_ERROR_NOK
 * @retval AMM_Function_Error_t::AMM_ERROR_ALREADY_INIT
 * @retval AMM_Function_Error_t::AMM_ERROR_BAD_POINTER
 * @retval AMM_Function_Error_t::AMM_ERROR_BAD_POOL_CONFIG
 * @retval AMM_Function_Error_t::AMM_ERROR_BAD_VIRTUAL_CONFIG
 * @retval AMM_Function_Error_t::AMM_ERROR_BAD_BMM_REGISTRATION
 * @retval AMM_Function_Error_t::AMM_ERROR_BAD_BMM_ALLOCATION
 */
AMM_Function_Error_t AMM_Init (const AMM_InitParameters_t * const p_InitParams);

/**
 * @brief  DeInitialize the Advance Memory Manager Pool
 * @details This function shall be called once every allocated memory has been freed
 * @return Status of the initialization
 * @retval AMM_Function_Error_t::AMM_ERROR_OK
 * @retval AMM_Function_Error_t::AMM_ERROR_NOK
 * @retval AMM_Function_Error_t::AMM_ERROR_NOT_INIT
 */
AMM_Function_Error_t AMM_DeInit (void);

/**
 * @brief  Allocate a buffer from the Advance Memory Manager Pool
 * @param  VirtualMemoryId: Virtual Memory Identifier - AMM_NO_VIRTUAL_ID Can be used -
 * @param  BufferSize: Size of the pool with a multiple of 32bits.
                       ie: BufferSize = 4; TotalSize = BufferSize * 32bits
                                                     = 4 * 32bits
                                                     = 128 bits
 * @param  p_CallBackFunction: Pointer onto the Callback to call in case of failure - Can be NULL -
 * @param  pp_AllocBuffer: Pointer onto the allocated buffer
 * @return Status of the allocation
 * @retval AMM_Function_Error_t::AMM_ERROR_OK
 * @retval AMM_Function_Error_t::AMM_ERROR_NOK
 * @retval AMM_Function_Error_t::AMM_ERROR_NOT_INIT
 * @retval AMM_Function_Error_t::AMM_ERROR_BAD_POINTER
 * @retval AMM_Function_Error_t::AMM_ERROR_UNKNOWN_ID
 * @retval AMM_Function_Error_t::AMM_ERROR_BAD_ALLOCATION_SIZE
 * @retval AMM_Function_Error_t::AMM_ERROR_ALLOCATION_FAILED
 */
AMM_Function_Error_t AMM_Alloc (const uint8_t VirtualMemoryId,
                                const uint32_t BufferSize,
                                uint32_t ** pp_AllocBuffer,
                                AMM_VirtualMemoryCallbackFunction_t * const p_CallBackFunction);

/**
 * @brief  Free the allocated buffer from the Advance Memory Manager Pool
 * @param  p_BufferAddr: Address of the buffer to free
 * @return Status of the free
 * @retval AMM_Function_Error_t::AMM_ERROR_OK
 * @retval AMM_Function_Error_t::AMM_ERROR_NOK
 * @retval AMM_Function_Error_t::AMM_ERROR_NOT_INIT
 * @retval AMM_Function_Error_t::AMM_ERROR_BAD_POINTER
 * @retval AMM_Function_Error_t::AMM_ERROR_NOT_ALIGNED
 * @retval AMM_Function_Error_t::AMM_ERROR_OUT_OF_RANGE
 * @retval AMM_Function_Error_t::AMM_ERROR_UNKNOWN_ID
 */
AMM_Function_Error_t AMM_Free (uint32_t * const p_BufferAddr);

/**
 * @brief  Background routine
 * @details Background routine that aims to call registered callbacks for an allocation retry
 * @return None
 */
void AMM_BackgroundProcess (void);

/* Exported functions to be implemented by the user if required ------------- */

/**
 * @brief  Register the Basic Memory Manager functions to use
 * @param  p_BasicMemoryManagerFunctions: Address of the basic memory manager functions
 * @return None
 */
void AMM_RegisterBasicMemoryManager (AMM_BasicMemoryManagerFunctions_t * const p_BasicMemoryManagerFunctions);

/**
 * @brief  Request to application for a Background process run
 * @return None
 */
void AMM_ProcessRequest (void);

#ifdef __cplusplus
}
#endif

#endif /* ADVANCED_MEMORY_MANAGER_H */
