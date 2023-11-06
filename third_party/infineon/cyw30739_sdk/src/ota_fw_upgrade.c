/*
 * Copyright 2016-2021, Cypress Semiconductor Corporation (an Infineon company) or
 * an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
 *
 * This software, including source code, documentation and related
 * materials ("Software") is owned by Cypress Semiconductor Corporation
 * or one of its affiliates ("Cypress") and is protected by and subject to
 * worldwide patent protection (United States and foreign),
 * United States copyright laws and international treaty provisions.
 * Therefore, you may use this Software only as provided in the license
 * agreement accompanying the software package from which you
 * obtained this Software ("EULA").
 * If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
 * non-transferable license to copy, modify, and compile the Software
 * source code solely for use in connection with Cypress's
 * integrated circuit products.  Any reproduction, modification, translation,
 * compilation, or representation of this Software except as specified
 * above is prohibited without the express written permission of Cypress.
 *
 * Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
 * reserves the right to make changes to the Software without notice. Cypress
 * does not assume any liability arising out of the application or use of the
 * Software or any product or circuit described in the Software. Cypress does
 * not authorize its products for use in any products where a malfunction or
 * failure of the Cypress product may reasonably be expected to result in
 * significant property damage, injury or death ("High Risk Product"). By
 * including Cypress's product in a High Risk Product, the manufacturer
 * of such system or application assumes all risk of such use and in doing
 * so agrees to indemnify Cypress against all liability.
 */

/** @file
 *
 * WICED Firmware Upgrade internal definitions specific to shim layer
 *
 * This file provides common functions required to support WICED Smart Ready Upgrade
 * whether it is being done over the air, UART, or SPI.  Primarily the
 * functionality is provided for storing and retrieving information from  Serial Flash
 * The data being stored is DS portion of burn image generated from CGS.
 */
#include <ota_fw_upgrade.h>
#include <stdio.h>
#include <wiced_bt_ota_firmware_upgrade.h>
#include <wiced_hal_eflash.h>
#include <wiced_hal_wdog.h>

#define EF_PAGE_SIZE (0x1000u)

//==================================================================================================
// Types
//==================================================================================================
//! Structure for FOUNDATION_CONFIG_ITEM_ID_CONFIG_LAYOUT.
#pragma pack(1)
typedef struct
{
    //! Base address or offset of the failsafe (not upgradable) dynamic section base.  This field
    //! must be present.
    UINT32 failsafe_ds_base;

    //! Base address or offset of the upgradable dynamic section base.  This field is optional for
    //! media types for which DFU is supported.
    UINT32 upgradable_ds_base;

    //! Base address or offset to the area reserved for volatile section copy 1.  Whether this is an
    //! address or offset depends on the media type, and is an internal detail of those media types'
    //! access functions.  Double-buffering of the volatile section alternates between the two
    //! copies when the active copy fills up and has to be consolidated to the other.  The volatile
    //! section stores information that is mutable at runtime, and is therefore subject to loss if a
    //! write operation is interrupted by loss of power.  Only an item that is currently being
    //! written is subject to loss.  Generally, NVRAM media with large page sizes (like flash) use
    //! double-buffering, while media with small page sizes (like EEPROM) allocate one or more
    //! complete pages per volatile section item.
    UINT32 vs_copy1_base;

    //! Base address or offset to the area reserved for volatile section copy 2.  Whether this is an
    //! address or offset depends on the media type, and is an internal detail of those media types'
    //! access functions.  See the documentation for vs_copy1_base, but note that not all media
    //! types use double-buffering.
    UINT32 vs_copy2_base;

    //! Length in bytes per copy of the area reserved for each volatile section copy.  If the target
    //! media uses double buffering to protect against loss, the total space used by the volatile
    //! section is twice this amount.  See the documentation for vs_copy1_base and vs_copy1_base.
    UINT32 vs_length_per_copy;

    //! Block size for volatile section items.  For media with small page sizes (like EEPROM) which
    //! allocate one or more pages per volatile section item, blocks must be a multiple of the media
    //! page size.
    UINT32 vs_block_size;

    //! Media page size.  This info is needed for managing volatile section contents.
    UINT32 media_page_size;
} FOUNDATION_CONFIG_ITEM_CONFIG_LAYOUT_t;
#pragma pack()

