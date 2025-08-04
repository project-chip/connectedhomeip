/* AUTO-GENERATED FILE, DO NOT EDIT. ALL CHANGES WILL BE LOST! */
/* Platform: PSOC_062_2M */
#ifndef CY_FLASH_MAP_H
#define CY_FLASH_MAP_H

static struct flash_area flash_areas[] = {
    { .fa_id = FLASH_AREA_BOOTLOADER, .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH, .fa_off = 0x0U, .fa_size = 0x28000U },
    { .fa_id = FLASH_AREA_IMG_1_PRIMARY, .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH, .fa_off = 0x28000U, .fa_size = 0x1c0000U },
    { .fa_id        = FLASH_AREA_IMG_1_SECONDARY,
      .fa_device_id = FLASH_DEVICE_EXTERNAL_FLASH(CY_BOOT_EXTERNAL_DEVICE_INDEX),
      .fa_off       = 0x200U,
      .fa_size      = 0x1c0000U },
    { .fa_id = FLASH_AREA_IMAGE_SWAP_STATUS, .fa_device_id = FLASH_DEVICE_INTERNAL_FLASH, .fa_off = 0x1e8000U, .fa_size = 0x6c00U },
    { .fa_id        = FLASH_AREA_IMAGE_SCRATCH,
      .fa_device_id = FLASH_DEVICE_EXTERNAL_FLASH(CY_BOOT_EXTERNAL_DEVICE_INDEX),
      .fa_off       = 0x440000U,
      .fa_size      = 0x80000U }
};

struct flash_area * boot_area_descs[] = { &flash_areas[0U], &flash_areas[1U], &flash_areas[2U],
                                          &flash_areas[3U], &flash_areas[4U], NULL };
#endif /* CY_FLASH_MAP_H */
