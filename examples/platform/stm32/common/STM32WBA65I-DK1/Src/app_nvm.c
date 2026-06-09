/**
  ******************************************************************************
  * @file    app_nvm.c
  * @author  MCD Application Team
  * @brief   Middleware between keymanager and flash_driver,
  *          to manage keys needed for Matter
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
#include "app_nvm.h"
#include "flash_driver.h"
#include "flash_manager.h"
#include "cmsis_os2.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
/* Max key name string size is 32: "keyType...;KeyName..." */
#define MATTER_KEY_NAME_MAX_LENGTH    (16U * 2U)
#define NVM_OFFSET_KEY                (512U)
#define NVM_BLOCK_SIZE                (NVM_OFFSET_KEY)
#define DEFAULT_VALUE                 (0xFFU)

/* Sizes and addresses (board dependent) */
#define SECTOR_MATTER_SIZE            (4096U * 5U)
#define SECTOR_OT_SIZE                (4096U)
#define SECTOR_SIZE                   (8192U)

#define START_NVM_MATTER              (0x081E4000UL)
#define NVM_MATTER_ADDR_INIT_PTR      ((void * const)START_NVM_MATTER)

#define NVM_FLASH_SIZE                (SECTOR_MATTER_SIZE + SECTOR_OT_SIZE)
#define START_SECTOR_FLASH_NVM        ((START_NVM_MATTER - FLASH_BASE) / (FLASH_PAGE_SIZE))
#define NB_SECTOR_FLASH_NVM           (NVM_FLASH_SIZE / SECTOR_SIZE)
#define NVM_32_BITS_SIZE              (NVM_FLASH_SIZE / 4U)

/* Private types -------------------------------------------------------------*/
typedef struct
{
  NVM_Sector id_sector;
  uint8_t   *ram_ptr;
  size_t     sector_size;
} NVM_Sector_Struct;

/* Private function prototypes -----------------------------------------------*/
/**
  * @brief  Flash Manager erase operation callback.
  * @param  None
  * @retval None
  */
static void FM_EraseCallback(FM_FlashOp_Status_t Status);

/**
  * @brief  Flash Manager erase-before-write callback.
  *         Called when erase is completed, then triggers write.
  * @param  None
  * @retval None
  */
static void FM_EraseBeforeWriteCallback(FM_FlashOp_Status_t Status);

/**
  * @brief  Flash Manager write operation callback.
  * @param  None
  * @retval None
  */
static void FM_WriteCallback(FM_FlashOp_Status_t Status);

/**
  * @brief  Read a key value from an in-memory key record.
  * @param  pKeyValue  Pointer to output buffer for key value.
  * @param  pKeyAddr   Pointer to key record in RAM.
  * @param  keySize    Size of output buffer.
  * @param  pReadSize  Pointer to size filled with actual read size (may be NULL).
  * @retval NVM_OK on success or an error code.
  */
static NVM_StatusTypeDef Cache_Read(uint8_t *pKeyValue,
                                   const uint8_t *pKeyAddr,
                                   size_t keySize,
                                   size_t *pReadSize);

/**
  * @brief  Append a new key/value pair into a sector RAM buffer.
  * @param  selectSector Sector descriptor (RAM pointer, size).
  * @param  pKeyName     Key name string.
  * @param  pKeyValue    Pointer to key value buffer.
  * @param  keySize      Size of key value.
  * @retval NVM_OK on success or an error code.
  */
static NVM_StatusTypeDef Cache_Update(const NVM_Sector_Struct selectSector,
                                      const uint8_t *pKeyName,
                                      const uint8_t *pKeyValue,
                                      size_t keySize);

/**
  * @brief  Replace an existing key by deleting and appending a new record.
  * @param  selectSector Sector descriptor (RAM pointer, size).
  * @param  pKeyFound    Pointer to existing key record.
  * @param  pKeyName     New key name.
  * @param  pKeyValue    New key value.
  * @param  keySize      New value size.
  * @retval NVM_OK on success or an error code.
  */
static NVM_StatusTypeDef Cache_Replace(const NVM_Sector_Struct selectSector,
                                       uint8_t *pKeyFound,
                                       const uint8_t *pKeyName,
                                       const uint8_t *pKeyValue,
                                       size_t keySize);

/**
  * @brief  Write a new key/value pair at a free location in RAM buffer.
  * @param  pKeyFree   Pointer to free space in RAM.
  * @param  pKey       Key name.
  * @param  pValue     Pointer to value buffer.
  * @param  valueSize  Size of value.
  * @retval NVM_OK on success or an error code.
  */
