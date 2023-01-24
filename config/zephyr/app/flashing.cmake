#
#   Copyright (c) 2021 Project CHIP Authors
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

# Adds rules to generate a flashing wrapper script for an nrfconnect firmware.

set(FLASHBUNDLE_FLASHER_COMMON ${CMAKE_PROJECT_NAME}/firmware_utils.py)
set(FLASHBUNDLE_FLASHER_PLATFORM ${CMAKE_PROJECT_NAME}/nrfconnect_firmware_utils.py)
set(FLASHBUNDLE_FLASHER_WRAPPER ${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}.flash.py)
set(FLASHBUNDLE_FIRMWARE ${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}.hex)
set(FLASHBUNDLE_FILES ${FLASHBUNDLE_FLASHER_COMMON} ${FLASHBUNDLE_FLASHER_PLATFORM} ${FLASHBUNDLE_FLASHER_WRAPPER} ${FLASHBUNDLE_FIRMWARE})

get_filename_component(FLASHBUNDLE_FIRMWARE_BASENAME ${FLASHBUNDLE_FIRMWARE} NAME)

list(JOIN FLASHBUNDLE_FILES "\n" FLASHBUNDLE_MANIFEST)

file(GENERATE
  OUTPUT ${CMAKE_PROJECT_NAME}.flashbundle.txt
  CONTENT "${FLASHBUNDLE_MANIFEST}\n"
  )

add_custom_command(OUTPUT "${FLASHBUNDLE_FLASHER_COMMON}"
    COMMAND ${CMAKE_COMMAND} ARGS -E copy "${PROJECT_SOURCE_DIR}/third_party/connectedhomeip/scripts/flashing/firmware_utils.py" "${FLASHBUNDLE_FLASHER_COMMON}"
    VERBATIM)

add_custom_command(OUTPUT "${FLASHBUNDLE_FLASHER_PLATFORM}"
    COMMAND ${CMAKE_COMMAND} ARGS -E copy "${PROJECT_SOURCE_DIR}/third_party/connectedhomeip/scripts/flashing/nrfconnect_firmware_utils.py" "${FLASHBUNDLE_FLASHER_PLATFORM}"
    VERBATIM)

if (merged_hex_to_flash)
  set(flashbundle_hex_to_copy "${merged_hex_to_flash}")
else()
  set(flashbundle_hex_to_copy "zephyr/${KERNEL_HEX_NAME}")
endif()

add_custom_command(OUTPUT "${FLASHBUNDLE_FIRMWARE}"
    DEPENDS ${flashbundle_hex_to_copy}
    COMMAND ${CMAKE_COMMAND} -E copy "${flashbundle_hex_to_copy}" "${FLASHBUNDLE_FIRMWARE}")

add_custom_command(OUTPUT "${CMAKE_PROJECT_NAME}/${CMAKE_PROJECT_NAME}.flash.py"
    COMMAND ${python}
            "${PROJECT_SOURCE_DIR}/third_party/connectedhomeip/scripts/flashing/gen_flashing_script.py" nrfconnect
            --output "${FLASHBUNDLE_FLASHER_WRAPPER}"
            --application "${FLASHBUNDLE_FIRMWARE_BASENAME}"
    COMMENT "To flash ${FLASHBUNDLE_FIRMWARE_BASENAME} run ${FLASHBUNDLE_FLASHER_WRAPPER}"
    VERBATIM)

add_custom_target(flashing_script DEPENDS ${FLASHBUNDLE_FILES})
