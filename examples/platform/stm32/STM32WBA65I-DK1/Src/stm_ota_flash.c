/**
 ******************************************************************************
 * @file    stm_ota_flash.c
 * @author  MCD Application Team
 * @brief   Write new image in internal flash
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
#include "stm_ota_flash.h"
#include "app_common.h"
#include "cmsis_os.h"

/* Private defines -----------------------------------------------------------*/

/* Private macros ------------------------------------------------------------*/
#include "flash_manager.h"
#include "app_nvm.h"

#if (OTA_EXTERNAL_FLASH_ENABLE == 0)

/* Private variables ---------------------------------------------------------*/
/* FlashOperationCompletedSema goal is to return to upper layer only when flash
 * operation requested is complete.
 */
osSemaphoreId_t FlashOperationCompletedSema;

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void FM_EraseCallback(FM_FlashOp_Status_t Status);
static void FM_WriteCallback(FM_FlashOp_Status_t Status);

static FM_CallbackNode_t FM_EraseStatusCallback = {
/* Header for chained list */
.NodeList = { .next = NULL, .prev = NULL },
/* Callback for request status */
.Callback = FM_EraseCallback };

static FM_CallbackNode_t FM_WriteStatusCallback = {
/* Header for chained list */
.NodeList = { .next = NULL, .prev = NULL },
/* Callback for request status */
.Callback = FM_WriteCallback };

static void FM_EraseCallback(FM_FlashOp_Status_t Status) {
	/* Update status */

  /* flash operation is complete */
  osSemaphoreRelease(FlashOperationCompletedSema);
  /* free FM semaphore for other FM users */
	UnLockFMThread();
}

static void FM_WriteCallback(FM_FlashOp_Status_t Status) {
	/* Update status */

  /* flash operation is complete */
  osSemaphoreRelease(FlashOperationCompletedSema);
  /* free FM semaphore for other FM users */
	UnLockFMThread();
}

/* Public functions ----------------------------------------------------------*/

STM_OTA_StatusTypeDef STM_OTA_FLASH_Init(void)
{
	//APP_DBG("STM_OTA_FLASH_Init.\n");

  /* Semaphore initialization (initial count = 0) */
  FlashOperationCompletedSema = osSemaphoreNew( 1, 0, NULL );
	if ( FlashOperationCompletedSema == NULL )
	{ 
		APP_DBG( "ERROR : FLASH OP COMPLETED SEMAPHORE CREATION FAILED" );
	}

	return STM_OTA_FLASH_OK;
}

/**
  * \brief        Delete internal FLASH area.
  *               Use EraseSector command with Internal FLASH Sector Size = 8Kb (8192 bytes).
  * \param[in]    Address  Starting address of area to delete in FLASH.
  * \param[in]    Length   Length to delete (Length is a multiple of 32bits: Length=1 means 32 bits).
  * \return       Returns STM_OTA_FLASH_OK if delete area is successful.
  */
STM_OTA_StatusTypeDef STM_OTA_FLASH_Delete_Image(uint32_t Address, uint32_t Length)
{
	//APP_DBG("STM_OTA_FLASH_Delete_Image @=0x%x, Length=%lu (=0x%x).\n", Address, Length, Length);	

  FM_Cmd_Status_t error = FM_ERROR;

	LockFMThread();
  
  /* Get start sector */
  uint32_t startSector = (Address - NS_ROM_ALIAS_BASE) / FLASH_PAGE_SIZE_WBA6;
    
  /* Get number of sectors to erase */
  uint32_t endAddress = Address + Length - 1;
  uint32_t endSector = (endAddress - NS_ROM_ALIAS_BASE) / FLASH_PAGE_SIZE_WBA6;
  uint32_t nbSectors = endSector - startSector + 1;

  //APP_DBG("STM_OTA_FLASH_Delete_Image startSector=0x%x, endAddress=0x%x, endSector=0x%x, nbSectors=%d.\n", startSector, endAddress, endSector, nbSectors);	    
  do
  {
    /* Flash manager erase */
    error = FM_Erase( startSector, 
      nbSectors, 
                      &FM_EraseStatusCallback );
    if (error == FM_OK)
    {    
      osSemaphoreAcquire(FlashOperationCompletedSema, osWaitForever);
    }                      
  }  while (error == FM_BUSY);

  return ((error == FM_OK) ? STM_OTA_FLASH_OK : STM_OTA_FLASH_DELETE_FAILED);
}

/**
  * \brief        Write internal FLASH area.
  * \param[in]    pDestAddress  Starting address of area to write in FLASH.
  * \param[in]    pSrcBuffer    Address of input buffer.
  * \param[in]    Length        Length to write (Length is a multiple of 32bits: Length=1 means 32 bits).
  * \return       Returns STM_OTA_FLASH_OK if write area is successful.
  */
STM_OTA_StatusTypeDef STM_OTA_FLASH_WriteChunk(uint32_t *pDestAddress, uint32_t *pSrcBuffer, uint32_t Length)
{	
	//APP_DBG("STM_OTA_FLASH_WriteChunk to @=0x%x, from=0x%x, Length=%lu (=0x%x).\n", pDestAddress, pSrcBuffer, Length, Length);

  FM_Cmd_Status_t error = FM_ERROR;

  /* check buffers pointers */
  if (pSrcBuffer == NULL) 
  {
      return STM_OTA_FLASH_INVALID_PARAM;
  }
  if (pDestAddress == NULL) 
  {
      return STM_OTA_FLASH_INVALID_PARAM;
  }  

  /* Do nothing if Length equal to 0 */
  if (Length == 0U) 
  {
      return STM_OTA_FLASH_OK;
  }

	LockFMThread();

  do
  {
    /* warning, the Length is a multiple of 32bits (size = 1 means 32bits), we need to divide it by 4 */ 
    error = FM_Write_ext (  pSrcBuffer, 
                            pDestAddress,
                            ((int32_t) (Length / 4)), 
                            &FM_WriteStatusCallback);
    if (error == FM_OK)
    {    
      osSemaphoreAcquire(FlashOperationCompletedSema, osWaitForever);
    }    
  }  while (error == FM_BUSY);

  return ((error == FM_OK) ? STM_OTA_FLASH_OK : STM_OTA_FLASH_WRITE_FAILED);
}

#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 0) */
