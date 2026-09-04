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
#include "app_entry.h"
#include "app_common.h"
#include "app_conf.h"
#include "log_module.h"
#include "main.h"
#include "stm32_rtos.h"
#if (CFG_LPM_LEVEL != 0)
#include "app_sys.h"
#include "stm32_lpm.h"
#endif /* (CFG_LPM_LEVEL != 0) */
#include "advanced_memory_manager.h"
#include "stm32_mm.h"
#include "stm32_timer.h"
#if (CFG_LOG_SUPPORTED != 0)
#include "serial_cmd_interpreter.h"
#include "stm32_adv_trace.h"
#endif /* CFG_LOG_SUPPORTED */
#include "app_ble.h"
#include "app_debug.h"
#include "app_sys.h"
#include "app_thread.h"
#include "ble_timer.h"
#include "blestack.h"
#include "crc_ctrl.h"
#include "flash_driver.h"
#include "flash_manager.h"
#include "ll_sys.h"
#include "ll_sys_if.h"
#include "otp.h"
#include "pka_ctrl.h"
#include "scm.h"
#include "simple_nvm_arbiter.h"
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
#include "adc_ctrl.h"
#include "temp_measurement.h"
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */
#if (CFG_RT_DEBUG_DTB == 1)
#include "RTDebug_dtb.h"
#endif /* CFG_RT_DEBUG_DTB */
#include "timer_if.h"

/* MATTER BEGIN */
#include "AppEvent.h"
#include "AppTask.h"
#include "psa/crypto.h"
#if (OTA_SUPPORT == 1)
#if (OTA_EXTERNAL_FLASH_ENABLE == 1)
#include "stm_ota_spi_flash.h"
#else
#include "stm_ota_flash.h"
#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */
#endif /* (OTA_SUPPORT == 1) */

#if (USE_TFM_PSA == 1)
#include "psa/crypto.h"
#include "tfm_ns_interface.h"
#endif /* (USE_TFM_PSA == 1) */

extern "C" void xPortSysTickHandler(void);      /* Redefinition of the function with C linkage */
extern "C" void vPortSetupTimerInterrupt(void); /* Redefinition of the function with C linkage */
#include "assert.h"
#include "stm32_lpm_if.h"

/* MATTER END */

/* Private includes -----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_bsp.h"

/* MATTER BEGIN */
#include "stm32wba65i_discovery.h"
#include "stm32wba65i_discovery_bus.h"
#if (CFG_LCD_SUPPORTED == 1)
#include "stm32_lcd.h"
#include "stm32wba65i_discovery_lcd.h"
#endif /* CFG_LCD_SUPPORTED */
/* MATTER END*/

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
#define AMM_POOL_SIZE (DIVC(CFG_MM_POOL_SIZE, sizeof(uint32_t)) + (AMM_VIRTUAL_INFO_ELEMENT_SIZE * CFG_AMM_VIRTUAL_MEMORY_NUMBER))
#define RTOS_FLAG_ALL_MASK ((1U << CFG_RTOS_FLAG_LAST) - 1)
#define HAL_TICK_RTC (1)
/* USER CODE BEGIN PD */
#if ((CONFIG_STM32_DAC_PRIVATE_KEY_PROVISIONING == 1) && (USE_TFM_PSA == 1))
#include "storage_interface.h"
#include <credentials/examples/ExampleDACs.h>

static void runDACKeyProvisioning(void);
#endif /* ((CONFIG_STM32_DAC_PRIVATE_KEY_PROVISIONING == 1) && (USE_TFM_PSA ==1))*/

/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private constants ---------------------------------------------------------*/
/* USER CODE BEGIN PC */

/* USER CODE END PC */

/* Private variables ---------------------------------------------------------*/
#if (CFG_LPM_LEVEL != 0)
static bool system_startup_done = FALSE;
#endif /* ( CFG_LPM_LEVEL != 0) */

#if (CFG_LOG_SUPPORTED != 0)
/* Log configuration
 * .verbose_level can be any value of the Log_Verbose_Level_t enum.
 * .region_mask can either be :
 * - LOG_REGION_ALL_REGIONS to enable all regions
 * or
 * - One or several specific regions (any value except LOG_REGION_ALL_REGIONS)
 *   from the Log_Region_t enum and matching the mask value.
 *
 *   For example, to enable both LOG_REGION_BLE and LOG_REGION_APP,
 *   the value assigned to the define is :
 *   (1U << LOG_REGION_BLE | 1U << LOG_REGION_APP)
 */
static Log_Module_t Log_Module_Config = { .verbose_level = APPLI_CONFIG_LOG_LEVEL, .region_mask = APPLI_CONFIG_LOG_REGION };
#endif /* (CFG_LOG_SUPPORTED != 0) */

/* AMM configuration */
static uint32_t AMM_Pool[AMM_POOL_SIZE];
static AMM_VirtualMemoryConfig_t vmConfig[CFG_AMM_VIRTUAL_MEMORY_NUMBER] = {
    /* Virtual Memory #1 */
    { .Id = CFG_AMM_VIRTUAL_BLE_TIMERS, .BufferSize = CFG_AMM_VIRTUAL_BLE_TIMERS_BUFFER_SIZE },
    /* Virtual Memory #2 */
    { .Id = CFG_AMM_VIRTUAL_BLE_EVENTS, .BufferSize = CFG_AMM_VIRTUAL_BLE_EVENTS_BUFFER_SIZE },
};

static AMM_InitParameters_t ammInitConfig = { .p_PoolAddr                = AMM_Pool,
                                              .PoolSize                  = AMM_POOL_SIZE,
                                              .VirtualMemoryNumber       = CFG_AMM_VIRTUAL_MEMORY_NUMBER,
                                              .p_VirtualMemoryConfigList = vmConfig };

static osMutexId_t PkaMutex;

static const osMutexAttr_t PkaMutex_attributes = {
    .name = "PKA Mutex", .attr_bits = TASK_DEFAULT_ATTR_BITS, .cb_mem = TASK_DEFAULT_CB_MEM, .cb_size = TASK_DEFAULT_CB_SIZE
};

static osSemaphoreId_t PkaEndOfOperationSemaphore;

static const osSemaphoreAttr_t PkaEndOfOperationSemaphore_attributes = { .name      = "PKA End of Operation Semaphore",
                                                                         .attr_bits = TASK_DEFAULT_ATTR_BITS,
                                                                         .cb_mem    = TASK_DEFAULT_CB_MEM,
                                                                         .cb_size   = TASK_DEFAULT_CB_SIZE };

/* Timer for OS tick declaration */
static UTIL_TIMER_Object_t TimerOsTick_Id;
/* Timer for HAL tick declaration */
static UTIL_TIMER_Object_t TimerHalTick_Id;

