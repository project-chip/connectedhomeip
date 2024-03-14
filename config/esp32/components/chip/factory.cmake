#
#   Copyright (c) 2023 Project CHIP Authors
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

include(${CMAKE_CURRENT_LIST_DIR}/factory_wrapper.cmake)

function(generate_build_time_partition fctry_partition esp_secure_cert_partition chip_root)
    set(options FLASH_IN_PROJECT)
    set(multi DEPENDS)
    cmake_parse_arguments(arg "${options}" "" "${multi}" "${ARGN}")
    get_filename_component(chip_root_abs_path ${chip_root} ABSOLUTE)

    set(generate_esp32_chip_factory_bin.py ${PYTHON} ${chip_root}/scripts/tools/generate_esp32_chip_factory_bin.py)

    partition_table_get_partition_info(fctry_partition_size "--partition-name ${fctry_partition}" "size")
    partition_table_get_partition_info(fctry_partition_offset "--partition-name ${fctry_partition}" "offset")

    partition_table_get_partition_info(secure_cert_partition_size "--partition-name ${esp_secure_cert_partition}" "size")
    partition_table_get_partition_info(secure_cert_partition_offset "--partition-name ${esp_secure_cert_partition}" "offset")

    message(STATUS "fctry_partition_size : ${fctry_partition_size}")
    message(STATUS "fctry_partition_offset : ${fctry_partition_offset}")
    message(STATUS "secure_cert_partition_size : ${secure_cert_partition_size}")
    message(STATUS "secure_cert_partition_offset : ${secure_cert_partition_offset}")

    if("${fctry_partition_size}" AND "${fctry_partition_offset}")

        set_values()

        set(PREVIOUS_VALUES_FILE "${CMAKE_BINARY_DIR}/previous_values.txt")

        if (NOT EXISTS ${PREVIOUS_VALUES_FILE})
            file(WRITE ${PREVIOUS_VALUES_FILE} "")
        endif()

        file(READ ${PREVIOUS_VALUES_FILE} PREVIOUS_VALUES)

        set(CURRENT_VALUES_STRING
        "${DEVICE_NAME}${VENDOR_NAME}${DISCRIMINATOR}${PASSCODE}${VENDOR_ID}${PRODUCT_ID}${HARDWARE_VERSION}${HARDWARE_VERSION_STR}${DAC_CERT}${DAC_KEY}${PAI_CERT}${CERT_DCLRN}")

        message(STATUS "Bulb Name: ${DEVICE_NAME}")
        message(STATUS "Vendor Name: ${VENDOR_NAME}")
        message(STATUS "Hardware Version: ${HARDWARE_VERSION}")
        message(STATUS "Hardware Version String: ${HARDWARE_VERSION_STR}")
        message(STATUS "Vendor ID: ${VENDOR_ID}")
        message(STATUS "Product ID: ${PRODUCT_ID}")
        message(STATUS "DAC Cert: ${DAC_CERT}")
        message(STATUS "DAC Key: ${DAC_KEY}")
        message(STATUS "PAI Cert: ${PAI_CERT}")
        message(STATUS "Certification Declaration: ${CERT_DCLRN}")
        message(STATUS "Passcode: ${PASSCODE}")
        message(STATUS "Discriminator: ${DISCRIMINATOR}")

        if (NOT "${CURRENT_VALUES_STRING}" STREQUAL "${PREVIOUS_VALUES}")
            message(STATUS "Values have changed. Triggering add_custom_target.")
            add_custom_target(build_time_partition ALL
            COMMAND ${generate_esp32_chip_factory_bin.py} -d ${DISCRIMINATOR}
                -p ${PASSCODE}
                --product-name "${DEVICE_NAME}"
                --vendor-name "${VENDOR_NAME}"
                --vendor-id ${VENDOR_ID}
                --product-id ${PRODUCT_ID}
                --hw-ver ${HARDWARE_VERSION}
                --hw-ver-str "${HARDWARE_VERSION_STR}"
                --dac-cert ${DAC_CERT}
                --dac-key ${DAC_KEY}
                --pai-cert ${PAI_CERT}
                --cd ${CERT_DCLRN}
            --dac-in-secure-cert
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
            file(WRITE ${PREVIOUS_VALUES_FILE} ${CURRENT_VALUES_STRING})
        else()
            add_custom_target(build_time_partition)
            message(STATUS "Values have not changed. Skipping add_custom_target.")
        endif()

        set(factory_partition_bin ${CMAKE_BINARY_DIR}/bin/factory_partition.bin)
        set(esp_secure_cert_partition_bin ${CMAKE_BINARY_DIR}/bin/esp_secure_cert_partititon.bin)
        idf_component_get_property(main_args esptool_py FLASH_ARGS)
        idf_component_get_property(sub_args esptool_py FLASH_SUB_ARGS)

        esptool_py_flash_target(${fctry_partition}-flash "${main_args}" "${sub_args}" ALWAYS_PLAINTEXT)
        esptool_py_flash_to_partition(${fctry_partition}-flash "${fctry_partition}" "${factory_partition_bin}")

        esptool_py_flash_target(${esp_secure_cert_partition}-flash "${main_args}" "${sub_args}" ALWAYS_PLAINTEXT)
        esptool_py_flash_to_partition(${esp_secure_cert_partition}-flash "${esp_secure_cert_partition}" "${esp_secure_cert_partition_bin}")

        add_dependencies(${fctry_partition}-flash build_time_partition)
        add_dependencies(${esp_secure_cert_partition}-flash build_time_partition)

        if(arg_FLASH_IN_PROJECT)
             esptool_py_flash_to_partition(flash "${fctry_partition}" "${factory_partition_bin}")
             esptool_py_flash_to_partition(flash "${esp_secure_cert_partition}" "${esp_secure_cert_partition_bin}")
             add_dependencies(flash build_time_partition)
        endif()
    else()
        set(message "Failed to create Factory partition image for partition '${partition}'. "
                "Check project configuration if using the correct partition table file.")
        fail_at_build_time(factory_${partition}_bin "${message}")

    endif()
endfunction()
