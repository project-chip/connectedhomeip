/**
  ******************************************************************************
  * @file    advanced_memory_manager.c
  * @author  MCD Application Team
  * @brief   Memory Manager
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

/* Includes ------------------------------------------------------------------*/
#include "stm_list.h"
#include "utilities_conf.h"
#include "advanced_memory_manager.h"

/* Private typedef -----------------------------------------------------------*/

/**
 * @brief   Virtual Memory information struct
 */
typedef struct __attribute__((packed, aligned(4))) VirtualMemoryInfo
{
  /* Identifier of the Virtual Memory */
  uint8_t Id;
  /* Size required for this Virtual Memory buffer with a multiple of 32bits */
  uint32_t RequiredSize;
  /* Current occupation of the Virtual Memory buffer with a multiple of 32bits */
  uint32_t OccupiedSize;
}VirtualMemoryInfo_t;

/* Private defines -----------------------------------------------------------*/

/* Defines that the module is not initialized */
#define NOT_INITIALIZED 0x00u
/* Defines that the module is initialized */
#define INITIALIZED     0x01u

/* Defines the bit to check for 32bits aligned values */
#define MASK_ALIGNED_32BITS  0x03u

/*
  Defines the length of the Virtual Memory Header in 32bits
    ----------------------------------------
   | +++ 8bits +++ | +++++++ 24bits +++++++ |
   | VirtualMem ID | Buffer Size (n*32bits) |
    ----------------------------------------
 */
#define VIRTUAL_MEMORY_HEADER_SIZE    0x01

/* Position of the ID field in Virtual Memory Header */
#define VIRTUAL_MEMORY_HEADER_ID_POS  0x18
/* Mask of the ID field in Virtual Memory Header */
#define VIRTUAL_MEMORY_HEADER_ID_MASK 0xFF000000

/* Position of the Buffer Size field in Virtual Memory Header */
#define VIRTUAL_MEMORY_HEADER_BUFFER_SIZE_POS  0x00
/* Mask of the Buffer Size field in Virtual Memory Header */
#define VIRTUAL_MEMORY_HEADER_BUFFER_SIZE_MASK 0x00FFFFFF

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* AMM Initialized flag */
static uint8_t AmmInitialized = NOT_INITIALIZED;

/* AMM pool address */
static uint32_t * p_AmmPoolAddress;

/* AMM pool size */
static uint32_t AmmPoolSize;

/* AMM occupied shared pool size */
static uint32_t AmmOccupiedSharedPoolSize;

/* AMM needed virtual memory */
static uint32_t AmmRequiredVirtualMemorySize;

/* AMM virtual memory number */
static uint32_t AmmVirtualMemoryNumber;

/* List of Virtual Memories info */
static VirtualMemoryInfo_t * p_AmmVirtualMemoryList;

/* Handler of the Basic Memory Manager functions */
static AMM_BasicMemoryManagerFunctions_t AmmBmmFunctionsHandler;

/* Pointer on the first element of the pending callbacks */
static AMM_VirtualMemoryCallbackHeader_t AmmPendingCallback;

/* Pointer on the first element of the active callbacks */
static AMM_VirtualMemoryCallbackHeader_t AmmActiveCallback;

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
 * @brief  Push a callback structure into the Pending FIFO
 * @param  p_CallbackElt: Pointer onto the callback to push
 * @return None
 */
static inline void pushPending (AMM_VirtualMemoryCallbackFunction_t * const p_CallbackElt);

/**
 * @brief  Push the Pending callback(s) into the Active FIFO
 * @return None
 */
static inline void passPendingToActive (void);

/**
 * @brief  Pop a callback structure from the Active FIFO
 * @return Pointer onto the popped callback
 */
static inline AMM_VirtualMemoryCallbackFunction_t * popActive (void);

/* Functions Definition ------------------------------------------------------*/