#if (CFG_LPM_LEVEL != 0)
/* Holds maximum number of FreeRTOS tick periods that can be suppressed */
static uint32_t maximumPossibleSuppressedTicks = 0;

/* Timer OS wakeup low power declaration */
static UTIL_TIMER_Object_t TimerOSwakeup_Id;

/* Time remaining variables to correct next OS tick */
static uint32_t timeDiffRemaining         = 0;
static uint32_t lowPowerTimeDiffRemaining = 0;
#endif /* ( CFG_LPM_LEVEL != 0) */

/* FreeRTOS objects declaration */
static const osThreadAttr_t WpanTask_attributes = { .name       = "Wpan Task",
                                                    .attr_bits  = TASK_DEFAULT_ATTR_BITS,
                                                    .cb_mem     = TASK_DEFAULT_CB_MEM,
                                                    .cb_size    = TASK_DEFAULT_CB_SIZE,
                                                    .stack_mem  = TASK_DEFAULT_STACK_MEM,
                                                    .stack_size = TASK_STACK_SIZE_WPAN,
                                                    .priority   = TASK_PRIO_WPAN };

static osMutexId_t crcCtrlMutex;

static const osMutexAttr_t crcCtrlMutex_attributes = {
    .name = "CRC CTRL Mutex", .attr_bits = TASK_DEFAULT_ATTR_BITS, .cb_mem = TASK_DEFAULT_CB_MEM, .cb_size = TASK_DEFAULT_CB_SIZE
};

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
static osMutexId_t adcCtrlMutex;

static const osMutexAttr_t adcCtrlMutex_attributes = {
    .name = "ADC CTRL Mutex", .attr_bits = TASK_DEFAULT_ATTR_BITS, .cb_mem = TASK_DEFAULT_CB_MEM, .cb_size = TASK_DEFAULT_CB_SIZE
};
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/
/* FreeRTOS objects declaration */
osThreadId_t WpanTaskHandle;
/* USER CODE BEGIN GV */

/* USER CODE END GV */

/* Private functions prototypes-----------------------------------------------*/
static void System_Init(void);
static void SystemPower_Config(void);
static void Config_HSE(void);
static void APPE_RNG_Init(void);
static void APPE_AMM_Init(void);
static void AMM_WrapperInit(uint32_t * const p_PoolAddr, const uint32_t PoolSize);
static uint32_t * AMM_WrapperAllocate(const uint32_t BufferSize);
static void AMM_WrapperFree(uint32_t * const p_BufferAddr);
static void APPE_FLASH_MANAGER_Init(void);
static void APPE_PKACTRL_Init(void);
int PKACTRL_MutexTake(void);
int PKACTRL_MutexRelease(void);
int PKACTRL_TakeSemEndOfOperation(void);
int PKACTRL_ReleaseSemEndOfOperation(void);
static void Wpan_Task_Entry(void * argument);
static void TimerOsTickCb(void * arg);
#if (HAL_TICK_RTC == 1)
static void TimerHalTickCb(void * arg);
static void HAL_StartTick(void);
#endif /* HAL_TICK_RTC */
#if (CFG_LPM_LEVEL != 0)
static void TimerOSwakeupCB(void * arg);
static uint32_t getCurrentTime(void);
#endif /* CFG_LPM_LEVEL */

/* MATTER BEGIN */
static void APPE_DisplayFwInfo(void);
static void APPE_SetBootReason(void);
/* MATTER END */

/* USER CODE BEGIN PFP */
/* MATTER BEGIN */
uint8_t m_Boot_Reason = 0;
/* MATTER END */
/* USER CODE END PFP */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Functions Definition ------------------------------------------------------*/
/**
 * @brief   Wireless Private Area Network configuration.
 */
void MX_APPE_Config(void)
{
    /* Configure HSE Tuning */
    Config_HSE();
}

/**
 * @brief   Wireless Private Area Network initialisation.
 */
uint32_t MX_APPE_Init(void * p_param)
{
    APP_DEBUG_SIGNAL_SET(APP_APPE_INIT);

    UNUSED(p_param);
    /* MATTER BEGIN */
    NVM_Initialize();
    APPE_SetBootReason();
    /* MATTER END */
    /* System initialization */
    System_Init();

    /* Configure the system Power Mode */
    SystemPower_Config();
    LOG_INFO_APP("System config done\n");
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
    /* Initialize the Temperature measurement */
    TEMPMEAS_Init();
#endif /* (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1) */

#if (USE_TFM_PSA == 1)
    /* Initialize TFM interface for the Non Secure app */
    tfm_ns_interface_init();
#endif /* (USE_TFM_PSA == 1) */

    /* MATTER BEGIN */
    APPE_DisplayFwInfo();
    /* MATTER END */

    /* Initialize the Advance Memory Manager module */
    APPE_AMM_Init();

    /* Initialize the Random Number Generator module */
    APPE_RNG_Init();

    /* Initialize the Flash Manager module */
    APPE_FLASH_MANAGER_Init();

    /* Create PKA mutex */
    PkaMutex = osMutexNew(&PkaMutex_attributes);
    if (PkaMutex == NULL)
    {
        LOG_ERROR_APP("PKA Mutex FreeRTOS objects creation FAILED");
        Error_Handler();
    }
    /* Create PKA end of operation flag semaphore */
    PkaEndOfOperationSemaphore = osSemaphoreNew(1U, 1U, &PkaEndOfOperationSemaphore_attributes);
    if (PkaEndOfOperationSemaphore == NULL)
    {
        LOG_ERROR_APP("PKA End of Operation Semaphore FreeRTOS objects creation FAILED");
        Error_Handler();
    }
    /* USER CODE BEGIN APPE_Init_1 */

    /* USER CODE END APPE_Init_1 */

    WpanTaskHandle = osThreadNew(Wpan_Task_Entry, NULL, &WpanTask_attributes);

    crcCtrlMutex = osMutexNew(&crcCtrlMutex_attributes);
    if (crcCtrlMutex == NULL)
    {
        LOG_ERROR_APP("CRC CTRL FreeRTOS mutex creation FAILED");
        Error_Handler();
    }

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
    adcCtrlMutex = osMutexNew(&adcCtrlMutex_attributes);
    if (adcCtrlMutex == NULL)
    {
        LOG_ERROR_APP("ADC CTRL FreeRTOS mutex creation FAILED");
        Error_Handler();
    }
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

    /* Initialize the Public Key Accelerator module */
    APPE_PKACTRL_Init();

    LOG_INFO_APP("Start Init BLE\n");
    APP_BLE_Init();

    /* Disable RFTS Bypass for flash operation - Since LL has not started yet */
    FD_SetStatus(FD_FLASHACCESS_RFTS_BYPASS, LL_FLASH_DISABLE);

    /* Thread Initialisation */
    LOG_INFO_APP("Start init Open Thread\n");
    APP_THREAD_Init();

#if (CFG_LPM_LEVEL != 0)
    /* create a SW timer to wakeup system from low power */
    UTIL_TIMER_Create(&TimerOSwakeup_Id, 0, UTIL_TIMER_ONESHOT, &TimerOSwakeupCB, 0);

    maximumPossibleSuppressedTicks = UINT32_MAX;
#endif /* ( CFG_LPM_LEVEL != 0) */

    /* MATTER BEGIN */

#if (OTA_SUPPORT == 1)
#if (OTA_EXTERNAL_FLASH_ENABLE == 1)
    STM_OTA_SPI_FLASH_Init();
#else
    STM_OTA_FLASH_Init();
#endif /* (OTA_EXTERNAL_FLASH_ENABLE == 1) */
#endif /* (OTA_SUPPORT == 1) */

#if (USE_TFM_PSA == 1)
    /* Init PSA */
    psa_status_t retval;
    retval = psa_crypto_init();
    if (retval != PSA_SUCCESS)
    {
        LOG_ERROR_APP("PSA init Failed\n");
        Error_Handler();
    }
    else
    {
        LOG_INFO_APP("PSA init Succeeded\n");
    }
#endif /* CONFIG_STM32_FACTORY_DATA_ENABLE */

    /* Init matter */
#if (CONFIG_STM32_DAC_PRIVATE_KEY_PROVISIONING == 1)
    /* Performs the provisioning of the DAC key used during commissioning.
     * Only calls this function during development/testing phases.
     * The real DAC key must be provisioned by an application in the factory. */
    runDACKeyProvisioning();
#endif /* (CONFIG_STM32_DAC_PRIVATE_KEY_PROVISIONING == 1) */

    LOG_INFO_APP("Start init Matter\n");
    CHIP_ERROR err = GetAppTask().InitMatter();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("Matter init failed\n");
    }

    err = GetAppTask().StartAppTask();
    if (err != CHIP_NO_ERROR)
    {
        LOG_ERROR_APP("Matter thread failed\n");
    }
    /* MATTER END */

    /* USER CODE BEGIN APPE_Init_2 */

    /* USER CODE END APPE_Init_2 */

    APP_DEBUG_SIGNAL_RESET(APP_APPE_INIT);

    return WPAN_SUCCESS;
}

