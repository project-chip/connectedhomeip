/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_debug.c
 * @author  MCD Application Team
 * @brief   Debug capabilities source file for STM32WPAN Middleware
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2020-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_common.h"

#include "app_debug.h"
#include "dbg_trace.h"
#include "shci.h"
#include "tl.h"
#include "utilities_common.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef PACKED_STRUCT
{
    GPIO_TypeDef * port;
    uint16_t pin;
    uint8_t enable;
    uint8_t reserved;
}
APPD_GpioConfig_t;
/* USER CODE END PTD */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define GPIO_NBR_OF_RF_SIGNALS 9
#define GPIO_CFG_NBR_OF_FEATURES 43
#define NBR_OF_TRACES_CONFIG_PARAMETERS 4
#define NBR_OF_GENERAL_CONFIG_PARAMETERS 4

/**
 * THIS SHALL BE SET TO A VALUE DIFFERENT FROM 0 ONLY ON REQUEST FROM ST SUPPORT
 */
#define BLE_DTB_CFG 0

/**
 * System Debug Options flags to be configured with:
 * - SHCI_C2_DEBUG_OPTIONS_IPCORE_LP
 * - SHCI_C2_DEBUG_OPTIONS_IPCORE_NO_LP
 * - SHCI_C2_DEBUG_OPTIONS_CPU2_STOP_EN
 * - SHCI_C2_DEBUG_OPTIONS_CPU2_STOP_DIS
 * which are used to set following configuration bits:
 * - bit 0:   0: IP BLE core in LP mode    1: IP BLE core in run mode (no LP supported)
 * - bit 1:   0: CPU2 STOP mode Enable     1: CPU2 STOP mode Disable
 * - bit [2-7]: bits reserved ( shall be set to 0)
 */
#define SYS_DBG_CFG1 (SHCI_C2_DEBUG_OPTIONS_IPCORE_LP | SHCI_C2_DEBUG_OPTIONS_CPU2_STOP_EN)
/* USER CODE END PD */

/* Private macros ------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
PLACE_IN_SECTION("MB_MEM2") ALIGN(4) static SHCI_C2_DEBUG_TracesConfig_t APPD_TracesConfig = { 0, 0, 0, 0 };
PLACE_IN_SECTION("MB_MEM2")
ALIGN(4) static SHCI_C2_DEBUG_GeneralConfig_t APPD_GeneralConfig = { BLE_DTB_CFG, SYS_DBG_CFG1, { 0, 0 } };

#ifdef CFG_DEBUG_TRACE_UART
#if (CFG_HW_LPUART1_ENABLED == 1)
extern void MX_LPUART1_UART_Init(void);
#endif
#if (CFG_HW_USART1_ENABLED == 1)
extern void MX_USART1_UART_Init(void);
#endif
#endif

/**
 * THE DEBUG ON GPIO FOR CPU2 IS INTENDED TO BE USED ONLY ON REQUEST FROM ST SUPPORT
 * It provides timing information on the CPU2 activity.
 * All configuration of (port, pin) is supported for each features and can be selected by the user
 * depending on the availability
 */