static NVM_StatusTypeDef Cache_Write(uint8_t *pKeyFree,
                                     const uint8_t *pKey,
                                     const uint8_t *pValue,
                                     size_t valueSize);

/**
  * @brief  Search for a key by name in a RAM sector buffer.
  * @param  pPage    Base address of sector RAM.
  * @param  pKeyName Key name string.
  * @param  nvmSize  Size of the sector.
  * @retval Pointer to key record or NULL if not found.
  */
static uint8_t *Cache_Search_Key(uint8_t *pPage,
                          const uint8_t *pKeyName,
                          size_t nvmSize);

/**
  * @brief  Delete a key in a RAM sector buffer and compact remaining records.
  * @param  selectSector Sector descriptor (RAM pointer, size).
  * @param  pKeyFound    Pointer to the key record to delete.
  * @retval NVM_OK on success or NVM_DELETE_FAILED.
  */
static NVM_StatusTypeDef Cache_Delete_Key(const NVM_Sector_Struct selectSector,
                                    uint8_t *pKeyFound);

/**
  * @brief  Acquire the Flash Manager synchronization semaphore.
  * @param  None
  * @retval None
  */
static void LockFMThread(void);

/**
  * @brief  Release the Flash Manager synchronization semaphore.
  * @param  None
  * @retval None
  */
static void UnLockFMThread(void);

/* Private variables ---------------------------------------------------------*/
static uint8_t ram_nvm[SECTOR_MATTER_SIZE + SECTOR_OT_SIZE] = {0};
static osSemaphoreId_t FMSemaphore;

/* Sector descriptors (RAM view of NVM) */
static const NVM_Sector_Struct sector_ot =
{
  .id_sector   = SECTOR_OT,
  .ram_ptr     = ram_nvm + SECTOR_MATTER_SIZE,
  .sector_size = SECTOR_OT_SIZE
};

static const NVM_Sector_Struct sector_matter =
{
  .id_sector   = SECTOR_MATTER,
  .ram_ptr     = ram_nvm,
  .sector_size = SECTOR_MATTER_SIZE
};

/* Flash Manager callback ----------------------------------------------------*/
static FM_CallbackNode_t FM_EraseStatusCallback =
{
  /* Header for chained list */
  .NodeList = { .next = NULL, .prev = NULL },
  /* Callback for request status */
  .Callback = FM_EraseCallback
};

static FM_CallbackNode_t FM_EraseBeforeWriteStatusCallback =
{
  /* Header for chained list */
  .NodeList = { .next = NULL, .prev = NULL },
  /* Callback for request status */
  .Callback = FM_EraseBeforeWriteCallback
};

static FM_CallbackNode_t FM_WriteStatusCallback =
{
  /* Header for chained list */
  .NodeList = { .next = NULL, .prev = NULL },
  /* Callback for request status */
  .Callback = FM_WriteCallback
};

/**
  * @brief  Flash Manager write operation callback.
  *         Called upon completion of a write operation.
  * @param  Status Flash operation status.
  * @retval None
  */
static void FM_WriteCallback(FM_FlashOp_Status_t Status)
{
  /* Unblock waiting thread */
  UnLockFMThread();
}

/**
  * @brief  Flash Manager erase callback.
  *         Called upon completion of an erase operation.
  *         Performs system reset after releasing semaphore.
  * @param  Status Flash operation status.
  * @retval None
  */
static void FM_EraseCallback(FM_FlashOp_Status_t Status)
{
  /* Unblock waiting thread and reset */
  UnLockFMThread();

  while (1)
  {
    NVIC_SystemReset();
  }
}

/**
  * @brief  Flash Manager erase-before-write callback.
  *         Called when erase operation is done to start write.
  * @param  Status Flash operation status.
  * @retval None
  */
static void FM_EraseBeforeWriteCallback(FM_FlashOp_Status_t Status)
{
  /* After erase completion, write back RAM buffer to flash */
  FM_Write((uint32_t *)ram_nvm,
            NVM_MATTER_ADDR_INIT_PTR,
            NVM_32_BITS_SIZE,
            &FM_WriteStatusCallback);
}

/* Public functions ----------------------------------------------------------*/

/**
  * @brief  Initialize NVM middleware and load NVM content to RAM.
  * @param  None
  * @retval NVM_OK on success, or error code.
  */
