/*! *********************************************************************************
* Copyright (c) 2015, Freescale Semiconductor, Inc.
* Copyright 2016-2017, 2019-2020 NXP
* All rights reserved.
*
* \file
*
* SPDX-License-Identifier: BSD-3-Clause
********************************************************************************** */

/*****************************************************************************
 *                               INCLUDED HEADERS                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the headers that this module needs to include.    *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
#include "EmbeddedTypes.h"
#include "PWR_Configuration.h"
#include "PWRLib.h"
#include "PWR_Interface.h"
#include "TimersManager.h"
#include "fsl_device_registers.h"
#include "fsl_os_abstraction.h"
#include "board.h"
#if gSupportBle
// TODO: Register pointers from application to call BLE/ZB functions to remove
//       the 2 following headers
#include "ble_general.h"
#include "controller_interface.h"
#include "fsl_xcvr.h"
#endif
#include "fsl_wtimer.h"

#include "fsl_fmeas.h"
#include "fsl_inputmux.h"
#include "fsl_power.h"
#include "GPIO_Adapter.h"

#include "fsl_rtc.h"

#include "Panic.h"

#define RTOS_TICKLESS 0
#if defined USE_RTOS && (USE_RTOS != 0)
  #ifdef FSL_RTOS_FREE_RTOS
  #include "FreeRTOSConfig.h"
  #include "FreeRTOS.h"
  #include "portmacro.h"
  #undef RTOS_TICKLESS
  #if (defined configUSE_TICKLESS_IDLE && (configUSE_TICKLESS_IDLE != 0) && (cPWR_FullPowerDownMode))
    #define RTOS_TICKLESS 1
  #else
    #define RTOS_TICKLESS 0
//  #define USE_WTIMER /* For now use RTC Wake counter */
  #endif
  #else /* not FreeRTOS */
  #error "RTOS unsupported"
  #endif
#endif

#if (cPWR_FullPowerDownMode)
extern void hardware_init(void);
#endif

/*****************************************************************************
 *                             PRIVATE MACROS                                *
 *---------------------------------------------------------------------------*
 * Add to this section all the access macros, registers mappings, bit access *
 * macros, masks, flags etc ...                                              *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
#ifndef BIT
#define BIT(x)                  (1 << (x))
#endif

#ifndef FMEAS_SYSCON
#if defined(FSL_FEATURE_FMEAS_USE_ASYNC_SYSCON) && (FSL_FEATURE_FMEAS_USE_ASYNC_SYSCON)
#define FMEAS_SYSCON ASYNC_SYSCON
#else
#define FMEAS_SYSCON SYSCON
#endif
#endif

#define SET_MPU_CTRL(x) (*(uint32_t*)0xe000ed94 = (uint32_t)(x))

#define PWR_PREVENT_SLEEP_IF_LESS_TICKS        (1000)

#define TICKS32K_TO_SECONDS(x)        ((x)>>15))   /* multiply by 32768 divide by 1000 */
#define TICKS32K_TO_MILLISECONDS(x)   (((x)*125)>>12) /* mult by 1000 divided by 32768*/
#define SECONDS_TO_TICKS32K(x)         ((x)<<15)
#define MILLISECONDS_TO_TICKS32K(x)   (((x)<<12)/125)

#define RTCTICKS_TO_SECONDS(x)        (x)
#define SECONDS_TO_RTCTICKS(x)        (x)
#define RTCTICKS_TO_MILLISECONDS(x)        ((x)*1000)
#define MILLISECONDS_TO_RTCTICKS(x)        ((x)/1000)
/* Tune FRO32K calibration
 * PWR_FRO32K_CAL_SCALE : Calibration period 4: 2^4*30.5us = 488us */
#define PWR_FRO32K_CAL_WAKEUP_END         0   /* Set to 1 to complete the Cal at the end of the SW wakeup     */
#define PWR_FRO32K_CAL_SCALE              6   /* ( 4 if PWR_FRO32K_CAL_WAKEUP_END==1 , 5 otherwise)           */
#define PWR_FRO32K_CAL_AVERAGE            5
#define PWR_FRO32K_CAL_SHIFT              4   /* passed to BLE controller via the PWR_cloch_hk_t structure  */
#define PWR_FRO32K_CAL_INCOMPLETE_PRINTF  1   /* set to 1 to PRINTF when CAL is completed when we check it    */

#if gSupportBle
#define PWR_DEBUG   (1)
#else
#define PWR_DEBUG   (0)
#endif

#if PWR_DEBUG
#include "fsl_debug_console.h"
#endif


/*****************************************************************************
 *                               PUBLIC VARIABLES                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have global      *
 * (project) scope.                                                          *
 * These variables / constants can be accessed outside this module.          *
 * These variables / constants shall be preceded by the 'extern' keyword in  *
 * the interface header.                                                     *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

/*****************************************************************************
 *                           PRIVATE FUNCTIONS PROTOTYPES                    *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions prototypes that have local (file)   *
 * scope.                                                                    *
 * These functions cannot be accessed outside this module.                   *
 * These declarations shall be preceded by the 'static' keyword.             *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
#if (cPWR_FullPowerDownMode)
static void vHandleSleepModes(uint32_t u32Mode);
#if cPWR_EnableDeepSleepMode_4
static void vHandleDeepDown(uint32_t u32Mode);
#endif

static void PWR_SetWakeUpConfig(uint32_t set_msk, uint32_t clr_msk);
static uint32_t PWR_GetWakeUpConfig(void);
static void vSetWakeUpIoConfig(void);
static uint64_t u64GetWakeupSourceConfig(uint32_t u32Mode);
#if (RTOS_TICKLESS)
static void PWR_RTCWakeupStart(void);
#endif
#endif /* (cPWR_FullPowerDownMode) */

/*****************************************************************************
 *                        PUBLIC TYPE DEFINITIONS                            *
 *---------------------------------------------------------------------------*
 * Add to this section all the data types definitions: structures, unions,    *
 * enumerations, typedefs ...                                                *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
typedef enum
{
    PWR_Run = 77,
    PWR_Sleep,
    PWR_PowerDown,
    PWR_DeepDown,
    PWR_OFF,
    PWR_Reset,
} PWR_PowerState_t;

typedef void (*pfHandleDeepSleepFunc_t)(uint32_t u32Config);
typedef struct
{
    pfHandleDeepSleepFunc_t pfFunc;
    uint32_t                u32Config;
} SleepModeTable_t;

typedef struct {
    uint32_t mbasepri;
    uint32_t scb_icsr;
    uint32_t scb_aircr;
    uint32_t scb_shp[12];
    uint32_t nvic_ip[NUMBER_OF_INT_VECTORS];
    uint32_t sysTick_CSR;
    uint32_t sysTick_RV;

} ARM_CM4_register_t;

/*****************************************************************************
 *                               PRIVATE VARIABLES                           *
 *---------------------------------------------------------------------------*
 * Add to this section all the variables and constants that have local       *
 * (file) scope.                                                             *
 * Each of this declarations shall be preceded by the 'static' keyword.      *
 * These variables / constants cannot be accessed outside this module.       *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
#if (cPWR_FullPowerDownMode)
uint8_t mLPMFlag               = gAllowDeviceToSleep_c;
static uint8_t mLpmXcvrDisallowCnt    = 0;
static uint32_t pwr_wakeup_io         = 0;
static bool_t mPreventEnterLowPower   = FALSE;

#if gClkUseFro32K && !gPWR_UseAlgoTimeBaseDriftCompensate
/* When using FRO32K without compensation, keep active AO LDO voltage in sleep */
#define PWR_CFG_AO_LDO_BUILD (PWR_CFG_MAINTAIN_AO_LDO)
#else
/* Allow more power saving by decreasing the Always ON LDO voltage in power down mode */
#define PWR_CFG_AO_LDO_BUILD (PWR_CFG_REDUCE_AO_LDO)
#endif

