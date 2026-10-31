#
#   Copyright (c) 2021-2026 Project CHIP Authors
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

find_package(Python3 REQUIRED)

#
# Create CMake target for building Matter OTA (Over-the-air update) image.
#
# Required arguments:
#   INPUT_FILES file1[, file2...]  Binary files to be included in Matter OTA image
#   OUTPUT_FILE file               Location of generated Matter OTA image
#
# Optional arguments:
#   DEPENDS file1[, file2...]      Build dependencies, if INPUT_FILES have no CMake
#                                  rule of their own (defaults to INPUT_FILES)
#
function(chip_ota_image TARGET_NAME)
    cmake_parse_arguments(ARG "" "OUTPUT_FILE" "INPUT_FILES;DEPENDS" ${ARGN})

    if (NOT ARG_INPUT_FILES OR NOT ARG_OUTPUT_FILE)
        message(FATAL_ERROR "Both INPUT_FILES and OUTPUT_FILE arguments must be specified")
    endif()

    if (NOT ARG_DEPENDS)
        set(ARG_DEPENDS ${ARG_INPUT_FILES})
    endif()

    # Prepare ota_image_tool.py argument list
    if(DEFINED APPVERSION)
        set(OTA_ARGS
            "--vendor-id"
            ${CONFIG_CHIP_DEVICE_VENDOR_ID}
            "--product-id"
            ${CONFIG_CHIP_DEVICE_PRODUCT_ID}
            "--version"
            ${APPVERSION}
            "--version-str"
            ${APP_VERSION_EXTENDED_STRING}
            "--digest-algorithm"
            "sha256"
        )
    else()
        set(OTA_ARGS
            "--vendor-id"
            ${CONFIG_CHIP_DEVICE_VENDOR_ID}
            "--product-id"
            ${CONFIG_CHIP_DEVICE_PRODUCT_ID}
            "--version"
            ${CONFIG_CHIP_DEVICE_SOFTWARE_VERSION}
            "--version-str"
            ${CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING}
            "--digest-algorithm"
            "sha256"
        )
    endif()

    separate_arguments(OTA_EXTRA_ARGS NATIVE_COMMAND "${CONFIG_CHIP_OTA_IMAGE_EXTRA_ARGS}")

    list(APPEND OTA_ARGS ${OTA_EXTRA_ARGS})
    list(APPEND OTA_ARGS ${ARG_INPUT_FILES})
    list(APPEND OTA_ARGS ${ARG_OUTPUT_FILE})

    # Convert the argument list to multi-line string
    string(REPLACE ";" "\n" OTA_ARGS "${OTA_ARGS}")

    # Pass the argument list via file to avoid hitting Windows command-line length limit
    file(GENERATE
        OUTPUT ${ARG_OUTPUT_FILE}.args
        CONTENT ${OTA_ARGS}
    )

    add_custom_command(OUTPUT ${ARG_OUTPUT_FILE}
        COMMAND ${Python3_EXECUTABLE} ${CHIP_ROOT}/src/app/ota_image_tool.py create @${ARG_OUTPUT_FILE}.args
        DEPENDS ${ARG_DEPENDS} ${CHIP_ROOT}/src/app/ota_image_tool.py
    )

    add_custom_target(${TARGET_NAME} ALL
        DEPENDS ${ARG_OUTPUT_FILE}
    )
endfunction()

#
# Create CMake target for building Matter OTA image for the configured OTA backend.
#
# Required arguments:
#   OUTPUT_FILE file               Location of generated Matter OTA image
#
# Optional arguments:
#   INPUT_FILES file1[, file2...]  Application images, used by backends that ship them
#                                  as is (e.g. MCUboot); required for those backends
#
# A custom OTA backend (CONFIG_CHIP_OTA_BACKEND_CUSTOM) provides its own image through
# the CHIP_OTA_IMAGE_INPUT_FILES and CHIP_OTA_IMAGE_DEPENDS global properties.
#
function(chip_zephyr_ota_image TARGET_NAME)
    cmake_parse_arguments(ARG "" "OUTPUT_FILE" "INPUT_FILES" ${ARGN})

    get_property(_backend_inputs GLOBAL PROPERTY CHIP_OTA_IMAGE_INPUT_FILES)
    get_property(_backend_depends GLOBAL PROPERTY CHIP_OTA_IMAGE_DEPENDS)
    if (_backend_inputs)
        set(ARG_INPUT_FILES ${_backend_inputs})
    endif()
    if (_backend_depends)
        set(_depends DEPENDS ${_backend_depends})
    endif()

    chip_ota_image(${TARGET_NAME}
        INPUT_FILES ${ARG_INPUT_FILES}
        OUTPUT_FILE ${ARG_OUTPUT_FILE}
        ${_depends}
    )
endfunction()
