/*
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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

/**
 *    @file
 *          Provides an implementation of the BLEManager singleton object
 *          for the MediaTek Genio platforms.
 */

#include "BLEManagerImpl.h"
#include "bt_gap_le.h"
#include "bt_gatts.h"
#include "bt_uuid.h"

#define CHIPOBLEPROFILE_SERVICE_UUID_16 (0xFFF6)
#define CHIPOBLEPROFILE_CHAR_UUID_RX                                                                                               \
    {                                                                                                                              \
        0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18                             \
    }
#define CHIPOBLEPROFILE_CHAR_UUID_TX                                                                                               \
    {                                                                                                                              \
        0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18                             \
    }

const bt_uuid_t CHIPOBLEPROFILE_CHAR_SERVER_TX_UUID128 = { CHIPOBLEPROFILE_CHAR_UUID_TX };
const bt_uuid_t CHIPOBLEPROFILE_CHAR_SERVER_RX_UUID128 = { CHIPOBLEPROFILE_CHAR_UUID_RX };

CHIPoBLECharCallback CHIPoBLEProfile_read_callback  = NULL;
CHIPoBLECharCallback CHIPoBLEProfile_write_callback = NULL;
CHIPoBLECharCallback CHIPoBLEProfile_ccc_callback   = NULL;

uint32_t ble_tx_charc_value_callback(const uint8_t rw, uint16_t handle, void * data, uint16_t size, uint16_t offset)
{
    printf("ble_tx_charc_value_callback: %d %d %p %d %d\n", rw, handle, data, size, offset);
    if (CHIPoBLEProfile_read_callback)
    {
        CHIPoBLEProfile_read_callback(handle, data, size);
    }

    return size;
}

uint32_t ble_rx_charc_callback(const uint8_t rw, uint16_t handle, void * data, uint16_t size, uint16_t offset)
{
    printf("ble_rx_charc_callback: %d %d %p %d %d\n", rw, handle, data, size, offset);
    if (CHIPoBLEProfile_write_callback)
    {
        CHIPoBLEProfile_write_callback(handle, data, size);
    }

    return size;
}

uint32_t ble_tx_ccc_callback(const uint8_t rw, uint16_t handle, void * data, uint16_t size, uint16_t offset)
{
    printf("ble_tx_ccc_callback: %d %d %p %d %d\n", rw, handle, data, size, offset);
    if (CHIPoBLEProfile_ccc_callback)
    {
        CHIPoBLEProfile_ccc_callback(handle, data, size);
    }

    return size;
}

BT_GATTS_NEW_PRIMARY_SERVICE_16(chipOBle_primary_service, CHIPOBLEPROFILE_SERVICE_UUID_16);

BT_GATTS_NEW_CHARC_128(chipOBle_profile_rx_char, BT_GATT_CHARC_PROP_WRITE, 0x0016, CHIPOBLEPROFILE_CHAR_UUID_RX);
BT_GATTS_NEW_CHARC_VALUE_CALLBACK(chipOBle_profile_rx_value, CHIPOBLEPROFILE_CHAR_SERVER_RX_UUID128,
                                  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ble_rx_charc_callback);

BT_GATTS_NEW_CHARC_128(chipOBle_profile_tx_char, BT_GATT_CHARC_PROP_READ | BT_GATT_CHARC_PROP_INDICATE, 0x0018,
                       CHIPOBLEPROFILE_CHAR_UUID_TX);

BT_GATTS_NEW_CHARC_VALUE_CALLBACK(chipOBle_profile_tx_value, CHIPOBLEPROFILE_CHAR_SERVER_TX_UUID128,
                                  BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE, ble_tx_charc_value_callback);

BT_GATTS_NEW_CLIENT_CHARC_CONFIG(chipOBle_profile_tx_config, BT_GATTS_REC_PERM_READABLE | BT_GATTS_REC_PERM_WRITABLE,
                                 ble_tx_ccc_callback);

static const bt_gatts_service_rec_t * chipOBle_service_rec[] = {
    (const bt_gatts_service_rec_t *) &chipOBle_primary_service,  (const bt_gatts_service_rec_t *) &chipOBle_profile_rx_char,
    (const bt_gatts_service_rec_t *) &chipOBle_profile_rx_value, (const bt_gatts_service_rec_t *) &chipOBle_profile_tx_char,
    (const bt_gatts_service_rec_t *) &chipOBle_profile_tx_value, (const bt_gatts_service_rec_t *) &chipOBle_profile_tx_config,
};

const bt_gatts_service_t chipOBle_gatts_service = {
    .starting_handle = 0x0014, .ending_handle = 0x0019, .required_encryption_key_size = 0, .records = chipOBle_service_rec
};
