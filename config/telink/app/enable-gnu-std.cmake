#
# SPDX-FileCopyrightText: (c) 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

add_library(gnu17 INTERFACE)
target_compile_options(gnu17 INTERFACE -std=gnu++17 -D_SYS__PTHREADTYPES_H_)
target_link_libraries(app PRIVATE gnu17)