const SleepModeTable_t maHandleDeepSleepTable[] =
{
                                                                                    /* Mode | Osc | RAM | AO LDO | PWRM mapping              */
    {vHandleSleepModes, PWR_CFG_AO_LDO_BUILD | PWR_CFG_OSC_ON | PWR_CFG_RAM_ON},    /* 1    | On  | On  | Build  | E_AHI_SLEEP_OSCON_RAMON   */
    {vHandleSleepModes, PWR_CFG_AO_LDO_BUILD | PWR_CFG_OSC_ON | PWR_CFG_RAM_OFF},   /* 2    | On  | Off | Build  | E_AHI_SLEEP_OSCON_RAMOFF  */
    {vHandleSleepModes, PWR_CFG_REDUCE_AO_LDO | PWR_CFG_OSC_ON | PWR_CFG_RAM_ON},   /* 3    | On  | On  | Reduce | -                         */
#if cPWR_EnableDeepSleepMode_4
    {vHandleDeepDown,   PWR_CFG_REDUCE_AO_LDO | PWR_CFG_OSC_OFF | PWR_CFG_RAM_OFF}, /* 4    | Off | Off | Reduce | -                         */
#else
    {(pfHandleDeepSleepFunc_t)0, 0},
#endif
    {vHandleSleepModes, PWR_CFG_REDUCE_AO_LDO | PWR_CFG_OSC_OFF | PWR_CFG_RAM_ON},  /* 5    | Off | On  | Reduce | E_AHI_SLEEP_OSCOFF_RAMON  */
    {vHandleSleepModes, PWR_CFG_REDUCE_AO_LDO | PWR_CFG_OSC_OFF | PWR_CFG_RAM_OFF}, /* 6    | Off | Off | Reduce | E_AHI_SLEEP_OSCOFF_RAMOFF */
};
static ARM_CM4_register_t  cm4_misc_regs;
static pfPWRCallBack_t gpfPWR_LowPowerEnterCb;
static pfPWRCallBack_t gpfPWR_LowPowerExitCb;

static PWR_clock_32k_hk_t mHk32k = {
    .freq_scale_shift = PWR_FRO32K_CAL_SHIFT,
    .freq32k   = (32768*PWR_FRO32K_CAL_SHIFT), /* expressed in 16th of Hz: (1<<19) */
    .ppm_32k = -0x6000,                        /* initialization of the 32k clock calibration in part per miliion */
};

static volatile bool_t        s_bWakeTimerActive = FALSE;
static PWR_tsWakeTimerEvent  *psNextWake = NULL;
static pm_power_config_t      pwrm_sleep_config;
static uint32_t               pwrm_force_retention;

/* Flag to say if BLE is active. Managed by application, but default depends
 * on build configuration.
 */
#if gSupportBle
static volatile bool_t        bBLE_Active = TRUE;
#else
static volatile bool_t        bBLE_Active = FALSE;
#endif

#if RTOS_TICKLESS
#define RESTORE_SYSTICK_ON_WAKEUP 1
uint8_t mPWR_DeepSleepTimeUpdated = 0;
uint32_t mPWR_DeepSleepTimeMs;
#endif
/*****************************************************************************
 *                             PRIVATE FUNCTIONS                             *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions that have local (file) scope.       *
 * These functions cannot be accessed outside this module.                   *
 * These definitions shall be preceded by the 'static' keyword.              *
 *---------------------------------------------------------------------------*
 *****************************************************************************/
static void Save_CM4_registers(ARM_CM4_register_t * reg_store)
{
#if defined(USE_RTOS) && (USE_RTOS)
    reg_store->mbasepri = __get_BASEPRI();
    for(int i=0;i<12;i++)
    {
        reg_store->scb_shp[i] = SCB->SHP[i];
    }
    reg_store->scb_icsr = SCB->ICSR;
    reg_store->scb_aircr = SCB->AIRCR;
#endif
    for(int i=0;i<NUMBER_OF_INT_VECTORS;i++)
    {
        reg_store->nvic_ip[i] = NVIC->IP[i];
    }
#ifdef RESTORE_SYSTICK_ON_WAKEUP
    reg_store->sysTick_CSR = SysTick->CTRL;
    /* Value to be programmed in LOAD at next SYSTICK restore */
    /* Nb Core ticks to countdown */
    uint32_t count_for_one_tick = CLOCK_GetFreq(kCLOCK_CoreSysClk) / configTICK_RATE_HZ;
    reg_store->sysTick_RV =  (count_for_one_tick -1UL) & SysTick_LOAD_RELOAD_Msk;

    /* Disable SysTick counter and interrupt */
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
    /* clear PendSysTick bit in ICSR, if set */
    SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;
#endif

}

static void Restore_CM4_registers(ARM_CM4_register_t * reg_store)
{
#if (defined(USE_RTOS) && (USE_RTOS))
    __set_BASEPRI(reg_store->mbasepri);
    __DSB();
    __ISB();
    for(int i=0;i<12;i++)
    {
        SCB->SHP[i] = reg_store->scb_shp[i];
    }
    SCB->ICSR = reg_store->scb_icsr;
    SCB->AIRCR = reg_store->scb_aircr;
#endif
    for(int i=0;i<NUMBER_OF_INT_VECTORS;i++)
    {
        NVIC->IP[i] = reg_store->nvic_ip[i];
    }
#ifdef RESTORE_SYSTICK_ON_WAKEUP
    SysTick->VAL = 0UL; /* Current value is reset */
    SysTick->LOAD = reg_store->sysTick_RV; /* Set reload value first */
    SysTick->CTRL = reg_store->sysTick_CSR & ~SysTick_CTRL_ENABLE_Msk; /* postpone systick restart */
#endif
}

#if PWR_DEBUG
#if defined DBG_PWR && (DBG_PWR > 0)

/* The macros and BLE register access functions below are copied from reg_blecore.h */

#define BLE_BASE_ADDR 0x400A0000

#define BLE_DEEPSLCNTL_OFFSET 0x00000030

#define REG_BLE_RD(offs)             (*(volatile uint32_t *)(BLE_BASE_ADDR+(offs)))

static inline uint32_t ble_deepslcntl_get(void)
{
    return REG_BLE_RD(BLE_DEEPSLCNTL_OFFSET);
}

#define BLE_DEEPSLSTAT_OFFSET 0x00000038

static inline uint32_t ble_deepslstat_get(void)
{
    return REG_BLE_RD(BLE_DEEPSLSTAT_OFFSET);
}

#define BLE_DEEPSLWKUP_OFFSET 0x00000034

static inline uint32_t ble_deepslwkup_get(void)
{
    return REG_BLE_RD(BLE_DEEPSLWKUP_OFFSET);
}

#define BLE_EXTWKUPDSB_POS            31
#define BLE_DEEP_SLEEP_STAT_POS       15
#define BLE_SOFT_WAKEUP_REQ_POS       4
#define BLE_DEEP_SLEEP_CORR_EN_POS    3
#define BLE_DEEP_SLEEP_ON_POS         2
#define BLE_RADIO_SLEEP_EN_POS        1
#define BLE_OSC_SLEEP_EN_POS          0

#define BLE_ENBPRESET_OFFSET 0x0000003C

static inline uint32_t ble_enbpreset_get(void)
{
    return REG_BLE_RD(BLE_ENBPRESET_OFFSET);
}


#if CHECK_PENDING_IRQ
static bool checkIrqPending() __keep_unused
{
    bool no_irq_pending = true;
    uint32_t     pmc_wakeiocause;

    for (IRQn_Type IRQ_Type = WDT_BOD_IRQn; IRQ_Type<=BLE_WAKE_UP_TIMER_IRQn ; IRQ_Type++)
    {
        bool irq_pending;
        irq_pending = NVIC_GetPendingIRQ(IRQ_Type);
        if (irq_pending)
        {
            no_irq_pending = false;
            PWR_DBG_LOG("IRQ=%d Pending", IRQ_Type);

            NVIC_EnableIRQ(IRQ_Type);
            //NVIC_DisableIRQ(IRQ_Type);
        }
    }

    if ( no_irq_pending )
    {
        PWR_DBG_LOG("No IRQ Pending");
    }

    pmc_wakeiocause = PMC->WAKEIOCAUSE;
    if (pmc_wakeiocause)
    {
        no_irq_pending = false;
        PWR_DBG_LOG("WAKEIOCAUSE=%x", pmc_wakeiocause);
    }
    return no_irq_pending;
}
#endif

