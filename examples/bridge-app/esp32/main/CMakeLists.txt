#
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

get_filename_component(CHIP_ROOT ${CMAKE_SOURCE_DIR}/third_party/connectedhomeip REALPATH)
get_filename_component(APP_COMMON_GEN_DIR ${CHIP_ROOT}/zzz_generated/app-common/app-common/zap-generated REALPATH)

idf_component_register(PRIV_INCLUDE_DIRS
                      "${CMAKE_CURRENT_LIST_DIR}/include"
                      "${CHIP_ROOT}/examples/providers"
                      "${CHIP_ROOT}/examples/bridge-app/bridge-common/include"
                      SRC_DIRS
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${APP_COMMON_GEN_DIR}/attributes"
                      "${APP_COMMON_GEN_DIR}"
                      "${CHIP_ROOT}/examples/platform/esp32/common"
                      "${CHIP_ROOT}/examples/providers"
                      "${CHIP_ROOT}/examples/bridge-app/bridge-common/src")

include(${CHIP_ROOT}/src/app/chip_data_model.cmake)
chip_configure_data_model(${COMPONENT_LIB}
    ZAP_FILE ${CHIP_ROOT}/examples/bridge-app/bridge-common/bridge-app.zap
)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
target_compile_options(${COMPONENT_LIB} PUBLIC
           "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)
