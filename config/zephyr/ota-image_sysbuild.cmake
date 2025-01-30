#
#   Copyright (c) 2021 Project CHIP Authors
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

#
# Create CMake target for building Matter OTA (Over-the-air update) image.
#
# Required arguments:
#   INPUT_FILES file1[, file2...]  Binary files to be included in Matter OTA image
#   OUTPUT_FILE file               Location of generated Matter OTA image
#
function(chip_ota_image TARGET_NAME)
  cmake_parse_arguments(ARG "" "OUTPUT_FILE" "INPUT_FILES" ${ARGN})

  if(NOT ARG_INPUT_FILES OR NOT ARG_OUTPUT_FILE)
    message(FATAL_ERROR "Both INPUT_FILES and OUTPUT_FILE arguments must be specified")
  endif()

  sysbuild_get(CONFIG_CHIP_DEVICE_SOFTWARE_VERSION IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_SOFTWARE_VERSION KCONFIG)
  sysbuild_get(CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING KCONFIG)
  sysbuild_get(CONFIG_CHIP_DEVICE_VENDOR_ID IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_VENDOR_ID KCONFIG)
  sysbuild_get(CONFIG_CHIP_DEVICE_PRODUCT_ID IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_PRODUCT_ID KCONFIG)

  # Prepare ota_image_tool.py argument list
  if(EXISTS ${APP_DIR}/VERSION)
    file(READ ${APP_DIR}/VERSION ver)
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${APP_DIR}/VERSION)
    string(REGEX MATCH "VERSION_MAJOR = ([0-9]*)" _ ${ver})
    set(app_version_major ${CMAKE_MATCH_1})
    string(REGEX MATCH "VERSION_MINOR = ([0-9]*)" _ ${ver})
    set(app_version_minor ${CMAKE_MATCH_1})
    string(REGEX MATCH "PATCHLEVEL = ([0-9]*)" _ ${ver})
    set(app_version_patchlevel ${CMAKE_MATCH_1})
    string(REGEX MATCH "VERSION_TWEAK = ([0-9]*)" _ ${ver})
    set(app_version_tweak ${CMAKE_MATCH_1})
    string(REGEX MATCH "EXTRAVERSION = ([a-z0-9]*)" _ ${ver})
    set(app_version_extra ${CMAKE_MATCH_1})

    if(app_version_extra)
      set(APP_VERSION_STRING "${app_version_major}.${app_version_minor}.${app_version_patchlevel}-${app_version_extra}+${app_version_tweak}")
    else()
      set(APP_VERSION_STRING "${app_version_major}.${app_version_minor}.${app_version_patchlevel}+${app_version_tweak}")
    endif()
    math(EXPR APPVERSION "(${app_version_major} << 24) | (${app_version_minor} << 16) | (${app_version_patchlevel} << 8) | ${app_version_tweak}" OUTPUT_FORMAT HEXADECIMAL)

    set(OTA_ARGS
        "--vendor-id"
        ${CONFIG_CHIP_DEVICE_VENDOR_ID}
        "--product-id"
        ${CONFIG_CHIP_DEVICE_PRODUCT_ID}
        "--version"
        ${APPVERSION}
        "--version-str"
        ${APP_VERSION_STRING}
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

  separate_arguments(OTA_EXTRA_ARGS NATIVE_COMMAND "${CHIP_OTA_IMAGE_EXTRA_ARGS}")

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
                    COMMAND ${Python3_EXECUTABLE} ${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/src/app/ota_image_tool.py create @${ARG_OUTPUT_FILE}.args
                    DEPENDS ${ARG_INPUT_FILES} ${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR}/src/app/ota_image_tool.py
  )

  add_custom_target(${TARGET_NAME} ALL
                    DEPENDS ${ARG_OUTPUT_FILE}
  )
endfunction()
