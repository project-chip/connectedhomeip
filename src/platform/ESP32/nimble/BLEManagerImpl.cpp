/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

/**
 *    @file
 *          Provides an implementation of the BLEManager singleton object
 *          for the ESP32 (NimBLE) platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "sdkconfig.h"

#if CONFIG_BT_NIMBLE_ENABLED

#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

#define MAX_ADV_DATA_LEN 31
#define CHIP_ADV_DATA_TYPE_FLAGS 0x01
#define CHIP_ADV_DATA_FLAGS 0x06
#define CHIP_ADV_DATA_TYPE_SERVICE_DATA 0x16

using namespace ::chip;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

struct ESP32ChipServiceData
{
    uint8_t ServiceUUID[2];
    ChipBLEDeviceIdentificationInfo DeviceIdInfo;
};

const ble_uuid128_t UUID_CHIPoBLEService = {
    BLE_UUID_TYPE_128, { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAF, 0xFE, 0x00, 0x00 }
};
const ble_uuid16_t ShortUUID_CHIPoBLEService = { BLE_UUID_TYPE_16, 0xFEAF };

const ble_uuid128_t UUID128_CHIPoBLEChar_RX = {
    BLE_UUID_TYPE_128, { 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 }
};
const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ble_uuid128_t UUID_CHIPoBLEChar_RX   = {
    { BLE_UUID_TYPE_128 }, { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D, 0x11 }
};

const ble_uuid128_t UUID128_CHIPoBLEChar_TX = {
    BLE_UUID_TYPE_128, { 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 }
};
const ChipBleUUID chipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };
const ble_uuid128_t UUID_CHIPoBLEChar_TX   = {
    { BLE_UUID_TYPE_128 }, { 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 }
};

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;

const struct ble_gatt_svc_def BLEManagerImpl::CHIPoBLEGATTAttrs[] = {
    { .type = BLE_GATT_SVC_TYPE_PRIMARY,
      .uuid = (ble_uuid_t *) (&ShortUUID_CHIPoBLEService),
      .characteristics =
          (struct ble_gatt_chr_def[]){
              {
                  .uuid       = (ble_uuid_t *) (&UUID128_CHIPoBLEChar_RX),
                  .access_cb  = gatt_svr_chr_access,
                  .flags      = BLE_GATT_CHR_F_WRITE,
                  .val_handle = &sInstance.mRXCharAttrHandle,
              },
              {
                  .uuid       = (ble_uuid_t *) (&UUID_CHIPoBLEChar_TX),
                  .access_cb  = gatt_svr_chr_access,
                  .flags      = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
                  .val_handle = &sInstance.mTXCharCCCDAttrHandle,
              },
              {
                  0, /* No more characteristics in this service */
              },
          } },

    {
        0, /* No more services. */
    },
};

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    // Initialize the Chip BleLayer.
    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);

    mRXCharAttrHandle     = 0;
    mTXCharCCCDAttrHandle = 0;
    mFlags                = CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART ? kFlag_AdvertisingEnabled : 0;
    mNumGAPCons           = 0;
    memset(mCons, 0, sizeof(mCons));
    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    memset(mDeviceName, 0, sizeof(mDeviceName));

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (val != mServiceMode)
    {
        mServiceMode = val;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (GetFlag(mFlags, kFlag_AdvertisingEnabled) != val)
    {
        SetFlag(mFlags, kFlag_AdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetFastAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (GetFlag(mFlags, kFlag_FastAdvertisingEnabled) != val)
    {
        SetFlag(mFlags, kFlag_FastAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    if (strlen(mDeviceName) >= bufSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    strcpy(buf, mDeviceName);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
    if (deviceName != NULL && deviceName[0] != 0)
    {
        if (strlen(deviceName) >= kMaxDeviceNameLength)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        strcpy(mDeviceName, deviceName);
        SetFlag(mFlags, kFlag_UseCustomDeviceName);
    }
    else
    {
        mDeviceName[0] = 0;
        ClearFlag(mFlags, kFlag_UseCustomDeviceName);
    }

exit:
    return err;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe:
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
        {
            ChipDeviceEvent connectionEvent;
            connectionEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
            PlatformMgr().PostEvent(&connectionEvent);
        }
        break;

    case DeviceEventType::kCHIPoBLEUnsubscribe:
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
        break;

    case DeviceEventType::kCHIPoBLEWriteReceived:
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Create(event->CHIPoBLEWriteReceived.Data));
        break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm:
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
        break;

    case DeviceEventType::kCHIPoBLEConnectionError:
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
        break;

    case DeviceEventType::kFabricMembershipChange:
    case DeviceEventType::kServiceProvisioningChange:
    case DeviceEventType::kAccountPairingChange:
    case DeviceEventType::kWiFiConnectivityChange:

        // If CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled, and there is a change to the
        // device's provisioning state, then automatically disable CHIPoBLE advertising if the device
        // is now fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED

        // Force the advertising configuration to be refreshed to reflect new provisioning state.
        ChipLogProgress(DeviceLayer, "Updating advertising data");
        ClearFlag(mFlags, kFlag_AdvertisingConfigured);
        SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);

        DriveBLEState();
        break;

    default:
        break;
    }
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    // Signal the ESP BLE layer to close the conneecction.
    err = ble_gap_terminate(conId, BLE_ERR_REM_USER_CONN_TERM);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ble_gap_terminate() failed: %s", ErrorStr(err));
    }

    // Force a refresh of the advertising state.
    SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
    ClearFlag(mFlags, kFlag_AdvertisingConfigured);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return (err == CHIP_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return ble_att_mtu(conId);
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    struct os_mbuf * om;

    VerifyOrExit(IsSubscribed(conId), err = CHIP_ERROR_INVALID_ARGUMENT);

    ESP_LOGD(TAG, "Sending indication for CHIPoBLE TX characteristic (con %u, len %u)", conId, data->DataLength());

    om = ble_hs_mbuf_from_flat(data->Start(), data->DataLength());
    if (om == NULL)
    {
        ChipLogError(DeviceLayer, "ble_hs_mbuf_from_flat failed:");
        err = CHIP_ERROR_NO_MEMORY;
        ExitNow();
    }

    err = ble_gattc_notify_custom(conId, mTXCharCCCDAttrHandle, om);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ble_gattc_notify_custom() failed: %s", ErrorStr(err));
        ExitNow();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManagerImpl::SendIndication() failed: %s", ErrorStr(err));
        return false;
    }
    return true;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBufferHandle pBuf)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     PacketBufferHandle pBuf)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SendReadRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SendReadResponse() not supported");
    return false;
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) {}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the Chip task is running.
    if (!GetFlag(mFlags, kFlag_AsyncInitCompleted))
    {
        SetFlag(mFlags, kFlag_AsyncInitCompleted);

        // If CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled,
        // disable CHIPoBLE advertising if the device is fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    }

    // Initializes the ESP BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !GetFlag(mFlags, kFlag_ESPBLELayerInitialized))
    {
        err = InitESPBleLayer();
        SuccessOrExit(err);

        // Add delay of 500msec while NimBLE host task gets up and running
        {
            vTaskDelay(500 / portTICK_RATE_MS);
        }
    }

    // If the application has enabled CHIPoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled &&
        GetFlag(mFlags, kFlag_AdvertisingEnabled)
#if CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION
        // and no connections are active...
        && (_NumConnections() == 0)
#endif
    )
    {
        // Start/re-start advertising if not already advertising, or if the advertising state of the
        // ESP BLE layer needs to be refreshed.
        if (!GetFlag(mFlags, kFlag_Advertising) || GetFlag(mFlags, kFlag_AdvertisingRefreshNeeded))
        {
            // Configure advertising data if it hasn't been done yet.  This is an asynchronous step which
            // must complete before advertising can be started.  When that happens, this method will
            // be called again, and execution will proceed to the code below.
            if (!GetFlag(mFlags, kFlag_AdvertisingConfigured))
            {
                err = ConfigureAdvertisingData();
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "Configure Adv Data failed: %s", ErrorStr(err));
                    ExitNow();
                }
            }

            // Start advertising.  This is also an asynchronous step.
            ESP_LOGD(TAG, "NimBLE start advertising...");
            err = StartAdvertising();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Start advertising failed: %s", ErrorStr(err));
                ExitNow();
            }

            ClearFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
            // Transition to the Advertising state...
            if (!GetFlag(mFlags, kFlag_Advertising))
            {
                ChipLogProgress(DeviceLayer, "CHIPoBLE advertising started");

                SetFlag(mFlags, kFlag_Advertising);

                // Post a CHIPoBLEAdvertisingChange(Started) event.
                {
                    ChipDeviceEvent advChange;
                    advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
                    advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Started;
                    PlatformMgr().PostEvent(&advChange);
                }
            }
        }
    }

    // Otherwise stop advertising if needed...
    else
    {
        if (GetFlag(mFlags, kFlag_Advertising))
        {
            err = ble_gap_adv_stop();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "ble_gap_adv_stop() failed: %s", ErrorStr(err));
                ExitNow();
            }

            // ClearFlag(mFlags, kFlag_AdvertisingRefreshNeeded);

            // Transition to the not Advertising state...
            if (GetFlag(mFlags, kFlag_Advertising))
            {
                ClearFlag(mFlags, kFlag_Advertising);

                ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");

                // Directly inform the ThreadStackManager that CHIPoBLE advertising has stopped.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
                ThreadStackMgr().OnCHIPoBLEAdvertisingStop();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

                // Post a CHIPoBLEAdvertisingChange(Stopped) event.
                {
                    ChipDeviceEvent advChange;
                    advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
                    advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
                    PlatformMgr().PostEvent(&advChange);
                }
            }

            ExitNow();
        }
    }

    // Stop the CHIPoBLE GATT service if needed.
    if (mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_Enabled && GetFlag(mFlags, kFlag_GATTServiceStarted))
    {
        // TODO: Not supported
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

void BLEManagerImpl::bleprph_on_reset(int reason)
{
    ESP_LOGE(TAG, "Resetting state; reason=%d\n", reason);
}

void BLEManagerImpl::bleprph_on_sync(void)
{
    int rc;

    SetFlag(sInstance.mFlags, kFlag_ESPBLELayerInitialized);
    SetFlag(sInstance.mFlags, kFlag_GATTServiceStarted);
    ESP_LOGI(TAG, "BLE host-controller synced");

    uint8_t own_addr_type = BLE_OWN_ADDR_PUBLIC;

    rc = ble_hs_id_infer_auto(0, &own_addr_type);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Error determining address type; rc=%d\n", rc);
        return;
    }
}

