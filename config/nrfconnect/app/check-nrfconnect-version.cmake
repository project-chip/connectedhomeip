#
# SPDX-FileCopyrightText: 2021 Project CHIP Authors
#
# SPDX-License-Identifier: Apache-2.0
#

# This function is only needed to prevent find_package from polluting the global
# variable namespace before loading Zephyr CMake module. Without this workaround
# the build may fail because Zephyr calls find_package(Python3) with different
# constraints and CMake is not able to handle such a scenario.
function(check_nrfconnect_version)
    find_package(Python3 REQUIRED)

    # Check nRF Connect SDK version in the CMake configuration phase
    execute_process(
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../..
        COMMAND ${Python3_EXECUTABLE} scripts/setup/nrfconnect/update_ncs.py --check --quiet)

    # Check nRF Connect SDK version in the build phase
    add_custom_target(check-nrfconnect-version ALL
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../../..
        COMMAND ${Python3_EXECUTABLE} scripts/setup/nrfconnect/update_ncs.py --check --quiet || (exit 0)
        USES_TERMINAL)
endfunction()

check_nrfconnect_version()
