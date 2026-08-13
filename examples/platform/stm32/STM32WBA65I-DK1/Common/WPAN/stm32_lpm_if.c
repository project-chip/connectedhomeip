/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License,
 * Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy
 * of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to
 * in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *
 * limitations under the License.
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
#include "arm_compat.h"
#endif /*(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) */
#include "RTDebug.h"
#include "app_sys.h"
#include "cmsis_compiler.h"
#include "main.h"
#include "peripheral_init.h"
#include "scm.h"
#include "stm32_lpm.h"
#include "stm32_lpm_if.h"
#include "stm32wbaxx.h"
#include "stm32wbaxx_hal_pwr.h"
#include "stm32wbaxx_ll_icache.h"
#include "utilities_common.h"
#if (CFG_RT_DEBUG_DTB == 1)
#include "RTDebug_dtb.h"
#endif /* CFG_RT_DEBUG_DTB */
#include "assert.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
#if (CFG_LPM_WAKEUP_TIME_PROFILING == 1)
#define LL_DEEPSLEEP_EXIT_TIME_US 230    /* Time in us needed for Link Layer deepsleep exit */
#define DEVICE_WAKEUP_STANDBY_TIME_US 60 /* Time in us needed by the device to exit from standby */
#if defined(RCC_LSI2_SUPPORT)
#define LSI2_FREQ_WORST_VALUE 24000   /* Worst value of LSI2 frequency */
#endif                                /* RCC_LSI2_SUPPORT */
#define RTOS_EXTRA_WAKEUP_TIME_US 100 /* Additional time in us to wakeup in context of an RTOS */

#endif /* CFG_LPM_WAKEUP_TIME_PROFILING */
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/**
 * @brief Array of LPM driver configurations.
 *
 * This array contains the configurations for different low power modes
 * and the corresponding functions to handle those modes.
 *
 *@note It must be ordered from the less efficient (index 0)
 *      to the most efficient low power mode
 */
const UTIL_LPM_Driver_fp UTIL_LPM_Driver[] = { LPM_IdleMode, LPM_SleepMode,
#if (CFG_LPM_STOP1_SUPPORTED == 1)
                                               LPM_Stop1Mode,
#endif /* (CFG_LPM_STOP1_SUPPORTED == 1) */
#if (CFG_LPM_STOP2_SUPPORTED == 1)
                                               LPM_Stop2Mode,
#endif /* (CFG_LPM_STOP2_SUPPORTED == 1) */
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
                                               LPM_StandbyMode
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1)  */
};

/**
 * @brief Number of LPM drivers.
 *
 * This constant holds the number of entries in the UTIL_LPM_Driver array.
 */
const uint32_t UTIL_LPM_Driver_num = sizeof(UTIL_LPM_Driver) / sizeof(UTIL_LPM_Driver_fp);

/**
 * @brief Assertion to ensure at least one driver is registered in interface file.
 */
static_assert(sizeof(UTIL_LPM_Driver) != 0, "at least one LPM driver is required");
/* Check not too many drivers are registered in interface file */

/**
 * @brief Assertion to ensure registered drivers are within boundaries.
 */
static_assert((sizeof(UTIL_LPM_Driver) / sizeof(UTIL_LPM_Driver_fp)) <= UTIL_LPM_DRIVER_MAX_NUM, "too many LPM drivers registered");

/**
 * @brief Assertion to ensure drivers and associated enum type are aligned
 */
static_assert((sizeof(UTIL_LPM_Driver) / sizeof(UTIL_LPM_Driver_fp)) == UTIL_LPM_NUM_MODES,
              "UTIL_LPM_Mode_t enum not aligned with UTIL_LPM_Driver size");

/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* Variable to store the MainStackPointer before entering standby wfi */
uint32_t backup_MSP;
uint32_t backup_MSPLIM;
uint32_t backup_PSP;
uint32_t backup_PSPLIM;
uint32_t backup_CONTROL;
uint32_t backup_prio_SysTick_IRQn;
uint32_t backup_prio_SVCall_IRQn;
uint32_t backup_prio_PendSV_IRQn;
static uint32_t boot_after_standby;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Private variables ---------------------------------------------------------*/
#if (CFG_SCM_SUPPORTED == 0)
/* If SCM is not supported, SRAM handles for waitsate configurations are defined here */
static RAMCFG_HandleTypeDef sram1_ns = {
    RAMCFG_SRAM1,           /* Instance */
    HAL_RAMCFG_STATE_READY, /* RAMCFG State */
    0U,                     /* RAMCFG Error Code */
};

static RAMCFG_HandleTypeDef sram2_ns = {
    RAMCFG_SRAM2,           /* Instance */
    HAL_RAMCFG_STATE_READY, /* RAMCFG State */
    0U,                     /* RAMCFG Error Code */
};
#endif /* CFG_SCM_SUPPORTED */

