/**
  ******************************************************************************
  * @file    hw_pka_p256.c
  * @author  MCD Application Team
  * @brief   This file is an optional part of the PKA driver for STM32WBA.
  *          It is dedicated to the P256 elliptic curve.
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

#include "app_common.h"
#include "stm32wbaxx_ll_pka.h"

/*****************************************************************************/

static const uint32_t HW_PKA_P256_gfp[8] =
{
  0xFFFFFFFF, /* LSB */
  0xFFFFFFFF,
  0xFFFFFFFF,
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000001,
  0xFFFFFFFF,
};

static const uint32_t HW_PKA_P256_r2[8] =
{
  0x00000002, /* LSB */
  0x00000005,
  0x00000003,
  0xFFFFFFFE,
  0xFFFFFFF9,
  0xFFFFFFFB,
  0xFFFFFFFC,
  0xFFFFFFFC,
};

static const uint32_t HW_PKA_P256_p_x[8] =
{
  0xD898C296, /* LSB */
  0xF4A13945,
  0x2DEB33A0,
  0x77037D81,
  0x63A440F2,
  0xF8BCE6E5,
  0xE12C4247,
  0x6B17D1F2,
};

static const uint32_t HW_PKA_P256_p_y[8] =
{
  0x37BF51F5, /* LSB */
  0xCBB64068,
  0x6B315ECE,
  0x2BCE3357,
  0x7C0F9E16,
  0x8EE7EB4A,
  0xFE1A7F9B,
  0x4FE342E2,
};

static const uint32_t HW_PKA_P256_a[8] =
{
  0x00000003, /* LSB */
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000000,
  0x00000000,
};

static const uint32_t HW_PKA_P256_b[8] =
{
  0x27D2604B, /* LSB */
  0x3BCE3C3E,
  0xCC53B0F6,
  0x651D06B0,
  0x769886BC,
  0xB3EBBD55,
  0xAA3A93E7,
  0x5AC635D8,
};

static const uint32_t HW_PKA_P256_n[8] =
{
  0XFC632551, /* LSB */
  0XF3B9CAC2,
  0XA7179E84,
  0XBCE6FAAD,
  0XFFFFFFFF,
  0XFFFFFFFF,
  0X00000000,
  0XFFFFFFFF
};

/*****************************************************************************/

void HW_PKA_P256_StartRangeCheck( const uint32_t* coord )
{
  /* Set the muber of bits of P */
  HW_PKA_WriteSingleInput( PKA_COMPARISON_IN_OP_NB_BITS, 256 );

  /* Set the coordinate */
  HW_PKA_WriteOperand( PKA_COMPARISON_IN_OP1, 8, coord );

  /* Set the modulus value p */
  HW_PKA_WriteOperand( PKA_COMPARISON_IN_OP2, 8, HW_PKA_P256_gfp );

  /* Start PKA hardware */
  HW_PKA_Start( LL_PKA_MODE_COMPARISON );
}

/*****************************************************************************/

uint32_t HW_PKA_P256_IsRangeCheckOk( void )
{
  return (HW_PKA_ReadSingleOutput( PKA_COMPARISON_OUT_RESULT ) == 0x916AUL);
}

/*****************************************************************************/

