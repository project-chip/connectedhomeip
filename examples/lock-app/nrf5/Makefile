#
#
#   Copyright (c) 2020 Project CHIP Authors
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
#         Makefile for building the CHIP nRF52840 Lock Example Application.
#

PROJECT_ROOT := $(realpath .)

CHIP_ROOT ?= $(realpath $(PROJECT_ROOT)/third_party/connectedhomeip)
BUILD_SUPPORT_DIR = $(CHIP_ROOT)/config/nrf5
NRF5_PLATFORM_DIR = $(CHIP_ROOT)/examples/platform/nrf528xx

include $(BUILD_SUPPORT_DIR)/nrf5-app.mk
include $(BUILD_SUPPORT_DIR)/nrf5-chip.mk

APP = chip-nrf52840-lock-example

SRCS = \
    $(PROJECT_ROOT)/main/main.cpp \
    $(PROJECT_ROOT)/main/AppTask.cpp \
    $(PROJECT_ROOT)/main/BoltLockManager.cpp \
    $(PROJECT_ROOT)/main/DataModelHandler.cpp \
    $(PROJECT_ROOT)/main/gen/call-command-handler.c \
    $(PROJECT_ROOT)/main/gen/callback-stub.c \
    $(PROJECT_ROOT)/main/gen/znet-bookkeeping.c \
    $(CHIP_ROOT)/src/app/util/attribute-size.c \
    $(CHIP_ROOT)/src/app/util/attribute-storage.c \
    $(CHIP_ROOT)/src/app/util/attribute-table.c \
    $(CHIP_ROOT)/src/app/util/chip-response.cpp \
    $(CHIP_ROOT)/src/app/util/client-api.c \
    $(CHIP_ROOT)/src/app/util/ember-print.cpp \
    $(CHIP_ROOT)/src/app/util/message.c \
    $(CHIP_ROOT)/src/app/util/process-cluster-message.c \
    $(CHIP_ROOT)/src/app/util/process-global-message.c \
    $(CHIP_ROOT)/src/app/util/util.c \
    $(CHIP_ROOT)/src/app/clusters/on-off-server/on-off.c \
    $(NRF5_PLATFORM_DIR)/app/support/CXXExceptionStubs.cpp \
    $(NRF5_PLATFORM_DIR)/app/support/nRF5Sbrk.c \
    $(NRF5_PLATFORM_DIR)/app/support/FreeRTOSNewlibLockSupport.c \
    $(NRF5_PLATFORM_DIR)/app/support/FreeRTOSNewlibLockSupport_test.c \
    $(NRF5_PLATFORM_DIR)/app/Server.cpp \
    $(NRF5_PLATFORM_DIR)/app/chipinit.cpp \
    $(NRF5_PLATFORM_DIR)/util/LEDWidget.cpp \
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
    $(NRF5_SDK_ROOT)/examples/multiprotocol/app_utils/multiprotocol_802154_config.c \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/nrf_cc310_plat/src/nrf_cc310_platform_mutex_freertos.c \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/nrf_cc310_plat/src/nrf_cc310_platform_abort_freertos.c \
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
    $(CHIP_OUTPUT_DIR)/third_party/openthread/include \
    $(CHIP_ROOT)/examples/platform \
    $(CHIP_ROOT)/src/include/ \
    $(CHIP_ROOT)/src/lib \
    $(CHIP_ROOT)/src/ \
    $(CHIP_ROOT)/src/app/util \
    $(CHIP_ROOT)/src/system \
    $(CHIP_ROOT)/third_party/openthread/repo/include \
    $(NRF5_PLATFORM_DIR)/app/include \
    $(NRF5_PLATFORM_DIR)/app/project_include \
    $(NRF5_PLATFORM_DIR)/util/include \
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
    $(NRF5_SDK_ROOT)/components/softdevice/s140/headers/nrf52 \
    $(NRF5_SDK_ROOT)/components/softdevice/mbr/nrf52840/headers \
    $(NRF5_SDK_ROOT)/components/toolchain/cmsis/include \
    $(NRF5_SDK_ROOT)/config/nrf52840/config \
    $(NRF5_SDK_ROOT)/examples/multiprotocol/app_utils \
    $(NRF5_SDK_ROOT)/external/fprintf \
    $(NRF5_SDK_ROOT)/external/freertos/config \
    $(NRF5_SDK_ROOT)/external/freertos/portable/CMSIS/nrf52 \
    $(NRF5_SDK_ROOT)/external/freertos/portable/GCC/nrf52 \
    $(NRF5_SDK_ROOT)/external/freertos/source/include \
    $(NRF5_SDK_ROOT)/external/nRF-IEEE-802.15.4-radio-driver/src \
    $(NRF5_SDK_ROOT)/external/nRF-IEEE-802.15.4-radio-driver/src/fem \
    $(NRF5_SDK_ROOT)/external/nRF-IEEE-802.15.4-radio-driver/src/fem/three_pin_gpio \
    $(NRF5_SDK_ROOT)/external/nRF-IEEE-802.15.4-radio-driver/src/rsch/raal \
    $(NRF5_SDK_ROOT)/external/nRF-IEEE-802.15.4-radio-driver/src/rsch/raal/softdevice \
    $(NRF5_SDK_ROOT)/external/openthread/include \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/config \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/config \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/include/ \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/include/mbedtls \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/mbedtls_plat_config \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/nrf_cc310_plat/include \
    $(NRF5_SDK_ROOT)/external/openthread/project/config \
    $(NRF5_SDK_ROOT)/external/openthread/project/nrf52840 \
    $(NRF5_SDK_ROOT)/external/segger_rtt \
    $(NRF5_SDK_ROOT)/integration/nrfx \
    $(NRF5_SDK_ROOT)/integration/nrfx/legacy \
    $(NRF5_SDK_ROOT)/modules/nrfx \
    $(NRF5_SDK_ROOT)/modules/nrfx/drivers/include \
    $(NRF5_SDK_ROOT)/modules/nrfx/hal \
    $(NRF5_SDK_ROOT)/modules/nrfx/mdk \

