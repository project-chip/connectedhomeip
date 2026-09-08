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
# Create the Matter OTA image for Silabs Zephyr targets.
#
# EFR32: wrap the MCUboot-signed application .bin.
# SiWx917: wrap the SoC-generated zephyr.rps (NWP / Security Bootloader path).
# ==============================================================================
if(CONFIG_CHIP_OTA_REQUESTOR)
    set(ZEPHYR_OUTPUT_DIR ${PROJECT_BINARY_DIR}/zephyr)

    if(CONFIG_SOC_SERIES_SIWG917)
        # ------------------------------------------------------------------
        # SiWx917: Matter OTA payload is the RPS produced by SoC post-build.
        # Depend on zephyr.bin (known CMake output); post-build creates the RPS
        # in the same step, so it is present when ota_image_tool runs.
        # Do not use add_custom_command(TARGET zephyr_final) — that target is
        # not created in this CMakeLists directory.
        # ------------------------------------------------------------------
        if(CONFIG_SIWX91X_SIGN_KEY OR CONFIG_SIWX91X_MIC_KEY)
            set(SIWX_RPS_INPUT ${ZEPHYR_OUTPUT_DIR}/zephyr.signed.rps)
        else()
            set(SIWX_RPS_INPUT ${ZEPHYR_OUTPUT_DIR}/zephyr.rps)
        endif()

        if(CONFIG_CHIP_OTA_IMAGE_BUILD)
            set(SIWX_OTA_OUTPUT ${ZEPHYR_OUTPUT_DIR}/${CONFIG_CHIP_OTA_IMAGE_FILE_NAME})

            if(DEFINED APPVERSION)
                set(SIWX_OTA_ARGS
                    "--vendor-id" ${CONFIG_CHIP_DEVICE_VENDOR_ID}
                    "--product-id" ${CONFIG_CHIP_DEVICE_PRODUCT_ID}
                    "--version" ${APPVERSION}
                    "--version-str" ${APP_VERSION_EXTENDED_STRING}
                    "--digest-algorithm" "sha256"
                )
            else()
                set(SIWX_OTA_ARGS
                    "--vendor-id" ${CONFIG_CHIP_DEVICE_VENDOR_ID}
                    "--product-id" ${CONFIG_CHIP_DEVICE_PRODUCT_ID}
                    "--version" ${CONFIG_CHIP_DEVICE_SOFTWARE_VERSION}
                    "--version-str" ${CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING}
                    "--digest-algorithm" "sha256"
                )
            endif()

            separate_arguments(SIWX_OTA_EXTRA_ARGS NATIVE_COMMAND "${CONFIG_CHIP_OTA_IMAGE_EXTRA_ARGS}")
            list(APPEND SIWX_OTA_ARGS ${SIWX_OTA_EXTRA_ARGS})
            list(APPEND SIWX_OTA_ARGS ${SIWX_RPS_INPUT} ${SIWX_OTA_OUTPUT})
            string(REPLACE ";" "\n" SIWX_OTA_ARGS_FILE "${SIWX_OTA_ARGS}")
            file(GENERATE OUTPUT ${SIWX_OTA_OUTPUT}.args CONTENT ${SIWX_OTA_ARGS_FILE})

            add_custom_command(
                OUTPUT ${SIWX_OTA_OUTPUT}
                COMMAND ${Python3_EXECUTABLE} ${CHIP_ROOT}/src/app/ota_image_tool.py create @${SIWX_OTA_OUTPUT}.args
                DEPENDS ${ZEPHYR_OUTPUT_DIR}/zephyr.bin ${CHIP_ROOT}/src/app/ota_image_tool.py
                COMMENT "Generating Matter OTA image from ${SIWX_RPS_INPUT}"
                VERBATIM
            )
            add_custom_target(chip-ota-image ALL DEPENDS ${SIWX_OTA_OUTPUT})
        endif()
    else()
        # ------------------------------------------------------------------
        # EFR32 / MCUboot: wrap the signed application binary.
        # ------------------------------------------------------------------
        if(CONFIG_MCUBOOT_SIGNATURE_KEY_FILE STREQUAL "")
            set(ZEPHYR_OUTPUT_NAME "zephyr")
        else()
            set(ZEPHYR_OUTPUT_NAME "zephyr.signed")
        endif()

        if(CONFIG_CHIP_OTA_IMAGE_BUILD)
            chip_ota_image(chip-ota-image
                INPUT_FILES ${ZEPHYR_OUTPUT_DIR}/${ZEPHYR_OUTPUT_NAME}.bin
                OUTPUT_FILE ${ZEPHYR_OUTPUT_DIR}/${CONFIG_CHIP_OTA_IMAGE_FILE_NAME}
            )
        endif()
    endif() # CONFIG_SOC_SERIES_SIWG917
endif() # CONFIG_CHIP_OTA_REQUESTOR