#if (CFG_LPM_WAKEUP_TIME_PROFILING == 1)
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
static uint32_t lpm_wakeup_time_standby = 0;
#if defined(RCC_LSI2_SUPPORT)
static uint8_t lpm_profiling_started_lsi2 = 0;
#endif /* RCC_LSI2_SUPPORT */
#endif /* CFG_LPM_STANDBY_SUPPORTED */
#endif /* CFG_LPM_WAKEUP_TIME_PROFILING */
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Private function prototypes -----------------------------------------------*/
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
void Standby_Restore_GPIO(void);
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) */
#if (CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP1_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1)
static void Enter_Stop_Standby_Mode(void);
static void Exit_Stop_Standby_Mode(void);
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP1_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1) */
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
extern void LINKLAYER_PLAT_NotifyWFIExit(void);
extern void LINKLAYER_PLAT_NotifyWFIEnter(void);
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

#if (CFG_LPM_STANDBY_SUPPORTED == 1)
__WEAK OPTIMIZED void Standby_Restore_GPIO(void)
{
    uint32_t temp;

    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX/RX clock */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    // ---------------------------------------------------------------------------

    /* Configure GPIOA_MODER 15:13 AF (JTAG), 12:0 Input */
    LL_GPIO_WriteReg(GPIOA, MODER,
                     ((LL_GPIO_MODE_ALTERNATE << (POSITION_VAL(LL_GPIO_PIN_15) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_ALTERNATE << (POSITION_VAL(LL_GPIO_PIN_14) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_ALTERNATE << (POSITION_VAL(LL_GPIO_PIN_13) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_12) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_11) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_10) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_9) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_8) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_7) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_6) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_5) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_4) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_3) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_2) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_1) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_0) * GPIO_MODER_MODE1_Pos))));

    __ASM("mov r0, r0"); /* Delay to allow GPIOx_IDR.IDy to be updated after GPIOx_MODER is set to Input */
    __ASM("mov r0, r0"); /* Delay to allow GPIOx_IDR.IDy to be updated after GPIOx_MODER is set to Input */

    temp = LL_GPIO_ReadInputPort(GPIOA);
    LL_GPIO_WriteOutputPort(GPIOA, temp); /* Restore Port A output drive levels */

    /* GPIOA_MODER set to reset value */
    LL_GPIO_WriteReg(GPIOA, MODER, 0xABFFFFFF);

    // ---------------------------------------------------------------------------

    /* Configure GPIOB_MODER 4:3 AF (JTAG), 15:5, 2:0 In */
    LL_GPIO_WriteReg(GPIOB, MODER,
                     ((LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_15) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_14) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_13) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_12) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_11) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_10) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_9) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_8) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_7) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_6) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_5) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_ALTERNATE << (POSITION_VAL(LL_GPIO_PIN_4) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_ALTERNATE << (POSITION_VAL(LL_GPIO_PIN_3) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_2) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_1) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_0) * GPIO_MODER_MODE1_Pos))));

    __ASM("mov r0, r0"); // Delay to allow GPIOx_IDR.IDy to be updated after GPIOx_MODER is set to Input
    __ASM("mov r0, r0"); // Delay to allow GPIOx_IDR.IDy to be updated after GPIOx_MODER is set to Input

    temp = LL_GPIO_ReadInputPort(GPIOB);
    LL_GPIO_WriteOutputPort(GPIOB, temp); // Restore Port B output drive levels

    /* GPIOB_MODER set to reset value */
    LL_GPIO_WriteReg(GPIOB, MODER, 0xFFFFFEBF);

    // ---------------------------------------------------------------------------

    /* Configure GPIOC_MODER 15:13 In */
    LL_GPIO_WriteReg(GPIOC, MODER,
                     ((LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_15) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_14) * GPIO_MODER_MODE1_Pos)) |
                      (LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_13) * GPIO_MODER_MODE1_Pos))));

    __ASM("mov r0, r0"); // Delay to allow GPIOx_IDR.IDy to be updated after GPIOx_MODER is set to Input
    __ASM("mov r0, r0"); // Delay to allow GPIOx_IDR.IDy to be updated after GPIOx_MODER is set to Input

    temp = LL_GPIO_ReadInputPort(GPIOC);
    LL_GPIO_WriteOutputPort(GPIOC, temp); // Restore Port C output drive levels

    /* GPIOC_MODER set to reset value */
    LL_GPIO_WriteReg(GPIOC, MODER, 0xFC000000);

    // ---------------------------------------------------------------------------

    /* Configure GPIOH_MODER 3 In */
    LL_GPIO_WriteReg(GPIOH, MODER, ((LL_GPIO_MODE_INPUT << (POSITION_VAL(LL_GPIO_PIN_3) * GPIO_MODER_MODE1_Pos))));

    __ASM("mov r0, r0"); // Delay to allow GPIOx_IDR.IDy to be updated after GPIOx_MODER is set to Input
    __ASM("mov r0, r0"); // Delay to allow GPIOx_IDR.IDy to be updated after GPIOx_MODER is set to Input

    temp = LL_GPIO_ReadInputPort(GPIOH);
    LL_GPIO_WriteOutputPort(GPIOH, temp); // Restore Port H output drive levels

    /* GPIOH_MODER set to reset value */
    LL_GPIO_WriteReg(GPIOH, MODER, 0x0000C000);

    // ---------------------------------------------------------------------------
}
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) */

