#    Copyright (c) 2021 Project CHIP Authors
#    All rights reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#
#        http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.

# Common cmake code for creating flash scripts.
# Must include the IDF project.cmake file before this file.
# Usage:
#
#   flashing_script([DEPENDS <dep1> <dep2>])
#
# where DEPENDS and its args are optional and list additional dependencies.
# (use full path).

function(get_additional_flashing_depends)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs DEPENDS)
    cmake_parse_arguments(MYARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    set(additional_flashing_depends "${MYARGS_DEPENDS}" PARENT_SCOPE)
endfunction(get_additional_flashing_depends)

macro(flashing_script)
  idf_build_get_property(build_dir BUILD_DIR)
  idf_build_get_property(project_path PROJECT_DIR)
  idf_build_get_property(sdkconfig SDKCONFIG)
  idf_build_get_property(idf_path IDF_PATH)

  if (${IDF_TARGET} MATCHES "esp32*")
    set(board_type "esp32")
  else()
    message(FATAL_ERROR "Unknown board type ${IDF_TARGET}")
  endif()

  set(flashing_utils_dir "${project_path}/third_party/connectedhomeip/scripts/flashing/")
  set(board_firmware_utils "${board_type}_firmware_utils.py")
  configure_file("${flashing_utils_dir}/${board_firmware_utils}" "${build_dir}/${board_firmware_utils}" COPYONLY)
  configure_file("${flashing_utils_dir}/firmware_utils.py" "${build_dir}/firmware_utils.py" COPYONLY)

  get_additional_flashing_depends(${ARGN})
  foreach(dep IN LISTS additional_flashing_depends)
    get_filename_component(filename ${dep} NAME)
    configure_file("${dep}" "${build_dir}/${filename}" COPYONLY)
    list(APPEND build_dir_depends "${build_dir}/${filename}")
  endforeach(dep)

  add_custom_target(flashing_script
    COMMAND ${python}
            "${project_path}/../../../scripts/flashing/gen_flashing_script.py" ${board_type}
            --output "${build_dir}/${CMAKE_PROJECT_NAME}.flash.py"
            --port "$ENV{ESPPORT}"
            --baud 460800
            --before ${CONFIG_ESPTOOLPY_BEFORE}
            --after ${CONFIG_ESPTOOLPY_AFTER}
            --application "${CMAKE_PROJECT_NAME}.bin"
            --bootloader "bootloader/bootloader.bin"
            --partition "partition_table/partition-table.bin"
            --use-partition-file "${build_dir}/partition_table/partition-table.bin"
            --use-parttool ${idf_path}/components/partition_table/parttool.py
            --use-sdkconfig ${project_path}/sdkconfig
    WORKING_DIRECTORY ${build_dir}
    DEPENDS "${build_dir}/${board_firmware_utils}"
            "${build_dir}/firmware_utils.py"
            "${build_dir_deps}"
    COMMENT "To flash ${build_dir}/${CMAKE_PROJECT_NAME}.bin run ./build/${CMAKE_PROJECT_NAME}.flash.py"
    VERBATIM)
endmacro(flashing_script)
