/*******************************************************************************
 * @file  rsi_ble_config.h
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#pragma once

#include "rsi_ble_apis.h"
#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
#include "rsi_bt_common_apis.h"
#include "rsi_user.h"
#else
#include <rsi_data_types.h>
#endif

#if SL_MATTER_GN_BUILD == 0
#include "sl_matter_wifi_config.h"
#endif // SL_MATTER_GN_BUILD

/******************************************************
 * *                      Macros
 * ******************************************************/
//! application event list
// TODO: remove this define after integration of the new wifi sdk
#ifndef RSI_FAILURE
// failure return value
#define RSI_FAILURE -1
#endif

#define RSI_SSID (0x0D)
#define RSI_SECTYPE (0x0E)
#define RSI_BLE_WLAN_DISCONN_NOTIFY (0x0F)
#define RSI_WLAN_ALREADY (0x10)
#define RSI_WLAN_NOT_ALREADY (0x11)
#define RSI_BLE_WLAN_TIMEOUT_NOTIFY (0x12)
#define RSI_BLE_WLAN_JOIN_STATUS (0x15)
#define RSI_APP_FW_VERSION (0x13)
#define RSI_BLE_WLAN_DISCONNECT_STATUS (0x14)

#define RSI_REM_DEV_ADDR_LEN (18)
#define RSI_REM_DEV_NAME_LEN (31)

#define RSI_BLE_DEV_NAME "CCP_DEVICE"
#define RSI_BLE_SET_RAND_ADDR "00:23:A7:12:34:56"
#define RSI_BLE_ADDR_LENGTH 6

#define CLEAR_WHITELIST (0x00)
#define ADD_DEVICE_TO_WHITELIST (0x01)
#define DELETE_DEVICE_FROM_WHITELIST (0x02)

#define CLEAR_ACCEPTLIST (0x00)
#define ADD_DEVICE_TO_ACCEPTLIST (0x01)
#define DELETE_DEVICE_FROM_ACCEPTLIST (0x02)

#define RSI_BLE_TX_OCTETS 251
#define RSI_BLE_TX_TIME 2120 // microseconds
#define RSI_BLE_MATTER_CUSTOM_SERVICE_DATA_LENGTH 240

#define GATT_READ_ZERO_OFFSET 0x00
#define GATT_READ_RESP 0x00
#define ALL_PHYS (0x00)

#define RSI_BLE_DEV_ADDR_RESOLUTION_ENABLE (0)

#define RSI_OPERMODE_WLAN_BLE (13)

/***********************************************************************************************************************************************/
//! Characteristic Presenatation Format Fields
/***********************************************************************************************************************************************/
#define RSI_BLE_UINT8_FORMAT (0x04)
#define RSI_BLE_EXPONENT (0x00)
#define RSI_BLE_PERCENTAGE_UNITS_UUID (0x27AD)
#define RSI_BLE_NAME_SPACE (0x01)
#define RSI_BLE_DESCRIPTION (0x010B)

//! BLE characteristic custom service uuid
#define RSI_BLE_CUSTOM_SERVICE_UUID (0xFFF6)
#define RSI_BLE_CUSTOM_LEVEL_UUID (0x1FF1)

#if (defined(RSI_M4_INTERFACE) || defined(SLI_SI91X_MCU_INTERFACE))
#define RSI_BLE_MAX_NBR_ATT_REC (20)

#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
#define RSI_BLE_MAX_NBR_PERIPHERALS (1)
#else
#define RSI_BLE_MAX_NBR_SLAVES (1)
#endif // (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)

#define RSI_BLE_NUM_CONN_EVENTS (2)
#else
#define RSI_BLE_MAX_NBR_ATT_REC (80)

#if (EXP_BOARD)
#define RSI_BLE_MAX_NBR_PERIPHERALS (3)
#else
#define RSI_BLE_MAX_NBR_SLAVES (3)
#endif // (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)

#define RSI_BLE_NUM_CONN_EVENTS (20)
#endif // (defined(RSI_M4_INTERFACE) || defined(SLI_SI91X_MCU_INTERFACE))

#define RSI_BLE_MAX_NBR_ATT_SERV (10)

#define RSI_BLE_GATT_ASYNC_ENABLE (1)
#define RSI_BLE_GATT_INIT (0)

#define RSI_BLE_START_SCAN (0x01)
#define RSI_BLE_STOP_SCAN (0x00)

#define RSI_BLE_SCAN_TYPE SCAN_TYPE_ACTIVE
#define RSI_BLE_SCAN_FILTER_TYPE SCAN_FILTER_TYPE_ALL
/* Number of BLE GATT RECORD SIZE IN (n*16 BYTES), eg:(0x40*16)=1024 bytes */
#define RSI_BLE_NUM_REC_BYTES (0x40)