#if (CFG_SCM_SUPPORTED != 1) &&                                                                                                    \
    ((CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP1_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1))
OPTIMIZED static void Clock_Switching(void)
{
    /* Activate HSE clock */
    LL_RCC_HSE_Enable();
    while (LL_RCC_HSE_IsReady() == 0)
        ;
    /* Apply PWR VOS1 power level */
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1); /* Double write for flag readiness to cope with hw latency */
    while (LL_PWR_IsActiveFlag_VOS() == 0)
        ;

    /* Switch HSE frequency from HSE16 to HSE32 */
    LL_RCC_HSE_DisablePrescaler();

    /* Switch CPU system clock to HSE */
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSE);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSE)
        ;

    /* Apply HSE32 compatible waitstates */
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_0);
    while (__HAL_FLASH_GET_LATENCY() != FLASH_LATENCY_0)
        ;
    HAL_RAMCFG_ConfigWaitState(&sram1_ns, RAMCFG_WAITSTATE_0);
    HAL_RAMCFG_ConfigWaitState(&sram2_ns, RAMCFG_WAITSTATE_0);

    /* Set HDIV 5 */
    LL_RCC_SetAHB5Divider(LL_RCC_AHB5_DIVIDER_1); /* divided by 1 */

    /* Switched to HSE, so disable HSI */
    LL_RCC_HSI_Disable();

    /* Ensure time base clock coherency */
    SystemCoreClockUpdate();
}
#endif /* (CFG_SCM_SUPPORTED != 1) && ((CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP1_SUPPORTED == 1) ||                       \
          (CFG_LPM_STOP2_SUPPORTED == 1)) */

#if (CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP1_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1)
OPTIMIZED static void Enter_Stop_Standby_Mode(void)
{
    /* Disabling ICACHE */
    LL_ICACHE_Disable();
#if defined(STM32WBAXX_SI_CUT1_0)
    /* Wait until ICACHE_SR.BUSYF is cleared */
    while (LL_ICACHE_IsActiveFlag_BUSY() == 1U)
        ;

    /* Wait until ICACHE_SR.BSYENDF is set */
    while (LL_ICACHE_IsActiveFlag_BSYEND() == 0U)
        ;
#endif /* STM32WBAXX_SI_CUT1_0 */

#if (CFG_SCM_SUPPORTED == 1)
    scm_setwaitstates(LP);
#else
    /* SCM module is not supported, apply low power compatible waitstates */
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_3);
    while (__HAL_FLASH_GET_LATENCY() != FLASH_LATENCY_3)
        ;
    HAL_RAMCFG_ConfigWaitState(&sram1_ns, RAMCFG_WAITSTATE_1);
    HAL_RAMCFG_ConfigWaitState(&sram2_ns, RAMCFG_WAITSTATE_1);
#endif /* CFG_SCM_SUPPORTED */

    LL_LPM_EnableDeepSleep();

    while (LL_PWR_IsActiveFlag_ACTVOS() == 0)
        ;
}

OPTIMIZED static void Exit_Stop_Standby_Mode(void)
{
    LL_ICACHE_Enable();
    while (LL_ICACHE_IsEnabled() == 0U)
        ;
#if (CFG_SCM_SUPPORTED == 1)
    if (LL_PWR_IsActiveFlag_STOP() == 1U)
    {
        /* SCM HSE BEGIN */
        /* Clear SW_HSERDY, if needed */
        if (isRadioActive() == SCM_RADIO_NOT_ACTIVE)
        {
            SCM_HSE_Clear_SW_HSERDY();
        }
        /* SCM HSE END */
        scm_setup();
    }
    else
    {
        scm_setwaitstates(RUN);
    }
#else
    if (LL_PWR_IsActiveFlag_STOP() == 1U)
    {
        Clock_Switching();
    }
    else
    {
        /* Apply waitsates for HSE32 configuration */
        __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_0);
        while (__HAL_FLASH_GET_LATENCY() != FLASH_LATENCY_0)
            ;
        HAL_RAMCFG_ConfigWaitState(&sram1_ns, RAMCFG_WAITSTATE_0);
        HAL_RAMCFG_ConfigWaitState(&sram2_ns, RAMCFG_WAITSTATE_0);
    }
#endif /* CFG_SCM_SUPPORTED */
}
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP1_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1) */