/* USER CODE BEGIN FD */
static void APPE_SetBootReason(void)
{
    /* Software or Hardware Watchdog BootReason Type */
    /* IWDG is controlled by Software or Hardware and it is checked through FLASH_OPTR_IWDG_SW bit */
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_IWDGRST))
    {
        m_Boot_Reason = (FLASH->OPTR & FLASH_OPTR_IWDG_SW) ? 0x03 : 0x04;
    }

    /* Brown-out reset BootReason Type */
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_BORRST))
    {
        m_Boot_Reason = 0x02;
    }

    /* Software Reset BootReason Type */
    /* The kSoftwareReset is triggered by NVIC_SystemReset() function */
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_SFTRST))
    {
#if (OTA_SUPPORT == 1)
        uint8_t SoftwareVersion[sizeof(X_CUBE_MATTER_VERSION)];
        size_t SizeRead   = 0;
        uint8_t error_nvm = 0;

        // retrieve software version to test if a OTA was made
        error_nvm = NVM_GetKeyValue(SoftwareVersion, SOFTWARE_VERSION_KEY, sizeof(SoftwareVersion), &SizeRead, SECTOR_MATTER);
        if (error_nvm == 0)
        {
            if (memcmp(SoftwareVersion, X_CUBE_MATTER_VERSION, sizeof(SoftwareVersion)) == 0)
            {
                m_Boot_Reason = 0x06;
            }
            else
            {
                // store new software version
                NVM_SetKeyValue((char *) X_CUBE_MATTER_VERSION, (char *) SOFTWARE_VERSION_KEY, sizeof(X_CUBE_MATTER_VERSION),
                                SECTOR_MATTER);
                m_Boot_Reason = 0x05;
            }
        }
        else
        {
            m_Boot_Reason = 0x06;
        }

#else
        m_Boot_Reason = 0x06;
#endif
    }

    /* Power on or Pin reset BootReason Type */
    else if (__HAL_RCC_GET_FLAG(RCC_FLAG_PINRST) || __HAL_RCC_GET_FLAG(RCC_FLAG_OBLRST))
    {
        m_Boot_Reason = 0x01;
    }

    /* Unspecified BootReason Type */
    else
    {
        m_Boot_Reason = 0x00;
    }

    /* Clear RCC reset flags after determining the boot reason type */
    __HAL_RCC_CLEAR_RESET_FLAGS();
}

uint8_t APPE_GetBootReason()
{
    return m_Boot_Reason;
}
/* USER CODE END FD */

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/

/**
 * @brief Configure HSE by read this Tuning from OTP
 *
 */
static void Config_HSE(void)
{
    OTP_Data_s * otp_ptr = NULL;

    /* Read HSE_Tuning from OTP */
    if (OTP_Read(DEFAULT_OTP_IDX, &otp_ptr) != HAL_OK)
    {
        /* OTP no present in flash, apply default gain */
        HAL_RCCEx_HSESetTrimming(0x0C);
    }
    else
    {
        HAL_RCCEx_HSESetTrimming(otp_ptr->hsetune);
    }
}

/**
 *
 */
static void System_Init(void)
{
    /* Clear RCC RESET flag */
    LL_RCC_ClearResetFlags();

    /* Initialize the Timer Server */
    UTIL_TIMER_Init();

#if (HAL_TICK_RTC == 1)
    HAL_StartTick();
#endif /* HAL_TICK_RTC */
    /* USER CODE BEGIN System_Init_1 */

    /* USER CODE END System_Init_1 */

    /* Enable wakeup out of standby from RTC ( UTIL_TIMER )*/
    HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN7_HIGH_3);

#if (CFG_LOG_SUPPORTED != 0)
    MX_USART1_UART_Init();
    MX_USART2_UART_Init();

    /* Initialize the logs ( using the USART ) */
    Log_Module_Init(Log_Module_Config);

    /* Initialize the Command Interpreter */
    Serial_CMD_Interpreter_Init();
#endif /* (CFG_LOG_SUPPORTED != 0) */

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
    ADCCTRL_Init();
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

#if (CFG_RT_DEBUG_DTB == 1)
    /* DTB initialization and configuration */
    RT_DEBUG_DTBInit();
    RT_DEBUG_DTBConfig();
#endif /* CFG_RT_DEBUG_DTB */
#if (CFG_RT_DEBUG_GPIO_MODULE == 1)
    /* RT DEBUG GPIO_Init */
    RT_DEBUG_GPIO_Init();
