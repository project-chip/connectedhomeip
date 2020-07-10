#
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
#      CMake configuration for the Nordic nRF5 (nRF52840) platform.
#

include("${CHIP_PROJECT_SOURCE_DIR}/integrations/cmake/toolchains/arm-none-eabi.cmake")

set(NRF5_SDK_ROOT $ENV{NRF5_SDK_ROOT} CACHE PATH "Root of Nordic NRF5")

if(NOT DEFINED LINKER_SCRIPT)
    set(LINKER_SCRIPT "${CHIP_PROJECT_SOURCE_DIR}/examples/platform/nrf528xx/app/ldscripts/chip-nrf52840-example.ld")
    message("Warning: No linker script defined -- using default.")
endif(NOT DEFINED LINKER_SCRIPT)
message("Linker script: ${LINKER_SCRIPT}")

set(PLATFORM_MACH_FLAGS "-mcpu=cortex-m4 -mthumb -mabi=aapcs -mfpu=fpv4-sp-d16 -mfloat-abi=hard")
set(PLATFORM_MACH_FLAGS "${PLATFORM_MACH_FLAGS} -fno-common -ffunction-sections -fdata-sections")

set(CMAKE_C_FLAGS "${PLATFORM_MACH_FLAGS}" CACHE INTERNAL "C Compiler options")
set(CMAKE_CXX_FLAGS "${PLATFORM_MACH_FLAGS}" CACHE INTERNAL "C++ Compiler options")
set(CMAKE_ASM_FLAGS "${PLATFORM_MACH_FLAGS} -x assembler-with-cpp" CACHE INTERNAL "ASM Compiler options")
set(CMAKE_EXE_LINKER_FLAGS "${PLATFORM_MACH_FLAGS} -T${LINKER_SCRIPT} -L${NRF5_SDK_ROOT}/modules/nrfx/mdk" CACHE INTERNAL "Linker options")

set(PLATFORM_LINK_START "-Wl,--start-group")
set(PLATFORM_LINK_END "-Wl,--end-group -Wl,--gc-sections")

#
# Override build configurations with ones this platform supports
#

set(with_crypto mbedtls)
set(with_system lwip)

#
# Define chip-config library to provide defines and include paths for platform
#

add_library(chip-config INTERFACE)

target_compile_definitions(chip-config INTERFACE
    ${CHIP_PRIVATE_DEFINES}

    CHIP_TARGET_STYLE_EMBEDDED=1
    CHIP_TARGET_STYLE_UNIX=0
    CHIP_SYSTEM_CONFIG_USE_SOCKETS=0
    CHIP_SYSTEM_CONFIG_USE_LWIP=1
    CHIP_SYSTEM_CONFIG_TEST=1
    INET_CONFIG_ENABLE_TCP_ENDPOINT=1
    INET_CONFIG_ENABLE_UDP_ENDPOINT=1
    INET_CONFIG_ENABLE_RAW_ENDPOINT=1
    INET_CONFIG_ENABLE_TUN_ENDPOINT=0
    INET_CONFIG_ENABLE_DNS_RESOLVER=0
    INET_CONFIG_TEST=1
    CHIP_LOGGING_STYLE_EXTERNAL=1
    CHIP_CRYPTO_MBEDTLS=1
    CHIP_WITH_NLFAULTINJECTION=1
    CHIP_CONFIG_TEST=1
    CHIP_CONFIG_ENABLE_ARG_PARSER=1           # TestInet expects this
    HAVE_MALLOC=0
    HAVE_FREE=0

    CONFIG_NETWORK_LAYER_BLE=1
    CONFIG_NETWORK_LAYER_INET=1
    CHIP_ENABLE_OPENTHREAD=0

    CONFIG_DEVICE_LAYER=1
    CHIP_DEVICE_LAYER_TARGET_NRF5=1
    CHIP_DEVICE_LAYER_TARGET=nRF5
    BLE_PLATFORM_CONFIG_INCLUDE=<platform/nRF5/BlePlatformConfig.h>
    INET_PLATFORM_CONFIG_INCLUDE=<platform/nRF5/InetPlatformConfig.h>
    SYSTEM_PLATFORM_CONFIG_INCLUDE=<platform/nRF5/SystemPlatformConfig.h>
    CHIP_PLATFORM_CONFIG_INCLUDE=<platform/nRF5/CHIPPlatformConfig.h>
    CHIP_DEVICE_PLATFORM_CONFIG_INCLUDE=<platform/nRF5/CHIPDevicePlatformConfig.h>

    NRF52840_XXAA
    BOARD_PCA10056
    BSP_DEFINES_ONLY
    FLOAT_ABI_HARD
    SOFTDEVICE_PRESENT
    S140
    __HEAP_SIZE=40960
    __STACK_SIZE=8192
    CONFIG_GPIO_AS_PINRESET
    PRINTF_DISABLE_SUPPORT_EXPONENTIAL

#   USE_APP_CONFIG=1
    # NRF_MODULE_ENABLED
    NRF_CLOCK_ENABLED=1
    NRF_SDH_SOC_ENABLED=1
    NRF_SDH_ENABLED=1
    NRF_SDH_BLE_ENABLED=1
    NRF_BLE_GATT_ENABLED=1
    NRF_FSTORAGE_ENABLED=1
    FDS_ENABLED=1
    MEM_MANAGER_ENABLED=1

    NRF_LOG_BACKEND_RTT_ENABLED=1
    NRF_LOG_BACKEND_UART_ENABLED=0

    #MBEDTLS_CONFIG_FILE=\"nrf-config.h\"
    #MBEDTLS_USER_CONFIG_FILE=\"nrf52840-mbedtls-config.h\"
    #MBEDTLS_ERROR_C
    #MBEDTLS_CCM_C

    MBEDTLS_NO_PLATFORM_ENTROPY
    MBEDTLS_TEST_NULL_ENTROPY
    MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES
)