static void debug_low_power(void)
{
#if CHECK_PENDING_IRQ
    uint32_t val = ble_deepslcntl_get();

    uint32_t lldeepsleep = ( val & BIT(BLE_DEEP_SLEEP_STAT_POS) );

    PWR_DBG_LOG("llDS=%d, DUR=%d, WKUP=%d", lldeepsleep, ble_deepslstat_get(), ble_deepslwkup_get());

    if ( lldeepsleep )
    {
        checkIrqPending();
    }
#else
    PWR_DBG_LOG("llDS=%d, DUR=%d, WKUP=%d", ble_deepslcntl_get(), ble_deepslstat_get(), ble_deepslwkup_get());
#endif
    /*
     * PWR_DBG_LOG("ENPB=0x%x", ble_enbpreset_get());
     * ENBPRESET is not restored yet so no use logging
     */
}
#endif
/* Default implementation of the WakeupTimer IRQ handler is weak:
 * here redefine body where an IO is lowered on entry and raised on exit
 * */
void BLE_WakeupTimer_IRQHandler(void)
{
#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
    BOARD_DbgIoSet(4, 0);
#endif
    NVIC_DisableIRQ(BLE_WAKE_UP_TIMER_IRQn);
#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
    BOARD_DbgIoSet(4, 1);
#endif
}
#endif /* PWR_DEBUG */

#if 0
static int WaitForRegisterBitToChange(uint32_t * address, uint32_t mask, uint32_t up_or_down)
{
#define MAX_WAIT_LOOP 10000
    int loopcnt = MAX_WAIT_LOOP;
    volatile uint32_t * reg = address;
    while (1)
    {
        loopcnt--;
        if (loopcnt > 0)
        {
            uint32_t val = ((*reg & mask) != 0);
            uint32_t expected = (up_or_down != 0);
            if (val == expected) break;
        }
        else
        {
            if (up_or_down)
                PRINTF("Register rise timeout address=%x, mask=%x reading=%x\r\n", reg, mask, *reg);
            else
                PRINTF("Register fall timeout address=%x, mask=%x reading=%x\r\n", reg, mask, *reg);
            panic( 0x20UL, (uint32_t)__FILE__, (uint32_t)__LINE__, 0UL);
            break;
        }
    }
    //PWR_DBG_LOG("%x took %d loops", address, MAX_WAIT_LOOP - loopcnt);
    return loopcnt;
}
#endif

#if gClkUseFro32K

/* suppose 32MHz crystal is running and 32K running */
void PWR_Start32KCalibration(void)
{
    INPUTMUX_Init(INPUTMUX);

    /* Setup to measure the selected target */
    INPUTMUX_AttachSignal(INPUTMUX, 1U, kINPUTMUX_Xtal32MhzToFreqmeas);
    INPUTMUX_AttachSignal(INPUTMUX, 0U, kINPUTMUX_32KhzOscToFreqmeas);

    /* Temporary fix for RFT1366 : JN518x Frequency measure does not work 32kHz
       if used for target clock */
    SYSCON->MODEMCTRL |= SYSCON_MODEMCTRL_BLE_LP_OSC32K_EN(1);

    CLOCK_EnableClock(kCLOCK_Fmeas);

    /* Start a measurement cycle and wait for it to complete. If the target
       clock is not running, the measurement cycle will remain active
       forever, so a timeout may be necessary if the target clock can stop */
    FMEAS_StartMeasureWithScale(FMEAS_SYSCON, PWR_FRO32K_CAL_SCALE);
}

// Return the result in unit of 1/(2^freq_scale)th of Hertz for higher accuracy
uint32_t PWR_Complete32KCalibration(uint8_t u8Shift)
{
    uint32_t        freqComp    = 0U;
    uint32_t        refCount    = 0U;
    uint32_t        targetCount = 0U;
    uint32_t        freqRef     = CLOCK_GetFreq(kCLOCK_Xtal32M);

    if (FMEAS_IsMeasureComplete(FMEAS_SYSCON))
    {
        /* Get computed frequency */
        FMEAS_GetCountWithScale(FMEAS_SYSCON, PWR_FRO32K_CAL_SCALE, &refCount, &targetCount);
        freqComp = (uint32_t)(((((uint64_t)freqRef)*refCount)<<u8Shift) / targetCount);

        /* Disable the clocks if disable previously */
        CLOCK_DisableClock(kCLOCK_Fmeas);
    }

    return freqComp;
}

void Update32kFrequency(uint32_t freq)
{
    if (freq != 0UL)
    {
        PWR_clock_32k_hk_t *hk = PWR_GetHk32kHandle();

        uint32_t freq32k = hk->freq32k;
        freq32k = ((freq32k << PWR_FRO32K_CAL_AVERAGE) - freq32k + freq) >> PWR_FRO32K_CAL_AVERAGE ;
        PWR_DBG_LOG("cal=%d - dbg_32k_freq=%d", freq>>hk->freq_scale_shift, freq32k>>hk->freq_scale_shift);
        hk->freq32k = freq32k; /* update structure shared with LL */
    }
    else
    {
#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
        BOARD_DbgIoSet(4, 1);
        BOARD_DbgIoSet(4, 0);
        BOARD_DbgIoSet(4, 1);
        BOARD_DbgIoSet(4, 0);
#endif

#if PWR_FRO32K_CAL_INCOMPLETE_PRINTF
        PWR_DBG_LOG("32K cal incomplete");
#endif
    }

}


#endif

/*---------------------------------------------------------------------------
 * Name: PWR_HandleDeepSleepMode
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
static void vHandleSleepModes(uint32_t mode)
{
#if gSupportBle
    uint8_t   power_down_mode = 0xff;
#endif
    WTIMER_status_t InterruptStatus1;

     /* TODO: Note from pwrm.c is to not disable interrupts "Keep commented
        - DEEP DOWN 1 fails with 4470 Application if interrupts are masked" */
    __disable_irq();


    PWR_ClearWakeupReason();

#if gSupportBle
    if (mode & PWR_CFG_RAM_OFF)
    {
        power_down_mode = kPmPowerDown1;
    }
    else
    {
        SYSCON->MODEMCTRL |= SYSCON_MODEMCTRL_BLE_LP_OSC32K_EN_MASK;
        PWR_DBG_LOG("BLE Sleep");
        power_down_mode = BLE_sleep();
        PWR_DBG_LOG("power_down_mode=%x", power_down_mode);
    }
    if (power_down_mode < kPmPowerDown0)
    {
        PWR_EnterSleep();
    }
    else
