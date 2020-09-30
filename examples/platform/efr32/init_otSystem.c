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

#include <support/c/CHIPPlatformMemory.h>

#include <mbedtls/platform.h>
#include <openthread/cli.h>
#include <openthread/dataset.h>
#include <openthread/error.h>
#include <openthread/heap.h>
#include <openthread/icmp6.h>
#include <openthread/instance.h>
#include <openthread/link.h>
#include <openthread/platform/openthread-system.h>
#include <openthread/platform/uart.h>
#include <openthread/tasklet.h>
#include <openthread/thread.h>

#include <openthread-core-config.h>
#include <openthread/config.h>

#include <assert.h>
#include <string.h>

#include "common/logging.hpp"

#include "bsp.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_core.h"
#include "em_emu.h"
#include "em_system.h"
#include "hal-config.h"
#include "hal_common.h"
#include "rail.h"
#include "sl_mpu.h"
#include "sl_sleeptimer.h"

#include "platform-efr32.h"

#if (HAL_FEM_ENABLE)
#include "fem-control.h"
#endif

#if DISPLAY_ENABLED
#include "lcd.h"
#endif

void halInitChipSpecific(void);

void initOtSysEFR(void)
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

#if defined(EFR32MG12)
    CMU_ClockSelectSet(cmuClock_LFE, cmuSelect_LFRCO);
    CMU_ClockEnable(cmuClock_CORELE, true);
#elif defined(EFR32MG21)
    CMU_OscillatorEnable(cmuOsc_LFRCO, true, true);
#else
#error "Enable Clocks for the used board"
#endif /* EFR32 PLATFORM */
    CMU_ClockEnable(cmuClock_RTCC, true);
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
    efr32RadioInit();
    efr32AlarmInit();
    efr32MiscInit();
#ifndef EFR32MG21
    efr32RandomInit();
#endif /* EFR32 PLATFORM */
    otHeapSetCAllocFree(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);
}