#if (CFG_LPM_STANDBY_SUPPORTED == 1)
OPTIMIZED static void PWR_EnterOffMode(void)
{
    SYSTEM_DEBUG_SIGNAL_SET(LOW_POWER_STANDBY_MODE_ENTER);

    /* USER CODE BEGIN PWR_EnterOffMode_1 */

    /* USER CODE END PWR_EnterOffMode_1 */

    /* Notify the Link Layer platform layer the system will enter in WFI
     * and AHB5 clock may be turned of regarding the 2.4Ghz radio state
     */
    LINKLAYER_PLAT_NotifyWFIEnter();

    /*
     * There is no risk to clear all the WUF here because in the current implementation, this API is called
     * in critical section. If an interrupt occurs while in that critical section before that point,
     * the flag is set and will be cleared here but the system will not enter Off Mode
     * because an interrupt is pending in the NVIC. The ISR will be executed when moving out
     * of this critical section
     */

    Enter_Stop_Standby_Mode();

    /* Set low power mode to standby */
    LL_PWR_SetPowerMode(LL_PWR_MODE_STANDBY);

    /* This option is used to ensure that store operations are completed */
#if defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
    __force_stores();
#endif /*(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050) */

    backup_CONTROL = __get_CONTROL();

    /* Check if Stack Pointer if pointing to PSP */
    if ((backup_CONTROL & CONTROL_SPSEL_Msk) == CONTROL_SPSEL_Msk)
    {
        __set_CONTROL(__get_CONTROL() & ~CONTROL_SPSEL_Msk); /* switch SP to MSP */
    }

    /* Save selected CPU peripheral regisers */
    backup_PSP               = __get_PSP();
    backup_PSPLIM            = __get_PSPLIM();
    backup_MSPLIM            = __get_MSPLIM();
    backup_prio_SysTick_IRQn = NVIC_GetPriority(SysTick_IRQn);
    backup_prio_SVCall_IRQn  = NVIC_GetPriority(SVCall_IRQn);
    backup_prio_PendSV_IRQn  = NVIC_GetPriority(PendSV_IRQn);
    backup_system_register();

    SYSTEM_DEBUG_SIGNAL_RESET(LOW_POWER_STANDBY_MODE_ENTER);
    SYSTEM_DEBUG_SIGNAL_SET(LOW_POWER_STANDBY_MODE_ACTIVE);

    /* Save Cortex general purpose registers on stack and call WFI instruction */
    CPUcontextSave();

    /* At this point, system comes out of standby. Restore selected CPU peripheral registers */
    restore_system_register();
#if defined(STM32WBAXX_SI_CUT1_0)
    SYS_WAITING_CYCLES_25();
#endif /* STM32WBAXX_SI_CUT1_0 */

    __set_MSPLIM(backup_MSPLIM);
    __set_PSPLIM(backup_PSPLIM);
    __set_PSP(backup_PSP);
    __set_CONTROL(backup_CONTROL); /* SP may switch back to PSP */

    if (1UL == boot_after_standby)
    {
#if (CFG_RT_DEBUG_DTB == 1) || (CFG_RT_DEBUG_GPIO_MODULE == 1)
        Standby_Restore_GPIO();
#endif /* ((CFG_RT_DEBUG_DTB == 1) || (CFG_RT_DEBUG_GPIO_MODULE == 1) */
#if (CFG_RT_DEBUG_DTB == 1)
        RT_DEBUG_DTBInit();
        RT_DEBUG_DTBConfig();
#endif /* CFG_RT_DEBUG_DTB */
#if (CFG_RT_DEBUG_GPIO_MODULE == 1)
        RT_DEBUG_GPIO_Init();
#endif /* (CFG_RT_DEBUG_GPIO_MODULE == 1) */
    }

    SYSTEM_DEBUG_SIGNAL_RESET(LOW_POWER_STANDBY_MODE_ACTIVE);

    /* USER CODE BEGIN PWR_EnterOffMode_2 */

    /* USER CODE END PWR_EnterOffMode_2 */
}
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) */

