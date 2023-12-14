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
#include "flash_driver.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Private defines -----------------------------------------------------------*/
#define MATTER_KEY_NAME_MAX_LENGTH (15 * 2) // ADD Max key name string size is 30 "keyType...;KeyName..."
// ^ STM32STORE_MAX_KEY_SIZE
#define NVM_OFFSET_KEY 512
#define NVM_END_FLASH
#define NVM_BLOCK_SIZE NVM_OFFSET_KEY
#define FLASH_START 0x08000000
#define DEFAULT_VALUE 0xFF
#define NB_SECTOR 3
#define NVM_MATTER_ADDR_INIT_SECURE 0x08082000
#define NVM_MATTER_ADDR_INIT_SECURE_PTR ((void * const) NVM_MATTER_ADDR_INIT_SECURE)
#define SECTOR_SIZE_SECURE 4096 * 2
#define NVM_MATTER_ADDR_INIT_NO_SECURE NVM_MATTER_ADDR_INIT_SECURE + SECTOR_SIZE_SECURE
#define NVM_MATTER_ADDR_INIT_NOSECURE_PTR ((void * const) NVM_MATTER_ADDR_INIT_NO_SECURE)
#define SECTOR_SIZE_NO_SECURE 4096
#define NVM_SIZE_FLASH (SECTOR_SIZE_SECURE + SECTOR_SIZE_NO_SECURE)
#define NVM_MAX_KEY NVM_SIZE_FLASH / NVM_OFFSET_KEY

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

uint8_t CheckSanity = 0;
/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static uint8_t flash_get(uint8_t * KeyValue, uint8_t * KeyAddr, size_t KeySize, size_t * read_by_size);
static uint8_t flash_update(const NVM_Sector_Struct select_sector, uint8_t * KeyName, uint8_t * KeyValue, size_t KeySize);
static NVM_StatusTypeDef flash_replace(const NVM_Sector_Struct select_sector, uint8_t * PtKeyfind, uint8_t * KeyName,
                                       uint8_t * KeyValue, size_t KeySize);
static NVM_StatusTypeDef flash_write(uint8_t * PtKeyFree, uint8_t * key, uint8_t * value, size_t value_size);
static uint8_t * SearchKey(uint8_t * PtPage, uint8_t * KeyName);
static NVM_StatusTypeDef delete_key(const NVM_Sector_Struct select_sector, uint8_t * PtkeyFind);

/* Public functions ----------------------------------------------------------*/

void NM_Init(void)
{
    // Copy Nvm flash to ram, it used one time for boot
    // copy no secure nvm  to no secure ram
    memcpy(sector_no_secure.ram_ptr, NVM_MATTER_ADDR_INIT_NOSECURE_PTR, sector_no_secure.sector_size);

    // copy secure nvm to secure ram  *SIMULATE TO EXAMPLE*
    memcpy(sector_secure.ram_ptr, NVM_MATTER_ADDR_INIT_SECURE_PTR, sector_secure.sector_size);
}

NVM_StatusTypeDef NM_Check_Validity(void)
{
    NVM_StatusTypeDef err = NVM_OK;
    if (CheckSanity != 0)
    {
        err = NVM_FLASH_CORRUPTION;
    }
    return err;
}

NVM_StatusTypeDef NM_Dump(void)
{
    NVM_StatusTypeDef err = NVM_DELETE_FAILED;

    err = FD_EraseSectors((NVM_MATTER_ADDR_INIT_SECURE - FLASH_START) / (NVM_SIZE_FLASH / NB_SECTOR), NB_SECTOR);
    if (err == 0)
    {
        err = FD_WriteData(NVM_MATTER_ADDR_INIT_SECURE, (uint64_t *) ram_nvm, (uint32_t) (NVM_SIZE_FLASH / sizeof(uint64_t)));
        if (err != 0)
        {
            err = NVM_WRITE_FAILED;
        }
        else
        {
            if (memcmp(ram_nvm, (void *) NVM_MATTER_ADDR_INIT_SECURE, (size_t) NVM_SIZE_FLASH))
            {
                err = NVM_WRITE_FAILED;
            }
            else
            {
                err = NVM_OK;
            }
        }
    }
    return err;
}

NVM_StatusTypeDef NM_GetKeyValue(void * KeyValue, const char * KeyName, uint32_t KeySize, size_t * read_by_size, NVM_Sector sector)
{

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

    uint8_t * key_search = SearchKey(select_nvm.ram_ptr, (uint8_t *) KeyName);
    if (key_search != NULL)
    {
        // copy Keyname's value in KeyValue and copy the size of KeyValue in read_by_size
        return flash_get(KeyValue, key_search, KeySize, read_by_size);
    }
    return NVM_KEY_NOT_FOUND;
}

NVM_StatusTypeDef NM_SetKeyValue(char * KeyValue, char * KeyName, uint32_t KeySize, NVM_Sector sector)
{

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
    Ptkey = SearchKey(select_nvm.ram_ptr, (uint8_t *) KeyName);

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

uint8_t NM_DeleteKey(const char * Keyname, NVM_Sector sector)
{

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
    uint8_t * Ptkey = SearchKey(select_nvm.ram_ptr, (uint8_t *) Keyname);
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
        FD_EraseSectors((NVM_MATTER_ADDR_INIT_SECURE - FLASH_START) / (NVM_SIZE_FLASH / NB_SECTOR), NB_SECTOR);
        NVIC_SystemReset();
    }
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

static uint8_t * SearchKey(uint8_t * PtPage, uint8_t * KeyName)
{

    uint8_t * i         = PtPage;
    size_t read_by_size = 0;

    while ((i >= PtPage) || (i < (PtPage + NVM_SIZE_FLASH)))
    {
        if (*i != DEFAULT_VALUE)
        {
            if (strcmp((char *) KeyName, (char *) i) == 0)
            {
                return i;
            }
            read_by_size = *(size_t *) ((uint8_t *) i + MATTER_KEY_NAME_MAX_LENGTH);
            i += read_by_size + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;
            // Flash is corrupted
            if ((i < PtPage) || (i > (PtPage + NVM_SIZE_FLASH)))
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
        while ((*PtKeyNext != 0xFF) && (PtKeyNext < (ram_nvm + NVM_SIZE_FLASH)))
        {
            size_key = *(size_t *) ((uint8_t *) PtKeyNext + MATTER_KEY_NAME_MAX_LENGTH);
            memcpy(PtKeyCpy, PtKeyNext, size_key + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH);
            PtKeyCpy += size_key + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;
            PtKeyNext += size_key + MATTER_KEY_NAME_MAX_LENGTH + sizeof(size_key);
        }
        memset(PtKeyCpy, DEFAULT_VALUE, (ram_nvm + NVM_SIZE_FLASH - PtKeyCpy));
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
