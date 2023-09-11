#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

add_library(gnu17 INTERFACE)
target_compile_options(gnu17 INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17> $<$<COMPILE_LANGUAGE:CXX>:-Wno-register> -D_DEFAULT_SOURCE)
target_link_libraries(app PRIVATE gnu17)
