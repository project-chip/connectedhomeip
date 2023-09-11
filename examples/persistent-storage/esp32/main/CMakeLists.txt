#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

idf_component_register(PRIV_INCLUDE_DIRS
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/persistent-storage"
                      SRC_DIRS
                      "${CMAKE_CURRENT_LIST_DIR}"
                      "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/persistent-storage"
                      PRIV_REQUIRES chip nvs_flash)

set_property(TARGET ${COMPONENT_LIB} PROPERTY CXX_STANDARD 17)
target_compile_options(${COMPONENT_LIB} PRIVATE "-DCHIP_HAVE_CONFIG_H")
