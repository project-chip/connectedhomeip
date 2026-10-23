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

# ==============================================================================
# EFR32 / MCUboot: wrap the (optionally signed) application .bin as Matter OTA.
# ==============================================================================
include(${CHIP_ROOT}/config/zephyr/ota-image.cmake)

set(ZEPHYR_OUTPUT_DIR ${PROJECT_BINARY_DIR}/zephyr)

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