#endif
    {
        pwrlib_pd_cfg_t pd_cfg;

        /* Prevent disabling the 32M XO at wakeup - see artf587675
         * XTAL32MCTRL is set to 3 or 1 from POR/PD by SW, need to set back to zero    */
        SYSCON->XTAL32MCTRL = SYSCON_XTAL32MCTRL_DEACTIVATE_PMC_CTRL(0);
#if gSupportBle
        SYSCON->XTAL32MCTRL =  SYSCON_XTAL32MCTRL_DEACTIVATE_BLE_CTRL(1);
#endif

#if gClkUseFro32K && gClkRecalFro32K && !PWR_FRO32K_CAL_WAKEUP_END
        uint32_t freq = PWR_Complete32KCalibration(mHk32k.freq_scale_shift);
        Update32kFrequency(freq);
#endif

        /* Going to go to sleep */
        /* Call any registered pre-sleep callback */
        if(gpfPWR_LowPowerEnterCb != NULL)
        {
            gpfPWR_LowPowerEnterCb();
        }

       /* XCVR_Deinit(); no longer required since Radio v2053 */
        SET_MPU_CTRL(0);

        /* get the IO wakeup configuration */
        vSetWakeUpIoConfig();

        memset(&pd_cfg, 0x0, sizeof(pwrlib_pd_cfg_t));

        pd_cfg.sleep_cfg = pwrm_force_retention;
        if (mode & PWR_CFG_MAINTAIN_AO_LDO)
        {
            pd_cfg.sleep_cfg |= PM_CFG_KEEP_AO_VOLTAGE;
        }

        pd_cfg.wakeup_io = PWR_GetWakeUpConfig();
        pd_cfg.wakeup_src = u64GetWakeupSourceConfig(mode);

        if (mode & PWR_CFG_OSC_OFF)
        {
            CLOCK_DisableClock(kCLOCK_Fro32k);
            CLOCK_DisableClock(kCLOCK_Xtal32k);
        }

        if (mode & PWR_CFG_RAM_ON)
        {
            /* Useless to save CM4 registers if RAM is not to be retained */
            Save_CM4_registers(&cm4_misc_regs);

            #if  RTOS_TICKLESS
            /* No need to store previous value of RTC IRQ priority: already done by Save_CM4_registers
             * that saves the Interrupt priorities for all IRQs */
            NVIC_ClearPendingIRQ(RTC_IRQn);
            NVIC_SetPriority(RTC_IRQn, 0);  /* Force RTC IRQ to raise its priority required ??? */
            /* Configure wakeup timer : RTC or optionally WTIMER */
            if(mPWR_DeepSleepTimeUpdated)
            {
                PWR_RTCSetWakeupTimeMs(mPWR_DeepSleepTimeMs);
                mPWR_DeepSleepTimeUpdated = FALSE;        // Coexistence with TMR Manager
            }
            PWR_RTCWakeupStart();
            #endif
/***********************END***************************************/
#if gSupportBle
            /* Clear ble wake up IRQs that could be pending */
            NVIC_ClearPendingIRQ(BLE_WAKE_UP_TIMER_IRQn);
#endif
            /* Go to sleep */
            PWRLib_EnterPowerDownMode(&pd_cfg);

            /* At this point, have woken up again */

            /* Restore the state of SysTick */
            // TODO : can not restore the systick due to a freeze in Radio init functions
            //SysTick->CTRL |= sysTickCtrl;

            /* Change CPU clock to appropriate clock source to speed up initialization
             * and update Flash wait states */

            /* Initialise hardware */
            hardware_init();
            Restore_CM4_registers(&cm4_misc_regs); /* Restore IRQs */

#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
            BOARD_DbgIoSet(0, 1);
#endif

            /* Get wakeup reason before waking up the link layer to avoid false LL wakeup detection */
            PWR_UpdateWakeupReason();

#if gSupportBle
            /* Enable BLE Interrupt */
            NVIC_SetPriority(BLE_LL_ALL_IRQn, 0x01U);
            NVIC_EnableIRQ(BLE_LL_ALL_IRQn);
            NVIC_ClearPendingIRQ(BLE_WAKE_UP_TIMER_IRQn);
#endif

#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
            BOARD_DbgIoSet(1, 1);
#endif

            BOARD_DbgDiagEnable();

#if gClkUseFro32K && gClkRecalFro32K
            PWR_Start32KCalibration();
#endif


#if gSupportBle && !(RTOS_TICKLESS && gTimerMgrUseLpcRtc_c)
            TMR_ReInit();
#endif
#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
            BOARD_DbgIoSet(4, 0);
#endif

#if defined DBG_PWR && (DBG_PWR > 0)
            debug_low_power();
#endif

            if (gpfPWR_LowPowerExitCb != NULL)
            {
                gpfPWR_LowPowerExitCb();
            }

#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
            BOARD_DbgIoSet(4, 1);
#endif



#if gClkUseFro32K && gClkRecalFro32K && PWR_FRO32K_CAL_WAKEUP_END
            uint32_t freq = PWR_Complete32KCalibration();
            Update32kFrequency(freq);
#endif

#if defined(gDbgIoCfg_c) && (gDbgIoCfg_c == 1)
            BOARD_DbgIoSet(4, 0);
#endif

            /* Check if WTIMER enabled and, if so, check running status */
            if (0 != (SYSCON->AHBCLKCTRLS[0] & SYSCON_AHBCLKCTRLSET0_WAKE_UP_TIMERS_CLK_SET_MASK))
            {
                InterruptStatus1 = WTIMER_GetStatusFlags(WTIMER_TIMER1_ID);

                if (   (InterruptStatus1 == WTIMER_STATUS_RUNNING)
                    || (InterruptStatus1 == WTIMER_STATUS_EXPIRED) )
                {
                    WTIMER_EnableInterrupts(WTIMER_TIMER1_ID);
                }
            }
        }
        else
        {
            /* Mode with RAM off; will not return here afterwards */
            if (   (pd_cfg.wakeup_src & POWER_WAKEUPSRC_ANA_COMP)
                || (mode & PWR_CFG_OSC_ON)
               )
            {
                /* if Analog comparator wakeup is requested or if oscillator
                   is to keep running, forbid deep down mode */
                PWRLib_EnterPowerDownModeRamOff(&pd_cfg);
            }
            else
            {
                PWRLib_EnterDeepDownMode(&pd_cfg);
            }
        }
    }

    //PWR_DisallowDeviceToSleep();

    __enable_irq();

}

#if cPWR_EnableDeepSleepMode_4
static void vHandleDeepDown(uint32_t u32Mode)
{
    PWR_EnterDeepDown();
}
#endif

/*---------------------------------------------------------------------------
 * Name: PWR_HandleDeepSleep
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
static PWR_WakeupReason_t PWR_HandleDeepSleep(void)
{
    uint8_t lpMode;
    PWR_ClearWakeupReason();

    lpMode = PWRLib_GetDeepSleepMode();

    if(lpMode)
    {
        SleepModeTable_t const *psEntry = &maHandleDeepSleepTable[lpMode - 1];
        if (psEntry->pfFunc)
        {
            psEntry->pfFunc(psEntry->u32Config);
        }
    }

    return PWRLib_MCU_WakeupReason;
}


/*---------------------------------------------------------------------------
 * Name: PWR_DeepSleepAllowed
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
static bool_t PWR_DeepSleepAllowed(void)
{
    bool_t state = TRUE;

    if((PWRLib_GetDeepSleepMode() != 6) && mLpmXcvrDisallowCnt)
    {
        state = FALSE;
    }

    return state;
}
#endif /* #if (cPWR_FullPowerDownMode)*/

PWR_clock_32k_hk_t *PWR_GetHk32kHandle(void)
{
#if (cPWR_FullPowerDownMode)
    return &mHk32k;
#else
    return NULL;
#endif
}

/*---------------------------------------------------------------------------
 * Name: PWR_CheckForAndEnterNewPowerState
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
PWR_WakeupReason_t PWR_CheckForAndEnterNewPowerState(PWR_PowerState_t NewPowerState)
{
    PWR_WakeupReason_t ReturnValue;

    ReturnValue.AllBits = 0;

    if (NewPowerState == PWR_Run)
    {
        /* ReturnValue = 0; */
    }
    else if(NewPowerState == PWR_Sleep)
    {
        PWR_EnterSleep();
    }
#if (cPWR_FullPowerDownMode)
    else if(NewPowerState == PWR_PowerDown)
    {
        ReturnValue = PWR_EnterPowerDown();
    }
    else if(NewPowerState == PWR_DeepDown)
    {
        /* Never returns */
        PWR_EnterDeepDown();
    }
    else if(NewPowerState == PWR_OFF)
    {
        /* Never returns */
        PWR_EnterPowerOff();
    }
#else
    else if(NewPowerState == PWR_PowerDown)
    {
        PWR_EnterSleep();
    }
    else if(NewPowerState == PWR_DeepDown)
    {
        PWR_EnterSleep();
    }
    else if(NewPowerState == PWR_OFF)
    {
        PWR_EnterSleep();
    }
#endif
    else if(NewPowerState == PWR_Reset)
    {
        /* Never returns */
        PWR_SystemReset();
    }
    else
    {
        /* ReturnValue = FALSE; */
    }

    return ReturnValue;
}


/*****************************************************************************
 *                             PUBLIC FUNCTIONS                              *
 *---------------------------------------------------------------------------*
 * Add to this section all the functions that have global (project) scope.   *
 * These functions can be accessed outside this module.                      *
 * These functions shall have their declarations (prototypes) within the     *
 * interface header file and shall be preceded by the 'extern' keyword.      *
 *---------------------------------------------------------------------------*
 *****************************************************************************/

