/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          for the Telink platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/BleUUID.h>
#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

/* Telink headers */ 
#include "types.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

using namespace ::chip;
using namespace ::chip::Ble;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

// Advertising data content definitions
#define CHIP_MAX_ADV_DATA_LEN 31

#define CHIP_ADE_DATA_LEN_FLAGS 0x02
#define CHIP_ADV_DATA_TYPE_FLAGS 0x01
#define CHIP_ADV_DATA_FLAGS 0x06
#define CHIP_ADV_DATA_TYPE_UUID 0x03
#define CHIP_ADV_DATA_TYPE_NAME 0x09
#define CHIP_ADV_DATA_TYPE_SERVICE_DATA 0x16
#define CHIP_ADV_SHORT_UUID_LEN 2
#define CHIP_ADV_SERVICE_DATA_LEN (sizeof(ChipBLEDeviceIdentificationInfo) + CHIP_ADV_SHORT_UUID_LEN + 1)


#define CHIP_BLE_THREAD_STACK_SIZE 4096
#define CHIP_BLE_THREAD_PRIORITY 2

#define WHITE_LED GPIO_PB6
#define STIMER_IRQ_NUM 1

// Full service UUID - CHIP_BLE_SVC_ID - taken from BleUUID.h header
const uint8_t chipUUID_CHIPoBLE_Service[CHIP_ADV_SHORT_UUID_LEN] = { 0xFF, 0xF6 };

const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };

const ChipBleUUID chipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;

/**
 * @brief		BLE SDK RF interrupt handler.
 * @param[in]	none
 * @return      none
 */
void rf_irq_handler(const void *paramiter)
{
    gpio_set_high_level(WHITE_LED);

    irq_blt_sdk_handler();

    gpio_set_low_level(WHITE_LED);
}

/**
 * @brief		BLE SDK System timer interrupt handler.
 * @param[in]	none
 * @return      none
 */
void stimer_irq_handler(const void *paramiter)
{
    gpio_set_high_level(WHITE_LED);

    irq_blt_sdk_handler();

    gpio_set_low_level(WHITE_LED);
}

void chip_ble_thread_entry_point(void *, void *, void *)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::BLEThreadEntry");

    while(true) {
        blt_sdk_main_loop();

        k_msleep(10);
    }
}

/* Thread for runing BLE main loop */
K_THREAD_DEFINE(chip_ble_thread, 
                CHIP_BLE_THREAD_STACK_SIZE, 
                chip_ble_thread_entry_point, 
                NULL, NULL, NULL,
                CHIP_BLE_THREAD_PRIORITY, 0, 0);

CHIP_ERROR BLEManagerImpl::_Init()
{
    uint8_t mac_public[6] = {0};
    uint8_t mac_random_static[6] = {0};

    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_Init");

    /* Init Radio driver */
    ble_radio_init();

    /* Generate MAC address if it does not exist or read it from flash if it is exist already */
    blc_initMacAddress(CFG_ADR_MAC_1M_FLASH, mac_public, mac_random_static);

    ChipLogDetail(DeviceLayer, "MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac_public[0],
                                                                     mac_public[1],
                                                                     mac_public[2],
                                                                     mac_public[3],
                                                                     mac_public[4],
                                                                     mac_public[5]);

    /* Init interrupts and DMA for BLE module ??? */
    blc_ll_initBasicMCU();

    /* Setup MAC Address */
    blc_ll_initStandby_module(mac_public);

    /* Init advertisement */
    blc_ll_initAdvertising_module();

    /* Init slave role */
    blc_ll_initSlaveRole_module();

    /* Resetup interrupts to handle BLE stack */
    int ret = irq_connect_dynamic(STIMER_IRQ_NUM, 2, stimer_irq_handler, NULL, 0);
    ChipLogDetail(DeviceLayer, "Stimer assigned vector %d", ret);

    irq_enable(STIMER_IRQ_NUM);

    /* Enable White LED for debug purposes */
    gpio_function_en(WHITE_LED);
    gpio_output_en(WHITE_LED);

    /* Switch off the LED on the beginning */
    gpio_set_low_level(WHITE_LED);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetCHIPoBLEServiceMode");

    return CHIP_NO_ERROR;
}


CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    ble_sts_t status = BLE_SUCCESS;
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint32_t index = 0;

    ChipBLEDeviceIdentificationInfo deviceIdInfo;

    ChipLogProgress(DeviceLayer, "BLEManagerImpl::ConfigureAdvertisingData");

    u8 adv[CHIP_MAX_ADV_DATA_LEN] = {0};
    // {
    //     0x05, 0x09, 'e', 'H', 'I', 'D',
    //     0x02, 0x01, 0x05, 						// BLE limited discoverable mode and BR/EDR not supported
    //     0x03, 0x19, 0x80, 0x01, 				// 384, Generic Remote Control, Generic category
    //     0x05, 0x02, 0x12, 0x18, 0x0F, 0x18,		// incomplete list of service class UUIDs (0x1812, 0x180F)
    // };

    // u8 srsp[] = 
    // {
    //     0x08, 0x09, 'e', 'S', 'a', 'm', 'p', 'l', 'e', // Device name
    // };

    /* Get BLE device identification info */
    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    SuccessOrExit(err);

    /* Fulfill BLE advertisement data */
    adv[index++] = CHIP_ADE_DATA_LEN_FLAGS;                                                 // length
    adv[index++] = CHIP_ADV_DATA_TYPE_FLAGS;                                                // AD type : flags
    adv[index++] = CHIP_ADV_DATA_FLAGS;                                                     // AD value
    adv[index++] = CHIP_ADV_SERVICE_DATA_LEN;                                               // length
    adv[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;                                         // AD type : Service Data
    adv[index++] = chipUUID_CHIPoBLE_Service[0];                                            // AD value
    adv[index++] = chipUUID_CHIPoBLE_Service[1];                                            // AD value
    memcpy(&adv[index], (void *) &deviceIdInfo, sizeof(deviceIdInfo));                      // AD value
    index += sizeof(deviceIdInfo);

    /* Set advetisment data */    
    status = bls_ll_setAdvData(adv, sizeof(adv));
    if(status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to set BLE advertisement data. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Set scan responce data */
    // status = bls_ll_setScanRspData(srsp, sizeof(srsp));
    // if(status != BLE_SUCCESS)
    // {
    //     ChipLogError(DeviceLayer, "Fail to set BLE scan responce data. Error %d", status);

    //     return CHIP_ERROR_INCORRECT_STATE;
    // }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    ble_sts_t status = BLE_SUCCESS;
    
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetAdvertisingEnabled");

    if(val == true)
    {
        ChipLogProgress(DeviceLayer, "Enable BLE");

        /* At first run always select fast advertising, on the next attemp slow down interval. */
        u16 intervalMin = mFlags.Has(Flags::kFastAdvertisingEnabled) ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN
                                                                     : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        u16 intervalMax = mFlags.Has(Flags::kFastAdvertisingEnabled) ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX
                                                                     : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;

        /* Setup advertisement paramiters */
        status = bls_ll_setAdvParam(intervalMin, 
                                    intervalMax,
                                    ADV_TYPE_CONNECTABLE_UNDIRECTED, 
                                    OWN_ADDRESS_PUBLIC,
                                    0,  NULL,
                                    BLT_ENABLE_ADV_ALL,
                                    ADV_FP_NONE);
        if(status != BLE_SUCCESS)
        {
            ChipLogError(DeviceLayer, "Fail to set BLE advertisement paramiters. Error %d", status);

            return CHIP_ERROR_INCORRECT_STATE;
        }

        /* Enable advertisement */
        status = bls_ll_setAdvEnable(BLC_ADV_ENABLE);
        if(status != BLE_SUCCESS)
        {
            ChipLogError(DeviceLayer, "Fail to start BLE advertisement. Error %d", status);

            return CHIP_ERROR_INCORRECT_STATE;
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Disable BLE");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::StartAdvertising");

    return _SetAdvertisingEnabled(true);
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::StopAdvertising");

    return _SetAdvertisingEnabled(false);
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetAdvertisingMode");

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_GetDeviceName");


    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * devName)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetDeviceName");

    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (deviceName != NULL && deviceName[0] != 0)
    {
        ChipLogError(DeviceLayer, "Invalid name");

        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if(strlen(devName) >= kMaxDeviceNameLength)
    {
        ChipLogError(DeviceLayer, "BLE device name is to long");

        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    strcpy(mDeviceName, deviceName);
    mFlags.Set(Flags::kDeviceNameSet);
    
    ChipLogProgress(DeviceLayer, "Setting device name to : \"%s\"", deviceName);

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_OnPlatformEvent");
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic");

    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic");

    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::CloseConnection");

    return false;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::GetMTU");

    return 0;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendIndication()");

    return false;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBufferHandle pBuf)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     PacketBufferHandle pBuf)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendReadRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendReadResponse() not supported");
    return false;
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::NotifyChipConnectionClosed");

}

CHIP_ERROR BLEManagerImpl::MapBLEError(int bleErr) const
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::MapBLEError");

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::DriveBLEState(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::DriveBLEState");

}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    return 0;
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::DriveBLEState");

}

CHIP_ERROR BLEManagerImpl::SetSubscribed(uint16_t conId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SetSubscribed");

    return CHIP_NO_ERROR;
}

bool BLEManagerImpl::UnsetSubscribed(uint16_t conId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::UnsetSubscribed");

    return false;
}

bool BLEManagerImpl::IsSubscribed(uint16_t conId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::IsSubscribed");

    return false;
}

void BLEManagerImpl::CancelBleAdvTimeoutTimer(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::CancelBleAdvTimeoutTimer");

}

void BLEManagerImpl::StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::StartBleAdvTimeoutTimer");

}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