NVM_StatusTypeDef NVM_Initialize(void)
{
  /* Initialize RAM mirror with default pattern */
  memset(ram_nvm, DEFAULT_VALUE, sizeof(ram_nvm));

  /* Load content from NVM into RAM mirror */
  memcpy(ram_nvm, NVM_MATTER_ADDR_INIT_PTR, sizeof(ram_nvm));

  /* Create semaphore for FM synchronization */
  FMSemaphore = osSemaphoreNew(1U, 1U, NULL);
  if (FMSemaphore == NULL)
  {
    LOG_ERROR_APP("ERROR FREERTOS : FMSemaphore SEMAPHORE CREATION FAILED");
    while (1)
    {
      /* Error loop */
    }
  }

  return NVM_OK;
}

/**
  * @brief  Dump RAM NVM image to Flash (erase + write).
  *         Erase is started; write is done in callback.
  * @param  None
  * @retval NVM_OK on successful request.
  */
NVM_StatusTypeDef NVM_Dump(void)
{
  /* Mutex will be released in callbacks */
  LockFMThread();

  FM_Erase(START_SECTOR_FLASH_NVM,
           NB_SECTOR_FLASH_NVM,
           &FM_EraseBeforeWriteStatusCallback);

  /* Actual write is done in FM_EraseBeforeWriteCallback */
  return NVM_OK;
}

/**
  * @brief  Finalize dump operation in case semaphore is still locked.
  * @param  None
  * @retval None
  */
void NVM_DumpFinish(void)
{
  /* Force semaphore release if needed */
  LockFMThread();
  UnLockFMThread();
}

/**
  * @brief  Check if a key exists in the selected NVM sector.
  * @param  pKeyName Key name string.
  * @param  sector   Sector identifier (matter / ot).
  * @retval NVM_OK if key exists, NVM_KEY_NOT_FOUND or error code.
  */
NVM_StatusTypeDef NVM_GetKeyExists(const char *pKeyName, NVM_Sector sector)
{
  NVM_Sector_Struct selectNvm;
  uint8_t *pKeySearch;
  NVM_StatusTypeDef status = NVM_KEY_NOT_FOUND;

  if (pKeyName == NULL)
  {
    return NVM_PARAM_ERROR;
  }

  selectNvm.id_sector   = sector;
  selectNvm.ram_ptr     = NULL;
  selectNvm.sector_size = 0U;

  switch (sector)
  {
    case SECTOR_OT:
      selectNvm = sector_ot;
      break;

    case SECTOR_MATTER:
      selectNvm = sector_matter;
      break;

    default:
      return NVM_WRITE_FAILED;
  }

  pKeySearch = Cache_Search_Key(selectNvm.ram_ptr,
                         (const uint8_t *)pKeyName,
                         selectNvm.sector_size);

  if (pKeySearch != NULL)
  {
    status = NVM_OK;
  }

  return status;
}

/**
  * @brief  Retrieve the value of a key from the selected NVM sector.
  * @param  pKeyValue  Output buffer for key value.
  * @param  pKeyName   Key name string.
  * @param  keySize    Size of the output buffer.
  * @param  pReadSize  Pointer to store actual read size (may be NULL).
  * @param  sector     Sector identifier (matter / ot).
  * @retval NVM_OK on success, NVM_KEY_NOT_FOUND, or error code.
  */
NVM_StatusTypeDef NVM_GetKeyValue(void *pKeyValue,
                                  const char *pKeyName,
                                  uint32_t keySize,
                                  size_t *pReadSize,
                                  NVM_Sector sector)
{
  NVM_Sector_Struct selectNvm;
  uint8_t *pKeySearch;

  if ((pKeyValue == NULL) || (pKeyName == NULL))
  {
    return NVM_PARAM_ERROR;
  }

  selectNvm.id_sector   = sector;
  selectNvm.ram_ptr     = NULL;
  selectNvm.sector_size = 0U;

  switch (sector)
  {
    case SECTOR_OT:
      selectNvm = sector_ot;
      break;

    case SECTOR_MATTER:
      selectNvm = sector_matter;
      break;

    default:
      return NVM_WRITE_FAILED;
  }

  pKeySearch = Cache_Search_Key(selectNvm.ram_ptr,
                         (const uint8_t *)pKeyName,
                         selectNvm.sector_size);

  if (pKeySearch != NULL)
  {
    /* Copy key value to pKeyValue and return actual size in pReadSize */
    return Cache_Read((uint8_t *)pKeyValue,
                     pKeySearch,
                     (size_t)keySize,
                     pReadSize);
  }

  return NVM_KEY_NOT_FOUND;
}