#endif /* (CFG_RT_DEBUG_GPIO_MODULE == 1) */

#if (CFG_LPM_LEVEL != 0)
    system_startup_done = TRUE;
    UNUSED(system_startup_done);
#endif /* ( CFG_LPM_LEVEL != 0) */

    return;
}

/**
 * @brief  Configure the system for power optimization
 *
 * @note  This API configures the system to be ready for low power mode
 *
 * @param  None
 * @retval None
 */
static void SystemPower_Config(void)
{
#if (CFG_SCM_SUPPORTED == 1)
    /* Initialize System Clock Manager */
    scm_init();
#else
    if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
    {
        Error_Handler();
    }
#endif /* CFG_SCM_SUPPORTED */

#if (CFG_DEBUGGER_LEVEL == 0)
    /* Setup GPIOA 13, 14, 15 in Analog no pull */
    if (__HAL_RCC_GPIOA_IS_CLK_ENABLED() == 0)
    {
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIOA->PUPDR &= ~0xFC000000;
        GPIOA->MODER |= 0xFC000000;
        __HAL_RCC_GPIOA_CLK_DISABLE();
    }
    else
    {
        GPIOA->PUPDR &= ~0xFC000000;
        GPIOA->MODER |= 0xFC000000;
    }

    /* Setup GPIOB 3, 4 in Analog no pull */
    if (__HAL_RCC_GPIOB_IS_CLK_ENABLED() == 0)
    {
        __HAL_RCC_GPIOB_CLK_ENABLE();
        GPIOB->PUPDR &= ~0x3C0;
        GPIOB->MODER |= 0x3C0;
        __HAL_RCC_GPIOB_CLK_DISABLE();
    }
    else
    {
        GPIOB->PUPDR &= ~0x3C0;
        GPIOB->MODER |= 0x3C0;
    }
#endif /* CFG_DEBUGGER_LEVEL */

#if (CFG_LPM_LEVEL != 0)
    /* Initialize low Power Manager. By default enabled */
    UTIL_LPM_Init();

#if ((CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1))
    /* Enable SRAM1, SRAM2 and RADIO retention*/
    LL_PWR_SetSRAM1SBRetention(LL_PWR_SRAM1_SB_FULL_RETENTION);
    LL_PWR_SetSRAM2SBRetention(LL_PWR_SRAM2_SB_FULL_RETENTION);
    LL_PWR_SetRadioSBRetention(LL_PWR_RADIO_SB_FULL_RETENTION); /* Retain sleep timer configuration */

#else  /* (CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1) */
    UTIL_LPM_SetMaxMode(1U << CFG_LPM_APP, UTIL_LPM_MAX_MODE);
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1) */
#endif /* (CFG_LPM_LEVEL != 0)  */

    /* USER CODE BEGIN SystemPower_Config */

    /* USER CODE END SystemPower_Config */
}

static void Wpan_Task_Entry(void * lArgument)
{
    UNUSED(lArgument);
    /* USER CODE BEGIN Wpan_Task_Entry_0 */

    /* USER CODE END Wpan_Task_Entry_0 */
    uint32_t actual_flags;

    for (;;)
    {
        actual_flags = osThreadFlagsWait(RTOS_FLAG_ALL_MASK, osFlagsWaitAny, osWaitForever);

        if (actual_flags & (1U << CFG_RTOS_FLAG_RNG))
        {
            HW_RNG_Process();
        }

        if (actual_flags & (1U << CFG_RTOS_FLAG_PKA))
        {
            PKACTRL_BG_Process();
        }

        if (actual_flags & (1U << CFG_RTOS_FLAG_FlashManager))
        {
            FM_BackgroundProcess();
        }

        /* Link Layer related */
        if (actual_flags & (1U << CFG_RTOS_FLAG_LinkLayer))
        {
            osMutexAcquire(LinkLayerMutex, osWaitForever);
            ll_sys_bg_process();
            osMutexRelease(LinkLayerMutex);
        }

        /* OT related */
        if (actual_flags & (1U << CFG_RTOS_FLAG_OT_Tasklet))
        {
            osMutexAcquire(LinkLayerMutex, osWaitForever);
            APP_THREAD_ProcessOpenThreadTasklets(NULL);
            osMutexRelease(LinkLayerMutex);
        }

        if (actual_flags & (1U << CFG_RTOS_FLAG_OT_Alarm_ms))
        {
            osMutexAcquire(LinkLayerMutex, osWaitForever);
            APP_THREAD_ProcessAlarm(NULL);
            osMutexRelease(LinkLayerMutex);
        }

        if (actual_flags & (1U << CFG_RTOS_FLAG_OT_Alarm_us))
        {
            osMutexAcquire(LinkLayerMutex, osWaitForever);
            APP_THREAD_ProcessUsAlarm(NULL);
            osMutexRelease(LinkLayerMutex);
        }

#if (OT_CLI_USE == 1)
        if (actual_flags & (1U << CFG_RTOS_FLAG_OT_CLIuart))
        {
            osMutexAcquire(LinkLayerMutex, osWaitForever);
            APP_THREAD_ProcessUart(NULL);
            osMutexRelease(LinkLayerMutex);
        }
#endif
#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
        if (actual_flags & (1U << CFG_RTOS_FLAG_TempRadioCalib))
        {
            TEMPMEAS_RequestTemperatureMeasurement();
        }
#endif
        if (actual_flags & (1U << CFG_RTOS_FLAG_AMM))
        {
            AMM_BackgroundProcess();
        }

        /* BLE related */
        if (actual_flags & (1U << CFG_RTOS_FLAG_BLEhost))
        {
            BleStack_Process_BG();
        }

        if (actual_flags & (1U << CFG_RTOS_FLAG_BLEhciEvt))
        {
            Ble_UserEvtRx();
        }

        if (actual_flags & (1U << CFG_RTOS_FLAG_BLEtimer))
        {
            BLE_TIMER_Background();
        }
    }
}

/**
 * @brief Initialize Random Number Generator module
 */
static void APPE_RNG_Init(void)
{
    HW_RNG_SetPoolThreshold(CFG_HW_RNG_POOL_THRESHOLD);
    HW_RNG_Init();
    HW_RNG_Start();
}

/**
 * @brief Initialize Flash Manager module
 */
static void APPE_FLASH_MANAGER_Init(void)
{
    /* Init the Flash Manager module */
    FM_Init();

    /* Disable flash before any use - RFTS */
    FD_SetStatus(FD_FLASHACCESS_RFTS, LL_FLASH_DISABLE);
    /* Enable RFTS Bypass for flash operation - Since LL has not started yet */
    FD_SetStatus(FD_FLASHACCESS_RFTS_BYPASS, LL_FLASH_ENABLE);
    /* Enable flash system flag */
    FD_SetStatus(FD_FLASHACCESS_SYSTEM, LL_FLASH_ENABLE);

    return;
}

