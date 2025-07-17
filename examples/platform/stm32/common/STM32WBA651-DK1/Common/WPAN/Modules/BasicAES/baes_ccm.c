/*****************************************************************************
 * @file    baes_ccm.c
 *
 * @brief   This file contains the AES CCM implementation.
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

#include "baes_global.h"

/*****************************************************************************/

/* ---------------------------------------------------------------------------
 *  Byte/word manipulation macro definitions
 * ---------------------------------------------------------------------------
 */

/* Returns the least significant byte from a word */
#define BYTE0(w)               ((uint8_t)((w) >>  0))

/* Returns the second least significant byte from a word */
#define BYTE1(w)               ((uint8_t)((w) >>  8))

/* Macro to set a 16-bit word into a byte table in big endian */
#define SET_U16_BE( b, i, v )  M_BEGIN uint16_t v_ = (uint16_t)(v); \
                                       uint8_t *b_ = &((uint8_t*)(b))[i]; \
                                       b_[1] = BYTE0(v_); \
                                       b_[0] = BYTE1(v_); M_END

int BAES_CcmCrypt( uint8_t mode,
                   const uint8_t* key,
                   uint8_t iv_length,
                   const uint8_t* iv,
                   uint16_t add_length,
                   const uint8_t* add,
                   uint16_t input_length,
                   const uint8_t* input,
                   uint8_t tag_length,
                   uint8_t* tag,
                   uint8_t* output )
{
#if CFG_BAES_SW == 0

  /* This implementation of AES CCM only supports HW AES and it also only
   * supports the following range for input parameters:
   *  - tag_length: 4..16 (multiple of 2)
   *  - iv_length:  7..13
   *  - add_length: 1..14
   */
  uint32_t left_len, b0[4], bx[4];
  uint8_t len, *b;

  /* Build B0 */
  b = (uint8_t*)b0;
  memset( b0, 0, 16 );
  b[0] = (1U << 6) | (((tag_length - 2) / 2) << 3) | (14U - iv_length);
  memcpy( b + 1, iv, iv_length );
  SET_U16_BE( b, 14, input_length );

  /* Build B1 */
  b = (uint8_t*)bx;
  memset( bx, 0, 16 );
  b[1] = add_length;
  memcpy( b + 2, add, add_length );

  /* Start CCM process with Init and Header phases */
  HW_AES_Enable( );
  HW_AES_InitCcm( mode, key, b0, bx );

  /* Continue CCM process with Payload Phase */
  left_len = input_length;
  while ( left_len > 0 )
  {
    len = 16;
    if ( left_len < 16 )
    {
      len = (uint8_t)left_len;
      memset( bx, 0, 16 );

      if ( mode )
        HW_AES_SetLast( len );
    }

    memcpy( b, input, len );
    HW_AES_Crypt( bx, bx );
    memcpy( output, b, len );
    input += len;
    output += len;
    left_len -= len;
  }

  /* End CCM process with Final Phase */
  HW_AES_EndCcm( tag_length, mode ? b : tag );
  HW_AES_Disable( );

  /* Verification of the tag in case of decryption */
  if ( mode )
  {
    uint8_t diff = 0;
    for ( int i = 0; i < tag_length; i++ )
      diff |= tag[i] ^ b[i];

    return (int)diff;
  }

#endif /* CFG_BAES_SW == 0 */

  return 0;
}

/*****************************************************************************/
