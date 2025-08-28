/**
  ******************************************************************************
  * @file    baes_ecb.c
  * @author  MCD Application Team
  * @brief   This file contains the AES ECB functions implementation.
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

void BAES_Reset( void )
{
#if CFG_BAES_SW == 0

  HW_AES_Disable( );

#endif
}

/*****************************************************************************/

void BAES_EcbCrypt( const uint8_t* key,
                    const uint8_t* input,
                    uint8_t* output,
                    int enc )
{
  uint32_t tmp[4];

#if CFG_BAES_SW == 0

  HW_AES_Enable( );
  HW_AES_SetKey( (enc ? (HW_AES_ENC | HW_AES_REV) : (HW_AES_DEC | HW_AES_REV)),
                 key );

#else /* CFG_BAES_SW != 0 */

  uint32_t exp_key[44];

  /* Retrieve all bytes from key */
  memcpy( exp_key, key, 16 );
  BAES_SWAP( exp_key );

#if CFG_BAES_SW_DECRYPTION != 0
  if ( !enc )
    BAES_DecKeySchedule( exp_key );
  else
#endif /* CFG_BAES_SW_DECRYPTION != 0 */
    BAES_EncKeySchedule( exp_key );

#endif /* CFG_BAES_SW != 0 */

  /* Retrieve all bytes from input */
  memcpy( tmp, input, 16 );
  BAES_SWAP( tmp );

#if CFG_BAES_SW == 0

  HW_AES_Crypt( tmp, tmp );
  HW_AES_Disable( );

#else /* CFG_BAES_SW != 0 */

#if CFG_BAES_SW_DECRYPTION != 0
  if ( !enc )
    BAES_RawDecrypt( tmp, tmp, exp_key );
  else
#endif /* CFG_BAES_SW_DECRYPTION != 0 */
    BAES_RawEncrypt( tmp, tmp, exp_key );

#endif /* CFG_BAES_SW != 0 */

  /* Write all bytes to output */
  BAES_SWAP( tmp );
  memcpy( output, tmp, 16 );
}

/*****************************************************************************/