#define RSI_BLE_INDICATE_CONFIRMATION_FROM_HOST (0)

/*=======================================================================*/
//! Advertising command parameters
/*=======================================================================*/

#define RSI_BLE_ADV_TYPE UNDIR_CONN
#define RSI_BLE_ADV_FILTER_TYPE ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY
#define RSI_BLE_ADV_DIR_ADDR_TYPE LE_RANDOM_ADDRESS
#define RSI_BLE_ADV_DIR_ADDR "00:15:83:6A:64:17"

//! Reduced the BLE adv interval time to match with EFR BLE
#define RSI_BLE_ADV_INT_MIN (0x20)
#define RSI_BLE_ADV_INT_MAX (0x20)

#define RSI_BLE_ADV_CHANNEL_MAP (0x07)

//! Advertise status
//!  Start the advertising process
#define RSI_BLE_START_ADV (0x01)
//! Stop the advertising process
#define RSI_BLE_STOP_ADV (0x00)

//! BLE Tx Power Index On Air
#define RSI_BLE_PWR_INX (30)

//! BLE Active H/w Pwr Features
#define BLE_DISABLE_DUTY_CYCLING (0)
#define BLE_DUTY_CYCLING (1)
#define BLR_DUTY_CYCLING (2)
#define BLE_4X_PWR_SAVE_MODE (4)
#define RSI_BLE_PWR_SAVE_OPTIONS BLE_DISABLE_DUTY_CYCLING

//! Advertise types

/* Advertising will be visible(discoverable) to all the devices.
 * Scanning/Connection is also accepted from all devices
 * */
#define UNDIR_CONN (0x80)

/* Advertising will be visible(discoverable) to the particular device
 * mentioned in RSI_BLE_ADV_DIR_ADDR only.
 * Scanning and Connection will be accepted from that device only.
 * */
#define DIR_CONN (0x81)

/* Advertising will be visible(discoverable) to all the devices.
 * Scanning will be accepted from all the devices.
 * Connection will be not be accepted from any device.
 * */
#define UNDIR_SCAN (0x82)

/* Advertising will be visible(discoverable) to all the devices.
 * Scanning and Connection will not be accepted from any device
 * */
#define UNDIR_NON_CONN (0x83)

/* Advertising will be visible(discoverable) to the particular device
 * mentioned in RSI_BLE_ADV_DIR_ADDR only.
 * Scanning and Connection will be accepted from that device only.
 * */
#define DIR_CONN_LOW_DUTY_CYCLE (0x84)

//! Advertising flags
#define LE_LIMITED_DISCOVERABLE (0x01)
#define LE_GENERAL_DISCOVERABLE (0x02)
#define LE_BR_EDR_NOT_SUPPORTED (0x04)

//! Advertise filters
#define ALLOW_SCAN_REQ_ANY_CONN_REQ_ANY (0x00)
#define ALLOW_SCAN_REQ_WHITE_LIST_CONN_REQ_ANY (0x01)
#define ALLOW_SCAN_REQ_ANY_CONN_REQ_WHITE_LIST (0x02)
#define ALLOW_SCAN_REQ_WHITE_LIST_CONN_REQ_WHITE_LIST (0x03)

//! Address types
#define LE_PUBLIC_ADDRESS (0x00)
#define LE_RANDOM_ADDRESS (0x01)
#define LE_RESOLVABLE_PUBLIC_ADDRESS (0x02)
#define LE_RESOLVABLE_RANDOM_ADDRESS (0x03)

/*=======================================================================*/

/*=======================================================================*/
//! Connection parameters
/*=======================================================================*/
#define LE_SCAN_INTERVAL (0x0100)
#define LE_SCAN_WINDOW (0x0050)

#define CONNECTION_INTERVAL_MIN (0x00A0)
#define CONNECTION_INTERVAL_MAX (0x00A0)

#define CONNECTION_LATENCY (0x0000)
#define SUPERVISION_TIMEOUT (0x07D0) // 2000

/*=======================================================================*/

/*=======================================================================*/
//! Scan command parameters
/*=======================================================================*/

#define SL_WFX_BLE_SCAN_TYPE SCAN_TYPE_ACTIVE
#define SL_WFX_BLE_SCAN_FILTER_TYPE SCAN_FILTER_TYPE_ALL

//! Scan status
#define SL_WFX_BLE_START_SCAN (0x01)
#define SL_WFX_BLE_STOP_SCAN (0x00)

//! Scan types
#define SCAN_TYPE_ACTIVE (0x01)

