/* USER CODE BEGIN Header */
/*
 *
 *    Copyright (c) 2026 Project CHIP Authors
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_CONF_H
#define APP_CONF_H

/* Includes ------------------------------------------------------------------*/
#include "ble_defs.h"
#include "hw_if.h"
#include "utilities_conf.h"

/* USER CODE BEGIN Includes */
#include "FreeRTOSConfig.h"
#include "cmsis_compiler.h"
/* USER CODE END Includes */

/******************************************************************************
 * Application Config
 ******************************************************************************/
/**< generic parameters ******************************************************/

#define APP_NAME "Lighting-app"
/* Version */
#define X_CUBE_MATTER_VERSION "v1.6.0"
#define PRODUCT_NAME "Dimmable Light"
#define VENDOR_NAME "STMicroelectronics"
#define MATTER_SDK_VERSION "Master"

/* HIGHWATERMARK */
#define HIGHWATERMARK 1

/* App task define */
#define APPTASK_NAME "APPTASK"
#define APP_ATTR_BITS (0)
#define APP_CB_MEM (0)
#define APP_CB_SIZE (0)
#define APP_STACK_MEM (0)
#define APP_PRIORITY osPriorityNormal
#define APP_STACK_SIZE (1024 * 9)
/**
 * Define Tx Power
 */
#define CFG_TX_POWER (0x19) /* 0x19 <=> -0.3 dBm */

/**
 * Definition of public BD Address,
 * when CFG_BD_ADDRESS = 0x000000000000 the BD address is generated based on Unique Device Number.
 */
#define CFG_BD_ADDRESS (0x000000000000)

/**
 * Define BD_ADDR type: define proper address. Can only be GAP_PUBLIC_ADDR (0x00) or GAP_STATIC_RANDOM_ADDR (0x01)
 */
#define CFG_BD_ADDRESS_DEVICE (GAP_STATIC_RANDOM_ADDR)

/**
 * Define privacy: PRIVACY_DISABLED or PRIVACY_ENABLED
 */
#define CFG_PRIVACY (PRIVACY_DISABLED)

/**
 * Define BLE Address Type
 * Bluetooth address types defined in ble_legacy.h
 * if CFG_PRIVACY equals PRIVACY_DISABLED, CFG_BLE_ADDRESS_TYPE has 2 allowed values: GAP_PUBLIC_ADDR or GAP_STATIC_RANDOM_ADDR
 * if CFG_PRIVACY equals PRIVACY_ENABLED, CFG_BLE_ADDRESS_TYPE has 2 allowed values: GAP_RESOLVABLE_PRIVATE_ADDR or
 * GAP_NON_RESOLVABLE_PRIVATE_ADDR
 */
#define CFG_BD_ADDRESS_TYPE (GAP_STATIC_RANDOM_ADDR)

#define ADV_INTERVAL_MIN (80)
#define ADV_INTERVAL_MAX (100)
#define ADV_LP_INTERVAL_MIN (800)
#define ADV_LP_INTERVAL_MAX (1000)
#define ADV_TYPE ADV_IND
#define ADV_FILTER NO_WHITE_LIST_USE

/**
 * Define IO Authentication
 */
#define CFG_BONDING_MODE (0)
#define CFG_ENCRYPTION_KEY_SIZE_MAX (16)
#define CFG_ENCRYPTION_KEY_SIZE_MIN (8)

/**
 * Define Input Output capabilities
 */
#define CFG_IO_CAPABILITY (IO_CAP_DISPLAY_YES_NO)

/**
 * Define Man In The Middle modes
 */
#define CFG_MITM_PROTECTION (MITM_PROTECTION_REQUIRED)

/**
 * Define Secure Connections Support
 */
#define CFG_SC_SUPPORT (SC_PAIRING_OPTIONAL)

/**
 * Define Keypress Notification Support
 */
#define CFG_KEYPRESS_NOTIFICATION_SUPPORT (KEYPRESS_NOT_SUPPORTED)

/**
 *   Identity root key used to derive IRK and DHK(Legacy)
 */
#define CFG_BLE_IR                                                                                                                 \
    {                                                                                                                              \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00                             \
    }

/**
 * Encryption root key used to derive LTK(Legacy) and CSRK
 */
#define CFG_BLE_ER                                                                                                                 \
    {                                                                                                                              \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00                             \
    }

/* USER CODE BEGIN Generic_Parameters */

/* USER CODE END Generic_Parameters */

/**< specific parameters */
/*****************************************************/

/* USER CODE BEGIN Specific_Parameters */

/* USER CODE END Specific_Parameters */

/******************************************************************************
 * BLE Stack
 ******************************************************************************/
/**
 * BLE stack options, bitmap to active or not some features at BleStack_Init() function call.
 */
#define CFG_BLE_OPTIONS (0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0)

/**
 * Maximum number of simultaneous connections and advertising that the device will support.
 * This setting should not exceed the number of BLE connection supported by BLE host stack.
 */
#define CFG_BLE_NUM_LINK (2)

/**
 * Maximum number of Services that can be stored in the GATT database.
 * Note that the GAP and GATT services are automatically added so this parameter should be 2 plus the number of user services
 */
#define CFG_BLE_NUM_GATT_SERVICES (8)

/**
 * Maximum number of Attributes
 * (i.e. the number of characteristic + the number of characteristic values + the number of descriptors, excluding the services)
 * that can be stored in the GATT database.
 * Note that certain characteristics and relative descriptors are added automatically during device initialization
 * so this parameters should be 9 plus the number of user Attributes
 */
#define CFG_BLE_NUM_GATT_ATTRIBUTES (68)

/**
 * Maximum supported ATT_MTU size
 * This setting should be aligned with ATT_MTU value configured in the ble host
 */
#define CFG_BLE_ATT_MTU_MAX (251)

/**
 * Size of the storage area for Attribute values
 *  This value depends on the number of attributes used by application. In particular the sum of the following quantities (in
 * octets) should be made for each attribute:
 *  - attribute value length
 *  - 5, if UUID is 16 bit; 19, if UUID is 128 bit
 *  - 2, if server configuration descriptor is used
 *  - 2*DTM_NUM_LINK, if client configuration descriptor is used
 *  - 2, if extended properties is used
 *  The total amount of memory needed is the sum of the above quantities for each attribute.
 */
#define CFG_BLE_ATT_VALUE_ARRAY_SIZE (1344)

/**
 * Maximum numbers of bearers that can be created for Enhanced ATT per ACL links
 */
#define CFG_BLE_EATT_BEARER_PER_LINK (0)

/**
 * depth of the PREPARE WRITE queue when PREPARE WRITE REQUEST
 */
#define CFG_BLE_ATTR_PREPARE_WRITE_VALUE_SIZE (30)

#define CFG_BLE_MBLOCK_COUNT_MARGIN (0x15)

#define PREP_WRITE_LIST_SIZE (BLE_DEFAULT_PREP_WRITE_LIST_SIZE)

/**
 * Number of allocated memory blocks used to transmit and receive data packets
 */
#define CFG_BLE_MBLOCK_COUNT                                                                                                       \
    (BLE_MBLOCKS_CALC(PREP_WRITE_LIST_SIZE, CFG_BLE_ATT_MTU_MAX, CFG_BLE_NUM_LINK) + CFG_BLE_MBLOCK_COUNT_MARGIN)

/**
 * Size of the RAM buffer allocated to store BLE host events
 */
#define CFG_BLE_HOST_EVENT_BUF_SIZE (512)

/**
 * Size of the RAM buffer allocated for the extension of BLE host commands
 */
#define CFG_BLE_EXTRA_DATA_BUF_SIZE (0)

/**
 * Size of the RAM buffer allocated for long write commands, can be 0 or 256.
 */
#define CFG_BLE_LONG_WRITE_DATA_BUF_SIZE (0)

/**
 * Appearance of device set into BLE GAP
 */
