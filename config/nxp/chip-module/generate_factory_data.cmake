#
# Copyright (c) 2024 Project CHIP Authors
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

# Generate certificates based on factory data given via Kconfigs.
#
# This function creates a list of arguments for external script and then run it to generate certificates.
#
# During generation process a some file will be created in zephyr's build directory:
# - Chip-PAI-NXP-<VID>-<PID>-Cert.der
# - Chip-PAI-NXP-<VID>-<PID>-Key.der
# - Chip-DAC-NXP-<VID>-<PID>-Cert.der
# - Chip-DAC-NXP-<VID>-<PID>-Key.der
#
# [Args]:
# script_path - a path to script that generates the certificates.
# output_path - a path to output directory, where generated certificates will be stored.
function(nxp_generate_certs script_path output_path)
    if(NOT EXISTS ${CHIP_ROOT}/out/chip-cert)
        message(FATAL_ERROR "Couldn't find chip-cert in ${CHIP_ROOT}/out folder.
                             Run following command from ${CHIP_ROOT} to build it:
                             gn gen out
                             ninja -C out chip-cert")
    endif()

    # generate all script arguments
    set(script_args)
    string(APPEND script_args "--gen_cd\n")
    string(APPEND script_args "--cd_type \"1\"\n")
    string(APPEND script_args "--chip_cert_path ${CHIP_ROOT}/out/chip-cert\n")
    string(APPEND script_args "--vendor_id ${CONFIG_CHIP_DEVICE_VENDOR_ID}\n")
    string(APPEND script_args "--product_id ${CONFIG_CHIP_DEVICE_PRODUCT_ID}\n")
    string(APPEND script_args "--vendor_name \"${CONFIG_CHIP_DEVICE_VENDOR_NAME}\"\n")
    string(APPEND script_args "--product_name \"${CONFIG_CHIP_DEVICE_PRODUCT_NAME}\"\n")
    string(APPEND script_args "--device_type \"${CONFIG_CHIP_DEVICE_TYPE}\"\n")
    string(APPEND script_args "--output \"${output_path}\"\n")

    # convert decimal VID to its hexadecimal representation to find out certification files in repository
    math(EXPR LOCAL_VID "${CONFIG_CHIP_DEVICE_VENDOR_ID}" OUTPUT_FORMAT HEXADECIMAL)
    string(SUBSTRING ${LOCAL_VID} 2 -1 raw_vid)
    string(TOUPPER ${raw_vid} raw_vid_upper)

    # convert decimal PID to its hexadecimal representation to find out certification files in repository
    math(EXPR LOCAL_PID "${CONFIG_CHIP_DEVICE_PRODUCT_ID}" OUTPUT_FORMAT HEXADECIMAL)
    string(SUBSTRING ${LOCAL_PID} 2 -1 raw_pid)
    string(TOUPPER ${raw_pid} raw_pid_upper)

    # execute the script which can generate the certificates
    separate_arguments(separated_script_args NATIVE_COMMAND ${script_args})
    add_custom_command(
        OUTPUT
        ${output_path}/Chip-PAI-NXP-${raw_vid_upper}-${raw_pid_upper}-Cert.pem
        ${output_path}/Chip-PAI-NXP-${raw_vid_upper}-${raw_pid_upper}-Key.pem
        ${output_path}/Chip-DAC-NXP-${raw_vid_upper}-${raw_pid_upper}-Cert.pem
        ${output_path}/Chip-DAC-NXP-${raw_vid_upper}-${raw_pid_upper}-Key.pem
        ${output_path}/Chip-PAI-NXP-${raw_vid_upper}-${raw_pid_upper}-Cert.der
        ${output_path}/Chip-PAI-NXP-${raw_vid_upper}-${raw_pid_upper}-Key.der
        ${output_path}/Chip-DAC-NXP-${raw_vid_upper}-${raw_pid_upper}-Cert.der
        ${output_path}/Chip-DAC-NXP-${raw_vid_upper}-${raw_pid_upper}-Key.der
        DEPENDS ${script_path}
        COMMAND ${Python3_EXECUTABLE} ${script_path} ${separated_script_args}
        COMMENT "Generating new certificates..."
    )
    add_custom_target(chip-certificates ALL
        DEPENDS
        ${output_path}/Chip-PAI-NXP-${raw_vid_upper}-${raw_pid_upper}-Cert.pem
        ${output_path}/Chip-PAI-NXP-${raw_vid_upper}-${raw_pid_upper}-Key.pem
        ${output_path}/Chip-DAC-NXP-${raw_vid_upper}-${raw_pid_upper}-Cert.pem
        ${output_path}/Chip-DAC-NXP-${raw_vid_upper}-${raw_pid_upper}-Key.pem
        ${output_path}/Chip-PAI-NXP-${raw_vid_upper}-${raw_pid_upper}-Cert.der
        ${output_path}/Chip-PAI-NXP-${raw_vid_upper}-${raw_pid_upper}-Key.der
        ${output_path}/Chip-DAC-NXP-${raw_vid_upper}-${raw_pid_upper}-Cert.der
        ${output_path}/Chip-DAC-NXP-${raw_vid_upper}-${raw_pid_upper}-Key.der
    )
endfunction()

# Create a .bin file with factory data in KLV format.
#
# This function creates a .bin file from given Kconfig.
#
# During generation process, following files will be created in zephyr's build directory:
# - <factory_data_target>.bin a binary file containing all raw factory data in KLV format.
#
# [Args]:
# script_path - a path to script that makes a factory data .bin file from given arguments.
# output_path - a path to output directory, where created bin file will be stored.
function(nxp_generate_factory_data_bin script_path output_path)
    if(NOT EXISTS ${CHIP_ROOT}/out/spake2p)
        message(FATAL_ERROR "Couldn't find spake2p in ${CHIP_ROOT}/out folder.
                             Run following command from ${CHIP_ROOT} to build it:
                             gn gen out
                             ninja -C out spake2p")
    endif()

    # convert decimal VID to its hexadecimal representation to find out certification files in repository
    math(EXPR LOCAL_VID "${CONFIG_CHIP_DEVICE_VENDOR_ID}" OUTPUT_FORMAT HEXADECIMAL)
    string(SUBSTRING ${LOCAL_VID} 2 -1 raw_vid)
    string(TOUPPER ${raw_vid} raw_vid_upper)

    # convert decimal PID to its hexadecimal representation to find out certification files in repository
    math(EXPR LOCAL_PID "${CONFIG_CHIP_DEVICE_PRODUCT_ID}" OUTPUT_FORMAT HEXADECIMAL)
    string(SUBSTRING ${LOCAL_PID} 2 -1 raw_pid)
    string(TOUPPER ${raw_pid} raw_pid_upper)

    # generate all script arguments
    set(script_args)
    string(APPEND script_args "--it \"${CONFIG_CHIP_DEVICE_SPAKE2_IT}\"\n")
    string(APPEND script_args "--salt \"${CONFIG_CHIP_DEVICE_SPAKE2_SALT}\"\n")
    string(APPEND script_args "--discriminator ${CONFIG_CHIP_DEVICE_DISCRIMINATOR}\n")
    string(APPEND script_args "--passcode ${CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE}\n")
    string(APPEND script_args "--vid ${CONFIG_CHIP_DEVICE_VENDOR_ID}\n")
    string(APPEND script_args "--pid ${CONFIG_CHIP_DEVICE_PRODUCT_ID}\n")
    string(APPEND script_args "--vendor_name \"${CONFIG_CHIP_DEVICE_VENDOR_NAME}\"\n")
    string(APPEND script_args "--product_name \"${CONFIG_CHIP_DEVICE_PRODUCT_NAME}\"\n")
    string(APPEND script_args "--hw_version ${CONFIG_CHIP_DEVICE_HARDWARE_VERSION}\n")
    string(APPEND script_args "--hw_version_str \"${CONFIG_CHIP_DEVICE_HARDWARE_VERSION_STRING}\"\n")
    string(APPEND script_args "--spake2p_path \"${CHIP_ROOT}/out/spake2p\"\n")
    string(APPEND script_args "--serial_num \"${CONFIG_CHIP_DEVICE_SERIAL_NUMBER}\"\n")
    string(APPEND script_args "--date \"${CONFIG_CHIP_DEVICE_MANUFACTURING_DATE}\"\n")
    string(APPEND script_args "--unique_id \"${CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID}\"\n")
    string(APPEND script_args "--product_finish ${CONFIG_CHIP_DEVICE_PRODUCT_FINISH}\n")
    string(APPEND script_args "--product_url ${CONFIG_CHIP_DEVICE_PRODUCT_URL}\n")
    string(APPEND script_args "--product_label ${CONFIG_CHIP_DEVICE_PRODUCT_LABEL}\n")
    string(APPEND script_args "--part_number ${CONFIG_CHIP_DEVICE_PART_NUMBER}\n")
    string(APPEND script_args "--out \"${output_path}/factory_data.bin\"\n")

    # for development purpose user can use default certs instead of generating or providing them
    if(CONFIG_CHIP_FACTORY_DATA_USE_DEFAULT_CERTS)
        # all certs are located in ${CHIP_ROOT}/credentials/development/attestation
        # it can be used during development without need to generate new certifications
        set(cd_cert ${CHIP_ROOT}/credentials/development/cd-certs/Chip-Test-CD-Cert.der)
        set(dac_cert ${CHIP_ROOT}/credentials/development/attestation/Matter-Development-DAC-${raw_vid_upper}-${raw_pid_upper}-Cert.der)
        set(dac_key ${CHIP_ROOT}/credentials/development/attestation/Matter-Development-DAC-${raw_vid_upper}-${raw_pid_upper}-Key.der)
        set(pai_cert ${CHIP_ROOT}/credentials/development/attestation/Matter-Development-PAI-${raw_vid_upper}-noPID-Cert.der)
    elseif(CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_USER)
        set(cd_cert ${CONFIG_CHIP_FACTORY_DATA_USER_CERTS_CD_CERT})
        set(dac_cert ${CONFIG_CHIP_FACTORY_DATA_USER_CERTS_DAC_CERT})
        set(dac_key ${CONFIG_CHIP_FACTORY_DATA_USER_CERTS_DAC_KEY})
        set(pai_cert ${CONFIG_CHIP_FACTORY_DATA_USER_CERTS_PAI_CERT})
    elseif(CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED)
        set(cd_cert ${output_path}/Chip-Test-CD-${raw_vid_upper}-${raw_pid_upper}.der)
        set(dac_cert ${output_path}/Chip-DAC-NXP-${raw_vid_upper}-${raw_pid_upper}-Cert.der)
        set(dac_key ${output_path}/Chip-DAC-NXP-${raw_vid_upper}-${raw_pid_upper}-Key.der)
        set(pai_cert ${output_path}/Chip-PAI-NXP-${raw_vid_upper}-${raw_pid_upper}-Cert.der)
    else()
        message(FATAL_ERROR "Either CONFIG_CHIP_FACTORY_DATA_USE_DEFAULT_CERTS or CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_USER
                             or CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED must be defined.
                             See ${CHIP_ROOT}/config/nxp/chip-module/Kconfig for details.")
    endif()

    string(APPEND script_args "--cert_declaration \"${cd_cert}\"\n")
    string(APPEND script_args "--dac_cert \"${dac_cert}\"\n")
    string(APPEND script_args "--dac_key \"${dac_key}\"\n")
    string(APPEND script_args "--pai_cert \"${pai_cert}\"\n")

    # check if spake2 verifier should be generated using script
    if(NOT CONFIG_CHIP_FACTORY_DATA_GENERATE_SPAKE2_VERIFIER)
        # Spake2 verifier should be provided using Kconfig
        string(APPEND script_args "--spake2p_verifier \"${CONFIG_CHIP_DEVICE_SPAKE2_TEST_VERIFIER}\"\n")
    endif()

    if(CONFIG_CHIP_ENCRYPTED_FACTORY_DATA)
        if(NOT CONFIG_CHIP_ENCRYPTED_FACTORY_DATA_AES128_KEY)
            message(FATAL_ERROR "CONFIG_CHIP_ENCRYPTED_FACTORY_DATA_AES128_KEY is required when using CONFIG_CHIP_ENCRYPTED_FACTORY_DATA !")
        endif()

        string(APPEND script_args "--aes128_key \"${CONFIG_CHIP_ENCRYPTED_FACTORY_DATA_AES128_KEY}\"\n")
    endif()

    if(CONFIG_CHIP_DEVICE_PRODUCT_COLOR)
        string(APPEND script_args "--product_primary_color ${CONFIG_CHIP_DEVICE_PRODUCT_COLOR}\n")
    endif()

    # execute the script which can generate the factory_data.bin file
    separate_arguments(separated_script_args NATIVE_COMMAND ${script_args})
    add_custom_command(
        OUTPUT ${output_path}/factory_data.bin
        DEPENDS ${script_path}
        COMMAND ${Python3_EXECUTABLE} ${script_path} ${separated_script_args}
        COMMENT "Generating new Factory Data..."
    )
    add_custom_target(factory_data ALL
        DEPENDS ${output_path}/factory_data.bin
    )

    if(CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED)
        # Make sure certificates are generated before trying to generate factory data
        add_dependencies(factory_data chip-certificates)
    endif()
endfunction()

# Generate factory data partition using given args
#
# During generation process a some file will be created in zephyr's build directory:
# - factory_data.bin: a raw binary file containing the factory data partition
# - CD, PAI, DEC certificates if CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED is set
#
function(nxp_generate_factory_data)
    find_package(Python REQUIRED)

    # CHIP_ROOT must be provided as a reference set all localization of scripts
    if(NOT CHIP_ROOT)
        message(FATAL_ERROR "CHIP_ROOT variable is not set, please add it to CMakeLists.txt file")
    endif()

    # Localize all scripts needed to generate factory data partition
    set(GENERATE_CERTS_SCRIPT_PATH ${CHIP_ROOT}/scripts/tools/nxp/generate_certs.py)
    set(GENERATE_FDATA_SCRIPT_PATH ${CHIP_ROOT}/scripts/tools/nxp/factory_data_generator/generate.py)
    set(OUTPUT_FILE_PATH ${APPLICATION_BINARY_DIR}/zephyr)

    if(CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED)
        # Generate certificates
        nxp_generate_certs(${GENERATE_CERTS_SCRIPT_PATH} ${OUTPUT_FILE_PATH})
    endif()

    nxp_generate_factory_data_bin(${GENERATE_FDATA_SCRIPT_PATH} ${OUTPUT_FILE_PATH})
endfunction()
