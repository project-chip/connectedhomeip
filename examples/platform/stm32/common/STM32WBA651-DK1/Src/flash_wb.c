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
#include "flash_manager.h"
#include "cmsis_os2.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Private defines -----------------------------------------------------------*/
#define MATTER_KEY_NAME_MAX_LENGTH (16 * 2) // ADD Max key name string size is 32 "keyType...;KeyName..."
#define NVM_OFFSET_KEY 512
#define NVM_BLOCK_SIZE NVM_OFFSET_KEY
#define DEFAULT_VALUE  0xFF
#define SECTOR_SIZE_SECURE 4096*5
#define SECTOR_SIZE_NO_SECURE 4096
#define SECTOR_SIZE 8192
//#define START_NVM_Matter 0x81E4000
#define START_NVM_Matter 0x81A0000
#define NVM_MATTER_ADDR_INIT_PTR ((void * const) START_NVM_Matter)
#define NVM_SIZE_FLASH (SECTOR_SIZE_SECURE + SECTOR_SIZE_NO_SECURE)
#define START_SECTOR_FLASH_NVM ((START_NVM_Matter - FLASH_BASE)/(FLASH_PAGE_SIZE))
#define NB_SECTOR_FLASH_NVM (NVM_SIZE_FLASH/SECTOR_SIZE)
#define NVM_SIZE_32_BITS (NVM_SIZE_FLASH/4)

typedef struct {
	NVM_Sector id_sector;
	uint8_t *ram_ptr;
	size_t sector_size;

} NVM_Sector_Struct;

static void FM_EraseCallback(FM_FlashOp_Status_t Status);
static void FM_EraseBeforeWriteCallback(FM_FlashOp_Status_t Status);
static void FM_WriteCallback(FM_FlashOp_Status_t Status);

static FM_CallbackNode_t FM_EraseStatusCallback = {
/* Header for chained list */
.NodeList = { .next = NULL, .prev = NULL },
/* Callback for request status */
.Callback = FM_EraseCallback };

static FM_CallbackNode_t FM_EraseBeforeWriteStatusCallback = {
/* Header for chained list */
.NodeList = { .next = NULL, .prev = NULL },
/* Callback for request status */
.Callback = FM_EraseBeforeWriteCallback };

static FM_CallbackNode_t FM_WriteStatusCallback = {
/* Header for chained list */
.NodeList = { .next = NULL, .prev = NULL },
/* Callback for request status */
.Callback = FM_WriteCallback };

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
uint8_t ram_nvm[SECTOR_SIZE_SECURE + SECTOR_SIZE_NO_SECURE] = { 0 };
static osSemaphoreId_t FMSemaphore;

const NVM_Sector_Struct sector_no_secure = { .id_sector = SECTOR_NO_SECURE,
		.ram_ptr = ram_nvm + SECTOR_SIZE_SECURE, .sector_size =
		SECTOR_SIZE_NO_SECURE };

//*SIMULATE TO EXAMPLE*
const NVM_Sector_Struct sector_secure = { .id_sector = SECTOR_SECURE, .ram_ptr =
		ram_nvm, .sector_size = SECTOR_SIZE_SECURE };

/* Global variables ----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static NVM_StatusTypeDef flash_get(uint8_t *KeyValue, uint8_t *KeyAddr, size_t KeySize,
		size_t *read_by_size);
static NVM_StatusTypeDef flash_update(const NVM_Sector_Struct select_sector,
		uint8_t *KeyName, uint8_t *KeyValue, size_t KeySize);
static NVM_StatusTypeDef flash_replace(const NVM_Sector_Struct select_sector,
		uint8_t *PtKeyfind, uint8_t *KeyName, uint8_t *KeyValue, size_t KeySize);
static NVM_StatusTypeDef flash_write(uint8_t *PtKeyFree, uint8_t *key,
		uint8_t *value, size_t value_size);
static uint8_t* SearchKey(uint8_t *PtPage, uint8_t *KeyName, size_t nvm_size);
static NVM_StatusTypeDef delete_key(const NVM_Sector_Struct select_sector,
		uint8_t *PtkeyFind);

void LockFMThread(void);
void UnLockFMThread(void);

static void FM_WriteCallback(FM_FlashOp_Status_t Status) {
	/* Update status */
	UnLockFMThread();
}

