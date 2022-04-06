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

find_package(Python3 REQUIRED)

#
# Create CMake target for building a Matter OTA content header that describes a list of files
# included in a Matter OTA image. The header format is specific to the nRF Connect platform.
#
# Required arguments:
#   FILE_TYPES type, [type...] - list of file types that identify files included in the image.
#   Currently recognized file types are:
#     * mcuboot     - MCUBoot bootloader image in the raw binary format
#     * app_mcuboot - application core image in the MCUBoot-defined format
#     * net_mcuboot - network core image in the MCUBoot-defined format
#   FILE_PATHS path, [path...] - list of paths to files included in the image
#   OUTPUT_FILE path - file path to write the newly created Matter OTA content header
#
function(chip_ota_content_header TARGET_NAME)
    cmake_parse_arguments(ARG "" "OUTPUT_FILE" "FILE_TYPES;FILE_PATHS" ${ARGN})

    if (NOT ARG_FILE_TYPES OR NOT ARG_FILE_PATHS OR NOT ARG_OUTPUT_FILE)
        message(FATAL_ERROR "All FILE_TYPES, FILE_PATHS and OUTPUT_FILE arguments must be specified")
    endif()

    # Prepare make_ota_content_header.py argument list
    set(SCRIPT_ARGS)

    foreach(file_info IN ZIP_LISTS ARG_FILE_TYPES ARG_FILE_PATHS)
        list(APPEND SCRIPT_ARGS
            --file
            "${file_info_0}"
            "${file_info_1}"
        )
    endforeach()

    list(APPEND SCRIPT_ARGS ${ARG_OUTPUT_FILE})

    # Convert the argument list to multi-line string
    string(REPLACE ";" "\n" SCRIPT_ARGS "${SCRIPT_ARGS}")

    # Pass the argument list via file to avoid hitting Windows command-line length limit
    file(GENERATE
        OUTPUT ${ARG_OUTPUT_FILE}.args
        CONTENT ${SCRIPT_ARGS}
    )

    add_custom_target(${TARGET_NAME} ALL
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/make_ota_content_header.py @${ARG_OUTPUT_FILE}.args
    )
endfunction()