set(CHIP_TARGET_STYLE_EMBEDDED 1)

set(CONFIG_DEVICE_LAYER 1)
set(CHIP_CRYPTO_MBEDTLS 1)
set(CHIP_WITH_LWIP_INTERNAL 1)
set(CHIP_LWIP_TARGET_NRF5 1)

set(MBEDTLS_NO_PLATFORM_ENTROPY)
set(MBEDTLS_TEST_NULL_ENTROPY)
set(MBEDTLS_NO_DEFAULT_ENTROPY_SOURCES)

set(CHIP_PLATFORM_CONFIG_MBEDTLS_INCLUDE
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/config
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/mbedtls_plat_config
    ${NRF5_SDK_ROOT}/external/openthread/project/nrf52840

    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/include/
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/include/mbedtls
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/nrf_cc310_plat/include
)

set(CHIP_PLATFORM_CONFIG_FREERTOS_INCLUDE

    #${CHIP_PROJECT_SOURCE_DIR}/examples/lock-app/nrf5/main/include
    #${NRF5_SDK_ROOT}/examples/ble_peripheral/ble_app_hrs_freertos/config
    #${NRF5_SDK_ROOT}/external/freertos/config
    ${NRF5_SDK_ROOT}/examples/peripheral/blinky_rtc_freertos/config
    #${NRF5_SDK_ROOT}/examples/multiprotocol/ble_thread/ble_thread_dyn_hrs_coap_srv_freertos/config
    #${NRF5_SDK_ROOT}/examples/multiprotocol/ble_thread/ble_thread_dyn_hrs_coap_srv_freertos/pca10056/s140/config
)

set(NRF5_SDK_INCLUDES
    # BSP portions for basic cstartup and FreeRTOS port 
    ${NRF5_SDK_ROOT}/config/nrf52840/config
    ${NRF5_SDK_ROOT}/components/toolchain/cmsis/include

    ${NRF5_SDK_ROOT}/integration/nrfx
    ${NRF5_SDK_ROOT}/integration/nrfx/legacy
    ${NRF5_SDK_ROOT}/modules/nrfx
    ${NRF5_SDK_ROOT}/modules/nrfx/hal
    ${NRF5_SDK_ROOT}/modules/nrfx/mdk
    ${NRF5_SDK_ROOT}/modules/nrfx/drivers/include
    ${NRF5_SDK_ROOT}/modules/nrfx/templates/nRF52840
    ${NRF5_SDK_ROOT}/components/libraries/balloc
    ${NRF5_SDK_ROOT}/components/libraries/log
    ${NRF5_SDK_ROOT}/components/libraries/log/src
    ${NRF5_SDK_ROOT}/components/libraries/memobj
    ${NRF5_SDK_ROOT}/components/libraries/strerror
    ${NRF5_SDK_ROOT}/components/libraries/util
    ${NRF5_SDK_ROOT}/components/libraries/experimental_section_vars

    # Softdevice
    ${NRF5_SDK_ROOT}/components/softdevice/common
    ${NRF5_SDK_ROOT}/components/softdevice/s140/headers
    ${NRF5_SDK_ROOT}/components/softdevice/s140/headers/nrf52
    # ${NRF5_SDK_ROOT}/components/softdevice/mbr/nrf52840/headers

    # FDS
    ${NRF5_SDK_ROOT}/components/libraries/atomic
    ${NRF5_SDK_ROOT}/components/libraries/atomic_fifo
    ${NRF5_SDK_ROOT}/components/libraries/fstorage

    # FreeRTOS core porting headers
    ${CHIP_PLATFORM_CONFIG_FREERTOS_INCLUDE}
    ${NRF5_SDK_ROOT}/external/freertos/portable/CMSIS/nrf52
    ${NRF5_SDK_ROOT}/external/freertos/portable/GCC/nrf52
    ${NRF5_SDK_ROOT}/external/freertos/source/include
#    ${CHIP_PROJECT_SOURCE_DIR}/config/nrf5

    # Required by DeviceLayer Thread integration
    ${NRF5_SDK_ROOT}/external/openthread/include
    ${NRF5_SDK_ROOT}/components/boards
    #${NRF5_SDK_ROOT}/external/fprintf
)

