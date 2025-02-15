/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

/**
 *    @file
 *          Provides a glue layer between Matter and NXP-SDK Low Power
 */

#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)

#include <platform/CHIPDeviceLayer.h>
#include <platform/ThreadStackManager.h>

#include "Keyboard.h"
#include "MacSched.h"
#include "OtaSupport.h"
#include "PWR_Configuration.h"
#include "PWR_Interface.h"
#include "RNG_Interface.h"
#include "app_dual_mode_low_power.h"
#include "app_dual_mode_switch.h"
#include "k32w0-chip-mbedtls-config.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;

extern "C" void InitLowPower();

extern "C" void vMMAC_IntHandlerBbc();
extern "C" void vMMAC_IntHandlerPhy();
extern "C" void BOARD_SetClockForPowerMode(void);
extern "C" void stopM2();
extern "C" void sched_enable();
extern "C" uint64_t otPlatTimeGet(void);
extern "C" void vOptimizeConsumption(void);
extern "C" void BOARD_CheckPowerDownDcdcVoltage(void);
extern "C" void BOARD_CheckWakeUpDcdcVoltage(void);

WEAK void dm_switch_wakeupCallBack(void);
WEAK void dm_switch_preSleepCallBack(void);
WEAK void vOptimizeConsumption(void);
WEAK void BOARD_CheckPowerDownDcdcVoltage(void);
WEAK void BOARD_CheckWakeUpDcdcVoltage(void);
static void ThreadExitSleep();
static void BOARD_SetClockForWakeup(void);

typedef struct
{
    bool_t threadInitialized;
    uint32_t threadWarmBootInitTime;
} sDualModeAppStates;

typedef void * appCallbackParam_t;
typedef void (*appCallbackHandler_t)(appCallbackParam_t param);

static sDualModeAppStates dualModeStates;
/* 15.4 warm time must be > 0, so this value will be
 * updated when 15.4 is initialized for the first time
 */
constexpr uint16_t kThreadWarmNotInitializedValue = 1000; /* 1 ms */

extern "C" void otTaskletsSignalPending(otInstance * p_instance);

#define ENABLE_LOW_POWER_LOGS 0

void InitLowPower()
{
    PWR_Init();

    /* Internal - MATTER-303: keep in retention the entire RAM1 for the moment */
    PWR_vAddRamRetention((uint32_t) 0x4020000, 0x10000);

    PWR_RegisterLowPowerExitCallback(dm_switch_wakeupCallBack);
    PWR_RegisterLowPowerEnterCallback(dm_switch_preSleepCallBack);

    dualModeStates.threadWarmBootInitTime = kThreadWarmNotInitializedValue;

    dm_lp_init();
}

extern "C" void setThreadInitialized(bool isInitialized)
{
    dualModeStates.threadInitialized = isInitialized;
}

extern "C" bool isThreadInitialized()
{
    return dualModeStates.threadInitialized;
}

uint32_t dm_switch_get15_4InitWakeUpTime(void)
{
    return dualModeStates.threadWarmBootInitTime;
}

WEAK void dm_switch_wakeupCallBack(void)
{
    /* Check VBAT voltage after wakeup and set DCDC voltage
    according for K32W041A/AM variants */
    BOARD_CheckWakeUpDcdcVoltage();

    BOARD_SetClockForWakeup();
    SHA_ClkInit(SHA_INSTANCE);
    CLOCK_EnableClock(kCLOCK_Aes);

#if ENABLE_LOW_POWER_LOGS
    K32W_LOG("dm_switch_wakeupCallBack");
    K32W_LOG("Warm up time actual value: %d", dualModeStates.threadWarmBootInitTime);
#endif

    RNG_Init();
    SecLib_Init();
    OTA_InitExternalMemory();

    KBD_PrepareExitLowPower();

#if gAdcUsed_d
    BOARD_ADCWakeupInit();
#endif

    PWR_WakeupReason_t wakeReason = PWR_GetWakeupReason();
    if (wakeReason.Bits.FromBLE_LLTimer == 1)
    {
#if ENABLE_LOW_POWER_LOGS
        K32W_LOG("woken up from LL");
#endif
    }
    else if (wakeReason.Bits.FromKeyBoard == 1)
    {
#if ENABLE_LOW_POWER_LOGS
        K32W_LOG("woken up from FromKeyBoard");
#endif
    }
    else if (wakeReason.Bits.FromTMR == 1)
    {
#if ENABLE_LOW_POWER_LOGS
        K32W_LOG("woken up from TMR");
#endif
    }
    else if (wakeReason.Bits.FromWakeTimer == 1)
    {
#if ENABLE_LOW_POWER_LOGS
        K32W_LOG("woken up from Wake Timer");
#endif
    }
    else
    {
#if ENABLE_LOW_POWER_LOGS
        K32W_LOG("woken up from unknown source");
#endif
    }
    dm_lp_wakeup();
}