DEFINES = \
    BOARD_PCA10056 \
    BSP_DEFINES_ONLY \
    CHIP_ENABLE_OPENTHREAD=1 \
    CONFIG_GPIO_AS_PINRESET \
    ENABLE_FEM \
    FLOAT_ABI_HARD \
    FREERTOS \
    MBEDTLS_CONFIG_FILE=\"nrf-config.h\" \
    MBEDTLS_USER_CONFIG_FILE=\"nrf52840-mbedtls-config.h\" \
    MULTIPROTOCOL_802154_CONFIG_PRESENT \
    NRF52840_XXAA \
    NRFX_PRS_ENABLED=0 \
    NRF_SD_BLE_API_VERSION=7 \
    PRINTF_DISABLE_SUPPORT_EXPONENTIAL \
    S140 \
    SOFTDEVICE_PRESENT \
    UART0_ENABLED=0 \
    UART1_ENABLED=1 \
    USE_APP_CONFIG \
    __HEAP_SIZE=40960 \
    __STACK_SIZE=8192 \

LIBS = \
    -lopenthread-cli-ftd \
    -lopenthread-ftd \
    -lopenthread-platform-utils \
    -lopenthread-nrf52840-transport \
    -lopenthread-nrf52840-softdevice-sdk \
    -lnordicsemi-nrf52840-radio-driver-softdevice \
    -lSetupPayload \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/lib/libmbedcrypto_glue.a \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/lib/libmbedcrypto_glue_cc310.a \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/lib/libmbedcrypto_glue_vanilla.a \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/lib/libmbedcrypto_cc310_backend.a \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/lib/libmbedcrypto_vanilla_backend.a \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/lib/libmbedtls_base_vanilla.a \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/lib/libmbedtls_tls_vanilla.a \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/lib/libmbedtls_x509_vanilla.a \
    $(NRF5_SDK_ROOT)/external/openthread/nrf_security/lib/libnrf_cc310_platform_0.9.1.a \

CFLAGS = \
    --specs=nano.specs

LDFLAGS = \
    --specs=nano.specs

ifdef DEVICE_FIRMWARE_REVISION
DEFINES += \
    CHIP_DEVICE_CONFIG_DEVICE_FIRMWARE_REVISION=\"$(DEVICE_FIRMWARE_REVISION)\"
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
    CHIP_DEFINES += CHIP_CONFIG_LOG_LEVEL=OT_LOG_LEVEL_NONE
else
    DEFINES += BUILD_RELEASE=0
    # Increase Tread logging level to INFO.
    CHIP_DEFINES += CHIP_CONFIG_LOG_LEVEL=OT_LOG_LEVEL_INFO
endif

CHIP_PROJECT_CONFIG = $(NRF5_PLATFORM_DIR)/app/project_include/CHIPProjectConfig.h

LINKER_SCRIPT = $(NRF5_PLATFORM_DIR)/app/ldscripts/chip-nrf52840-example.ld

$(call GenerateBuildRules)
