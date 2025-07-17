/**
  ******************************************************************************
  * @file    storage_interface.c
  * @author  MCD Application Team
  * @brief   Implementation of storage interface
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
#include "storage_interface.h"
#include "string.h"
#include <stdio.h>
#include "psa/crypto.h"
#include "psa_its_alt.h"
/* Uncomment the line below if you want some debug logs */
#define FLASH_IF_DBG
#ifdef FLASH_IF_DBG
#define FLASH_IF_TRACE printf
#else
#define FLASH_IF_TRACE(...)
#endif /* FLASH_IF_DBG */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FLASH_IF_MIN_WRITE_LEN (8U)     /* Flash programming by 64 bits */
#define NB_PAGE_SECTOR_PER_ERASE  (1U)  /* Nb page erased per erase */


#define ITS_LOCATION         0X081EA000 /* ITS start address */
#define ITS_MAX_SIZE         (8*1024U)  /* 8 KBytes */
#define ITS_SLOT_MAX_NUMBER  (16U)
#define ITS_SLOT_OFFSET      0x00000080 /* 128 words (32 bits)) */

#if defined(PSA_USE_ENCRYPTED_ITS)
/* Encrypted ITS secret key Location
 * user should protect storage where the key is stored
 * not used when using HUK nothing store at the
 * given address but needed for code purpose
 * so better add a valid Flash ITS address
 */
#define ITS_ENCRYPTION_KEY_LOCATION    0X081EB000
#endif /* PSA_USE_ENCRYPTED_ITS */


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static  uint32_t Slot_Counter = 0; /* Slot 0 to ITS_SLOT_MAX_NUMBER  */

/* Private function prototypes -----------------------------------------------*/
static uint32_t GetPage(uint32_t uAddr);
static HAL_StatusTypeDef FLASH_If_Init(void);
static HAL_StatusTypeDef FLASH_If_Write(void *pDestination, const void *pSource,
                                        uint32_t uLength);
static HAL_StatusTypeDef FLASH_If_Erase_Size(void *pStart, uint32_t uLength);
/* Functions Definition ------------------------------------------------------*/

/**
  * @brief  Gets the page of a given address
  * @param  uAddr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t uAddr)
{
  uint32_t page = 0U;

  if (uAddr < (FLASH_BASE + (FLASH_BANK_SIZE)))
  {
    /* Bank 1 */
    page = (uAddr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (uAddr - (FLASH_BASE + (FLASH_BANK_SIZE))) / FLASH_PAGE_SIZE;
  }

  return page;
}

/**
  * @brief  Unlocks Flash for write access
  * @param  None
  * @retval HAL Status.
  */
static HAL_StatusTypeDef FLASH_If_Init(void)
{
  HAL_StatusTypeDef ret = HAL_ERROR;

  /* Unlock the Program memory */
  if (HAL_FLASH_Unlock() == HAL_OK)
  {

    /* Clear all FLASH flags */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
    /* Unlock the Program memory */
    if (HAL_FLASH_Lock() == HAL_OK)
    {
      ret = HAL_OK;
    }
#ifdef FLASH_IF_DBG
    else
    {
      FLASH_IF_TRACE("[FLASH_IF] Lock failure\r\n");
    }
#endif /* FLASH_IF_DBG */
  }
#ifdef FLASH_IF_DBG
  else
  {
    FLASH_IF_TRACE("[FLASH_IF] Unlock failure\r\n");
  }
#endif /* FLASH_IF_DBG */
  return ret;
}

/**
  * @brief  This function does an erase of n (depends on Length) pages in user flash area
  * @param  pStart: Start of user flash area
  * @param  uLength: number of bytes.
  * @retval HAL_OK if success, an error code otherwise..
  */
