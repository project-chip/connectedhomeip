/**
 ******************************************************************************
 * File Name          : app_conf.h
 * Description        : Application configuration file for STM32WPAN middleWare.
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
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef APP_CONF_H
#define APP_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "hw.h"
#include "hw_conf.h"
#include "hw_if.h"

/******************************************************************************
 * Application Config
 ******************************************************************************/

/**< generic parameters */

/**
 *
 * Define Tx Power
 */
#define CFG_TX_POWER (0x18) /**< 0dbm */

/**
 * Define Advertising parameters
 */
#define CFG_ADV_BD_ADDRESS (0x7257acd87a6c)
#define CFG_FAST_CONN_ADV_INTERVAL_MIN (0x80) /**< 80ms */
#define CFG_FAST_CONN_ADV_INTERVAL_MAX (0xa0) /**< 100ms */
#define CFG_LP_CONN_ADV_INTERVAL_MIN (0x640)  /**< 1s */
#define CFG_LP_CONN_ADV_INTERVAL_MAX (0xfa0)  /**< 2.5s */

/**
 * Define IO Authentication
 */
#define CFG_BONDING_MODE (1)
#define CFG_FIXED_PIN (111111)
#define CFG_USED_FIXED_PIN (0)
#define CFG_ENCRYPTION_KEY_SIZE_MAX (16)
#define CFG_ENCRYPTION_KEY_SIZE_MIN (8)

/**
 * Define IO capabilities
 */
#define CFG_IO_CAPABILITY_DISPLAY_ONLY (0x00)
#define CFG_IO_CAPABILITY_DISPLAY_YES_NO (0x01)
#define CFG_IO_CAPABILITY_KEYBOARD_ONLY (0x02)
#define CFG_IO_CAPABILITY_NO_INPUT_NO_OUTPUT (0x03)
#define CFG_IO_CAPABILITY_KEYBOARD_DISPLAY (0x04)

#define CFG_IO_CAPABILITY CFG_IO_CAPABILITY_DISPLAY_YES_NO

/**
 * Define MITM modes
 */
#define CFG_MITM_PROTECTION_NOT_REQUIRED (0x00)
#define CFG_MITM_PROTECTION_REQUIRED (0x01)

#define CFG_MITM_PROTECTION CFG_MITM_PROTECTION_REQUIRED

/**
 * Define PHY
 */
#define ALL_PHYS_PREFERENCE 0x00
#define RX_2M_PREFERRED 0x02
#define TX_2M_PREFERRED 0x02
#define TX_1M 0x01
#define TX_2M 0x02
#define RX_1M 0x01
#define RX_2M 0x02

/* freertos defines */
#define CFG_SHCI_USER_EVT_PROCESS_NAME "SHCI_USER_EVT_PROCESS"
#define CFG_SHCI_USER_EVT_PROCESS_ATTR_BITS (0)
#define CFG_SHCI_USER_EVT_PROCESS_CB_MEM (0)
#define CFG_SHCI_USER_EVT_PROCESS_CB_SIZE (0)
#define CFG_SHCI_USER_EVT_PROCESS_STACK_MEM (0)
#define CFG_SHCI_USER_EVT_PROCESS_PRIORITY osPriorityNormal
#define CFG_SHCI_USER_EVT_PROCESS_STACK_SIZE (128 * 20)

#define CFG_PUSH_BUTTON_EVT_PROCESS_NAME "PUSH_BUTTON_EVT_PROCESS"
#define CFG_PUSH_BUTTON_EVT_PROCESS_ATTR_BITS (0)
#define CFG_PUSH_BUTTON_EVT_PROCESS_CB_MEM (0)
#define CFG_PUSH_BUTTON_EVT_PROCESS_CB_SIZE (0)
#define CFG_PUSH_BUTTON_EVT_PROCESS_STACK_MEM (0)
#define CFG_PUSH_BUTTON_EVT_PROCESS_PRIORITY osPriorityNormal
#define CFG_PUSH_BUTTON_EVT_PROCESS_STACK_SIZE (128 * 4)

#define CFG_SEND_COAP_NAME "SEND_COAP_EVT_PROCESS"