void BLEManagerImpl::bleprph_host_task(void * param)
{
    ESP_LOGD(TAG, "BLE Host Task Started");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();
    nimble_port_freertos_deinit();
}

CHIP_ERROR BLEManagerImpl::InitESPBleLayer(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(!GetFlag(mFlags, kFlag_ESPBLELayerInitialized), /* */);

    for (int i = 0; i < kMaxConnections; i++)
    {
        mSubscribedConIds[i] = BLE_CONNECTION_UNINITIALIZED;
    }

    err = esp_nimble_hci_and_controller_init();
    SuccessOrExit(err);

    nimble_port_init();

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb        = bleprph_on_reset;
    ble_hs_cfg.sync_cb         = bleprph_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;

    // Register the CHIPoBLE GATT attributes with the ESP BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled)
    {
        ble_svc_gap_init();
        ble_svc_gatt_init();

        err = ble_gatts_count_cfg(CHIPoBLEGATTAttrs);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ble_gatts_count_cfg failed: %s", ErrorStr(err));
            ExitNow();
        }

        err = ble_gatts_add_svcs(CHIPoBLEGATTAttrs);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ble_gatts_add_svcs failed: %s", ErrorStr(err));
            ExitNow();
        }
    }

    nimble_port_freertos_init(bleprph_host_task);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    CHIP_ERROR err;
    uint8_t advData[MAX_ADV_DATA_LEN];
    uint8_t index = 0;

    // If a custom device name has not been specified, generate a CHIP-standard name based on the
    // bottom digits of the Chip device id.
    uint16_t discriminator;
    SuccessOrExit(err = ConfigurationMgr().GetSetupDiscriminator(discriminator));

    if (!GetFlag(mFlags, kFlag_UseCustomDeviceName))
    {
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
        mDeviceName[kMaxDeviceNameLength] = 0;
    }

    // Configure the BLE device name.
    err = ble_svc_gap_device_name_set(mDeviceName);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ble_svc_gap_device_name_set() failed: %s", ErrorStr(err));
        ExitNow();
    }

    memset(advData, 0, sizeof(advData));
    advData[index++] = 0x02;                                                                // length
    advData[index++] = CHIP_ADV_DATA_TYPE_FLAGS;                                            // AD type : flags
    advData[index++] = CHIP_ADV_DATA_FLAGS;                                                 // AD value
    advData[index++] = 0x0A;                                                                // length
    advData[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;                                     // AD type: (Service Data - 16-bit UUID)
    advData[index++] = static_cast<uint8_t>(ShortUUID_CHIPoBLEService.value & 0xFF);        // AD value
    advData[index++] = static_cast<uint8_t>((ShortUUID_CHIPoBLEService.value >> 8) & 0xFF); // AD value

    chip::Ble::ChipBLEDeviceIdentificationInfo deviceIdInfo;
    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetBLEDeviceIdentificationInfo(): %s", ErrorStr(err));
        ExitNow();
    }

    VerifyOrExit(index + sizeof(deviceIdInfo) <= sizeof(advData), err = BLE_ERROR_OUTBOUND_MESSAGE_TOO_BIG);
    memcpy(&advData[index], &deviceIdInfo, sizeof(deviceIdInfo));
    index = static_cast<uint8_t>(index + sizeof(deviceIdInfo));

    // Construct the Chip BLE Service Data to be sent in the scan response packet.
    err = ble_gap_adv_set_data(advData, sizeof(advData));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ble_gap_adv_set_data failed: %s %d", ErrorStr(err), discriminator);
        ExitNow();
    }

