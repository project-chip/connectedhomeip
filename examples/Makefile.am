#
#    Copyright (c) 2020 Project CHIP Authors
#    Copyright (c) 2014-2018 Nest Labs, Inc.
#    Copyright (c) 2018 Google LLC
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

#
#    Description:
#      This file is the GNU automake template for the CHIP SDK.
#

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

SUBDIRS                                                       = \
    $(NULL)

EXTRA_DIST                                                    = \
    $(NULL)

PRETTY_FILES                                                  = \
    lock-app/efr32/include/AppEvent.h                           \
    lock-app/efr32/include/LEDWidget.h                          \
    lock-app/efr32/include/AppTask.h                            \
    lock-app/efr32/include/CHIPProjectConfig.h                  \
    lock-app/efr32/include/sample_qr_code.h                     \
    lock-app/efr32/include/BoltLockManager.h                    \
    lock-app/efr32/include/AppConfig.h                          \
    lock-app/efr32/include/ble-configuration.h                  \
    lock-app/efr32/include/init_board.h                         \
    lock-app/efr32/include/init_mcu.h                           \
    lock-app/efr32/include/hal-config-app-common.h              \
    lock-app/efr32/include/init_lcd.h                           \
    lock-app/efr32/include/FreeRTOSConfig.h                     \
    lock-app/efr32/include/hal-config.h                         \
    lock-app/efr32/include/board_features.h                     \
    lock-app/efr32/include/mbedtls-config.h                     \
    lock-app/efr32/include/ButtonHandler.h                      \
    lock-app/efr32/src/main.cpp                                 \
    lock-app/efr32/src/display/init_lcd.c                       \
    lock-app/efr32/src/display/sample_qr_code.c                 \
    lock-app/efr32/src/AppTask.cpp                              \
    lock-app/efr32/src/LEDWidget.cpp                            \
    lock-app/efr32/src/ButtonHandler.cpp                        \
    lock-app/nrf5/main/BoltLockManager.cpp                      \
    lock-app/nrf5/main/include/OpenThreadConfig.h               \
    lock-app/nrf5/main/include/nrf_log_ctrl_internal.h          \
    lock-app/nrf5/main/include/AppEvent.h                       \
    lock-app/nrf5/main/include/LEDWidget.h                      \
    lock-app/nrf5/main/include/AppTask.h                        \
    lock-app/nrf5/main/include/CHIPProjectConfig.h              \
    lock-app/nrf5/main/include/freertos_tasks_c_additions.h     \
    lock-app/nrf5/main/include/BoltLockManager.h                \
    lock-app/nrf5/main/include/FreeRTOSConfig.h                 \
    lock-app/nrf5/main/include/app_config.h                     \
    lock-app/nrf5/main/main.cpp                                 \
    lock-app/nrf5/main/AppTask.cpp                              \
    lock-app/nrf5/main/LEDWidget.cpp                            \
    lock-app/nrf5/main/ldscripts/chip-nrf52840-lock-example.ld  \
    lock-app/nrf5/main/support/AltPrintf.c                      \
    lock-app/nrf5/main/support/FreeRTOSNewlibLockSupport.c      \
    lock-app/nrf5/main/support/nRF5Sbrk.c                       \
    $(NULL)

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