static void FM_EraseCallback(FM_FlashOp_Status_t Status) {
	/* Update status */
	UnLockFMThread();
	while (1) {
		NVIC_SystemReset();
	}
}

static void FM_EraseBeforeWriteCallback(FM_FlashOp_Status_t Status) {
	/* Update status */
	FM_Write((uint32_t*) ram_nvm, NVM_MATTER_ADDR_INIT_PTR,
	NVM_SIZE_32_BITS, &FM_WriteStatusCallback);

}

/* Public functions ----------------------------------------------------------*/

NVM_StatusTypeDef NM_Init(void) {
	memset(ram_nvm, DEFAULT_VALUE, sizeof(ram_nvm));
	memcpy(ram_nvm, NVM_MATTER_ADDR_INIT_PTR, sizeof(ram_nvm));
		/* Create thread for FM and semaphore to control it*/
		FMSemaphore = osSemaphoreNew(1, 1, NULL);
	if (FMSemaphore == NULL) {
		APP_DBG( "ERROR FREERTOS : FMSemaphore SEMAPHORE CREATION FAILED" );
		while (1)
			;
		}
	return NVM_OK;

}

NVM_StatusTypeDef NM_Dump(void) {
	// Mutex will be release after the write
	LockFMThread();
	FM_Erase(START_SECTOR_FLASH_NVM, NB_SECTOR_FLASH_NVM,
	         &FM_EraseBeforeWriteStatusCallback);
	// write when erase is finish in the CallBack
	return NVM_OK;
}

NVM_StatusTypeDef NM_GetKeyExists(const char *KeyName, NVM_Sector sector) {

	NVM_StatusTypeDef status = NVM_KEY_NOT_FOUND;

	if (KeyName == NULL) {
		return NVM_PARAM_ERROR;
	}
	NVM_Sector_Struct select_nvm = { 0 };
	switch (sector) {
	case SECTOR_NO_SECURE:
		select_nvm = sector_no_secure;
		break;

	case SECTOR_SECURE:
		select_nvm = sector_secure;
		break;

	default:
		return NVM_WRITE_FAILED;
	}

	uint8_t *key_search = SearchKey(select_nvm.ram_ptr, (uint8_t*) KeyName,
			select_nvm.sector_size);
	if (key_search != NULL) {
		status = NVM_OK;
	}
	return status;
}

NVM_StatusTypeDef NM_GetKeyValue(void *KeyValue, const char *KeyName,
		uint32_t KeySize, size_t *read_by_size, NVM_Sector sector) {

	if (KeyValue == NULL) {
		return NVM_PARAM_ERROR;
	}
	NVM_Sector_Struct select_nvm = { 0 };
	switch (sector) {
	case SECTOR_NO_SECURE:
		select_nvm = sector_no_secure;
		break;

	case SECTOR_SECURE:
		select_nvm = sector_secure;
		break;

	default:
		return NVM_WRITE_FAILED;
	}

	uint8_t *key_search = SearchKey(select_nvm.ram_ptr, (uint8_t*) KeyName,
			select_nvm.sector_size);
	if (key_search != NULL) {
		// copy Keyname's value in KeyValue and copy the size of KeyValue in read_by_size
		return flash_get(KeyValue, key_search, KeySize, read_by_size);
	}
	return NVM_KEY_NOT_FOUND;
}

NVM_StatusTypeDef NM_GetOtNVMAddr(uint32_t *NVMAddr) {
	if (NVMAddr == NULL) {
		return NVM_PARAM_ERROR;
	}
	*NVMAddr = (uint32_t) ram_nvm + SECTOR_SIZE_SECURE;
	return NVM_OK;
}

