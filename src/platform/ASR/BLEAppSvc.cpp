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
 * @file \app_custom_svc.c
 *
 * @brief custom service db
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif
#include "app.h"
#include "sonata_att.h"
#include "sonata_gap.h"
#include "sonata_gap_api.h"
#include "sonata_gatt_api.h"
#include <stdio.h>
#ifdef __cplusplus
}
#endif
#include "BLEAppSvc.h"
#include <ble/CHIPBleServiceData.h>
#include <platform/ConfigurationManager.h>
#include <platform/internal/BLEManager.h>
using namespace chip::DeviceLayer;
using namespace chip::DeviceLayer::Internal;

static uint8_t current_connect_id = SONATA_ADDR_NONE;
static uint16_t service_handle    = 0;
/*
 * MACRO DEFINES
 ****************************************************************************************
 */
#define APPSVC "APP_SVC"
/*
 * ENUM DEFINITIONS
 ****************************************************************************************
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 ****************************************************************************************
 */

ble_gatt_att_reg_t matter_csvc_atts[CSVC_IDX_NB] = {
    [CSVC_IDX_SVC] = { { { 0xF6, 0xFF, 0 }, 0, 0, 0 }, { 0, 0 } },
    // ATT_DECL_CHARACTERISTIC (Write)
    [CSVC_IDX_RX_CHAR] = { { { 0X03, 0X28, 0 }, PRD_NA, 0, 0 }, { 0, 0 } },
    [CSVC_IDX_RX_VAL]  = { { { 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 },
                             PWR_NA,
                             247,
                             PRI | SONATA_PERM(UUID_LEN, UUID_128) },
                           { matter_rx_char_write_cb, 0 } },

    // ATT_DECL_CHARACTERISTIC (Read,Notify)
    [CSVC_IDX_TX_CHAR] = { { { 0X03, 0X28, 0 }, PRD_NA, 0, 0 }, { 0, 0 } },
    [CSVC_IDX_TX_VAL]  = { { { 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 },
                             PRD_NA | PNTF_NA | PIND_NA,
                             247,
                             SONATA_PERM(UUID_LEN, UUID_128) },
                           { 0, 0 } },
    [CSVC_IDX_TX_CFG]  = { { { 0X02, 0X29, 0 }, PRD_NA | PWR_NA, 2, PRI }, { matter_tx_CCCD_write_cb, matter_tx_CCCD_read_cb } },
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    [CSVC_IDX_C3_CHAR] = { { { 0X03, 0X28, 0 }, PRD_NA, 0, 0 }, { 0, 0 } },
    [CSVC_IDX_C3_VAL]  = { { { 0x04, 0x8F, 0x21, 0x83, 0x8A, 0x74, 0x7D, 0xB8, 0xF2, 0x45, 0x72, 0x87, 0x38, 0x02, 0x63, 0x64 },
                             PRD_NA,
                             512,
                             PRI | SONATA_PERM(UUID_LEN, UUID_128) },
                           { 0, matter_c3_char_read_cb } },
#endif
};
/*
 * FUNCTION DEFINITIONS
 ****************************************************************************************
 */
void matter_ble_stack_open(void)
{
    ChipLogProgress(DeviceLayer, "matter_ble_stack_open");
    app_ble_stack_start(USER_MATTER_MODULE_ID);
}

void matter_ble_start_adv(bool fast)
{
    ble_adv_data_t data;
    ble_scan_data_t scan_data;
    sonata_gap_directed_adv_create_param_t param = { 0 };

    memset(&data, 0, sizeof(ble_adv_data_t));
    memset(&scan_data, 0, sizeof(ble_scan_data_t));
    BLEMgrImpl().SetAdvertisingData((uint8_t *) &data.ble_advdata, (uint8_t *) &data.ble_advdataLen);
    BLEMgrImpl().SetScanRspData((uint8_t *) &scan_data.ble_respdata, (uint8_t *) &scan_data.ble_respdataLen);

    param.disc_mode  = SONATA_GAP_ADV_MODE_GEN_DISC;
    param.prop       = SONATA_GAP_ADV_PROP_UNDIR_CONN_MASK;
    param.max_tx_pwr = 0xE2;
    param.filter_pol = SONATA_ADV_ALLOW_SCAN_ANY_CON_ANY;
    param.addr_type  = SONATA_GAP_STATIC_ADDR;
    param.chnl_map   = 0x07;
    param.phy        = SONATA_GAP_PHY_LE_1MBPS;
    if (fast)
    {
        param.adv_intv_min = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
        param.adv_intv_max = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
        ChipLogProgress(DeviceLayer, "fast advertising");
    }
    else
    {
        param.adv_intv_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        param.adv_intv_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
        ChipLogProgress(DeviceLayer, "slow advertising");
    }
    app_ble_start_advertising_with_param(APP_MATTER_ADV_IDX, SONATA_GAP_STATIC_ADDR, &param, &data, &scan_data);
}