AMM_Function_Error_t AMM_Init (const AMM_InitParameters_t * const p_InitParams)
{
  AMM_Function_Error_t error = AMM_ERROR_NOK;

  uint32_t neededPoolSize = 0x00;

  /* Check if not already initialized */
  if (AmmInitialized == INITIALIZED)
  {
    error = AMM_ERROR_ALREADY_INIT;
  }
  /* Check parameter null pointer */
  else if (p_InitParams == NULL)
  {
    error = AMM_ERROR_BAD_POINTER;
  }
  /* Check Pool address null pointer */
  else if (p_InitParams->p_PoolAddr == NULL)
  {
    error = AMM_ERROR_BAD_POOL_CONFIG;
  }
  /* Check if Pool address is 32bits aligned */
  else if ((MASK_ALIGNED_32BITS & (uint32_t)p_InitParams->p_PoolAddr) != 0)
  {
    error = AMM_ERROR_BAD_POOL_CONFIG;
  }
  /* Check Pool size shall be non zero */
  else if (p_InitParams->PoolSize == 0)
  {
    error = AMM_ERROR_BAD_POOL_CONFIG;
  }
  /* Check that Virtual memories can not be declared without a proper configuration list */
  else if ((p_InitParams->VirtualMemoryNumber != 0x00) &&
           (p_InitParams->p_VirtualMemoryConfigList == NULL))
  {
      error = AMM_ERROR_BAD_VIRTUAL_CONFIG;
  }
  else
  {
    neededPoolSize = p_InitParams->VirtualMemoryNumber * AMM_VIRTUAL_INFO_ELEMENT_SIZE;

    /* Check the parameters relative to virtual memories */
    for (uint32_t memIdx = 0x00;
         (memIdx < p_InitParams->VirtualMemoryNumber) && (error == AMM_ERROR_NOK);
         memIdx++)
    {
      /* Add the amount of needed space for this virtual memory */
      neededPoolSize = neededPoolSize + p_InitParams->p_VirtualMemoryConfigList[memIdx].BufferSize;

      /* Check the virtual memory ID. Shall not be zero */
      if (p_InitParams->p_VirtualMemoryConfigList[memIdx].Id == 0)
      {
        error = AMM_ERROR_BAD_VIRTUAL_CONFIG;
      }
      /* Check if size is not zero */
      else if (p_InitParams->p_VirtualMemoryConfigList[memIdx].BufferSize == 0)
      {
        error = AMM_ERROR_BAD_VIRTUAL_CONFIG;
      }
      /* Check if amount of needed memory has overlapped current pool size */
      else if (p_InitParams->PoolSize < neededPoolSize)
      {
        error = AMM_ERROR_BAD_VIRTUAL_CONFIG;
      }
      else
      {
        /* Do nothing, all ok yet */
      }
    }

    /* Check if parameters are still ok */
    if (error == AMM_ERROR_NOK)
    {
      /* Init all private variables: Pool relative */
      p_AmmPoolAddress = NULL;
      AmmPoolSize = 0x00;
      AmmOccupiedSharedPoolSize = 0x00;
      AmmRequiredVirtualMemorySize = 0x00;

      /* Init all private variables: Virtual Memory relative */
      AmmVirtualMemoryNumber = 0x00;
      p_AmmVirtualMemoryList = NULL;

      /* Init all private variables: BMM relative */
      AmmBmmFunctionsHandler.Init = NULL;
      AmmBmmFunctionsHandler.Allocate = NULL;
      AmmBmmFunctionsHandler.Free = NULL;

      /* Init all private variables: Callbacks relative */
      AmmPendingCallback.next = NULL;
      AmmPendingCallback.prev = NULL;
      AmmActiveCallback.next = NULL;
      AmmActiveCallback.prev = NULL;

      /* First get the Basic Memory Manager functions back */
      AMM_RegisterBasicMemoryManager (&AmmBmmFunctionsHandler);

      if ((AmmBmmFunctionsHandler.Init == NULL)
       || (AmmBmmFunctionsHandler.Allocate == NULL)
       || (AmmBmmFunctionsHandler.Free == NULL))
      {
        error = AMM_ERROR_BAD_BMM_REGISTRATION;
      }
      else
      {
        /* Store the pool info */
        p_AmmPoolAddress = p_InitParams->p_PoolAddr;
        AmmPoolSize = p_InitParams->PoolSize;

        /* First init the Basic Memory Manager */
        AmmBmmFunctionsHandler.Init (p_AmmPoolAddress,
                                     AmmPoolSize);

        /* Allocate memory for the virtual memories info */
        p_AmmVirtualMemoryList = (VirtualMemoryInfo_t *)
                                  (AmmBmmFunctionsHandler. Allocate (AMM_VIRTUAL_INFO_ELEMENT_SIZE
                                                                    * p_InitParams->VirtualMemoryNumber));

        /* Check if allocation is OK*/
        if ((p_AmmVirtualMemoryList == NULL) && (p_InitParams->VirtualMemoryNumber > 0))
        {
          error = AMM_ERROR_BAD_BMM_ALLOCATION;
        }
        else
        {
          /* Init Critical section */
          UTIL_SEQ_INIT_CRITICAL_SECTION ();

          /* Init both pending and active list */
          LST_init_head (&AmmPendingCallback);
          LST_init_head (&AmmActiveCallback);

          /* Keep going on init, fulfill the virtual memories info */
          AmmVirtualMemoryNumber = p_InitParams->VirtualMemoryNumber;

          /* Actualize actual shared pool occupied size */
          AmmOccupiedSharedPoolSize = AmmOccupiedSharedPoolSize + (AMM_VIRTUAL_INFO_ELEMENT_SIZE
                                                                   * AmmVirtualMemoryNumber);

          for (uint32_t memIdx = 0x00;
               memIdx < AmmVirtualMemoryNumber;
               memIdx++)
          {
            p_AmmVirtualMemoryList[memIdx].Id = p_InitParams->p_VirtualMemoryConfigList[memIdx].Id;
            p_AmmVirtualMemoryList[memIdx].RequiredSize = p_InitParams->p_VirtualMemoryConfigList[memIdx].BufferSize;
            p_AmmVirtualMemoryList[memIdx].OccupiedSize = 0x00;

            AmmRequiredVirtualMemorySize = AmmRequiredVirtualMemorySize + p_AmmVirtualMemoryList[memIdx].RequiredSize;
          }

          /* Set init flag */
          AmmInitialized = INITIALIZED;

          /* All info are stored and AMM is initialized */
          error = AMM_ERROR_OK;
        }
      }
    }
  }

  return error;
}