//! Enumeration used to specify one of the three sections of config data.
//!                                                                                         <br><br>
//! If config data is stored in NVRAM:
//!                                                                                         <br><br>
//! Static section is written once during manufacturing, and never again.  This section includes
//! per-device information like crystal trimming information and an assigned address like BD_ADDR
//! for Bluetooth devices or a MAC address for ethernet or WLAN devices.  The static section also
//! includes key layout information like whether a volatile section is present and if so, where it
//! is located.
//!                                                                                         <br><br>
//! Dynamic section is written during manufacturing.  This section might be subject to upgrades in
//! the field, by the end user.  An example of such an upgrade process is USB device firmware
//! upgrade.  If this section is subject to upgrade in the field, then a failsafe config must be
//! present, which if present would either force the device into an upgrade-only mode, or fall back
//! to the un-upgraded behavior it would have exhibited when it left the factory.
//!                                                                                         <br><br>
//! Volatile section is used to hold information that can change at runtime, for example storing
//! pairing information for pairing with other devices.  The volatile section is implemented as
//! failsafe as possible for the target media, such that the most recently written "nugget" of
//! information is subject to loss, but contents that were present before a given write operation
//! will be preserved.
//!                                                                                         <br><br>
//! The "volatile" nomenclature is somewhat misleading because this section is only ever present on
//! NVRAM (nonvolatile memory).  The "volatile" nomenclature is simply used to highlight the fact
//! that the contents are subject to loss.  This is generally a non-issue, but if multiple "nuggets"
//! of information are interdependent but written independently, then it is possible for one
//! "nugget" in the interdependent set to be lost, in which case the firmware that uses this
//! information needs to be ready to recognize that situation and take appropriate action to discard
//! or if possible repair the rest of the set.  If no "nuggets" of volatile information form
//! interdependent sets then loss of power during a write operation is functionally equivalent to
//! loss of power immediately before the write operation was initiated.
//!                                                                                         <br><br>
//! If config data is stored in RAM (downloaded by the host):
//!                                                                                         <br><br>
//! Only the static and dynamic sections are relevant.  The distinction between the two halves is
//! more or less irrelevant, merely being a reflection of the NVRAM organization.  Nonetheless, the
//! location in which certain pieces of information are stored is influenced by the NVRAM
//! organization.  A volatile section should never be specified for RAM config data.
typedef enum
{
    //! Configuration data section containing per-device information and key layout information.
    //! The layout information communicates to firmware where to find the rest of the configuration
    //! data.  See the documentation for the config_section_id_t enumeration as a whole for more
    //! complete info.
    CONFIG_STATIC,

    //! Configuration data section containing per-product or product family information.  See the
    //! documentation for the config_section_id_t enumeration as a whole for more complete info.
    CONFIG_DYNAMIC,

    //! Configuration data section in NVRAM containing information that can be changed at runtime.
    //! This refers to info that needs to be preserved across resets or power cycles.  See the
    //! documentation for the config_section_id_t enumeration as a whole for more complete info,
    //! including where the seemingly contradictory name comes from.
    CONFIG_VOLATILE
} config_section_id_t;

//! \internal
//! Structure used internally by the config module to achieve config media abstraction.  It stores
//! layout information for any supported config data media type, as well as media-specific function
//! pointers for various tasks.
typedef struct
{
    //! Access function pointer to read raw data from the media on which config data is stored.
    void (*fp_ReadRaw)(int offset, config_section_id_t which_section, OUT BYTE * buffer, int length);

    //! Access function pointer to write raw data to the media on which config data is stored.
    void (*fp_WriteRaw)(int offset, config_section_id_t which_section, IN BYTE * buffer, int length);

    //! Address of the static section.
    UINT32 ss_base;

    //! Function to handle when the layout config item below has been filled in.  It will have been
    //! filled in using content from the static section, then this function will be called.
    void (*fp_ConfigLayoutHasBeenSet)(void);

    //! Address of the valid dynamic section (which might be the failsafe copy, or might be the
    //! upgradable copy).
    UINT32 active_ds_base;

    //! Access function pointer to read a volatile section item from config data.  The function is
    //! presented as being specific to the type of media, but it really reflects the partitioning
    //! scheme used by this media as dictated by its physical page size.  The truly media-specific
    //! access function is in fp_ReadRaw.
    UINT16 (*fp_ReadVolatileSectionItem)(UINT16 group_id, UINT16 sub_id_in_group, OUT BYTE * buffer, UINT16 max_length);

    //! Access function pointer to write a volatile section item to config data.  The function is
    //! presented as being specific to the type of media, but it really reflects the partitioning
    //! scheme used by this media as dictated by its physical page size.  The truly media-specific
    //! access function is in fp_WriteRaw.
    void (*fp_WriteVolatileSectionItem)(UINT16 group_id, UINT16 sub_id_in_group, IN BYTE * buffer, UINT16 length);

    //! Layout info, retrieved from the static section.
    FOUNDATION_CONFIG_ITEM_CONFIG_LAYOUT_t layout;

    //! Checksum/CRC info for validating segment by segment in the dynamic section.
    UINT32 checksum;
    UINT32 crc32;
    BOOL8 valid_crc32;

    //! Used to allow faster access to the config if it is memory mapped (not in serial flash for example)
    BOOL8 direct_access;

    //! Whether a valid DS section was found or not.
    BOOL8 valid_ds_found;
} CONFIG_INFO_t;

