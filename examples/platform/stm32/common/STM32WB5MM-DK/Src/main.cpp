/**
 ******************************************************************************
 * @file           : app_main.cpp
 * @author 		   : MCD Application Team
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2019-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 @verbatim
 ==============================================================================
 ##### IMPORTANT NOTE #####
 ==============================================================================

 This application requests having the stm32wb5x_BLE_Thread_ForMatter_fw.bin binary
 flashed on the Wireless Coprocessor.
 If it is not the case, you need to use STM32CubeProgrammer to load the appropriate
 binary.

 All available binaries are located under following directory:
 /Projects/STM32_Copro_Wireless_Binaries

 Refer to UM2237 to learn how to use/install STM32CubeProgrammer.
 Refer to /Projects/STM32_Copro_Wireless_Binaries/ReleaseNote.html for the
 detailed procedure to change the Wireless Coprocessor binary.

 @endverbatim
 ******************************************************************************
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "AppTask.h"
#include "STM32FreeRtosHooks.h"
#include "app_common.h"
#include "app_entry.h"
#include "app_thread.h"
#include "cmsis_os.h"
#include "dbg_trace.h"
#include "stm32_lpm.h"
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Global variables ---------------------------------------------------------*/

RTC_HandleTypeDef hrtc = { 0 }; /**< RTC handler declaration */
RNG_HandleTypeDef hrng;
IPCC_HandleTypeDef hipcc;

void Error_Handler(void);
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void Reset_BackupDomain(void);
static void Init_RTC(void);
static void Reset_Device(void);
static void Reset_IPCC(void);
static void Init_Exti(void);
static void MX_GPIO_Init(void);
static void MX_RNG_Init(void);
static void MX_IPCC_Init(void);
static void PeriphCommonClock_Config(void);
static void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void)
{
    /* STM32WBxx HAL library initialization:
     - Configure the Flash prefetch
     - Systick timer is configured by default as source of time base, but user
     can eventually implement his proper time base source (a general purpose
     timer for example or other time source), keeping in mind that Time base
     duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
     handled in milliseconds basis.
     - Set NVIC Group Priority to 4
     - Low Level Initialization
     */
    /* USER CODE END 1 */
    /**
     * The OPTVERR flag is wrongly set at power on
     * It shall be cleared before using any HAL_FLASH_xxx() api
     */

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR);

    /**
     * Reset some configurations so that the system behave in the same way
     * when either out of nReset or Power On
     */
    Reset_Device();

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();
    PeriphCommonClock_Config();
    /* USER CODE BEGIN SysInit */
    /* Set default off value for each leds */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    Init_Exti();
    MX_RNG_Init();
    Init_RTC();
    osKernelInitialize();
    MX_GPIO_Init();
    /* IPCC initialisation */
    MX_IPCC_Init();
    freertos_mbedtls_init();
    APPE_Init();
    GetAppTask().InitMatter();
    osKernelStart();
    while (1)
    {
    }
}

static void MX_RNG_Init(void)
{

    /* USER CODE BEGIN RNG_Init 0 */

    /* USER CODE END RNG_Init 0 */

    /* USER CODE BEGIN RNG_Init 1 */

    /* USER CODE END RNG_Init 1 */
    hrng.Instance                 = RNG;
    hrng.Init.ClockErrorDetection = RNG_CED_ENABLE;
    if (HAL_RNG_Init(&hrng) != HAL_OK)
    {
    }
    /* USER CODE BEGIN RNG_Init 2 */

    /* USER CODE END RNG_Init 2 */
}

/**
 * @brief IPCC Initialization Function
 * @param None
 * @retval None
 */