NVM_StatusTypeDef NM_SetKeyValue(char *KeyValue, char *KeyName,
		uint32_t KeySize, NVM_Sector sector) {

	if ((KeyValue == NULL) || (KeyName == NULL)) {
		return NVM_PARAM_ERROR;
	}
	NVM_Sector_Struct select_nvm = { 0 };
	void *Ptkey = NULL;

	switch (sector) {
	case SECTOR_NO_SECURE:
		select_nvm = sector_no_secure;
		break;

	case SECTOR_SECURE:
		select_nvm = sector_secure;
		break;

	default:
		return NVM_WRITE_FAILED;
	}

	if (KeySize > NVM_BLOCK_SIZE) {
		return NVM_BLOCK_SIZE_OVERFLOW;
	}
	// call function to search the pointer of key if it exist else return  null
	Ptkey = SearchKey(select_nvm.ram_ptr, (uint8_t*) KeyName,
			select_nvm.sector_size);

	if (Ptkey == NULL) {
		return flash_update(select_nvm, (uint8_t*) KeyName, (uint8_t*) KeyValue,
				KeySize);
	} else {
		if (!flash_replace(select_nvm, Ptkey, (uint8_t*) KeyName,
				(uint8_t*) KeyValue, KeySize)) {
			return NVM_OK;
		}
	}
	return NVM_WRITE_FAILED;
}

NVM_StatusTypeDef NM_DeleteKey(const char *Keyname, NVM_Sector sector) {

	if (Keyname == NULL) {
		return NVM_PARAM_ERROR;
	}
	NVM_Sector_Struct select_nvm = { 0 };
	switch (sector) {
	case SECTOR_NO_SECURE:
		select_nvm = sector_no_secure;
		break;

	case SECTOR_SECURE:
		select_nvm = sector_secure;
		break;

	default:
		return NVM_WRITE_FAILED;
	}
	uint8_t *Ptkey = SearchKey(select_nvm.ram_ptr, (uint8_t*) Keyname,
			select_nvm.sector_size);
	if (Ptkey != NULL) {
		return delete_key(select_nvm, Ptkey);
	}
	return NVM_KEY_NOT_FOUND;
}

void NM_ResetFactory(void) {
	LockFMThread();
	FM_Erase(START_SECTOR_FLASH_NVM, NB_SECTOR_FLASH_NVM,
			&FM_EraseStatusCallback);

}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

static uint8_t* SearchKey(uint8_t *PtPage, uint8_t *KeyName, size_t nvm_size) {

	uint8_t *i = PtPage;
	size_t read_by_size = 0;

	while ((i >= PtPage) || (i < (PtPage + nvm_size))) {
		if (*i != DEFAULT_VALUE) {
			if (strcmp((const char*)KeyName, (const char*) i) == 0) {
				return i;
			}
			read_by_size =
					*(size_t*) ((uint8_t*) i + MATTER_KEY_NAME_MAX_LENGTH);
			// ensure that the size of the data being read does not exceed the remaining size of the buffer
			if (read_by_size + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH
					> nvm_size - (i - PtPage)) {
				return NULL;
			}
			i += read_by_size + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;
			//Flash is corrupted
			if ((i < PtPage) || (i > (PtPage + nvm_size))) {
				NM_ResetFactory();
			}
		} else {
			return NULL;
		}

	}
	return NULL;

}

static NVM_StatusTypeDef flash_get(uint8_t *KeyValue, uint8_t *KeyAddr, size_t KeySize, size_t *read_by_size) {
    size_t size = *(size_t *) ((uint8_t *) KeyAddr + MATTER_KEY_NAME_MAX_LENGTH);
    if (KeySize >= size) {
        memcpy(KeyValue, KeyAddr + MATTER_KEY_NAME_MAX_LENGTH + sizeof(size_t), size);
	if (read_by_size) {
            *read_by_size = size;
        }
        return NVM_OK;
    } else {
        return NVM_BUFFER_TOO_SMALL;
    }

}