typedef struct ds_header
{
    char signature[8];
    uint32_t crc32;
    uint32_t length;
    uint8_t data[0];
} ds_header_t;

typedef struct upgrade_xs
{
    ds_header_t ds_header;
    uint32_t crc32;
    uint32_t length;
    uint32_t compressed_data_crc32;
    uint32_t compressed_data_length;
    uint8_t compressed_data[0];
} upgrade_xs_t;

extern const CONFIG_INFO_t g_config_Info;

static uint32_t upgrade_location_write(uint32_t offset, const uint8_t * data, uint32_t len);
static bool lzss_decompress(const void * src, size_t n, bool (*data_writer)(uint32_t, int))
    __attribute__((section(".text_in_ram")));
static bool xs_data_writer(uint32_t data_offset, int c) __attribute__((section(".text_in_ram")));
static uint32_t calc_crc32(const uint8_t * buf, uint32_t len) __attribute__((section(".text_in_ram")));
static uint32_t ef_offset(uint32_t offset) __attribute__((section(".text_in_ram")));
static uint32_t upgrade_ds_location(void);

/******************************************************
 *               Function Definitions
 ******************************************************/
__attribute__((section(".init_code"))) void wiced_firmware_upgrade_bootloader(void)
{
    const ds_header_t * ds_header   = (const ds_header_t *) g_config_Info.active_ds_base;
    const upgrade_xs_t * upgrade_xs = (upgrade_xs_t *) XS_LOCATION_UPGRADE;

    /* Check the DS header of the upgrade XS */
    if (memcmp(&upgrade_xs->ds_header, ds_header, sizeof(*ds_header)) != 0)
    {
        return;
    }

    /* Erase the active XS */
    if (WICED_SUCCESS != wiced_hal_eflash_erase(ef_offset(XS_LOCATION_ACTIVE), upgrade_xs->length))
    {
        return;
    }

    /* Copy the upgrade XS to the active XS */
    if (!lzss_decompress(upgrade_xs->compressed_data, upgrade_xs->compressed_data_length, xs_data_writer))
    {
        goto reset;
    }

    /* Verify the active XS */
    if (calc_crc32((void *) XS_LOCATION_ACTIVE, upgrade_xs->length) != upgrade_xs->crc32)
        goto reset;

    /* Erase the upgrade XS */
    wiced_hal_eflash_erase(ef_offset(XS_LOCATION_UPGRADE), EF_PAGE_SIZE);

    return;

reset:
    wiced_hal_wdog_reset_system();
    while (1)
        ;
}

bool wiced_firmware_upgrade_prepare(void)
{
    const uint32_t ds1_length = g_config_Info.layout.upgradable_ds_base - g_config_Info.layout.failsafe_ds_base;
    const uint32_t ds2_length = XS_LOCATION_ACTIVE - g_config_Info.layout.upgradable_ds_base;

    printf("Active DS: 0x%08x\n", g_config_Info.active_ds_base);
    printf("Active XS: 0x%08x\n", XS_LOCATION_ACTIVE);

    if (upgrade_ds_location() == 0 || ds1_length != ds2_length)
    {
        return false;
    }

    printf("Erasing Upgrade DS: 0x%08lx, len: 0x%08lx\n", upgrade_ds_location(), ds1_length);
    if (WICED_SUCCESS != wiced_hal_eflash_erase(ef_offset(upgrade_ds_location()), ds1_length))
    {
        printf("ERROR erase\n");
        return false;
    }

    const uint32_t upgrade_xs_length = FLASH_SIZE - ef_offset(XS_LOCATION_UPGRADE);
    printf("Erasing Upgrade XS: 0x%08x, len: 0x%08lx\n", XS_LOCATION_UPGRADE, upgrade_xs_length);
    if (WICED_SUCCESS != wiced_hal_eflash_erase(ef_offset(XS_LOCATION_UPGRADE), upgrade_xs_length))
    {
        printf("ERROR erase\n");
        return false;
    }

    return true;
}

