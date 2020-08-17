/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
/*******************************************************************************
 * @file
 * @brief init_mcu.c
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#if defined(HAL_CONFIG)
#include "bsphalconfig.h"
#include "hal-config.h"
#else
#include "bspconfig.h"
#endif

#include "board_features.h"

#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_rtcc.h"

#include "FreeRTOSConfig.h"
#include "bsp.h"
#include "init_mcu.h"

// Bit [19] in MODULEINFO is the HFXOCALVAL:
// 1=No factory cal, use default XO tunning value in FW
// 0=Factory Cal, use XO tunning value in DI
#define DEVINFO_MODULEINFO_HFXOCALVAL_MASK 0x00080000UL
// Calibration value for HFXO CTUNE is at DEVINFO Offset 0x08
#define DEVINFO_MODULEINFO_CRYSTALOSCCALVAL (*((uint16_t *) (uint32_t)(DEVINFO_BASE + 0x8UL)))
// [15:9] : (LFXOTUNING) Calibration for LFXO TUNING
// [8:0]  : (HFXOCTUNE) Calibration for HFXO CTUNE
#define DEVINFO_HFXOCTUNE_MASK 0x01FFUL

#define set_HFXO_CTUNE(val)                                                                                                        \
    do                                                                                                                             \
    {                                                                                                                              \
        hfxoInit.ctuneSteadyState = (val);                                                                                         \
    } while (0)

static void initMcu_clocks(void);
static void initHFXO(void);

void initMcu(void)
{
    // ISR safe FreeRTOS API functions must *only* be called
    // from interrupts that have been assigned a priority at or below
    // configMAX_SYSCALL_INTERRUPT_PRIORITY.
    // Here we init all IRQ prio to configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
    // to make sure no IRQ use default priority of zero as that is the highest possible priority
    for (IRQn_Type i = 0; i < EXT_IRQ_COUNT; i++)
    {
        NVIC_SetPriority(i, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY);
    }
    // Device errata
    CHIP_Init();

    // Set up DC-DC converter
    EMU_DCDCInit_TypeDef dcdcInit = BSP_DCDC_INIT;
#if HAL_DCDC_BYPASS
    dcdcInit.dcdcMode = emuDcdcMode_Bypass;
#endif
    EMU_DCDCInit(&dcdcInit);

    // Set up clocks
    initMcu_clocks();

    RTCC_Init_TypeDef rtccInit = RTCC_INIT_DEFAULT;
    rtccInit.enable            = true;
    rtccInit.debugRun          = false;
    rtccInit.precntWrapOnCCV0  = false;
    rtccInit.cntWrapOnCCV1     = false;
    rtccInit.prescMode         = rtccCntTickPresc;
    rtccInit.presc             = rtccCntPresc_1;
    rtccInit.enaOSCFailDetect  = false;
    rtccInit.cntMode           = rtccCntModeNormal;
    RTCC_Init(&rtccInit);

#if defined(EMU_VSCALE_PRESENT)
    // Set up EM0, EM1 energy mode configuration
    EMU_EM01Init_TypeDef em01Init = EMU_EM01INIT_DEFAULT;
    EMU_EM01Init(&em01Init);
#endif // EMU_VSCALE_PRESENT
    // Set up EM2, EM3 energy mode configuration
    EMU_EM23Init_TypeDef em23init = EMU_EM23INIT_DEFAULT;
#if defined(EMU_VSCALE_PRESENT)
    em23init.vScaleEM23Voltage = emuVScaleEM23_LowPower;
#endif // EMU_VSCALE_PRESENT
    EMU_EM23Init(&em23init);

#if defined(RMU_PRESENT)
    // Set reset mode for sysreset back to DEFAULT (extended), this might have
    // been changed by the bootloader to FULL reset.
    RMU->CTRL = (RMU->CTRL & ~_RMU_CTRL_SYSRMODE_MASK) | RMU_CTRL_SYSRMODE_DEFAULT;
#endif
}

static void initMcu_clocks(void)
{
    // Initialize HFXO
    initHFXO();

    // Set system HFXO frequency
    SystemHFXOClockSet(BSP_CLK_HFXO_FREQ);

    // Enable HFXO oscillator, and wait for it to be stable
    CMU_OscillatorEnable(cmuOsc_HFXO, true, true);

    // Enable HFXO Autostart only if EM2 voltage scaling is disabled.
    // In 1.0 V mode the chip does not support frequencies > 21 MHz,
    // this is why HFXO autostart is not supported in this case.
#if !defined(_EMU_CTRL_EM23VSCALE_MASK)
    // Automatically start and select HFXO
    CMU_HFXOAutostartEnable(0, true, true);
#else
    CMU_ClockSelectSet(cmuClock_HF, cmuSelect_HFXO);
#endif //_EMU_CTRL_EM23VSCALE_MASK

    // HFRCO not needed when using HFXO
    CMU_OscillatorEnable(cmuOsc_HFRCO, false, false);

    // Enabling HFBUSCLKLE clock for LE peripherals
    CMU_ClockEnable(cmuClock_HFLE, true);

    // Initialize LFXO
    CMU_LFXOInit_TypeDef lfxoInit = BSP_CLK_LFXO_INIT;
    lfxoInit.ctune                = BSP_CLK_LFXO_CTUNE;
    CMU_LFXOInit(&lfxoInit);
    // Set system LFXO frequency
    SystemLFXOClockSet(BSP_CLK_LFXO_FREQ);

    // Set LFXO if selected as LFCLK
    CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
    CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
    CMU_ClockSelectSet(cmuClock_LFE, cmuSelect_LFXO);
}

static void initHFXO(void)
{
    // Initialize HFXO
    // Use BSP_CLK_HFXO_INIT as last result (4th)
    CMU_HFXOInit_TypeDef hfxoInit = BSP_CLK_HFXO_INIT;
    // if Factory Cal exists in DEVINFO then use it above all (1st)
    if (0 == (DEVINFO->MODULEINFO & DEVINFO_MODULEINFO_HFXOCALVAL_MASK))
    {
#if defined(_SILICON_LABS_32B_SERIES_1)
        set_HFXO_CTUNE(DEVINFO_MODULEINFO_CRYSTALOSCCALVAL);
#elif defined(_SILICON_LABS_32B_SERIES_2)
        set_HFXO_CTUNE(DEVINFO->MODXOCAL & _DEVINFO_MODXOCAL_HFXOCTUNEXIANA_MASK);
#endif
    }
    // if User page has CTUNE from studio use that in 2nd place
#if (MFG_CTUNE_EN == 1)
    else if (MFG_CTUNE_VAL != 0xFFFF)
    {
        set_HFXO_CTUNE(MFG_CTUNE_VAL);
    }
#endif
    // 3rd option, get data from header defined for product/board
#if defined(BSP_CLK_HFXO_CTUNE) && BSP_CLK_HFXO_CTUNE >= 0
    else
    {
        set_HFXO_CTUNE(BSP_CLK_HFXO_CTUNE);
    }
#endif
    CMU_HFXOInit(&hfxoInit);
}