#define CFG_SWITCH_PROTOCOL_EVT_PROCESS_NAME "SWITCH_PROTCOL_EVT_PROCESS"
#define CFG_SWITCH_PROTOCOL_EVT_PROCESS_ATTR_BITS (0)
#define CFG_SWITCH_PROTOCOL_EVT_PROCESS_CB_MEM (0)
#define CFG_SWITCH_PROTOCOL_EVT_PROCESS_CB_SIZE (0)
#define CFG_SWITCH_PROTOCOL_EVT_PROCESS_STACK_MEM (0)
#define CFG_SWITCH_PROTOCOL_EVT_PROCESS_PRIORITY osPriorityNormal
#define CFG_SWITCH_PROTOCOL_EVT_PROCESS_STACK_SIZE (128 * 8)

#define CFG_THREAD_MSG_M0_TO_M4_PROCESS_NAME "THREAD_MSG_M0_TO_M4_PROCESS"
#define CFG_THREAD_MSG_M0_TO_M4_PROCESS_ATTR_BITS (0)
#define CFG_THREAD_MSG_M0_TO_M4_PROCESS_CB_MEM (0)
#define CFG_THREAD_MSG_M0_TO_M4_PROCESS_CB_SIZE (0)
#define CFG_THREAD_MSG_M0_TO_M4_PROCESS_STACK_MEM (0)
#define CFG_THREAD_MSG_M0_TO_M4_PROCESS_PRIORITY osPriorityNormal
#define CFG_THREAD_MSG_M0_TO_M4_PROCESS_STACK_SIZE (128 * 8)

#define CFG_THREAD_CLI_PROCESS_NAME "THREAD_CLI_PROCESS"
#define CFG_THREAD_CLI_PROCESS_ATTR_BITS (0)
#define CFG_THREAD_CLI_PROCESS_CB_MEM (0)
#define CFG_THREAD_CLI_PROCESS_CB_SIZE (0)
#define CFG_THREAD_CLI_PROCESS_STACK_MEM (0)
#define CFG_THREAD_CLI_PROCESS_PRIORITY osPriorityNormal
#define CFG_THREAD_CLI_PROCESS_STACK_SIZE (128 * 8)

#define CFG_THREAD_SEND_COAP_MSG_PROCESS_NAME "THREAD_SEND_COAP_MSG_PROCESS"
#define CFG_THREAD_SEND_COAP_MSG_PROCESS_ATTR_BITS (0)
#define CFG_THREAD_SEND_COAP_MSG_PROCESS_CB_MEM (0)
#define CFG_THREAD_SEND_COAP_MSG_PROCESS_CB_SIZE (0)
#define CFG_THREAD_SEND_COAP_MSG_PROCESS_STACK_MEM (0)
#define CFG_THREAD_SEND_COAP_MSG_PROCESS_PRIORITY osPriorityNormal
#define CFG_THREAD_SEND_COAP_MSG_PROCESS_STACk_SIZE (128 * 8)

#define CFG_THREAD_SET_SED_MODE_PROCESS_NAME "THREAD_SET_SED_MODE_PROCESS"
#define CFG_THREAD_SET_SED_MODE_PROCESS_ATTR_BITS (0)
#define CFG_THREAD_SET_SED_MODE_PROCESS_CB_MEM (0)
#define CFG_THREAD_SET_SED_MODE_PROCESS_CB_SIZE (0)
#define CFG_THREAD_SET_SED_MODE_PROCESS_STACK_MEM (0)
#define CFG_THREAD_SET_SED_MODE_PROCESS_PRIORITY osPriorityNormal
#define CFG_THREAD_SET_SED_MODE_PROCESS_STACk_SIZE (128 * 8)

#define CFG_HCI_USER_EVT_PROCESS_NAME "HCI_USER_EVT_PROCESS"
#define CFG_HCI_USER_EVT_PROCESS_ATTR_BITS (0)
#define CFG_HCI_USER_EVT_PROCESS_CB_MEM (0)
#define CFG_HCI_USER_EVT_PROCESS_CB_SIZE (0)
#define CFG_HCI_USER_EVT_PROCESS_STACK_MEM (0)
#define CFG_HCI_USER_EVT_PROCESS_PRIORITY osPriorityNormal
#define CFG_HCI_USER_EVT_PROCESS_STACK_SIZE (128 * 40)