/**
  * @brief  Get pointer to OT NVM RAM area (non-secure part).
  * @param  pNvmAddr Output pointer to base address.
  * @retval NVM_OK or NVM_PARAM_ERROR.
  */
NVM_StatusTypeDef NVM_GetOtNVMAddr(uint32_t *pNvmAddr)
{
  if (pNvmAddr == NULL)
  {
    return NVM_PARAM_ERROR;
  }

  *pNvmAddr = (uint32_t)(ram_nvm + SECTOR_MATTER_SIZE);
  return NVM_OK;
}

/**
  * @brief  Set or update a key value in the selected NVM sector.
  *         If key exists, it is replaced; otherwise, it is appended.
  * @param  pKeyValue Pointer to key value.
  * @param  pKeyName  Pointer to key name string.
  * @param  keySize   Size of key value.
  * @param  sector    Sector identifier (matter / ot).
  * @retval NVM_OK on success or error code.
  */
NVM_StatusTypeDef NVM_SetKeyValue(const char *pKeyValue,
                                  const char *pKeyName,
                                  uint32_t keySize,
                                  NVM_Sector sector)
{
  NVM_Sector_Struct selectNvm;
  uint8_t *pKeyFound;

  if ((pKeyValue == NULL) || (pKeyName == NULL))
  {
    return NVM_PARAM_ERROR;
  }

  if (keySize > NVM_BLOCK_SIZE)
  {
    return NVM_BLOCK_SIZE_OVERFLOW;
  }

  selectNvm.id_sector   = sector;
  selectNvm.ram_ptr     = NULL;
  selectNvm.sector_size = 0U;

  switch (sector)
  {
    case SECTOR_OT:
      selectNvm = sector_ot;
      break;

    case SECTOR_MATTER:
      selectNvm = sector_matter;
      break;

    default:
      return NVM_WRITE_FAILED;
  }

  /* Search existing key */
  pKeyFound = Cache_Search_Key(selectNvm.ram_ptr,
                        (const uint8_t *)pKeyName,
                        selectNvm.sector_size);

  if (pKeyFound == NULL)
  {
    /* New key */
    return Cache_Update(selectNvm,
                        (const uint8_t *)pKeyName,
                        (const uint8_t *)pKeyValue,
                        (size_t)keySize);
  }
  else
  {
    /* Replace existing key */
    if (Cache_Replace(selectNvm,
                      pKeyFound,
                      (const uint8_t *)pKeyName,
                      (const uint8_t *)pKeyValue,
                      (size_t)keySize) == NVM_OK)
    {
      return NVM_OK;
    }
  }

  return NVM_WRITE_FAILED;
}

/**
  * @brief  Delete a key from the selected NVM sector.
  * @param  pKeyName Key name string.
  * @param  sector   Sector identifier (matter / ot).
  * @retval NVM_OK on success, NVM_KEY_NOT_FOUND or error code.
  */
NVM_StatusTypeDef NVM_DeleteKey(const char *pKeyName, NVM_Sector sector)
{
  NVM_Sector_Struct selectNvm;
  uint8_t *pKeyFound;

  if (pKeyName == NULL)
  {
    return NVM_PARAM_ERROR;
  }

  selectNvm.id_sector   = sector;
  selectNvm.ram_ptr     = NULL;
  selectNvm.sector_size = 0U;

  switch (sector)
  {
    case SECTOR_OT:
      selectNvm = sector_ot;
      break;

    case SECTOR_MATTER:
      selectNvm = sector_matter;
      break;

    default:
      return NVM_WRITE_FAILED;
  }

  pKeyFound = Cache_Search_Key(selectNvm.ram_ptr,
                        (const uint8_t *)pKeyName,
                        selectNvm.sector_size);

  if (pKeyFound != NULL)
  {
    return Cache_Delete_Key(selectNvm, pKeyFound);
  }

  return NVM_KEY_NOT_FOUND;
}

/**
  * @brief  Reset NVM content to factory settings by erasing NVM region.
  *         System reset is performed in the erase callback.
  * @param  None
  * @retval None
  */