static const APPD_GpioConfig_t aGpioConfigList[GPIO_CFG_NBR_OF_FEATURES] = {
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* BLE_ISR - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* BLE_STACK_TICK - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* BLE_CMD_PROCESS - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* BLE_ACL_DATA_PROCESS - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* SYS_CMD_PROCESS - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* RNG_PROCESS - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* NVM_PROCESS - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_GENERAL - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_BLE_CMD_RX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_BLE_EVT_TX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_BLE_ACL_DATA_RX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_SYS_CMD_RX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_SYS_EVT_TX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_CLI_CMD_RX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_OT_CMD_RX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_OT_ACK_TX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_CLI_ACK_TX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_MEM_MANAGER_RX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IPCC_TRACES_TX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* HARD_FAULT - Set on Entry / Reset on Exit */
                                    /* From v1.1.1 */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* IP_CORE_LP_STATUS - Set on Entry / Reset on Exit */
                                    /* From v1.2.0 */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* END_OF_CONNECTION_EVENT - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* TIMER_SERVER_CALLBACK - Toggle on Entry */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* PES_ACTIVITY - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* MB_BLE_SEND_EVT - Set on Entry / Reset on Exit */
                                    /* From v1.3.0 */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* BLE_NO_DELAY - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* BLE_STACK_STORE_NVM_CB - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* NVMA_WRITE_ONGOING - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* NVMA_WRITE_COMPLETE - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* NVMA_CLEANUP - Set on Entry / Reset on Exit */
                                    /* From v1.4.0 */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 }, /* NVMA_START - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },
    /* FLASH_EOP - Set on Entry / Reset on Exit */ /* The FLASH_EOP Debug GPIO trace is not supported since v1.5.0 */
                                                   /* From v1.5.0 */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* FLASH_WRITE - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* FLASH_ERASE - Set on Entry / Reset on Exit */
                                                   /* From v1.6.0 */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* BLE_RESCHEDULE_EVENT - Set on Entry / Reset on Exit */
                                                   /* From v1.8.0 */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* IPCC_BLE_LLD_CMD_RX - Set on Entry / Reset on Exit */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* IPCC_BLE_LLD_ACK_TX - Set on Entry / Reset on Exit */
                                                   /* From v1.9.0 */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* BLE_ASYNCH_EVENT_NACKED - Set on Entry / Reset on Exit */
                                                   /* From v1.17.0 */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* DYNAMIC CONCURRENT - RTSM_SFTIMER_IRQ */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* DYNAMIC CONCURRENT - RTSM_COMPC_WRAP */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* DYNAMIC CONCURRENT - RTSM_SWITCH_RADIO */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* DYNAMIC CONCURRENT - RTSM_PROG_SFTIMER */
    { GPIOA, LL_GPIO_PIN_0, 0, 0 },                /* DYNAMIC CONCURRENT - RTSM_RADIO_GRANTED_TO_15_4 */
};

/**
 * THE DEBUG ON GPIO FOR CPU2 IS INTENDED TO BE USED ONLY ON REQUEST FROM ST SUPPORT
 * This table is relevant only for BLE
 * It provides timing information on BLE RF activity.
 * New signals may be allocated at any location when requested by ST
 * The GPIO allocated to each signal depend on the BLE_DTB_CFG value and cannot be changed
 */
#if (BLE_DTB_CFG == 7)
static const APPD_GpioConfig_t aRfConfigList[GPIO_NBR_OF_RF_SIGNALS] = {
    { GPIOB, LL_GPIO_PIN_2, 0, 0 },  /* DTB10 - Tx/Rx SPI */
    { GPIOB, LL_GPIO_PIN_7, 0, 0 },  /* DTB11 - Tx/Tx SPI Clk */
    { GPIOA, LL_GPIO_PIN_8, 0, 0 },  /* DTB12 - Tx/Rx Ready & SPI Select */
    { GPIOA, LL_GPIO_PIN_9, 0, 0 },  /* DTB13 - Tx/Rx Start */
    { GPIOA, LL_GPIO_PIN_10, 0, 0 }, /* DTB14 - FSM0 */
    { GPIOA, LL_GPIO_PIN_11, 0, 0 }, /* DTB15 - FSM1 */
    { GPIOB, LL_GPIO_PIN_8, 0, 0 },  /* DTB16 - FSM2 */
    { GPIOB, LL_GPIO_PIN_11, 0, 0 }, /* DTB17 - FSM3 */
    { GPIOB, LL_GPIO_PIN_10, 0, 0 }, /* DTB18 - FSM4 */
};
#endif
/* USER CODE END PV */

/* Global variables ----------------------------------------------------------*/
/* USER CODE BEGIN GV */
/* USER CODE END GV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static void APPD_SetCPU2GpioConfig(void);
static void APPD_BleDtbCfg(void);
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/
void APPD_Init(void)
{
    /* USER CODE BEGIN APPD_Init */
    APPD_SetCPU2GpioConfig();
    APPD_BleDtbCfg();

    /* USER CODE END APPD_Init */
    return;
}

void APPD_EnableCPU2(void)
{
    /* USER CODE BEGIN APPD_EnableCPU2 */
    SHCI_C2_DEBUG_Init_Cmd_Packet_t DebugCmdPacket = { { { 0, 0, 0 } }, /**< Does not need to be initialized */
                                                       { (uint8_t *) aGpioConfigList, (uint8_t *) &APPD_TracesConfig,
                                                         (uint8_t *) &APPD_GeneralConfig, GPIO_CFG_NBR_OF_FEATURES,
                                                         NBR_OF_TRACES_CONFIG_PARAMETERS, NBR_OF_GENERAL_CONFIG_PARAMETERS } };

    /**< Traces channel initialization */
    TL_TRACES_Init();

    /** GPIO DEBUG Initialization */
    SHCI_C2_DEBUG_Init(&DebugCmdPacket);

    /* USER CODE END APPD_EnableCPU2 */
    return;
}