void matter_ble_stop_adv(void)
{
    app_ble_advertising_stop(APP_MATTER_ADV_IDX);
}
void matter_ble_add_service()
{
    int ret = 0;
    ret = app_ble_gatt_add_svc_helper(&service_handle, sizeof(matter_csvc_atts) / sizeof(ble_gatt_att_reg_t), 1, matter_csvc_atts);
    if (ret != 0)
    {
        ChipLogError(DeviceLayer, "matter_ble_add_service add service failed");
    }
    else
    {
        ChipLogProgress(DeviceLayer, "matter_ble_add_service add service service_handle=%d", service_handle);
    }
}
void matter_set_connection_id(uint8_t conId)
{
    ChipLogProgress(DeviceLayer, "matter_set_connection_id conId=%d", conId);
    BLEMgrImpl().AllocConnectionState(conId);
    current_connect_id = conId;
}

void matter_close_connection(uint8_t conId)
{
    if (current_connect_id != SONATA_ADDR_NONE && conId != current_connect_id)
    {
        ChipLogError(DeviceLayer, "wrong connection id");
    }
    sonata_ble_gap_disconnect(conId, 0);
}

uint16_t matter_ble_complete_event_handler(int opt_id, uint8_t status, uint16_t param, uint32_t dwparam)
{
    uint16_t ret = MATTER_EVENT_DONE_CONTINUE;

    switch (opt_id)
    {
    case SONATA_GAP_CMP_BLE_ON:
        matter_ble_add_service();
        BLEMgrImpl().SetStackInit();
        break;
    case SONATA_GAP_CMP_ADVERTISING_START: // 0x0F06
        if (is_matter_activity(param))
        {
            BLEMgrImpl().SetAdvStartFlag();
        }
        break;
    case SONATA_GAP_CMP_ADVERTISING_STOP:
        if (is_matter_activity(param))
        {
            BLEMgrImpl().SetAdvEndFlag();
        }
        break;
    case SONATA_GATT_CMP_INDICATE:
        if (current_connect_id != SONATA_ADDR_NONE)
        {
            BLEMgrImpl().SendIndicationConfirm(current_connect_id);
            ret = MATTER_EVENT_DONE_FINISHED;
        }
        break;
    case APP_BLE_STACK_EVENT_CONNECTION_REPORT:
        matter_set_connection_id(param);
        break;
    case APP_BLE_STACK_EVENT_DISCONNECTED:
        if (param == current_connect_id)
        {
            BLEMgrImpl().ReleaseConnectionState(current_connect_id);
            current_connect_id = SONATA_ADDR_NONE;
            ret                = MATTER_EVENT_DONE_FINISHED;
        }
        PlatformMgr().ScheduleWork(BLEMgrImpl().DriveBLEState, 0);
        break;
    case APP_BLE_STACK_EVENT_MTU_CHANGED:
        if (param == current_connect_id)
        {
            BLEMgrImpl().SetConnectionMtu(param, dwparam);
            ret = MATTER_EVENT_DONE_FINISHED;
        }
        break;
    default:
        break;
    }
    return ret;
}

void matter_tx_CCCD_write_cb(uint8_t * data, uint16_t size)
{
    BLEMgrImpl().HandleTXCharCCCDWrite(current_connect_id, size, data);
}
void matter_rx_char_write_cb(uint8_t * data, uint16_t size)
{
    BLEMgrImpl().HandleRXCharWrite(current_connect_id, size, data);
}
void matter_tx_CCCD_read_cb(uint8_t * data, uint16_t * size)
{
    BLEMgrImpl().HandleTXCharCCCDRead(current_connect_id, size, data);
}
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
void matter_c3_char_read_cb(uint8_t * data, uint16_t * size)
{
    BLEMgrImpl().HandleC3CharRead(current_connect_id, size, data);
}
#endif
void matter_tx_char_send_indication(uint8_t conId, uint16_t size, uint8_t * data)
{
    ChipLogProgress(DeviceLayer, "matter_tx_char_send_indication conId=%d size=%d data=%p service_handle=%d", conId, size, data,
                    service_handle);
    app_ble_gatt_data_send(conId, service_handle, CSVC_IDX_TX_VAL, size, data);
}

void matter_init_callback(void)
{
    ble_matter_event_callback_reg(matter_ble_complete_event_handler);
}
