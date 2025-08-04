/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef H_BOOTUTIL_
#define H_BOOTUTIL_

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Attempt to boot the contents of the primary slot. */
#define BOOT_SWAP_TYPE_NONE 1

/**
 * Swap to the secondary slot.
 * Absent a confirm command, revert back on next boot.
 */
#define BOOT_SWAP_TYPE_TEST 2

/**
 * Swap to the secondary slot,
 * and permanently switch to booting its contents.
 */
#define BOOT_SWAP_TYPE_PERM 3

/** Swap back to alternate slot.  A confirm changes this state to NONE. */
#define BOOT_SWAP_TYPE_REVERT 4

/** Swap failed because image to be run is not valid */
#define BOOT_SWAP_TYPE_FAIL 5

/** Swapping encountered an unrecoverable error */
#define BOOT_SWAP_TYPE_PANIC 0xff

#define BOOT_MAX_ALIGN 8

/* This is not actually used by mcuboot's code but can be used by apps
 * when attempting to read/write a trailer.
 */
struct image_trailer
{
    uint8_t swap_type;
    uint8_t pad1[BOOT_MAX_ALIGN - 1];
    uint8_t copy_done;
    uint8_t pad2[BOOT_MAX_ALIGN - 1];
    uint8_t image_ok;
    uint8_t pad3[BOOT_MAX_ALIGN - 1];
    uint8_t magic[16];
};

int boot_swap_type_multi(int image_index);
int boot_swap_type(void);

int boot_set_pending(int image, int permanent);
int boot_set_confirmed(void);

#ifdef __cplusplus
}
#endif

#endif
