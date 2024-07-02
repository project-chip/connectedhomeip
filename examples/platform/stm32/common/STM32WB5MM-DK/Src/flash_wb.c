/**
 ******************************************************************************
 * @file    flash_wb.c
 * @author  MCD Application Team
 * @brief   Middleware between keymanager and flahs_driver ,
 *          to manage key needed for Matter
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
#include "flash_wb.h"
#include "stm_ext_flash.h"

#if (OTA_SUPPORT == 1)
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#include "mapping_fwimg.h"
#include "mapping_sbsfu.h"
#elif defined(__ICCARM__) || defined(__GNUC__)
#include "mapping_export.h"
#endif /* __CC_ARM || __ARMCC_VERSION */
#endif

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#if (OTA_SUPPORT == 1)
#define NVM_MATTER_ADDR_INIT_SECURE BACKUP_END + 1 // start after back up slot
#else
#define NVM_MATTER_ADDR_INIT_SECURE EXTERNAL_FLASH_ADDRESS
#endif
// #define NVM_MATTER_ADDR_INIT_SECURE EXTERNAL_FLASH_ADDRESS + OTA_MAX_SIZE  // start after back up slot
#define MATTER_KEY_NAME_MAX_LENGTH (16 * 2) // ADD Max key name string size is 32 "keyType...;KeyName..."
#define NVM_OFFSET_KEY 512
#define NVM_BLOCK_SIZE NVM_OFFSET_KEY
#define DEFAULT_VALUE 0xFF
#define SECTOR_SIZE_SECURE 4096 * 5
#define NVM_MATTER_ADDR_INIT_NO_SECURE NVM_MATTER_ADDR_INIT_SECURE + SECTOR_SIZE_SECURE
#define SECTOR_SIZE_NO_SECURE 4096
#define NVM_SIZE_FLASH (SECTOR_SIZE_SECURE + SECTOR_SIZE_NO_SECURE)

typedef struct
{
    NVM_Sector id_sector;
    uint8_t * ram_ptr;
    size_t sector_size;

} NVM_Sector_Struct;

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint8_t ram_nvm[SECTOR_SIZE_SECURE + SECTOR_SIZE_NO_SECURE] = { 0 };

const NVM_Sector_Struct sector_no_secure = { .id_sector   = SECTOR_NO_SECURE,
                                             .ram_ptr     = ram_nvm + SECTOR_SIZE_SECURE,
                                             .sector_size = SECTOR_SIZE_NO_SECURE };

//*SIMULATE TO EXAMPLE*
const NVM_Sector_Struct sector_secure = { .id_sector = SECTOR_SECURE, .ram_ptr = ram_nvm, .sector_size = SECTOR_SIZE_SECURE };

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static uint8_t flash_get(uint8_t * KeyValue, uint8_t * KeyAddr, size_t KeySize, size_t * read_by_size);
static uint8_t flash_update(const NVM_Sector_Struct select_sector, uint8_t * KeyName, uint8_t * KeyValue, size_t KeySize);
static NVM_StatusTypeDef flash_replace(const NVM_Sector_Struct select_sector, uint8_t * PtKeyfind, uint8_t * KeyName,
                                       uint8_t * KeyValue, size_t KeySize);
static NVM_StatusTypeDef flash_write(uint8_t * PtKeyFree, uint8_t * key, uint8_t * value, size_t value_size);
static uint8_t * SearchKey(uint8_t * PtPage, uint8_t * KeyName, size_t nvm_size);
static NVM_StatusTypeDef delete_key(const NVM_Sector_Struct select_sector, uint8_t * PtkeyFind);

/* Public functions ----------------------------------------------------------*/

NVM_StatusTypeDef NM_Init(void)
{
    NVM_StatusTypeDef err = NVM_OK;

    memset(ram_nvm, DEFAULT_VALUE, sizeof(ram_nvm));
    err = STM_EXT_FLASH_ReadChunk(NVM_MATTER_ADDR_INIT_SECURE, ram_nvm, sizeof(ram_nvm));
    return err;
}

NVM_StatusTypeDef NM_Dump(void)
{
    NVM_StatusTypeDef err = NVM_DELETE_FAILED;

    if (STM_EXT_FLASH_Delete_Image(NVM_MATTER_ADDR_INIT_SECURE, sizeof(ram_nvm)) == STM_EXT_FLASH_OK)
    {
        if (STM_EXT_FLASH_WriteChunk(NVM_MATTER_ADDR_INIT_SECURE, ram_nvm, sizeof(ram_nvm)) == STM_EXT_FLASH_OK)
        {
            err = NVM_OK;
        }
        else
        {
            err = NVM_WRITE_FAILED;
        }
    }
    return err;
}

