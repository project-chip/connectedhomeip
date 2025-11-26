/**
  ******************************************************************************
  * @file    flash_driver.c
  * @author  MCD Application Team
  * @brief   The Flash Driver module is the interface layer between Flash
  *          management modules and HAL Flash drivers
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

/* Includes ------------------------------------------------------------------*/
#include "flash_driver.h"
#include "utilities_conf.h"

/* Global variables ----------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/

#define FD_CTRL_NO_BIT_SET   (0UL) /* value used to reset the Flash Control status */

/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/**
 * @brief variable used to represent the Flash Control status
 */
static volatile FD_Flash_ctrl_bm_t FD_Flash_Control_status = FD_CTRL_NO_BIT_SET;

/* Private function prototypes -----------------------------------------------*/
/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Update Flash Control status
  * @param  Flags_bm: Bit mask identifying the caller (1 bit per user)
  * @param  Status:   Action requested (enable or disable flash access)
  * @retval None
  */
void FD_SetStatus(FD_Flash_ctrl_bm_t Flags_bm, FD_FLASH_Status_t Status)
{
  UTILS_ENTER_CRITICAL_SECTION();

  switch (Status)
  {
    case LL_FLASH_DISABLE:
    {
      FD_Flash_Control_status |= (1u << Flags_bm);
      break;
    }
    case LL_FLASH_ENABLE:
    {
      FD_Flash_Control_status &= ~(1u << Flags_bm);
      break;
    }
    default :
    {
      break;
    }
  }

  UTILS_EXIT_CRITICAL_SECTION();
}

/**
  * @brief  Write a block of 128 bits (4 32-bit words) in Flash
  * @param  Dest: Address where to write in Flash (128-bit aligned)
  * @param  Payload: Address of data to be written in Flash (32-bit aligned)
  * @retval FD_FlashOp_Status_t: Success or failure of Flash write operation
  */
FD_FlashOp_Status_t FD_WriteData(uint32_t Dest, uint32_t Payload)
{
  FD_FlashOp_Status_t status = FD_FLASHOP_FAILURE;

  /* Check if RFTS OR Application allow flash access */
  if ((FD_Flash_Control_status & (1u << FD_FLASHACCESS_RFTS)) &&
      (FD_Flash_Control_status & (1u << FD_FLASHACCESS_RFTS_BYPASS)))
  { /* Access not allowed */
    return status;
  }

  /* Wait for system to allow flash access */
  while (FD_Flash_Control_status & (1u << FD_FLASHACCESS_SYSTEM));

  if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, Dest, Payload) == HAL_OK)
  {
    status = FD_FLASHOP_SUCCESS;
  }
  return status;
}

/**
  * @brief  Erase one sector of Flash
  * @param  Sect: Identifier of the sector to erase
  * @retval FD_FlashOp_Status_t: Success or failure of Flash erase operation
  */
FD_FlashOp_Status_t FD_EraseSectors(uint32_t Sect)
{
  FD_FlashOp_Status_t status = FD_FLASHOP_FAILURE;
  uint32_t page_error;
  FLASH_EraseInitTypeDef p_erase_init;

#ifndef FLASH_DBANK_SUPPORT
  if (FLASH_PAGE_NB < Sect)
#else
  if ((FLASH_PAGE_NB * 2u) < Sect)
#endif
  {
    return status;
  }

  /* Check if LL allows flash access */
  if ((FD_Flash_Control_status & (1u << FD_FLASHACCESS_RFTS)) &&
      (FD_Flash_Control_status & (1u << FD_FLASHACCESS_RFTS_BYPASS)))
  { /* Access not allowed */
    return status;
  }

  /* Wait for system to allow flash access */
  while (FD_Flash_Control_status & (1u << FD_FLASHACCESS_SYSTEM));

  p_erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
  p_erase_init.Page = (Sect & (FLASH_PAGE_NB - 1u));
  p_erase_init.NbPages = 1;

#if defined(FLASH_DBANK_SUPPORT)
  /* Verify which Bank is impacted */
  if ((FLASH_PAGE_NB <= Sect) ^ (OB_SWAP_BANK_ENABLE == READ_BIT (FLASH->OPTR, FLASH_OPTR_SWAP_BANK_Msk)))
  {
    p_erase_init.Banks = FLASH_BANK_2;
  }
  else
  {
    p_erase_init.Banks = FLASH_BANK_1;
  }
#endif

  if (HAL_FLASHEx_Erase(&p_erase_init, &page_error) == HAL_OK)
  {
    status = FD_FLASHOP_SUCCESS;
  }

  return status;
}