uint32_t wiced_firmware_upgrade_process_block(uint32_t offset, const uint8_t * data, uint32_t len)
{
    const ds_header_t * ds_header = (const ds_header_t *) upgrade_ds_location();

    if (offset == 0)
    {
        ds_header = (const ds_header_t *) data;
    }
    else
    {
        ds_header = (const ds_header_t *) upgrade_ds_location();
    }

    const uint32_t ds_length = sizeof(*ds_header) + ds_header->length;
    uint32_t ds_write_length;
    uint32_t xs_write_length;
    if (offset < ds_length)
    {
        if (offset + len <= ds_length)
        {
            ds_write_length = len;
            xs_write_length = 0;
        }
        else
        {
            ds_write_length = ds_length - offset;
            xs_write_length = len - ds_write_length;
        }
    }
    else
    {
        ds_write_length = 0;
        xs_write_length = len;
    }

    uint32_t byte_written = 0;
    if (ds_write_length > 0)
    {
        const uint32_t ds_offset = offset + upgrade_ds_location();
        byte_written += upgrade_location_write(ds_offset, data, ds_write_length);
    }
    if (xs_write_length > 0)
    {
        const uint32_t xs_offset = offset + ds_write_length - ds_length + XS_LOCATION_UPGRADE;
        byte_written += upgrade_location_write(xs_offset, data + ds_write_length, xs_write_length);
    }
    return byte_written;
}

uint32_t upgrade_location_write(uint32_t offset, const uint8_t * data, uint32_t len)
{
    // reserve first 4 bytes of download to commit when complete, in case of unexpected power loss
    // boot rom checks this signature to validate DS
    uint32_t offset_adjustment;
    if (offset == upgrade_ds_location())
    {
        offset_adjustment = 4;
    }
    else
    {
        offset_adjustment = 0;
    }
    offset += offset_adjustment;
    data += offset_adjustment;
    len -= offset_adjustment;

    printf("write: offset: 0x%08lx len: %lu\n", offset, len);
    /* write length should in words */
    if (wiced_hal_eflash_write(ef_offset(offset), (uint8_t *) data, (len + 3) & 0xfffffffc) == WICED_SUCCESS)
    {
        return len + offset_adjustment;
    }
    else
    {
        printf("ERROR write\n");
        return 0;
    }
}

bool wiced_firmware_upgrade_finalize(void)
{
    const ds_header_t * ds_header   = (ds_header_t *) upgrade_ds_location();
    const upgrade_xs_t * upgrade_xs = (upgrade_xs_t *) XS_LOCATION_UPGRADE;

    const uint32_t ds_crc32 = calc_crc32(ds_header->data, ds_header->length);
    const uint32_t cx_crc32 = calc_crc32(upgrade_xs->compressed_data, upgrade_xs->compressed_data_length);

    printf("DS: length 0x%08lx, crc32 0x%08lx\n", ds_header->length, ds_crc32);
    printf("XS: length 0x%08lx, crc32 0x%08lx\n", upgrade_xs->length, upgrade_xs->crc32);
    printf("CX: length 0x%08lx, crc32 0x%08lx\n", upgrade_xs->compressed_data_length, cx_crc32);

    return ds_header->crc32 == ds_crc32 && upgrade_xs->compressed_data_crc32 == cx_crc32;
}

bool wiced_firmware_upgrade_apply(void)
{
    enum
    {
        SIGNATURE = 0x4d435242,
    };
    wiced_result_t result;
    uint32_t signature = SIGNATURE;

    printf("Switching DS to 0x%08lx\n", upgrade_ds_location());

    // commit reserved first 4 bytes of download to complete
    // this is done last and after crc in case of power loss during download
    // boot rom checks this signature to validate DS, checking DS1 first, then DS2
    wiced_hal_eflash_write(ef_offset(upgrade_ds_location()), (uint8_t *) &signature, 4);

    // check that the write completed
    wiced_hal_eflash_read(ef_offset(upgrade_ds_location()), (uint8_t *) &signature, 4);
    if (signature != SIGNATURE)
    {
        return false;
    }

    // clear first active DS sector in eflash, so that on next boot, CRC check will fail and ROM code boots from upgraded DS
    result = wiced_hal_eflash_erase(ef_offset(g_config_Info.active_ds_base), EF_PAGE_SIZE);
    printf("wiced_hal_eflash_erase status %d\n", result);

    return result == WICED_SUCCESS;
}

