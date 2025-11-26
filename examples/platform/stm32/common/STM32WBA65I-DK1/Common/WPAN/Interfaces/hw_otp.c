/**
  ******************************************************************************
  * @file    hw_otp.c
  * @author  MCD Application Team
  * @brief   This file contains the OTP driver.
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
#include "stm32wbaxx_hal.h"

/*****************************************************************************/

int HW_OTP_Read( uint8_t index, HW_OTP_data_t ** otp_ptr )
{
  /* OTP data:
   *  additional_data 8 bytes   LSB
   *  bd_address      6 bytes
   *  hsetune         1 byte
   *  index           1 byte    MSB
   */
  *otp_ptr = (HW_OTP_data_t*)(FLASH_OTP_BASE + FLASH_OTP_SIZE - 16);

  while ( ( (*otp_ptr)->index != index ) &&
          ( (*otp_ptr) != (HW_OTP_data_t*) FLASH_OTP_BASE ) )
  {
    (*otp_ptr) -= 1;
  }

  if ( (*otp_ptr)->index != index )
  {
    return 1; /* error */
  }

  return HW_OK;
}

/*****************************************************************************/

int HW_OTP_Write( uint8_t* additional_data,
                  uint8_t* bd_address,
                  uint8_t hsetune,
                  uint8_t index )
{
  int err = 1; /* error */
  HW_OTP_data_t otp;
  int i;

  /* Fill OTP_Data_s structure */
  for ( i = 0; i < sizeof(otp.additional_data); i++ )
  {
    otp.additional_data[i] = additional_data[i];
  }

  for ( i = 0; i < sizeof(otp.bd_address); i++ )
  {
    otp.bd_address[i] = bd_address[i];
  }

  otp.hsetune = hsetune;
  otp.index = index;

  /* Find free space */
  uint32_t free_address = FLASH_OTP_BASE;
  while ( ( *(uint64_t*)(free_address) != 0xFFFFFFFFFFFFFFFFUL ) &&
          ( *(uint64_t*)(free_address + 8u) != 0xFFFFFFFFFFFFFFFFUL ) &&
          ( (free_address + 16u) != FLASH_OTP_BASE + FLASH_OTP_SIZE ) )
  {
    free_address += 16;
  }

  if ( ( *(uint64_t*)(free_address) != 0xFFFFFFFFFFFFFFFFUL ) &&
       ( *(uint64_t*)(free_address + 8u) != 0xFFFFFFFFFFFFFFFFUL ) )
  {
    return 1; /* error */
  }

  /* Store OTP structure in OTP area */

  /* Clear all Flash flags before write operation*/
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

  err = HAL_FLASH_Unlock( );
  err |= HAL_FLASH_Program( FLASH_NSCR1_PG,
                            free_address, (uint32_t)&otp );
  err |= HAL_FLASH_Lock( );

  return err;
}

/*****************************************************************************/
