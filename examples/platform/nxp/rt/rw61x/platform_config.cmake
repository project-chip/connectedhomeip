
#
#   Copyright (c) 2024 Project CHIP Authors
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
#   @file
#     CMake file that defines cmake variables specific to this platform
#

# Board configs
set(CONFIG_CORE_FOLDER "")

if (CONFIG_BOARD_VARIANT STREQUAL "frdm")
    set(CONFIG_BOARD_NAME "frdmrw612")
else()
    set(CONFIG_BOARD_NAME "rdrw612bga")
endif()

# MCUBoot OTA configs
set(CONFIG_MCUBOOT_BUILD_TYPE "flash_release")
set(CONFIG_CHIP_MCUBOOT_SLOT_SIZE 0x440000)
set(CONFIG_CHIP_MCUBOOT_MAX_SECTORS 1088)
set(CONFIG_CHIP_MCUBOOT_HEADER_SIZE 0x1000)

# CHIP configs
set(CONFIG_CHIP_DEVICE_VENDOR_ID 4151)
set(CONFIG_CHIP_DEVICE_PRODUCT_ID 41510)

set(CONFIG_CHIP_DEVICE_VENDOR_NAME "NXP Semiconductors")
if (NOT CONFIG_CHIP_DEVICE_PRODUCT_NAME)
    set(CONFIG_CHIP_DEVICE_PRODUCT_NAME "Thermostat")
endif()
if (NOT CONFIG_CHIP_DEVICE_TYPE)
    set(CONFIG_CHIP_DEVICE_TYPE 769)
endif()
set(CONFIG_CHIP_DEVICE_SPAKE2_IT 10000)
set(CONFIG_CHIP_DEVICE_SPAKE2_SALT "UXKLzwHdN3DZZLBaL2iVGhQi/OoQwIwJRQV4rpEalbA=")
if (NOT CONFIG_CHIP_DEVICE_DISCRIMINATOR)
    set(CONFIG_CHIP_DEVICE_DISCRIMINATOR 2560)
endif()
if (NOT CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE)
    set(CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE 14014)
endif()
set(CONFIG_CHIP_DEVICE_HARDWARE_VERSION 1)
set(CONFIG_CHIP_DEVICE_HARDWARE_VERSION_STRING "1.0")
set(CONFIG_CHIP_DEVICE_SERIAL_NUMBER "12345678")
set(CONFIG_CHIP_DEVICE_MANUFACTURING_DATE "2024-01-01")
set(CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID "00112233445566778899aabbccddeeff")
set(CONFIG_CHIP_DEVICE_PRODUCT_FINISH "Matte")
set(CONFIG_CHIP_DEVICE_PRODUCT_COLOR "Green")
set(CONFIG_CHIP_DEVICE_PRODUCT_URL "https://www.nxp.com/products/wireless/wi-fi-plus-bluetooth-plus-802-15-4/wireless-mcu-with-integrated-tri-radiobr1x1-wi-fi-6-plus-bluetooth-low-energy-5-3-802-15-4:RW612")
set(CONFIG_CHIP_DEVICE_PRODUCT_LABEL "RW612")
set(CONFIG_CHIP_DEVICE_PART_NUMBER "RW612")
set(CONFIG_CHIP_ENCRYPTED_FACTORY_DATA_AES128_KEY "2B7E151628AED2A6ABF7158809CF4F3C")
set(CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED true)
set(CONFIG_CHIP_FACTORY_DATA_GENERATE_SPAKE2_VERIFIER true)
set(CONFIG_CHIP_ENCRYPTED_FACTORY_DATA true)

# build configs
set(CONFIG_CHIP_NVM_COMPONENT "nvs")
set(CONFIG_CHIP_RT_PLATFORM "rw61x")
set(CONFIG_CHIP_NXP_PLATFORM "rt/${CONFIG_CHIP_RT_PLATFORM}")
set(CONFIG_CHIP_FACTORY_DATA 1)
set(CONFIG_CHIP_SECURE_DAC_PRIVATE_KEY_STORAGE false)
set(CONFIG_CHIP_ENABLE_SECURE_WHOLE_FACTORY_DATA false)
set(CONFIG_CHIP_BUILD_APP_WITH_GN true)
set(CONFIG_LOG true)
set(CONFIG_MATTER_LOG_LEVEL 4) # detail logs

# BLE is enabled by default
if (NOT DEFINED CONFIG_BT)
    set(CONFIG_BT true)
endif()

if (NOT DEFINED CONFIG_CHIP_IPV4)
    set(CONFIG_CHIP_IPV4 true)
endif()

if (NOT DEFINED CONFIG_CHIP_DEVICE_SOFTWARE_VERSION)
    set(CONFIG_CHIP_DEVICE_SOFTWARE_VERSION 1)
endif()

if (NOT DEFINED CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING)
    set(CONFIG_CHIP_DEVICE_SOFTWARE_VERSION_STRING "1.0")
endif()

if(NOT DEFINED CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART)
    set(CONFIG_CHIP_ENABLE_PAIRING_AUTOSTART true)
endif()


# define sections to remove from the binary when converting the elf to bin with objcopy
if (CONFIG_CHIP_OTA_REQUESTOR)
    set(CONFIG_REMOVE_SECTIONS_FROM_BIN -R .flash_config -R .NVM)
else()
    set(CONFIG_REMOVE_SECTIONS_FROM_BIN -R .NVM)
endif()