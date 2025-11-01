/**
  ******************************************************************************
  * @file    nvm.h
  * @author  MCD Application Team
  * @brief   This file contains the interface of the NVM manager.
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

#ifndef NVM_H__
#define NVM_H__

#include <stdint.h>

/* Enumerated values used for the return of the functions:
   (warning: this enum must be aligned with BLEPLAT corresponding one) */

enum
{
  NVM_OK    =  0,
  NVM_FULL  = -1,
  NVM_BUSY  = -2,
  NVM_EOF   = -3,
  NVM_WARN  = -4,
  NVM_ERROR = -5
};

/* Enumerated values used for the 'mode' of 'NVM_Get()' and
   'NVM_Discard()' functions:
   (warning: this enum must be aligned with BLEPLAT corresponding one) */

enum
{
  NVM_FIRST    =  0,
  NVM_NEXT     =  1,
  NVM_CURRENT  =  2,
  NVM_ALL      =  3
};

#define CFG_NVM_ALIGN (1)

#if CFG_NVM_ALIGN != 0
extern void NVM_Init( uint64_t* buffer,
                      uint16_t size,
                      uint16_t max_size );
#else
extern void NVM_Init( uint32_t* buffer );
#endif

extern int NVM_Add( uint8_t type,
                    const uint8_t* data,
                    uint16_t size,
                    const uint8_t* extra_data,
                    uint16_t extra_size );

extern int NVM_Get( uint8_t mode,
                    uint8_t type,
                    uint16_t offset,
                    uint8_t* data,
                    uint16_t size );

extern int NVM_Compare( uint16_t offset,
                        const uint8_t* data,
                        uint16_t size );

extern void NVM_Discard( uint8_t mode );

/*
 * Callbacks
 */

extern void NVMCB_Store( const uint32_t* ptr, uint32_t size );

#endif /* NVM_H__ */
