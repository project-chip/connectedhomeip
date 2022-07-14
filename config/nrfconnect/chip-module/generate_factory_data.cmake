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


# Create a JSON file based on factory data given via kConfigs.
#
# This function creates a list of arguments for external script and then run it to write a JSON file.
# Created JSON file can be checked using JSON SCHEMA file if it is provided.
#
# This script can be manipulated using following kConfigs:
# - To merge generated factory data with final zephyr.hex file set kConfig CONFIG_CHIP_FACTORY_DATA_MERGE_WITH_FIRMWARE=y
# - To use default certification paths set CONFIG_CHIP_FACTORY_DATA_USE_DEFAULTS_CERTS_PATH=y 
# 
# During generation process a some file will be created in zephyr's build directory:
# - <factory_data_target>.args a file containing arguments for nrfconnect_generate_partition.py script.
# - <factory_data_target>.json a file containing all factory data written in JSON format.
#
# [Args]:
#   factory_data_target - a name for target to generate factory_data.
#   script_path         - a path to script that makes a JSON factory data file from given arguments.
#   schema_path         - a path to JSON schema file which can be used to verify generated factory data JSON file.
#                         This argument is optional, if you don't want to verify the JSON file put it empty "".
#   output_path         - a path to output directory, where created JSON file will be stored.
function(nrfconnect_create_factory_data_json factory_data_target script_path schema_path output_path)

# set script args for future purpose
set(script_args)
## generate all script arguments
string(APPEND script_args "--sn \"${CONFIG_CHIP_DEVICE_SERIAL_NUMBER}\"\n")
string(APPEND script_args "--date \"${CONFIG_CHIP_DEVICE_MANUFACTURING_DATE}\"\n")
string(APPEND script_args "--vendor_id ${CONFIG_CHIP_DEVICE_VENDOR_ID}\n")
string(APPEND script_args "--product_id ${CONFIG_CHIP_DEVICE_PRODUCT_ID}\n")
string(APPEND script_args "--vendor_name \"${CONFIG_CHIP_DEVICE_VENDOR_NAME}\"\n")
string(APPEND script_args "--product_name \"${CONFIG_CHIP_DEVICE_PRODUCT_NAME}\"\n")
string(APPEND script_args "--hw_ver ${CONFIG_CHIP_DEVICE_HARDWARE_VERSION}\n")
string(APPEND script_args "--hw_ver_str \"${CONFIG_CHIP_DEVICE_HARDWARE_VERSION_STRING}\"\n")

# check if Rotating Device Id Unique Id should be generated
if(NOT CONFIG_CHIP_DEVICE_GENERATE_ROTATING_DEVICE_UID)
    if(NOT DEFINED CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID)
        message(FATAL_ERROR "CHIP_DEVICE_ROTATING_DEVICE_UID was not provided. To generate it use CONFIG_CHIP_DEVICE_GENERATE_ROTATING_DEVICE_UID=y")
    else()
        string(APPEND script_args "--rd_uid \"${CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID}\"\n")
    endif()
else()
    string(APPEND script_args "--generate_rd_uid\n")
endif()

# for development purpose user can use default certs instead of generating or providing them
if(CONFIG_CHIP_FACTORY_DATA_USE_DEFAULT_CERTS)
    # convert decimal PID to its hexadecimal representation to find out certification files in repository
    math(EXPR LOCAL_PID "${CONFIG_CHIP_DEVICE_PRODUCT_ID}" OUTPUT_FORMAT HEXADECIMAL)
    string(SUBSTRING ${LOCAL_PID} 2 -1 raw_pid)
    # all certs are located in ${CHIP_ROOT}/credentials/development/attestation
    # it can be used during development without need to generate new certifications
    string(APPEND script_args "--dac_cert \"${CHIP_ROOT}/credentials/development/attestation/Matter-Development-DAC-${raw_pid}-Cert.der\"\n")
    string(APPEND script_args "--dac_key \"${CHIP_ROOT}/credentials/development/attestation/Matter-Development-DAC-${raw_pid}-Key.der\"\n")
    string(APPEND script_args "--pai_cert \"${CHIP_ROOT}/credentials/development/attestation/Matter-Development-PAI-noPID-Cert.der\"\n")
else()
    find_program(chip_cert_exe NAMES chip-cert REQUIRED)
    string(APPEND script_args "--gen_cd\n")
    string(APPEND script_args "--chip_cert_path ${chip_cert_exe}\n")
endif()

# add Password-Authenticated Key Exchange parameters
string(APPEND script_args "--spake2_it \"${CONFIG_CHIP_DEVICE_SPAKE2_IT}\"\n")
string(APPEND script_args "--spake2_salt \"${CONFIG_CHIP_DEVICE_SPAKE2_SALT}\"\n")
string(APPEND script_args "--discriminator ${CONFIG_CHIP_DEVICE_DISCRIMINATOR}\n")
string(APPEND script_args "--passcode ${CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE}\n")

# check if spake2 verifier should be generated using script
if(CONFIG_CHIP_FACTORY_DATA_GENERATE_SPAKE2_VERIFIER)
    # request script to generate a new spake2_verifier
    # by adding an argument to script_args
    find_program(spake_exe NAMES spake2p REQUIRED)
    string(APPEND script_args "--spake2p_path ${spake_exe}\n")   
else()
    # Spake2 verifier should be provided using kConfig
    string(APPEND script_args "--spake2_verifier \"${CONFIG_CHIP_DEVICE_SPAKE2_TEST_VERIFIER}\"\n")