exit:
    return err;
}

void BLEManagerImpl::HandleRXCharWrite(struct ble_gatt_char_context * param)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    uint16_t data_len = 0;

    ESP_LOGI(TAG, "Write request received for CHIPoBLE RX characteristic con %u %u", param->conn_handle, param->attr_handle);

    // Copy the data to a PacketBuffer.
    PacketBufferHandle buf = PacketBuffer::New(0);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    data_len = OS_MBUF_PKTLEN(param->ctxt->om);
    VerifyOrExit(buf->AvailableDataLength() >= data_len, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    ble_hs_mbuf_to_flat(param->ctxt->om, buf->Start(), data_len, NULL);
    buf->SetDataLength(data_len);

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = param->conn_handle;
        event.CHIPoBLEWriteReceived.Data  = buf.Release_ForNow();
        PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleTXCharRead(struct ble_gatt_char_context * param)
{
    /* Not supported */
    ChipLogError(DeviceLayer, "BLEManagerImpl::HandleTXCharRead() not supported");
}

void BLEManagerImpl::HandleTXCharCCCDRead(void * param)
{
    /* Not Supported */
    ChipLogError(DeviceLayer, "BLEManagerImpl::HandleTXCharCCCDRead() not supported");
}

void BLEManagerImpl::HandleTXCharCCCDWrite(struct ble_gap_event * gapEvent)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool indicationsEnabled;
    bool notificationsEnabled;

    ChipLogProgress(DeviceLayer,
                    "Write request/command received for CHIPoBLE TX CCCD characteristic (con %" PRIu16
                    " ) indicate = %d notify = %d",
                    gapEvent->subscribe.conn_handle, gapEvent->subscribe.cur_indicate, gapEvent->subscribe.cur_notify);

    // Determine if the client is enabling or disabling indications/notification.
    indicationsEnabled   = gapEvent->subscribe.cur_indicate;
    notificationsEnabled = gapEvent->subscribe.cur_notify;

    // If the client has requested to enabled indications/notifications
    if (indicationsEnabled || notificationsEnabled)
    {
        // If indications are not already enabled for the connection...
        if (!IsSubscribed(gapEvent->subscribe.conn_handle))
        {
            // Record that indications have been enabled for this connection.  If this fails because
            err = SetSubscribed(gapEvent->subscribe.conn_handle);
            VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, err = CHIP_NO_ERROR);
            SuccessOrExit(err);
        }
    }

    else
    {
        // If indications had previously been enabled for this connection, record that they are no longer
        // enabled.
        UnsetSubscribed(gapEvent->subscribe.conn_handle);
    }

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling indications.
    {
        ChipDeviceEvent event;
        event.Type = (indicationsEnabled || notificationsEnabled) ? DeviceEventType::kCHIPoBLESubscribe
                                                                  : DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = gapEvent->subscribe.conn_handle;
        PlatformMgr().PostEvent(&event);
    }

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received",
                    (indicationsEnabled || notificationsEnabled) ? "subscribe" : "unsubscribe");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
        // TODO: fail connection???
    }

    return;
}

