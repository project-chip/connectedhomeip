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


# Create a .hex file in CBOR format based on factory data given via kConfigs.
#
# This function creates a list of arguments for external script and then run it to write a JSON file.
# Created JSON file can be checked using JSON SCHEMA file if it is provided.
# Next, the resulting .hex file is generated based on previously created JSON file.
#
# This script can be manipulated using following kConfigs:
# - To merge generated factory data with final zephyr.hex file set kConfig CONFIG_CHIP_FACTORY_DATA_MERGE_WITH_FIRMWARE=y
# - To use default certification paths set CONFIG_CHIP_FACTORY_DATA_USE_DEFAULTS_CERTS_PATH=y 
# 
# During generation process the following files will be created in zephyr's build directory:
# - <factory_data_target>.json a file containing all factory data written in JSON format.
# - <factory_data_target>.hex a file containing all factory data in CBOR format.
# - <factory_data_target>.bin a binary file containing all raw factory data in CBOR format.
# - <factory_data_target>.cbor a file containing all factory data in CBOR format.
#
# [Args]:
#   factory_data_target - a name for target to generate factory_data.
#   script_path         - a path to script that makes a JSON factory data file from given arguments.
#   schema_path         - a path to JSON schema file which can be used to verify generated factory data JSON file.
#                         This argument is optional, if you don't want to verify the JSON file put it empty "".

#   output_path         - a path to output directory, where created hex and JSON files will be stored.
function(nrfconnect_create_factory_data factory_data_target script_path schema_path output_path)

# Transforming kConfigs to sysbuild format
if(SB_CONFIG_PARTITION_MANAGER)
    sysbuild_get(CONFIG_CHIP_DEVICE_SERIAL_NUMBER IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_SERIAL_NUMBER KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_MANUFACTURING_DATE IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_MANUFACTURING_DATE KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_VENDOR_ID IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_VENDOR_ID KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_PRODUCT_ID IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_PRODUCT_ID KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_VENDOR_NAME IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_VENDOR_NAME KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_PRODUCT_NAME IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_PRODUCT_NAME KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_HARDWARE_VERSION IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_HARDWARE_VERSION KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_HARDWARE_VERSION_STRING IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_HARDWARE_VERSION_STRING KCONFIG)
    sysbuild_get(CONFIG_CHIP_ROTATING_DEVICE_ID IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_ROTATING_DEVICE_ID KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_GENERATE_ROTATING_DEVICE_UID IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_GENERATE_ROTATING_DEVICE_UID KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID KCONFIG)
    sysbuild_get(CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED KCONFIG)
    sysbuild_get(CONFIG_CHIP_FACTORY_DATA_GENERATE_CD IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_FACTORY_DATA_GENERATE_CD KCONFIG)
    sysbuild_get(CONFIG_CHIP_FACTORY_DATA_USE_DEFAULT_CERTS IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_FACTORY_DATA_USE_DEFAULT_CERTS KCONFIG)
    sysbuild_get(CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_USER IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_USER KCONFIG)
    sysbuild_get(CONFIG_CHIP_FACTORY_DATA_USER_CERTS_DAC_CERT IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_FACTORY_DATA_USER_CERTS_DAC_CERT KCONFIG)
    sysbuild_get(CONFIG_CHIP_FACTORY_DATA_USER_CERTS_DAC_KEY IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_FACTORY_DATA_USER_CERTS_DAC_KEY KCONFIG)
    sysbuild_get(CONFIG_CHIP_FACTORY_DATA_USER_CERTS_PAI_CERT IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_FACTORY_DATA_USER_CERTS_PAI_CERT KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_SPAKE2_IT IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_SPAKE2_IT KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_SPAKE2_SALT IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_SPAKE2_SALT KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_DISCRIMINATOR IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_DISCRIMINATOR KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_SPAKE2_TEST_VERIFIER IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_SPAKE2_TEST_VERIFIER KCONFIG)
    sysbuild_get(CONFIG_CHIP_FACTORY_DATA_GENERATE_SPAKE2_VERIFIER IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_FACTORY_DATA_GENERATE_SPAKE2_VERIFIER KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_PRODUCT_FINISH IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_PRODUCT_FINISH KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_PRODUCT_COLOR IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_PRODUCT_COLOR KCONFIG)
    sysbuild_get(CONFIG_CHIP_FACTORY_DATA_GENERATE_ONBOARDING_CODES IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_FACTORY_DATA_GENERATE_ONBOARDING_CODES KCONFIG)
    sysbuild_get(CONFIG_CHIP_DEVICE_ENABLE_KEY IMAGE ${DEFAULT_IMAGE} VAR CONFIG_CHIP_DEVICE_ENABLE_KEY KCONFIG)