endif()

if(CONFIG_CHIP_DEVICE_ENABLE_KEY)
# Add optional EnableKey that triggers user-specific action.
string(APPEND script_args "--enable_key \"${CONFIG_CHIP_DEVICE_ENABLE_KEY}\"\n")
endif()

# Set output JSON file and path to SCHEMA file to validate generated factory data
string(APPEND script_args "-o \"${output_path}/${factory_data_target}.json\"\n")
string(APPEND script_args "-s \"${schema_path}\"\n")

# execute first script to create a JSON file
separate_arguments(separated_script_args NATIVE_COMMAND ${script_args})
add_custom_target(${factory_data_target} ALL
    COMMAND ${Python3_EXECUTABLE} ${FACTORY_DATA_SCRIPT_PATH} ${separated_script_args}
    COMMENT "Generating new Factory Data..."
    )

endfunction()


# Create a .hex file with factory data in CBOR format.
#
# This function creates a .hex and .cbor files from given JSON factory data file.
#
# 
# During generation process some files will be created in zephyr's build directory:
# - <factory_data_target>_cbor.args a file containing arguments for nrfconnect_generate_partition.py script.
# - <factory_data_target>.hex a file containing all factory data in CBOR format.
# - <factory_data_target>.bin a binary file containing all raw factory data in CBOR format.
# - <factory_data_target>.cbor a file containing all factory data in CBOR format.
#
# [Args]:
#   factory_data_target - a name for target to generate factory_data.
#   script_path         - a path to script that makes a factory data .hex file from given arguments.
#   output_path         - a path to output directory, where created JSON file will be stored.
#   output_hex          - an output variable to store a .hex file. This variable can be used to merge with firmware .hex file.
function(nrfconnect_create_factory_data_hex_file factory_data_target script_path output_path output_hex)

# Pass the argument list via file
set(cbor_script_args "-i ${output_path}/${factory_data_target}.json\n")
string(APPEND cbor_script_args "-o ${output_path}/${factory_data_target}\n")
# get partition address and offset from partition manager during compilation
string(APPEND cbor_script_args "--offset $<TARGET_PROPERTY:partition_manager,PM_FACTORY_DATA_ADDRESS>\n")
string(APPEND cbor_script_args "--size $<TARGET_PROPERTY:partition_manager,PM_FACTORY_DATA_OFFSET>\n")
string(APPEND cbor_script_args "-r\n")

# execute second script to create a hex file containing factory data in cbor format
separate_arguments(separated_cbor_script_args NATIVE_COMMAND ${cbor_script_args})
set(factory_data_hex ${output_path}/${factory_data_target}.hex)

# return output hex to parent scope
set(${output_hex} ${factory_data_hex} PARENT_SCOPE)
add_custom_command(OUTPUT ${factory_data_hex}
    COMMAND ${Python3_EXECUTABLE} ${script_path} ${separated_cbor_script_args}
    COMMENT "Generating factory data HEX file..."
    DEPENDS ${factory_data_target}
    )

endfunction()

# Generate factory data partition using given args
#
# 
# During generation process a some file will be created in zephyr's build directory:
# - merged.hex a file containing firmware and factory data merged to single file
# - factory_data.hex a file containing only a factory data partition including proper offset
#
function(nrfconnect_generate_factory_data)

find_package(Python REQUIRED)

# CHIP_ROOT must be provided as a reference set all localization of scripts
if(NOT CHIP_ROOT)
message(FATAL_ERROR "CHIP_ROOT variable is not set, please add it to CMakeLists.txt file")
endif()

# Localize all scripts needed to generate factory data partition
set(FACTORY_DATA_SCRIPT_PATH ${CHIP_ROOT}/scripts/tools/nrfconnect/generate_nrfconnect_chip_factory_data.py)
set(GENERATE_CBOR_SCRIPT_PATH ${CHIP_ROOT}/scripts/tools/nrfconnect/nrfconnect_generate_partition.py)
SET(MERGE_HEX_SCRIPT_PATH ${CHIP_ROOT}/config/nrfconnect/chip-module/merge_factory_data.py)
set(FACTORY_DATA_SCHEMA_PATH ${CHIP_ROOT}/scripts/tools/nrfconnect/nrfconnect_factory_data.schema)
set(OUTPUT_FILE_PATH ${APPLICATION_BINARY_DIR}/zephyr)

# create a JSON file with all factory data
nrfconnect_create_factory_data_json(factory_data
                                    ${FACTORY_DATA_SCRIPT_PATH} 
                                    ${FACTORY_DATA_SCHEMA_PATH} 
                                    ${OUTPUT_FILE_PATH})

# create a .hex file with factory data in CBOR format based on the JSON file created previously 
nrfconnect_create_factory_data_hex_file(factory_data
                                        ${GENERATE_CBOR_SCRIPT_PATH} 
                                        ${OUTPUT_FILE_PATH} 
                                        factory_data_hex)

if(CONFIG_CHIP_FACTORY_DATA_MERGE_WITH_FIRMWARE)                                   
    # set custom target for merging factory_data hex file
    add_custom_target(factory_data_merge
        DEPENDS ${factory_data_hex}
        )
    set_property(GLOBAL PROPERTY 
    factory_data_PM_HEX_FILE 
        ${factory_data_hex}
        )
    set_property(GLOBAL PROPERTY
        ${parent_slot}_PM_TARGET
        ${target_name}_merge
        )
endif()


endfunction()