/*---------------------------------------------------------------------------
 * Name: PWR_Init
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWR_Init(void)
{
#if (cPWR_FullPowerDownMode)

#if gClkUseFro32K /* Using 32k FRO */
    #if gClkRecalFro32K /* Will recalibrate 32k FRO on each warm start */
    // TODO MCB-539: paralyze FRO32K calibration to reduce the cold boot time
    uint32_t freq;
    PWR_clock_32k_hk_t *hk = PWR_GetHk32kHandle();

    PWR_Start32KCalibration();
    do {
       freq = PWR_Complete32KCalibration(hk->freq_scale_shift);
    }  while (!freq);

    PWR_DBG_LOG("freq=%d", freq>>hk->freq_scale_shift);

    hk->freq32k = freq;
    #else
    /* Does selected sleep mode require 32kHz oscillator? */
    if (0 != (PWR_GetDeepSleepConfig() & PWR_CFG_OSC_ON))
    {
        bool            fmeas_clk_enable;
        bool            mdm_clk_enable;
        uint32_t        freqComp;

        /* Check if XTAL32K has been enabled by application, otherwise
         * enable the FRO32K and calibrate it - the XTAL32K may not be
         * present on the board */
        if (   (0 == (SYSCON->OSC32CLKSEL & SYSCON_OSC32CLKSEL_SEL32KHZ_MASK))
            || (0 == (PMC->PDRUNCFG & (1UL << kPDRUNCFG_PD_XTAL32K_EN)))
           )
        {
            /* Enable FRO32k */
            CLOCK_EnableClock(kCLOCK_Fro32k);

            /* Enable 32MHz XTAL if not running - FRO32K already enable if we are here*/
            if ( !(ASYNC_SYSCON->XTAL32MCTRL & ASYNC_SYSCON_XTAL32MCTRL_XO32M_TO_MCU_ENABLE_MASK) )
            {
                CLOCK_EnableClock(kCLOCK_Xtal32M);
            }

            /* Fmeas clock gets enabled by the generic calibration code, so note if we
               should disable it again afterwards */
            fmeas_clk_enable = CLOCK_IsClockEnable(kCLOCK_Fmeas);

            /* RFT1366 requires BLE LP clock to be used for 32kHz measurement. It gets
               enabled within the generic calibration code, so note if we should
               disable it again afterwards */
            mdm_clk_enable = SYSCON->MODEMCTRL & SYSCON_MODEMCTRL_BLE_LP_OSC32K_EN_MASK;

            /* Call Low Power function to start calibration */
            PWR_Start32KCalibration();

            /* Call Low Power function to wait for end of calibration */
            do
            {
                freqComp = PWR_Complete32KCalibration(0);
            } while (0 == freqComp);

            /* Disable the clocks if disable previously */
            if ( !fmeas_clk_enable )
            {
                CLOCK_DisableClock(kCLOCK_Fmeas);
            }

            if ( !mdm_clk_enable )
            {
                SYSCON->MODEMCTRL &= ~SYSCON_MODEMCTRL_BLE_LP_OSC32K_EN(1);
            }

            mHk32k.freq32k = freqComp;
        }
        else
        {
            mHk32k.freq32k = (32768<<PWR_FRO32K_CAL_SHIFT);
        }
    }
    #endif /* gClkRecalFro32K */
#else
    /* If using XTAL 32k, set calibration value to expected value */
    mHk32k.freq32k =  (32768<<PWR_FRO32K_CAL_SHIFT);
#endif  /* gClkUseFro32K */

    s_bWakeTimerActive = FALSE;
    psNextWake = NULL;
    pwrm_force_retention = 0;
    memset(&pwrm_sleep_config, 0x0, sizeof(pm_power_config_t));

    PWRLib_Init();
#endif
}


/*---------------------------------------------------------------------------
 * Name: PWR_SystemReset
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWR_SystemReset(void)
{
    RESET_SystemReset();
}


/*---------------------------------------------------------------------------
 * Name: PWR_AllowDeviceToSleep
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWR_AllowDeviceToSleep(void)
{
#if (cPWR_FullPowerDownMode)
    OSA_InterruptDisable();

    if(mLPMFlag > 0)
    {
        mLPMFlag--;
    }
    PWR_DBG_LOG("mLPMFlag=%d", mLPMFlag);

    OSA_InterruptEnable();
#endif /* (cPWR_FullPowerDownMode) */
}

/*---------------------------------------------------------------------------
 * Name: PWR_DisallowDeviceToSleep
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWR_DisallowDeviceToSleep(void)
{
#if (cPWR_FullPowerDownMode)
    uint8_t prot;

    OSA_InterruptDisable();

    prot = mLPMFlag + 1;

    if(prot != 0)
    {
        mLPMFlag++;
    }
    PWR_DBG_LOG("mLPMFlag=%d", mLPMFlag);

    OSA_InterruptEnable();
#endif /* (cPWR_FullPowerDownMode) */
}


/*---------------------------------------------------------------------------
 * Name: PWR_CheckIfDeviceCanGoToSleep
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
bool_t PWR_CheckIfDeviceCanGoToSleep(void)
{
    bool_t returnValue = FALSE;
#if (cPWR_FullPowerDownMode)
    uint8_t  pwr_mode = 0xff;
    bool_t tmr_all_off = FALSE;
    do {
        /* Check if device can sleep for Application */
        if (mLPMFlag > 0) break;
        /* Check if Timers are all OFF */
#if (!cPWR_DiscardRunningTimerForPowerDown) && gSupportBle
        tmr_all_off = TMR_AreAllTimersOff();
        if (!tmr_all_off) break;
#endif

        /* Check if WTIMER enabled and, if so, check running status */
        if (0 != (SYSCON->AHBCLKCTRLS[0] & SYSCON_AHBCLKCTRL0_WAKE_UP_TIMERS_MASK))
        {
            bool dealine_too_close = FALSE;
            WTIMER_status_t InterruptStatus[2];
            uint32_t        timer_count[2];
            for (int i = 0; i < 2; i++)
            {
                InterruptStatus[i] = WTIMER_GetStatusFlags((WTIMER_timer_id_t)i);
                timer_count[i]     = WTIMER_ReadTimer((WTIMER_timer_id_t)i);

                if (   (timer_count[i] < PWR_PREVENT_SLEEP_IF_LESS_TICKS)
                    && (InterruptStatus[i] == WTIMER_STATUS_RUNNING)
                   )
                {
                    dealine_too_close = TRUE;
                    break; /* for */
                }
            }
            if (dealine_too_close) break; /* do .. while (0) */

            /* MCUZIGBEE-946: If pwrm wake timer is not running and sleep mode
             * is 32k OSC ON, prevent sleep.
             * MCUZIGBEE-2469: However, checking for a running wake timer 1 in
             * OSC ON mode is not necessary or desirable when BLE is running,
             * as it sleeps with the 32k OSC ON but no wake timer.
             */
            if (   (0 != (PWR_GetDeepSleepConfig() & PWR_CFG_OSC_ON))
                && (InterruptStatus[1] != WTIMER_STATUS_RUNNING)
                && (FALSE == bBLE_Active)
               )
            {
                break;
            }
        }
        if  (0 != (SYSCON->AHBCLKCTRLS[0] & SYSCON_AHBCLKCTRL0_RTC_MASK))
        {
            uint32_t rtc_ctrl = RTC->CTRL;
            if (rtc_ctrl & (RTC_CTRL_ALARM1HZ_MASK | RTC_CTRL_WAKE1KHZ_MASK))
            {
                /* Alarm or wake interrupt have already fired " prevent sleep */
                break;
            }
            if (rtc_ctrl &  RTC_CTRL_WAKEDPD_EN_MASK)
            {
                /* 1kHz clock timer is running */
                /* Check if the deadline is not too close to go to sleep */
                uint32_t wake_countdown = RTC->WAKE;
                if ((wake_countdown > 0) && (MILLISECONDS_TO_TICKS32K(wake_countdown) < PWR_PREVENT_SLEEP_IF_LESS_TICKS))
                  break;
            }
        }

#if gSupportBle
        if (PWR_GetDeepSleepConfig() & PWR_CFG_RAM_ON)
        {
        	/* Check if BLE LL can sleep */
        	pwr_mode = (uint8_t)BLE_get_sleep_mode();
        	if (pwr_mode < kPmPowerDown0) break;
        }
#endif

       /* All conditions successfully passed */
       returnValue = TRUE;

    } while (0);
    PWR_DBG_LOG("mLPMFlag=%d TimerOff=%d ble pwr_mode=%d", mLPMFlag, tmr_all_off, pwr_mode);
    NOT_USED(pwr_mode);
    NOT_USED(tmr_all_off);
#endif
    return returnValue;
}

