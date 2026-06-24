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

/***********************************************************************************************************************
 * Included files
 **********************************************************************************************************************/
#include "peripherals.h"

/***********************************************************************************************************************
 * BOARD_InitPeripherals functional group
 **********************************************************************************************************************/

/* LittleFS context */
extern struct lfs_mflash_ctx LittleFS_ctx;
const struct lfs_config LittleFS_config = { .context = (void *) &LittleFS_ctx,
                                            .read    = lfs_mflash_read,
                                            .prog    = lfs_mflash_prog,
                                            .erase   = lfs_mflash_erase,
                                            .sync    = lfs_mflash_sync,
#ifdef LFS_THREADSAFE
                                            .lock   = lfs_mutex_lock,
                                            .unlock = lfs_mutex_unlock,
#endif
                                            .read_size      = LITTLEFS_READ_SIZE,
                                            .prog_size      = LITTLEFS_PROG_SIZE,
                                            .block_size     = LITTLEFS_BLOCK_SIZE,
                                            .block_count    = LITTLEFS_BLOCK_COUNT,
                                            .block_cycles   = LITTLEFS_BLOCK_CYCLES,
                                            .cache_size     = LITTLEFS_CACHE_SIZE,
                                            .lookahead_size = LITTLEFS_LOOKAHEAD_SIZE };

/* Empty initialization function (commented out)
static void LittleFS_init(void) {
} */

/***********************************************************************************************************************
 * Initialization functions
 **********************************************************************************************************************/
void BOARD_InitPeripherals(void)
{
    /* Initialize components */
}

/***********************************************************************************************************************
 * BOARD_InitBootPeripherals function
 **********************************************************************************************************************/
void BOARD_InitBootPeripherals(void)
{
    BOARD_InitPeripherals();
}
