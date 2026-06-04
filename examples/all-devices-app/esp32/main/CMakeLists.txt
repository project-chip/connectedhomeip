#
#    Copyright (c) 2025 Project CHIP Authors
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
#

get_filename_component(CHIP_ROOT "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip" REALPATH)

# ALL_DEVICES_COMMON_DIR is derived from CMAKE_CURRENT_LIST_DIR (this file's own
# directory) rather than from CHIP_ROOT.  During IDF's component requirements phase
# cmake processes this file with CMAKE_SOURCE_DIR set to the build directory, so
# CHIP_ROOT resolves to a non-existent build-tree path.  CMAKE_CURRENT_LIST_DIR
# always points to the actual source location of this file.
get_filename_component(ALL_DEVICES_COMMON_DIR "${CMAKE_CURRENT_LIST_DIR}/../../all-devices-common" REALPATH)

include("${ALL_DEVICES_COMMON_DIR}/devices/enabled_devices.cmake")

set(PRIV_INCLUDE_DIRS_LIST
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${ALL_DEVICES_COMMON_DIR}"
                      "${CHIP_ROOT}/examples/providers"
                      "${CMAKE_CURRENT_BINARY_DIR}"  # for generated app_config/enabled_devices.h
)

# Keep top-level src/app files explicit to avoid pulling unrelated src/app/*.cpp.
set(APP_TOPLEVEL_EXTRA_SRCS
    "${CHIP_ROOT}/src/app/SafeAttributePersistenceProvider.cpp"
    "${CHIP_ROOT}/src/app/StorageDelegateWrapper.cpp"
)

set(SRC_DIRS_LIST
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CHIP_ROOT}/examples/providers"
    "${CHIP_ROOT}/examples/platform/esp32/shell_extension"
    "${CHIP_ROOT}/src/data-model-providers/codedriven"
    "${CHIP_ROOT}/src/data-model-providers/codedriven/endpoint"
    "${CHIP_ROOT}/src/app/server"
    "${CHIP_ROOT}/src/app/persistence"
    "${CHIP_ROOT}/src/app/server-cluster"
    ${ALL_DEVICES_DEVICE_SRCDIRS}
)

idf_component_register(PRIV_INCLUDE_DIRS ${PRIV_INCLUDE_DIRS_LIST}
                       SRC_DIRS ${SRC_DIRS_LIST})

target_sources(${COMPONENT_LIB} PRIVATE ${APP_TOPLEVEL_EXTRA_SRCS})

target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
target_compile_options(${COMPONENT_LIB} PUBLIC
           "-DCHIP_ADDRESS_RESOLVE_IMPL_INCLUDE_HEADER=<lib/address_resolve/AddressResolve_DefaultImpl.h>"
)