/**
 * @brief Initialize Public Key Accelerator module
 */
static void APPE_PKACTRL_Init(void) {}

static void APPE_AMM_Init(void)
{
    /* Initialize the Advance Memory Manager */
    if (AMM_Init(&ammInitConfig) != AMM_ERROR_OK)
    {
        Error_Handler();
    }
}

/* Timer OS tick callback */
static void TimerOsTickCb(void * arg)
{
    xPortSysTickHandler();
    /* USER CODE BEGIN TimerOsTickCb */

    /* USER CODE END TimerOsTickCb */
    return;
}

#if (HAL_TICK_RTC == 1)
/* Timer HAL tick callback */
static void TimerHalTickCb(void * arg)
{
    HAL_IncTick();
    /* USER CODE BEGIN TimerHalTickCb */

    /* USER CODE END TimerHalTickCb */
    return;
}
#endif /* HAL_TICK_RTC */

#if (CFG_LPM_LEVEL != 0)
/* OS wakeup callback */
static void TimerOSwakeupCB(void * arg)
{
    /* USER CODE BEGIN TimerOSwakeupCB */

    /* USER CODE END TimerOSwakeupCB */
    return;
}

/* return current time since boot, continue to count in standby low power mode */
static uint32_t getCurrentTime(void)
{
    return TIMER_IF_GetTimerValue();
}
#endif /* ( CFG_LPM_LEVEL != 0) */

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

/* USER CODE END FD_LOCAL_FUNCTIONS */

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/

void PKACTRL_CB_Process(void)
{
    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_PKA);
}

/**
 * @brief Callback used by Random Number Generator to launch Task to generate Random Numbers
 */
void HWCB_RNG_Process(void)
{
    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_RNG);
}

void AMM_RegisterBasicMemoryManager(AMM_BasicMemoryManagerFunctions_t * const p_BasicMemoryManagerFunctions)
{
    /* Fulfill the function handle */
    p_BasicMemoryManagerFunctions->Init     = AMM_WrapperInit;
    p_BasicMemoryManagerFunctions->Allocate = AMM_WrapperAllocate;
    p_BasicMemoryManagerFunctions->Free     = AMM_WrapperFree;
}

void AMM_ProcessRequest(void)
{
    /* Trigger to call Advance Memory Manager process function */
    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_AMM);
}

static void AMM_WrapperInit(uint32_t * const p_PoolAddr, const uint32_t PoolSize)
{
    UTIL_MM_Init((uint8_t *) p_PoolAddr, ((size_t) PoolSize * sizeof(uint32_t)));
}

static uint32_t * AMM_WrapperAllocate(const uint32_t BufferSize)
{
    return (uint32_t *) UTIL_MM_GetBuffer(((size_t) BufferSize * sizeof(uint32_t)));
}

static void AMM_WrapperFree(uint32_t * const p_BufferAddr)
{
    UTIL_MM_ReleaseBuffer((void *) p_BufferAddr);
}

void FM_ProcessRequest(void)
{
    /* Trigger to call Flash Manager process function */
    osThreadFlagsSet(WpanTaskHandle, 1U << CFG_RTOS_FLAG_FlashManager);
}

#if ((CFG_LOG_SUPPORTED == 0) && (CFG_LPM_LEVEL != 0))
/* RNG module turn off HSI clock when traces are not used and low power used */
void RNG_KERNEL_CLK_OFF(void)
{
    /* USER CODE BEGIN RNG_KERNEL_CLK_OFF_1 */

    /* USER CODE END RNG_KERNEL_CLK_OFF_1 */
    LL_RCC_HSI_Disable();
    /* USER CODE BEGIN RNG_KERNEL_CLK_OFF_2 */

    /* USER CODE END RNG_KERNEL_CLK_OFF_2 */
}

#if (CFG_SCM_SUPPORTED == 1)
/* SCM module turn off HSI clock when traces are not used and low power used */
void SCM_HSI_CLK_OFF(void)
{
    /* USER CODE BEGIN SCM_HSI_CLK_OFF_1 */

    /* USER CODE END SCM_HSI_CLK_OFF_1 */
    LL_RCC_HSI_Disable();
    /* USER CODE BEGIN SCM_HSI_CLK_OFF_2 */

    /* USER CODE END SCM_HSI_CLK_OFF_2 */
}
#endif /* CFG_SCM_SUPPORTED */
#endif /* ((CFG_LOG_SUPPORTED == 0) && (CFG_LPM_LEVEL != 0)) */

#if (CFG_LOG_SUPPORTED != 0)
void UTIL_ADV_TRACE_PreSendHook(void)
{
#if (CFG_LPM_LEVEL != 0)
    /* Disable Stop mode before sending a LOG message over UART */
    UTIL_LPM_SetMaxMode(1U << CFG_LPM_LOG, UTIL_LPM_SLEEP_MODE);
#endif /* (CFG_LPM_LEVEL != 0) */
       /* USER CODE BEGIN UTIL_ADV_TRACE_PreSendHook */

    /* USER CODE END UTIL_ADV_TRACE_PreSendHook */
}

void UTIL_ADV_TRACE_PostSendHook(void)
{
#if (CFG_LPM_LEVEL != 0)
    /* Enable Stop mode after LOG message over UART sent */
    UTIL_LPM_SetMaxMode(1U << CFG_LPM_LOG, UTIL_LPM_MAX_MODE);
#endif /* (CFG_LPM_LEVEL != 0) */
       /* USER CODE BEGIN UTIL_ADV_TRACE_PostSendHook */

    /* USER CODE END UTIL_ADV_TRACE_PostSendHook */
}

/**
 * @brief  Treat Serial commands.
 *
 * @param  pRxBuffer      Pointer on received data from USART.
 * @param  iRxBufferSize  Number of received data.
 * @retval None
 */
void Serial_CMD_Interpreter_CmdExecute(uint8_t * pRxBuffer, uint16_t iRxBufferSize)
{
    /* USER CODE BEGIN Serial_CMD_Interpreter_CmdExecute_1 */

    /* USER CODE END Serial_CMD_Interpreter_CmdExecute_1 */
}

#endif /* (CFG_LOG_SUPPORTED != 0) */