void HW_PKA_P256_StartPointCheck( const uint32_t* x,
                                  const uint32_t* y )
{
  /* Set the muber of bits of p */
  HW_PKA_WriteSingleInput( PKA_POINT_CHECK_IN_MOD_NB_BITS, 256 );

  /* Set the coefficient a sign */
  HW_PKA_WriteSingleInput( PKA_POINT_CHECK_IN_A_COEFF_SIGN, 1 );

  /* Set the coefficient |a| */
  HW_PKA_WriteOperand( PKA_POINT_CHECK_IN_A_COEFF, 8, HW_PKA_P256_a );

  /* Set the coefficient b */
  HW_PKA_WriteOperand( PKA_POINT_CHECK_IN_B_COEFF, 8, HW_PKA_P256_b );

  /* Set the modulus value p */
  HW_PKA_WriteOperand( PKA_POINT_CHECK_IN_MOD_GF, 8, HW_PKA_P256_gfp );

  /* Set the point coordinate x */
  HW_PKA_WriteOperand( PKA_POINT_CHECK_IN_INITIAL_POINT_X, 8, x );

  /* Set the point coordinate y */
  HW_PKA_WriteOperand( PKA_POINT_CHECK_IN_INITIAL_POINT_Y, 8, y );

  /* Set the Montgomery parameter */
  HW_PKA_WriteOperand( PKA_POINT_CHECK_IN_MONTGOMERY_PARAM,
                       8, HW_PKA_P256_r2 );

  /* Start PKA hardware */
  HW_PKA_Start( LL_PKA_MODE_POINT_CHECK );
}

/*****************************************************************************/

uint32_t HW_PKA_P256_IsPointCheckOk( void )
{
  return (HW_PKA_ReadSingleOutput( PKA_POINT_CHECK_OUT_ERROR ) == 0xD60DUL);
}

/*****************************************************************************/

void HW_PKA_P256_StartEccScalarMul( const uint32_t* k,
                                    const uint32_t* p_x,
                                    const uint32_t* p_y )
{
  /* Set the scalar multiplier k length */
  HW_PKA_WriteSingleInput( PKA_ECC_SCALAR_MUL_IN_EXP_NB_BITS, 256 );

  /* Set the modulus length */
  HW_PKA_WriteSingleInput( PKA_ECC_SCALAR_MUL_IN_OP_NB_BITS, 256 );

  /* Set the coefficient a sign */
  HW_PKA_WriteSingleInput( PKA_ECC_SCALAR_MUL_IN_A_COEFF_SIGN, 1 );

  /* Set the coefficient |a| */
  HW_PKA_WriteOperand( PKA_ECC_SCALAR_MUL_IN_A_COEFF, 8, HW_PKA_P256_a );

  /* Set the coefficient b */
  HW_PKA_WriteOperand( PKA_ECC_SCALAR_MUL_IN_B_COEFF, 8, HW_PKA_P256_b );

  /* Set the modulus value p */
  HW_PKA_WriteOperand( PKA_ECC_SCALAR_MUL_IN_MOD_GF, 8, HW_PKA_P256_gfp );

  /* Set the scalar multiplier k */
  HW_PKA_WriteOperand( PKA_ECC_SCALAR_MUL_IN_K, 8, k );

  /* Set the point P coordinate x */
  HW_PKA_WriteOperand( PKA_ECC_SCALAR_MUL_IN_INITIAL_POINT_X,
                       8, p_x ? p_x : HW_PKA_P256_p_x );

  /* Set the point P coordinate y */
  HW_PKA_WriteOperand( PKA_ECC_SCALAR_MUL_IN_INITIAL_POINT_Y,
                       8, p_y ? p_y : HW_PKA_P256_p_y );

  /* Set the prime order n */
  HW_PKA_WriteOperand( PKA_ECC_SCALAR_MUL_IN_N_PRIME_ORDER,
                       8, HW_PKA_P256_n );

  /* Start PKA hardware */
  HW_PKA_Start( LL_PKA_MODE_ECC_MUL );
}

/*****************************************************************************/

void HW_PKA_P256_ReadEccScalarMul( uint32_t* p_x,
                                   uint32_t* p_y )
{
  /* Read the output point X */
  if ( p_x )
  {
    HW_PKA_ReadResult( PKA_ECC_SCALAR_MUL_OUT_RESULT_X, 8, p_x );
  }

  /* Read the output point Y as the second half of the result */
  if ( p_y )
  {
    HW_PKA_ReadResult( PKA_ECC_SCALAR_MUL_OUT_RESULT_Y, 8, p_y );
  }
}

/*****************************************************************************/