#define CFG_ADV_UPDATE_PROCESS_NAME "ADV_UPDATE_PROCESS"
#define CFG_ADV_UPDATE_PROCESS_ATTR_BITS (0)
#define CFG_ADV_UPDATE_PROCESS_CB_MEM (0)
#define CFG_ADV_UPDATE_PROCESS_CB_SIZE (0)
#define CFG_ADV_UPDATE_PROCESS_STACK_MEM (0)
#define CFG_ADV_UPDATE_PROCESS_PRIORITY osPriorityNormal
#define CFG_ADV_UPDATE_PROCESS_STACK_SIZE (128 * 20)

#define CFG_P2P_SERVER_PROCESS_NAME "P2P_SERVER_PROCESS"
#define CFG_P2P_SERVER_PROCESS_ATTR_BITS (0)
#define CFG_P2P_SERVER_PROCESS_CB_MEM (0)
#define CFG_P2P_SERVER_PROCESS_CB_SIZE (0)
#define CFG_P2P_SERVER_PROCESS_STACK_MEM (0)
#define CFG_P2P_SERVER_PROCESS_PRIORITY osPriorityNormal
#define CFG_P2P_SERVER_PROCESS_STACK_SIZE (128 * 20)

#define LED_PROCESS_NAME "LED_CUBE_PROCESS"
#define LED_PROCESS_ATTR_BITS (0)
#define LED_PROCESS_CB_MEM (0)
#define LED_PROCESS_CB_SIZE (0)
#define LED_PROCESS_STACK_MEM (0)
#define LED_PROCESS_PRIORITY osPriorityNormal
#define LED_PROCESS_STACK_SIZE (128 * 10)

#define APPTASK_NAME "APPTASK"
#define APP_ATTR_BITS (0)
#define APP_CB_MEM (0)
#define APP_CB_SIZE (0)
#define APP_STACK_MEM (0)
#define APP_PRIORITY osPriorityNormal
#define APP_STACK_SIZE (1024 * 6)

/**
 *   Identity root key used to derive LTK and CSRK
 */
#define CFG_BLE_IRK                                                                                                                \
    {                                                                                                                              \
        0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0, 0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0                             \
    }

/**
 * Encryption root key used to derive LTK and CSRK
 */
#define CFG_BLE_ERK                                                                                                                \
    {                                                                                                                              \
        0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, 0x21, 0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, 0x21                             \
    }

/* USER CODE BEGIN Generic_Parameters */
/**
 * SMPS supply
 * SMPS not used when Set to 0
 * SMPS used when Set to 1
 */
#define CFG_USE_SMPS 1
/* USER CODE END Generic_Parameters */

/**< specific parameters */
/*****************************************************/
#define PUSH_BUTTON_SW1_EXTI_IRQHandler EXTI4_IRQHandler
#define PUSH_BUTTON_SW2_EXTI_IRQHandler EXTI0_IRQHandler
#define PUSH_BUTTON_SW3_EXTI_IRQHandler EXTI1_IRQHandler

#define P2P_SERVER1 1 /*1 = Device is Peripherique*/

#define CFG_DEV_ID_P2P_SERVER1 (0x83)

#define RADIO_ACTIVITY_EVENT 1 /* 1 for OOB Demo */

/**
 * AD Element - Group B Feature
 */
/* LSB - First Byte */
#define CFG_FEATURE_THREAD_SWITCH (0x40)

/* LSB - Second Byte */
#define CFG_FEATURE_OTA_REBOOT (0x20)

#define CONN_L(x) ((int) ((x) / 0.625f))
#define CONN_P(x) ((int) ((x) / 1.25f))

/*  L2CAP Connection Update request parameters used for test only with smart Phone */
#define L2CAP_REQUEST_NEW_CONN_PARAM 1

