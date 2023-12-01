# MCUBoot for TI CC13x4 devices 

The TI CC13x4 devices use MCUBoot for OTA operations. MCUBoot can be built separately in the TI Simplelink SDK and loaded onto a CC13x4 device with a MCUBoot compatible Matter image. This guide will go over the configurations needed to build MCUBoot for Matter OTA.

## Building MCUBoot
Import the MCUBoot project from the TI Simplelink SDK for your board into CCS. The example will be located at `{SDK Install Root}/examples/nortos/{board}/mcuboot/mcuboot/gcc` . 

Follow the README instuructions to build the MCUBoot project. Additional configurations needed to make the image compatiblw with Matter are mentioned below:

In the project directory, navigate to `flash_map_backend/flash_map_backend.h` and update `BOOT_PRIMARY_1_SIZE` and `BOOT_PRIMARY_2_SIZE` to have a value of `0xF6000`. 

Next, go to `mcuboot_config/mcuboot_config.h` and make sure that `TI_BOOT_USE_EXTERNAL_FLASH` and `MCUBOOT_OVERWRITE_ONLY` are uncommented. If `MCUBOOT_DIRECT_XIP` is uncommented, comment it out. Update `MCUBOOT_MAX_IMG_SECTORS` to be 492. 

In the project settings, go to Build > GNU Objcopy Utility and enable the utility. Click on "Edit Flags" and add "`-O ihex`". 

You can now build MCUBoot and have an image that can be merged with a MCUBoot-compatible Matter application. 

## Creating image for OTA

The MCUBoot-compatible Matter application will be named `{project_name}_header.hex`. This can be merged with the MCUBoot hex file using the `oad_merge_tool.py` script in `{Matter repo root}/third_party/ti_simplelink_sdk`. Follow the instructions at the top of the script to generate the final hex image. 
