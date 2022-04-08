/***************************************************************************/ /**
                                                                               * File Name: cycfg_gatt_db.c
                                                                               * Version: 2.1
                                                                               *
                                                                               * Description:
                                                                               * BLE device's GATT database and device
                                                                               *configuration.
                                                                               *
                                                                               ********************************************************************************
                                                                               * Copyright 2020 Cypress Semiconductor Corporation
                                                                               * SPDX-License-Identifier: Apache-2.0
                                                                               *
                                                                               * Licensed under the Apache License, Version 2.0 (the
                                                                               *"License"); you may not use this file except in
                                                                               *compliance with the License. You may obtain a copy
                                                                               *of the License at
                                                                               *
                                                                               *     http://www.apache.org/licenses/LICENSE-2.0
                                                                               *
                                                                               * Unless required by applicable law or agreed to in
                                                                               *writing, software distributed under the License is
                                                                               *distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
                                                                               *OR CONDITIONS OF ANY KIND, either express or
                                                                               *implied. See the License for the specific language
                                                                               *governing permissions and limitations under the
                                                                               *License.
                                                                               *******************************************************************************/

#include "cycfg_gatt_db.h"
#include "wiced_bt_gatt.h"
#include "wiced_bt_uuid.h"
#ifdef BLE_OTA_FW_UPGRADE
#include <wiced_bt_ota_firmware_upgrade.h>
#endif

/*************************************************************************************
 * GATT server definitions
 *************************************************************************************/

const uint8_t gatt_database[] = {
    /* Primary Service: Generic Access */
    PRIMARY_SERVICE_UUID16(HDLS_GAP, __UUID_SERVICE_GENERIC_ACCESS),

    /* Characteristic: Device Name */
    CHARACTERISTIC_UUID16(HDLC_GAP_DEVICE_NAME, HDLC_GAP_DEVICE_NAME_VALUE, __UUID_CHARACTERISTIC_DEVICE_NAME,
                          LEGATTDB_CHAR_PROP_READ, LEGATTDB_PERM_READABLE),

    /* Characteristic: Appearance */
    CHARACTERISTIC_UUID16(HDLC_GAP_APPEARANCE, HDLC_GAP_APPEARANCE_VALUE, __UUID_CHARACTERISTIC_APPEARANCE, LEGATTDB_CHAR_PROP_READ,
                          LEGATTDB_PERM_READABLE),

    /* Primary Service: Generic Attribute */
    PRIMARY_SERVICE_UUID16(HDLS_GATT, __UUID_SERVICE_GENERIC_ATTRIBUTE),

    /* Primary Service: Custom Service */
    PRIMARY_SERVICE_UUID16(HDLS_CHIP_SERVICE, __UUID16_CHIPoBLEService),

    /* Characteristic: C1 */
    CHARACTERISTIC_UUID128_WRITABLE(HDLC_CHIP_SERVICE_CHAR_C1, HDLC_CHIP_SERVICE_CHAR_C1_VALUE, __UUID128_CHIPoBLEChar_C1,
                                    LEGATTDB_CHAR_PROP_WRITE,
                                    LEGATTDB_PERM_VARIABLE_LENGTH | LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),
    /* Characteristic: C2 */
    CHARACTERISTIC_UUID128_WRITABLE(HDLC_CHIP_SERVICE_CHAR_C2, HDLC_CHIP_SERVICE_CHAR_C2_VALUE, __UUID128_CHIPoBLEChar_C2,
                                    LEGATTDB_CHAR_PROP_READ | LEGATTDB_CHAR_PROP_NOTIFY,
                                    LEGATTDB_PERM_RELIABLE_WRITE | LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITABLE),

    /* Descriptor: Client Characteristic Configuration */
    CHAR_DESCRIPTOR_UUID16_WRITABLE(HDLD_CHIP_SERVICE_RX_CLIENT_CHAR_CONFIG, __UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                    LEGATTDB_PERM_AUTH_READABLE | LEGATTDB_PERM_WRITE_REQ),

#ifdef BLE_OTA_FW_UPGRADE
    /* WICED Upgrade Service. */
    PRIMARY_SERVICE_UUID128(HANDLE_OTA_FW_UPGRADE_SERVICE, UUID_OTA_FW_UPGRADE_SERVICE),

    /* characteristic Control Point */
    CHARACTERISTIC_UUID128_WRITABLE(HANDLE_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT, HANDLE_OTA_FW_UPGRADE_CONTROL_POINT,
                                    UUID_OTA_FW_UPGRADE_CHARACTERISTIC_CONTROL_POINT,
                                    LEGATTDB_CHAR_PROP_WRITE | LEGATTDB_CHAR_PROP_NOTIFY | LEGATTDB_CHAR_PROP_INDICATE,
                                    LEGATTDB_PERM_VARIABLE_LENGTH | LEGATTDB_PERM_WRITE_REQ),

    /* client characteristic configuration descriptor */
    CHAR_DESCRIPTOR_UUID16_WRITABLE(HANDLE_OTA_FW_UPGRADE_CLIENT_CONFIGURATION_DESCRIPTOR,
                                    UUID_DESCRIPTOR_CLIENT_CHARACTERISTIC_CONFIGURATION,
                                    LEGATTDB_PERM_READABLE | LEGATTDB_PERM_WRITE_REQ),

    /* characteristic Data. */
    CHARACTERISTIC_UUID128_WRITABLE(HANDLE_OTA_FW_UPGRADE_CHARACTERISTIC_DATA, HANDLE_OTA_FW_UPGRADE_DATA,
                                    UUID_OTA_FW_UPGRADE_CHARACTERISTIC_DATA, LEGATTDB_CHAR_PROP_WRITE,
                                    LEGATTDB_PERM_VARIABLE_LENGTH | LEGATTDB_PERM_WRITE_REQ | LEGATTDB_PERM_RELIABLE_WRITE),
#endif /* BLE_OTA_FW_UPGRADE */
};