endif()

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
if(CONFIG_CHIP_ROTATING_DEVICE_ID)
    if(NOT CONFIG_CHIP_DEVICE_GENERATE_ROTATING_DEVICE_UID)
        if(NOT DEFINED CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID)
            message(FATAL_ERROR "CHIP_DEVICE_ROTATING_DEVICE_UID was not provided. To generate it use CONFIG_CHIP_DEVICE_GENERATE_ROTATING_DEVICE_UID=y")
        else()
            string(APPEND script_args "--rd_uid \"${CONFIG_CHIP_DEVICE_ROTATING_DEVICE_UID}\"\n")
        endif()
    else()
        string(APPEND script_args "--generate_rd_uid\n")
    endif()
endif()

if(CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED OR CONFIG_CHIP_FACTORY_DATA_GENERATE_CD)
    find_program(chip_cert_exe NAMES chip-cert REQUIRED)
    string(APPEND script_args "--chip_cert_path ${chip_cert_exe}\n")
endif()

if(CONFIG_CHIP_FACTORY_DATA_GENERATE_CD)
    string(APPEND script_args "--gen_cd\n")
endif()

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
    string(APPEND script_args "--dac_cert \"${CHIP_ROOT}/credentials/development/attestation/Matter-Development-DAC-${raw_vid_upper}-${raw_pid_upper}-Cert.der\"\n")
    string(APPEND script_args "--dac_key \"${CHIP_ROOT}/credentials/development/attestation/Matter-Development-DAC-${raw_vid_upper}-${raw_pid_upper}-Key.der\"\n")
    string(APPEND script_args "--pai_cert \"${CHIP_ROOT}/credentials/development/attestation/Matter-Development-PAI-${raw_vid_upper}-noPID-Cert.der\"\n")
elseif(CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_USER)
    set(dac_cert "${CONFIG_CHIP_FACTORY_DATA_USER_CERTS_DAC_CERT}")
    set(dac_key "${CONFIG_CHIP_FACTORY_DATA_USER_CERTS_DAC_KEY}")
    set(pai_cert "${CONFIG_CHIP_FACTORY_DATA_USER_CERTS_PAI_CERT}")
    string(CONFIGURE "${dac_cert}" dac_cert)
    string(CONFIGURE "${dac_key}" dac_key)
    string(CONFIGURE "${pai_cert}" pai_cert)
    string(APPEND script_args "--dac_cert \"${dac_cert}\"\n")
    string(APPEND script_args "--dac_key \"${dac_key}\"\n")
    string(APPEND script_args "--pai_cert \"${pai_cert}\"\n")
elseif(CONFIG_CHIP_FACTORY_DATA_CERT_SOURCE_GENERATED)
    string(APPEND script_args "--gen_certs\n")
endif()

# add Password-Authenticated Key Exchange parameters
string(APPEND script_args "--spake2_it \"${CONFIG_CHIP_DEVICE_SPAKE2_IT}\"\n")
string(APPEND script_args "--spake2_salt \"${CONFIG_CHIP_DEVICE_SPAKE2_SALT}\"\n")
string(APPEND script_args "--discriminator ${CONFIG_CHIP_DEVICE_DISCRIMINATOR}\n")
string(APPEND script_args "--passcode ${CONFIG_CHIP_DEVICE_SPAKE2_PASSCODE}\n")
string(APPEND script_args "--include_passcode\n")
string(APPEND script_args "--overwrite\n")
# check if spake2 verifier should be generated using script
if(NOT CONFIG_CHIP_FACTORY_DATA_GENERATE_SPAKE2_VERIFIER)
    # Spake2 verifier should be provided using kConfig
    string(APPEND script_args "--spake2_verifier \"${CONFIG_CHIP_DEVICE_SPAKE2_TEST_VERIFIER}\"\n")
endif()

# Product appearance
string(APPEND script_args "--product_finish ${CONFIG_CHIP_DEVICE_PRODUCT_FINISH}\n")
if(CONFIG_CHIP_DEVICE_PRODUCT_COLOR)
    string(APPEND script_args "--product_color ${CONFIG_CHIP_DEVICE_PRODUCT_COLOR}\n")
endif()

if(CONFIG_CHIP_FACTORY_DATA_GENERATE_ONBOARDING_CODES)
    string(APPEND script_args "--generate_onboarding\n")