#define L2CAP_INTERVAL_MIN CONN_P(1000) /* 1s */
#define L2CAP_INTERVAL_MAX CONN_P(1000) /* 1s */
#define L2CAP_SLAVE_LATENCY 0x0000
#define L2CAP_TIMEOUT_MULTIPLIER 0x1F4

/******************************************************************************
 * BLE Stack
 ******************************************************************************/
/**
 * Maximum number of simultaneous connections that the device will support.
 * Valid values are from 1 to 8
 */
#define CFG_BLE_NUM_LINK 8

/**
 * Maximum number of Services that can be stored in the GATT database.
 * Note that the GAP and GATT services are automatically added so this parameter should be 2 plus the number of user services
 */
#define CFG_BLE_NUM_GATT_SERVICES 8

/**
 * Maximum number of Attributes
 * (i.e. the number of characteristic + the number of characteristic values + the number of descriptors, excluding the services)
 * that can be stored in the GATT database.
 * Note that certain characteristics and relative descriptors are added automatically during device initialization
 * so this parameters should be 9 plus the number of user Attributes
 */
#define CFG_BLE_NUM_GATT_ATTRIBUTES 68

/**
 * Maximum supported ATT_MTU size
 */
#define CFG_BLE_MAX_ATT_MTU (251)

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
 * Prepare Write List size in terms of number of packet with ATT_MTU=23 bytes
 */
#define CFG_BLE_PREPARE_WRITE_LIST_SIZE BLE_PREP_WRITE_X_ATT(CFG_BLE_MAX_ATT_MTU)

/**
 * Number of allocated memory blocks
 */
#define CFG_BLE_MBLOCK_COUNT (BLE_MBLOCKS_CALC(CFG_BLE_PREPARE_WRITE_LIST_SIZE, CFG_BLE_MAX_ATT_MTU, CFG_BLE_NUM_LINK))

/**
 * Enable or disable the Extended Packet length feature. Valid values are 0 or 1.
 */
#define CFG_BLE_DATA_LENGTH_EXTENSION 1

/**
 * Sleep clock accuracy in Slave mode (ppm value)
 */
#define CFG_BLE_SLAVE_SCA 500

/**
 * Sleep clock accuracy in Master mode
 * 0 : 251 ppm to 500 ppm
 * 1 : 151 ppm to 250 ppm
 * 2 : 101 ppm to 150 ppm
 * 3 : 76 ppm to 100 ppm
 * 4 : 51 ppm to 75 ppm
 * 5 : 31 ppm to 50 ppm
 * 6 : 21 ppm to 30 ppm
 * 7 : 0 ppm to 20 ppm
 */
#define CFG_BLE_MASTER_SCA 0

/**
 *  Source for the 32 kHz slow speed clock
 *  1 : internal RO
 *  0 : external crystal ( no calibration )
 */
#define CFG_BLE_LSE_SOURCE 0

/**
 * Start up time of the high speed (16 or 32 MHz) crystal oscillator in units of 625/256 us (~2.44 us)
 */
#define CFG_BLE_HSE_STARTUP_TIME 0x148

/**
 * Maximum duration of the connection event when the device is in Slave mode in units of 625/256 us (~2.44 us)
 */
#define CFG_BLE_MAX_CONN_EVENT_LENGTH (0xFFFFFFFF)

/**
 * Viterbi Mode
 * 1 : enabled
 * 0 : disabled
 */
#define CFG_BLE_VITERBI_MODE 1

/**
 * BLE stack Options flags to be configured with:
 * - SHCI_C2_BLE_INIT_OPTIONS_LL_ONLY
 * - SHCI_C2_BLE_INIT_OPTIONS_LL_HOST
 * - SHCI_C2_BLE_INIT_OPTIONS_NO_SVC_CHANGE_DESC
 * - SHCI_C2_BLE_INIT_OPTIONS_WITH_SVC_CHANGE_DESC
 * - SHCI_C2_BLE_INIT_OPTIONS_DEVICE_NAME_RO
 * - SHCI_C2_BLE_INIT_OPTIONS_DEVICE_NAME_RW
 * - SHCI_C2_BLE_INIT_OPTIONS_POWER_CLASS_1
 * - SHCI_C2_BLE_INIT_OPTIONS_POWER_CLASS_2_3
 * which are used to set following configuration bits:
 * (bit 0): 1: LL only
 *          0: LL + host
 * (bit 1): 1: no service change desc.
 *          0: with service change desc.
 * (bit 2): 1: device name Read-Only
 *          0: device name R/W
 * (bit 7): 1: LE Power Class 1
 *          0: LE Power Class  2-3
 * other bits: reserved (shall be set to 0)
 */