/*---------------------------------------------------------------------------
 * Name: PWR_ChangeDeepSleepMode
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
bool_t PWR_ChangeDeepSleepMode(uint8_t dsMode)
{
    bool_t result = TRUE;
    PWR_DBG_LOG("dsMode=%d", dsMode);

#if (cPWR_FullPowerDownMode)
    if ((dsMode > sizeof(maHandleDeepSleepTable)/sizeof(SleepModeTable_t)) ||
        (maHandleDeepSleepTable[dsMode - 1].pfFunc == (pfHandleDeepSleepFunc_t)0))
    {
        result = FALSE;
    }
    else
    {
        PWRLib_SetDeepSleepMode(dsMode);
    }
#endif /* (cPWR_FullPowerDownMode) */

    return result;
}

/*---------------------------------------------------------------------------
 * Name: PWR_GetDeepSleepMode
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint8_t PWR_GetDeepSleepMode(void)
{
    uint8_t result = 0;
#if (cPWR_FullPowerDownMode)
    result = PWRLib_GetDeepSleepMode();
#endif
    return result;
}

/*---------------------------------------------------------------------------
 * Name: PWR_GetDeepSleepConfig
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
uint32_t PWR_GetDeepSleepConfig(void)
{
    uint32_t result = 0;
#if (cPWR_FullPowerDownMode)
    uint8_t  lpMode;
    lpMode = PWRLib_GetDeepSleepMode();

    if (lpMode)
    {
        SleepModeTable_t const *psEntry = &maHandleDeepSleepTable[lpMode - 1];
        if (psEntry->pfFunc)
        {
            result = psEntry->u32Config;
        }
    }
#endif
    return result;
}

/*---------------------------------------------------------------------------
 * Name: PWR_EnterLowPower
 * Description: - Main entry power to switch to low power mode - checks if power
 * down mode is allowed and enter in power down. If not allowed, will fall back
 * to sleep mode (WFI)
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
extern PWR_WakeupReason_t PWR_EnterLowPower(void)
{
    PWR_DBG_LOG("");
    PWR_WakeupReason_t ReturnValue;

    ReturnValue.AllBits = 0;

    OSA_InterruptDisable();

#if (cPWR_FullPowerDownMode)
    if( !mPreventEnterLowPower && PWR_CheckIfDeviceCanGoToSleep() && PWR_DeepSleepAllowed())
    {
        ReturnValue = PWR_EnterPowerDown();
        OSA_InterruptEnable();
    }
    else
#endif
    {
        OSA_InterruptEnable();
        PWR_EnterSleep();
    }

    return ReturnValue;
}

/*---------------------------------------------------------------------------
 * Name: PWR_EnterSleep
 * Description: - Enter the chip in WFI state - CPU clock is switched OFF
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWR_EnterSleep(void)
{
    PWR_DBG_LOG("");
    PWRLib_MCU_Enter_Sleep();
}

/*---------------------------------------------------------------------------
 * Name: PWR_EnterPowerDown
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
PWR_WakeupReason_t PWR_EnterPowerDown(void)
{
    PWR_DBG_LOG("");
    PWR_WakeupReason_t ReturnValue;
    ReturnValue.AllBits = 0;
#if (cPWR_FullPowerDownMode)
    OSA_InterruptDisable();

    ReturnValue = PWR_HandleDeepSleep();

    OSA_InterruptEnable();
#endif
    return ReturnValue;
}

/*---------------------------------------------------------------------------
 * Name: PWR_EnterDeepDown
 * Description: - All the chip is OFF - only wakeup by IO or NTAG
 * Parameters: -
 * Return: - never return
 *---------------------------------------------------------------------------*/
void PWR_EnterDeepDown(void)
{
    PWR_DBG_LOG("");
#if (cPWR_FullPowerDownMode)
    /* Never returns */
    pwrlib_pd_cfg_t pd_cfg;

    OSA_InterruptDisable();

    /* get the IO wakeup configuration */
    vSetWakeUpIoConfig();

    memset( &pd_cfg, 0x0, sizeof(pwrlib_pd_cfg_t) );
    pd_cfg.wakeup_io = PWR_GetWakeUpConfig();

    /* call any registered pre-sleep callbacks */
    if(gpfPWR_LowPowerEnterCb != NULL)
    {
        gpfPWR_LowPowerEnterCb();
    }

    PWRLib_EnterDeepDownMode(&pd_cfg);
#endif
}

/*---------------------------------------------------------------------------
 * Name: PWR_EnterPowerOff
 * Description: - All the chip is OFF - only wakeup by HW reset
 * Parameters: -
 * Return: - never return
 *---------------------------------------------------------------------------*/
void PWR_EnterPowerOff(void)
{
    PWR_DBG_LOG("");
#if (cPWR_FullPowerDownMode)
    /* Never returns */
    pwrlib_pd_cfg_t pd_cfg;

    OSA_InterruptDisable();

    memset( &pd_cfg, 0x0, sizeof(pwrlib_pd_cfg_t) );

    /* call any registered pre-sleep callbacks */
    if(gpfPWR_LowPowerEnterCb != NULL)
    {
        gpfPWR_LowPowerEnterCb();
    }

    PWRLib_EnterDeepDownMode(&pd_cfg);
#endif
}

/*---------------------------------------------------------------------------
 * Name: PWR_RegisterLowPowerEnterCallback
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWR_RegisterLowPowerEnterCallback(pfPWRCallBack_t lowPowerEnterCallback)
{
#if (cPWR_FullPowerDownMode)
    gpfPWR_LowPowerEnterCb = lowPowerEnterCallback;
#endif /* (cPWR_FullPowerDownMode) */
}

/*---------------------------------------------------------------------------
 * Name: PWR_RegisterLowPowerExitCallback
 * Description: -
 * Parameters: -
 * Return: -
 *---------------------------------------------------------------------------*/
void PWR_RegisterLowPowerExitCallback(pfPWRCallBack_t lowPowerExitCallback)
{
#if (cPWR_FullPowerDownMode)
    gpfPWR_LowPowerExitCb = lowPowerExitCallback;
#endif /* (cPWR_FullPowerDownMode) */
}


/*---------------------------------------------------------------------------
 * Name: PWR_PreventEnterLowPower
 * Description: Forced prevention of entering low-power
 * Parameters: prevent - if TRUE prevents the entering to low-power
 * Return: -
 *---------------------------------------------------------------------------*/
void PWR_PreventEnterLowPower(bool_t prevent)
{
#if (cPWR_FullPowerDownMode)
    mPreventEnterLowPower = prevent;
#endif
}

/*---------------------------------------------------------------------------
 * Name: PWR_IndicateBLEActive
 * Description: Indicate BLE active. This information is used to determine if
 * wake timer needs to be running when going to a power down mode with the 32k
 * clock running: if BLE is active then wake timer is not needed to be
 * running, otherwise it is.
 * Parameters: bBLE_ActiveIndication - TRUE if BLE is active, FALSE if not
 * Return: -
 *---------------------------------------------------------------------------*/
void PWR_IndicateBLEActive(bool_t bBLE_ActiveIndication)
{
#if (cPWR_FullPowerDownMode)
    bBLE_Active = bBLE_ActiveIndication;
#endif
}


void PWR_Start32kCounter(void)
{
    WTIMER_status_t     timer_status;
    if (0 == (SYSCON->AHBCLKCTRLS[0] & SYSCON_AHBCLKCTRLSET0_WAKE_UP_TIMERS_CLK_SET_MASK))
    {
        WTIMER_Init();
    }
    /* Check wake timers */
    timer_status = WTIMER_GetStatusFlags(WTIMER_TIMER0_ID);
    if ( timer_status != WTIMER_STATUS_RUNNING )
    {
        /* was not running yet : start free running count */
        WTIMER_StartTimer(WTIMER_TIMER0_ID, ~0UL);
    }
}


uint32_t PWR_Get32kTimestamp(void)
{
     return WTIMER_ReadTimer(WTIMER_TIMER0_ID);
}

#if (cPWR_FullPowerDownMode)
/*---------------------------------------------------------------------------
 * Name: PWR_SetWakeUpConfig
 * Description:
 * Parameters:
 * Return: -
 *---------------------------------------------------------------------------*/
static void PWR_SetWakeUpConfig(uint32_t set_msk, uint32_t clr_msk)
{
    pwr_wakeup_io &= ~clr_msk;
    pwr_wakeup_io |= set_msk;
    PWR_DBG_LOG("wakesrc=%x", pwr_wakeup_io);
}

/*---------------------------------------------------------------------------
 * Name: PWR_GetWakeUpConfig
 * Description:
 * Parameters:
 * Return: -
 *---------------------------------------------------------------------------*/