static HAL_StatusTypeDef FLASH_If_Erase_Size(void *pStart, uint32_t uLength)
{
  uint32_t page_error = 0U;
  uint32_t uStart = (uint32_t)pStart;
  FLASH_EraseInitTypeDef EraseInitStruct;
  HAL_StatusTypeDef e_ret_status = HAL_ERROR;
  uint32_t first_page = 0U;
  uint32_t nb_pages = 0U;
  uint32_t chunk_nb_pages;

  /* Initialize Flash */
  e_ret_status = FLASH_If_Init();
  /* Unlock the Flash to enable the flash control register access *************/
  HAL_FLASH_Unlock();

  if (e_ret_status == HAL_OK)
  {
    /* Unlock the Flash to enable the flash control register access *************/
    if (HAL_FLASH_Unlock() == HAL_OK)
    {
      first_page = GetPage(uStart);
      /* Get the number of pages to erase from 1st page */
      nb_pages = GetPage(uStart + uLength - 1U) - first_page + 1U;
      EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
      EraseInitStruct.Banks = ((uStart-FLASH_BASE) >= FLASH_BANK_SIZE) ? FLASH_BANK_2 : FLASH_BANK_1;//1U + (uStart-FLASH_BASE)/FLASH_BANK_SIZE;//(((uStart-FLASH_BASE) >= FLASH_BANK_SIZE) ? 2 : 1);

      /* Erase flash per NB_PAGE_SECTOR_PER_ERASE to avoid watch-dog */
      do
      {
        chunk_nb_pages = (nb_pages >= NB_PAGE_SECTOR_PER_ERASE) ? NB_PAGE_SECTOR_PER_ERASE : nb_pages;
        EraseInitStruct.Page = first_page;
        EraseInitStruct.NbPages = chunk_nb_pages;
        first_page += chunk_nb_pages;
        nb_pages -= chunk_nb_pages;
        if (HAL_FLASHEx_Erase(&EraseInitStruct, &page_error) != HAL_OK)
        {
          HAL_FLASH_GetError();
          e_ret_status = HAL_ERROR;
        }
      } while (nb_pages > 0);

      /* Lock the Flash to disable the flash control register access (recommended
      to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();

    }
    else
    {
      e_ret_status = HAL_ERROR;
    }
  }

  return e_ret_status;
}

/**
  * @brief  This function writes a data buffer in flash (data are 32-bit aligned).
  * @note   After writing data buffer, the flash content is checked.
  * @retval HAL_OK if success, an error code otherwise..
  */
static HAL_StatusTypeDef FLASH_If_Write(void *pDestination, const void *pSource, uint32_t uLength)
{
  HAL_StatusTypeDef e_ret_status = HAL_ERROR;
  uint32_t i = 0U;
  uint32_t pdata = (uint32_t)pSource;

  /* Initialize Flash */
  e_ret_status = FLASH_If_Init();

  if (e_ret_status == HAL_OK)
  {
    /* Unlock the Flash to enable the flash control register access *************/
    if (HAL_FLASH_Unlock() != HAL_OK)
    {
      return HAL_ERROR;

    }
    else
    {
      /* DataLength must be a multiple of 64 bit */
      for (i = 0U; i < uLength; i += 16U)
      {
        /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
        be done by word */
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, (uint32_t)pDestination,  (uint32_t)(pdata+i))
            == HAL_OK)
        {
          /* Check the written value */
          if (*(uint64_t *)pDestination != *(uint64_t *)(pdata + i))
          {
            /* Flash content doesn't match SRAM content */
            e_ret_status = HAL_ERROR;
            break;
          }
          /* Increment FLASH Destination address */
          pDestination = (void *)((uint32_t)pDestination + 16U);
        }
        else
        {
          /* Error occurred while writing data in Flash memory */
          e_ret_status = HAL_ERROR;
          break;
        }
      }
      /* Lock the Flash to disable the flash control register access (recommended
      to protect the FLASH memory against possible unwanted operation) *********/
      HAL_FLASH_Lock();
    }
  }
  return e_ret_status;
}


/**
  * @brief  A function that store the object in storage.
  * @param  obj_uid : unique identifier used for identifying the object.
  * @param  obj_length : size of the object in bytes.
  * @param  p_obj : a pointer to the object to be stored.
  * @retval PSA_SUCCESS if success, an error code otherwise.
  */
psa_status_t storage_set(uint64_t obj_uid,
                         uint32_t obj_length,
                         const void *p_obj)
{
  uint32_t *p_storage = NULL;

  if (Slot_Counter >= ITS_SLOT_MAX_NUMBER) {
    return PSA_ERROR_INSUFFICIENT_STORAGE;
  }

#if defined(PSA_USE_ENCRYPTED_ITS)
  if (obj_uid == ITS_ENCRYPTION_SECRET_KEY_ID)
  {
    p_storage = (uint32_t *)ITS_ENCRYPTION_KEY_LOCATION;
  }
  else
#endif /* PSA_USE_ENCRYPTED_ITS */
  {
    p_storage = (uint32_t *)ITS_LOCATION;
  }

  /*Adjust dimension to 64-bit length */
  if (obj_length % FLASH_IF_MIN_WRITE_LEN != 0U)
  {
    obj_length += (FLASH_IF_MIN_WRITE_LEN - (obj_length % FLASH_IF_MIN_WRITE_LEN));
  }

  p_storage = p_storage + (ITS_SLOT_OFFSET * Slot_Counter);
  if (FLASH_If_Write(p_storage, p_obj, obj_length) != HAL_OK)
  {
    return PSA_ERROR_STORAGE_FAILURE;
  }

  Slot_Counter += 1U;

  return PSA_SUCCESS;
}

/**
  * @brief  A function that retrieve the object from storage.
  * @param  obj_uid : unique identifier used for identifying the object.
  * @param  obj_offset : The starting offset of the object.
  * @param  obj_length : size of the object in bytes.
  * @param  p_obj : a pointer to the object to be retrieved.
  * @retval PSA_SUCCESS if success, an error code otherwise.
  */
psa_status_t storage_get(uint64_t obj_uid,
                         uint32_t obj_offset,
                         uint32_t obj_length,
                         void *p_obj)
{
  psa_status_t status = PSA_ERROR_DOES_NOT_EXIST;
  /* User code start */
  uint32_t *p_storage = NULL;
  uint32_t i = 0;

#if defined(PSA_USE_ENCRYPTED_ITS)
  if (obj_uid == ITS_ENCRYPTION_SECRET_KEY_ID)
  {
    p_storage = (uint32_t *)ITS_ENCRYPTION_KEY_LOCATION;
  }
  else
#endif /* PSA_USE_ENCRYPTED_ITS */
  {
    p_storage = (uint32_t *)ITS_LOCATION;
  }

  do{
    if (memcmp(&obj_uid, p_storage, sizeof(uint32_t)) == 0 ){
      /* Ignore object info, ID, size, flags */
      p_storage = ((uint32_t*)(p_storage + sizeof(its_obj_info_t) / 4U));

      /* Copy object from storage */
      memcpy(p_obj, p_storage, obj_length);
      return PSA_SUCCESS;
    }

    p_storage += 1;
    i += sizeof(uint32_t);
 
  }while(i < ITS_MAX_SIZE);
  /* User code end */

  return status;
}

/**
  * @brief  A function that retrieve the object info using the object unique
  *         identifier.
  * @param  obj_uid : unique identifier used for identifying data.
  * @param  p_obj_info : a pointer to metadata: ID, size, flags.
  * @param  obj_info_size : size of the object info.
  * @retval PSA_SUCCESS if success, an error code otherwise.
  */
psa_status_t storage_get_info(uint64_t obj_uid, void *p_obj_info, uint32_t obj_info_size)
{
  psa_status_t status = PSA_ERROR_DOES_NOT_EXIST;
  /* User code start */
  uint32_t i = 0;
  uint32_t *p_storage = (uint32_t *)ITS_LOCATION;

  /* ITS encryption key is stored in specific location */
#if defined(PSA_USE_ENCRYPTED_ITS)
  if (obj_uid == ITS_ENCRYPTION_SECRET_KEY_ID)
  {
    p_storage = (uint32_t *)ITS_ENCRYPTION_KEY_LOCATION;
  }
  else
#endif /* PSA_USE_ENCRYPTED_ITS */
  {
    p_storage = (uint32_t *)ITS_LOCATION;
  }

  /* Fetch the object in ITS storage */
  do{
	if (memcmp(&obj_uid, p_storage, sizeof(uint32_t)) == 0 )
    {

      /* Copy object info from storage */
      memcpy(p_obj_info, p_storage, obj_info_size);
      return PSA_SUCCESS;
    }

    p_storage += 1;
    i += sizeof(uint32_t);

    }while(i < ITS_MAX_SIZE);
  /* User code end */

   return status;
}

/**
  * @brief  A function that remove the object from storage.
  * @param  obj_uid : unique identifier used for identifying the object.
  * @param  obj_size : size of object to be removed from storage.
  * @retval PSA_SUCCESS if success, an error code otherwise.
  */
psa_status_t storage_remove(uint64_t obj_uid, uint32_t obj_size)
{
  psa_status_t status = PSA_ERROR_STORAGE_FAILURE;

  /* User code start */
  uint32_t *p_storage = NULL;

#if defined(PSA_USE_ENCRYPTED_ITS)
  if (obj_uid == ITS_ENCRYPTION_SECRET_KEY_ID)
  {
    p_storage = (uint32_t *)ITS_ENCRYPTION_KEY_LOCATION;
  }
  else
#endif /* PSA_USE_ENCRYPTED_ITS */
  {
    p_storage = (uint32_t *)ITS_LOCATION;
  }

  if ((status = FLASH_If_Erase_Size(p_storage, obj_size)) == HAL_OK)
  {
    return PSA_SUCCESS;
  }
  /* User code end */

  return status;
}