NVM_StatusTypeDef NM_GetKeyValue(void * KeyValue, const char * KeyName, uint32_t KeySize, size_t * read_by_size, NVM_Sector sector)
{

    if ((KeyValue == NULL) || (read_by_size == NULL))
    {
        return NVM_PARAM_ERROR;
    }
    NVM_Sector_Struct select_nvm = { 0 };
    switch (sector)
    {
    case SECTOR_NO_SECURE:
        select_nvm = sector_no_secure;
        break;

    case SECTOR_SECURE:
        select_nvm = sector_secure;
        break;

    default:
        return NVM_WRITE_FAILED;
    }

    uint8_t * key_search = SearchKey(select_nvm.ram_ptr, (uint8_t *) KeyName, select_nvm.sector_size);
    if (key_search != NULL)
    {
        // copy Keyname's value in KeyValue and copy the size of KeyValue in read_by_size
        return flash_get(KeyValue, key_search, KeySize, read_by_size);
    }
    return NVM_KEY_NOT_FOUND;
}

NVM_StatusTypeDef NM_GetOtNVMAddr(uint32_t * NVMAddr)
{
    if (NVMAddr == NULL)
    {
        return NVM_PARAM_ERROR;
    }
    *NVMAddr = (uint32_t) ram_nvm + SECTOR_SIZE_SECURE;
    return NVM_OK;
}

NVM_StatusTypeDef NM_SetKeyValue(char * KeyValue, char * KeyName, uint32_t KeySize, NVM_Sector sector)
{

    if ((KeyValue == NULL) || (KeyName == NULL))
    {
        return NVM_PARAM_ERROR;
    }
    NVM_Sector_Struct select_nvm = { 0 };
    void * Ptkey                 = NULL;

    switch (sector)
    {
    case SECTOR_NO_SECURE:
        select_nvm = sector_no_secure;
        break;

    case SECTOR_SECURE:
        select_nvm = sector_secure;
        break;

    default:
        return NVM_WRITE_FAILED;
    }

    if (KeySize > NVM_BLOCK_SIZE)
    {
        return NVM_BLOCK_SIZE_OVERFLOW;
    }
    // call function to search the pointer of key if it exist else return  null
    Ptkey = SearchKey(select_nvm.ram_ptr, (uint8_t *) KeyName, select_nvm.sector_size);

    if (Ptkey == NULL)
    {
        return flash_update(select_nvm, (uint8_t *) KeyName, (uint8_t *) KeyValue, KeySize);
    }
    else
    {
        if (!flash_replace(select_nvm, Ptkey, (uint8_t *) KeyName, (uint8_t *) KeyValue, KeySize))
        {
            return NVM_OK;
        }
    }
    return NVM_WRITE_FAILED;
}

NVM_StatusTypeDef NM_DeleteKey(const char * Keyname, NVM_Sector sector)
{

    if (Keyname == NULL)
    {
        return NVM_PARAM_ERROR;
    }
    NVM_Sector_Struct select_nvm = { 0 };
    switch (sector)
    {
    case SECTOR_NO_SECURE:
        select_nvm = sector_no_secure;
        break;

    case SECTOR_SECURE:
        select_nvm = sector_secure;
        break;

    default:
        return NVM_WRITE_FAILED;
    }
    uint8_t * Ptkey = SearchKey(select_nvm.ram_ptr, (uint8_t *) Keyname, select_nvm.sector_size);
    if (Ptkey != NULL)
    {
        return delete_key(select_nvm, Ptkey);
    }
    return NVM_KEY_NOT_FOUND;
}

