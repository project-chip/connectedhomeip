/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    Copyright (c) 2024 NXP
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef _PERIPHERALS_H_
#define _PERIPHERALS_H_

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "fsl_common.h"

#include "lfs.h"

#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

/***********************************************************************************************************************
 * User definitions
 **********************************************************************************************************************/
extern uint32_t NV_STORAGE_START_ADDRESS[];
extern uint32_t NV_STORAGE_MAX_SECTORS[];

#define LITTLEFS_START_ADDR (uint32_t)(NV_STORAGE_START_ADDRESS)

/***********************************************************************************************************************
 * Definitions
 **********************************************************************************************************************/
/* Definitions for BOARD_InitPeripherals functional group */
/* Maximum block read size definition */
#define LITTLEFS_READ_SIZE 16
/* Maximum block program size definition */
#define LITTLEFS_PROG_SIZE 128
/* Erasable block size definition */
#define LITTLEFS_BLOCK_SIZE 8192
/* Block count definition */
#define LITTLEFS_BLOCK_COUNT (uint32_t)(NV_STORAGE_MAX_SECTORS)
/* Block cycles definition */
#define LITTLEFS_BLOCK_CYCLES 100
/* Minimum block cache size definition */
#define LITTLEFS_CACHE_SIZE 1024
/* Minimum lookahead buffer size definition */
#define LITTLEFS_LOOKAHEAD_SIZE 16
/* Block starting address definition */
#define LITTLEFS_START_ADDR (uint32_t)(NV_STORAGE_START_ADDRESS)

/***********************************************************************************************************************
 * Global variables
 **********************************************************************************************************************/
/* LittleFS configuration */
extern const struct lfs_config LittleFS_config;

/***********************************************************************************************************************
 * Callback functions
 **********************************************************************************************************************/
/* LittleFS read a block region callback*/
extern int lfs_mflash_read(const struct lfs_config *, lfs_block_t, lfs_off_t, void *, lfs_size_t);
/* LittleFS program a block region callback*/
extern int lfs_mflash_prog(const struct lfs_config *, lfs_block_t, lfs_off_t, const void *, lfs_size_t);
/* LittleFS erase a block callback*/
extern int lfs_mflash_erase(const struct lfs_config *, lfs_block_t);
/* LittleFS state sync callback*/
extern int lfs_mflash_sync(const struct lfs_config *);
/* LittleFS state lock callback*/
extern int lfs_mutex_lock(const struct lfs_config *);
/* LittleFS state unlock callback*/
extern int lfs_mutex_unlock(const struct lfs_config *);

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/

void BOARD_InitPeripherals(void);

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/

void BOARD_InitBootPeripherals(void);

#if defined(__cplusplus)
}
#endif

#endif /* _PERIPHERALS_H_ */