CHIP_ERROR BLEManagerImpl::HandleTXComplete(struct ble_gap_event * gapEvent)
{
    ChipLogProgress(DeviceLayer, "Confirm received for CHIPoBLE TX characteristic indication (con %" PRIu16 ") status= %d ",
                    gapEvent->notify_tx.conn_handle, gapEvent->notify_tx.status);

    // Signal the BLE Layer that the outstanding indication is complete.
    if (gapEvent->notify_tx.status == 0 || gapEvent->notify_tx.status == BLE_HS_EDONE)
    {
        // Post an event to the Chip queue to process the indicate confirmation.
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
        event.CHIPoBLEIndicateConfirm.ConId = gapEvent->notify_tx.conn_handle;
        PlatformMgr().PostEvent(&event);
    }

    else
    {
        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId  = gapEvent->notify_tx.conn_handle;
        event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
        PlatformMgr().PostEvent(&event);
    }

    return CHIP_NO_ERROR;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    uint16_t numCons = 0;
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mSubscribedConIds[i] != BLE_CONNECTION_UNINITIALIZED)
        {
            numCons++;
        }
    }

    return numCons;
}

CHIP_ERROR BLEManagerImpl::HandleGAPConnect(struct ble_gap_event * gapEvent)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(DeviceLayer, "BLE GAP connection established (con %" PRIu16 ")", gapEvent->connect.conn_handle);

    // Track the number of active GAP connections.
    mNumGAPCons++;
    err = SetSubscribed(gapEvent->connect.conn_handle);
    VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, err = CHIP_NO_ERROR);
    SuccessOrExit(err);

    SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
    ClearFlag(mFlags, kFlag_AdvertisingConfigured);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::HandleGAPDisconnect(struct ble_gap_event * gapEvent)
{
    ChipLogProgress(DeviceLayer, "BLE GAP connection terminated (con %" PRIu16, " reason 0x%02 " PRIx8 ")",
                    gapEvent->disconnect.conn.conn_handle, gapEvent->disconnect.reason);

    // Update the number of GAP connections.
    if (mNumGAPCons > 0)
    {
        mNumGAPCons--;
    }

    if (UnsetSubscribed(gapEvent->disconnect.conn.conn_handle))
    {
        CHIP_ERROR disconReason;
        switch (gapEvent->disconnect.reason)
        {
        case BLE_ERR_REM_USER_CONN_TERM:
            disconReason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;
        case BLE_ERR_CONN_TERM_LOCAL:
            disconReason = BLE_ERROR_APP_CLOSED_CONNECTION;
            break;
        default:
            disconReason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
            break;
        }
        HandleConnectionError(gapEvent->disconnect.conn.conn_handle, disconReason);
    }

    // Force a reconfiguration of advertising in case we switched to non-connectable mode when
    // the BLE connection was established.
    SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
    ClearFlag(mFlags, kFlag_AdvertisingConfigured);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::SetSubscribed(uint16_t conId)
{
    uint16_t freeIndex = kMaxConnections;

    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mSubscribedConIds[i] == conId)
        {
            return CHIP_NO_ERROR;
        }
        else if (mSubscribedConIds[i] == BLE_CONNECTION_UNINITIALIZED && i < freeIndex)
        {
            freeIndex = i;
        }
    }

    if (freeIndex < kMaxConnections)
    {
        mSubscribedConIds[freeIndex] = conId;
        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_NO_MEMORY;
    }
}