#if (CFG_LPM_STANDBY_SUPPORTED == 1)
OPTIMIZED static void PWR_ExitOffMode(void)
{
    SYSTEM_DEBUG_SIGNAL_SET(LOW_POWER_STANDBY_MODE_EXIT);

    /* USER CODE BEGIN PWR_ExitOffMode_1 */

    /* USER CODE END PWR_ExitOffMode_1 */

    if (1UL == boot_after_standby)
    {
        boot_after_standby = 0;
#if (CFG_SCM_SUPPORTED == 1)
        /* SCM HSE BEGIN */
        SCM_HSE_Clear_SW_HSERDY();
        /* SCM HSE END */
#endif /* CFG_SCM_SUPPORTED */
        HAL_NVIC_SetPriority(RADIO_INTR_NUM, RADIO_INTR_PRIO_LOW, 0);
        HAL_NVIC_EnableIRQ(RADIO_INTR_NUM);
        HAL_NVIC_SetPriority(RADIO_SW_LOW_INTR_NUM, RADIO_SW_LOW_INTR_PRIO, 0);
        HAL_NVIC_EnableIRQ(RADIO_SW_LOW_INTR_NUM);
        HAL_NVIC_SetPriority(RCC_IRQn, RCC_INTR_PRIO, 0);
        HAL_NVIC_EnableIRQ(RCC_IRQn);
        HAL_NVIC_SetPriority(RTC_IRQn, 0x07, 0);
        HAL_NVIC_EnableIRQ(RTC_IRQn);
        NVIC_SetPriority(SysTick_IRQn, backup_prio_SysTick_IRQn);
        NVIC_EnableIRQ(SysTick_IRQn);
        NVIC_SetPriority(SVCall_IRQn, backup_prio_SVCall_IRQn);
        NVIC_EnableIRQ(SVCall_IRQn);
        NVIC_SetPriority(PendSV_IRQn, backup_prio_PendSV_IRQn);
        NVIC_EnableIRQ(PendSV_IRQn);

        /*
         ***********************************
         * Restore SoC HW configuration
         ***********************************
         */

        /* Enable AHB5ENR peripheral clock (bus CLK) */
        __HAL_RCC_RADIO_CLK_ENABLE();

        /* Notify the Link Layer platform layer the system exited WFI
         * and AHB5 clock may be resynchronized as is may have been
         * turned of during low power mode entry.
         */
        LINKLAYER_PLAT_NotifyWFIExit();

        /* USER CODE BEGIN PWR_ExitOffMode_2 */

        /* USER CODE END PWR_ExitOffMode_2 */

        /* Apply Prefetch configuration is enabled */
#if (PREFETCH_ENABLE != 0U)
        __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
#endif /* PREFETCH_ENABLE */

        MX_StandbyExit_PeripheralInit();

        /* Restore system clock configuration */
#if (CFG_SCM_SUPPORTED == 1)
        scm_standbyexit();
#else
        Clock_Switching();
#endif /* CFG_SCM_SUPPORTED */

        /* Enable RTC peripheral clock */
        LL_PWR_EnableBkUpAccess();
        __HAL_RCC_RTCAPB_CLK_ENABLE();

        /* Important note: at this point, all the IOs configuration is done */

        /* Clear all IOs retention status  */
        HAL_PWREx_DisableStandbyRetainedIOState(PWR_GPIO_A, PWR_GPIO_PIN_MASK);
        HAL_PWREx_DisableStandbyRetainedIOState(PWR_GPIO_B, PWR_GPIO_PIN_MASK);
        HAL_PWREx_DisableStandbyRetainedIOState(PWR_GPIO_C, PWR_GPIO_PIN_MASK);
        HAL_PWREx_DisableStandbyRetainedIOState(PWR_GPIO_H, PWR_GPIO_PIN_MASK);

#if (CFG_LPM_WAKEUP_TIME_PROFILING == 1)
        if (LPM_is_wakeup_time_profiling_done() == 0)
        {
            /* Compute amount of time spent to start from standby wakeup */
            lpm_wakeup_time_standby = SysTick->LOAD;
            lpm_wakeup_time_standby -= SysTick->VAL; /* Compute time the sysTick has counted */
            lpm_wakeup_time_standby *= 1000000U;
            if (LL_RCC_GetSystickClockSource() == LL_RCC_SYSTICK_CLKSOURCE_LSE)
            {
                lpm_wakeup_time_standby /= LSE_VALUE;
            }
#if defined(RCC_LSI2_SUPPORT)
            else if (LL_RCC_GetSystickClockSource() == LL_RCC_SYSTICK_CLKSOURCE_LSI)
            {
                lpm_wakeup_time_standby /= LSI2_FREQ_WORST_VALUE;
            }
#endif /* RCC_LSI2_SUPPORT */
            else
            {
                /* Such situation shall not happen, Systick clock source has changed during wakeup time profiling */
                Error_Handler();
            }
            lpm_wakeup_time_standby += LL_DEEPSLEEP_EXIT_TIME_US + DEVICE_WAKEUP_STANDBY_TIME_US;
            lpm_wakeup_time_standby += RTOS_EXTRA_WAKEUP_TIME_US; /* Additional time to wakeup in context of an RTOS */

#if defined(RCC_LSI2_SUPPORT)
            /** If LSI2 is used for profiling, since the profile value is based on the worst
             * case for LSI2 frequency, the minimum value between the value profiled
             * and the default value is considered for lpm_wakeup_time_standby
             */
            if (LL_RCC_GetSystickClockSource() == LL_RCC_SYSTICK_CLKSOURCE_LSI)
            {
                lpm_wakeup_time_standby = MIN(lpm_wakeup_time_standby, CFG_LPM_STDBY_WAKEUP_TIME);
            }
#endif /* RCC_LSI2_SUPPORT */
            /* USER CODE BEGIN CFG_LPM_WAKEUP_TIME_PROFILING_1 */

            /* USER CODE END CFG_LPM_WAKEUP_TIME_PROFILING_1 */

            APP_SYS_SetWakeupOffset(lpm_wakeup_time_standby);
#if defined(RCC_LSI2_SUPPORT)
            /* Disable LSI2 if used for wakeup time profiling */
            if (lpm_profiling_started_lsi2 != 0)
            {
                uint8_t pwrclkchanged = 0;
                if (__HAL_RCC_PWR_IS_CLK_ENABLED() != 1U)
                {
                    __HAL_RCC_PWR_CLK_ENABLE();
                    pwrclkchanged = 1;
                }

                /* Enable write access to Backup domain */
                LL_PWR_EnableBkUpAccess();
                while (LL_PWR_IsEnabledBkUpAccess() == 0UL)
                    ;

                LL_RCC_LSI2_Disable();

                if (pwrclkchanged == 1)
                {
                    __HAL_RCC_PWR_CLK_DISABLE();
                }
            }
#endif /* RCC_LSI2_SUPPORT */
            /* Disable SysTick Timer */
            CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);

            /* Put back user sysTick clock source settings */
            /* Select SysTick source clock */
            HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_LSE);

            /* Initialize SysTick */
            HAL_StatusTypeDef hal_status;
            hal_status = HAL_InitTick(TICK_INT_PRIORITY);
            if (hal_status != HAL_OK)
            {
                assert_param(0);
            }
        }
