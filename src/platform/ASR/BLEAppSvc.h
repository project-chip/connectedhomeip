/*
 * Copyright (c) 2022 ASR Microelectronics (Shanghai) Co., Ltd. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 ****************************************************************************************
 *
 * @file \app_custom_svc.h
 *
 * @brief custom service db
 *
 ****************************************************************************************
 */
#ifndef _APP_CUSTOM_SVC_H_
#define _APP_CUSTOM_SVC_H_
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <platform/DeviceInstanceInfoProvider.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "sonata_ble_api.h"
#include "sonata_gatt_api.h"

/*
 * MACRO DEFINES
 ****************************************************************************************
 */

/*
 * ENUM DEFINITIONS
 ****************************************************************************************
 */

enum csvc_att_db_handles
{
    CSVC_IDX_SVC,
    CSVC_IDX_RX_CHAR,
    CSVC_IDX_RX_VAL,
    CSVC_IDX_TX_CHAR,
    CSVC_IDX_TX_VAL,
    CSVC_IDX_TX_CFG,
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    CSVC_IDX_C3_CHAR,
    CSVC_IDX_C3_VAL,
#endif
    CSVC_IDX_NB,
};

enum _matter_event_states
{
    MATTER_EVENT_DONE_CONTINUE,
    MATTER_EVENT_DONE_FINISHED,
};

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void matter_ble_add_service();

uint16_t matter_ble_complete_event_handler(int opt_id, uint8_t status, uint16_t param, uint32_t dwparam);

void matter_ble_stop_adv();

void matter_ble_start_adv(bool fast);

void matter_set_connection_id(uint8_t conId);
void matter_close_connection(uint8_t conId);
void matter_tx_CCCD_write_cb(uint8_t * data, uint16_t size);
void matter_rx_char_write_cb(uint8_t * data, uint16_t size);
void matter_tx_CCCD_read_cb(uint8_t * data, uint16_t * size);
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
void matter_c3_char_read_cb(uint8_t * data, uint16_t * size);
#endif
void matter_tx_char_send_indication(uint8_t conId, uint16_t size, uint8_t * data);

void matter_init_callback(void);
void matter_ble_stack_open(void);

#ifdef __cplusplus
}
#endif
#endif // BLE_SOC_APP_CUSTOM_SVC_H