AMM_Function_Error_t AMM_DeInit (void)
{
  AMM_Function_Error_t error = AMM_ERROR_NOK;

  /* Check if initialized */
  if (AmmInitialized == INITIALIZED)
  {
    /* Free resources */
    AmmBmmFunctionsHandler.Free ((uint32_t *)p_AmmVirtualMemoryList);

    /* Reset all the variables */
    AmmBmmFunctionsHandler.Init = NULL;
    AmmBmmFunctionsHandler.Allocate = NULL;
    AmmBmmFunctionsHandler.Free = NULL;

    p_AmmPoolAddress = 0x00;
    AmmPoolSize = 0x00;
    AmmOccupiedSharedPoolSize = 0x00;
    AmmRequiredVirtualMemorySize = 0x00;
    AmmVirtualMemoryNumber = 0x00;

    /* Unset the flag */
    AmmInitialized = 0x00;

    error = AMM_ERROR_OK;
  }
  else
  {
    error = AMM_ERROR_NOT_INIT;
  }

  return error;
}

AMM_Function_Error_t AMM_Alloc (const uint8_t VirtualMemoryId,
                                const uint32_t BufferSize,
                                uint32_t ** pp_AllocBuffer,
                                AMM_VirtualMemoryCallbackFunction_t * const p_CallBackFunction)
{
  AMM_Function_Error_t error = AMM_ERROR_NOK;

  uint32_t selfAvailable = 0x00;

  uint32_t * p_TmpAllocAddr = NULL;

  if (AmmInitialized == NOT_INITIALIZED)
  {
    error = AMM_ERROR_NOT_INIT;
  }
  /* Check if buffer size is not zero */
  else if (BufferSize == 0)
  {
    error = AMM_ERROR_BAD_ALLOCATION_SIZE;
  }
  /* Check if no virtual ID requested */
  else if (VirtualMemoryId == AMM_NO_VIRTUAL_ID)
  {
    /* Enter critical section */
    UTIL_SEQ_ENTER_CRITICAL_SECTION ();

    /* Check for enough space in the shared pool */
    if (BufferSize < (AmmPoolSize - AmmOccupiedSharedPoolSize - AmmRequiredVirtualMemorySize))
    {
      /* Try Allocation. Do not forget the header */
      p_TmpAllocAddr = AmmBmmFunctionsHandler.Allocate (BufferSize + VIRTUAL_MEMORY_HEADER_SIZE);

      /* Check if allocation is OK */
      if (p_TmpAllocAddr != NULL)
      {
        /* Fulfill the header */
        *p_TmpAllocAddr = (uint32_t)(((uint32_t)VirtualMemoryId << VIRTUAL_MEMORY_HEADER_ID_POS)
                                        & VIRTUAL_MEMORY_HEADER_ID_MASK)
                                      | ((BufferSize << VIRTUAL_MEMORY_HEADER_BUFFER_SIZE_POS)
                                        & VIRTUAL_MEMORY_HEADER_BUFFER_SIZE_MASK);

        /* Provide the right address to user, ie without the header */
        *pp_AllocBuffer = (uint32_t *)(p_TmpAllocAddr + VIRTUAL_MEMORY_HEADER_SIZE);

        /* Actualize the current memory occupation of the shared space */
        AmmOccupiedSharedPoolSize = AmmOccupiedSharedPoolSize + BufferSize + VIRTUAL_MEMORY_HEADER_SIZE;

        error = AMM_ERROR_OK;
      }
      else
      {
        /* Register the callback for a future retry */
        pushPending (p_CallBackFunction);

        error = AMM_ERROR_ALLOCATION_FAILED;
      }
    }
    else
    {
      /* Register the callback for a future retry */
      pushPending (p_CallBackFunction);

      error = AMM_ERROR_BAD_ALLOCATION_SIZE;
    }

    /* Exit critical section */
    UTIL_SEQ_EXIT_CRITICAL_SECTION ();
  }
  /* A specific ID is requested */
  else
  {
    error = AMM_ERROR_UNKNOWN_ID;

    /* Enter critical section */
    UTIL_SEQ_ENTER_CRITICAL_SECTION ();

    /* Check virtual memory info */
    for (uint32_t memIdx = 0x00;
         (memIdx < AmmVirtualMemoryNumber) && (error == AMM_ERROR_UNKNOWN_ID);
         memIdx++)
    {
      /* Check if ID is known */
      if (VirtualMemoryId == p_AmmVirtualMemoryList[memIdx].Id)
      {
        /* Check if all the reserved memory has been consumed */
        if (p_AmmVirtualMemoryList[memIdx].OccupiedSize < p_AmmVirtualMemoryList[memIdx].RequiredSize)
        {
          /* Compute what is remaining */
          selfAvailable = p_AmmVirtualMemoryList[memIdx].RequiredSize
                          - p_AmmVirtualMemoryList[memIdx].OccupiedSize;
        }

        /* Check if there is enough space in the shared pool plus in our virtual memory pool */
        if (BufferSize < (AmmPoolSize - AmmOccupiedSharedPoolSize - AmmRequiredVirtualMemorySize + selfAvailable))
        {
          /* Try Allocation. Do not forget the header */
          p_TmpAllocAddr = AmmBmmFunctionsHandler.Allocate ((BufferSize + VIRTUAL_MEMORY_HEADER_SIZE));

          /* Check if allocation is OK */
          if (p_TmpAllocAddr != NULL)
          {
            /* Fulfill the header */
            *p_TmpAllocAddr = (uint32_t)(((uint32_t)VirtualMemoryId << VIRTUAL_MEMORY_HEADER_ID_POS)
                                        & VIRTUAL_MEMORY_HEADER_ID_MASK)
                                      | ((BufferSize << VIRTUAL_MEMORY_HEADER_BUFFER_SIZE_POS)
                                        & VIRTUAL_MEMORY_HEADER_BUFFER_SIZE_MASK);

            /* Provide the right address to user, ie without the header */
            *pp_AllocBuffer = (uint32_t *)(p_TmpAllocAddr + VIRTUAL_MEMORY_HEADER_SIZE);

            /* Actualize our current memory occupation */
            p_AmmVirtualMemoryList[memIdx].OccupiedSize = p_AmmVirtualMemoryList[memIdx].OccupiedSize
                                                          + BufferSize
                                                          + VIRTUAL_MEMORY_HEADER_SIZE;

            /* Check for overlapping the reserved memory */
            if (p_AmmVirtualMemoryList[memIdx].RequiredSize < p_AmmVirtualMemoryList[memIdx].OccupiedSize)
            {
              /* Actualize the shared memory occupation */
              AmmOccupiedSharedPoolSize = AmmOccupiedSharedPoolSize
                                          + BufferSize - selfAvailable
                                          + VIRTUAL_MEMORY_HEADER_SIZE;
            }

            error = AMM_ERROR_OK;
          }
          else
          {
            /* Register the callback for a future retry */
            pushPending (p_CallBackFunction);

            error = AMM_ERROR_ALLOCATION_FAILED;
          }
        }
        else
        {
          /* Register the callback for a future retry */
          pushPending (p_CallBackFunction);

          error = AMM_ERROR_BAD_ALLOCATION_SIZE;
        }
      }
    }

    /* Exit critical section */
    UTIL_SEQ_EXIT_CRITICAL_SECTION ();
  }

  return error;
}

