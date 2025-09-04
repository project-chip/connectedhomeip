/**
  ******************************************************************************
  * @file    otp.c
  * @author  MCD HW Board & MCD Application Team
  * @brief   Source file for One Time Programmable (OTP) area
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

/* Includes ------------------------------------------------------------------*/

#include "otp.h"

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Exported variables --------------------------------------------------------*/

/* Exported functions --------------------------------------------------------*/

HAL_StatusTypeDef OTP_Read(uint8_t index, OTP_Data_s** otp_ptr)
{
  *otp_ptr = (OTP_Data_s*) (FLASH_OTP_BASE + FLASH_OTP_SIZE - 16);

  while ( (*otp_ptr)->index != index && (*otp_ptr) != (OTP_Data_s*) FLASH_OTP_BASE)
  {
    (*otp_ptr) -= 1;
  }

  if ((*otp_ptr)->index != index)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

HAL_StatusTypeDef OTP_Write(uint8_t* additional_data, uint8_t* bd_address, uint8_t hsetune, uint8_t index)
{
  HAL_StatusTypeDef err = HAL_ERROR;
  OTP_Data_s otp_data;
  int i;

  /* Fill OTP_Data_s structure */
  for (i = 0; i < sizeof(otp_data.additional_data); i++)
  {
      otp_data.additional_data[i] = additional_data[i];
  }

  for (i = 0; i < sizeof(otp_data.bd_address); i++)
  {
      otp_data.bd_address[i] = bd_address[i];
  }

  otp_data.hsetune = hsetune;
  otp_data.index = index;

  /* Find free space */
  uint32_t free_address = FLASH_OTP_BASE;
  while ( ( *(uint64_t*)(free_address) != 0xFFFFFFFFFFFFFFFFUL ) &&
          ( *(uint64_t*)(free_address + 8) != 0xFFFFFFFFFFFFFFFFUL ) &&
          ( (free_address + 16) != FLASH_OTP_BASE + FLASH_OTP_SIZE ) )
  {
    free_address += 16;
  }

  if ( ( *(uint64_t*)(free_address) == 0xFFFFFFFFFFFFFFFFUL ) &&
       ( *(uint64_t*)(free_address + 8) == 0xFFFFFFFFFFFFFFFFUL ) )
  {
   /* Store OTP structure in OTP area */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS); /* Clear all Flash flags before write operation*/

   err = HAL_FLASH_Unlock();
   err |= HAL_FLASH_Program(FLASH_NSCR1_PG, free_address, (uint32_t) &otp_data);
   err |= HAL_FLASH_Lock();
  }

  return err;
}

/* Private functions ---------------------------------------------------------*/

