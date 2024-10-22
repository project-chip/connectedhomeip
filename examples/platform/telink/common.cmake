#
#    Copyright (c) 2024 Project CHIP Authors
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

string(REPLACE "_retention" "" BASE_BOARD ${BOARD})
string(REGEX REPLACE "_v[0-9]+" "" BASE_BOARD ${BASE_BOARD})

if(NOT FLASH_SIZE)
  if(${BASE_BOARD} MATCHES "tlsr9118bdk40d")
    set(FLASH_SIZE "4m")
  else()
    set(FLASH_SIZE "2m")
  endif()
endif()
message(STATUS "Flash memory size is set to: " ${FLASH_SIZE} "b")

if(${TLNK_MARS_BOARD} MATCHES y)
  set(MARS_BOOT_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BASE_BOARD}_mars_boot.overlay")
  if(NOT EXISTS "${MARS_BOOT_DTC_OVERLAY_FILE}")
    message(FATAL_ERROR "${MARS_BOOT_DTC_OVERLAY_FILE} doesn't exist")
  endif()
  set(MARS_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}_mars.overlay")
  if(NOT EXISTS "${MARS_DTC_OVERLAY_FILE}")
    message(FATAL_ERROR "${MARS_DTC_OVERLAY_FILE} doesn't exist")
  endif()
  set(MARS_CONF_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}_mars.conf")
  if(NOT EXISTS "${MARS_CONF_OVERLAY_FILE}")
    message(FATAL_ERROR "${MARS_CONF_OVERLAY_FILE} doesn't exist")
  endif()
else()
  unset(MARS_BOOT_DTC_OVERLAY_FILE)
  unset(MARS_DTC_OVERLAY_FILE)
  unset(MARS_CONF_OVERLAY_FILE)
endif()

if(${TLNK_USB_DONGLE} MATCHES y)
  set(USB_BOOT_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BASE_BOARD}_usb_boot.overlay")
  if(NOT EXISTS "${USB_BOOT_DTC_OVERLAY_FILE}")
    message(FATAL_ERROR "${USB_BOOT_DTC_OVERLAY_FILE} doesn't exist")
  endif()
  set(USB_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}_usb.overlay")
  if(NOT EXISTS "${USB_DTC_OVERLAY_FILE}")
    message(FATAL_ERROR "${USB_DTC_OVERLAY_FILE} doesn't exist")
  endif()
  set(USB_CONF_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}_usb.conf")
  if(NOT EXISTS "${USB_CONF_OVERLAY_FILE}")
    message(FATAL_ERROR "${USB_CONF_OVERLAY_FILE} doesn't exist")
  endif()
else()
  unset(USB_BOOT_DTC_OVERLAY_FILE)
  unset(USB_DTC_OVERLAY_FILE)
  unset(USB_CONF_OVERLAY_FILE)
endif()

set(BOOT_CONF_OVERLAY_FILE "${CHIP_ROOT}/config/telink/app/bootloader.conf")
if(NOT EXISTS "${BOOT_CONF_OVERLAY_FILE}")
  message(FATAL_ERROR "${BOOT_CONF_OVERLAY_FILE} doesn't exist")
endif()

if(${CONFIG_USB_TELINK_B9X} MATCHES y)
  set(BOOT_USB_CONF_OVERLAY_FILE "${CHIP_ROOT}/config/telink/app/bootloader_usb.conf")
  if(NOT EXISTS "${BOOT_USB_CONF_OVERLAY_FILE}")
    message(FATAL_ERROR "${BOOT_USB_CONF_OVERLAY_FILE} doesn't exist")
  endif()
else()
  unset(BOOT_USB_CONF_OVERLAY_FILE)
endif()

set(GLOBAL_BOOT_CONF_OVERLAY_FILE "${BOOT_CONF_OVERLAY_FILE} ${BOOT_USB_CONF_OVERLAY_FILE}")

set(LOCAL_DTC_OVERLAY_FILE "${CMAKE_CURRENT_SOURCE_DIR}/boards/${BASE_BOARD}.overlay")
if(NOT EXISTS "${LOCAL_DTC_OVERLAY_FILE}")
  message(STATUS "${LOCAL_DTC_OVERLAY_FILE} doesn't exist")
  unset(LOCAL_DTC_OVERLAY_FILE)
endif()

set(GLOBAL_BOOT_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BASE_BOARD}.overlay")
if(NOT EXISTS "${GLOBAL_BOOT_DTC_OVERLAY_FILE}")
  message(STATUS "${GLOBAL_BOOT_DTC_OVERLAY_FILE} doesn't exist")
  unset(GLOBAL_BOOT_DTC_OVERLAY_FILE)
endif()

set(GLOBAL_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BOARD}.overlay")
if(NOT EXISTS "${GLOBAL_DTC_OVERLAY_FILE}")
  message(STATUS "${GLOBAL_DTC_OVERLAY_FILE} doesn't exist")
  unset(GLOBAL_DTC_OVERLAY_FILE)
endif()

set(FLASH_DTC_OVERLAY_FILE "${CHIP_ROOT}/src/platform/telink/${BASE_BOARD}_${FLASH_SIZE}_flash.overlay")
if(NOT EXISTS "${FLASH_DTC_OVERLAY_FILE}")
  message(STATUS "${FLASH_DTC_OVERLAY_FILE} doesn't exist")
  unset(FLASH_DTC_OVERLAY_FILE)
endif()

if(DTC_OVERLAY_FILE)
  set(DTC_OVERLAY_FILE
    "${DTC_OVERLAY_FILE} ${GLOBAL_DTC_OVERLAY_FILE} ${USB_DTC_OVERLAY_FILE} ${MARS_DTC_OVERLAY_FILE} ${FLASH_DTC_OVERLAY_FILE} ${LOCAL_DTC_OVERLAY_FILE}"
    CACHE STRING "" FORCE
  )
else()
  set(DTC_OVERLAY_FILE ${GLOBAL_DTC_OVERLAY_FILE} ${USB_DTC_OVERLAY_FILE} ${MARS_DTC_OVERLAY_FILE} ${FLASH_DTC_OVERLAY_FILE} ${LOCAL_DTC_OVERLAY_FILE})
endif()

if(NOT CONF_FILE)
  set(CONF_FILE ${USB_CONF_OVERLAY_FILE} ${MARS_CONF_OVERLAY_FILE} prj.conf)
endif()

# Load NCS/Zephyr build system
list(APPEND ZEPHYR_EXTRA_MODULES ${CHIP_ROOT}/config/telink/chip-module)
find_package(Zephyr HINTS $ENV{ZEPHYR_BASE})
