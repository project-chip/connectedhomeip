#
#   Copyright (c) 2025 Project CHIP Authors
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

include(${CHIP_ROOT}/config/zephyr/ota-image.cmake)
include(${CHIP_ROOT}/config/nxp/chip-module/generate_factory_data.cmake)

# ==============================================================================
# Define 'chip-ota-image' target for building CHIP OTA image
# ==============================================================================
if(CONFIG_CHIP_OTA_REQUESTOR)
    if(CONFIG_MCUBOOT_SIGNATURE_KEY_FILE STREQUAL "")
        set(ZEPHYR_OUTPUT_NAME "zephyr")
    else()
        set(ZEPHYR_OUTPUT_NAME "zephyr.signed")
    endif()

    set(GLOBAL_BOOTLOADER_CONF_OVERLAY_FILE "${CHIP_ROOT}/config/nxp/app/bootloader.conf")

    set(ZEPHYR_OUTPUT_DIR ${PROJECT_BINARY_DIR}/zephyr)

    add_custom_target(build_mcuboot ALL
        COMMAND
        west build -b ${BOARD} -d build_mcuboot ${ZEPHYR_BASE}/../bootloader/mcuboot/boot/zephyr
            -- -DOVERLAY_CONFIG=${GLOBAL_BOOTLOADER_CONF_OVERLAY_FILE}
            -DEXTRA_DTC_OVERLAY_FILE="${DTC_OVERLAY_FILE};${EXTRA_DTC_OVERLAY_FILE}"
        COMMAND
        cp ${ZEPHYR_OUTPUT_DIR}/../build_mcuboot/zephyr/zephyr.bin ${ZEPHYR_OUTPUT_DIR}/zephyr.mcuboot.bin
    )
    add_dependencies(build_mcuboot app)
    set(BLOCK_SIZE "1024")
    dt_nodelabel(dts_partition_path NODELABEL "boot_partition")
    dt_reg_size(mcuboot_size PATH ${dts_partition_path})
    math(EXPR boot_blocks "${mcuboot_size} / ${BLOCK_SIZE}" OUTPUT_FORMAT DECIMAL)

    add_custom_command( 
        OUTPUT ${ZEPHYR_OUTPUT_DIR}/zephyr_full.bin
        DEPENDS build_mcuboot ${ZEPHYR_OUTPUT_DIR}/${ZEPHYR_OUTPUT_NAME}.bin
        COMMAND dd if=${ZEPHYR_OUTPUT_DIR}/zephyr.mcuboot.bin of=${ZEPHYR_OUTPUT_DIR}/zephyr_full.bin
        COMMAND dd if=${ZEPHYR_OUTPUT_DIR}/${ZEPHYR_OUTPUT_NAME}.bin of=${ZEPHYR_OUTPUT_DIR}/zephyr_full.bin bs=${BLOCK_SIZE} seek=${boot_blocks}
    )

    add_custom_target(merge_mcuboot ALL
        DEPENDS ${ZEPHYR_OUTPUT_DIR}/zephyr_full.bin
    )

    if (CONFIG_CHIP_OTA_IMAGE_BUILD)
        chip_ota_image(chip-ota-image
            INPUT_FILES ${ZEPHYR_OUTPUT_DIR}/${ZEPHYR_OUTPUT_NAME}.bin
            OUTPUT_FILE ${ZEPHYR_OUTPUT_DIR}/${CONFIG_CHIP_OTA_IMAGE_FILE_NAME}
        )
    endif()
endif()

# ==============================================================================
# Define 'factory_data' target for generating a factory data partition
# ==============================================================================

if (CONFIG_CHIP_FACTORY_DATA_BUILD)
    nxp_generate_factory_data()
endif()