endif()

if(CONFIG_CHIP_DEVICE_ENABLE_KEY)
# Add optional EnableKey that triggers user-specific action.
string(APPEND script_args "--enable_key \"${CONFIG_CHIP_DEVICE_ENABLE_KEY}\"\n")
endif()

# Set output path and path to SCHEMA file to validate generated factory data
set(factory_data_output_path ${output_path}/${factory_data_target})
string(APPEND script_args "-o \"${factory_data_output_path}\"\n")
string(APPEND script_args "-s \"${schema_path}\"\n")

# Add optional offset and size arguments to generate .hex file as well as .json.
if(SB_CONFIG_PARTITION_MANAGER OR CONFIG_PARTITION_MANAGER_ENABLED)
    string(APPEND script_args "--offset $<TARGET_PROPERTY:partition_manager,PM_FACTORY_DATA_ADDRESS>\n")
    string(APPEND script_args "--size $<TARGET_PROPERTY:partition_manager,PM_FACTORY_DATA_OFFSET>\n")
else()
    dt_nodelabel(factory_data_partition_path TARGET ${DEFAULT_IMAGE} NODELABEL "factory_data_partition" REQUIRED)
    dt_reg_addr(factory_data_addr TARGET ${DEFAULT_IMAGE} PATH ${factory_data_partition_path})
    dt_reg_size(factory_data_size TARGET ${DEFAULT_IMAGE} PATH ${factory_data_partition_path})
    string(APPEND script_args "--offset ${factory_data_addr}\n")
    string(APPEND script_args "--size ${factory_data_size}\n")
endif()

# execute first script to create a JSON file
separate_arguments(separated_script_args NATIVE_COMMAND ${script_args})
add_custom_command(
    OUTPUT ${factory_data_output_path}.hex
    DEPENDS ${FACTORY_DATA_SCRIPT_PATH}
    COMMAND ${Python3_EXECUTABLE} ${FACTORY_DATA_SCRIPT_PATH} ${separated_script_args}
    COMMENT "Generating new Factory Data..."
    )
add_custom_target(${factory_data_target} ALL
    DEPENDS ${factory_data_output_path}.hex
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

find_package(Python3 REQUIRED)

# CHIP_ROOT must be provided as a reference set all localization of scripts
if(NOT CHIP_ROOT AND ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR)
  set(CHIP_ROOT ${ZEPHYR_CONNECTEDHOMEIP_MODULE_DIR})
endif()

if(NOT CHIP_ROOT)
message(FATAL_ERROR "CHIP_ROOT variable is not set, please add it to CMakeLists.txt file")
endif()

# Localize all scripts needed to generate factory data partition
set(FACTORY_DATA_SCRIPT_PATH ${CHIP_ROOT}/scripts/tools/nrfconnect/generate_nrfconnect_chip_factory_data.py)
set(GENERATE_CBOR_SCRIPT_PATH ${CHIP_ROOT}/scripts/tools/nrfconnect/nrfconnect_generate_partition.py)
set(FACTORY_DATA_SCHEMA_PATH ${CHIP_ROOT}/scripts/tools/nrfconnect/nrfconnect_factory_data.schema)

if(SB_CONFIG_PARTITION_MANAGER OR CONFIG_PARTITION_MANAGER_ENABLED)
  set(OUTPUT_FILE_PATH ${APPLICATION_BINARY_DIR}/zephyr)
else()
  # Factory data is now a separate image, so we need to store output files in its build directory
  # to be compatible with the whole system.
  set(OUTPUT_FILE_PATH ${APPLICATION_BINARY_DIR}/matter_factory_data/zephyr)

  import_kconfig("CONFIG" ${CMAKE_BINARY_DIR}/${DEFAULT_IMAGE}/zephyr/.config)
endif()

# create a .hex file with factory data in CBOR format based on the JSON file created previously 
nrfconnect_create_factory_data(factory_data
                               ${FACTORY_DATA_SCRIPT_PATH}
                               ${FACTORY_DATA_SCHEMA_PATH}
                               ${OUTPUT_FILE_PATH})

if(SB_CONFIG_PARTITION_MANAGER AND SB_CONFIG_MATTER_FACTORY_DATA_MERGE_WITH_FIRMWARE)
    # set custom target for merging factory_data hex file
    set_property(GLOBAL PROPERTY factory_data_PM_HEX_FILE ${OUTPUT_FILE_PATH}/factory_data.hex)
    set_property(GLOBAL PROPERTY factory_data_PM_TARGET factory_data)
endif()


endfunction()
