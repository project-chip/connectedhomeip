#
#    Copyright (c) 2020 Project CHIP Authors
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
#      This file is the GNU automake template for the CHIP OSAL library
#      (Operating System Abstraction Layer).

include $(abs_top_nlbuild_autotools_dir)/automake/pre.am

SUBDIRS                                             = tests

lib_LIBRARIES                                       = libChipOsal.a

libChipOsal_a_SOURCES                               = $(NULL)

libChipOsal_adir                                    = $(includedir)/osal

dist_libChipOsal_a_HEADERS                          = \
    @top_builddir@/src/include/chip/osal.h            \
    @top_builddir@/src/osal/include/Ring.h            \
    @top_builddir@/src/osal/tests/test_util.h         \
    $(NULL)

libChipOsal_a_CPPFLAGS                              = \
    $(NLASSERT_CPPFLAGS)                              \
    -I$(top_srcdir)/src/include                       \
    -I$(top_srcdir)/src/osal/include                  \
    $(NULL)

if CHIP_DEVICE_LAYER_TARGET_LINUX

libChipOsal_a_CPPFLAGS                             += \
    -D_GNU_SOURCE                                     \
    -I$(top_srcdir)/src/osal/posix                    \
    $(NULL)

libChipOsal_a_SOURCES                              += \
    posix/os_queue.cc                                 \
    posix/os_mutex.c                                  \
    posix/os_sem.c                                    \
    posix/os_task.c                                   \
    posix/os_time.c                                   \
    posix/os_timer.c                                  \
    posix/os_utils.c                                  \
    $(NULL)

dist_libChipOsal_a_HEADERS                         += \
    @top_builddir@/src/osal/posix/chip/os_time.h      \
    @top_builddir@/src/osal/posix/chip/os_types.h     \
    @top_builddir@/src/osal/posix/chip/os_port.h      \
    @top_builddir@/src/osal/posix/os_utils.h          \
    @top_builddir@/src/osal/posix/RingPthread.h       \
    $(NULL)

endif # CHIP_DEVICE_LAYER_TARGET_LINUX

if CHIP_DEVICE_LAYER_TARGET_NRF5

FREERTOS_DIR ?= $(NRF5_SDK_ROOT)/external/freertos

# nRF5 specific dependencies in FreeRTOSConfig.h
libChipOsal_a_CPPFLAGS                             +=     \
    -I$(NRF5_SDK_ROOT)/modules/nrfx/mdk                   \
    -I$(NRF5_SDK_ROOT)/components/toolchain/cmsis/include \
    -I$(NRF5_SDK_ROOT)/components/softdevice/s140/headers \
    -I$(NRF5_SDK_ROOT)/components/libraries/util          \
    $(NULL)

libChipOsal_a_CPPFLAGS                             += \
    -D_GNU_SOURCE                                     \
    -I$(FREERTOS_DIR)/config                          \
    -I$(FREERTOS_DIR)/portable/CMSIS/nrf52            \
    -I$(FREERTOS_DIR)/portable/GCC/nrf52              \
    -I$(FREERTOS_DIR)/source/include                  \
    -I$(top_srcdir)/src/osal/freertos                 \
    $(NULL)

libChipOsal_a_SOURCES                              += \
    freertos/os_queue.c                               \
    freertos/os_mutex.c                               \
    freertos/os_sem.c                                 \
    freertos/os_task.c                                \
    freertos/os_time.c                                \
    $(NULL)

dist_libChipOsal_a_HEADERS                         += \
    @top_builddir@/src/osal/freertos/chip/os_port.h   \
    @top_builddir@/src/osal/freertos/os_hw.h          \
    $(NULL)

endif # CHIP_DEVICE_LAYER_TARGET_NRF5

include $(abs_top_nlbuild_autotools_dir)/automake/post.am