static uint32_t PWR_GetWakeUpConfig(void)
{
    return pwr_wakeup_io;
}

/*---------------------------------------------------------------------------
 * Name: vSetWakeUpIoConfig
 * Description:
 * Parameters:
 * Return: -
 *---------------------------------------------------------------------------*/
static void vSetWakeUpIoConfig(void)
{
    uint32_t wkup_src;

    /* Get GPIO wake-up sources from framework GPIO */
    wkup_src = GpioGetIoWakeupSource();
    /* Add wake-up sources registered directly (backwards compatibility) */
    wkup_src |= pwrm_sleep_config.pm_wakeup_io;

    /* Write to LP configuration store */
    PWR_SetWakeUpConfig(wkup_src, (uint32_t)(BIT(22)-1));
}

/* Added from PWRM */
static uint64_t u64GetWakeupSourceConfig(uint32_t u32Mode)
{
    /* Check for WTIMER block enabled, and 32k clock running in sleep */
    if (0 != (u32Mode & PWR_CFG_OSC_ON))
    {
        if (0 != (SYSCON->AHBCLKCTRLS[0] & SYSCON_AHBCLKCTRLSET0_WAKE_UP_TIMERS_CLK_SET(1)))
        {
            WTIMER_status_t     timer_status;
            /* Check wake timers */
            timer_status = WTIMER_GetStatusFlags(WTIMER_TIMER1_ID);
            if ( timer_status == WTIMER_STATUS_RUNNING )
            {
                if ( !(psNextWake && (PWR_E_TIMER_RUNNING == psNextWake->u8Status)))
                {
                    /* Timer running but no wake event queued: incorrect state but
                     * not critical, so carry on */
                }
                pwrm_sleep_config.pm_wakeup_src |= POWER_WAKEUPSRC_WAKE_UP_TIMER1;
            }
            else
            {
                pwrm_sleep_config.pm_wakeup_src &= ~POWER_WAKEUPSRC_WAKE_UP_TIMER1;
            }

            timer_status = WTIMER_GetStatusFlags(WTIMER_TIMER0_ID);
            if ( timer_status == WTIMER_STATUS_RUNNING )
            {
                pwrm_sleep_config.pm_wakeup_src |= POWER_WAKEUPSRC_WAKE_UP_TIMER0;
            }
            else
            {
                pwrm_sleep_config.pm_wakeup_src &= ~POWER_WAKEUPSRC_WAKE_UP_TIMER0;
            }
        }
        else
        {
            /* No wake timers enabled */
            pwrm_sleep_config.pm_wakeup_src &= ~(  POWER_WAKEUPSRC_WAKE_UP_TIMER0
                                                 | POWER_WAKEUPSRC_WAKE_UP_TIMER1);
        }

        if (0 != (SYSCON->AHBCLKCTRLS[0] & SYSCON_AHBCLKCTRL0_RTC(1)))
        {
            if (RTC->CTRL & RTC_CTRL_RTC_EN_MASK)
            {
                pwrm_sleep_config.pm_wakeup_src |=  POWER_WAKEUPSRC_RTC;
            }
            else
            {
                pwrm_sleep_config.pm_wakeup_src &=  ~POWER_WAKEUPSRC_RTC;
            }
        }
    }
    return pwrm_sleep_config.pm_wakeup_src;
}

PWR_teStatus PWR_vWakeUpIO(uint32_t io_mask)
{
    PWR_teStatus status;

    //if ( !(E_AHI_SLEEP_DEEP == s_ePowerMode) )
    if ( io_mask == 0 )
    {
        /* remove io wake up source */
        pwrm_sleep_config.pm_wakeup_io   = 0;
        pwrm_sleep_config.pm_wakeup_src &= ~POWER_WAKEUPSRC_IO;

        status = PWR_E_OK;
    }
    else if ( (io_mask & (~((1<<23)-1))) !=0 )
    {
        /* bit beyond 23 is set */
        status = PWR_E_IO_INVALID;
    }
    else
    {
        pwrm_sleep_config.pm_wakeup_io   = io_mask;
        pwrm_sleep_config.pm_wakeup_src |= POWER_WAKEUPSRC_IO;

        status = PWR_E_OK;
    }

    return status;
}

PWR_teStatus PWR_vWakeUpConfig(uint32_t pwrm_config)
{
    PWR_teStatus status;

    if ( pwrm_config == 0 )
    {
        /* remove io wake up source */
        pwrm_sleep_config.pm_wakeup_io   = 0;
        pwrm_sleep_config.pm_wakeup_src &= ~ (POWER_WAKEUPSRC_IO
                                              | POWER_WAKEUPSRC_NFCTAG
                                              | PWR_ANA_COMP_WAKEUP
                                              | PWR_BOD_WAKEUP);

        status = PWR_E_OK;
    }
    else if ( (pwrm_config & (~((PWR_BOD_WAKEUP<<1)-1))) !=0 )
    {
        /* bit beyond PWRM_BOD_WAKEUP is set */
        status = PWR_E_IO_INVALID;
    }
    else
    {
        uint32_t io_cfg = pwrm_config & ( PWR_NTAG_FD_WAKEUP-1 );
        if ( io_cfg )
        {
            pwrm_sleep_config.pm_wakeup_io   = io_cfg;
            pwrm_sleep_config.pm_wakeup_src |= POWER_WAKEUPSRC_IO;
        }
        if ( pwrm_config & PWR_NTAG_FD_WAKEUP )
        {
            pwrm_sleep_config.pm_wakeup_src |= POWER_WAKEUPSRC_NFCTAG;
        }
        if ( pwrm_config & PWR_ANA_COMP_WAKEUP )
        {
            pwrm_sleep_config.pm_wakeup_src |= POWER_WAKEUPSRC_ANA_COMP;
        }
        if ( pwrm_config & PWR_BOD_WAKEUP )
        {
            pwrm_sleep_config.pm_wakeup_src |= POWER_WAKEUPSRC_SYSTEM;
        }

        status = PWR_E_OK;
    }

    return status;
}


void PWR_vForceRamRetention(uint32_t u32RetainBitmap)
{
    pwrm_force_retention &= ~(PM_CFG_SRAM_ALL_RETENTION << PM_CFG_SRAM_BANK_BIT_BASE);
    pwrm_force_retention |= (u32RetainBitmap & PM_CFG_SRAM_ALL_RETENTION) << PM_CFG_SRAM_BANK_BIT_BASE;
}

void PWR_vAddRamRetention(uint32_t u32Start, uint32_t u32Length)
{
    uint32_t u32RamBanks;

    u32RamBanks = PWRLib_u32RamBanksSpanned(u32Start, u32Length);
    pwrm_force_retention |= (u32RamBanks & PM_CFG_SRAM_ALL_RETENTION) << PM_CFG_SRAM_BANK_BIT_BASE;
}

uint32_t PWR_u32GetRamRetention(void)
{
    return pwrm_force_retention;
}

void PWR_vForceRadioRetention(bool_t bRetain)
{
    if (bRetain)
    {
        pwrm_force_retention |= PM_CFG_RADIO_RET;
    }
    else
    {
        pwrm_force_retention &= ~PM_CFG_RADIO_RET;
    }
}

