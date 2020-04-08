#
#
#   Copyright (c) 2019 Google LLC.
#   All rights reserved.
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#

# 
#   @file
#         Makefile for building the OpenWeave nRF52840 Lock Example Application.
#

PROJECT_ROOT := $(realpath .)

OPENWEAVE_ROOT ?= $(PROJECT_ROOT)/third_party/openweave-core

BUILD_SUPPORT_DIR = $(OPENWEAVE_ROOT)/build/nrf5

include $(BUILD_SUPPORT_DIR)/nrf5-app.mk
include $(BUILD_SUPPORT_DIR)/nrf5-openweave.mk
include $(BUILD_SUPPORT_DIR)/nrf5-openthread.mk

APP = openweave-nrf52840-lock-example

SRCS = \
    $(PROJECT_ROOT)/main/main.cpp \
    $(PROJECT_ROOT)/main/AppTask.cpp \
    $(PROJECT_ROOT)/main/LEDWidget.cpp \
    $(PROJECT_ROOT)/main/BoltLockManager.cpp \
    $(PROJECT_ROOT)/main/WDMFeature.cpp \
    $(PROJECT_ROOT)/main/traits/BoltLockTraitDataSource.cpp \
    $(PROJECT_ROOT)/main/traits/BoltLockSettingsTraitDataSink.cpp \
    $(PROJECT_ROOT)/main/traits/DeviceIdentityTraitDataSource.cpp \
    $(PROJECT_ROOT)/main/schema/BoltLockTrait.cpp \
    $(PROJECT_ROOT)/main/schema/BoltLockSettingsTrait.cpp \
    $(PROJECT_ROOT)/main/schema/DeviceIdentityTrait.cpp \
    $(PROJECT_ROOT)/main/support/CXXExceptionStubs.cpp \
    $(PROJECT_ROOT)/main/support/nRF5Sbrk.c \
    $(PROJECT_ROOT)/main/support/FreeRTOSNewlibLockSupport.c \
    $(PROJECT_ROOT)/main/support/AltPrintf.c \
    $(PROJECT_ROOT)/third_party/printf/printf.c \
    $(NRF5_SDK_ROOT)/components/ble/common/ble_advdata.c \
    $(NRF5_SDK_ROOT)/components/ble/common/ble_srv_common.c \
    $(NRF5_SDK_ROOT)/components/ble/nrf_ble_gatt/nrf_ble_gatt.c \
    $(NRF5_SDK_ROOT)/components/boards/boards.c \
    $(NRF5_SDK_ROOT)/components/libraries/atomic/nrf_atomic.c \
    $(NRF5_SDK_ROOT)/components/libraries/atomic_fifo/nrf_atfifo.c \
    $(NRF5_SDK_ROOT)/components/libraries/balloc/nrf_balloc.c \
    $(NRF5_SDK_ROOT)/components/libraries/button/app_button.c \
    $(NRF5_SDK_ROOT)/components/libraries/crc16/crc16.c \
    $(NRF5_SDK_ROOT)/components/libraries/experimental_section_vars/nrf_section_iter.c \
    $(NRF5_SDK_ROOT)/components/libraries/fds/fds.c \
    $(NRF5_SDK_ROOT)/components/libraries/fstorage/nrf_fstorage.c \
    $(NRF5_SDK_ROOT)/components/libraries/fstorage/nrf_fstorage_sd.c \
    $(NRF5_SDK_ROOT)/components/libraries/log/src/nrf_log_backend_rtt.c \
    $(NRF5_SDK_ROOT)/components/libraries/log/src/nrf_log_backend_serial.c \
    $(NRF5_SDK_ROOT)/components/libraries/log/src/nrf_log_default_backends.c \
    $(NRF5_SDK_ROOT)/components/libraries/log/src/nrf_log_frontend.c \
    $(NRF5_SDK_ROOT)/components/libraries/log/src/nrf_log_str_formatter.c \
    $(NRF5_SDK_ROOT)/components/libraries/mem_manager/mem_manager.c \
    $(NRF5_SDK_ROOT)/components/libraries/memobj/nrf_memobj.c \
    $(NRF5_SDK_ROOT)/components/libraries/queue/nrf_queue.c \
    $(NRF5_SDK_ROOT)/components/libraries/ringbuf/nrf_ringbuf.c \
    $(NRF5_SDK_ROOT)/components/libraries/strerror/nrf_strerror.c \
    $(NRF5_SDK_ROOT)/components/libraries/timer/app_timer_freertos.c \
    $(NRF5_SDK_ROOT)/components/libraries/uart/retarget.c \
    $(NRF5_SDK_ROOT)/components/libraries/util/app_error.c \
    $(NRF5_SDK_ROOT)/components/libraries/util/app_error_handler_gcc.c \
    $(NRF5_SDK_ROOT)/components/libraries/util/app_error_weak.c \
    $(NRF5_SDK_ROOT)/components/libraries/util/app_util_platform.c \
    $(NRF5_SDK_ROOT)/components/libraries/util/nrf_assert.c \
    $(NRF5_SDK_ROOT)/components/softdevice/common/nrf_sdh.c \
    $(NRF5_SDK_ROOT)/components/softdevice/common/nrf_sdh_ble.c \
    $(NRF5_SDK_ROOT)/components/softdevice/common/nrf_sdh_soc.c \
    $(NRF5_SDK_ROOT)/components/thread/freertos_mbedtls_mutex/freertos_mbedtls_mutex.c \
    $(NRF5_SDK_ROOT)/external/fprintf/nrf_fprintf.c \
    $(NRF5_SDK_ROOT)/external/fprintf/nrf_fprintf_format.c \
    $(NRF5_SDK_ROOT)/external/freertos/portable/CMSIS/nrf52/port_cmsis.c \
    $(NRF5_SDK_ROOT)/external/freertos/portable/CMSIS/nrf52/port_cmsis_systick.c \
    $(NRF5_SDK_ROOT)/external/freertos/portable/GCC/nrf52/port.c \
    $(NRF5_SDK_ROOT)/external/freertos/source/croutine.c \
    $(NRF5_SDK_ROOT)/external/freertos/source/event_groups.c \
    $(NRF5_SDK_ROOT)/external/freertos/source/list.c \
    $(NRF5_SDK_ROOT)/external/freertos/source/portable/MemMang/heap_3.c \
    $(NRF5_SDK_ROOT)/external/freertos/source/queue.c \
    $(NRF5_SDK_ROOT)/external/freertos/source/stream_buffer.c \
    $(NRF5_SDK_ROOT)/external/freertos/source/tasks.c \
    $(NRF5_SDK_ROOT)/external/freertos/source/timers.c \
    $(NRF5_SDK_ROOT)/external/segger_rtt/SEGGER_RTT.c \
    $(NRF5_SDK_ROOT)/external/segger_rtt/SEGGER_RTT_printf.c \
    $(NRF5_SDK_ROOT)/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c \
    $(NRF5_SDK_ROOT)/integration/nrfx/legacy/nrf_drv_clock.c \
    $(NRF5_SDK_ROOT)/integration/nrfx/legacy/nrf_drv_rng.c \
    $(NRF5_SDK_ROOT)/modules/nrfx/drivers/src/nrfx_clock.c \
    $(NRF5_SDK_ROOT)/modules/nrfx/drivers/src/nrfx_gpiote.c \
    $(NRF5_SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uart.c \
    $(NRF5_SDK_ROOT)/modules/nrfx/drivers/src/nrfx_uarte.c \
    $(NRF5_SDK_ROOT)/modules/nrfx/drivers/src/prs/nrfx_prs.c \
    $(NRF5_SDK_ROOT)/modules/nrfx/mdk/gcc_startup_nrf52840.S \
    $(NRF5_SDK_ROOT)/modules/nrfx/mdk/system_nrf52840.c

INC_DIRS = \
    $(PROJECT_ROOT) \
    $(PROJECT_ROOT)/main \
    $(PROJECT_ROOT)/main/include \
    $(PROJECT_ROOT)/main/traits/include \
    $(PROJECT_ROOT)/main/schema/include \
    $(PROJECT_ROOT)/third_party/printf \
    $(NRF5_SDK_ROOT)/components \
    $(NRF5_SDK_ROOT)/components/boards \
    $(NRF5_SDK_ROOT)/components/ble/ble_advertising \
    $(NRF5_SDK_ROOT)/components/ble/common \
    $(NRF5_SDK_ROOT)/components/ble/nrf_ble_gatt \
    $(NRF5_SDK_ROOT)/components/libraries/atomic \
    $(NRF5_SDK_ROOT)/components/libraries/atomic_fifo \
    $(NRF5_SDK_ROOT)/components/libraries/balloc \
    $(NRF5_SDK_ROOT)/components/libraries/bsp \
    $(NRF5_SDK_ROOT)/components/libraries/button \
    $(NRF5_SDK_ROOT)/components/libraries/crc16 \
    $(NRF5_SDK_ROOT)/components/libraries/delay \
    $(NRF5_SDK_ROOT)/components/libraries/experimental_section_vars \
    $(NRF5_SDK_ROOT)/components/libraries/fds \
    $(NRF5_SDK_ROOT)/components/libraries/fstorage \
    $(NRF5_SDK_ROOT)/components/libraries/log \
    $(NRF5_SDK_ROOT)/components/libraries/log/src \
    $(NRF5_SDK_ROOT)/components/libraries/memobj \
    $(NRF5_SDK_ROOT)/components/libraries/mem_manager \
    $(NRF5_SDK_ROOT)/components/libraries/mutex \
    $(NRF5_SDK_ROOT)/components/libraries/queue \
    $(NRF5_SDK_ROOT)/components/libraries/ringbuf \
    $(NRF5_SDK_ROOT)/components/libraries/stack_info \
    $(NRF5_SDK_ROOT)/components/libraries/strerror \
    $(NRF5_SDK_ROOT)/components/libraries/timer \
    $(NRF5_SDK_ROOT)/components/libraries/util \
    $(NRF5_SDK_ROOT)/components/softdevice/common \
    $(NRF5_SDK_ROOT)/components/softdevice/s140/headers \
    $(NRF5_SDK_ROOT)/components/softdevice/mbr/nrf52840/headers \
    $(NRF5_SDK_ROOT)/components/thread/freertos_mbedtls_mutex \
    $(NRF5_SDK_ROOT)/components/toolchain/cmsis/include \
    $(NRF5_SDK_ROOT)/config/nrf52840/config \
    $(NRF5_SDK_ROOT)/external/fprintf \
    $(NRF5_SDK_ROOT)/external/freertos/config \
    $(NRF5_SDK_ROOT)/external/freertos/portable/CMSIS/nrf52 \
    $(NRF5_SDK_ROOT)/external/freertos/portable/GCC/nrf52 \
    $(NRF5_SDK_ROOT)/external/freertos/source/include \
    $(NRF5_SDK_ROOT)/external/segger_rtt \
    $(NRF5_SDK_ROOT)/integration/nrfx \
    $(NRF5_SDK_ROOT)/integration/nrfx/legacy \
    $(NRF5_SDK_ROOT)/modules/nrfx \
    $(NRF5_SDK_ROOT)/modules/nrfx/drivers/include \
    $(NRF5_SDK_ROOT)/modules/nrfx/hal \
    $(NRF5_SDK_ROOT)/modules/nrfx/mdk

DEFINES = \
    NRF52840_XXAA \
    BOARD_PCA10056 \
    BSP_DEFINES_ONLY \
    CONFIG_GPIO_AS_PINRESET \
    FLOAT_ABI_HARD \
    USE_APP_CONFIG \
    __HEAP_SIZE=40960 \
    __STACK_SIZE=8192 \
    SOFTDEVICE_PRESENT \
    PRINTF_DISABLE_SUPPORT_EXPONENTIAL

CFLAGS = \
    --specs=nano.specs

LDFLAGS = \
    --specs=nano.specs

ifdef DEVICE_FIRMWARE_REVISION
DEFINES += \
    WEAVE_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION=\"$(DEVICE_FIRMWARE_REVISION)\"
endif

# A product's firmware usually comes with two build "flavors":
# 1) a "development" build which typically enables debugging artifacts along with logging,
# and disables security and optimizations and
# 2) a "release" build which targets end-user devices
# and disables debugging artifacts and logging, and enables security and optimizations.
#
# To facilitate supporting these two build flavors, the sample app source code supports
# the build configuration option BUILD_RELEASE. By default, BUILD_RELEASE is not defined
# and a development build is produced. If make is invoked with BUILD_RELEASE=1, then
# a "pseudo-release" build is produced.
#
# We say "pseudo-release" build because the sample app is never to be used as-is
# as the firmware for an end-user device. However, since real products may start off with
# the sample app source code, we show how the BUILD_RELEASE build configuration option
# can be used to produce two build flavors (development and release) of the firmware.
#
# IMPORTANT: By no means does the use of BUILD_RELEASE in the sample app cover all key aspects that
# one should be concerned with (e.g. security, performance) when targeting real world end-user
# devices. This is simply provided for guidance on how to support two build flavors.

ifdef BUILD_RELEASE
    DEFINES += BUILD_RELEASE=1
    # Disable Thread logging.
    OPENTHREAD_DEFINES += OPENTHREAD_CONFIG_LOG_LEVEL=OT_LOG_LEVEL_NONE
else
    DEFINES += BUILD_RELEASE=0
    # Increase Tread logging level to INFO.
    OPENTHREAD_DEFINES += OPENTHREAD_CONFIG_LOG_LEVEL=OT_LOG_LEVEL_INFO
endif

OPENWEAVE_PROJECT_CONFIG = $(PROJECT_ROOT)/main/include/WeaveProjectConfig.h

OPENTHREAD_PROJECT_CONFIG = $(PROJECT_ROOT)/main/include/OpenThreadConfig.h

OPENTHREAD_CONFIGURE_OPTIONS += \
    --enable-child-supervision \
    --enable-jam-detection

LINKER_SCRIPT = $(PROJECT_ROOT)/main/ldscripts/openweave-nrf52840-lock-example.ld

$(call GenerateBuildRules)