set(NRF5_SDK_SOURCES
    # Minimal cstartup
    ${NRF5_SDK_ROOT}/modules/nrfx/mdk/system_nrf52840.c
    ${NRF5_SDK_ROOT}/modules/nrfx/mdk/gcc_startup_nrf52840.S

    # Bsp portion portion of FreeRTOS porting layer
    ${NRF5_SDK_ROOT}/modules/nrfx/drivers/src/nrfx_clock.c
    ${NRF5_SDK_ROOT}/modules/nrfx/drivers/src/nrfx_power.c
    ${NRF5_SDK_ROOT}/integration/nrfx/legacy/nrf_drv_clock.c

    # Application common utils
    ${NRF5_SDK_ROOT}/components/libraries/util/app_error.c
    ${NRF5_SDK_ROOT}/components/libraries/util/app_error_handler_gcc.c
    ${NRF5_SDK_ROOT}/components/libraries/util/app_error_weak.c
    ${NRF5_SDK_ROOT}/components/libraries/util/app_util_platform.c

    # Log
    ${NRF5_SDK_ROOT}/components/libraries/log/src/nrf_log_backend_rtt.c
    ${NRF5_SDK_ROOT}/components/libraries/log/src/nrf_log_backend_serial.c
    ${NRF5_SDK_ROOT}/components/libraries/log/src/nrf_log_default_backends.c
    ${NRF5_SDK_ROOT}/components/libraries/log/src/nrf_log_frontend.c
    ${NRF5_SDK_ROOT}/components/libraries/log/src/nrf_log_str_formatter.c
    ${NRF5_SDK_ROOT}/components/libraries/experimental_section_vars/nrf_section_iter.c

    # File system
    ${NRF5_SDK_ROOT}/components/libraries/fds/fds.c
    ${NRF5_SDK_ROOT}/components/libraries/mem_manager/mem_manager.c
    ${NRF5_SDK_ROOT}/components/libraries/fstorage/nrf_fstorage.c
    ${NRF5_SDK_ROOT}/components/libraries/atomic//nrf_atomic.c
    ${NRF5_SDK_ROOT}/components/libraries/atomic_fifo/nrf_atfifo.c
    ${NRF5_SDK_ROOT}/components/libraries/fstorage/nrf_fstorage_sd.c

    # BLE
    ${NRF5_SDK_ROOT}/components/ble/common/ble_advdata.c
    ${NRF5_SDK_ROOT}/components/ble/common/ble_srv_common.c
    ${NRF5_SDK_ROOT}/components/ble/nrf_ble_gatt/nrf_ble_gatt.c

    # Softdevice
    ${NRF5_SDK_ROOT}/components/softdevice/common/nrf_sdh.c
    ${NRF5_SDK_ROOT}/components/softdevice/common/nrf_sdh_ble.c
    ${NRF5_SDK_ROOT}/components/softdevice/common/nrf_sdh_soc.c

    # SEGGER
    #${NRF5_SDK_ROOT}/external/segger_rtt/SEGGER_RTT.c
    #${NRF5_SDK_ROOT}/external/segger_rtt/SEGGER_RTT_printf.c
    #${NRF5_SDK_ROOT}/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c

    #${NRF5_SDK_ROOT}/external/openthread/nrf_security/nrf_cc310_plat/src/nrf_cc310_platform_mutex_freertos.c
    #${NRF5_SDK_ROOT}/external/openthread/nrf_security/nrf_cc310_plat/src/nrf_cc310_platform_abort_freertos.c
)