#define CFG_GAP_APPEARANCE (GAP_APPEARANCE_UNKNOWN)

/**
 * Connection Oriented Channel parameters
 */
#define CFG_BLE_COC_NBR_MAX (64)
#define CFG_BLE_COC_MPS_MAX (248)
#define CFG_BLE_COC_INITIATOR_NBR_MAX (32)

/**
 * PHY preferences
 */
#define CFG_PHY_PREF (0)
#define CFG_PHY_PREF_TX (HCI_TX_PHYS_LE_2M_PREF)
#define CFG_PHY_PREF_RX (HCI_RX_PHYS_LE_2M_PREF)

/* USER CODE BEGIN BLE_Stack */

/* USER CODE END BLE_Stack */

/******************************************************************************
 * Low Power
 *
 *  When CFG_LPM_LEVEL is set to:
 *   - 0 : Low Power Mode is not activated, RUN mode will be used.
 *   - 1 : Low power active, mode(s) selected with CFG_LPM_mode_SUPPORTED
 *   - 2 : In addition log and debug are disabled to reach lowest power figures.
 ******************************************************************************/
#define CFG_LPM_LEVEL (0U)

#define CFG_LPM_STOP1_SUPPORTED (0U)
#define CFG_LPM_STOP2_SUPPORTED (0U)
#define CFG_LPM_STANDBY_SUPPORTED (0U)

/**
 * Defines to use dynamic low power wakeup time profilling.
 * With this option at boot wake up time is profiled and then is used.
 */
#define CFG_LPM_WAKEUP_TIME_PROFILING (1U)

/**
 * Defines time to wake up from standby before radio event to meet timings
 * This value will be dynamically updated when using CFG_LPM_WAKEUP_TIME_PROFILING
 */
#define CFG_LPM_STDBY_WAKEUP_TIME (1500U)

/* USER CODE BEGIN Low_Power 0 */

/* USER CODE END Low_Power 0 */

/**
 * Supported requester to the MCU Low Power Manager - can be increased up  to 32
 * It list a bit mapping of all user of the Low Power Manager
 */
typedef enum
{
    CFG_LPM_APP,
    CFG_LPM_LOG,
    CFG_LPM_LL_DEEPSLEEP,
    CFG_LPM_LL_HW_RCO_CLBR,
    CFG_LPM_APP_THREAD,
    CFG_LPM_PKA,
    CFG_LPM_PKA_OVR_IT,
    /* USER CODE BEGIN CFG_LPM_Id_t */

    /* USER CODE END CFG_LPM_Id_t */
} CFG_LPM_Id_t;

/* USER CODE BEGIN Low_Power 1 */

/* USER CODE END Low_Power 1 */

/******************************************************************************
 * RTC
 ******************************************************************************/

/* USER CODE BEGIN RTC */

/* USER CODE END RTC */

/*****************************************************************************
 * Logs
 *
 * Applications must call LOG_INFO_APP for logs.
 * By default, CFG_LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE is set to 0.
 * As a result, there is no time stamp insertion inside the logs.
 *
 * For advanced log use cases, see the log_module.h file.
 * This file is customizable, you can create new verbose levels and log regions.
 *****************************************************************************/
/**
 * Enable or disable LOG over UART in the application.
 * Low power level(CFG_LPM_LEVEL) above 1 will disable LOG.
 * Enabled low power modes above STOP1 (STOP2 or STANDBY) will disable LOG.
 */
#define CFG_LOG_SUPPORTED (1)

/* Usart used by LOG */
extern UART_HandleTypeDef huart1;
#define LOG_UART_HANDLER huart1
/* Configure Log display settings */
#define CFG_LOG_INSERT_COLOR_INSIDE_THE_TRACE (1U)
#define CFG_LOG_INSERT_TIME_STAMP_INSIDE_THE_TRACE (0U)
#define CFG_LOG_INSERT_EOL_INSIDE_THE_TRACE (0U)

#define CFG_LOG_TRACE_FIFO_SIZE (4096U)
#define CFG_LOG_TRACE_BUF_SIZE (256U)

