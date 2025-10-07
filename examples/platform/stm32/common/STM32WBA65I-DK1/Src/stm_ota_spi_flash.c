/**
 ******************************************************************************
 * @file    stm_ota_spi_flash.c
 * @author  MCD Application Team
 * @brief   Write new image in spi flash
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
#include "stm_ota_spi_flash.h"
#include "app_conf.h"

#include "Driver_Flash.h" /* coming from Middlewares\Third_Party\mcuboot\platform\ext\driver */

#if (OTA_EXTERNAL_FLASH_ENABLE == 1)

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

extern ARM_DRIVER_FLASH Driver_SPI_FLASH0;
#define LOADER_FLASH_DEV_NAME  Driver_SPI_FLASH0

uint32_t m_uFlashSectorSize;
uint32_t m_uFlashMinWriteSize;

/* Public functions -----------------;-----------------------------------------*/

/**
  * \brief        Initialize external FLASH.
  * \return       Returns STM_OTA_FLASH_OK if initialization is successful.
  */
STM_OTA_StatusTypeDef STM_OTA_SPI_FLASH_Init(void)
{
  //APP_DBG("STM_OTA_SPI_FLASH_Init.\n");

  if ( LOADER_FLASH_DEV_NAME.Initialize(NULL) != ARM_DRIVER_OK)
  {
    APP_DBG("Driver Flash Init : Failed\n");
  }

  ARM_FLASH_INFO *data = LOADER_FLASH_DEV_NAME.GetInfo();

   m_uFlashSectorSize = data->sector_size;
   m_uFlashMinWriteSize = data->program_unit;

	return STM_OTA_FLASH_OK;
}

/**
  * \brief        Delete external FLASH area.
  *               Use EraseSector command with EEPROM M95P32, Sector Size = 4 Kb (4096 bytes).
  * \param[in]    Address  Starting address of area to delete in FLASH. (The external FLASH address starts at 0).
  * \param[in]    Length   Length to delete (Length is a multiple of 32bits: Length=1 means 32 bits).
  * \return       Returns STM_OTA_FLASH_OK if delete area is successful.
  */
STM_OTA_StatusTypeDef STM_OTA_SPI_FLASH_Delete_Image(uint32_t Address, uint32_t Length)
{
	//APP_DBG("STM_OTA_SPI_FLASH_Delete_Image @=0x%x, Length=0x%x (%lu).\n", Address, Length, Length);	

  STM_OTA_StatusTypeDef ret = STM_OTA_FLASH_OK;
  int32_t ret_arm;
  uint32_t sector_address;

  /* Clear download area */
  for (sector_address = Address;
       sector_address < Address + Length;
       sector_address += m_uFlashSectorSize)
  {
    ret_arm = LOADER_FLASH_DEV_NAME.EraseSector(sector_address);
    if (ret_arm < 0)
    {
      APP_DBG("External flash erase error for sector=0x%x.\n", sector_address);		
      ret = STM_OTA_FLASH_DELETE_FAILED;
    }
  }

  return ret;
}

/**
  * \brief        Write external FLASH area.
  * \param[in]    pDestAddress  Starting address of area to write in FLASH. (The external FLASH address starts at 0).
  * \param[in]    pSrcBuffer    Address of input buffer.
  * \param[in]    Length        Length to write (Length is a multiple of 32bits: Length=1 means 32 bits).
  * \return       Returns STM_OTA_FLASH_OK if write area is successful.
  */
STM_OTA_StatusTypeDef STM_OTA_SPI_FLASH_WriteChunk(uint32_t *pDestAddress, uint32_t *pSrcBuffer, uint32_t Length)
{	
  //APP_DBG("STM_OTA_SPI_FLASH_WriteChunk to @=0x%x, from=0x%x, Length=%lu (=0x%x).\n", pDestAddress, pSrcBuffer, Length, Length);

  STM_OTA_StatusTypeDef ret = STM_OTA_FLASH_OK;  

  /* check buffers pointers */
  if (pSrcBuffer == NULL) 
  {
      return STM_OTA_FLASH_INVALID_PARAM;
  }  

  if (LOADER_FLASH_DEV_NAME.ProgramData((uint32_t)pDestAddress, pSrcBuffer, Length) != ARM_DRIVER_OK)
  {
      APP_DBG("Write external flash error\n");		
      ret = STM_OTA_FLASH_WRITE_FAILED;
  }
  return ret;
}

/**
  * \brief        Read external FLASH area.
  * \param[in]    pSrcAddress   Starting address of area to read in FLASH. (The external FLASH address starts at 0).
  * \param[out]   pDestBuffer    Address of output buffer.
  * \param[in]    Length        Length to read (Length is a multiple of 32bits: Length=1 means 32 bits).
  * \return       Returns STM_OTA_FLASH_OK if read area is successful.
  */
STM_OTA_StatusTypeDef STM_OTA_SPI_FLASH_ReadChunk(uint32_t *pSrcAddress, uint32_t *pDestBuffer, uint32_t Length)
{	
  //APP_DBG("STM_OTA_SPI_FLASH_ReadChunk from @=0x%x, to=0x%x, Length=%lu (=0x%x).\n", pSrcAddress, pDestBuffer, Length, Length);

  STM_OTA_StatusTypeDef ret = STM_OTA_FLASH_OK;  

  /* check buffers pointers */
  if (pDestBuffer == NULL) 
  {
      return STM_OTA_FLASH_INVALID_PARAM;
  }  

  if (LOADER_FLASH_DEV_NAME.ReadData((uint32_t)pSrcAddress, pDestBuffer, Length) != ARM_DRIVER_OK)
  {
      APP_DBG("Read external flash error\n");		
      ret = STM_OTA_FLASH_READ_FAILED;
  }

  return ret;
}

#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */