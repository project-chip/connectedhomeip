/*******************************************************************************
 * @file  wfx_sl_ble_init.h
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
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
/*************************************************************************
 *
 */

/**
 * Include files
 * */

#ifndef WFX_SL_BLE_INIT
#define WFX_SL_BLE_INIT
#define RSI_BLE_ENABLE 1

// BLE include file to refer BLE APIs
#include "FreeRTOS.h"
#include "event_groups.h"
#include "rsi_ble_config.h"
#include "task.h"
#include "timers.h"
#include "wfx_host_events.h"
#include "wfx_rsi.h"
#include <rsi_ble.h>
#include <rsi_ble_apis.h>
#include <rsi_ble_common_config.h>
#include <rsi_ble_config.h>
#include <rsi_bootup_config.h>
#include <rsi_bt_common.h>
#include <rsi_bt_common_apis.h>
#include <rsi_common_apis.h>
#include <rsi_driver.h>
#include <rsi_wlan_apis.h>
#include <rsi_wlan_config.h>
#include <rsi_wlan_non_rom.h>
#include <stdbool.h>
#include <string.h>

#ifdef RSI_M4_INTERFACE
#include "rsi_board.h"
#endif

#define ATT_REC_IN_HOST 0

#define RSI_BLE_MATTER_CUSTOM_SERVICE_UUID 0
#define RSI_BLE_MATTER_CUSTOM_SERVICE_SIZE 2
#define RSI_BLE_MATTER_CUSTOM_SERVICE_VALUE_16 0xFFF6
#define RSI_BLE_MATTER_CUSTOM_SERVICE_DATA 0x00

#define RSI_BLE_CUSTOM_CHARACTERISTIC_RX_SIZE 16
#define RSI_BLE_CUSTOM_CHARACTERISTIC_RX_RESERVED 0x00, 0x00, 0x00
#define RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_1 0x18EE2EF5
#define RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_2 0x263D
#define RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_3 0x4559
#define RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_4 0x9F, 0x95, 0x9C, 0x4F, 0x11, 0x9D, 0x9F, 0x42
#define RSI_BLE_CHARACTERISTIC_RX_ATTRIBUTE_HANDLE_LOCATION 1
#define RSI_BLE_CHARACTERISTIC_RX_VALUE_HANDLE_LOCATION 2

#define RSI_BLE_CUSTOM_CHARACTERISTIC_TX_SIZE 16
#define RSI_BLE_CUSTOM_CHARACTERISTIC_TX_RESERVED 0x00, 0x00, 0x00
#define RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_1 0x18EE2EF5
#define RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_2 0x263D
#define RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_3 0x4559
#define RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_4 0x9F, 0x95, 0x9C, 0x4F, 0x12, 0x9D, 0x9F, 0x42
#define RSI_BLE_CHARACTERISTIC_TX_ATTRIBUTE_HANDLE_LOCATION 3
#define RSI_BLE_CHARACTERISTIC_TX_MEASUREMENT_HANDLE_LOCATION 4
#define RSI_BLE_CHARACTERISTIC_TX_GATT_SERVER_CLIENT_HANDLE_LOCATION 5

typedef struct sl_wfx_msg_s
{
    uint8_t connectionHandle;
    uint8_t bondingHandle;
    uint32_t event_num;
    uint16_t reason;
    uint16_t event_id;
    uint16_t resp_status;
    rsi_ble_event_mtu_t rsi_ble_mtu;
    rsi_ble_event_write_t rsi_ble_write;
    rsi_ble_event_enhance_conn_status_t resp_enh_conn;
    rsi_ble_event_disconnect_t * resp_disconnect;
    rsi_ble_set_att_resp_t rsi_ble_event_set_att_rsp;
    uint32_t ble_app_event_map;
    uint32_t ble_app_event_mask;
    uint16_t rsi_ble_measurement_hndl;
    uint16_t rsi_ble_gatt_server_client_config_hndl;
} sl_wfx_msg_t;

int32_t ble_rsi_task(void);
void rsi_ble_on_connect_event(rsi_ble_event_conn_status_t * resp_conn);
void rsi_ble_on_disconnect_event(rsi_ble_event_disconnect_t * resp_disconnect, uint16_t reason);
void rsi_ble_on_enhance_conn_status_event(rsi_ble_event_enhance_conn_status_t * resp_enh_conn);
void rsi_ble_on_gatt_write_event(uint16_t event_id, rsi_ble_event_write_t * rsi_ble_write);
void rsi_ble_on_mtu_event(rsi_ble_event_mtu_t * rsi_ble_mtu);
void rsi_ble_on_event_indication_confirmation(uint16_t resp_status, rsi_ble_set_att_resp_t * rsi_ble_event_set_att_rsp);
void rsi_gatt_add_attribute_to_list(rsi_ble_t * p_val, uint16_t handle, uint16_t data_len, uint8_t * data, uuid_t uuid,
                                    uint8_t char_prop);
void rsi_ble_add_char_serv_att(void * serv_handler, uint16_t handle, uint8_t val_prop, uint16_t att_val_handle,
                               uuid_t att_val_uuid);
void rsi_ble_add_char_val_att(void * serv_handler, uint16_t handle, uuid_t att_type_uuid, uint8_t val_prop, uint8_t * data,
                              uint8_t data_len, uint8_t auth_read);
uint32_t rsi_ble_add_matter_service(void);
void rsi_ble_app_set_event(uint32_t event_num);
int32_t rsi_ble_app_get_event(void);
void rsi_ble_app_clear_event(uint32_t event_num);
void rsi_ble_app_init_events();
void rsi_ble_event_handling_task(void);

#endif