static void MX_IPCC_Init(void)
{

    /* USER CODE BEGIN IPCC_Init 0 */

    /* USER CODE END IPCC_Init 0 */

    /* USER CODE BEGIN IPCC_Init 1 */

    /* USER CODE END IPCC_Init 1 */
    hipcc.Instance = IPCC;
    if (HAL_IPCC_Init(&hipcc) != HAL_OK)
    {
        Error_Handler();
    }
    /* USER CODE BEGIN IPCC_Init 2 */

    /* USER CODE END IPCC_Init 2 */
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static void Init_Exti(void)
{
    /**< Disable all wakeup interrupt on CPU1  except LPUART(25), IPCC(36), HSEM(38) */
    LL_EXTI_DisableIT_0_31((~0) & (~(LL_EXTI_LINE_25)));
    LL_EXTI_DisableIT_32_63((~0) & (~(LL_EXTI_LINE_36 | LL_EXTI_LINE_38)));

    return;
}

static void Reset_Device(void)
{
#if (CFG_HW_RESET_BY_FW == 1)
    Reset_BackupDomain();

    Reset_IPCC();
#endif

    return;
}

static void Reset_IPCC(void)
{
    LL_AHB3_GRP1_EnableClock(LL_AHB3_GRP1_PERIPH_IPCC);

    LL_C1_IPCC_ClearFlag_CHx(IPCC,
                             LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 | LL_IPCC_CHANNEL_5 |
                                 LL_IPCC_CHANNEL_6);

    LL_C2_IPCC_ClearFlag_CHx(IPCC,
                             LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 | LL_IPCC_CHANNEL_5 |
                                 LL_IPCC_CHANNEL_6);

    LL_C1_IPCC_DisableTransmitChannel(IPCC,
                                      LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 |
                                          LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

    LL_C2_IPCC_DisableTransmitChannel(IPCC,
                                      LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 |
                                          LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

    LL_C1_IPCC_DisableReceiveChannel(IPCC,
                                     LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 |
                                         LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

    LL_C2_IPCC_DisableReceiveChannel(IPCC,
                                     LL_IPCC_CHANNEL_1 | LL_IPCC_CHANNEL_2 | LL_IPCC_CHANNEL_3 | LL_IPCC_CHANNEL_4 |
                                         LL_IPCC_CHANNEL_5 | LL_IPCC_CHANNEL_6);

    return;
}

static void Reset_BackupDomain(void)
{
    if ((LL_RCC_IsActiveFlag_PINRST() != FALSE) && (LL_RCC_IsActiveFlag_SFTRST() == FALSE))
    {
        HAL_PWR_EnableBkUpAccess(); /**< Enable access to the RTC registers */

        /**
         *  Write twice the value to flush the APB-AHB bridge
         *  This bit shall be written in the register before writing the next one
         */
        HAL_PWR_EnableBkUpAccess();

        __HAL_RCC_BACKUPRESET_FORCE();
        __HAL_RCC_BACKUPRESET_RELEASE();
    }

    return;
}

static void Init_RTC(void)
{
    HAL_PWR_EnableBkUpAccess(); /**< Enable access to the RTC registers */

    /**
     *  Write twice the value to flush the APB-AHB bridge
     *  This bit shall be written in the register before writing the next one
     */
    HAL_PWR_EnableBkUpAccess();

    __HAL_RCC_RTC_CONFIG(RCC_RTCCLKSOURCE_LSE); /**< Select LSI as RTC Input */

    __HAL_RCC_RTC_ENABLE(); /**< Enable RTC */

    hrtc.Instance = RTC; /**< Define instance */

    /**
     * Set the Asynchronous prescaler
     */
    hrtc.Init.AsynchPrediv = CFG_RTC_ASYNCH_PRESCALER;
    hrtc.Init.SynchPrediv  = CFG_RTC_SYNCH_PRESCALER;
    HAL_RTC_Init(&hrtc);

    /* Disable RTC registers write protection */
    LL_RTC_DisableWriteProtection(RTC);

    LL_RTC_WAKEUP_SetClock(RTC, CFG_RTC_WUCKSEL_DIVIDER);

    /* Enable RTC registers write protection */
    LL_RTC_EnableWriteProtection(RTC);

    return;
}

/**
 * @brief  Configure the system clock
 *
 * @note   This API configures
 *         - The system clock source
 *           - The AHBCLK, APBCLK dividers
 *           - The flash latency
 *           - The PLL settings (when required)
 *
 * @param  None
 * @retval None
 */
static void PeriphCommonClock_Config(void)
{

    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = { 0 };

    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS;
    PeriphClkInitStruct.SmpsClockSelection   = RCC_SMPSCLKSOURCE_HSI;
    PeriphClkInitStruct.SmpsDivSelection     = RCC_SMPSCLKDIV_RANGE1;

    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
}

static void SystemClock_Config(void)
{
    /**
     *  Write twice the value to flush the APB-AHB bridge to ensure the  bit is written
     */
    HAL_PWR_EnableBkUpAccess(); /**< Enable access to the RTC registers */
    HAL_PWR_EnableBkUpAccess();
    /**
     * Select LSE clock
     */
    LL_RCC_LSE_Enable();
    while (!LL_RCC_LSE_IsReady())
        ;
    /**
     * Select wakeup source of BLE RF
     */
    LL_RCC_SetRFWKPClockSource(LL_RCC_RFWKP_CLKSOURCE_LSE);

    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Configure the main internal regulator output voltage
     */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType      = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState            = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState        = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource       = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM            = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN            = 16;
    RCC_OscInitStruct.PLL.PLLP            = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLR            = RCC_PLLR_DIV4;
    RCC_OscInitStruct.PLL.PLLQ            = RCC_PLLQ_DIV2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }
    /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4 | RCC_CLOCKTYPE_HCLK2 | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
        RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV2;
    RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
    {
        Error_Handler();
    }

    return;
}

static void MX_GPIO_Init(void)
{

    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();
}

/*************************************************************
 *
 * WRAP FUNCTIONS
 *
 *************************************************************/
/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM17 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim)
{
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM17)
    {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}

void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    while (1)
    {
        HAL_Delay(100);
    }
    /* USER CODE END Error_Handler_Debug */
}

void RTOS_AppConfigureTimerForRuntimeStats() {}

uint32_t RTOS_AppGetRuntimeCounterValueFromISR()

{

    return HAL_GetTick();
}