//! Scan filters
#define SCAN_FILTER_TYPE_ALL (0x00)
#define SCAN_FILTER_TYPE_ONLY_WHITE_LIST (0x01)

#define SL_WFX_SEL_INTERNAL_ANTENNA (0x00)
#define SL_WFX_SEL_EXTERNAL_ANTENNA (0x01)

#define SL_WFX_BT_CTRL_REMOTE_USER_TERMINATED (0x4E13)
#define SL_WFX_BT_CTRL_REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES (0x4E14)
#define SL_WFX_BT_CTRL_REMOTE_POWERING_OFF (0x4E15)
#define SL_WFX_BT_CTRL_TERMINATED_MIC_FAILURE (0x4E3D)
#define SL_WFX_BT_FAILED_TO_ESTABLISH_CONN (0x4E3E)
#define SL_WFX_BT_INVALID_RANGE (0x4E60)

/***********************************************************************************************************************************************/
//! RS9116 Firmware Configurations
/***********************************************************************************************************************************************/
/*=======================================================================*/
//! Power save command parameters
/*=======================================================================*/

#define BLE_ATT_REC_SIZE (500)
#define NO_OF_VAL_ATT (5) //! Attribute value count

#if (SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
#define RSI_BLE_MAX_NBR_CENTRALS (1)
#define FRONT_END_SWITCH_SEL2 BIT(30)
#define RSI_FEATURE_BIT_MAP                                                                                                        \
    (SL_SI91X_FEAT_ULP_GPIO_BASED_HANDSHAKE | SL_SI91X_FEAT_DEV_TO_HOST_ULP_GPIO_1) //! To set wlan feature select bit map
#define RSI_TCP_IP_FEATURE_BIT_MAP                                                                                                 \
    (SL_SI91X_TCP_IP_FEAT_DHCPV4_CLIENT) //! TCP/IP feature select bitmap for selecting TCP/IP features
#define RSI_CUSTOM_FEATURE_BIT_MAP SL_SI91X_CUSTOM_FEAT_EXTENTION_VALID //! To set custom feature select bit map

// Enable front-end internal switch control for ACX module boards
#if (SL_SI91X_ACX_MODULE == 1)
#define FRONT_END_SWITCH_CTRL SL_SI91X_EXT_FEAT_FRONT_END_INTERNAL_SWITCH
#else
#define FRONT_END_SWITCH_CTRL SL_SI91X_EXT_FEAT_FRONT_END_SWITCH_PINS_ULP_GPIO_4_5_0
#endif

#ifdef SLI_SI917
#if WIFI_ENABLE_SECURITY_WPA3_TRANSITION // Adding Support for WPA3 transition
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP                                                                                             \
    (SL_SI91X_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(1) | SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV |              \
     FRONT_END_SWITCH_CTRL | SL_SI91X_EXT_FEAT_IEEE_80211W)
#else
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP                                                                                             \
    (SL_SI91X_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(1) | SL_SI91X_RAM_LEVEL_NWP_BASIC_MCU_ADV |              \
     FRONT_END_SWITCH_CTRL)
#endif /* WIFI_ENABLE_SECURITY_WPA3_TRANSITION */
#else  // EXP_BOARD
#define RSI_EXT_CUSTOM_FEATURE_BIT_MAP (SL_SI91X_EXT_FEAT_LOW_POWER_MODE | SL_SI91X_EXT_FEAT_XTAL_CLK_ENABLE(2))
#endif /* SLI_SI917 */

#define RSI_EXT_TCPIP_FEATURE_BITMAP 0
#define RSI_BT_FEATURE_BITMAP (SL_SI91X_BT_RF_TYPE | SL_SI91X_ENABLE_BLE_PROTOCOL)
#define RSI_CONFIG_FEATURE_BITMAP 0
#define RSI_TCP_IP_BYPASS RSI_ENABLE //! TCP IP BYPASS feature checks
#else
#define RSI_BLE_MAX_NBR_MASTERS (1)
#define RSI_HAND_SHAKE_TYPE GPIO_BASED
#endif
/***********************************************************************************************************************************************/
//! user defined structure
/***********************************************************************************************************************************************/
typedef struct rsi_ble_att_list_s
{
    uuid_t char_uuid;
    uint16_t handle;
    uint16_t value_len;
    uint16_t max_value_len;
    uint8_t char_val_prop;
    void * value;
} rsi_ble_att_list_t;
typedef struct rsi_ble_s
{
    uint8_t DATA[BLE_ATT_REC_SIZE];
    uint16_t DATA_ix;
    uint16_t att_rec_list_count;
    rsi_ble_att_list_t att_rec_list[NO_OF_VAL_ATT];
} rsi_ble_t;
