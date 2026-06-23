#
#   Copyright (c) 2026 Project CHIP Authors
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

# ==============================================================================
# Build MCUboot, merge it with the signed application, and create the Matter
# OTA image for Silabs Zephyr targets.
# ==============================================================================
if(CONFIG_CHIP_OTA_REQUESTOR)
    if(CONFIG_MCUBOOT_SIGNATURE_KEY_FILE STREQUAL "")
        set(ZEPHYR_OUTPUT_NAME "zephyr")
    else()
        set(ZEPHYR_OUTPUT_NAME "zephyr.signed")
    endif()

    # The standalone bootloader build does not auto-merge the application's
    # boards/<board>.conf, so append a per-board overlay (when present) for
    # layout-specific options such as overwrite-only and external-slot drivers.
    set(BOOTLOADER_CONF_OVERLAY_FILES "${CHIP_ROOT}/config/silabs/app/bootloader.conf")
    set(BOARD_BOOTLOADER_CONF_OVERLAY_FILE "${APPLICATION_SOURCE_DIR}/boards/${BOARD}_bootloader.conf")
    if(EXISTS ${BOARD_BOOTLOADER_CONF_OVERLAY_FILE})
        list(APPEND BOOTLOADER_CONF_OVERLAY_FILES ${BOARD_BOOTLOADER_CONF_OVERLAY_FILE})
    endif()

    # Simplicity Commander and imgtool resolve CONFIG_BOOT_SIGNATURE_KEY_FILE from
    # the bootloader build directory, so supply it as an absolute path via overlay.
    get_filename_component(MCUBOOT_SIGNING_KEY
        "${ZEPHYR_BASE}/../${CONFIG_MCUBOOT_SIGNATURE_KEY_FILE}" ABSOLUTE)
    set(MCUBOOT_SIGNING_KEY_CONF "${PROJECT_BINARY_DIR}/mcuboot_signing_key.conf")
    file(GENERATE OUTPUT ${MCUBOOT_SIGNING_KEY_CONF}
        CONTENT "CONFIG_BOOT_SIGNATURE_KEY_FILE=\"${MCUBOOT_SIGNING_KEY}\"\n")

    set(ZEPHYR_OUTPUT_DIR ${PROJECT_BINARY_DIR}/zephyr)

    add_custom_target(build_mcuboot ALL
        COMMAND
        west build -b ${BOARD} -d build_mcuboot ${ZEPHYR_BASE}/../bootloader/mcuboot/boot/zephyr
            -- -DOVERLAY_CONFIG="${BOOTLOADER_CONF_OVERLAY_FILES};${MCUBOOT_SIGNING_KEY_CONF}"
            -DEXTRA_DTC_OVERLAY_FILE="${DTC_OVERLAY_FILE};${EXTRA_DTC_OVERLAY_FILE}"
        COMMAND
        ${CMAKE_COMMAND} -E copy ${ZEPHYR_OUTPUT_DIR}/../build_mcuboot/zephyr/zephyr.bin ${ZEPHYR_OUTPUT_DIR}/zephyr.mcuboot.bin
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

    if(CONFIG_CHIP_OTA_IMAGE_BUILD)
        chip_ota_image(chip-ota-image
            INPUT_FILES ${ZEPHYR_OUTPUT_DIR}/${ZEPHYR_OUTPUT_NAME}.bin
            OUTPUT_FILE ${ZEPHYR_OUTPUT_DIR}/${CONFIG_CHIP_OTA_IMAGE_FILE_NAME}
        )
    endif()
endif()