/* USER CODE BEGIN Logs */

/* USER CODE END Logs */

/******************************************************************************
 * Configure Log level for Application
 *
 * APPLI_CONFIG_LOG_LEVEL can be any value of the Log_Verbose_Level_t enum.
 *
 * APPLI_CONFIG_LOG_REGION can either be :
 * - LOG_REGION_ALL_REGIONS to enable all regions
 * or
 * - One or several specific regions (any value except LOG_REGION_ALL_REGIONS)
 *   from the Log_Region_t enum and matching the mask value.
 *
 *   For example, to enable both LOG_REGION_BLE and LOG_REGION_APP,
 *   the value assigned to the define is :
 *   (1U << LOG_REGION_BLE | 1U << LOG_REGION_APP)
 ******************************************************************************/
#define APPLI_CONFIG_LOG_LEVEL LOG_VERBOSE_INFO
#define APPLI_CONFIG_LOG_REGION (LOG_REGION_ALL_REGIONS)
/* USER CODE BEGIN Log_level */

/* USER CODE END Log_level */

/******************************************************************************
 * Configure Serial Link used for Thread Command Line
 ******************************************************************************/
/* if OT_CLI_USE is defined to be 1, the LPM will be disabled */
#define OT_CLI_USE (0U)
extern UART_HandleTypeDef huart2;
#define OT_CLI_UART_HANDLER huart2

/******************************************************************************
 * RTOS configuration
 ******************************************************************************/

typedef enum
{
    CFG_RTOS_FLAG_RNG = 0,
    CFG_RTOS_FLAG_PKA,
    CFG_RTOS_FLAG_FlashManager,
    CFG_RTOS_FLAG_LinkLayer,
    CFG_RTOS_FLAG_OT_Tasklet,
    CFG_RTOS_FLAG_OT_Alarm_ms,
    CFG_RTOS_FLAG_OT_Alarm_us,
    CFG_RTOS_FLAG_OT_CLIuart,
    CFG_RTOS_FLAG_BLEhost,
    CFG_RTOS_FLAG_BLEhciEvt,
    CFG_RTOS_FLAG_BLEtimer,
    CFG_RTOS_FLAG_TempRadioCalib,
    CFG_RTOS_FLAG_AMM,
    /* USER CODE BEGIN CFG_RTOS_FLAG_Id_t */

    /* USER CODE END CFG_RTOS_FLAG_Id_t */
    CFG_RTOS_FLAG_LAST
} CFG_RTOS_FLAG_Id_t;
/* USER CODE BEGIN RTOS_config */

/* USER CODE END RTOS_config */
/******************************************************************************
 * NVM configuration
 ******************************************************************************/

#define CFG_SNVMA_START_SECTOR_ID ((FLASH_SIZE / FLASH_PAGE_SIZE) - 2u)

#define CFG_SNVMA_START_ADDRESS (FLASH_BASE + (FLASH_PAGE_SIZE * (CFG_SNVMA_START_SECTOR_ID)))

/* Number of 64-bit words in NVM flash area */
#define CFG_BLE_NVM_SIZE_MAX ((2048 / 8) - 4)
/**
 * This is the max size of data the THREAD Stack needs to write in NVM
 * This is different to the size allocated in the EEPROM emulator
 * The THREAD Stack shall write all data at an address in the range of [0 : (y-1)]
 * The size is a number of 32bits values
 */
#define CFG_NVMA_THREAD_NVM_SIZE (0u)

/* USER CODE BEGIN NVM_Configuration */

/* USER CODE END NVM_Configuration */

/******************************************************************************
 * Debugger
 *
 *  When CFG_DEBUGGER_LEVEL is set to:
 *   - 0 : No Debugger available, SWD/JTAG pins are disabled.
 *   - 1 : Debugger available in RUN mode only.
 *   - 2 : Debugger available in low power mode.
 *
 ******************************************************************************/
#define CFG_DEBUGGER_LEVEL (1)

/******************************************************************************
 * RealTime GPIO debug module configuration
 ******************************************************************************/