AMM_Function_Error_t AMM_Free (uint32_t * const p_BufferAddr)
{
  AMM_Function_Error_t error = AMM_ERROR_NOK;

  uint8_t virtualId = 0x00;
  int32_t occupiedOverRequired = 0x00;
  uint32_t allocatedSize = 0x00;

  uint32_t * p_TmpAllocAddr = NULL;

  if (AmmInitialized == NOT_INITIALIZED)
  {
    error = AMM_ERROR_NOT_INIT;
  }
  else if (p_BufferAddr == NULL)
  {
    error = AMM_ERROR_BAD_POINTER;
  }
  else if ((MASK_ALIGNED_32BITS & (uint32_t)p_BufferAddr) != 0)
  {
    error = AMM_ERROR_NOT_ALIGNED;
  }
  /* Check if address is managed by this AMM */
  else if ((p_BufferAddr > (p_AmmPoolAddress + AmmPoolSize))
           || (p_BufferAddr < p_AmmPoolAddress))
  {
    error = AMM_ERROR_OUT_OF_RANGE;
  }
  else
  {
    /* Enter critical section */
    UTIL_SEQ_ENTER_CRITICAL_SECTION ();

    /* First correct the address by adding the header */
    p_TmpAllocAddr = (uint32_t *)(p_BufferAddr - VIRTUAL_MEMORY_HEADER_SIZE);

    /* Get the virtual memory information */
    virtualId = (*p_TmpAllocAddr & VIRTUAL_MEMORY_HEADER_ID_MASK) >> VIRTUAL_MEMORY_HEADER_ID_POS;
    allocatedSize = (*p_TmpAllocAddr & VIRTUAL_MEMORY_HEADER_BUFFER_SIZE_MASK) >> VIRTUAL_MEMORY_HEADER_BUFFER_SIZE_POS;

    /* Add header size to allocated size */
    allocatedSize = allocatedSize + VIRTUAL_MEMORY_HEADER_SIZE;

    /* Free the allocated memory */
    AmmBmmFunctionsHandler.Free(p_TmpAllocAddr);

    /* Update the occupation counters depending on the ID */
    if (virtualId == AMM_NO_VIRTUAL_ID)
    {
      /* Update the occupation size */
      AmmOccupiedSharedPoolSize = AmmOccupiedSharedPoolSize - allocatedSize;

      error = AMM_ERROR_OK;
    }
    else
    {
      error = AMM_ERROR_UNKNOWN_ID;

      for (uint32_t memIdx = 0x00;
           (memIdx < AmmVirtualMemoryNumber) && (error == AMM_ERROR_UNKNOWN_ID);
           memIdx++)
      {
        /* Check if it is the right ID */
        if (virtualId == p_AmmVirtualMemoryList[memIdx].Id)
        {
          occupiedOverRequired = (p_AmmVirtualMemoryList[memIdx].OccupiedSize - p_AmmVirtualMemoryList[memIdx].RequiredSize);

          /* Check whether the occupied size has overlapped the required or not */
          if (occupiedOverRequired > 0x00)
          {
            /* Check if reserved memory is overlapped */
            if (allocatedSize > occupiedOverRequired)
            {
              /* Update the occupation size */
              AmmOccupiedSharedPoolSize = AmmOccupiedSharedPoolSize
                                          - (p_AmmVirtualMemoryList[memIdx].OccupiedSize
                                          - p_AmmVirtualMemoryList[memIdx].RequiredSize);
            }
            else
            {
              AmmOccupiedSharedPoolSize = AmmOccupiedSharedPoolSize - allocatedSize;
            }
          }

          /* Update the occupation size */
          p_AmmVirtualMemoryList[memIdx].OccupiedSize = p_AmmVirtualMemoryList[memIdx].OccupiedSize
                                                        - allocatedSize;

          error = AMM_ERROR_OK;
        }
      }
    }

    /* Pop pending callbacks and add them to the active fifo */
    passPendingToActive ();

    /* Exit critical section */
    UTIL_SEQ_EXIT_CRITICAL_SECTION ();

    /* Ask the user task to proceed to a background process call */
    AMM_ProcessRequest();
  }

  return error;
}