void wiced_firmware_upgrade_abort(void) {}

bool lzss_decompress(const void * src, size_t n, bool (*data_writer)(uint32_t, int))
{
    enum
    {
        /* size of ring buffer */
        N = 4096,
        /* upper limit for match_length */
        F = 18,
        /*
         * encode string into position and length
         * if match_length is greater than this
         */
        THRESHOLD = 2,
    };

    /* ring buffer of size N, with extra F-1 bytes to facilitate string comparison */
    static uint8_t ring_buf[N + F - 1];

    int r;
    int c;
    unsigned int flags;
    uint32_t offset        = 0;
    const uint8_t * s      = src;
    const uint8_t * s_stop = s + n;

    memset(ring_buf, 0, N - F);

    r     = N - F;
    flags = 0;
    while (s != s_stop)
    {
        if (((flags >>= 1) & 0x100) == 0)
        {
            c     = *s++;
            flags = c | 0xff00; /* ues higher byte cleverly */
        }                       /* to count eight */
        if (flags & 1)
        {
            c             = *s++;
            ring_buf[r++] = c;
            r &= N - 1;
            if (!data_writer(offset++, c))
                return FALSE;
        }
        else
        {
            int i;
            int patloc = *s++;
            int patlen = *s++;

            patloc |= ((patlen & 0xf0) << 4);
            patlen = (patlen & 0x0f) + THRESHOLD;

            for (i = 0; i <= patlen; i++)
            {
                c             = ring_buf[(patloc + i) & (N - 1)];
                ring_buf[r++] = c;
                r &= N - 1;
                if (!data_writer(offset++, c))
                    return FALSE;
            }
        }
    }
    return data_writer(offset, EOF);
}

bool xs_data_writer(uint32_t data_offset, int c)
{
    static uint8_t xs_data_buf[EF_PAGE_SIZE];

    const uint32_t offset = data_offset % sizeof(xs_data_buf);

    if (c != EOF)
    {
        xs_data_buf[offset] = c;
    }

    uint32_t write_length = 0;
    if (offset + 1 == sizeof(xs_data_buf))
    {
        write_length = offset + 1;
    }
    else if (c == EOF)
    {
        write_length = offset;
    }

    if (write_length > 0)
    {
        const uint32_t write_offset = XS_LOCATION_ACTIVE + data_offset - offset;
        if (WICED_SUCCESS != wiced_hal_eflash_write(ef_offset(write_offset), xs_data_buf, write_length))
        {
            return FALSE;
        }
    }
    return TRUE;
}

uint32_t calc_crc32(const uint8_t * buf, uint32_t len)
{
    uint32_t crc32_Update(uint32_t crc, const uint8_t * buf, uint16_t len);
    uint32_t crc32 = 0xffffffff;
    uint32_t i;

    for (i = 0; i < len; i += UINT16_MAX)
    {
        crc32 = crc32_Update(crc32, buf + i, MIN(len - i, UINT16_MAX));
    }

    return crc32 ^ 0xffffffff;
}

uint32_t ef_offset(uint32_t offset)
{
    return offset - FLASH_BASE_ADDRESS;
}

uint32_t upgrade_ds_location(void)
{
    if (g_config_Info.active_ds_base == g_config_Info.layout.failsafe_ds_base)
    {
        return g_config_Info.layout.upgradable_ds_base;
    }
    else if (g_config_Info.active_ds_base == g_config_Info.layout.upgradable_ds_base)
    {
        return g_config_Info.layout.failsafe_ds_base;
    }
    else
    {
        return 0;
    }
}

/* Dummy stub */
wiced_bool_t wiced_ota_fw_upgrade_init(void * public_key, wiced_ota_firmware_upgrade_status_callback_t * p_status_callback,
                                       wiced_ota_firmware_upgrade_send_data_callback_t * p_send_data_callback)
{
    return TRUE;
}
