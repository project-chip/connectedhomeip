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

function(set_default_value VAR DEFAULT_VALUE)
    get_property(VAR_CACHE_TYPE CACHE ${VAR} PROPERTY TYPE)
    message(status "var cache type : ${VAR} ${VAR_CACHE_TYPE}")
    if (VAR_CACHE_TYPE STREQUAL "UNINITIALIZED")
	set(${VAR}_EXPLICITLY_SET TRUE CACHE BOOL "${VAR} is not explicitly set")
        set(${VAR} ${DEFAULT_VALUE} CACHE STRING ${VAR})
    else()
	set(${VAR}_EXPLICITLY_SET FALSE CACHE BOOL "${VAR} is explicitly set.")
        set(${VAR} ${DEFAULT_VALUE} CACHE STRING ${VAR})
    endif()
endfunction()

function(set_values)
    # Set variables with default values if not defined
    set_default_value(DEVICE_NAME "My bulb")
    set_default_value(VENDOR_NAME "Test-vendor")
    set_default_value(DISCRIMINATOR 3841)
    set_default_value(PASSCODE 20202020)
    set_default_value(VENDOR_ID 0xFFF2)
    set_default_value(PRODUCT_ID 0x8001)
    set_default_value(HARDWARE_VERSION 1)
    set_default_value(DISCOVERY_MODE 2)
    set_default_value(HARDWARE_VERSION_STR "Devkit")
    set_default_value(DAC_CERT "${CHIP_ROOT}/credentials/test/attestation/Chip-Test-DAC-FFF2-8001-0008-Cert.der")
    set_default_value(DAC_KEY  "${CHIP_ROOT}/credentials/test/attestation/Chip-Test-DAC-FFF2-8001-0008-Key.der")
    set_default_value(PAI_CERT  "${CHIP_ROOT}/credentials/test/attestation/Chip-Test-PAI-FFF2-8001-Cert.der")
    set_default_value(CERT_DCLRN "${CHIP_ROOT}/credentials/test/certification-declaration/Chip-Test-CD-FFF2-8001.der")
endfunction()

function(generate_build_time_partition fctry_partition esp_secure_cert_partition chip_root)
    set(options FLASH_IN_PROJECT)
    set(multi DEPENDS)
    cmake_parse_arguments(arg "${options}" "" "${multi}" "${ARGN}")
    get_filename_component(chip_root_abs_path ${chip_root} ABSOLUTE)

    set(generate_esp32_chip_factory_bin.py ${PYTHON} ${chip_root}/scripts/tools/generate_esp32_chip_factory_bin.py)
    set(gen_att_certs.py ${PYTHON} ${chip_root}/scripts/tools/gen_att_certs.py)

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
        message(STATUS "Vendor id set: ${VENDOR_ID_EXPLICITLY_SET}")
        message(STATUS "Product id set: ${PRODUCT_ID_EXPLICITLY_SET}")

        if ("${VENDOR_ID_EXPLICITLY_SET}" AND "${PRODUCT_ID_EXPLICITLY_SET}")
            string(RANDOM LENGTH 8 ALPHABET 0123456789 OUTPUT_VARIABLE RANDOM_PASSCODE)
            set(PASSCODE ${RANDOM_PASSCODE})
            message(STATUS "random passcode : ${RANDOM_PASSCODE}")

            math(EXPR PASSCODE_MOD "${RANDOM_PASSCODE} % 999999998")
            message(STATUS "Random passcode Mod:  ${PASSCODE_MOD}")
            set(PASSCODE ${PASSCODE_MOD})

            string(RANDOM LENGTH 4 ALPHABET 0123456789 OUTPUT_VARIABLE RANDOM_DISCRIMINATOR)
            set(DISCRIMINATOR ${RANDOM_DISCRIMINATOR})
            message(STATUS "random discriminator : ${RANDOM_DISCRIMINATOR}")

            math(EXPR DISCRIMINATOR_MOD "${RANDOM_DISCRIMINATOR} % 4096")
            message(STATUS "Random discriminator Mod: ${DISCRIMINATOR_MOD}")
            set(DISCRIMINATOR ${DISCRIMINATOR_MOD})

	    math(EXPR VENDOR_DEC ${VENDOR_ID} OUTPUT_FORMAT DECIMAL)
	    math(EXPR PRODUCT_DEC ${PRODUCT_ID} OUTPUT_FORMAT DECIMAL)

	    message(STATUS "Vendor Decimal: ${VENDOR_DEC}")
	    message(STATUS "Product Decimal: ${PRODUCT_DEC}")

	    set(OUTDIR attestation_${VENDOR_DEC}_${PRODUCT_DEC})
	    message(STATUS "Outdir: ${OUTDIR}")

	    execute_process(COMMAND ${gen_att_certs.py} --vendor-id ${VENDOR_ID} --product-id ${PRODUCT_ID}
		    WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

	    set(DAC_CERT ${CMAKE_BINARY_DIR}/certs/${OUTDIR}/DAC_cert.der)
            set(DAC_KEY ${CMAKE_BINARY_DIR}/certs/${OUTDIR}/DAC_key.der)
	    set(PAI_CERT ${CMAKE_BINARY_DIR}/certs/${OUTDIR}/PAI_cert.der)
	    set(CERT_DCLRN ${CMAKE_BINARY_DIR}/certs/${OUTDIR}/CD.der)
        endif()

        set(PREVIOUS_VALUES_FILE "${CMAKE_BINARY_DIR}/previous_values.txt")

        if (NOT EXISTS ${PREVIOUS_VALUES_FILE})
            file(WRITE ${PREVIOUS_VALUES_FILE} "")
        endif()

        file(READ ${PREVIOUS_VALUES_FILE} PREVIOUS_VALUES)

        set(CURRENT_VALUES_STRING
        "${DEVICE_NAME}${VENDOR_NAME}${DISCRIMINATOR}${PASSCODE}${VENDOR_ID}${PRODUCT_ID}${HARDWARE_VERSION}${HARDWARE_VERSION_STR}${DAC_CERT}${DAC_KEY}${PAI_CERT}${CERT_DCLRN}")

        message(STATUS "Vendor id set: ${VENDOR_ID_EXPLICITLY_SET}")
        message(STATUS "Bulb Name: ${DEVICE_NAME}")
        message(STATUS "Vendor Name: ${VENDOR_NAME}")
        message(STATUS "Hardware Version: ${HARDWARE_VERSION}")
        message(STATUS "Hardware Version String: ${HARDWARE_VERSION_STR}")
        message(STATUS "Vendor ID: ${VENDOR_ID}")
        message(STATUS "Product ID: ${PRODUCT_ID}")
	    message(STATUS "Discovery Mode : ${DISCOVERY_MODE}")
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
		--discovery-mode ${DISCOVERY_MODE}
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
