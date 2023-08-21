/**
 ******************************************************************************
  * File Name          : tl_dbg_conf.h
  * Description        : Debug configuration file for stm32wpan transport layer interface.
  *
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
#ifndef __TL_DBG_CONF_H
#define __TL_DBG_CONF_H

/* USER CODE BEGIN Tl_Conf */

/* Includes ------------------------------------------------------------------*/
#include "app_conf.h"   /* required as some configuration used in dbg_trace.h are set there */
#include "dbg_trace.h"
#include "hw_if.h"

/**
 * Enable or Disable traces
 * The raw data output is the hci binary packet format as specified by the BT specification *
 */
#define TL_SHCI_CMD_DBG_EN      0   /* Reports System commands sent to CPU2 and the command response */
#define TL_SHCI_CMD_DBG_RAW_EN  0   /* Reports raw data System commands sent to CPU2 and the command response */
#define TL_SHCI_EVT_DBG_EN      0   /* Reports System Asynchronous Events received from CPU2 */
#define TL_SHCI_EVT_DBG_RAW_EN  0   /* Reports raw data System Asynchronous Events received from CPU2 */

#define TL_HCI_CMD_DBG_EN       0   /* Reports BLE command sent to CPU2 and the command response */
#define TL_HCI_CMD_DBG_RAW_EN   0   /* Reports raw data BLE command sent to CPU2 and the command response */
#define TL_HCI_EVT_DBG_EN       0   /* Reports BLE Asynchronous Events received from CPU2 */
#define TL_HCI_EVT_DBG_RAW_EN   0   /* Reports raw data BLE Asynchronous Events received from CPU2 */

#define TL_MM_DBG_EN            0   /* Reports the information of the buffer released to CPU2 */

/**
 * Macro definition
 */

/**
 * System Transport Layer
 */
#if (TL_SHCI_CMD_DBG_EN != 0)
#define TL_SHCI_CMD_DBG_MSG             PRINT_MESG_DBG
#define TL_SHCI_CMD_DBG_BUF             PRINT_LOG_BUFF_DBG
#else
#define TL_SHCI_CMD_DBG_MSG(...)
#define TL_SHCI_CMD_DBG_BUF(...)
#endif

#if (TL_SHCI_CMD_DBG_RAW_EN != 0)
#define TL_SHCI_CMD_DBG_RAW(_PDATA_, _SIZE_)  HW_UART_Transmit(hw_uart1, (uint8_t*)_PDATA_, _SIZE_, (~0))
#else
#define TL_SHCI_CMD_DBG_RAW(...)
#endif

#if (TL_SHCI_EVT_DBG_EN != 0)
#define TL_SHCI_EVT_DBG_MSG             PRINT_MESG_DBG
#define TL_SHCI_EVT_DBG_BUF             PRINT_LOG_BUFF_DBG
#else
#define TL_SHCI_EVT_DBG_MSG(...)
#define TL_SHCI_EVT_DBG_BUF(...)
#endif

#if (TL_SHCI_EVT_DBG_RAW_EN != 0)
#define TL_SHCI_EVT_DBG_RAW(_PDATA_, _SIZE_)  HW_UART_Transmit(hw_uart1, (uint8_t*)_PDATA_, _SIZE_, (~0))
#else
#define TL_SHCI_EVT_DBG_RAW(...)
#endif

/**
 * BLE Transport Layer
 */
#if (TL_HCI_CMD_DBG_EN != 0)
#define TL_HCI_CMD_DBG_MSG             PRINT_MESG_DBG
#define TL_HCI_CMD_DBG_BUF             PRINT_LOG_BUFF_DBG
#else
#define TL_HCI_CMD_DBG_MSG(...)
#define TL_HCI_CMD_DBG_BUF(...)
#endif

#if (TL_HCI_CMD_DBG_RAW_EN != 0)
#define TL_HCI_CMD_DBG_RAW(_PDATA_, _SIZE_)  HW_UART_Transmit(hw_uart1, (uint8_t*)_PDATA_, _SIZE_, (~0))
#else
#define TL_HCI_CMD_DBG_RAW(...)
#endif

#if (TL_HCI_EVT_DBG_EN != 0)
#define TL_HCI_EVT_DBG_MSG             PRINT_MESG_DBG
#define TL_HCI_EVT_DBG_BUF             PRINT_LOG_BUFF_DBG
#else
#define TL_HCI_EVT_DBG_MSG(...)
#define TL_HCI_EVT_DBG_BUF(...)
#endif

#if (TL_HCI_EVT_DBG_RAW_EN != 0)
#define TL_HCI_EVT_DBG_RAW(_PDATA_, _SIZE_)  HW_UART_Transmit(hw_uart1, (uint8_t*)_PDATA_, _SIZE_, (~0))
#else
#define TL_HCI_EVT_DBG_RAW(...)
#endif

/**
 * Memory Manager - Released buffer tracing
 */
#if (TL_MM_DBG_EN != 0)
#define TL_MM_DBG_MSG             PRINT_MESG_DBG
#else
#define TL_MM_DBG_MSG(...)
#endif

/* USER CODE END Tl_Conf */

#endif /*__TL_DBG_CONF_H */

