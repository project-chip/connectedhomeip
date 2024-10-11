/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 * Copyright (c) 2020 Cypress Semiconductor Corporation
 * Copyright (c) 2021 Texas Instruments
 *
 * SPDX-License-Identifier: Apache-2.0
 */
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
/*******************************************************************************/

// clang-fomat off
#ifndef __FLASH_MAP_BACKEND_H__
#define __FLASH_MAP_BACKEND_H__

#include <mcuboot_config/mcuboot_config.h>
#define FLASH_DEVICE_INDEX_MASK (0x7F)
#define FLASH_DEVICE_GET_EXT_INDEX(n) ((n) &FLASH_DEVICE_INDEX_MASK)
#define FLASH_DEVICE_EXTERNAL_FLAG (0x80)
#define FLASH_DEVICE_INTERNAL_FLASH (0x7F)
#define FLASH_DEVICE_EXTERNAL_FLASH(index) (FLASH_DEVICE_EXTERNAL_FLAG | index)

#ifndef TI_BOOT_EXTERNAL_DEVICE_INDEX
/* assume first(one) SMIF device is used */
#define TI_BOOT_EXTERNAL_DEVICE_INDEX (0)
#endif

/*
 * ============== XXX: Definitions changed from upstream example ==============
 *
 * These definitions have been modified to work with the Matter examples.
 * Currently the example application takes up as much flash space as possible
 * in the SoC. This may change in the future when a secure image is produced.
 *
 * This is for external flash storage of the secondary image
 */
#ifndef TI_BOOT_USE_EXTERNAL_FLASH
#error "This file must be used with TI_BOOT_USE_EXTERNAL_FLASH enabled"
#endif

#if defined(DeviceFamily_CC13X4) || defined(DeviceFamily_CC26X4)
#if (MCUBOOT_IMAGE_NUMBER == 2)
#define BOOT_SLOT_1_SIZE 0x0002B000
#define BOOT_SLOT_2_SIZE 0x000CC800

/* Internal Flash locations */
#define BOOTLOADER_BASE_ADDRESS 0x00000800
#define BOOT_BOOTLOADER_SIZE 0x00005800

#define BOOT_PRIMARY_1_BASE_ADDRESS (BOOTLOADER_BASE_ADDRESS + BOOT_BOOTLOADER_SIZE)
#define BOOT_PRIMARY_1_SIZE BOOT_SLOT_1_SIZE

#define BOOT_PRIMARY_2_BASE_ADDRESS (BOOT_PRIMARY_1_BASE_ADDRESS + BOOT_PRIMARY_1_SIZE)
#define BOOT_PRIMARY_2_SIZE BOOT_SLOT_2_SIZE

/* EXT Flash locations */
#define BOOT_SECONDARY_1_BASE_ADDRESS 0x00000000
#define BOOT_SECONDARY_1_SIZE BOOT_SLOT_1_SIZE

#define BOOT_SECONDARY_2_BASE_ADDRESS (BOOT_SECONDARY_1_BASE_ADDRESS + BOOT_SECONDARY_1_SIZE)
#define BOOT_SECONDARY_2_SIZE BOOT_SLOT_2_SIZE
#else
#define BOOT_SLOT_1_SIZE 0x000F2000

/* Internal Flash locations */
#define BOOTLOADER_BASE_ADDRESS 0x00000000
#define BOOT_BOOTLOADER_SIZE 0x00006000

#define BOOT_PRIMARY_1_BASE_ADDRESS (BOOTLOADER_BASE_ADDRESS + BOOT_BOOTLOADER_SIZE)
#define BOOT_PRIMARY_1_SIZE BOOT_SLOT_1_SIZE

/* EXT Flash locations */
#define BOOT_SECONDARY_1_BASE_ADDRESS 0x00000000
#define BOOT_SECONDARY_1_SIZE BOOT_SLOT_1_SIZE
#endif

#else
#error "DeviceFamily not recognized, is MCUBoot meant to be used?"

#endif /* DeviceFamily_XXXXXX */

