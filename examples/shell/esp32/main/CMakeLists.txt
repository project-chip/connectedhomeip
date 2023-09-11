#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

set(CHIP_SHELL_DIR "${CMAKE_SOURCE_DIR}/third_party/connectedhomeip/examples/shell")


idf_component_register(SRCS main.cpp
                      "${CHIP_SHELL_DIR}/shell_common/globals.cpp"
                      PRIV_INCLUDE_DIRS
                      "${CHIP_SHELL_DIR}/shell_common/include"
                      PRIV_REQUIRES chip nvs_flash bt)