WEAK void vOptimizeConsumption(void)
{
    /* Intentionally left empty, user needs to redefine it at application level */
}

WEAK void BOARD_CheckPowerDownDcdcVoltage(void)
{
    /* Intentionally left empty, will be linked to board.c function */
}

WEAK void BOARD_CheckWakeUpDcdcVoltage(void)
{
    /* Intentionally left empty, will be linked to board.c function */
}

WEAK void dm_switch_preSleepCallBack(void)
{
#if ENABLE_LOW_POWER_LOGS
    K32W_LOG("dm_switch_preSleepCallBack");
#endif

    /* Inform the low power dual mode module that we will sleep.
       It disables the MAC scheduler.
       Disabling the scheduler must be done before calling vMMAC_Disable()
       so it works correctly */
    dm_lp_preSleep();

    if (dualModeStates.threadInitialized)
    {
        otPlatRadioDisable(NULL);
        setThreadInitialized(FALSE);
    }

    EEPROM_DeInit();

    vOptimizeConsumption();

    /* disable SHA clock */
    SHA_ClkDeinit(SHA_INSTANCE);

    /* disable the AES clock */
    CLOCK_DisableClock(kCLOCK_Aes);

    /* configure pins for power down mode */
    BOARD_SetPinsForPowerMode();
    /* DeInitialize application support for drivers */
    BOARD_DeInitAdc();
    /* DeInit the necessary clocks */
    BOARD_SetClockForPowerMode();

    /* Check VBAT voltage before going to sleep and set DCDC voltage
    according for K32W041A/AM variants */
    BOARD_CheckPowerDownDcdcVoltage();
}

void dm_switch_init15_4AfterWakeUp(void)
{
    uint64_t tick1 = 0;

    if (dualModeStates.threadWarmBootInitTime == kThreadWarmNotInitializedValue)
    {
        /* Get a 32K tick */
        PWR_Start32kCounter();
        tick1 = otPlatTimeGet();
    }

    OSA_InstallIntHandler(ZIGBEE_MAC_IRQn, vMMAC_IntHandlerBbc);
    OSA_InstallIntHandler(ZIGBEE_MODEM_IRQn, vMMAC_IntHandlerPhy);

    /* Radio must be re-enabled after waking up from sleep.
       The module is completely disabled in power down mode.
       15.4 is set as active by default */
    otPlatRadioEnable(NULL);

    /* set the correct state */
    vDynRequestState(E_DYN_SLAVE, E_DYN_STATE_INACTIVE); /* 15.4 */
    vDynRequestState(E_DYN_MASTER, E_DYN_STATE_ACTIVE);  /* BLE */

    sched_enable();

    if (dualModeStates.threadWarmBootInitTime == kThreadWarmNotInitializedValue)
    {
        dualModeStates.threadWarmBootInitTime = (uint32_t) (otPlatTimeGet() - tick1);

        /* Add a margin of 0.5 ms */
        dualModeStates.threadWarmBootInitTime += 500;

#if ENABLE_LOW_POWER_LOGS
        K32W_LOG("Calibration: %d", dualModeStates.threadWarmBootInitTime);
#endif
    }

    setThreadInitialized(TRUE);

    /* wake up the Thread stack and check if any processing needs to be done.
       We're called form ISR context */
    otSysEventSignalPending();
}

extern "C" void App_NotifyWakeup(void)
{
    if ((!dualModeStates.threadInitialized))
    {
        /* Notify the dual mode low power mode */
        dm_lp_processEvent((void *) e15_4WakeUpEnded);
    }
}

extern "C" void App_AllowDeviceToSleep()
{
    PWR_AllowDeviceToSleep();
}

extern "C" void App_DisallowDeviceToSleep()
{
    PWR_DisallowDeviceToSleep();
}

static void BOARD_SetClockForWakeup(void)
{
    /* Enables the clock for the I/O controller block. 0: Disable. 1: Enable.: 0x01u */
    CLOCK_EnableClock(kCLOCK_Iocon);
    /* Enables the clock for the GPIO0 module */
    CLOCK_EnableClock(kCLOCK_Gpio0);
}

#endif