/**
 *
 * Provides abstraction of flash regions for type of use.
 * I.e. dude where's my image?
 *
 * System will contain a map which contains flash areas. Every
 * region will contain flash identifier, offset within flash and length.
 *
 * 1. This system map could be in a file within filesystem (Initializer
 * must know/figure out where the filesystem is at).
 * 2. Map could be at fixed location for project (compiled to code)
 * 3. Map could be at specific place in flash (put in place at mfg time).
 *
 * Note that the map you use must be valid for BSP it's for,
 * match the linker scripts when platform executes from flash,
 * and match the target offset specified in download script.
 */
#include <inttypes.h>

/**
 * @brief Structure describing an area on a flash device.
 *
 * Multiple flash devices may be available in the system, each of
 * which may have its own areas. For this reason, flash areas track
 * which flash device they are part of.
 */
struct flash_area
{
    /**
     * This flash area's ID; unique in the system.
     */
    uint8_t fa_id;

    /**
     * ID of the flash device this area is a part of.
     */
    uint8_t fa_device_id;

    uint16_t pad16;

    /**
     * This area's offset, relative to the beginning of its flash
     * device's storage.
     */
    uint32_t fa_off;

    /**
     * This area's size, in bytes.
     */
    uint32_t fa_size;
};

/**
 * @brief Structure describing a sector within a flash area.
 *
 * Each sector has an offset relative to the start of its flash area
 * (NOT relative to the start of its flash device), and a size. A
 * flash area may contain sectors with different sizes.
 */
struct flash_sector
{
    /**
     * Offset of this sector, from the start of its flash area (not device).
     */
    uint32_t fs_off;

    /**
     * Size of this sector, in bytes.
     */
    uint32_t fs_size;
};

struct flash_map_entry
{
    uint32_t magic;
    struct flash_area area;
    unsigned int ref_count;
};

/*
 * Retrieve a memory-mapped flash device's base address.
 * On success, the address will be stored in the value pointed to by
 * ret.
 * Returns 0 on success, or an error code on failure.
 */
int flash_device_base(uint8_t fd_id, uintptr_t * ret);

/*< Opens the area for use. id is one of the `fa_id`s */
int flash_area_open(uint8_t id, const struct flash_area **);
void flash_area_close(const struct flash_area *);
/*< Reads `len` bytes of flash memory at `off` to the buffer at `dst` */
int flash_area_read(const struct flash_area *, uint32_t off, void * dst, uint32_t len);
/*< Writes `len` bytes of flash memory at `off` from the buffer at `src` */
int flash_area_write(const struct flash_area *, uint32_t off, const void * src, uint32_t len);
/*< Erases `len` bytes of flash memory at `off` */
int flash_area_erase(const struct flash_area *, uint32_t off, uint32_t len);
/*< Returns this `flash_area`s alignment */
size_t flash_area_align(const struct flash_area *);
/*< Initializes an array of flash_area elements for the slot's sectors */
int flash_area_to_sectors(int idx, int * cnt, struct flash_area * ret);
/*< Returns the `fa_id` for slot, where slot is 0 (primary) or 1 (secondary) */
int flash_area_id_from_image_slot(int slot);
/*< Returns the slot, for the `fa_id` supplied */
int flash_area_id_to_image_slot(int area_id);

int flash_area_id_from_multi_image_slot(int image_index, int slot);
int flash_area_id_to_multi_image_slot(int image_index, int area_id);
#ifdef MCUBOOT_USE_FLASH_AREA_GET_SECTORS
int flash_area_get_sectors(int idx, uint32_t * cnt, struct flash_sector * ret);
#endif
#ifdef MCUBOOT_HW_ROLLBACK_PROT
void flash_area_lock(const struct flash_area * fa);
#endif
/*
 * Returns the value expected to be read when accesing any erased
 * flash byte.
 */
uint8_t flash_area_erased_val(const struct flash_area * fap);

/*
 * Reads len bytes from off, and checks if the read data is erased.
 *
 * Returns 1 if erased, 0 if non-erased, and -1 on failure.
 */
int flash_area_read_is_empty(const struct flash_area * fa, uint32_t off, void * dst, uint32_t len);

#endif /* __FLASH_MAP_BACKEND_H__ */

// clang-format on