bool BLEManagerImpl::UnsetSubscribed(uint16_t conId)
{
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mSubscribedConIds[i] == conId)
        {
            mSubscribedConIds[i] = BLE_CONNECTION_UNINITIALIZED;
            return true;
        }
    }
    return false;
}

bool BLEManagerImpl::IsSubscribed(uint16_t conId)
{
    if (conId != BLE_CONNECTION_UNINITIALIZED)
    {
        for (uint16_t i = 0; i < kMaxConnections; i++)
        {
            if (mSubscribedConIds[i] == conId)
            {
                return true;
            }
        }
    }
    return false;
}

int BLEManagerImpl::ble_svr_gap_event(struct ble_gap_event * event, void * arg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        /* A new connection was established or a connection attempt failed */
        err = sInstance.HandleGAPConnect(event);
        SuccessOrExit(err);
        break;

    case BLE_GAP_EVENT_DISCONNECT:
        err = sInstance.HandleGAPDisconnect(event);
        SuccessOrExit(err);
        break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGD(TAG, "BLE_GAP_EVENT_ADV_COMPLETE event");
        break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        if (event->subscribe.attr_handle == sInstance.mTXCharCCCDAttrHandle)
        {
            sInstance.HandleTXCharCCCDWrite(event);
        }

        break;

    case BLE_GAP_EVENT_NOTIFY_TX:
        err = sInstance.HandleTXComplete(event);
        SuccessOrExit(err);
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGD(TAG, "BLE_GAP_EVENT_MTU = %d channel id = %d", event->mtu.value, event->mtu.channel_id);
        break;

    default:
        break;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        sInstance.mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }

    // Schedule DriveBLEState() to run.
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return err;
}

