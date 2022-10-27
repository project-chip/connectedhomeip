#-------------------------------------------------------------------------------
# Copyright (c) 2020-2022, Arm Limited. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

cmake_policy(SET CMP0076 NEW)
set(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR})

#========================= Platform region defs ===============================#

target_include_directories(platform_region_defs
    INTERFACE
        partition
)

#========================= Platform common defs ===============================#

if (${CMAKE_C_COMPILER_ID} STREQUAL ARMClang)
    if (${CMAKE_C_COMPILER_VERSION} VERSION_LESS "6.14")
        message(FATAL_ERROR "CPU (Cortex-M55) is only supported in ARMCLANG version 6.14 or newer.")
    endif()
endif()

# Specify the location of platform specific build dependencies.
target_sources(tfm_s
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/device/source/startup_an552_s.c
)
target_add_scatter_file(tfm_s
    $<$<C_COMPILER_ID:ARMClang>:${CMAKE_SOURCE_DIR}/platform/ext/common/armclang/tfm_common_s.sct>
    $<$<C_COMPILER_ID:GNU>:${CMAKE_SOURCE_DIR}/platform/ext/common/gcc/tfm_common_s.ld>
    $<$<C_COMPILER_ID:IAR>:${CMAKE_SOURCE_DIR}/platform/ext/common/iar/tfm_common_s.icf>
)

if(NS)
    target_sources(tfm_ns
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/device/source/startup_an552_ns.c
    )
    target_add_scatter_file(tfm_ns
        $<$<C_COMPILER_ID:ARMClang>:${CMAKE_CURRENT_SOURCE_DIR}/device/source/armclang/an552_ns.sct>
        $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/device/source/gcc/an552_ns.ld>
        $<$<C_COMPILER_ID:IAR>:${CMAKE_CURRENT_SOURCE_DIR}/device/source/iar/an552_ns.icf>
    )
    target_link_libraries(CMSIS_5_tfm_ns
        INTERFACE
            $<$<C_COMPILER_ID:ARMClang>:CMSIS_5_RTX_V8MMN>
            $<$<AND:$<C_COMPILER_ID:GNU>,$<STREQUAL:${CONFIG_TFM_FP},hard>>:CMSIS_5_RTX_V8MMFN>
            $<$<AND:$<C_COMPILER_ID:GNU>,$<STREQUAL:${CONFIG_TFM_FP},soft>>:CMSIS_5_RTX_V8MMN>
            $<$<C_COMPILER_ID:IAR>:CMSIS_5_RTX_V81MMN>
    )
    target_compile_options(tfm_ns
        PUBLIC
            ${COMPILER_CP_FLAG}
    )
    target_link_options(tfm_ns
        PUBLIC
            ${LINKER_CP_OPTION}
    )
endif()

if(BL2)
    target_sources(bl2
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/device/source/startup_an552_bl2.c
    )
    target_add_scatter_file(bl2
            $<$<C_COMPILER_ID:ARMClang>:${CMAKE_CURRENT_SOURCE_DIR}/device/source/armclang/an552_bl2.sct>
            $<$<C_COMPILER_ID:GNU>:${CMAKE_CURRENT_SOURCE_DIR}/device/source/gcc/an552_bl2.ld>
            $<$<C_COMPILER_ID:IAR>:${CMAKE_CURRENT_SOURCE_DIR}/device/source/iar/an552_bl2.icf>
    )
endif()

#========================= Platform Secure ====================================#

target_include_directories(platform_s
    PUBLIC
        .
        ../common
        cmsis_drivers
        cmsis_drivers/config
        device
        device/config
        device/include
        device/source/armclang
        native_drivers
        partition
        services/src
        ${PLATFORM_DIR}/..
)

