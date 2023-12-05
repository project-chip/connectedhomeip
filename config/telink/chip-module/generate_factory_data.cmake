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


# Create a binary file with factory data given via kConfigs.
#
# This function creates a list of arguments for external script and then run it to write a factory data file.
#
# This script can be manipulated using following kConfigs:
# - To merge generated factory data with final zephyr.hex file set kConfig CONFIG_CHIP_FACTORY_DATA_MERGE_WITH_FIRMWARE=y
# - To use default certification paths set CONFIG_CHIP_FACTORY_DATA_USE_DEFAULTS_CERTS_PATH=y 
# 
# During generation process a some file will be created in zephyr's build directory under factory subdirectory:
# - factory_data.bin
# - factory_data.hex
# - DAC_cert.der
# - DAC_cert.pem
# - DAC_key.pem
# - DAC_private_key.bin
# - DAC_public_key.bin
# - pai_cert.der
# - onb_codes.csv
# - pin_disc.csv
# - qrcode.png
# - summary.json
#
# [Args]:
#   factory_data_target - a name for target to generate factory_data.
#   script_path         - a path to script that makes a JSON factory data file from given arguments.
#   output_path         - a path to output directory, where created JSON file will be stored.
function(telink_create_factory_data_json factory_data_target script_path output_path)

# set script args for future purpose
set(script_args)
## generate all script arguments
string(APPEND script_args "--serial-num \"${CONFIG_CHIP_DEVICE_SERIAL_NUMBER}\"\n")
string(APPEND script_args "--mfg-date \"${CONFIG_CHIP_DEVICE_MANUFACTURING_DATE}\"\n")
string(APPEND script_args "--vendor-id ${CONFIG_CHIP_DEVICE_VENDOR_ID}\n")
string(APPEND script_args "--product-id ${CONFIG_CHIP_DEVICE_PRODUCT_ID}\n")
string(APPEND script_args "--vendor-name \"${CONFIG_CHIP_DEVICE_VENDOR_NAME}\"\n")
string(APPEND script_args "--product-name \"${CONFIG_CHIP_DEVICE_PRODUCT_NAME}\"\n")
string(APPEND script_args "--hw-ver ${CONFIG_CHIP_DEVICE_HARDWARE_VERSION}\n")
string(APPEND script_args "--hw-ver-str \"${CONFIG_CHIP_DEVICE_HARDWARE_VERSION_STRING}\"\n")
string(APPEND script_args "--overwrite\n")
string(APPEND script_args "--in-tree\n")

# check if Rotating Device Id Unique Id should be generated
if(NOT CONFIG_CHIP_DEVICE_GENERATE_ROTATING_DEVICE_UID)
    if(NOT DEFINED CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID)
        message(FATAL_ERROR "CHIP_DEVICE_ROTATING_DEVICE_UID was not provided. To generate it use CONFIG_CHIP_DEVICE_GENERATE_ROTATING_DEVICE_UID=y")
    else()
        string(APPEND script_args "--rd-id-uid \"${CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID}\"\n")
    endif()
else()
    string(APPEND script_args "--enable-rotating-device-id\n")
endif()

# find chip cert tool
find_program(chip_cert_exe NAMES chip-cert PATHS ${CHIP_ROOT}/out REQUIRED)
string(APPEND script_args "--chip-cert-path ${chip_cert_exe}\n")

# for development purpose user can use default certs instead of generating or providing them
if(CONFIG_CHIP_FACTORY_DATA_USE_DEFAULT_CERTS)
    # convert decimal VID to its hexadecimal representation to find out certification files in repository
    math(EXPR LOCAL_VID "${CONFIG_CHIP_DEVICE_VENDOR_ID}" OUTPUT_FORMAT HEXADECIMAL)
    string(SUBSTRING ${LOCAL_VID} 2 -1 raw_vid)
    string(TOUPPER ${raw_vid} raw_vid_upper)
    # convert decimal PID to its hexadecimal representation to find out certification files in repository
    math(EXPR LOCAL_PID "${CONFIG_CHIP_DEVICE_PRODUCT_ID}" OUTPUT_FORMAT HEXADECIMAL)
    string(SUBSTRING ${LOCAL_PID} 2 -1 raw_pid)
    string(TOUPPER ${raw_pid} raw_pid_upper)
    # all certs are located in ${CHIP_ROOT}/credentials/development/attestation
    # it can be used during development without need to generate new certifications
    string(APPEND script_args "--dac-cert \"${CHIP_ROOT}/credentials/development/attestation/Matter-Development-DAC-${raw_vid_upper}-${raw_pid_upper}-Cert.pem\"\n")
    string(APPEND script_args "--dac-key \"${CHIP_ROOT}/credentials/development/attestation/Matter-Development-DAC-${raw_vid_upper}-${raw_pid_upper}-Key.pem\"\n")
    string(APPEND script_args "--cert \"${CHIP_ROOT}/credentials/development/attestation/Matter-Development-PAI-${raw_vid_upper}-noPID-Cert.pem\"\n")
    string(APPEND script_args "--key \"${CHIP_ROOT}/credentials/development/attestation/Matter-Development-PAI-${raw_vid_upper}-noPID-Key.pem\"\n")
    string(APPEND script_args "-cd \"${CHIP_ROOT}/credentials/development/cd-certs/Chip-Test-CD-Cert.der\"\n")
    string(APPEND script_args "--pai\n")
