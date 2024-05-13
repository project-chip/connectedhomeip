/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    flash_driver.h
 * @author  MCD Application Team
 * @brief   Dual core Flash driver interface
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2020-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FLASH_DRIVER_H
#define FLASH_DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
typedef enum
{
    SINGLE_FLASH_OPERATION_DONE,
    SINGLE_FLASH_OPERATION_NOT_EXECUTED,
} SingleFlashOperationStatus_t;

typedef enum
{
    WAITED_SEM_BUSY,
    WAITED_SEM_FREE,
} WaitedSemStatus_t;

typedef enum
{
    WAIT_FOR_SEM_BLOCK_FLASH_REQ_BY_CPU1,
    WAIT_FOR_SEM_BLOCK_FLASH_REQ_BY_CPU2,
} WaitedSemId_t;

typedef enum
{
    ReadyToWrite,
    NotReadyToWrite,

} StatusReadyToWrite;
/* Exported functions ------------------------------------------------------- */

/**
 * @brief  Implements the Dual core algorithm to erase multiple sectors in flash with CPU1
 *         It calls for each sector to be erased the API FD_EraseSingleSector()
 *
 * @param  FirstSector:   The first sector to be erased
 *                        This parameter must be a value between 0 and (SFSA - 1)
 * @param  NbrOfSectors:  The number of sectors to erase
 *                        This parameter must be a value between 1 and (SFSA - FirstSector)
 * @retval Number of sectors not erased:
 *                        Depending on the implementation of FD_WaitForSemAvailable(),
 *                        it may still have some sectors not erased when the timing protection has been
 *                        enabled by either CPU1 or CPU2. When the value returned is not 0, the application
 *                        should wait until both timing protection before retrying to erase the last missing sectors.
 *
 *                        In addition, When the returned value is not 0:
 *                        - The Sem2 is NOT released
 *                        - The FLASH is NOT locked
 *                        - SHCI_C2_FLASH_EraseActivity(ERASE_ACTIVITY_OFF) is NOT called
 *                        It is expected that the user will call one more time this function to finish the process
 */
uint32_t FD_EraseSectors(uint32_t FirstSector, uint32_t NbrOfSectors);

/**
 * @brief  Implements the Dual core algorithm to write multiple 64bits data in flash with CPU1
 *         The user shall first make sure the location to be written has been first erase.
 *         Otherwise, the API will loop for ever as it will be not able to write in flash
 *         The only value that can be written even though the destination is not erased is 0.
 *         It calls for each 64bits to be written the API FD_WriteSingleData()
 *
 * @param  DestAddress: Address of the flash to write the first data. It shall be 64bits aligned
 * @param  pSrcBuffer:  Address of the buffer holding the 64bits data to be written in flash
 * @param  NbrOfData:   Number of 64bits data to be written
 * @retval Number of 64bits data not written:
 *                      Depending on the implementation of FD_WaitForSemAvailable(),
 *                      it may still have 64bits data not written when the timing protection has been
 *                      enabled by either CPU1 or CPU2. When the value returned is not 0, the application
 *                      should wait until both timing protection before retrying to write the last missing 64bits data.
 *
 *                      In addition, When the returned value is not 0:
 *                        - The Sem2 is NOT released
 *                        - The FLASH is NOT locked
 *                        It is expected that the user will call one more time this function to finish the process
 */
uint32_t FD_WriteData(uint32_t DestAddress, uint64_t * pSrcBuffer, uint32_t NbrOfData);

/**
 * @brief  Implements the Dual core algorithm to erase one sector in flash with CPU1
 *
 *         It expects the following point before calling this API:
 *         - The Sem2 is taken
 *         - The FLASH is unlocked
 *         - SHCI_C2_FLASH_EraseActivity(ERASE_ACTIVITY_ON) has been called
 *         It expects the following point to be done when no more sectors need to be erased
 *         - The Sem2 is released
 *         - The FLASH is locked
 *         - SHCI_C2_FLASH_EraseActivity(ERASE_ACTIVITY_OFF) is called
 *
 *         The two point above are implemented in FD_EraseSectors()
 *         This API needs to be used instead of FD_EraseSectors() in case a provided library is taking
 *         care of these two points and request only a single operation.
 *
 * @param  FirstSector:   The sector to be erased
 *                        This parameter must be a value between 0 and (SFSA - 1)
 * @retval: SINGLE_FLASH_OPERATION_DONE -> The data has been written
 *          SINGLE_FLASH_OPERATION_NOT_EXECUTED -> The data has not been written due to timing protection
 *                                         from either CPU1 or CPU2. On a failure status, the user should check
 *                                         both timing protection before retrying.
 */
SingleFlashOperationStatus_t FD_EraseSingleSector(uint32_t SectorNumber);

/**
 * @brief  Implements the Dual core algorithm to write one 64bits data in flash with CPU1
 *         The user shall first make sure the location to be written has been first erase.
 *         Otherwise, the API will loop for ever as it will be not able to write in flash
 *         The only value that can be written even though the destination is not erased is 0.
 *
 *         It expects the following point before calling this API:
 *         - The Sem2 is taken
 *         - The FLASH is unlocked
 *         It expects the following point to be done when no more sectors need to be erased
 *         - The Sem2 is released
 *         - The FLASH is locked
 *
 *         The two point above are implemented in FD_WriteData()
 *         This API needs to be used instead of FD_WriteData() in case a provided library is taking
 *         care of these two points and request only a single operation.
 *
 * @param  DestAddress: Address of the flash to write the data. It shall be 64bits aligned
 * @param  Data:  64bits Data to be written
 * @retval: SINGLE_FLASH_OPERATION_DONE -> The data has been written
 *          SINGLE_FLASH_OPERATION_NOT_EXECUTED -> The data has not been written due to timing protection
 *                                         from either CPU1 or CPU2. On a failure status, the user should check
 *                                         both timing protection before retrying.
 */
SingleFlashOperationStatus_t FD_WriteSingleData(uint32_t DestAddress, uint64_t Data);

/**
 * By default, this function is implemented weakly in flash_driver.c to return WAITED_SEM_BUSY.
 * When the semaphore is busy, this will result in either FD_WriteSingleData() or FD_EraseSingleSector()
 * to loop until the semaphore is free.
 *
 * This function may be implemented so that when using either an OS or the UTIL_SEQ_WaitEvt() API from the sequencer,
 * it could possible to run other tasks or enter idle mode until the waited semaphore is free.
 * This function shall not take the waited semaphore but just return when it is free.
 *
 * @param  WaitedSemId: The semaphore ID this function should not return until it is free
 * @retval: WAITED_SEM_BUSY -> The function returned before waiting for the semaphore to be free. This will exit the loop
 *                             from either FD_EraseSingleSector() or FD_WriteSingleData() and the number of actions left to
 *                             be processed are reported to the user
 *          WAITED_SEM_FREE -> The semaphore has been checked as free. Both FD_EraseSingleSector() and FD_WriteSingleData()
 *                             try again to process one more time the flash.
 */
WaitedSemStatus_t FD_WaitForSemAvailable(WaitedSemId_t WaitedSemId);

#ifdef __cplusplus
}
#endif

#endif /*FLASH_DRIVER_H */