target_include_directories(chip-config INTERFACE
    # FreeRTOS core porting headers
    ${CHIP_PLATFORM_CONFIG_FREERTOS_INCLUDE}

    ${NRF5_SDK_INCLUDES}

    # CHIP-specific configuration headers for platform
    ${CHIP_PROJECT_SOURCE_DIR}/third_party/nlassert/repo/include
    ${CHIP_PROJECT_SOURCE_DIR}/third_party/nlio/repo/include

    # LWIP
    ${CHIP_PROJECT_SOURCE_DIR}/src/lwip/nrf5
    ${CHIP_PROJECT_SOURCE_DIR}/src/lwip/freertos
    ${CHIP_PROJECT_SOURCE_DIR}/third_party/lwip/repo/lwip/src/include

    # OpenThread
    ${CHIP_PROJECT_SOURCE_DIR}/third_party/openthread/repo/include
)

#
# Define PlatformOs library to provide canned build of FreeRTOS and platform-specific port
#

add_library(PlatformOs
    # FreeRTOS port
    ${NRF5_SDK_ROOT}/external/freertos/portable/GCC/nrf52/port.c
    ${NRF5_SDK_ROOT}/external/freertos/portable/CMSIS/nrf52/port_cmsis.c
    ${NRF5_SDK_ROOT}/external/freertos/portable/CMSIS/nrf52/port_cmsis_systick.c

    # FreeRTOS generic
    ${NRF5_SDK_ROOT}/external/freertos/source/croutine.c
    ${NRF5_SDK_ROOT}/external/freertos/source/event_groups.c
    ${NRF5_SDK_ROOT}/external/freertos/source/list.c
    ${NRF5_SDK_ROOT}/external/freertos/source/portable/MemMang/heap_3.c
    ${NRF5_SDK_ROOT}/external/freertos/source/queue.c
    ${NRF5_SDK_ROOT}/external/freertos/source/stream_buffer.c
    ${NRF5_SDK_ROOT}/external/freertos/source/tasks.c
    ${NRF5_SDK_ROOT}/external/freertos/source/timers.c
)

target_link_libraries(PlatformOs PRIVATE chip-config)

#
# Define PlatformBsp library to provide cstartup for executables.
#

add_library(PlatformBsp
    ${NRF5_SDK_SOURCES}
)

target_link_libraries(PlatformBsp PUBLIC chip-config)

set(CHIP_PLATFORM_BSP_LIBS
    ${NRF5_SDK_ROOT}/external/openthread/lib/nrf52840/gcc/libopenthread-cli-ftd.a
    ${NRF5_SDK_ROOT}/external/openthread/lib/nrf52840/gcc/libopenthread-ftd.a
    ${NRF5_SDK_ROOT}/external/openthread/lib/nrf52840/gcc/libopenthread-platform-utils.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_glue.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_glue_cc310.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_glue_vanilla.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_cc310_backend.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_vanilla_backend.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedtls_base_vanilla.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedtls_tls_vanilla.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedtls_x509_vanilla.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libnrf_cc310_platform_0.9.1.a
    ${NRF5_SDK_ROOT}/external/openthread/lib/nrf52840/gcc/libopenthread-nrf52840-softdevice-sdk.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_glue.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_glue_cc310.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_glue_vanilla.a
    ${NRF5_SDK_ROOT}/external/openthread/lib/nrf52840/gcc/libnordicsemi-nrf52840-radio-driver-softdevice.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_cc310_backend.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedcrypto_vanilla_backend.a
    ${NRF5_SDK_ROOT}/external/openthread/lib/nrf52840/gcc/libopenthread-platform-utils.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedtls_base_vanilla.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedtls_tls_vanilla.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libmbedtls_x509_vanilla.a
    ${NRF5_SDK_ROOT}/external/openthread/nrf_security/lib/libnrf_cc310_platform_0.9.1.a
    ${NRF5_SDK_ROOT}/external/openthread/lib/nrf52840/gcc/libopenthread-ftd.a
)


set(CHIP_PLATFORM_LIBS_PACKAGE
    mbedtls
    lwip
    lwipTarget
    DeviceLayer
    PlatformOs
    PlatformBsp
)

set(CHIP_PLATFORM_LIBS
    ${CHIP_PLATFORM_LIBS_PACKAGE}
    c_nano
    --specs=nano.specs
    --specs=nosys.specs
    chip-config
)