/* Length of the GATT database */
const uint16_t gatt_database_len = sizeof(gatt_database);

/*************************************************************************************
 * GATT Initial Value Arrays
 ************************************************************************************/

uint8_t app_gap_device_name[kMaxDeviceNameLength] = { 'I', 'F', 'X', ' ', 'C', 'H', 'I', 'P' };
uint8_t app_gap_appearance[]                      = {
    0x00,
    0x00,
};
uint8_t app_chip_service_char_tx_client_char_config[] = {
    0x00,
    0x00,
};

/************************************************************************************
 * GATT Lookup Table
 ************************************************************************************/

gatt_db_lookup_table_t app_gatt_db_ext_attr_tbl[] = {
    /* { attribute handle,                                       maxlen, curlen, attribute data } */
    { HDLC_GAP_DEVICE_NAME_VALUE, sizeof(app_gap_device_name), sizeof(app_gap_device_name), app_gap_device_name },
    { HDLC_GAP_APPEARANCE_VALUE, sizeof(app_gap_appearance), sizeof(app_gap_appearance), app_gap_appearance },
    { HDLD_CHIP_SERVICE_RX_CLIENT_CHAR_CONFIG, sizeof(app_chip_service_char_tx_client_char_config),
      sizeof(app_chip_service_char_tx_client_char_config), app_chip_service_char_tx_client_char_config },
};

/* Number of Lookup Table entries */
const uint16_t app_gatt_db_ext_attr_tbl_size = (sizeof(app_gatt_db_ext_attr_tbl) / sizeof(gatt_db_lookup_table_t));

/* Number of GATT initial value arrays entries */
const uint16_t app_gap_device_name_len                         = (sizeof(app_gap_device_name));
const uint16_t app_gap_appearance_len                          = (sizeof(app_gap_appearance));
const uint16_t app_chip_service_char_tx_client_char_config_len = (sizeof(app_chip_service_char_tx_client_char_config));