void AMM_BackgroundProcess (void)
{
  AMM_VirtualMemoryCallbackFunction_t * p_tmpCallback = NULL;

  do
  {
    /* Pop an active callback request */
    p_tmpCallback = popActive();

    if (p_tmpCallback != NULL)
    {
      /* Invoke the callback for an alloc retry */
      p_tmpCallback->Callback();
    }
  }while (p_tmpCallback != NULL);
}

/* Private Functions Definition ------------------------------------------------------*/

void pushPending (AMM_VirtualMemoryCallbackFunction_t * const p_CallbackElt)
{
  if (p_CallbackElt != NULL)
  {
    /* Add the new callback */
    LST_insert_tail (&AmmPendingCallback, (tListNode *)p_CallbackElt);
  }
}

void passPendingToActive (void)
{
  AMM_VirtualMemoryCallbackFunction_t * p_TmpElt = NULL;

  while (LST_is_empty (&AmmPendingCallback) == FALSE)
  {
    /* Remove the head element */
    LST_remove_head (&AmmPendingCallback, (tListNode**)&p_TmpElt);
    /* Add at the bottom */
    LST_insert_tail (&AmmActiveCallback, (tListNode *)p_TmpElt);
  }
}

AMM_VirtualMemoryCallbackFunction_t * popActive (void)
{
  AMM_VirtualMemoryCallbackFunction_t * p_error = NULL;

  if (LST_is_empty (&AmmActiveCallback) == FALSE)
  {
    /* Remove last element */
    LST_remove_head (&AmmActiveCallback, (tListNode**)&p_error);
  }

  return p_error;
}
