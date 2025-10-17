/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bpka.c
  * @author  MCD Application Team
  * @brief   This file implements the BLE PKA module.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#include "app_common.h"
#include "bpka.h"

/*****************************************************************************/

enum
{
  BPKA_IDLE = 0,
  BPKA_P256_KEY_GEN,
  BPKA_RANGE_X_CHECK,
  BPKA_RANGE_Y_CHECK,
  BPKA_POINT_CHECK,
  BPKA_DH_KEY_GEN,
};

/*****************************************************************************/

static uint8_t BPKA_state;
static uint8_t BPKA_error;
static uint32_t BPKA_buffer[24];

/*****************************************************************************/

void BPKA_Reset( void )
{
  /* Disable PKA hardware */
  HW_PKA_Disable( );

  BPKA_state = BPKA_IDLE;
}

/*****************************************************************************/

int BPKA_StartP256Key( const uint32_t* local_private_key )
{
  /* Enable PKA hardware */
  if ( ! HW_PKA_Enable( ) )
    return BPKA_BUSY;

  /* Call the PKA scalar multiplication with the local private key
     as k and the standard point as starting point,
     in order to compute the local public key */
  HW_PKA_P256_StartEccScalarMul( local_private_key, NULL, NULL );

  BPKA_state = BPKA_P256_KEY_GEN;

  BPKACB_Process( );

  return BPKA_OK;
}

/*****************************************************************************/

void BPKA_ReadP256Key( uint32_t* local_public_key )
{
  /* Get local public key from buffer */
  memcpy( local_public_key, BPKA_buffer, 64 );
}

/*****************************************************************************/

int BPKA_StartDhKey( const uint32_t* local_private_key,
                     const uint32_t* remote_public_key )
{
  /* Enable PKA hardware */
  if ( ! HW_PKA_Enable( ) )
    return BPKA_BUSY;

  /* Call the PKA range check operation for public key X coordinate */
  HW_PKA_P256_StartRangeCheck( remote_public_key );

  /* Save input data */
  memcpy( BPKA_buffer, local_private_key, 32 );
  memcpy( BPKA_buffer + 8, remote_public_key, 64 );

  BPKA_state = BPKA_RANGE_X_CHECK;
  BPKA_error = 1;

  BPKACB_Process( );

  return BPKA_OK;
}

/*****************************************************************************/

int BPKA_ReadDhKey( uint32_t* dh_key )
{
  if ( BPKA_error )
    return BPKA_EOF;

  /* Get DH key from buffer */
  memcpy( dh_key, BPKA_buffer, 32 );

  return BPKA_OK;
}

/*****************************************************************************/

__WEAK void BPKACB_Complete( void )
{
}

__WEAK void BPKACB_Process( void )
{
}

/*****************************************************************************/

int BPKA_Process( void )
{
  /* This function implements the offline key computation using the PKA
   */
  if ( BPKA_state == BPKA_IDLE )
  {
    return BPKA_OK;
  }

  /* Check if the current operation is finished */
  if ( ! HW_PKA_EndOfOperation( ) )
    return BPKA_BUSY;

  switch ( BPKA_state )
  {
  case BPKA_P256_KEY_GEN:

    /* Read the PKA scalar multiplication result which is the local public
       key */
    HW_PKA_P256_ReadEccScalarMul( BPKA_buffer, BPKA_buffer + 8 );

    break;

  case BPKA_RANGE_X_CHECK:

    /* Test result of range check operation for public key X coordinate */
    if ( ! HW_PKA_P256_IsRangeCheckOk( ) )
      break;

    /* Call the PKA range check operation for public key Y coordinate */
    HW_PKA_P256_StartRangeCheck( BPKA_buffer + 16 );

    BPKA_state = BPKA_RANGE_Y_CHECK;

    return BPKA_BUSY;

  case BPKA_RANGE_Y_CHECK:

    /* Test result of range check operation for public key Y coordinate */
    if ( ! HW_PKA_P256_IsRangeCheckOk( ) )
      break;

    /* Call the PKA point check operation for remote public key */
    HW_PKA_P256_StartPointCheck( BPKA_buffer + 8,
                                 BPKA_buffer + 16 );

    BPKA_state = BPKA_POINT_CHECK;

    return BPKA_BUSY;

  case BPKA_POINT_CHECK:

    /* Test result of point check operation for remote public key */
    if ( ! HW_PKA_P256_IsPointCheckOk( ) )
      break;

    /* Call the PKA scalar multiplication with the local private key
       as k and the remote public key as starting point,
       in order to compute the DH key */
    HW_PKA_P256_StartEccScalarMul( BPKA_buffer,
                                   BPKA_buffer + 8,
                                   BPKA_buffer + 16 );

    BPKA_state = BPKA_DH_KEY_GEN;

    return BPKA_BUSY;

  case BPKA_DH_KEY_GEN:

    /* Read the PKA scalar multiplication result which is the DH key */
    HW_PKA_P256_ReadEccScalarMul( BPKA_buffer, NULL );

    BPKA_error = 0;

    break;
  }

  /* Callback to inform the BLE stack of the completion of PKA operation */
  BPKACB_Complete( );

  /* End of process: reset the PKA module */
  BPKA_Reset( );

  return BPKA_OK;
}

void BPKA_BG_Process( void )
{
  if( BPKA_Process( ) != 0)
  {
    BPKACB_Process( );
  }
}

/*****************************************************************************/