/*************************************************************
 *
 * LOCAL FUNCTIONS
 *
 *************************************************************/
static void APPD_SetCPU2GpioConfig(void)
{
    /* USER CODE BEGIN APPD_SetCPU2GpioConfig */
    GPIO_InitTypeDef gpio_config = { 0 };
    uint8_t local_loop;
    uint16_t gpioa_pin_list;
    uint16_t gpiob_pin_list;
    uint16_t gpioc_pin_list;

    gpioa_pin_list = 0;
    gpiob_pin_list = 0;
    gpioc_pin_list = 0;

    for (local_loop = 0; local_loop < GPIO_CFG_NBR_OF_FEATURES; local_loop++)
    {
        if (aGpioConfigList[local_loop].enable != 0)
        {
            switch ((uint32_t) aGpioConfigList[local_loop].port)
            {
            case (uint32_t) GPIOA:
                gpioa_pin_list |= aGpioConfigList[local_loop].pin;
                break;

            case (uint32_t) GPIOB:
                gpiob_pin_list |= aGpioConfigList[local_loop].pin;
                break;

            case (uint32_t) GPIOC:
                gpioc_pin_list |= aGpioConfigList[local_loop].pin;
                break;

            default:
                break;
            }
        }
    }

    gpio_config.Pull  = GPIO_NOPULL;
    gpio_config.Mode  = GPIO_MODE_OUTPUT_PP;
    gpio_config.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    if (gpioa_pin_list != 0)
    {
        gpio_config.Pin = gpioa_pin_list;
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_C2GPIOA_CLK_ENABLE();
        HAL_GPIO_Init(GPIOA, &gpio_config);
        HAL_GPIO_WritePin(GPIOA, gpioa_pin_list, GPIO_PIN_RESET);
    }

    if (gpiob_pin_list != 0)
    {
        gpio_config.Pin = gpiob_pin_list;
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_C2GPIOB_CLK_ENABLE();
        HAL_GPIO_Init(GPIOB, &gpio_config);
        HAL_GPIO_WritePin(GPIOB, gpiob_pin_list, GPIO_PIN_RESET);
    }

    if (gpioc_pin_list != 0)
    {
        gpio_config.Pin = gpioc_pin_list;
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_C2GPIOC_CLK_ENABLE();
        HAL_GPIO_Init(GPIOC, &gpio_config);
        HAL_GPIO_WritePin(GPIOC, gpioc_pin_list, GPIO_PIN_RESET);
    }

    /* USER CODE END APPD_SetCPU2GpioConfig */
    return;
}

static void APPD_BleDtbCfg(void)
{
/* USER CODE BEGIN APPD_BleDtbCfg */
#if (BLE_DTB_CFG != 0)
    GPIO_InitTypeDef gpio_config = { 0 };
    uint8_t local_loop;
    uint16_t gpioa_pin_list;
    uint16_t gpiob_pin_list;

    gpioa_pin_list = 0;
    gpiob_pin_list = 0;

    for (local_loop = 0; local_loop < GPIO_NBR_OF_RF_SIGNALS; local_loop++)
    {
        if (aRfConfigList[local_loop].enable != 0)
        {
            switch ((uint32_t) aRfConfigList[local_loop].port)
            {
            case (uint32_t) GPIOA:
                gpioa_pin_list |= aRfConfigList[local_loop].pin;
                break;

            case (uint32_t) GPIOB:
                gpiob_pin_list |= aRfConfigList[local_loop].pin;
                break;

            default:
                break;
            }
        }
    }

    gpio_config.Pull      = GPIO_NOPULL;
    gpio_config.Mode      = GPIO_MODE_AF_PP;
    gpio_config.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio_config.Alternate = GPIO_AF6_RF_DTB7;

    if (gpioa_pin_list != 0)
    {
        gpio_config.Pin = gpioa_pin_list;
        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_C2GPIOA_CLK_ENABLE();
        HAL_GPIO_Init(GPIOA, &gpio_config);
    }

    if (gpiob_pin_list != 0)
    {
        gpio_config.Pin = gpiob_pin_list;
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_C2GPIOB_CLK_ENABLE();
        HAL_GPIO_Init(GPIOB, &gpio_config);
    }
#endif

    /* USER CODE END APPD_BleDtbCfg */
    return;
}
