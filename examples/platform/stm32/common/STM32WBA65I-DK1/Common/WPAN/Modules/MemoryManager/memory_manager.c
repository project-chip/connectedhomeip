/**
  ******************************************************************************
  * @file    memory_manager.c
  * @author  MCD Application Team
  * @brief   Memory Manager
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

/* Includes ------------------------------------------------------------------*/
#include "utilities_common.h"

#include "memory_manager.h"
#include "stm_list.h"

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/**
 * The average timing is @32Mhz :
 * Legacy MM
 *  + Alloc = 3.5us
 *  + Free = 2.5us
 * New MM
 *  + Alloc = 6.5us
 *  + Free = 4.5us
 */
#define USE_NEW_MM   1

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

#if(USE_NEW_MM == 0)
#pragma default_variable_attributes = @"MM_CONTEXT"

static uint8_t QueueSize;
static tListNode BufferPool;
static MM_pCb_t BufferFreeCb;
static uint8_t *p_StartPoolAdd;
static uint8_t *p_EndPoolAdd;

#pragma default_variable_attributes =

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/
/**
 * @brief  Initialize the Pools
 * @param  p_pool: The pool of memory to manage
 * @param  pool_size: The size of the pool
 * @param  elt_size: The size of one element in the pool
 * @retval None
 */
void MM_Init(uint8_t *p_pool, uint32_t pool_size,  uint32_t elt_size)
{
  uint32_t elt_size_corrected;

  QueueSize = 0;
  elt_size_corrected = 4*DIVC( elt_size, 4 );

  /**
   * Save the first and last address of the pool of memory
   */
  p_StartPoolAdd = p_pool;
  p_EndPoolAdd = p_pool + pool_size - 1;

  /**
   *  Initialize list
   */
  LST_init_head (&BufferPool);

  /**
   *  Initialize the queue
   */
  while(pool_size >= elt_size_corrected)
  {
    LST_insert_tail(&BufferPool, (tListNode *)p_pool);
    p_pool += elt_size_corrected;
    QueueSize++;
    pool_size -= elt_size_corrected;
  }

  return;
}

/**
 * @brief  Provide a buffer
 * @note   The buffer allocated to the user shall be at least sizeof(tListNode) bytes
 *         to store the memory manager chaining information
 *
 * @param  size: The size of the buffer requested
 * @param  cb: The callback to be called when a buffer is made available later on
 *                   if there is no buffer currently available when this API is called
 * @retval The buffer address when available or NULL when there is no buffer
 */
MM_pBufAdd_t MM_GetBuffer( uint32_t size, MM_pCb_t cb )
{
  MM_pBufAdd_t buffer_address;
  uint32_t primask_bit;
  uint8_t allocation_exit = FALSE;

  while( allocation_exit == FALSE )
  {
    primask_bit = __get_PRIMASK();    /**< backup PRIMASK bit */
    __disable_irq();                  /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
    if ( QueueSize )
    {
      QueueSize--;
      LST_remove_head( &BufferPool, ( tListNode ** )&buffer_address );
      if((buffer_address >= p_StartPoolAdd) && (buffer_address <= (p_EndPoolAdd - size + 1)))
      {
        /* The buffer is in a valid range */
        BufferFreeCb = 0;
        allocation_exit = TRUE;
      }
      else
      {
        /**
         * The buffer is not in a valid range.
         * Keep the reference out of the memory pool and try again
         */
      }
    }
    else
    {
      BufferFreeCb = cb;
      buffer_address = 0;
      allocation_exit = TRUE;
    }
    __set_PRIMASK( primask_bit );     /**< Restore PRIMASK bit*/
  }

  return buffer_address;
}

/**
 * @brief  Release a buffer
 * @param  p_buffer: The data buffer address
 * @retval None
 */
void MM_ReleaseBuffer( MM_pBufAdd_t p_buffer )
{
  uint32_t primask_bit;

  if((p_buffer >= p_StartPoolAdd) && (p_buffer <= p_EndPoolAdd))
  {
    primask_bit = __get_PRIMASK();  /**< backup PRIMASK bit */
    __disable_irq();                  /**< Disable all interrupts by setting PRIMASK bit on Cortex*/
    LST_insert_tail( &BufferPool, ( tListNode * )p_buffer );
    QueueSize++;
    __set_PRIMASK( primask_bit );     /**< Restore PRIMASK bit*/
    if( BufferFreeCb )
    {
      /**
       * The application is waiting for a free buffer
       */
      BufferFreeCb();
    }
  }

  return;
}
#else
#include "stm32_mm.h"