#define CFG_RT_DEBUG_GPIO_MODULE (0)
#define CFG_RT_DEBUG_DTB (0)

/******************************************************************************
 * System Clock Manager module configuration
 *
 *  When CFG_SCM_SUPPORTED is set to:
 *   - 0 : System Clock Manager is disabled and user must handle himself
 *         all clock management, taking care of radio requirements.
 *         (radio operation requires HSE 32MHz with Voltage Scaling Range 1)
 *   - 1 : System Clock Manager ensures proper clock settings and switchings
 *         according to radio requirements and user preferences
 *
 ******************************************************************************/
#define CFG_SCM_SUPPORTED (1)

/******************************************************************************
 * HW RADIO configuration
 ******************************************************************************/
/* Link Layer uses temperature based calibration (0 --> NO ; 1 --> YES) */
#define USE_TEMPERATURE_BASED_RADIO_CALIBRATION (0)

#define RADIO_INTR_NUM RADIO_IRQn /* 2.4GHz RADIO global interrupt */
#define RADIO_INTR_PRIO_HIGH                                                                                                       \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY) /* 2.4GHz RADIO interrupt priority when radio is Active */
#define RADIO_INTR_PRIO_LOW                                                                                                        \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY +                                                                                \
     2) /* 2.4GHz RADIO interrupt priority when radio is Not Active - Sleep Timer Only */

#define RADIO_SW_LOW_INTR_NUM COMP_IRQn /* Selected interrupt vector for 2.4GHz RADIO low ISR */
#define RADIO_SW_LOW_INTR_PRIO (15)     /* 2.4GHz RADIO low ISR priority */

#define RCC_INTR_PRIO (1) /* HSERDY and PLL1RDY */

/* RF TX power table ID selection:
 *   0 -> RF TX output level from -20 dBm to +10 dBm. VDDRFPA at VDD level.
 *   1 -> RF TX output level from -20 dBm to +3 dBm. VDDRFPA at VDD11 level like on ST MB1803 and MB2130 boards.
 */
#define CFG_RF_TX_POWER_TABLE_ID (0)

/* Radio sleep clock LSE accuracy configuration */
#define CFG_RADIO_LSE_SLEEP_TIMER_CUSTOM_SCA_RANGE (0x04)

/* USER CODE BEGIN Radio_Configuration */

/* USER CODE END Radio_Configuration */

/******************************************************************************
 * HW_RNG configuration
 ******************************************************************************/

/* Number of 32-bit random numbers stored in internal pool */
#define CFG_HW_RNG_POOL_SIZE (32)

/* Threshold of random numbers available before triggering pool refill */
#define CFG_HW_RNG_POOL_THRESHOLD (16)

/* USER CODE BEGIN HW_RNG_Configuration */

/* USER CODE END HW_RNG_Configuration */

/******************************************************************************
 * PKA configuration
 ******************************************************************************/
/* PKA IRQ priority of the PKA end of process */
#define PKA_INTR_PRIO_PROCEND (7)

/******************************************************************************
 * MEMORY MANAGER
 ******************************************************************************/

#define CFG_MM_POOL_SIZE (4000U) /* bytes */
#define CFG_AMM_VIRTUAL_MEMORY_NUMBER (2U)
#define CFG_AMM_VIRTUAL_BLE_TIMERS (1U)
#define CFG_AMM_VIRTUAL_BLE_TIMERS_BUFFER_SIZE (400U) /* words (32 bits) */
#define CFG_AMM_VIRTUAL_BLE_EVENTS (2U)
#define CFG_AMM_VIRTUAL_BLE_EVENTS_BUFFER_SIZE (64U) /* words (32 bits) */

/* USER CODE BEGIN MEMORY_MANAGER_Configuration */

/* USER CODE END MEMORY_MANAGER_Configuration */

/* USER CODE BEGIN Defines */
#define CFG_BSP_ON_FREERTOS (1)
#define CFG_BSP_ON_DISCOVERY (1)

