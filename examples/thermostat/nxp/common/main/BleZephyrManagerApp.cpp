/*
 *
 *    Copyright (c) 2024 Project CHIP Authors
 *    Copyright 2024 NXP
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

#include "BLEApplicationManager.h"

#include <platform/internal/CHIPDeviceLayerInternal.h>

#include "BLEManagerImpl.h"
#include <ble/Ble.h>
#include <platform/ConfigurationManager.h>
#include <zephyr/bluetooth/gatt.h>


#define ADV_LEN 2

using namespace ::chip::DeviceLayer;
using namespace ::chip::DeviceLayer::Internal;
using namespace ::chip::NXP::App;

BLEApplicationManager BLEApplicationManager::sInstance;

struct ServiceData
{
    uint8_t uuid[2];
    chip::Ble::ChipBLEDeviceIdentificationInfo deviceIdInfo;
} __attribute__((packed));

ServiceData serviceData;
std::array<bt_data, 3> advertisingData;
std::array<bt_data, 1> scanResponseData;

constexpr uint8_t kAdvertisingFlags = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;

uint8_t manuf_data[ADV_LEN] = {
    0x25,
    0x00,
};

bt_uuid_16 UUID16_CHIPoBLEService = BT_UUID_INIT_16(0xFFF6);

/**
 * @brief Init BLE application manager
 *
 * In this example, the application manager is used to customize BLE advertising
 * parameters. This example is provided for platforms with Zephyr BLE manager support.
 *
 * @note This example set custom Zephyr BLE manager Matter advertising parameters (
 *  CONFIG_CHIP_CUSTOM_BLE_ADV_DATA for kconfig or ble_adv_data_custom for GN is set).
 *  To keep the advertising support for commissining, it is needed to register the whole
 *  adv data (adv flags + Matter adv data + custom adv data) and register scan
 *  response data as default adv and response data will be skipped.
 *  kAdvertisingFlags and manuf_data are given for examples, size of advertisingData and
 *  scanResponseData have to be set according to custom requirements.
 *
 * For custom Gatt services, APIs bt_gatt_service_register and bt_gatt_service_unregister
 * could be called at application layer. It will not override Matter Gatt services but
 * add new one.
 *
 * @note At the end of the commissioning advertising will be stopped.
 *
 * To start new advertising process, APIs :
 * chip::DeviceLayer::BLEAdvertisingArbiter::InsertRequest
 * chip::DeviceLayer::BLEAdvertisingArbiter::CancelRequest
 * could be called. If InsertRequest API is called several time, only the request with the
 * higher priority will be advertise.
 *
 */
void BLEApplicationManager::Init(void)
{
    /* Register Matter adv data + custom adv data */
    static_assert(sizeof(serviceData) == 10, "Unexpected size of BLE advertising data!");
    const char * name      = bt_get_name();
    const uint8_t nameSize = static_cast<uint8_t>(strlen(name));
    Encoding::LittleEndian::Put16(serviceData.uuid, UUID16_CHIPoBLEService.val);
    chip::DeviceLayer::ConfigurationMgr().GetBLEDeviceIdentificationInfo(serviceData.deviceIdInfo);

    advertisingData[0] = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
    /* Matter adv data for commissining */
    advertisingData[1] = BT_DATA(BT_DATA_SVC_DATA16, &serviceData, sizeof(serviceData));
    /* Example of custom BLE adv data */
    advertisingData[2]  = BT_DATA(BT_DATA_MANUFACTURER_DATA, manuf_data, ADV_LEN);
    scanResponseData[0] = BT_DATA(BT_DATA_NAME_COMPLETE, name, nameSize);
    chip::DeviceLayer::Internal::BLEMgrImpl().SetCustomAdvertising(Span<bt_data>(advertisingData));
    chip::DeviceLayer::Internal::BLEMgrImpl().SetCustomScanResponse(Span<bt_data>(scanResponseData));
}
