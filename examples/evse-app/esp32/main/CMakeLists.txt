#
# Copyright (c) 2021-2024 Project CHIP Authors
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
get_filename_component(EVSE_COMMON_DIR ${CHIP_ROOT}/examples/evse-app/evse-common REALPATH)
get_filename_component(ENERGY_MGMT_DIR ${CHIP_ROOT}/examples/energy-management REALPATH)
get_filename_component(EVSE_TRIGGERS_DIR ${CHIP_ROOT}/examples/evse-app/energy-management-triggers REALPATH)
get_filename_component(APP_COMMON_GEN_DIR ${CHIP_ROOT}/zzz_generated/app-common/app-common/zap-generated REALPATH)

set(PRIV_INCLUDE_DIRS_LIST
  "${CHIP_ROOT}/examples/providers"
  "${EVSE_COMMON_DIR}/include"
  "${ENERGY_MGMT_DIR}/device-energy-management/include"
  "${ENERGY_MGMT_DIR}/electrical-sensor/include"
  "${EVSE_TRIGGERS_DIR}"
  "${CMAKE_CURRENT_LIST_DIR}/include"
  "${CHIP_ROOT}/examples/platform/esp32"
)
set(SRC_DIRS_LIST
  "${CMAKE_CURRENT_LIST_DIR}"
  "${APP_COMMON_GEN_DIR}/attributes"
  "${APP_COMMON_GEN_DIR}"
  "${CHIP_ROOT}/examples/providers"
  "${EVSE_COMMON_DIR}/src"
  "${ENERGY_MGMT_DIR}/device-energy-management/src"
  "${ENERGY_MGMT_DIR}/electrical-sensor/src"
  "${EVSE_TRIGGERS_DIR}"
  "${CHIP_ROOT}/examples/platform/esp32/ota"
  "${CHIP_ROOT}/examples/platform/esp32/common"
  "${CHIP_ROOT}/examples/platform/esp32/time"
  "${CHIP_ROOT}/examples/platform/esp32/shell_extension"
)

idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
  SRC_DIRS ${SRC_DIRS_LIST})

include(${CHIP_ROOT}/src/app/chip_data_model.cmake)
chip_configure_data_model(${COMPONENT_LIB}
    ZAP_FILE ${EVSE_COMMON_DIR}/evse-app.zap
)

if(CONFIG_ENABLE_ESP_INSIGHTS_TRACE)
  target_add_binary_data(${COMPONENT_TARGET} "insights_auth_key.txt" TEXT)
endif()

target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
target_compile_options(${COMPONENT_LIB} PUBLIC
  "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)