target_sources(platform_s
    PRIVATE
        cmsis_drivers/Driver_Flash.c
        cmsis_drivers/Driver_AN552_MPC.c
        cmsis_drivers/Driver_SSE300_PPC.c
        cmsis_drivers/Driver_USART.c
        device/source/device_definition.c
        device/source/system_core_init.c
        native_drivers/mpc_sie_drv.c
        native_drivers/mpu_armv8m_drv.c
        native_drivers/ppc_sse300_drv.c
        native_drivers/emulated_flash_drv.c
        native_drivers/syscounter_armv8-m_cntrl_drv.c
        native_drivers/uart_cmsdk_drv.c
        spm_hal.c
        $<$<AND:$<BOOL:${TFM_PSA_API}>,$<NOT:$<BOOL:${TEST_NS_FPU}>>>:${CMAKE_CURRENT_SOURCE_DIR}/tfm_interrupts.c>
        target_cfg.c
        tfm_peripherals_def.c
        tfm_hal_isolation.c
        tfm_hal_platform.c
        ${CMAKE_SOURCE_DIR}/platform/ext/common/tfm_hal_nvic.c
        ${CMAKE_SOURCE_DIR}/platform/ext/common/tfm_hal_isolation_mpu_v8m.c
        $<$<OR:$<BOOL:${TEST_NS_SLIH_IRQ}>,$<BOOL:${TEST_NS_FLIH_IRQ}>>:${CMAKE_CURRENT_SOURCE_DIR}/plat_test.c>
        $<$<BOOL:${TFM_PARTITION_PLATFORM}>:${CMAKE_CURRENT_SOURCE_DIR}/services/src/tfm_platform_system.c>
)

target_sources(tfm_sprt
    PRIVATE
        # SLIH test Partition and FLIH test Partition access the timer as ARoT Partitions.
        # Put the driver to SPRT so that both SLIH and FLIH tests can access it.
        $<$<OR:$<BOOL:${TEST_NS_SLIH_IRQ}>,$<BOOL:${TEST_NS_FLIH_IRQ}>>:${CMAKE_CURRENT_SOURCE_DIR}/native_drivers/systimer_armv8-m_drv.c>

        # FPU client test partition access the timer as ARoT Partitions.
        # Put the driver to SPRT so that FPU tests can access it.
        $<$<BOOL:${TEST_NS_FPU}>:${CMAKE_CURRENT_SOURCE_DIR}/native_drivers/systimer_armv8-m_drv.c>
        $<$<BOOL:${TEST_NS_FPU}>:${CMAKE_CURRENT_SOURCE_DIR}/plat_test.c>
)

# Adding new defined handler function to the library where the weak handler
# funtion symbol are, to override weak handler funtion.
# Otherwise the weak handler funtion cannot be replaced by the new defined
# hanlder function which symbol is in another library.
target_sources(tfm_s
    PRIVATE
        $<$<BOOL:${TEST_NS_FPU}>:${CMAKE_CURRENT_SOURCE_DIR}/tfm_interrupts.c>
)

target_link_libraries(platform_s
    PRIVATE
        tfm_utilities
)

target_compile_options(platform_s
    PUBLIC
        ${COMPILER_CMSE_FLAG}
)

# To configure S and NS timer in S side for FP interrupt test
target_compile_definitions(platform_s
    PUBLIC
        $<$<BOOL:${TEST_NS_FPU}>:TEST_NS_FPU>
)

#========================= Platform Non-Secure ================================#

target_sources(platform_ns
    PRIVATE
        cmsis_drivers/Driver_Flash.c
        cmsis_drivers/Driver_USART.c
        device/source/device_definition.c
        device/source/system_core_init.c
        native_drivers/uart_cmsdk_drv.c
        native_drivers/systimer_armv8-m_drv.c
)

target_include_directories(platform_ns
    PUBLIC
        .
        ../common
        ${PLATFORM_DIR}/..
        cmsis_drivers
        cmsis_drivers/config
        device
        device/config
        device/include
        device/source/armclang
        native_drivers
        partition
)

# Adding new defined handler function to the library where the weak handler
# funtion symbol are, to override weak handler funtion.
# Otherwise the weak handler funtion cannot be replaced by the new defined
# hanlder function which symbol is in another library.
target_sources(tfm_ns
    PRIVATE
        $<$<BOOL:${TEST_NS_FPU}>:${CMAKE_CURRENT_SOURCE_DIR}/ns_fp_test_interrupt.c>
)

#========================= Platform BL2 =======================================#

if(BL2)
    target_sources(platform_bl2
        PRIVATE
            cmsis_drivers/Driver_Flash.c
            cmsis_drivers/Driver_USART.c
            device/source/device_definition.c
            device/source/system_core_init.c
            native_drivers/uart_cmsdk_drv.c
            native_drivers/emulated_flash_drv.c
            boot_hal_bl2.c
    )

    target_include_directories(platform_bl2
        PUBLIC
            cmsis_drivers
            cmsis_drivers/config
            device
            device/config
            device/include
            device/source/armclang
            native_drivers
            partition
            services/src

        PRIVATE
            .
            ${PLATFORM_DIR}/..
            native_drivers
    )
endif()