#define CFG_BLE_OPTIONS SHCI_C2_BLE_INIT_OPTIONS_LL_HOST

#define CFG_BLE_MAX_COC_INITIATOR_NBR (32)

#define CFG_BLE_MIN_TX_POWER (0)

#define CFG_BLE_MAX_TX_POWER (0)
/******************************************************************************
 * Transport Layer
 ******************************************************************************/
/**
 * Queue length of BLE Event
 * This parameter defines the number of asynchronous events that can be stored in the HCI layer before
 * being reported to the application. When a command is sent to the BLE core coprocessor, the HCI layer
 * is waiting for the event with the Num_HCI_Command_Packets set to 1. The receive queue shall be large
 * enough to store all asynchronous events received in between.
 * When CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE is set to 27, this allow to store three 255 bytes long asynchronous events
 * between the HCI command and its event.
 * This parameter depends on the value given to CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE. When the queue size is to small,
 * the system may hang if the queue is full with asynchronous events and the HCI layer is still waiting
 * for a CC/CS event, In that case, the notification TL_BLE_HCI_ToNot() is called to indicate
 * to the application a HCI command did not receive its command event within 30s (Default HCI Timeout).
 */
#define CFG_TLBLE_EVT_QUEUE_LENGTH 5

/**
 * This parameter should be set to fit most events received by the HCI layer. It defines the buffer size of each element
 * allocated in the queue of received events and can be used to optimize the amount of RAM allocated by the Memory Manager.
 * It should not exceed 255 which is the maximum HCI packet payload size (a greater value is a lost of memory as it will
 * never be used)
 * It shall be at least 4 to receive the command status event in one frame.
 * The default value is set to 27 to allow receiving an event of MTU size in a single buffer. This value maybe reduced
 * further depending on the application.
 *
 */
#define CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE 255 /**< Set to 255 with the memory manager and the mailbox */

#define TL_BLE_EVENT_FRAME_SIZE (TL_EVT_HDR_SIZE + CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE)

/******************************************************************************
 * UART interfaces
 ******************************************************************************/
#define CFG_DEBUG_TRACE_UART hw_uart1
#define CFG_CLI_UART hw_lpuart1

/******************************************************************************
 * USB interface
 ******************************************************************************/

/**
 * Enable/Disable USB interface
 */
#define CFG_USB_INTERFACE_ENABLE 0

/******************************************************************************
 * Low Power
 *
 *  When CFG_FULL_LOW_POWER is set to 1, the system is configured in full
 *  low power mode. It means that all what can have an impact on the consumptions
 *  are powered down.(For instance LED, Access to Debugger, Etc.)
 *
 *  When CFG_FULL_LOW_POWER is set to 0, the low power mode is not activated
 *
 ******************************************************************************/

#define CFG_FULL_LOW_POWER 0

#if (CFG_FULL_LOW_POWER == 1)
#undef CFG_LPM_SUPPORTED
#define CFG_LPM_SUPPORTED 1
#endif /* CFG_FULL_LOW_POWER */

/* FOR DEBUGGING ONLY ! */
// #define CFG_LPM_SUPPORTED   1

/******************************************************************************
 * Timer Server
 ******************************************************************************/