/* Implement weak function to setup a timer that will trig OS ticks */
void vPortSetupTimerInterrupt(void)
{
    UTIL_TIMER_Create(&TimerOsTick_Id, portTICK_PERIOD_MS, UTIL_TIMER_PERIODIC, &TimerOsTickCb, 0);

    UTIL_TIMER_StartWithPeriod(&TimerOsTick_Id, portTICK_PERIOD_MS);
    /* USER CODE BEGIN vPortSetupTimerInterrupt */
    if (portTICK_PERIOD_MS == 10U)
    {
        uwTickFreq = HAL_TICK_FREQ_10HZ;
    }
    else if (portTICK_PERIOD_MS == 100U)
    {
        uwTickFreq = HAL_TICK_FREQ_100HZ;
    }
    else if (portTICK_PERIOD_MS == 1000U)
    {
        uwTickFreq = HAL_TICK_FREQ_1KHZ;
    }
    else
    {
        uwTickFreq = HAL_TICK_FREQ_DEFAULT;
    }
    /* USER CODE END vPortSetupTimerInterrupt */
    return;
}
#if (CFG_LPM_LEVEL != 0)
void vPortSuppressTicksAndSleep(uint32_t xExpectedIdleTime)
{
    uint32_t lowPowerTimeBeforeSleep, lowPowerTimeAfterSleep, lowPowerTimeDiff, timeDiff;
    eSleepModeStatus eSleepStatus;

    if ((system_startup_done != FALSE) && (UTIL_LPM_GetMaxMode() == UTIL_LPM_SLEEP_MODE))
    {
        LL_LPM_EnableSleep();
        __WFI();
        return;
    }
    else if ((system_startup_done != FALSE) && (UTIL_LPM_GetMaxMode() == UTIL_LPM_IDLE_MODE))
    {
        return;
    }
    /* Stop the timer that is generating the OS tick interrupt. */
    UTIL_TIMER_Stop(&TimerOsTick_Id);

    /* Make sure the SysTick reload value does not overflow the counter. */
    if (xExpectedIdleTime > maximumPossibleSuppressedTicks)
    {
        xExpectedIdleTime = maximumPossibleSuppressedTicks;
    }

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
     * method as that will mask interrupts that should exit sleep mode. */
    __asm volatile("cpsid i" ::: "memory");
    __asm volatile("dsb");
    __asm volatile("isb");

    eSleepStatus = eTaskConfirmSleepModeStatus();

    /* If a context switch is pending or a task is waiting for the scheduler
     * to be unsuspended then abandon the low power entry. */
    if (eSleepStatus == eAbortSleep)
    {
        /* Restart the timer that is generating the OS tick interrupt. */
        UTIL_TIMER_StartWithPeriod(&TimerOsTick_Id, portTICK_PERIOD_MS);

        /* Re-enable interrupts - see comments above the cpsid instruction above. */
        __asm volatile("cpsie i" ::: "memory");
    }
    else
    {
        if (eSleepStatus != eNoTasksWaitingTimeout)
        {
            /* Configure an interrupt to bring the microcontroller out of its low
               power state at the time the kernel next needs to execute. The
               interrupt must be generated from a source that remains operational
               when the microcontroller is in a low power state. */
            UTIL_TIMER_StartWithPeriod(&TimerOSwakeup_Id, (xExpectedIdleTime - 1) * portTICK_PERIOD_MS);
        }

        /* Read the current time from RTC, maintainned in standby */
        lowPowerTimeBeforeSleep = getCurrentTime();

        /* Enter the low power state. */

        LL_PWR_ClearFlag_STOP();
#if ((CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1))
#if (CFG_LPM_STOP2_SUPPORTED == 1)
        if ((system_startup_done != FALSE) && (UTIL_LPM_GetMaxMode() >= UTIL_LPM_STOP2_MODE))
#else /* (CFG_LPM_STOP2_SUPPORTED == 1) */
#if (CFG_LPM_STANDBY_SUPPORTED == 1)
        if ((system_startup_done != FALSE) && (UTIL_LPM_GetMaxMode() >= UTIL_LPM_STANDBY_MODE))
#endif /* (CFG_LPM_STANDBY_SUPPORTED == 1) */
#endif /* (CFG_LPM_STOP2_SUPPORTED == 1) */
        {
            APP_SYS_BLE_EnterDeepSleep();
        }
#endif /* ((CFG_LPM_STANDBY_SUPPORTED == 1) || (CFG_LPM_STOP2_SUPPORTED == 1)) */
        LL_RCC_ClearResetFlags();

        HAL_SuspendTick();
        UTIL_LPM_Enter(0); /* WFI instruction call is inside this API */
        HAL_ResumeTick();

        /* Stop the timer that may wakeup us as wakeup source can be another one */
        UTIL_TIMER_Stop(&TimerOSwakeup_Id);

        /* Determine how long the microcontroller was actually in a low power
         state for, which will be less than xExpectedIdleTime if the
         microcontroller was brought out of low power mode by an interrupt
         other than that configured by the vSetWakeTimeInterrupt() call.
         Note that the scheduler is suspended before
         portSUPPRESS_TICKS_AND_SLEEP() is called, and resumed when
         portSUPPRESS_TICKS_AND_SLEEP() returns.  Therefore no other tasks will
         execute until this function completes. */
        lowPowerTimeAfterSleep = getCurrentTime();

        /* Compute time spent in low power state and report precision loss */
        lowPowerTimeDiff = (lowPowerTimeAfterSleep - lowPowerTimeBeforeSleep) + lowPowerTimeDiffRemaining;
        /* Store precision loss during RTC time conversion to report it for next OS tick. */
        timeDiff                  = TIMER_IF_Convert_Tick2ms(lowPowerTimeDiff);
        lowPowerTimeDiffRemaining = lowPowerTimeDiff - TIMER_IF_Convert_ms2Tick(timeDiff);
        /* Report precision loss */
        timeDiff += timeDiffRemaining;
        /* Store precision loss during OS tick time conversion to report it for next OS tick. */
        timeDiffRemaining = timeDiff % portTICK_PERIOD_MS;

        /* Correct the kernel tick count to account for the time spent in the low power state. */
        if ((timeDiff / portTICK_PERIOD_MS) != 0U)
        {
            vTaskStepTick(timeDiff / portTICK_PERIOD_MS);
        }

        /* Re-enable interrupts to allow the interrupt that brought the MCU
         * out of sleep mode to execute immediately.  See comments above
         * the cpsid instruction above. */
        __asm volatile("cpsie i" ::: "memory");
        __asm volatile("dsb");
        __asm volatile("isb");

        /* Put the radio in active state */
        if (system_startup_done != FALSE)
        {
            LL_AHB5_GRP1_EnableClock(LL_AHB5_GRP1_PERIPH_RADIO);
            (void) ll_sys_dp_slp_exit();
        }
        UTIL_LPM_SetMaxMode(1U << CFG_LPM_LL_DEEPSLEEP, UTIL_LPM_MAX_MODE);

        /* Restart the timer that is generating the OS tick interrupt. */
        UTIL_TIMER_StartWithPeriod(&TimerOsTick_Id, portTICK_PERIOD_MS);

        /* USER CODE BEGIN vPortSuppressTicksAndSleep */
        APP_BSP_PostIdle();
        /* USER CODE END vPortSuppressTicksAndSleep */
    }
    return;
}
#else
void vPortSuppressTicksAndSleep(uint32_t xExpectedIdleTime)
{
    return;
}
#endif /* ( CFG_LPM_LEVEL != 0) */

