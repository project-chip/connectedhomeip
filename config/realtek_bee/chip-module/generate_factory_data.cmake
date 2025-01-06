#
#   Copyright (c) 2022 Project CHIP Authors
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


function(realtek_generate_factory_data_bin script_path output_path)
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

# python3 factory_data.py -p 20202021 -d 3840 --spake2p_path=../test/spake2p 
# --vendor-id 0xFFF1 --vendor-name "TEST_VENDOR" --product-id 0x0123 --product-name "TEST_PRODUCT" --serial-num "TEST_SN" 
# --hw-ver 1 --hw-ver-str "1.0" --rd-id-uid 00112233445566778899aabbccddeeff --dac_cert ../test/Chip-DAC-Cert.der --dac_key 
# ../test/Chip-DAC-Key.der --pai_cert ../test/Chip-PAI-Cert.der --cd ../test/CD.der


    # generate all script arguments
    set(script_args)
    # string(APPEND script_args "--it \"${CONFIG_CHIP_DEVICE_SPAKE2_IT}\"\n")
    # string(APPEND script_args "--salt \"${CONFIG_CHIP_DEVICE_SPAKE2_SALT}\"\n")
    string(APPEND script_args "--discriminator ${CONFIG_CHIP_DEVICE_DISCRIMINATOR}\n")
    string(APPEND script_args "--passcode ${CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE}\n")
    string(APPEND script_args "--vendor-id ${CONFIG_CHIP_DEVICE_VENDOR_ID}\n")
    string(APPEND script_args "--product-id ${CONFIG_CHIP_DEVICE_PRODUCT_ID}\n")
    string(APPEND script_args "--vendor-name \"${CONFIG_CHIP_DEVICE_VENDOR_NAME}\"\n")
    string(APPEND script_args "--product-name \"${CONFIG_CHIP_DEVICE_PRODUCT_NAME}\"\n")
    string(APPEND script_args "--hw-ver ${CONFIG_CHIP_DEVICE_HARDWARE_VERSION}\n")
    string(APPEND script_args "--hw-ver-str \"${CONFIG_CHIP_DEVICE_HARDWARE_VERSION_STRING}\"\n")
    string(APPEND script_args "--spake2p_path \"${CHIP_ROOT}/out/spake2p\"\n")
    string(APPEND script_args "--serial-num \"${CONFIG_CHIP_DEVICE_SERIAL_NUMBER}\"\n")
    # string(APPEND script_args "--date \"${CONFIG_CHIP_DEVICE_MANUFACTURING_DATE}\"\n")
    string(APPEND script_args "--rd-id-uid \"${CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID}\"\n")
    # string(APPEND script_args "--product_finish ${CONFIG_CHIP_DEVICE_PRODUCT_FINISH}\n")
    # string(APPEND script_args "--product_url ${CONFIG_CHIP_DEVICE_PRODUCT_URL}\n")
    # string(APPEND script_args "--product_label ${CONFIG_CHIP_DEVICE_PRODUCT_LABEL}\n")
    # string(APPEND script_args "--part_number ${CONFIG_CHIP_DEVICE_PART_NUMBER}\n")
    string(APPEND script_args "--out \"${output_path}\"\n")

    set(cd_cert ${output_path}/Chip-Test-CD-${raw_vid_upper}-${raw_pid_upper}.der)
    set(dac_cert ${output_path}/Chip-DAC-RTK-${raw_vid_upper}-${raw_pid_upper}-Cert.der)
    set(dac_key ${output_path}/Chip-DAC-RTK-${raw_vid_upper}-${raw_pid_upper}-Key.der)
    set(pai_cert ${output_path}/Chip-PAI-RTK-${raw_vid_upper}-${raw_pid_upper}-Cert.der)

    string(APPEND script_args "--cd \"${cd_cert}\"\n")
    string(APPEND script_args "--dac_cert \"${dac_cert}\"\n")
    string(APPEND script_args "--dac_key \"${dac_key}\"\n")
    string(APPEND script_args "--pai_cert \"${pai_cert}\"\n")

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


function(realtek_generate_certs script_path output_path)
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

    message("separated_script_args: ${separated_script_args} \r\n raw_vid_upper ${raw_vid_upper} raw_pid_upper ${raw_pid_upper}")
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

function(realtek_generate_factory_data)
    find_package(Python REQUIRED)

    # CHIP_ROOT must be provided as a reference set all localization of scripts
    if(NOT CHIP_ROOT)
        message(FATAL_ERROR "CHIP_ROOT variable is not set, please add it to CMakeLists.txt file")
    endif()

    # Localize all scripts needed to generate factory data partition
    set(GENERATE_CERTS_SCRIPT_PATH ${CHIP_ROOT}/scripts/tools/realtek/generate_certs.py)
    message("APPLICATION_BINARY_DIR: ${APPLICATION_BINARY_DIR}")
    set(OUTPUT_FILE_PATH ${APPLICATION_BINARY_DIR}/zephyr)
    set(GENERATE_FDATA_SCRIPT_PATH ${CHIP_ROOT}/scripts/tools/realtek/factory_data.py)

    # Generate certificates
    realtek_generate_certs(${GENERATE_CERTS_SCRIPT_PATH} ${OUTPUT_FILE_PATH})

    realtek_generate_factory_data_bin(${GENERATE_FDATA_SCRIPT_PATH} ${OUTPUT_FILE_PATH})
endfunction()