static NVM_StatusTypeDef flash_update(const NVM_Sector_Struct select_sector,
		uint8_t *KeyName, uint8_t *KeyValue, size_t KeySize) {

	uint8_t *i = select_sector.ram_ptr;
	size_t read_by_size = 0;
	while (i < (select_sector.ram_ptr + select_sector.sector_size)) {
		if (*i == DEFAULT_VALUE) {
			return flash_write(i, KeyName, KeyValue, KeySize);
		}
		read_by_size = *(size_t*) ((uint8_t*) i + MATTER_KEY_NAME_MAX_LENGTH);
		if (read_by_size > NVM_BLOCK_SIZE) {
			return NVM_ERROR_BLOCK_ALIGN;

		}
		i += read_by_size + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;
	}
	return NVM_SIZE_FULL;
}

static NVM_StatusTypeDef flash_replace(const NVM_Sector_Struct select_sector,
		uint8_t *PtKeyfind, uint8_t *KeyName, uint8_t *KeyValue, size_t KeySize) {

	NVM_StatusTypeDef err = NVM_OK;
	if ((PtKeyfind != NULL) && (KeyName != NULL) && (KeyValue != NULL)) {
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

static NVM_StatusTypeDef delete_key(const NVM_Sector_Struct select_sector,
		uint8_t *PtkeyFind) {
	uint8_t *PtKeyNext = NULL;
	uint8_t *PtKeyCpy = NULL;
	size_t size_key = 0;

	if (PtkeyFind != NULL) {
		size_key =
				*(size_t*) ((uint8_t*) PtkeyFind + MATTER_KEY_NAME_MAX_LENGTH);
		PtKeyNext = PtkeyFind + size_key + MATTER_KEY_NAME_MAX_LENGTH
				+ sizeof(size_key);
		PtKeyCpy = PtkeyFind;
		while ((*PtKeyNext != 0xFF)
				&& (PtKeyNext
						< (select_sector.ram_ptr + select_sector.sector_size))) {
			size_key = *(size_t*) ((uint8_t*) PtKeyNext
					+ MATTER_KEY_NAME_MAX_LENGTH);
			memcpy(PtKeyCpy, PtKeyNext,
					size_key + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH);
			PtKeyCpy += size_key + sizeof(size_t) + MATTER_KEY_NAME_MAX_LENGTH;
			PtKeyNext += size_key + MATTER_KEY_NAME_MAX_LENGTH
					+ sizeof(size_key);
		}
		memset(PtKeyCpy, DEFAULT_VALUE,
				(select_sector.ram_ptr + select_sector.sector_size - PtKeyCpy));
		return NVM_OK;
	}
	return NVM_DELETE_FAILED;
}

static NVM_StatusTypeDef flash_write(uint8_t *PtKeyFree, uint8_t *key,
		uint8_t *value, size_t value_size) {

	if ((PtKeyFree != NULL) && (key != NULL) && (value != NULL)) {

		memset(PtKeyFree, DEFAULT_VALUE, value_size);
		memset(PtKeyFree, 0x00, MATTER_KEY_NAME_MAX_LENGTH);
		memcpy(PtKeyFree, key, strlen((char*) key));
		memcpy(PtKeyFree + MATTER_KEY_NAME_MAX_LENGTH, &value_size,
				sizeof(size_t));
		memcpy(PtKeyFree + MATTER_KEY_NAME_MAX_LENGTH + sizeof(size_t), value,
				value_size);
		return NVM_OK;
	}
	return NVM_WRITE_FAILED;
}

void LockFMThread(void) {
	osSemaphoreAcquire(FMSemaphore, osWaitForever);
}

void UnLockFMThread(void) {
	osSemaphoreRelease(FMSemaphore);
}