/**
 * @brief Function Assert AEABI in case of not described on 'libc' libraries.
 */
__WEAK void __aeabi_assert(const char * szExpression, const char * szFile, int iLine)
{
    Error_Handler();
    __builtin_unreachable();
}

CRCCTRL_Cmd_Status_t CRCCTRL_MutexTake(void)
{
    osStatus_t os_status;
    CRCCTRL_Cmd_Status_t crc_status;
    /* USER CODE BEGIN CRCCTRL_MutexTake_0 */

    /* USER CODE END CRCCTRL_MutexTake_0 */
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        os_status = osMutexAcquire(crcCtrlMutex, osWaitForever);
    }
    else
    {
        /* If the scheduler is not running, we can assume that the mutex is available */
        os_status = osOK;
    }

    if (os_status != osOK)
    {
        crc_status = CRCCTRL_NOK;
    }
    else
    {
        crc_status = CRCCTRL_OK;
    }
    /* USER CODE BEGIN CRCCTRL_MutexTake_1 */

    /* USER CODE END CRCCTRL_MutexTake_1 */
    return crc_status;
}

CRCCTRL_Cmd_Status_t CRCCTRL_MutexRelease(void)
{
    osStatus_t os_status;
    CRCCTRL_Cmd_Status_t crc_status;
    /* USER CODE BEGIN CRCCTRL_MutexRelease_0 */

    /* USER CODE END CRCCTRL_MutexRelease_0 */
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        os_status = osMutexRelease(crcCtrlMutex);
    }
    else
    {
        /* If the scheduler is not running, we can assume that the mutex is available */
        os_status = osOK;
    }

    if (os_status != osOK)
    {
        crc_status = CRCCTRL_NOK;
    }
    else
    {
        crc_status = CRCCTRL_OK;
    }
    /* USER CODE BEGIN CRCCTRL_MutexRelease_1 */

    /* USER CODE END CRCCTRL_MutexRelease_1 */
    return crc_status;
}

#if (USE_TEMPERATURE_BASED_RADIO_CALIBRATION == 1)
ADCCTRL_Cmd_Status_t ADCCTRL_MutexTake(void)
{
    osStatus_t os_status;
    ADCCTRL_Cmd_Status_t adc_status;
    /* USER CODE BEGIN ADCCTRL_MutexTake_0 */

    /* USER CODE END ADCCTRL_MutexTake_0 */
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        os_status = osMutexAcquire(adcCtrlMutex, osWaitForever);
    }
    else
    {
        /* If the scheduler is not running, we can assume that the mutex is available */
        os_status = osOK;
    }

    if (os_status != osOK)
    {
        adc_status = ADCCTRL_NOK;
    }
    else
    {
        adc_status = ADCCTRL_OK;
    }
    /* USER CODE BEGIN ADCCTRL_MutexTake_1 */

    /* USER CODE END ADCCTRL_MutexTake_1 */
    return adc_status;
}

ADCCTRL_Cmd_Status_t ADCCTRL_MutexRelease(void)
{
    osStatus_t os_status;
    ADCCTRL_Cmd_Status_t adc_status;
    /* USER CODE BEGIN ADCCTRL_MutexRelease_0 */

    /* USER CODE END ADCCTRL_MutexRelease_0 */
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
    {
        os_status = osMutexRelease(adcCtrlMutex);
    }
    else
    {
        /* If the scheduler is not running, we can assume that the mutex is available */
        os_status = osOK;
    }

    if (os_status != osOK)
    {
        adc_status = ADCCTRL_NOK;
    }
    else
    {
        adc_status = ADCCTRL_OK;
    }
    /* USER CODE BEGIN ADCCTRL_MutexRelease_1 */

    /* USER CODE END ADCCTRL_MutexRelease_1 */
    return adc_status;
}
#endif /* USE_TEMPERATURE_BASED_RADIO_CALIBRATION */

#if (HAL_TICK_RTC == 1)
/* Overwrite HAL Tick functions to not use sysTick but RTC */
HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority)
{
    if (TimerHalTick_Id.Callback == NULL)
    {
        /* Create an RTC based timer to trigger HAL tick increment */
        UTIL_TIMER_Create(&TimerHalTick_Id, HAL_TICK_FREQ_100HZ, UTIL_TIMER_PERIODIC, &TimerHalTickCb, 0);
        uwTickFreq = HAL_TICK_FREQ_100HZ;
    }
    return HAL_OK;
}

void HAL_SuspendTick(void)
{
    UTIL_TIMER_Stop(&TimerHalTick_Id);
    return;
}

void HAL_ResumeTick(void)
{
    UTIL_TIMER_Start(&TimerHalTick_Id);
    return;
}

static void HAL_StartTick(void)
{
    UTIL_TIMER_StartWithPeriod(&TimerHalTick_Id, HAL_TICK_FREQ_100HZ);
    return;
}
#endif /* HAL_TICK_RTC */

int PKACTRL_MutexTake(void)
{
    int error = 0;
    osStatus_t os_status;

    /* Take the mutex */
    os_status = osMutexAcquire(PkaMutex, osWaitForever);

    if (os_status != osOK)
    {
        error = -1; /* Error */
    }

    return error;
}

int PKACTRL_MutexRelease(void)
{
    int error = 0;
    osStatus_t os_status;

    /* Take the mutex */
    os_status = osMutexRelease(PkaMutex);

    if (os_status != osOK)
    {
        error = -1; /* Error */
    }

    return error;
}

int PKACTRL_TakeSemEndOfOperation(void)
{
    int error = 0;
    osStatus_t os_status;

#if (CFG_LPM_LEVEL != 0)
    /* Avoid going in low power during computation */
    UTIL_LPM_SetMaxMode(1U << CFG_LPM_PKA_OVR_IT, UTIL_LPM_SLEEP_MODE);
#endif /* (CFG_LPM_LEVEL != 0) */

    /* Take the semaphore */
    os_status = osSemaphoreAcquire(PkaEndOfOperationSemaphore, osWaitForever);

    if (os_status != osOK)
    {
        error = -1; /* Error */
    }

    return error;
}

int PKACTRL_ReleaseSemEndOfOperation(void)
{
    int error = 0;
    osStatus_t os_status;

    /* Release the semaphore */
    os_status = osSemaphoreRelease(PkaEndOfOperationSemaphore);

#if (CFG_LPM_LEVEL != 0)
    /* Restore low power */
    UTIL_LPM_SetMaxMode(1U << CFG_LPM_PKA_OVR_IT, UTIL_LPM_MAX_MODE);
#endif /* (CFG_LPM_LEVEL != 0) */

    if (os_status != osOK)
    {
        error = -1; /* Error */
    }

    return error;
}