void NVM_ResetFactory(void)
{
  LockFMThread();

  FM_Erase(START_SECTOR_FLASH_NVM,
           NB_SECTOR_FLASH_NVM,
           &FM_EraseStatusCallback);
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
  * @brief  Search a key in a RAM sector buffer by name.
  * @param  pPage    Base address of sector RAM.
  * @param  pKeyName Key name string.
  * @param  nvmSize  Size of the sector.
  * @retval Pointer to key record or NULL.
  */
static uint8_t *Cache_Search_Key(uint8_t *pPage,
                          const uint8_t *pKeyName,
                          size_t nvmSize)
{
  uint8_t *p = pPage;
  size_t readSize;

  if ((pPage == NULL) || (pKeyName == NULL) || (nvmSize == 0U))
  {
    return NULL;
  }

  /* Linear walk through packed key records */
  while (p < (pPage + nvmSize))
  {
    if (*p != DEFAULT_VALUE)
    {
      /* Compare key names */
      if (strcmp((const char *)pKeyName, (const char *)p) == 0)
      {
        return p;
      }

      /* Read stored value size */
      readSize = *(const size_t *)(void *)(p + MATTER_KEY_NAME_MAX_LENGTH);

      /* Check bounds: name + size field + value must fit inside remaining NVM */
      if ((readSize + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH) >
          (nvmSize - (size_t)(p - pPage)))
      {
        /* Corrupted structure */
        return NULL;
      }

      p += readSize + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;

      /* Corruption check */
      if ((p < pPage) || (p > (pPage + nvmSize)))
      {
        NVM_ResetFactory();
      }
    }
    else
    {
      /* First empty entry, no more keys */
      return NULL;
    }
  }

  return NULL;
}

/**
  * @brief  Retrieve a key's value from a key record in RAM.
  * @param  pKeyValue  Output buffer for key value.
  * @param  pKeyAddr   Pointer to key record in RAM.
  * @param  keySize    Size of output buffer.
  * @param  pReadSize  Pointer to store actual size read (may be NULL).
  * @retval NVM_OK on success or error code.
  */
static NVM_StatusTypeDef Cache_Read(uint8_t *pKeyValue,
                                   const uint8_t *pKeyAddr,
                                   size_t keySize,
                                   size_t *pReadSize)
{
  size_t storedSize;

  if ((pKeyValue == NULL) || (pKeyAddr == NULL))
  {
    return NVM_PARAM_ERROR;
  }

  storedSize =
    *(const size_t *)(const void *)(pKeyAddr + MATTER_KEY_NAME_MAX_LENGTH);

  if (keySize >= storedSize)
  {
    memcpy(pKeyValue,
           pKeyAddr + MATTER_KEY_NAME_MAX_LENGTH + sizeof(size_t),
           storedSize);

    if (pReadSize != NULL)
    {
      *pReadSize = storedSize;
    }

    return NVM_OK;
  }

  return NVM_BUFFER_TOO_SMALL;
}

/**
  * @brief  Find space in a sector RAM buffer and append a new key/value pair.
  * @param  selectSector Sector descriptor (RAM pointer, size).
  * @param  pKeyName     Key name.
  * @param  pKeyValue    Pointer to value buffer.
  * @param  keySize      Size of value.
  * @retval NVM_OK on success or error code.
  */
static NVM_StatusTypeDef Cache_Update(const NVM_Sector_Struct selectSector,
                                      const uint8_t *pKeyName,
                                      const uint8_t *pKeyValue,
                                      size_t keySize)
{
  uint8_t *p = selectSector.ram_ptr;
  size_t readSize;

  if ((p == NULL) || (pKeyName == NULL) || (pKeyValue == NULL))
  {
    return NVM_PARAM_ERROR;
  }

  while (p < (selectSector.ram_ptr + selectSector.sector_size))
  {
    if (*p == DEFAULT_VALUE)
    {
      /* Free slot found */
      return Cache_Write(p, pKeyName, pKeyValue, keySize);
    }

    readSize = *(const size_t *)(void *)(p + MATTER_KEY_NAME_MAX_LENGTH);

    if (readSize > NVM_BLOCK_SIZE)
    {
      return NVM_ERROR_BLOCK_ALIGN;
    }

    p += readSize + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;
  }

  return NVM_SIZE_FULL;
}

/**
  * @brief  Replace an existing key by deleting and re-adding the new value.
  * @param  selectSector Sector descriptor (RAM pointer, size).
  * @param  pKeyFound    Pointer to existing key record.
  * @param  pKeyName     New key name.
  * @param  pKeyValue    New key value.
  * @param  keySize      New value size.
  * @retval NVM_OK on success or error code.
  */
static NVM_StatusTypeDef Cache_Replace(const NVM_Sector_Struct selectSector,
                                       uint8_t *pKeyFound,
                                       const uint8_t *pKeyName,
                                       const uint8_t *pKeyValue,
                                       size_t keySize)
{
  NVM_StatusTypeDef status;

  if ((pKeyFound == NULL) || (pKeyName == NULL) || (pKeyValue == NULL))
  {
    return NVM_WRITE_FAILED;
  }

  status = Cache_Delete_Key(selectSector, pKeyFound);
  if (status != NVM_OK)
  {
    return status;
  }

  status = Cache_Update(selectSector, pKeyName, pKeyValue, keySize);
  if (status != NVM_OK)
  {
    return status;
  }

  return NVM_OK;
}

/**
  * @brief  Write a key and its value into free RAM space.
  * @param  pKeyFree   Pointer to free space in sector RAM.
  * @param  pKey       Key name.
  * @param  pValue     Pointer to value buffer.
  * @param  valueSize  Size of value.
  * @retval NVM_OK on success or error code.
  */
static NVM_StatusTypeDef Cache_Write(uint8_t *pKeyFree,
                                     const uint8_t *pKey,
                                     const uint8_t *pValue,
                                     size_t valueSize)
{
  size_t keyLen;

  if ((pKeyFree == NULL) || (pKey == NULL) || (pValue == NULL))
  {
    return NVM_WRITE_FAILED;
  }

  keyLen = strlen((const char *)pKey);
  if (keyLen >= MATTER_KEY_NAME_MAX_LENGTH)
  {
    /* Name too long to fit in fixed field */
    return NVM_PARAM_ERROR;
  }

  /* Clear area for value to avoid leftovers */
  memset(pKeyFree, DEFAULT_VALUE, valueSize);

  /* Clear full key name field */
  memset(pKeyFree, 0x00, MATTER_KEY_NAME_MAX_LENGTH);

  /* Copy key name */
  memcpy(pKeyFree, pKey, keyLen);

  /* Store value size */
  memcpy(pKeyFree + MATTER_KEY_NAME_MAX_LENGTH,
               &valueSize,
               sizeof(size_t));

  /* Store value */
  memcpy(pKeyFree + MATTER_KEY_NAME_MAX_LENGTH + sizeof(size_t),
               pValue,
               valueSize);

  return NVM_OK;
}

static NVM_StatusTypeDef Cache_Delete_Key(const NVM_Sector_Struct selectSector,
                                    uint8_t *pKeyFound)
{
  uint8_t *pKeyNext;
  uint8_t *pKeyCopy;
  size_t sizeKey;

  if ((selectSector.ram_ptr == NULL) || (pKeyFound == NULL))
  {
    return NVM_DELETE_FAILED;
  }

  sizeKey = *(const size_t *)(void *)(pKeyFound + MATTER_KEY_NAME_MAX_LENGTH);

  pKeyNext = pKeyFound + sizeKey + MATTER_KEY_NAME_MAX_LENGTH + sizeof(sizeKey);
  pKeyCopy = pKeyFound;

  /* Compact data after the deleted key */
  while ((pKeyNext >= selectSector.ram_ptr) && (pKeyNext < (selectSector.ram_ptr + selectSector.sector_size)) &&
         (*pKeyNext != DEFAULT_VALUE))
  {
    sizeKey =
      *(const size_t *)(void *)(pKeyNext + MATTER_KEY_NAME_MAX_LENGTH);

    memcpy(pKeyCopy,
           pKeyNext,
           sizeKey + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH);

    pKeyCopy += sizeKey + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;
    pKeyNext += sizeKey + MATTER_KEY_NAME_MAX_LENGTH + sizeof(sizeKey);
  }

  /* Clear the remaining area */
  memset(pKeyCopy,
         DEFAULT_VALUE,
        (size_t)((selectSector.ram_ptr + selectSector.sector_size) - pKeyCopy));

  return NVM_OK;
}
/**
  * @brief  Acquire the Flash Manager thread semaphore.
  * @param  None
  * @retval None
  */
static void LockFMThread(void)
{
  osSemaphoreAcquire(FMSemaphore, osWaitForever);
}

/**
  * @brief  Release the Flash Manager thread semaphore.
  * @param  None
  * @retval None
  */
static void UnLockFMThread(void)
{
  osSemaphoreRelease(FMSemaphore);
}