#endif /* CFG_LPM_WAKEUP_TIME_PROFILING */
    }
    else
    {
        Exit_Stop_Standby_Mode();
    }

    /* USER CODE BEGIN PWR_ExitOffMode_3 */

    /* USER CODE END PWR_ExitOffMode_3 */

    SYSTEM_DEBUG_SIGNAL_RESET(LOW_POWER_STANDBY_MODE_EXIT);
}
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) */

#if (CFG_LPM_STOP2_SUPPORTED == 1)
OPTIMIZED static void PWR_EnterStop2Mode(void)
{
    SYSTEM_DEBUG_SIGNAL_SET(LOW_POWER_STOP2_MODE_ENTER);

    /* USER CODE BEGIN PWR_EnterStop2Mode_1 */

    /* USER CODE END PWR_EnterStop2Mode_1 */

    /* Notify the Link Layer platform layer the system will enter in WFI
     * and AHB5 clock may be turned of regarding the 2.4Ghz radio state
     */
    LINKLAYER_PLAT_NotifyWFIEnter();

    /*
     * There is no risk to clear all the WUF here because in the current implementation, this API is called
     * in critical section. If an interrupt occurs while in that critical section before that point,
     * the flag is set and will be cleared here but the system will not enter Off Mode
     * because an interrupt is pending in the NVIC. The ISR will be executed when moving out
     * of this critical section
     */

    Enter_Stop_Standby_Mode();

    /* Set low power mode to stop2 */
    LL_PWR_SetPowerMode(LL_PWR_MODE_STOP2);

    /* Set unretained peripherals GPIOs to output mode */
    if (READ_BIT(SPI3->CFG2, SPI_CFG2_MASTER) != 0UL)
    {
        /* SPI3 SCK */
        if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0) == 1UL)
        {
            LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_0);
        }
        else
        {
            LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_0);
        }
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_OUTPUT);
        /* SPI3 MOSI */
        LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
    }
    else
    {
        /* SPI3 MISO */
        LL_GPIO_SetPinMode(GPIO, LL_GPIO_PIN_, LL_GPIO_MODE_OUTPUT);
    }
    /* USART1 TX */
    LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_12);
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);
    /* USART2 TX */
    LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_12, LL_GPIO_MODE_OUTPUT);

    SYSTEM_DEBUG_SIGNAL_RESET(LOW_POWER_STOP2_MODE_ENTER);
    SYSTEM_DEBUG_SIGNAL_SET(LOW_POWER_STOP2_MODE_ACTIVE);

    /* Wait for interrupt */
    __WFI();

    SYSTEM_DEBUG_SIGNAL_RESET(LOW_POWER_STOP2_MODE_ACTIVE);

    /* USER CODE BEGIN PWR_EnterStop2Mode_2 */

    /* USER CODE END PWR_EnterStop2Mode_2 */
}
#endif /* (CFG_LPM_STOP2_SUPPORTED == 1) */

