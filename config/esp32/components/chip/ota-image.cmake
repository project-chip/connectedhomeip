#
# SPDX-FileCopyrightText: 2022 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

find_package(Python3 REQUIRED)

#
# Create CMake target for building Matter OTA (Over-the-air update) image.
# Required arguments:
#   INPUT_FILES file1, [file2...] - binary files which Matter OTA image will be composed of
#   OUTPUT_FILE file - where to store newly created Matter OTA image
#
function(chip_ota_image TARGET_NAME)
    cmake_parse_arguments(ARG "" "OUTPUT_FILE" "INPUT_FILES" ${ARGN})

    if (NOT ARG_INPUT_FILES OR NOT ARG_OUTPUT_FILE)
        message(FATAL_ERROR "Both INPUT_FILES and OUTPUT_FILE arguments must be specified")
    endif()

    # Prepare ota_image_tool.py argument list
    set(OTA_ARGS
        "--vendor-id"
        ${CONFIG_DEVICE_VENDOR_ID}
        "--product-id"
        ${CONFIG_DEVICE_PRODUCT_ID}
        "--version"
        ${PROJECT_VER_NUMBER}
        "--version-str"
        ${PROJECT_VER}
        "--digest-algorithm"
        "sha256"
    )

    separate_arguments(OTA_EXTRA_ARGS NATIVE_COMMAND "${CONFIG_CHIP_OTA_IMAGE_EXTRA_ARGS}")

    list(APPEND OTA_ARGS ${OTA_EXTRA_ARGS})
    list(APPEND OTA_ARGS ${ARG_INPUT_FILES})
    list(APPEND OTA_ARGS ${ARG_OUTPUT_FILE})

    # Convert the argument list to multi-line string
    string(REPLACE ";" "\n" OTA_ARGS "${OTA_ARGS}")

    # Pass the argument list via file to avoid hitting Windows command-line length limit
    file(GENERATE
        OUTPUT ${BUILD_DIR}/args-ota-image.tmp
        CONTENT ${OTA_ARGS}
    )

    add_custom_target(${TARGET_NAME} ALL
        COMMAND ${Python3_EXECUTABLE} ${CHIP_ROOT}/src/app/ota_image_tool.py create @${BUILD_DIR}/args-ota-image.tmp
    )
endfunction()