/**
 *  CFG_RTC_WUCKSEL_DIVIDER:  This sets the RTCCLK divider to the wakeup timer.
 *  The higher is the value, the better is the power consumption and the accuracy of the timerserver
 *  The lower is the value, the finest is the granularity
 *
 *  CFG_RTC_ASYNCH_PRESCALER: This sets the asynchronous prescaler of the RTC. It should as high as possible ( to output
 *  clock as low as possible) but the output clock should be equal or higher frequency compare to the clock feeding
 *  the wakeup timer. A lower clock speed would impact the accuracy of the timer server.
 *
 *  CFG_RTC_SYNCH_PRESCALER: This sets the synchronous prescaler of the RTC.
 *  When the 1Hz calendar clock is required, it shall be sets according to other settings
 *  When the 1Hz calendar clock is not needed, CFG_RTC_SYNCH_PRESCALER should be set to 0x7FFF (MAX VALUE)
 *
 *  CFG_RTCCLK_DIVIDER_CONF:
 *  Shall be set to either 0,2,4,8,16
 *  When set to either 2,4,8,16, the 1Hhz calendar is supported
 *  When set to 0, the user sets its own configuration
 *
 *  The following settings are computed with LSI as input to the RTC
 */
#define CFG_RTCCLK_DIVIDER_CONF 0

#if (CFG_RTCCLK_DIVIDER_CONF == 0)
/**
 * Custom configuration
 * It does not support 1Hz calendar
 * It divides the RTC CLK by 16
 */
#define CFG_RTCCLK_DIV (16)
#define CFG_RTC_WUCKSEL_DIVIDER (0)
#define CFG_RTC_ASYNCH_PRESCALER (CFG_RTCCLK_DIV - 1)
#define CFG_RTC_SYNCH_PRESCALER (0x7FFF)

#else

#if (CFG_RTCCLK_DIVIDER_CONF == 2)
/**
 * It divides the RTC CLK by 2
 */
#define CFG_RTC_WUCKSEL_DIVIDER (3)
#endif

#if (CFG_RTCCLK_DIVIDER_CONF == 4)
/**
 * It divides the RTC CLK by 4
 */
#define CFG_RTC_WUCKSEL_DIVIDER (2)
#endif

#if (CFG_RTCCLK_DIVIDER_CONF == 8)
/**
 * It divides the RTC CLK by 8
 */
#define CFG_RTC_WUCKSEL_DIVIDER (1)
#endif

#if (CFG_RTCCLK_DIVIDER_CONF == 16)
/**
 * It divides the RTC CLK by 16
 */
#define CFG_RTC_WUCKSEL_DIVIDER (0)
#endif

#define CFG_RTCCLK_DIV CFG_RTCCLK_DIVIDER_CONF
#define CFG_RTC_ASYNCH_PRESCALER (CFG_RTCCLK_DIV - 1)
#define CFG_RTC_SYNCH_PRESCALER (DIVR(LSE_VALUE, (CFG_RTC_ASYNCH_PRESCALER + 1)) - 1)

#endif

/** tick timer value in us */
#define CFG_TS_TICK_VAL DIVR((CFG_RTCCLK_DIV * 1000000), LSE_VALUE)

typedef enum
{
    CFG_TIM_PROC_ID_ISR,
} CFG_TimProcID_t;

/******************************************************************************
 * Debug
 ******************************************************************************/
/**
 * When set, this resets some hw resources to set the device in the same state than the power up
 * The FW resets only register that may prevent the FW to run properly
 *
 * This shall be set to 0 in a final product
 *
 */
#define CFG_HW_RESET_BY_FW 1

/**
 * keep debugger enabled while in any low power mode when set to 1
 * should be set to 0 in production
 */
#define CFG_DEBUGGER_SUPPORTED 1

#if (CFG_FULL_LOW_POWER == 1)
#undef CFG_DEBUGGER_SUPPORTED
#define CFG_DEBUGGER_SUPPORTED 0
#endif /* CFG_FULL_LOW_POWER */

/*****************************************************************************
 * Traces
 * Enable or Disable traces in application
 * When CFG_DEBUG_TRACE is set, traces are activated
 *
 * Note : Refer to utilities_conf.h file in order to details
 *        the level of traces : CFG_DEBUG_TRACE_FULL or CFG_DEBUG_TRACE_LIGHT
 *****************************************************************************/

/**
 * When set to 1, the traces are enabled in the BLE services
 */
#define CFG_DEBUG_BLE_TRACE 1