/* USER CODE BEGIN FD_WRAP_FUNCTIONS */

/* USER CODE END FD_WRAP_FUNCTIONS */

/* MATTER BEGIN */

/* Callback function to handle pushbutton to AppTask */
PushButtonCallback PbCb = NULL;

void APPE_PushButtonSetReceiveCb(PushButtonCallback aCallback)
{
    PbCb = aCallback;
}

/**
 * @brief Joystick Down Action
 *
 * @param  None
 * @return None
 */
void APP_BSP_JoystickDownAction(void)
{
    ButtonDesc_t Message;
    Message.button = B1;
    Message.State  = 1;
    PbCb(&Message); // call matter callback to handle push button
}

/**
 * @brief Joystick Select Action
 *
 * @param  None
 * @return None
 */
void APP_BSP_JoystickSelectAction(void)
{
    ButtonDesc_t Message;
    Message.button = B2;
    Message.State  = 1;
    PbCb(&Message); // call matter callback to handle push button
}

/**
 * @brief Joystick Left Action
 *
 * @param  None
 * @return None
 */
void APP_BSP_JoystickLeftAction(void)
{
    ButtonDesc_t Message;
    Message.button = B3;
    Message.State  = 1;
    if (APP_BSP_JoystickIsInitialPress())
    {
        Message.longPressed = 0;
    }
    else
    {
        if (APP_BSP_JoystickIsLongPressed())
        {
            Message.longPressed = 1;
        }
        else
        {
            Message.State = 0;
        }
    }
    PbCb(&Message); // call matter callback to handle push button
}

static void APPE_DisplayFwInfo(void)
{

    LOG_INFO_APP("FW info :\n");
    LOG_INFO_APP("**********************************************************\n");
    LOG_INFO_APP("PRODUCT NAME : " PRODUCT_NAME "\n");
    LOG_INFO_APP("VENDOR NAME : " VENDOR_NAME "\n");
#if defined(USE_STM32WBA65I_DK1)
    LOG_INFO_APP("HARDWARE : STM32WBA65I-DK1\n");
#else
    LOG_INFO_APP("HARDWARE : UNSPECIFIED\n");
#endif
    LOG_INFO_APP("SOFTWARE : X-CUBE-MATTER release revision " X_CUBE_MATTER_VERSION "\n");
    LOG_INFO_APP("Embedded SW components :\n");
    LOG_INFO_APP("- Matter SDK version : " MATTER_SDK_VERSION "\n");
    LOG_INFO_APP("**********************************************************\n");
}

#if ((CONFIG_STM32_DAC_PRIVATE_KEY_PROVISIONING == 1) && (USE_TFM_PSA == 1))
static void runDACKeyProvisioning(void)
{
    /* This function performs the provisioning of the DAC key used during commissioning.
     * It uses test keys and should only be activated during development/testing phases.
     * The real DAC key must be provisioned by an application in the factory.
     */

    /* Uses DevelopmentCerts::kDacPrivateKey
     * This key is defined in connectedhomeip\src\credentials\examples\ExampleDACs.cpp
     * For Lighting application, it uses MATTER test key = kDevelopmentDAC_PrivateKey_FFF1_8004
     * It is possible to use an hard-coded key
     *
     * const uint8_t Private_Key[32] = {
     *     0x82, 0x0a, 0x24, 0x2a, 0x03, 0x0e, 0xbc, 0xe1, 0x1f, 0x38, 0x73, 0x5a, 0xcf, 0x1a, 0x6f, 0x37,
     *     0xc3, 0xad, 0xa6, 0xe4, 0x32, 0xd2, 0x47, 0x0a, 0x8a, 0x41, 0x37, 0x43, 0xf8, 0x95, 0x63, 0xf3,
     * };
     */

    psa_status_t retval;
    /* Key attributes */
    psa_key_attributes_t key_attributes;
    psa_key_id_t key_handle_private; /* Private key */

    uint32_t fw_version = psa_framework_version();
    APP_DBG("PSA Framework Version = %d.%d\r\n", fw_version >> 8, fw_version & 0xFF);

    APP_DBG("Start DAC private Key provisioning\n\r");

    /* Init the key attributes */
    key_attributes = psa_key_attributes_init();

    /* Reset the key attribute */
    psa_reset_key_attributes(&key_attributes);

    /* Setup the key policy for private key */
    psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
    psa_set_key_algorithm(&key_attributes, PSA_ALG_ECDSA(PSA_ALG_SHA_256));
    psa_set_key_type(&key_attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1));

    psa_set_key_bits(&key_attributes, 8U * chip::DevelopmentCerts::kDacPrivateKey.size());
    /* If using a local Private_Key instead of MATTER test key, comment previous line and use following line instead.
     * psa_set_key_bits(&key_attributes, 8U*sizeof(Private_Key));
     */

    psa_set_key_lifetime(
        &key_attributes,
        PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_DEFAULT, PSA_KEY_LOCATION_LOCAL_STORAGE));
    psa_set_key_id(&key_attributes, DEVICE_ATTESTATION_PRIVATE_KEY_ID_USER);

    /* Verify if the key already exist */
    retval = psa_open_key(DEVICE_ATTESTATION_PRIVATE_KEY_ID_USER, &key_handle_private);
    if (retval == PSA_SUCCESS)
    {
        APP_DBG("DAC Private key already existing\r\n");
    }
    else
    {
        /* Import a private key */
        // retval = psa_import_key(&key_attributes, Private_Key, sizeof(Private_Key), &key_handle_private);
        retval = psa_import_key(&key_attributes, chip::DevelopmentCerts::kDacPrivateKey.data(),
                                chip::DevelopmentCerts::kDacPrivateKey.size(), &key_handle_private);
        if (retval != PSA_SUCCESS)
        {
            APP_DBG("ERROR when importing DAC Private key\n\r");
        }
        else
        {
            APP_DBG("DAC Private key imported successfully\r\n");
        }
    }

    /* Reset the key attribute */
    psa_reset_key_attributes(&key_attributes);

    /* Close the key to remove it from volatile memory */
    retval = psa_close_key(key_handle_private);
    if (retval != PSA_SUCCESS)
    {
        APP_DBG("ERROR psa_close_key()\n\r");
    }

    APP_DBG("End DAC Private Key provisioning\n\r");
}

#endif /* ((CONFIG_STM32_DAC_PRIVATE_KEY_PROVISIONING == 1) && (USE_TFM_PSA ==1)) */

/* MATTER END */
