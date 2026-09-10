#
#    Copyright (c) 2026 Project CHIP Authors
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

include(${APP_DIR}/cmake/board_files.cmake)

# BOARD may include qualifiers (e.g. xg26_rb4118a/efr32mg26b510f3200il136); our
# per-board filenames use the bare board name.
all_devices_board_name(${BOARD} _board_name)

all_devices_find_board_file(${APP_DIR}/boards ${_board_name}.overlay _board_overlay)
all_devices_append_image_files(zephyr_DTC_OVERLAY_FILE ${_board_overlay})
all_devices_append_image_files(mcuboot_EXTRA_DTC_OVERLAY_FILE ${_board_overlay})

all_devices_find_board_file(${APP_DIR}/boards ${_board_name}.conf _board_conf EXCLUDE_FLAT)
all_devices_append_image_files(zephyr_EXTRA_CONF_FILE ${_board_conf})

all_devices_find_board_file(${APP_DIR}/boards ${_board_name}_mcuboot.conf _board_mcuboot_conf)
set(_mcuboot_extra_conf ${APP_DIR}/sysbuild/mcuboot.conf)
if(_board_mcuboot_conf)
    list(APPEND _mcuboot_extra_conf ${_board_mcuboot_conf})
endif()
all_devices_append_image_files(mcuboot_EXTRA_CONF_FILE ${_mcuboot_extra_conf})

# Apply boards/${BOARD}_sysbuild.conf (optionally grouped under a vendor subfolder).
all_devices_find_board_file(${APP_DIR}/boards ${_board_name}_sysbuild.conf _board_sysbuild_conf)
if(_board_sysbuild_conf)
    list(APPEND SB_EXTRA_CONF_FILE ${_board_sysbuild_conf})
endif()

find_package(Sysbuild REQUIRED HINTS $ENV{ZEPHYR_BASE})

project(sysbuild LANGUAGES)
