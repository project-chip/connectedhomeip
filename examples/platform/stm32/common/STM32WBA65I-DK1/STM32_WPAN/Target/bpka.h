/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    bpka.h
  * @author  MCD Application Team
  * @brief   This file contains the interface of the BLE PKA module.
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

#ifndef BPKA_H__
#define BPKA_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Enumerated values used for the return of the functions:
 * (warning: this enum must be aligned with BLEPLAT corresponding one)
 */
enum
{
  BPKA_OK    =  0,
  BPKA_FULL  = -1,
  BPKA_BUSY  = -2,
  BPKA_EOF   = -3,
  BPKA_ERROR = -5
};

void BPKA_Reset( void );

int BPKA_StartP256Key( const uint32_t* local_private_key );

void BPKA_ReadP256Key( uint32_t* local_public_key );

int BPKA_StartDhKey( const uint32_t* local_private_key,
                     const uint32_t* remote_public_key );

int BPKA_ReadDhKey( uint32_t* dh_key );

int BPKA_Process( void );

void BPKA_BG_Process( void );

/* Callback used by BPKA_Process to indicate the end of the processing
 */
void BPKACB_Complete( void );

void BPKACB_Process( void );

#ifdef __cplusplus
}
#endif

#endif /* BPKA_H__ */
