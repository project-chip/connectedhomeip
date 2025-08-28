/**
  ******************************************************************************
  * @file    baes_global.h
  * @author  MCD Application Team
  * @brief   This file contains the internal definitions of the AES software
  *          module.
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

#ifndef BAES_GLOBAL_H__
#define BAES_GLOBAL_H__

#include "app_common.h"
#include "baes.h"

/* Default software configuration */
#define     AES_BLOCK_SIZE_BIT      128u            /* AES Size in Bits */
#define     AES_BLOCK_SIZE_BYTE     16u             /* AES Size in Bytes */
#define     AES_BLOCK_SIZE_WORD     4u              /* AES Size in Words */

#define     AES_EXPANDED_KEY_SIZE   44u

/* By default, use of AES H/W implementation instead of S/W */
#ifndef CFG_BAES_SW
#define CFG_BAES_SW                                        0
#endif

#if CFG_BAES_SW == 1

/* Enables to include AES S/W decryption when set to 1 */
#ifndef CFG_BAES_SW_DECRYPTION
#define CFG_BAES_SW_DECRYPTION                             0
#endif

/* Choice of the AES S/W algorithm version:
 * 1: slow version with 522 bytes of look-up tables
 * 2: fast version with 2048 bytes of look-up tables */
#ifndef CFG_BAES_SW_ALGORITHM
#define CFG_BAES_SW_ALGORITHM                              1
#endif

#endif /* CFG_BAES_SW == 1 */

/* Internal macros */

/* Reverse the words in a 4-word array */
#define BAES_SWAP( w ) \
        M_BEGIN uint32_t t_; \
        t_ = (w)[0]; (w)[0] = (w)[3]; (w)[3] = t_; \
        t_ = (w)[1]; (w)[1] = (w)[2]; (w)[2] = t_; \
        M_END

/* Reverse the bytes and words in a 4-word array */
#define BAES_SWAP_REV( w ) \
        M_BEGIN uint32_t t_; \
        t_ = (w)[0]; (w)[0] = __REV((w)[3]); (w)[3] = __REV(t_); \
        t_ = (w)[1]; (w)[1] = __REV((w)[2]); (w)[2] = __REV(t_); \
        M_END

/* Reverse the words of a 4-word array and xor the result */
#define BAES_SWAP_XOR( w, x ) \
        M_BEGIN uint32_t t_; \
        t_ = (w)[0]; (w)[0] = (w)[3] ^ ((x)[0]); (w)[3] = t_ ^ ((x)[3]); \
        t_ = (w)[1]; (w)[1] = (w)[2] ^ ((x)[1]); (w)[2] = t_ ^ ((x)[2]); \
        M_END

/* Reverse the bytes of a 4-word array and xor the result */
#define BAES_REV_XOR( w, x ) \
        M_BEGIN \
        (w)[0] = __REV((w)[0]) ^ ((x)[0]); (w)[1] = __REV((w)[1]) ^ ((x)[1]); \
        (w)[2] = __REV((w)[2]) ^ ((x)[2]); (w)[3] = __REV((w)[3]) ^ ((x)[3]); \
        M_END

/* Copy and reverse the words of a 4-word array */
#define BAES_COPY_SWAP( d, s ) \
        M_BEGIN \
        (d)[0] = (s)[3]; (d)[1] = (s)[2]; \
        (d)[2] = (s)[1]; (d)[3] = (s)[0]; \
        M_END

/* Copy and reverse the bytes of a 4-word array */
#define BAES_COPY_REV( d, s ) \
        M_BEGIN \
        (d)[0] = __REV((s)[0]); (d)[1] = __REV((s)[1]); \
        (d)[2] = __REV((s)[2]); (d)[3] = __REV((s)[3]); \
        M_END

/* Modifies a byte in a word array in Big Endian */
#define BAES_OR_BYTE_BE( w, n, b ) \
          (w)[(n)/4] |= ((uint32_t)(b)) << (8 * (3-((n)%4)))

/* Note: BYTE0, BYTE1, BYTE2, BYTE3 and BTOW macros are also used
   but they should be defined in "common.h" */

/* Internal functions */

extern void BAES_EncKeySchedule( uint32_t* p_exp_key );

extern void BAES_DecKeySchedule( uint32_t* p_exp_key );

extern void BAES_RawEncrypt( const uint32_t* p_in,
                             uint32_t* p_out,
                             const uint32_t *p_exp_key );

extern void BAES_RawDecrypt( const uint32_t* p_in,
                             uint32_t* p_out,
                             const uint32_t* p_exp_key );

#endif /* BAES_GLOBAL_H__ */
