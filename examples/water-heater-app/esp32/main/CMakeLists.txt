#
# Copyright (c) 2021-2026 Project CHIP Authors
# All rights reserved.
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
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

get_filename_component(CHIP_ROOT ${CMAKE_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(WATER_HEATER_COMMON_DIR ${CHIP_ROOT}/examples/water-heater-app/water-heater-common REALPATH)
get_filename_component(ENERGY_MGMT_DIR ${CHIP_ROOT}/examples/energy-management REALPATH)
get_filename_component(APP_COMMON_GEN_DIR ${CHIP_ROOT}/zzz_generated/app-common/app-common/zap-generated REALPATH)

set(PRIV_INCLUDE_DIRS_LIST
  "${CHIP_ROOT}/examples/providers"
  "${WATER_HEATER_COMMON_DIR}/include"
  "${ENERGY_MGMT_DIR}/device-energy-management/include"
  "${ENERGY_MGMT_DIR}/electrical-sensor/include"
  "${ENERGY_MGMT_DIR}/utils/include"
  "${ENERGY_MGMT_DIR}/energy-management-triggers"
  "${CMAKE_CURRENT_LIST_DIR}/include"
  "${CHIP_ROOT}/examples/platform/esp32"
)
set(SRC_DIRS_LIST
  "${CMAKE_CURRENT_LIST_DIR}"
  "${APP_COMMON_GEN_DIR}/attributes"
  "${APP_COMMON_GEN_DIR}"
  "${CHIP_ROOT}/examples/providers"
  "${WATER_HEATER_COMMON_DIR}/src"
  "${ENERGY_MGMT_DIR}/device-energy-management/src"
  "${ENERGY_MGMT_DIR}/electrical-sensor/src"
  "${ENERGY_MGMT_DIR}/utils/src"
  "${ENERGY_MGMT_DIR}/energy-management-triggers"
  "${CHIP_ROOT}/examples/platform/esp32/ota"
  "${CHIP_ROOT}/examples/platform/esp32/common"
  "${CHIP_ROOT}/examples/platform/esp32/time"
  "${CHIP_ROOT}/examples/platform/esp32/shell_extension"
)

idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
  SRC_DIRS ${SRC_DIRS_LIST})

target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format-nonliteral")

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17>")