#if (CFG_LPM_STOP2_SUPPORTED == 1)
OPTIMIZED static void PWR_ExitStop2Mode(void)
{
    SYSTEM_DEBUG_SIGNAL_SET(LOW_POWER_STOP2_MODE_EXIT);

    /* USER CODE BEGIN PWR_ExitStop2Mode_1 */

    /* USER CODE END PWR_ExitStop2Mode_1 */

    /* Notify the Link Layer platform layer the system exited WFI
     * and AHB5 clock may be resynchronized as is may have been
     * turned of during low power mode entry.
     */
    LINKLAYER_PLAT_NotifyWFIExit();

    /* effectively gone in stop2 */
    if (LL_PWR_IsActiveFlag_STOP2() == 1U)
    {

        LL_ICACHE_Enable();
        while (LL_ICACHE_IsEnabled() == 0U)
            ;

#if (CFG_SCM_SUPPORTED == 1)
        /* SCM HSE BEGIN */
        SCM_HSE_Clear_SW_HSERDY();
        /* SCM HSE END */
#endif /* (CFG_SCM_SUPPORTED == 1) */

        /* Restore unretained peripherals */
        MX_Stop2Exit_PeripheralInit();

#if (CFG_SCM_SUPPORTED == 1)
        scm_standbyexit();
#else
        Clock_Switching();
#endif /* (CFG_SCM_SUPPORTED == 1) */
    }
    else
    {
        /* Restore unretained peripherals GPIOs to alternate mode */
        if (READ_BIT(SPI3->CFG2, SPI_CFG2_MASTER) != 0UL)
        {
            /* SPI3 SCK */
            LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_ALTERNATE);
            /* SPI3 MOSI */
            LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
        }
        else
        {
            /* SPI3 MISO */
            LL_GPIO_SetPinMode(GPIO, LL_GPIO_PIN_, LL_GPIO_MODE_ALTERNATE);
        }
        /* USART1 TX */
        LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_12, LL_GPIO_MODE_ALTERNATE);
        /* USART2 TX */
        LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_12, LL_GPIO_MODE_ALTERNATE);

        Exit_Stop_Standby_Mode();
    }

    /* USER CODE BEGIN PWR_ExitStop2Mode_2 */

    /* USER CODE END PWR_ExitStop2Mode_2 */

    SYSTEM_DEBUG_SIGNAL_RESET(LOW_POWER_STOP2_MODE_EXIT);
}
#endif /* (CFG_LPM_STOP2_SUPPORTED == 1) */

#if (CFG_LPM_STOP1_SUPPORTED == 1)
OPTIMIZED static void PWR_EnterStopMode(void)
{
    SYSTEM_DEBUG_SIGNAL_SET(LOW_POWER_STOP_MODE_ENTER);

    /* USER CODE BEGIN PWR_EnterStopMode_1 */

    /* USER CODE END PWR_EnterStopMode_1 */

    /* Notify the Link Layer platform layer the system will enter in WFI
     * and AHB5 clock may be turned of regarding the 2.4Ghz radio state
     */
    LINKLAYER_PLAT_NotifyWFIEnter();

    Enter_Stop_Standby_Mode();

    LL_PWR_SetPowerMode(LL_PWR_MODE_STOP1);

    SYSTEM_DEBUG_SIGNAL_RESET(LOW_POWER_STOP_MODE_ENTER);
    SYSTEM_DEBUG_SIGNAL_SET(LOW_POWER_STOP_MODE_ACTIVE);

    __WFI();

    SYSTEM_DEBUG_SIGNAL_RESET(LOW_POWER_STOP_MODE_ACTIVE);

#if defined(STM32WBAXX_SI_CUT1_0)
    SYS_WAITING_CYCLES_25();
#endif /* STM32WBAXX_SI_CUT1_0 */
       /* USER CODE BEGIN PWR_EnterStopMode_2 */

    /* USER CODE END PWR_EnterStopMode_2 */
}
#endif /* (CFG_LPM_STOP1_SUPPORTED == 1) */

#if (CFG_LPM_STOP1_SUPPORTED == 1)
OPTIMIZED static void PWR_ExitStopMode(void)
{
    SYSTEM_DEBUG_SIGNAL_SET(LOW_POWER_STOP_MODE_EXIT);

    /* USER CODE BEGIN PWR_ExitStopMode_1 */

    /* USER CODE END PWR_ExitStopMode_1 */

    /* Notify the Link Layer platform layer the system exited WFI
     * and AHB5 clock may be resynchronized as is may have been
     * turned of during low power mode entry.
     */
    LINKLAYER_PLAT_NotifyWFIExit();

    Exit_Stop_Standby_Mode();

    /* USER CODE BEGIN PWR_ExitStopMode_2 */

    /* USER CODE END PWR_ExitStopMode_2 */

    SYSTEM_DEBUG_SIGNAL_RESET(LOW_POWER_STOP_MODE_EXIT);
}
#endif /* (CFG_LPM_STOP1_SUPPORTED == 1) */

OPTIMIZED static void PWR_EnterSleepMode(void)
{
    /* USER CODE BEGIN PWR_EnterSleepMode_1 */

    /* USER CODE END PWR_EnterSleepMode_1 */

    /* Notify the Link Layer platform layer the system will enter in WFI
     * and AHB5 clock may be turned of regarding the 2.4Ghz radio state
     */
    LINKLAYER_PLAT_NotifyWFIEnter();

    LL_LPM_EnableSleep();
    __WFI();

    /* USER CODE BEGIN PWR_EnterSleepMode_2 */

    /* USER CODE END PWR_EnterSleepMode_2 */
}

