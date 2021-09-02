/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "AppConfig.h"
#include <lib/support/CHIPPlatformMemory.h>
#include <platform/CHIPDeviceLayer.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <assert.h>
#include <string.h>

#include <mbedtls/platform.h>

#if CHIP_ENABLE_OPENTHREAD
#include <openthread-core-config.h>
#include <openthread/cli.h>
#include <openthread/config.h>
#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/heap.h>
#include <openthread/icmp6.h>
#include <openthread/instance.h>
#include <openthread/link.h>
#include <openthread/platform/openthread-system.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>
#include <utils/uart.h>

#include "platform-efr32.h"

#if OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
#include "openthread/heap.h"
#endif // OPENTHREAD_CONFIG_HEAP_EXTERNAL_ENABLE
#endif // CHIP_ENABLE_OPENTHREAD

#include "bsp.h"
#include "bsp_init.h"
#include "dmadrv.h"
#include "ecode.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_core.h"
#include "em_device.h"
#include "em_emu.h"
#include "em_system.h"
#include "hal-config.h"
#include "hal_common.h"
#include "init_efrPlatform.h"
#include "rail.h"
#include "sl_mpu.h"
#include "sl_sleeptimer.h"

#if (HAL_FEM_ENABLE)
#include "fem-control.h"
#endif

#if DISPLAY_ENABLED
#include "lcd.h"
#endif

static void halInitChipSpecific(void);
void initAntenna(void);

void init_efrPlatform(void)
{
    sl_status_t status;
    uint32_t PrioGoupPos = 0;
    uint32_t SubPrio     = 0;
#if defined(EFR32MG21)
    // FreeRTOS recommends a lower configuration for CortexM3 and asserts
    // in some places if using default PRIGROUP_POSITION.
    SubPrio = 1;
#endif

    __disable_irq();

#undef FIXED_EXCEPTION
#define FIXED_EXCEPTION(vectorNumber, functionName, deviceIrqn, deviceIrqHandler)
#define EXCEPTION(vectorNumber, functionName, deviceIrqn, deviceIrqHandler, priorityLevel, subpriority)                            \
    PrioGoupPos = PRIGROUP_POSITION - SubPrio;                                                                                     \
    NVIC_SetPriority(deviceIrqn, NVIC_EncodePriority(PrioGoupPos, priorityLevel, subpriority));
#include NVIC_CONFIG
#undef EXCEPTION

    NVIC_SetPriorityGrouping(PrioGoupPos);
    CHIP_Init();
    halInitChipSpecific();
#if DISPLAY_ENABLED
    initLCD();
#endif
    BSP_Init(BSP_INIT_BCC);

    // Enable LE peripheral clock. Needed for RTCC initialization in sl_sleeptimer_init()
#if !defined(_SILICON_LABS_32B_SERIES_2)
    CMU_ClockEnable(cmuClock_HFLE, true);
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
#endif // !defined(_SILICON_LABS_32B_SERIES_2)

    status = sl_sleeptimer_init();
    assert(status == SL_STATUS_OK);

#if (HAL_FEM_ENABLE)
    initFem();
    wakeupFem();
#endif

    __enable_irq();

#if EFR32_LOG_ENABLED
    efr32LogInit();
#endif

#if CHIP_ENABLE_OPENTHREAD
    efr32RadioInit();
    efr32AlarmInit();
    efr32MiscInit();
#endif // CHIP_ENABLE_OPENTHREAD
}

void halInitChipSpecific(void)
{
#if defined(BSP_DK) && !defined(RAIL_IC_SIM_BUILD)
    BSP_Init(BSP_INIT_DK_SPI);
#endif
    BSP_initDevice();

#if !defined(RAIL_IC_SIM_BUILD)
    BSP_initBoard();
#endif

#if HAL_PTI_ENABLE
    RAIL_PtiConfig_t railPtiConfig = {
#if HAL_PTI_MODE == HAL_PTI_MODE_SPI
        .mode = RAIL_PTI_MODE_SPI,
#elif HAL_PTI_MODE == HAL_PTI_MODE_UART
        .mode = RAIL_PTI_MODE_UART,
#elif HAL_PTI_MODE == HAL_PTI_MODE_UART_ONEWIRE
        .mode = RAIL_PTI_MODE_UART_ONEWIRE,
#else
        .mode = RAIL_PTI_MODE_DISABLED,
#endif
        .baud = HAL_PTI_BAUD_RATE,
#ifdef BSP_PTI_DOUT_LOC
        .doutLoc = BSP_PTI_DOUT_LOC,
#endif
        .doutPort = (uint8_t) BSP_PTI_DOUT_PORT,
        .doutPin  = BSP_PTI_DOUT_PIN,
#if HAL_PTI_MODE == HAL_PTI_MODE_SPI
#ifdef BSP_PTI_DCLK_LOC
        .dclkLoc = BSP_PTI_DCLK_LOC,
#endif
        .dclkPort = (uint8_t) BSP_PTI_DCLK_PORT,
        .dclkPin  = BSP_PTI_DCLK_PIN,
#endif
#if HAL_PTI_MODE != HAL_PTI_MODE_UART_ONEWIRE
#ifdef BSP_PTI_DFRAME_LOC
        .dframeLoc = BSP_PTI_DFRAME_LOC,
#endif
        .dframePort = (uint8_t) BSP_PTI_DFRAME_PORT,
        .dframePin  = BSP_PTI_DFRAME_PIN
#endif
    };

    RAIL_ConfigPti(RAIL_EFR32_HANDLE, &railPtiConfig);
#endif // HAL_PTI_ENABLE

#if !defined(RAIL_IC_SIM_BUILD)
#if CHIP_ENABLE_OPENTHREAD
    initAntenna();
#endif // CHIP_ENABLE_OPENTHREAD

// Disable any unused peripherals to ensure we enter a low power mode
#if defined(BSP_EXTFLASH_USART) && !defined(HAL_DISABLE_EXTFLASH)
#include "mx25flash_spi.h"
    MX25_init();
    MX25_DP();
#endif

#endif // RAIL_IC_SIM_BUILD

#if defined(HAL_VCOM_ENABLE)
    // Enable VCOM if requested
    GPIO_PinModeSet(BSP_VCOM_ENABLE_PORT, BSP_VCOM_ENABLE_PIN, gpioModePushPull, HAL_VCOM_ENABLE);
#endif // HAL_VCOM_ENABLE

#if RAIL_DMA_CHANNEL == DMA_CHANNEL_DMADRV
    Ecode_t dmaError = DMADRV_Init();
    if ((dmaError == ECODE_EMDRV_DMADRV_ALREADY_INITIALIZED) || (dmaError == ECODE_EMDRV_DMADRV_OK))
    {
        unsigned int channel;
        dmaError = DMADRV_AllocateChannel(&channel, NULL);
        if (dmaError == ECODE_EMDRV_DMADRV_OK)
        {
            RAIL_UseDma(channel);
        }
    }
#elif defined(RAIL_DMA_CHANNEL) && (RAIL_DMA_CHANNEL != DMA_CHANNEL_INVALID)
    LDMA_Init_t ldmaInit = LDMA_INIT_DEFAULT;
    LDMA_Init(&ldmaInit);
    RAIL_UseDma(RAIL_DMA_CHANNEL);
#endif
}

#ifdef __cplusplus
}
#endif
