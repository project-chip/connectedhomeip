/**
  ******************************************************************************
  * @file    baes_cmac.c
  * @author  MCD Application Team
  * @brief   This file contains the AES CMAC implementation.
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

#include "baes_global.h"

/*****************************************************************************/

typedef struct
{
  uint32_t iv[4];          /* Temporary result/IV */

#if CFG_BAES_SW != 0

  uint32_t exp_key[44];    /* Expanded AES key */

#endif /* CFG_BAES_SW != 0 */

} BAES_CMAC_t;

/*****************************************************************************/

BAES_CMAC_t BAES_CMAC_var;

/*****************************************************************************/

/*
 * AES key roll for CMAC Mode
 *
 */

static void BAES_CmacKeyRoll( uint32_t* key )
{
  uint32_t carry = ((key[0] >> 31) & 1) * 0x87UL;
  key[0] = (key[0] << 1) | (key[1] >> 31);
  key[1] = (key[1] << 1) | (key[2] >> 31);
  key[2] = (key[2] << 1) | (key[3] >> 31);
  key[3] = (key[3] << 1) ^ carry;
}

/*****************************************************************************/

/*
 * AES ECB encryption for CMAC Mode
 *
 */

static void BAES_CmacRawEncrypt( const uint32_t* input,
                                 uint32_t* output )
{
#if CFG_BAES_SW == 0

  HW_AES_Crypt( input, output );

#else /* CFG_BAES_SW != 0 */

  BAES_CMAC_t *av = &BAES_CMAC_var;

  BAES_RawEncrypt( input, output, av->exp_key );

#endif /* CFG_BAES_SW != 0 */
}

/*****************************************************************************/

/*
 * Initialization for AES-CMAC for Authentication TAG Generation.
 * Must be called each time a new CMAC has to be computed.
 */

void BAES_CmacSetKey( const uint8_t* key )
{
  BAES_CMAC_t *av = &BAES_CMAC_var;

  /* Initialize for ECB encoding */

#if CFG_BAES_SW == 0

  HW_AES_Enable( );
  HW_AES_SetKey( HW_AES_ENC, key );

#else /* CFG_BAES_SW != 0 */

  uint32_t tmp[4];
  memcpy( tmp, key, 16 );
  BAES_COPY_REV( av->exp_key, tmp );

  BAES_EncKeySchedule( av->exp_key );

#endif /* CFG_BAES_SW != 0 */

  /* set IV to zero */
  av->iv[0] = av->iv[1] = av->iv[2] = av->iv[3] = 0;
}

/*
 * Initialization for AES-CMAC for Authentication TAG Generation.
 * Must be called each time a new CMAC has to be computed.
 */

void BAES_CmacSetVector( const uint8_t * pIV )
{
    BAES_CMAC_t  * av = &BAES_CMAC_var;

    // -- Update IV if exist else set to zero --
    if ( pIV != NULL )
      { memcpy( av->iv, pIV, AES_BLOCK_SIZE_BYTE ); }
    else
      { memset( av->iv, 0x00, AES_BLOCK_SIZE_BYTE ); }
}

/*****************************************************************************/

/*
 * AES Encryption in CMAC Mode
 *
 * This function can be called multiple times with "size" multiple of 16 and
 * "output" parameter set to NULL. However, in the last call to this function,
 * any positive value for "size" is allowed and the "output" parameter must not
 * be NULL.
 */

void BAES_CmacCompute( const uint8_t* input,
                       uint32_t size,
                       uint8_t* output )
{
  BAES_CMAC_t *av = &BAES_CMAC_var;
  uint32_t i;
  uint32_t last_size = 0;
  uint32_t tmp[4], key[4];
  const uint8_t* ptr = input;

  if ( output )
  {
    /* In case of final append, compute size of last block */
    last_size = size % 16;
    if ( (size != 0) && (last_size == 0) )
      last_size = 16;
    size -= last_size;
  }

  while ( size )
  {
    /* Load the input of all blocks but the last one
       and xor data with previous tag */
    memcpy( tmp, ptr, 16 );
    BAES_REV_XOR( tmp, av->iv );

    /* Encrypt block */
    BAES_CmacRawEncrypt( tmp, av->iv );

    /* Next block */
    ptr += 16;
    size -= 16;
  }

  if ( output )
  {
    /* Load the input bytes left with 0 padding */
    tmp[0] = tmp[1] = tmp[2] = tmp[3] = 0;
    for ( i = 0; i < last_size; i++ )
    {
      BAES_OR_BYTE_BE( tmp, i, ptr[i] );
    }

    /* Compute K1 */
    key[0] = key[1] = key[2] = key[3] = 0;
    BAES_CmacRawEncrypt( key, key );
    BAES_CmacKeyRoll( key );

    /* Add padding and compute K2 if the last block is not full */
    if ( last_size < 16 )
    {
      BAES_OR_BYTE_BE( tmp, last_size, 0x80 );
      BAES_CmacKeyRoll( key );
    }

    /* Xor data with previous tag and key */
    for ( i = 0; i < 4; i++ )
    {
      tmp[i] ^= av->iv[i] ^ key[i];
    }

    /* Encrypt block */
    BAES_CmacRawEncrypt( tmp, av->iv );

#if CFG_BAES_SW == 0

    HW_AES_Disable( );

#endif /* CFG_BAES_SW == 0 */

    /* Write the tag */
    BAES_COPY_REV( tmp, av->iv );
    memcpy( output, tmp, 16 );
  }
}

/*****************************************************************************/