PWR_teStatus PWR_eScheduleActivity(PWR_tsWakeTimerEvent *psWake,
                                   uint32_t u32TimeMs,
                                   void (*prCallbackfn)(void))
{
    uint32_t              u32CurrentCount;
    PWR_tsWakeTimerEvent *psCurrentNode, *psNextNode;
    uint64_t              u64AdjustedTicks;

    /* Ensure that sleep mode keeps 32k oscillator running */
    if (PWR_GetDeepSleepConfig() & PWR_CFG_OSC_ON)
    {
        if (PWR_E_TIMER_RUNNING == psWake->u8Status)
        {
            return PWR_E_TIMER_RUNNING;
        }

        uint32_t u32Ticks;
        uint32_t freqHz = mHk32k.freq32k>>PWR_FRO32K_CAL_SHIFT;
        u64AdjustedTicks = u32TimeMs;
        u64AdjustedTicks = u64AdjustedTicks * freqHz / 1000;

        if (u64AdjustedTicks > 0xffffffff)
        {
            /* Overflowed, so limit to maximum uint32 value */
            u32Ticks = 0xffffffff;
        }
        else
        {
            u32Ticks = (uint32_t)u64AdjustedTicks;
        }

        psWake->psNext = NULL;
        psWake->prCallbackfn = prCallbackfn;
        psWake->u8Status = PWR_E_TIMER_RUNNING;

        /* Enable WTIMER if needed. Do not clear flag or stop the timer in
         * case a timer was already running */
        if (0 == (SYSCON->AHBCLKCTRLS[0] & SYSCON_AHBCLKCTRLSET0_WAKE_UP_TIMERS_CLK_SET_MASK))
        {
            WTIMER_Init();
        }
        WTIMER_EnableInterrupts(WTIMER_TIMER1_ID);

        if (psNextWake == NULL)
        {
            // List currently empty, add at the head
            psNextWake           = psWake;
            psWake->u32TickDelta = u32Ticks;
            WTIMER_StartTimer(WTIMER_TIMER1_ID, u32Ticks);
            s_bWakeTimerActive   = TRUE;

            return PWR_E_OK;
        }

        // catch time remaining on current timer
        u32CurrentCount = WTIMER_ReadTimerSafe(WTIMER_TIMER1_ID);
        // stop the timer to prevent timeouts while editing the list
        WTIMER_StopTimer(WTIMER_TIMER1_ID);

        if (u32Ticks < u32CurrentCount)
        {
            // Inserting at head of queue
            // Adjust time remaining on interrupted timer
            psWake->u32TickDelta       = u32Ticks;
            psNextWake->u32TickDelta   = u32CurrentCount - u32Ticks;
            psWake->psNext             = psNextWake;
            psNextWake                 = psWake;
            // start the timer
            WTIMER_StartTimer(WTIMER_TIMER1_ID, u32Ticks);
            s_bWakeTimerActive         = TRUE;

            return PWR_E_OK;
        }

        // Find where in the list to insert new wake point
        for (psCurrentNode = psNextWake; psCurrentNode != NULL;
                    psCurrentNode = psCurrentNode->psNext)
        {
            if (psCurrentNode == psNextWake)
            {
                u32Ticks -= u32CurrentCount;
            } else
            {
                u32Ticks -= psCurrentNode->u32TickDelta;
            }

            if (psCurrentNode->psNext == NULL)
            {
                // Add at end of list
                psWake->u32TickDelta   = u32Ticks;
                psWake->psNext         = NULL;
                psCurrentNode->psNext  = psWake;
                // Restart the interrupted timer
                WTIMER_StartTimer(WTIMER_TIMER1_ID, u32CurrentCount);
                s_bWakeTimerActive     = TRUE;

                return PWR_E_OK;
            }
            else
            {
                psNextNode = psCurrentNode->psNext;
                if (u32Ticks < psNextNode->u32TickDelta)
                {
                    // Insert in to middle of list
                    // adjust delta time of event after insertion point
                    psWake->u32TickDelta       = u32Ticks;
                    psNextNode->u32TickDelta  -= u32Ticks;
                    psWake->psNext = psNextNode;
                    psCurrentNode->psNext      = psWake;
                    // Restart the interrupted timer
                    WTIMER_StartTimer(WTIMER_TIMER1_ID, u32CurrentCount);
                    s_bWakeTimerActive         = TRUE;

                    return PWR_E_OK;
                }
            }
        }
    }
    return PWR_E_TIMER_INVALID;
}

void PWR_vWakeInterruptCallback(void)
{
    if (psNextWake == NULL)
    {
        WTIMER_StopTimer(WTIMER_TIMER1_ID);
        s_bWakeTimerActive = FALSE;
    }
    else
    {
        // If there is a callback function call it
        if (psNextWake->prCallbackfn != NULL)
        {
            psNextWake->prCallbackfn();
        }

        // Free up the timer for future use
        psNextWake->u8Status  = PWR_E_TIMER_FREE;
        // Next wake event in the list
        psNextWake            = psNextWake->psNext;

        if (psNextWake != NULL)
        {
            // Start the timer for the next next
            WTIMER_StartTimer(WTIMER_TIMER1_ID, psNextWake->u32TickDelta);
            s_bWakeTimerActive = TRUE;
        }
        else
        {
            // No more scheculed events, stop the timer
            WTIMER_StopTimer(WTIMER_TIMER1_ID);
            s_bWakeTimerActive = FALSE;
        }
    }
}

void PWR_vColdStart(void)
{
    reset_cause_t reset_cause;

    vAppRegisterPWRCallbacks();

    reset_cause = POWER_GetResetCause();

    /* if coming from power down mode, stop the timer if running and clear interrupts (No WAKE Cb to call in cold start)
     * if coming from other reset cause, reset the Wakeup timer
     *Do not reset the Wake timer if coming from power down mode */
    if ( reset_cause == RESET_WAKE_PD )
    {
        // TODO: need to check whether the timer has been correctly initialized previously (AHBCLK ON)
        WTIMER_StopTimer(WTIMER_TIMER1_ID);
    }
    else
    {
        RESET_SetPeripheralReset(kWKT_RST_SHIFT_RSTn);
        RESET_ClearPeripheralReset(kWKT_RST_SHIFT_RSTn);
    }

    if ( (reset_cause == RESET_WAKE_DEEP_PD) || (reset_cause == RESET_WAKE_PD) )
    {
        // Call any registered callbacks -
        if (gpfPWR_LowPowerExitCb != NULL)
        {
            gpfPWR_LowPowerExitCb();
        }
    }
}
#else
void PWR_vWakeInterruptCallback(void)
{
    /* Empty content if cPWR_FullPowerDownMode is disabled */
}
#endif


void PWR_SetDeepSleepTimeInMs(uint32_t deepSleepTimeMs)
{
#if (RTOS_TICKLESS)
    PWR_DBG_LOG("timeMs=%d", deepSleepTimeMs);
    if(deepSleepTimeMs != 0)
    {
        mPWR_DeepSleepTimeMs = deepSleepTimeMs;
        mPWR_DeepSleepTimeUpdated = TRUE;
    }
#else
    (void) deepSleepTimeMs;
#endif /* (cPWR_UsePowerDownMode) */
}

#if (RTOS_TICKLESS)
/*---------------------------------------------------------------------------
* Name: PWR_GetTotalSleepDurationMS
* Description: -
* Parameters: -
* Return: -
*---------------------------------------------------------------------------*/
uint32_t PWR_GetTotalSleepDurationMs(uint32_t start_of_sleep)
{

    int32_t time = 0;
    OSA_InterruptDisable();
#ifdef USE_WTIMER
    int time_delta; /* signed */
    uint32_t currentSleepTime;

    currentSleepTime = PWR_Get32kTimestamp();
    time_delta = (start_of_sleep - currentSleepTime);
    /* time_delta is a number of 32kHz ticks: convert to seconds */
    time = TICKS32K_TO_MILLISECONDS(time_delta);
#else
    /* the counter is counting down so previous value is greater.
     * already expressed in 1kHz ticks */
    time = (int32_t)start_of_sleep - (int32_t)RTC_GetWakeupCount(RTC);
#endif

    OSA_InterruptEnable();

    /* If time is negative, that means power down was aborted and the wake value
     * didn't get set. Set to zero in this case to avoid corruption of the next
     * daeadline update in the tickless hook. */
    if (time < 0)
    {
        time = 0;
    }

    PWR_DBG_LOG("timeMs=%d", time);
    return (uint32_t)time;

}


/*---------------------------------------------------------------------------
* Name: PWR_RTCSetWakeupTimeMs
* Description: -
* Parameters: wakeupTimeMs: New wakeup time in milliseconds
* Return: -
*---------------------------------------------------------------------------*/
void PWR_RTCSetWakeupTimeMs (uint32_t wakeupTimeMs)
{
    PWR_DBG_LOG("timer ms=%d", wakeupTimeMs);
    /* Countdown of 1kHz clock */
    RTC_SetWakeupCount(RTC, wakeupTimeMs);
}

static void PWR_RTCWakeupStart(void)
{
    PWR_DBG_LOG("");
    uint32_t enabled_interrupts = RTC_GetEnabledInterrupts(RTC);
    enabled_interrupts |= kRTC_WakeupInterruptEnable;
    RTC_EnableInterrupts(RTC, enabled_interrupts);
}
#endif /* RTOS_TICKLESS */