int BLEManagerImpl::gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt * ctxt, void * arg)
{
    struct ble_gatt_char_context param;
    CHIP_ERROR err = CHIP_NO_ERROR;

    memset(&param, 0, sizeof(struct ble_gatt_char_context));

    switch (ctxt->op)
    {
    case BLE_GATT_ACCESS_OP_READ_CHR:

        param.conn_handle = conn_handle;
        param.attr_handle = attr_handle;
        param.ctxt        = ctxt;
        param.arg         = arg;
        sInstance.HandleTXCharRead(&param);
        break;

    case BLE_GATT_ACCESS_OP_READ_DSC:

        param.conn_handle = conn_handle;
        param.attr_handle = attr_handle;
        param.ctxt        = ctxt;
        param.arg         = arg;
        sInstance.HandleTXCharCCCDRead(&param);
        break;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        param.conn_handle = conn_handle;
        param.attr_handle = attr_handle;
        param.ctxt        = ctxt;
        param.arg         = arg;
        sInstance.HandleRXCharWrite(&param);
        break;

    default:
        err = BLE_ATT_ERR_UNLIKELY;
        break;
    }

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err;
    ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    uint8_t own_addr_type = BLE_OWN_ADDR_PUBLIC;

    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    // Inform the ThreadStackManager that CHIPoBLE advertising is about to start.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    {
        ThreadStackMgr().OnCHIPoBLEAdvertisingStart();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    ClearFlag(mFlags, kFlag_AdvertisingRefreshNeeded);

    // Advertise connectable if we haven't reached the maximum number of connections.
    size_t numCons       = _NumConnections();
    bool connectable     = (numCons < kMaxConnections);
    adv_params.conn_mode = connectable ? BLE_GAP_CONN_MODE_UND : BLE_GAP_CONN_MODE_NON;

    // Advertise in fast mode if not fully provisioned and there are no CHIPoBLE connections, or
    // if the application has expressly requested fast advertising.
    adv_params.itvl_min = adv_params.itvl_max = (numCons == 0 && !ConfigurationMgr().IsFullyProvisioned())
        ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL
        : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL;

    ChipLogProgress(DeviceLayer, "Configuring CHIPoBLE advertising (interval %" PRIu32 " ms, %sconnectable, device name %s)",
                    (((uint32_t) adv_params.itvl_min) * 10) / 16, (connectable) ? "" : "non-", mDeviceName);

    {
        err = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_svr_gap_event, NULL);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ble_gap_adv_start() failed: %s", ErrorStr(err));
            ExitNow();
        }
    }

exit:
    if (err == BLE_HS_EALREADY && connectable)
    {
        ChipLogProgress(DeviceLayer,
                        "Connectable advertising failed because device was already advertising , stop active advertisement");
        err = ble_gap_adv_stop();
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ble_gap_adv_stop() failed: %s", ErrorStr(err));
        }
        else
        {
            err = ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_svr_gap_event, NULL);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Retried adv start; ble_gap_adv_start() failed: %s", ErrorStr(err));
            }
        }
    }
    return err;
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CONFIG_BT_NIMBLE_ENABLED

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
