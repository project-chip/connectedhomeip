/**
  ******************************************************************************
  * @file    baes.h 
  * @author  MCD Application Team
  * @brief   This file contains the interface of the basic AES software module.
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

#ifndef BAES_H__
#define BAES_H__

#include <stdint.h>

/* Basic AES module dedicated to BLE stack with the following features:
 *   - AES ECB encryption
 *   - AES CMAC computation
 *
 * Configuration: the file "app_common.h" is included in this module.
 * It must define:
 *   - CFG_BAES_SW equals to 1 for software implementation
 *   - CFG_BAES_SW equals to 0 for use of hardware accelerator
 *
 * Notes:
 *   - only 128-bit key is supported
 *   - re-entrance is not supported
 */

/* General interface */

extern void BAES_Reset( void );

/* AES ECB interface */

extern void BAES_EcbCrypt( const uint8_t* key,
                           const uint8_t* input,
                           uint8_t* output,
                           int enc );

/* AES CMAC interface */

extern void BAES_CmacSetKey( const uint8_t* key );

extern void BAES_CmacSetVector( const uint8_t * pIV );

extern void BAES_CmacCompute( const uint8_t* input,
                              uint32_t size,
                              uint8_t* output );

/* AES CCM interface */

extern int BAES_CcmCrypt( uint8_t mode,
                          const uint8_t* key,
                          uint8_t iv_length,
                          const uint8_t* iv,
                          uint16_t add_length,
                          const uint8_t* add,
                          uint16_t input_length,
                          const uint8_t* input,
                          uint8_t tag_length,
                          uint8_t* tag,
                          uint8_t* output );

#endif /* BAES_H__ */