/**
 * Enable or Disable traces in application
 */
#define CFG_DEBUG_APP_TRACE 1

#if (CFG_DEBUG_APP_TRACE != 0)
#define APP_DBG_MSG PRINT_MESG_DBG
#else
#define APP_DBG_MSG PRINT_NO_MESG
#endif
#if ((CFG_DEBUG_BLE_TRACE != 0) || (CFG_DEBUG_APP_TRACE != 0))
#define CFG_DEBUG_TRACE 1
#endif

#if (CFG_FULL_LOW_POWER == 1)
#undef CFG_DEBUG_TRACE
#define CFG_DEBUG_TRACE 0
#endif /* CFG_FULL_LOW_POWER */

/**
 * When CFG_DEBUG_TRACE_FULL is set to 1, the trace are output with the API name, the file name and the line number
 * When CFG_DEBUG_TRACE_LIGHT is set to 1, only the debug message is output
 *
 * When both are set to 0, no trace are output
 * When both are set to 1,  CFG_DEBUG_TRACE_FULL is selected
 */
#define CFG_DEBUG_TRACE_LIGHT 1
#define CFG_DEBUG_TRACE_FULL 0

#if ((CFG_DEBUG_TRACE != 0) && (CFG_DEBUG_TRACE_LIGHT == 0) && (CFG_DEBUG_TRACE_FULL == 0))
#undef CFG_DEBUG_TRACE_FULL
#undef CFG_DEBUG_TRACE_LIGHT
#define CFG_DEBUG_TRACE_FULL 0
#define CFG_DEBUG_TRACE_LIGHT 1
#endif

#if (CFG_DEBUG_TRACE == 0)
#undef CFG_DEBUG_TRACE_FULL
#undef CFG_DEBUG_TRACE_LIGHT
#define CFG_DEBUG_TRACE_FULL 0
#define CFG_DEBUG_TRACE_LIGHT 0
#endif

/**
 * When not set, the traces is looping on sending the trace over UART
 */
#define DBG_TRACE_USE_CIRCULAR_QUEUE 1

/**
 * max buffer Size to queue data traces and max data trace allowed.
 * Only Used if DBG_TRACE_USE_CIRCULAR_QUEUE is defined
 */
#define DBG_TRACE_MSG_QUEUE_SIZE (1024 * 5)
#define MAX_DBG_TRACE_MSG_SIZE 1024

/******************************************************************************
 * Configure Log level for Application
 ******************************************************************************/
#define APPLI_CONFIG_LOG_LEVEL LOG_LEVEL_INFO
#define APPLI_PRINT_FILE_FUNC_LINE 0

/* USER CODE BEGIN Defines */
/******************************************************************************
 * User interaction
 * When CFG_LED_SUPPORTED is set, LEDS are activated if requested
 * When CFG_BUTTON_SUPPORTED is set, the push button are activated if requested
 ******************************************************************************/
#if (CFG_FULL_LOW_POWER == 1)
#define CFG_LED_SUPPORTED 0
#define CFG_BUTTON_SUPPORTED 0
#else
#define CFG_LED_SUPPORTED 1
#define CFG_BUTTON_SUPPORTED 1
#endif /* CFG_FULL_LOW_POWER */
/* USER CODE END Defines */

/******************************************************************************
 * LOW POWER
 ******************************************************************************/
/**
 * Supported requester to the MCU Low Power Manager - can be increased up  to 32
 * It lists a bit mapping of all user of the Low Power Manager
 */
typedef enum
{
    CFG_LPM_APP,
    CFG_LPM_APP_BLE,
    CFG_LPM_APP_THREAD
    /* USER CODE BEGIN CFG_LPM_Id_t */

    /* USER CODE END CFG_LPM_Id_t */
} CFG_LPM_Id_t;

/******************************************************************************
 * OTP manager
 ******************************************************************************/
#define CFG_OTP_BASE_ADDRESS OTP_AREA_BASE

#define CFG_OTP_END_ADDRESS OTP_AREA_END_ADDR

typedef enum
{
    BUTTON_1 = 0x1,
} button_type_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* APP_CONF_H */