void NM_ResetFactory(void)
{
    while (1)
    {
        STM_EXT_FLASH_Delete_Image(NVM_MATTER_ADDR_INIT_SECURE, sizeof(ram_nvm));
        NVIC_SystemReset();
    }
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

static uint8_t * SearchKey(uint8_t * PtPage, uint8_t * KeyName, size_t nvm_size)
{

    uint8_t * i         = PtPage;
    size_t read_by_size = 0;

    while ((i >= PtPage) || (i < (PtPage + nvm_size)))
    {
        if (*i != DEFAULT_VALUE)
        {
            if (strcmp(KeyName, (uint8_t *) i) == 0)
            {
                return i;
            }
            read_by_size = *(size_t *) ((uint8_t *) i + MATTER_KEY_NAME_MAX_LENGTH);
            // ensure that the size of the data being read does not exceed the remaining size of the buffer
            if (read_by_size + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH > nvm_size - (i - PtPage))
            {
                return NULL;
            }
            i += read_by_size + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;
            // Flash is corrupted
            if ((i < PtPage) || (i > (PtPage + nvm_size)))
            {
                NM_ResetFactory();
            }
        }
        else
        {
            return NULL;
        }
    }
    return NULL;
}

static uint8_t flash_get(uint8_t * KeyValue, uint8_t * KeyAddr, size_t KeySize, size_t * read_by_size)
{
    *read_by_size = *(size_t *) ((uint8_t *) KeyAddr + MATTER_KEY_NAME_MAX_LENGTH);
    if (KeySize >= *read_by_size)
    {
        memcpy(KeyValue, KeyAddr + MATTER_KEY_NAME_MAX_LENGTH + sizeof(size_t), *read_by_size);
        return NVM_OK;
    }
    else
    {
        return NVM_BUFFER_TOO_SMALL;
    }
}

static NVM_StatusTypeDef flash_update(const NVM_Sector_Struct select_sector, uint8_t * KeyName, uint8_t * KeyValue, size_t KeySize)
{

    uint8_t * i         = select_sector.ram_ptr;
    size_t read_by_size = 0;
    while (i < (select_sector.ram_ptr + select_sector.sector_size))
    {
        if (*i == DEFAULT_VALUE)
        {
            return flash_write(i, KeyName, KeyValue, KeySize);
        }
        read_by_size = *(size_t *) ((uint8_t *) i + MATTER_KEY_NAME_MAX_LENGTH);
        if (read_by_size > NVM_BLOCK_SIZE)
        {
            return NVM_ERROR_BLOCK_ALIGN;
        }
        i += read_by_size + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;
    }
    return NVM_SIZE_FULL;
}

static NVM_StatusTypeDef flash_replace(const NVM_Sector_Struct select_sector, uint8_t * PtKeyfind, uint8_t * KeyName,
                                       uint8_t * KeyValue, size_t KeySize)
{

    NVM_StatusTypeDef err = NVM_OK;
    if ((PtKeyfind != NULL) && (KeyName != NULL) && (KeyValue != NULL))
    {
        err = delete_key(select_sector, PtKeyfind);
        if (err != NVM_OK)
            return err;

        err = flash_update(select_sector, KeyName, KeyValue, KeySize);
        if (err != NVM_OK)
            return err;
        return err;
    }
    return NVM_WRITE_FAILED;
}

static NVM_StatusTypeDef delete_key(const NVM_Sector_Struct select_sector, uint8_t * PtkeyFind)
{
    uint8_t * PtKeyNext = NULL;
    uint8_t * PtKeyCpy  = NULL;
    size_t size_key     = 0;

    if (PtkeyFind != NULL)
    {
        size_key  = *(size_t *) ((uint8_t *) PtkeyFind + MATTER_KEY_NAME_MAX_LENGTH);
        PtKeyNext = PtkeyFind + size_key + MATTER_KEY_NAME_MAX_LENGTH + sizeof(size_key);
        PtKeyCpy  = PtkeyFind;
        while ((*PtKeyNext != 0xFF) && (PtKeyNext < (select_sector.ram_ptr + select_sector.sector_size)))
        {
            size_key = *(size_t *) ((uint8_t *) PtKeyNext + MATTER_KEY_NAME_MAX_LENGTH);
            memcpy(PtKeyCpy, PtKeyNext, size_key + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH);
            PtKeyCpy += size_key + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;
            PtKeyNext += size_key + MATTER_KEY_NAME_MAX_LENGTH + sizeof(size_key);
        }
        memset(PtKeyCpy, DEFAULT_VALUE, (select_sector.ram_ptr + select_sector.sector_size - PtKeyCpy));
        return NVM_OK;
    }
    return NVM_DELETE_FAILED;
}

static NVM_StatusTypeDef flash_write(uint8_t * PtKeyFree, uint8_t * key, uint8_t * value, size_t value_size)
{

    if ((PtKeyFree != NULL) && (key != NULL) && (value != NULL))
    {

        memset(PtKeyFree, DEFAULT_VALUE, value_size);
        memset(PtKeyFree, 0x00, MATTER_KEY_NAME_MAX_LENGTH);
        memcpy(PtKeyFree, key, strlen((char *) key));
        memcpy(PtKeyFree + MATTER_KEY_NAME_MAX_LENGTH, &value_size, sizeof(size_t));
        memcpy(PtKeyFree + MATTER_KEY_NAME_MAX_LENGTH + sizeof(size_t), value, value_size);
        return NVM_OK;
    }
    return NVM_WRITE_FAILED;
}