OPTIMIZED static void PWR_ExitSleepMode(void)
{
    /* USER CODE BEGIN PWR_ExitSleepMode */

    /* USER CODE END PWR_ExitSleepMode */

    /* Notify the Link Layer platform layer the system exited WFI
     * and AHB5 clock may be resynchronized as is may have been
     * turned of during low power mode entry.
     */
    LINKLAYER_PLAT_NotifyWFIExit();
}

uint32_t is_boot_from_standby(void)
{
#if (CFG_DEBUGGER_LEVEL <= 1)
    LL_DBGMCU_DisableDBGStopMode();
    LL_DBGMCU_DisableDBGStandbyMode();
#endif /* CFG_DEBUGGER_LEVEL */

    __HAL_RCC_PWR_CLK_ENABLE();

    LL_PWR_EnableUltraLowPowerMode();
    __HAL_FLASH_SLEEP_POWERDOWN_ENABLE();

    /* Ensure this is a return from Standby, and not a reset */
    if ((LL_PWR_IsActiveFlag_SB() == 1UL) && (READ_REG(RCC->CSR) == 0U))
    {
        /* When exit from standby, disable IRQ so that restore and PWR_ExitOffMode are in critical section */
        __disable_irq();

        boot_after_standby = 1;
#if (CFG_LPM_WAKEUP_TIME_PROFILING == 1)
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
        /* Perform standby wakeup time profiling if not already done */
        if (LPM_is_wakeup_time_profiling_done() == 0)
        {
            /* Set Systick clock source on LSE if LSE is ready, if not use LSI2 */
            CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk);
            if (LL_RCC_LSE_IsReady() == 1UL)
            {
                /* Configure sysTick clock source to LSE */
                LL_RCC_SetSystickClockSource(LL_RCC_SYSTICK_CLKSOURCE_LSE);
            }
            else
            {
#if defined(RCC_LSI2_SUPPORT)
                /* Enable LSI2 clock if not already done */
                if (LL_RCC_LSI2_IsReady() == 0UL)
                {
                    uint8_t pwrclkchanged = 0;
                    if (__HAL_RCC_PWR_IS_CLK_ENABLED() != 1U)
                    {
                        __HAL_RCC_PWR_CLK_ENABLE();
                        pwrclkchanged = 1;
                    }

                    /* Enable write access to Backup domain */
                    LL_PWR_EnableBkUpAccess();
                    while (LL_PWR_IsEnabledBkUpAccess() == 0UL)
                        ;

                    LL_RCC_LSI2_Enable();
                    while (LL_RCC_LSI2_IsReady() != 1UL)
                        ;

                    if (pwrclkchanged == 1)
                    {
                        __HAL_RCC_PWR_CLK_DISABLE();
                    }

                    /* Remember that LSI2 is used for wakeup time profiling */
                    lpm_profiling_started_lsi2 = 1;
                }

                /* Configure sysTick clock source to LSI2 */
                LL_RCC_SetSystickClockSource(LL_RCC_SYSTICK_CLKSOURCE_LSI);
#else
                Error_Handler();
#endif /* RCC_LSI2_SUPPORT */
            }

            /* Configure SysTick to full scale */
            WRITE_REG(SysTick->LOAD, 0x00FFFFFFUL);

            /* Enable SysTick Timer, starts to count */
            WRITE_REG(SysTick->VAL, 0UL);
            SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
        }
#endif /* CFG_LPM_STANDBY_SUPPORTED */
#endif /* CFG_LPM_WAKEUP_TIME_PROFILING */
    }
    else
    {
        boot_after_standby = 0;
    }

    return boot_after_standby;
}

#if (CFG_LPM_WAKEUP_TIME_PROFILING == 1)
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
/* returns 0 if wakeup time profiling is not done */
uint32_t LPM_is_wakeup_time_profiling_done(void)
{
    return (lpm_wakeup_time_standby != 0);
}
#endif /* CFG_LPM_STANDBY_SUPPORTED */
#endif /* CFG_LPM_WAKEUP_TIME_PROFILING */

void LPM_IdleMode(uint32_t param)
{
    /* no low power action! */
    /* USER CODE BEGIN LPM_IdleMode */
    /* USER CODE END LPM_IdleMode */
}

void LPM_SleepMode(uint32_t param)
{
    PWR_EnterSleepMode();
    PWR_ExitSleepMode();
}

#if (CFG_LPM_STOP1_SUPPORTED == 1)
void LPM_Stop1Mode(uint32_t param)
{
    PWR_EnterStopMode();
    PWR_ExitStopMode();
}
#endif

#if (CFG_LPM_STOP2_SUPPORTED == 1)
void LPM_Stop2Mode(uint32_t param)
{
    PWR_EnterStop2Mode();
    PWR_ExitStop2Mode();
}
#endif

#if (CFG_LPM_STANDBY_SUPPORTED == 1)
void LPM_StandbyMode(uint32_t param)
{
    PWR_EnterOffMode();
    PWR_ExitOffMode();
}
#endif

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