static uint8_t *p_StartPoolAdd __attribute__((section("MM_CONTEXT")));
static uint8_t *p_EndPoolAdd __attribute__((section("MM_CONTEXT")));
static MM_pCb_t BufferFreeCb __attribute__((section("MM_CONTEXT")));

/* Functions Definition ------------------------------------------------------*/
void MM_Init(uint8_t *p_pool, uint32_t pool_size,  uint32_t elt_size)
{
  /**
   * Save the first and last address of the pool of memory
   */
  p_StartPoolAdd = p_pool;
  p_EndPoolAdd = p_pool + pool_size - 1;

  UTIL_MM_Init(p_pool, pool_size);

  return;
}

/**
 * @brief  Provide a buffer
 * @note   The buffer allocated to the user shall be at least sizeof(tListNode) bytes
 *         to store the memory manager chaining information
 *         During execution, this API shall not be interrupted with a call to either
 *         MM_GetBuffer() or MM_ReleaseBuffer()
 *         MM_GetBuffer() is called only from background so this will never happen
 *         MM_ReleaseBuffer() is called from IPCC interrupt. Masking IPCC interrupt is
 *         enough to comply to the requirement
 *         Note that the primask bit cannot be used as the length of the critical section is too long compare to
 *         the maximum latency of the BLE Irq. Therefore, the BLE Irq shall not be masked.
 *
 * @param  size: The size of the buffer requested
 * @param  cb: The callback to be called when a buffer is made available later on
 *                   if there is no buffer currently available when this API is called
 * @retval The buffer address when available or NULL when there is no buffer
 */
MM_pBufAdd_t MM_GetBuffer( uint32_t size, MM_pCb_t cb )
{
  MM_pBufAdd_t buffer_address;
  /* uint32_t nvic_ipcc_status; */
  uint8_t allocation_exit = FALSE;

  while( allocation_exit == FALSE )
  {

    /* nvic_ipcc_status = NVIC_GetEnableIRQ(IPCC_C2_RX_C2_TX_HSEM_IRQn); */ /**< backup IPCC just in case it has been disabled by the user */
    /* NVIC_DisableIRQ(IPCC_C2_RX_C2_TX_HSEM_IRQn);  */
    __disable_irq();
    buffer_address = (MM_pBufAdd_t)UTIL_MM_GetBuffer( size );
    __enable_irq();
    /*
    if(nvic_ipcc_status != 0)
    {
      NVIC_EnableIRQ(IPCC_C2_RX_C2_TX_HSEM_IRQn);
    }
    */

    if(buffer_address != 0)
    {
      if((buffer_address >= p_StartPoolAdd) && (buffer_address <= (p_EndPoolAdd - size + 1)))
      {
        /* The buffer is in a valid range */
        BufferFreeCb = 0;
        allocation_exit = TRUE;
      }
      else
      {
        /**
         * The buffer is not in a valid range.
         * Keep the reference out of the memory pool and try again
         */
      }
    }
    else
    {
      BufferFreeCb = cb;
      allocation_exit = TRUE;
    }
  }

  return buffer_address;
}

/**
 * @brief  Release a buffer
 *         During execution, this API shall not be interrupted with a call to either
 *         MM_GetBuffer() or MM_ReleaseBuffer()
 *         MM_GetBuffer() is called only from background so this will never happen
 *         MM_ReleaseBuffer() is called from IPCC interrupt so it cannot be nested.
 *         There is no need of a critical section
 *         Note that anyway, the primask bit cannot be used as the length of the critical section
 *         is too long compare to the maximum latency of the BLE Irq. Therefore, the BLE Irq shall not be masked.
 *
 * @param  p_buffer: The data buffer address
 * @retval None
 */
void MM_ReleaseBuffer( MM_pBufAdd_t p_buffer )
{
  if((p_buffer >= p_StartPoolAdd) && (p_buffer <= p_EndPoolAdd))
  {
    UTIL_MM_ReleaseBuffer( p_buffer );

    if( BufferFreeCb )
    {
      /**
       * The application is waiting for a free buffer
       */
      BufferFreeCb();
    }
  }

  return;
}

#endif
