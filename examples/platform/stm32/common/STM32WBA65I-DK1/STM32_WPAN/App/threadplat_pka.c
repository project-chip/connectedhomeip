/*****************************************************************************
 * @file    threadplat_pka.c
 * @author  MDG
 * @brief   This file implements the PKA platform functions for openthread stack
 *          library.
 *****************************************************************************
 * @attention
 *
 * Copyright (c) 2018-2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 *****************************************************************************
 */
//Include
#include "stm32_seq.h"
#include "stm32_lpm.h"
#include "threadplat_pka.h"
#include "stm32wbaxx_ll_pka.h"
#include "app_conf.h"
#include "mbedtls/bignum.h"
#include "mbedtls/ecp.h"
#include "hw.h"
#include "cmsis_compiler.h"

#if (OTPLAT_PKA_IT == 0)
__WEAK void APP_THREAD_SchedulePka(void)
{
  while(1);
}

__WEAK void APP_THREAD_WaitPkaEndOfOperation(void)
{
  while(1);
}

__WEAK void APP_THREAD_PostPkaEndOfOperation(void)
{
  while(1);
}
#endif
/*****************************************************************************/

int otPlatPkaEnable( void )
{
#if (OTPLAT_PKA_IT == 1)
  return HW_PKA_IT_Enable( );
#else
  /* Disable LPM, PKA cannot work in stop or standby mode */
//  UTIL_LPM_SetStopMode( 1 << CFG_LPM_PKA, UTIL_LPM_DISABLE );
//  UTIL_LPM_SetOffMode( 1 << CFG_LPM_PKA, UTIL_LPM_DISABLE );
  return HW_PKA_Enable( );
#endif
}

/*****************************************************************************/

void otPlatPkaP256StartEccScalarMul ( const uint32_t* k,
                                      const uint32_t* p_x,
                                      const uint32_t* p_y )
{
  HW_PKA_P256_StartEccScalarMul(k, p_x, p_y);
}

/*****************************************************************************/

int otPlatEndOfOperation(void)
{
#if (OTPLAT_PKA_IT == 1)
  return HW_PKA_IT_EndOfOperation();
#else
  /* Launch otPlatPkaProccessLoop task to poll on PROCEND flag */
  APP_THREAD_SchedulePka();

  /* Wait for PKA_END_OF_OPERATION (set by otPlatPkaProccessLoop when PKA operation ended) */
  APP_THREAD_WaitPkaEndOfOperation();

  return 1;
#endif
}

/*****************************************************************************/

void otPlatPkaP256ReadEccScalarMul( uint32_t* p_x,
                                    uint32_t* p_y )
{
  HW_PKA_P256_ReadEccScalarMul(p_x, p_y);
}

/*****************************************************************************/

void otPlatPkaDisable( void )
{
#if (OTPLAT_PKA_IT == 1)
  HW_PKA_IT_Disable( );
#else

  HW_PKA_Disable( );
  /* Enable LPM */
//  UTIL_LPM_SetStopMode( 1 << CFG_LPM_PKA, UTIL_LPM_ENABLE );
//  UTIL_LPM_SetOffMode( 1 << CFG_LPM_PKA, UTIL_LPM_ENABLE );
#endif
}

int otPlatPkaMpiInvMod(mbedtls_mpi* X,const mbedtls_mpi* A,const mbedtls_mpi* N)
{
 // if (!otPlatPkaEnable())
 // {
 //   return MBEDTLS_ERR_ECP_HW_ACCEL_FAILED;
 // }
  
  /* 
  From RM0434 rev6 - Ch. 23.4.7:
    Modular inversion of Operand A, modulus N and result need to be ROS size
    
  From RM0434 rev6 - Ch. 23.4.1:
    ROS (RSA Operand Size): data size is (rsa_size/32+1) words, with rsa_size equal
    to the chosen modulus length. For example, when computing RSA with an
    operand size of 1024 bits, ROS is equal to 33 words, or 1056 bits. 

  So, even if OT stack return a size higher for A than N, we shall take N's size as we
  need to satisfy condition (0 <= A < n) and (0 < result < n).
  The (N's size + 1 word) shall be filled with 0s for the PKA accelerator to work properly.
  */

  //HW_PKA_WriteSingleInput( PKA_MODULAR_INV_NB_BITS, N->n*32);

  //HW_PKA_WriteOperand( PKA_MODULAR_INV_IN_OP1, N->n, A->p);

  //if (N != NULL)
  //{
    //HW_PKA_WriteOperand( PKA_MODULAR_INV_IN_OP2_MOD, N->n, N->p);
  //}

  /* Start PKA hardware */
  //HW_PKA_Start( LL_PKA_MODE_MODULAR_INV );

  //otPlatEndOfOperation();

  /* Read the output point X */
  //X->n = N->n;
  //HW_PKA_ReadResult( PKA_MODULAR_INV_OUT_RESULT, X->n, X->p );

  //otPlatPkaDisable();

  return 0;
}

//Not functionnal
//int otPlatPkaMpiMulMod(mbedtls_mpi* X,const mbedtls_mpi* A, const mbedtls_mpi* B, const mbedtls_mpi* N)
//{
//  otPlatPkaEnable();
//
//  HW_PKA_WriteSingleInput( PKA_MONTGOMERY_MUL_NB_BITS, A->n*32);
//
//  HW_PKA_WriteOperand( PKA_MONTGOMERY_MUL_IN_OP1, A->n,A->p);
//
//  HW_PKA_WriteOperand( PKA_MONTGOMERY_MUL_IN_OP2, B->n,B->p);
//
//  if (N != NULL)
//  {
//    HW_PKA_WriteOperand( PKA_MONTGOMERY_MUL_IN_OP3_MOD, N->n, N->p);
//  }
//
//  /* Start PKA hardware */
//  HW_PKA_Start( LL_PKA_MODE_MONTGOMERY_MUL );
//
//  otPlatEndOfOperation();
//
//  /* Read the output point X */
//  X->n = PKA->RAM[1] / 32UL;
//
//  HW_PKA_ReadResult( PKA_MONTGOMERY_MUL_OUT_RESULT, X->n, X->p );
//
//  otPlatPkaDisable();
//
//  return 0;
//}

/*****************************************************************************/
#if (OTPLAT_PKA_IT == 0)
void otPlatPkaProccessLoop(void)
{
  /* Check PKA end of operation */
  if (HW_PKA_EndOfOperation() == 1)
  {
    /* Pka operation ended */
    APP_THREAD_PostPkaEndOfOperation();
  }
  else
  {
    /* Pka operation in progress, re-schedule proccess task */
    APP_THREAD_SchedulePka();
  }
}
#endif
/*****************************************************************************/