/**
 * User interaction
 * When CFG_LED_SUPPORTED is set, LEDS are activated if requested
 * When CFG_JOYSTICK_SUPPORTED is set, the push button are activated if requested
 */

#define CFG_LED_SUPPORTED (1)
#define CFG_JOYSTICK_SUPPORTED (1)
#define CFG_JOYSTICK_USE_TYPE (JOYSTICK_USE_AS_BUTTON_WITH_TIME)
#define CFG_JOYSTICK_MODE (JOY_MODE_POLLING)
#define CFG_LCD_SUPPORTED (1)

/**
 * Overwrite LPM configuration imposed by OpenThread CLI Option
 */
#if (OT_CLI_USE == 1)
#if CFG_LPM_LEVEL
#undef CFG_LPM_LEVEL
#define CFG_LPM_LEVEL (0)
#endif /* CFG_LPM_LEVEL */
#if CFG_LPM_STDBY_SUPPORTED
#undef CFG_LPM_STDBY_SUPPORTED
#define CFG_LPM_STDBY_SUPPORTED (0)
#endif /* CFG_LPM_STDBY_SUPPORTED */
#endif /* OT_CLI_USE */

/**
 * Overwrite some configuration imposed by Low Power level selected.
 */
#if (CFG_LPM_LEVEL > 0)
#if CFG_LED_SUPPORTED
#undef CFG_LED_SUPPORTED
#define CFG_LED_SUPPORTED (0)
#endif /* CFG_LED_SUPPORTED */
#if CFG_LCD_SUPPORTED
#undef CFG_LCD_SUPPORTED
#define CFG_LCD_SUPPORTED (0)
#endif /* CFG_LCD_SUPPORTED */
#endif /* CFG_LPM_LEVEL */

/* USER CODE END Defines */

/**
 * Overwrite some configuration imposed by Low Power level selected.
 */
#if (CFG_LPM_LEVEL > 1)
#if CFG_LOG_SUPPORTED
#undef CFG_LOG_SUPPORTED
#define CFG_LOG_SUPPORTED (0U)
#endif /* CFG_LOG_SUPPORTED */
#if CFG_DEBUGGER_LEVEL
#undef CFG_DEBUGGER_LEVEL
#define CFG_DEBUGGER_LEVEL (0U)
#endif /* CFG_DEBUGGER_LEVEL */
#endif /* CFG_LPM_LEVEL */

#if (CFG_LPM_LEVEL == 0)
#undef CFG_LPM_STOP1_SUPPORTED
#define CFG_LPM_STOP1_SUPPORTED (0U)
#undef CFG_LPM_STOP2_SUPPORTED
#define CFG_LPM_STOP2_SUPPORTED (0U)
#undef CFG_LPM_STANDBY_SUPPORTED
#define CFG_LPM_STANDBY_SUPPORTED (0U)
#endif

#if !defined(PWR_STOP2_SUPPORT)
#undef CFG_LPM_STOP2_SUPPORTED
#define CFG_LPM_STOP2_SUPPORTED (0U)
#endif

/*********************************************************************
 * CAUTION: CFG_LPM_STDBY_SUPPORTED is deprecated and must be removed
 * Please use a combination of previous defines instead
 * Temporary define for backward compatibility
 *********************************************************************/
#if (CFG_LPM_STANDBY_SUPPORTED == 1U)
#define CFG_LPM_STDBY_SUPPORTED (1U)
#else
#define CFG_LPM_STDBY_SUPPORTED (0U)
#endif

/* USER CODE BEGIN Defines_2 */

/* USER CODE BEGIN Defines_2 */

/******************************************************************************
 * Matter Factory data
 ******************************************************************************/
#define CONFIG_STM32_FACTORY_DATA_ENABLE (0)

/******************************************************************************
 * OTA support
 ******************************************************************************/
#define OTA_SUPPORT (0)
#define OTA_EXTERNAL_FLASH_ENABLE (0)

/* USER CODE END Defines_2 */

#endif /*APP_CONF_H */