else()
    # generate PAI and DAC certs
    string(APPEND script_args "--cert \"${CHIP_ROOT}/credentials/test/attestation/Chip-Test-PAA-NoVID-Cert.pem\"\n")
    string(APPEND script_args "--key \"${CHIP_ROOT}/credentials/test/attestation/Chip-Test-PAA-NoVID-Key.pem\"\n")
    string(APPEND script_args "-cd \"${CHIP_ROOT}/credentials/development/cd-certs/Chip-Test-CD-Cert.der\"\n")
    string(APPEND script_args "--paa\n")
endif()

# find chip tool requied for generating QRCode
find_program(chip_tool_exe NAMES chip-tool PATHS ${CHIP_ROOT}/out REQUIRED)
string(APPEND script_args "--chip-tool-path ${chip_tool_exe}\n")

# add Password-Authenticated Key Exchange parameters
string(APPEND script_args "--spake2-it \"${CONFIG_CHIP_DEVICE_SPAKE2_IT}\"\n")
string(APPEND script_args "--discriminator ${CONFIG_CHIP_DEVICE_DISCRIMINATOR}\n")
string(APPEND script_args "--passcode ${CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE}\n")

# request spake2p to generate a new spake2_verifier
find_program(spake_exe NAMES spake2p PATHS ${CHIP_ROOT}/out REQUIRED)
string(APPEND script_args "--spake2-path ${spake_exe}\n")

if(CONFIG_CHIP_DEVICE_ENABLE_KEY)
# Add optional EnableKey that triggers user-specific action.
string(APPEND script_args "--enable-key \"${CONFIG_CHIP_DEVICE_ENABLE_KEY}\"\n")
endif()

# get code-partition factory_partition address
dt_nodelabel(dts_partition_path NODELABEL "factory_partition")
dt_reg_addr(factory_off PATH ${dts_partition_path})

# get code-partition factory_partition size
dt_nodelabel(dts_partition_path NODELABEL "factory_partition")
dt_reg_size(factory_size PATH ${dts_partition_path})

string(APPEND script_args "--offset ${factory_off}\n")
string(APPEND script_args "--size ${factory_size}\n")

string(APPEND script_args "--output \"${output_path}\"/factory\n")

set(factory_data_bin ${output_path}/factory/factory_data.bin)

# execute a script to create a factory data file
separate_arguments(separated_script_args NATIVE_COMMAND ${script_args})
add_custom_command(
    OUTPUT ${factory_data_bin}
    DEPENDS ${FACTORY_DATA_SCRIPT_PATH}
    COMMAND ${Python3_EXECUTABLE} ${FACTORY_DATA_SCRIPT_PATH} ${separated_script_args}
    COMMENT "Generating new Factory Data..."
    )
add_custom_target(${factory_data_target} ALL
    DEPENDS ${factory_data_bin}
    )

endfunction()


# Generate factory data partition using given args
#
# 
# During generation process a some file will be created in zephyr's build directory:
# - merged.hex a file containing firmware and factory data merged to single file
# - factory_data.hex a file containing only a factory data partition including proper offset
#
function(telink_generate_factory_data)

find_package(Python REQUIRED)

# CHIP_ROOT must be provided as a reference set all localization of scripts
if(NOT CHIP_ROOT)
message(FATAL_ERROR "CHIP_ROOT variable is not set, please add it to CMakeLists.txt file")
endif()

# Localize all scripts needed to generate factory data partition
set(FACTORY_DATA_SCRIPT_PATH ${CHIP_ROOT}/scripts/tools/telink/mfg_tool.py)
set(OUTPUT_FILE_PATH ${APPLICATION_BINARY_DIR}/zephyr)

# create a binary file with all factory data
telink_create_factory_data_json(factory_data
                                    ${FACTORY_DATA_SCRIPT_PATH}
                                    ${OUTPUT_FILE_PATH})

endfunction()
