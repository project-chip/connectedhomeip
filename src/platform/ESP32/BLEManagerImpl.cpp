/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          for the ESP32 platform.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/internal/BLEManager.h>
#include <BleLayer/WeaveBleServiceData.h>
#include <new>

#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"

using namespace ::nl;
using namespace ::nl::Ble;

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

namespace {


struct ESP32WeaveServiceData
{
    uint8_t ServiceUUID[2];
    WeaveBLEDeviceIdentificationInfo DeviceIdInfo;
};

const uint16_t WoBLEAppId = 0x235A;

const uint8_t UUID_PrimaryService[] = { 0x00, 0x28 };
const uint8_t UUID_CharDecl[] = { 0x03, 0x28 };
const uint8_t UUID_ClientCharConfigDesc[] = { 0x02, 0x29 };
const uint8_t UUID_WoBLEService[] = { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAF, 0xFE, 0x00, 0x00 };
const uint8_t ShortUUID_WoBLEService[] = { 0xAF, 0xFE };
const uint8_t UUID_WoBLEChar_RX[] = { 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 };
const uint8_t UUID_WoBLEChar_TX[] = { 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 };
const WeaveBleUUID WeaveUUID_WoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D, 0x11 } };
const WeaveBleUUID WeaveUUID_WoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D, 0x12 } };

const uint8_t CharProps_ReadNotify =  ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
const uint8_t CharProps_Write =  ESP_GATT_CHAR_PROP_BIT_WRITE;

// Offsets into WoBLEGATTAttrs for specific attributes.
enum
{
    kAttrIndex_ServiceDeclaration = 0,
    kAttrIndex_RXCharValue = 2,
    kAttrIndex_TXCharValue = 4,
    kAttrIndex_TXCharCCCDValue = 5,
};

// Table of attribute definitions for Weave over BLE GATT service.
const esp_gatts_attr_db_t WoBLEGATTAttrs[] =
{
    // Service Declaration for Weave over BLE Service
    { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_16, (uint8_t *)UUID_PrimaryService, ESP_GATT_PERM_READ, ESP_UUID_LEN_128, ESP_UUID_LEN_128, (uint8_t *)UUID_WoBLEService } },

    // ----- Weave over BLE RX Characteristic -----

    // Characteristic declaration
    { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_16, (uint8_t *)UUID_CharDecl, ESP_GATT_PERM_READ, 1, 1, (uint8_t *)&CharProps_Write } },
    // Characteristic value
    { { ESP_GATT_RSP_BY_APP }, { ESP_UUID_LEN_128, (uint8_t *)UUID_WoBLEChar_RX, ESP_GATT_PERM_WRITE, 512, 0, NULL } },

    // ----- Weave over BLE TX Characteristic -----

    // Characteristic declaration
    { { ESP_GATT_AUTO_RSP }, { ESP_UUID_LEN_16, (uint8_t *)UUID_CharDecl, ESP_GATT_PERM_READ, 1, 1, (uint8_t *)&CharProps_ReadNotify } },
    // Characteristic value
    { { ESP_GATT_RSP_BY_APP }, { ESP_UUID_LEN_128, (uint8_t *)UUID_WoBLEChar_TX, ESP_GATT_PERM_READ, 512, 0, NULL } },
    // Client characteristic configuration description (CCCD) value
    { { ESP_GATT_RSP_BY_APP }, { ESP_UUID_LEN_16, (uint8_t *)UUID_ClientCharConfigDesc, ESP_GATT_PERM_READ|ESP_GATT_PERM_WRITE, 2, 0, NULL } },
};

const uint16_t WoBLEGATTAttrCount = sizeof(WoBLEGATTAttrs) / sizeof(WoBLEGATTAttrs[0]);

} // unnamed namespace


BLEManagerImpl BLEManagerImpl::sInstance;

WEAVE_ERROR BLEManagerImpl::_Init()
{
    WEAVE_ERROR err;

    // Initialize the Weave BleLayer.
    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);

    memset(mCons, 0, sizeof(mCons));
    mServiceMode = ConnectivityManager::kWoBLEServiceMode_Enabled;
    mAppIf = ESP_GATT_IF_NONE;
    mServiceAttrHandle = 0;
    mRXCharAttrHandle = 0;
    mTXCharAttrHandle = 0;
    mTXCharCCCDAttrHandle = 0;
    mFlags = kFlag_AdvertisingEnabled;
    memset(mDeviceName, 0, sizeof(mDeviceName));

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::_SetWoBLEServiceMode(WoBLEServiceMode val)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kWoBLEServiceMode_NotSupported, err = WEAVE_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mServiceMode == ConnectivityManager::kWoBLEServiceMode_NotSupported, err = WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE);

    if (val != mServiceMode)
    {
        mServiceMode = val;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(mServiceMode == ConnectivityManager::kWoBLEServiceMode_NotSupported, err = WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE);

    if (GetFlag(mFlags, kFlag_AdvertisingEnabled) != val)
    {
        SetFlag(mFlags, kFlag_AdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::_SetFastAdvertisingEnabled(bool val)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(mServiceMode == ConnectivityManager::kWoBLEServiceMode_NotSupported, err = WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE);

    if (GetFlag(mFlags, kFlag_FastAdvertisingEnabled) != val)
    {
        SetFlag(mFlags, kFlag_FastAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    if (strlen(mDeviceName) >= bufSize)
    {
        return WEAVE_ERROR_BUFFER_TOO_SMALL;
    }
    strcpy(buf, mDeviceName);
    return WEAVE_NO_ERROR;
}

WEAVE_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    if (mServiceMode == ConnectivityManager::kWoBLEServiceMode_NotSupported)
    {
        return WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE;
    }
    if (deviceName != NULL && deviceName[0] != 0)
    {
        if (strlen(deviceName) >= kMaxDeviceNameLength)
        {
            return WEAVE_ERROR_INVALID_ARGUMENT;
        }
        strcpy(mDeviceName, deviceName);
        SetFlag(mFlags, kFlag_UseCustomDeviceName);
    }
    else
    {
        mDeviceName[0] = 0;
        ClearFlag(mFlags, kFlag_UseCustomDeviceName);
    }
    return WEAVE_NO_ERROR;
}

void BLEManagerImpl::_OnPlatformEvent(const WeaveDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kWoBLESubscribe:
        HandleSubscribeReceived(event->WoBLESubscribe.ConId, &WEAVE_BLE_SVC_ID, &WeaveUUID_WoBLEChar_TX);
        {
            WeaveDeviceEvent event;
            event.Type = DeviceEventType::kWoBLEConnectionEstablished;
            PlatformMgr().PostEvent(&event);
        }
        break;

    case DeviceEventType::kWoBLEUnsubscribe:
        HandleUnsubscribeReceived(event->WoBLEUnsubscribe.ConId, &WEAVE_BLE_SVC_ID, &WeaveUUID_WoBLEChar_TX);
        break;

    case DeviceEventType::kWoBLEWriteReceived:
        HandleWriteReceived(event->WoBLEWriteReceived.ConId, &WEAVE_BLE_SVC_ID, &WeaveUUID_WoBLEChar_RX, event->WoBLEWriteReceived.Data);
        break;

    case DeviceEventType::kWoBLEIndicateConfirm:
        HandleIndicationConfirmation(event->WoBLEIndicateConfirm.ConId, &WEAVE_BLE_SVC_ID, &WeaveUUID_WoBLEChar_TX);
        break;

    case DeviceEventType::kWoBLEConnectionError:
        HandleConnectionError(event->WoBLEConnectionError.ConId, event->WoBLEConnectionError.Reason);
        break;

    case DeviceEventType::kFabricMembershipChange:
    case DeviceEventType::kServiceProvisioningChange:
    case DeviceEventType::kAccountPairingChange:

        // If WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled, and there is a change to the
        // device's provisioning state, then automatically disable WoBLE advertising if the device
        // is now fully provisioned.
#if WEAVE_DEVICE_CONFIG_WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            WeaveLogProgress(DeviceLayer, "WoBLE advertising disabled because device is fully provisioned");
        }
#endif // WEAVE_DEVICE_CONFIG_WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED

        // Force the advertising configuration to be refreshed to reflect new provisioning state.
        ClearFlag(mFlags, kFlag_AdvertisingConfigured);

        DriveBLEState();

    default:
        break;
    }
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    WeaveLogProgress(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    WeaveLogProgress(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    WEAVE_ERROR err;

    WeaveLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    // Signal the ESP BLE layer to close the conntion.
    err = esp_ble_gatts_close(mAppIf, conId);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "esp_ble_gatts_close() failed: %s", ErrorStr(err));
    }

    // Release the associated connection state record.
    ReleaseConnectionState(conId);

    // Force a refresh of the advertising state.
    SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
    ClearFlag(mFlags, kFlag_AdvertisingConfigured);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return (err == WEAVE_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    WoBLEConState * conState = const_cast<BLEManagerImpl *>(this)->GetConnectionState(conId);
    return (conState != NULL) ? conState->MTU : 0;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId, PacketBuffer * data)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    WoBLEConState * conState = GetConnectionState(conId);

    ESP_LOGD(TAG, "Sending indication for WoBLE TX characteristic (con %u, len %u)", conId, data->DataLength());

    VerifyOrExit(conState != NULL, err = WEAVE_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(conState->PendingIndBuf == NULL, err = WEAVE_ERROR_INCORRECT_STATE);

    err = esp_ble_gatts_send_indicate(mAppIf, conId, mTXCharAttrHandle, data->DataLength(), data->Start(), false);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "esp_ble_gatts_send_indicate() failed: %s", ErrorStr(err));
        ExitNow();
    }

    // Save a reference to the buffer until we get a indication from the ESP BLE layer that it
    // has been sent.
    conState->PendingIndBuf = data;
    data = NULL;

exit:
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "BLEManagerImpl::SendIndication() failed: %s", ErrorStr(err));
        PacketBuffer::Free(data);
        return false;
    }
    return true;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId, PacketBuffer * pBuf)
{
    WeaveLogError(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId, PacketBuffer * pBuf)
{
    WeaveLogError(DeviceLayer, "BLEManagerImpl::SendReadRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    WeaveLogError(DeviceLayer, "BLEManagerImpl::SendReadResponse() not supported");
    return false;
}

void BLEManagerImpl::NotifyWeaveConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
}

void BLEManagerImpl::DriveBLEState(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    // Perform any initialization actions that must occur after the Weave task is running.
    if (!GetFlag(mFlags, kFlag_AsyncInitCompleted))
    {
        SetFlag(mFlags, kFlag_AsyncInitCompleted);

        // If WEAVE_DEVICE_CONFIG_WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled,
        // disable WoBLE advertising if the device is fully provisioned.
#if WEAVE_DEVICE_CONFIG_WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            WeaveLogProgress(DeviceLayer, "WoBLE advertising disabled because device is fully provisioned");
        }
#endif // WEAVE_DEVICE_CONFIG_WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    }

    // If there's already a control operation in progress, wait until it completes.
    VerifyOrExit(!GetFlag(mFlags, kFlag_ControlOpInProgress), /* */);

    // Initializes the ESP BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kWoBLEServiceMode_Enabled && !GetFlag(mFlags, kFlag_ESPBLELayerInitialized))
    {
        err = InitESPBleLayer();
        SuccessOrExit(err);
    }

    // Register the WoBLE application with the ESP BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kWoBLEServiceMode_Enabled && !GetFlag(mFlags, kFlag_AppRegistered))
    {
        err = esp_ble_gatts_app_register(WoBLEAppId);
        if (err != WEAVE_NO_ERROR)
        {
            WeaveLogError(DeviceLayer, "esp_ble_gatts_app_register() failed: %s", ErrorStr(err));
            ExitNow();
        }

        SetFlag(mFlags, kFlag_ControlOpInProgress);

        ExitNow();
    }

    // Register the WoBLE GATT attributes with the ESP BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kWoBLEServiceMode_Enabled && !GetFlag(mFlags, kFlag_AttrsRegistered))
    {
        err = esp_ble_gatts_create_attr_tab(WoBLEGATTAttrs, mAppIf, WoBLEGATTAttrCount, 0);
        if (err != WEAVE_NO_ERROR)
        {
            WeaveLogError(DeviceLayer, "esp_ble_gatts_create_attr_tab() failed: %s", ErrorStr(err));
            ExitNow();
        }

        SetFlag(mFlags, kFlag_ControlOpInProgress);

        ExitNow();
    }

    // Start the WoBLE GATT service if needed.
    if (mServiceMode == ConnectivityManager::kWoBLEServiceMode_Enabled && !GetFlag(mFlags, kFlag_GATTServiceStarted))
    {
        err = esp_ble_gatts_start_service(mServiceAttrHandle);
        if (err != WEAVE_NO_ERROR)
        {
            WeaveLogError(DeviceLayer, "esp_ble_gatts_start_service() failed: %s", ErrorStr(err));
            ExitNow();
        }

        SetFlag(mFlags, kFlag_ControlOpInProgress);

        ExitNow();
    }

    // If the application has enabled WoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kWoBLEServiceMode_Enabled && GetFlag(mFlags, kFlag_AdvertisingEnabled)
#if WEAVE_DEVICE_CONFIG_WOBLE_SINGLE_CONNECTION
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
                ExitNow();
            }

            // Start advertising.  This is also an asynchronous step.
            err = StartAdvertising();
            ExitNow();
        }
    }

    // Otherwise stop advertising if needed...
    else
    {
        if (GetFlag(mFlags, kFlag_Advertising))
        {
            err = esp_ble_gap_stop_advertising();
            if (err != WEAVE_NO_ERROR)
            {
                WeaveLogError(DeviceLayer, "esp_ble_gap_stop_advertising() failed: %s", ErrorStr(err));
                ExitNow();
            }

            SetFlag(mFlags, kFlag_ControlOpInProgress);

            ExitNow();
        }
    }

    // Stop the WoBLE GATT service if needed.
    if (mServiceMode != ConnectivityManager::kWoBLEServiceMode_Enabled && GetFlag(mFlags, kFlag_GATTServiceStarted))
    {
        // TODO: what to do about existing connections??

        err = esp_ble_gatts_stop_service(mServiceAttrHandle);
        if (err != WEAVE_NO_ERROR)
        {
            WeaveLogError(DeviceLayer, "esp_ble_gatts_stop_service() failed: %s", ErrorStr(err));
            ExitNow();
        }

        SetFlag(mFlags, kFlag_ControlOpInProgress);

        ExitNow();
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Disabling WoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kWoBLEServiceMode_Disabled;
    }
}

WEAVE_ERROR BLEManagerImpl::InitESPBleLayer(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(!GetFlag(mFlags, kFlag_ESPBLELayerInitialized), /* */);

    // If the ESP Bluetooth controller has not been initialized...
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE)
    {
        // Since Weave only uses BLE, release memory held by ESP classic bluetooth stack.
        err = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
        if (err != WEAVE_NO_ERROR)
        {
            WeaveLogError(DeviceLayer, "esp_bt_controller_mem_release() failed: %s", ErrorStr(err));
            ExitNow();
        }

        // Initialize the ESP Bluetooth controller.
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        err = esp_bt_controller_init(&bt_cfg);
        if (err != WEAVE_NO_ERROR)
        {
            WeaveLogError(DeviceLayer, "esp_bt_controller_init() failed: %s", ErrorStr(err));
            ExitNow();
        }
    }

    // If the ESP Bluetooth controller has not been enabled, enable it now.
    if (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_ENABLED)
    {
        err = esp_bt_controller_enable(ESP_BT_MODE_BLE);
        if (err != WEAVE_NO_ERROR)
        {
            WeaveLogError(DeviceLayer, "esp_bt_controller_enable() failed: %s", ErrorStr(err));
            ExitNow();
        }
    }

    // If the ESP Bluedroid stack has not been initialized, initialize it now.
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_UNINITIALIZED)
    {
        err = esp_bluedroid_init();
        if (err != WEAVE_NO_ERROR)
        {
            WeaveLogError(DeviceLayer, "esp_bluedroid_init() failed: %s", ErrorStr(err));
            ExitNow();
        }
    }

    // If the ESP Bluedroid stack has not been enabled, enable it now.
    if (esp_bluedroid_get_status() != ESP_BLUEDROID_STATUS_ENABLED)
    {
        err = esp_bluedroid_enable();
        if (err != WEAVE_NO_ERROR)
        {
            WeaveLogError(DeviceLayer, "esp_bluedroid_enable() failed: %s", ErrorStr(err));
            ExitNow();
        }
    }

    // Register a callback to receive GATT events.
    err = esp_ble_gatts_register_callback(HandleGATTEvent);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "esp_ble_gatts_register_callback() failed: %s", ErrorStr(err));
        ExitNow();
    }

    // Register a callback to receive GAP events.
    err = esp_ble_gap_register_callback(HandleGAPEvent);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "esp_ble_gap_register_callback() failed: %s", ErrorStr(err));
        ExitNow();
    }

    // Set the maximum supported MTU size.
    err = esp_ble_gatt_set_local_mtu(ESP_GATT_MAX_MTU_SIZE);
    if (err != WEAVE_NO_ERROR){
        WeaveLogError(DeviceLayer, "esp_ble_gatt_set_local_mtu() failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    SetFlag(mFlags, kFlag_ESPBLELayerInitialized);

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    WEAVE_ERROR err;
    esp_ble_adv_data_t advertData;
    ESP32WeaveServiceData weaveServiceData;

    // If a custom device name has not been specified, generate a Nest-standard name based on the
    // bottom digits of the Weave device id.
    if (!GetFlag(mFlags, kFlag_UseCustomDeviceName))
    {
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04" PRIX32,
                 WEAVE_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX,
                 (uint32_t)FabricState.LocalNodeId);
        mDeviceName[kMaxDeviceNameLength] = 0;
    }

    // Configure the BLE device name.
    err = esp_ble_gap_set_device_name(mDeviceName);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "esp_ble_gap_set_device_name() failed: %s", ErrorStr(err));
        ExitNow();
    }

    // Configure the contents of the advertising packet.
    memset(&advertData, 0, sizeof(advertData));
    advertData.set_scan_rsp = false;
    advertData.include_name = true;
    advertData.include_txpower = false;
    advertData.min_interval = 0;
    advertData.max_interval = 0;
    advertData.appearance = 0;
    advertData.manufacturer_len = 0;
    advertData.p_manufacturer_data = NULL;
    advertData.service_data_len = 0;
    advertData.p_service_data = NULL;
    advertData.service_uuid_len = sizeof(UUID_WoBLEService);
    advertData.p_service_uuid = (uint8_t *)UUID_WoBLEService;
    advertData.flag = ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT;
    err = esp_ble_gap_config_adv_data(&advertData);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "esp_ble_gap_config_adv_data(<advertising data>) failed: %s", ErrorStr(err));
        ExitNow();
    }

    // Construct the Weave BLE Service Data to be sent in the scan response packet.  On the ESP32,
    // the buffer given to esp_ble_gap_config_adv_data() must begin with the Weave BLE service UUID.
    memcpy(weaveServiceData.ServiceUUID, ShortUUID_WoBLEService, sizeof(weaveServiceData.ServiceUUID));
    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(weaveServiceData.DeviceIdInfo);
    SuccessOrExit(err);

    // Configure the contents of the scan response packet.
    memset(&advertData, 0, sizeof(advertData));
    advertData.set_scan_rsp = true;
    advertData.include_name = false;
    advertData.include_txpower = true;
    advertData.min_interval = 0;
    advertData.max_interval = 0;
    advertData.appearance = 0;
    advertData.manufacturer_len = 0;
    advertData.p_manufacturer_data = NULL;
    advertData.service_data_len = sizeof(weaveServiceData);
    advertData.p_service_data = (uint8_t *)&weaveServiceData;
    advertData.service_uuid_len = 0;
    advertData.p_service_uuid = NULL;
    advertData.flag = 0;
    err = esp_ble_gap_config_adv_data(&advertData);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "esp_ble_gap_config_adv_data(<scan response>) failed: %s", ErrorStr(err));
        ExitNow();
    }

    SetFlag(mFlags, kFlag_ControlOpInProgress);

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::StartAdvertising(void)
{
    WEAVE_ERROR err;
    esp_ble_adv_params_t advertParams =
    {
        0,                                  // adv_int_min
        0,                                  // adv_int_max
        ADV_TYPE_IND,                       // adv_type
        BLE_ADDR_TYPE_PUBLIC,               // own_addr_type
        { 0 },                              // peer_addr
        BLE_ADDR_TYPE_RANDOM,               // peer_addr_type
        ADV_CHNL_ALL,                       // channel_map
        ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,  // adv_filter_policy
    };

    // Inform the ThreadStackManager that WoBLE advertising is about to start.
#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
    ThreadStackMgr().OnWoBLEAdvertisingStart();
#endif // WEAVE_DEVICE_CONFIG_ENABLE_THREAD

    // Advertise connectable if we haven't reached the maximum number of connections.
    size_t numCons = NumConnections();
    bool connectable = (numCons < kMaxConnections);
    advertParams.adv_type = connectable ? ADV_TYPE_IND : ADV_TYPE_NONCONN_IND;

    // Advertise in fast mode if not fully provisioned and there are no WoBLE connections, or
    // if the application has expressly requested fast advertising.
    advertParams.adv_int_min = advertParams.adv_int_max =
        ((numCons == 0 && !ConfigurationMgr().IsFullyProvisioned()) || GetFlag(mFlags, kFlag_FastAdvertisingEnabled))
        ? WEAVE_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL
        : WEAVE_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL;

    WeaveLogProgress(DeviceLayer, "Configuring WoBLE advertising (interval %" PRIu32 " ms, %sconnectable, device name %s)",
             (((uint32_t)advertParams.adv_int_min) * 10) / 16,
             (connectable) ? "" : "non-",
             mDeviceName);

    err = esp_ble_gap_start_advertising(&advertParams);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "esp_ble_gap_start_advertising() failed: %s", ErrorStr(err));
        ExitNow();
    }

    SetFlag(mFlags, kFlag_ControlOpInProgress);

exit:
    return err;
}

void BLEManagerImpl::HandleGATTControlEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    bool controlOpComplete = false;

    // Ignore GATT control events that do not pertain to the WoBLE application, except for ESP_GATTS_REG_EVT.
    if (event != ESP_GATTS_REG_EVT && (!GetFlag(mFlags, kFlag_AppRegistered) || gatts_if != mAppIf))
    {
        ExitNow();
    }

    switch (event)
    {
    case ESP_GATTS_REG_EVT:

        if (param->reg.app_id == WoBLEAppId)
        {
            if (param->reg.status != ESP_GATT_OK)
            {
                WeaveLogError(DeviceLayer, "ESP_GATTS_REG_EVT error: %d", (int)param->reg.status);
                ExitNow(err = ESP_ERR_INVALID_RESPONSE);
            }

            // Save the 'interface type' assigned to the WoBLE application by the ESP BLE layer.
            mAppIf = gatts_if;

            SetFlag(mFlags, kFlag_AppRegistered);
            controlOpComplete = true;
        }

        break;

    case ESP_GATTS_CREAT_ATTR_TAB_EVT:

        if (param->add_attr_tab.status != ESP_GATT_OK)
        {
            WeaveLogError(DeviceLayer, "ESP_GATTS_CREAT_ATTR_TAB_EVT error: %d", (int)param->add_attr_tab.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        // Save the attribute handles assigned by the ESP BLE layer to the WoBLE attributes.
        mServiceAttrHandle = param->add_attr_tab.handles[kAttrIndex_ServiceDeclaration];
        mRXCharAttrHandle = param->add_attr_tab.handles[kAttrIndex_RXCharValue];
        mTXCharAttrHandle = param->add_attr_tab.handles[kAttrIndex_TXCharValue];
        mTXCharCCCDAttrHandle = param->add_attr_tab.handles[kAttrIndex_TXCharCCCDValue];

        SetFlag(mFlags, kFlag_AttrsRegistered);
        controlOpComplete = true;

        break;

    case ESP_GATTS_START_EVT:

        if (param->start.status != ESP_GATT_OK)
        {
            WeaveLogError(DeviceLayer, "ESP_GATTS_START_EVT error: %d", (int)param->start.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        WeaveLogProgress(DeviceLayer, "WoBLE GATT service started");

        SetFlag(mFlags, kFlag_GATTServiceStarted);
        controlOpComplete = true;

        break;


    case ESP_GATTS_STOP_EVT:

        if (param->stop.status != ESP_GATT_OK)
        {
            WeaveLogError(DeviceLayer, "ESP_GATTS_STOP_EVT error: %d", (int)param->stop.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        WeaveLogProgress(DeviceLayer, "WoBLE GATT service stopped");

        ClearFlag(mFlags, kFlag_GATTServiceStarted);
        controlOpComplete = true;

        break;

    case ESP_GATTS_RESPONSE_EVT:
        ESP_LOGD(TAG, "ESP_GATTS_RESPONSE_EVT (handle %u, status %d)", param->rsp.handle, (int)param->rsp.status);
        break;

    default:
        // Ignore all other event types.
        break;
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Disabling WoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kWoBLEServiceMode_Disabled;
    }
    if (controlOpComplete)
    {
        ClearFlag(mFlags, kFlag_ControlOpInProgress);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

void BLEManagerImpl::HandleGATTCommEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param)
{
    // Ignore the event if the WoBLE service hasn't been started, or if the event is for a different
    // BLE application.
    if (!GetFlag(sInstance.mFlags, kFlag_GATTServiceStarted) || gatts_if != sInstance.mAppIf)
    {
        return;
    }

    switch (event)
    {
    case ESP_GATTS_CONNECT_EVT:
        WeaveLogProgress(DeviceLayer, "BLE GATT connection established (con %u)", param->connect.conn_id);

        // Allocate a connection state record for the new connection.
        GetConnectionState(param->mtu.conn_id, true);

        // Receiving a connection stops the advertising processes.  So force a refresh of the advertising
        // state.
        SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
        ClearFlag(mFlags, kFlag_AdvertisingConfigured);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);

        break;

    case ESP_GATTS_DISCONNECT_EVT:
        HandleDisconnect(param);
        break;

    case ESP_GATTS_READ_EVT:
        if (param->read.handle == mTXCharAttrHandle)
        {
            HandleTXCharRead(param);
        }
        if (param->read.handle == mTXCharCCCDAttrHandle)
        {
            HandleTXCharCCCDRead(param);
        }
        break;

    case ESP_GATTS_WRITE_EVT:
        if (param->write.handle == mRXCharAttrHandle)
        {
            HandleRXCharWrite(param);
        }
        if (param->write.handle == mTXCharCCCDAttrHandle)
        {
            HandleTXCharCCCDWrite(param);
        }
        break;

    case ESP_GATTS_CONF_EVT:
        {
            WoBLEConState * conState = GetConnectionState(param->conf.conn_id);
            if (conState != NULL)
            {
                HandleTXCharConfirm(conState, param);
            }
        }
        break;

    case ESP_GATTS_MTU_EVT:
        {
            ESP_LOGD(TAG, "MTU for con %u: %u", param->mtu.conn_id, param->mtu.mtu);
            WoBLEConState * conState = GetConnectionState(param->mtu.conn_id);
            if (conState != NULL)
            {
                conState->MTU = param->mtu.mtu;
            }
        }
        break;

    default:
        break;
    }
}

void BLEManagerImpl::HandleRXCharWrite(esp_ble_gatts_cb_param_t * param)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    PacketBuffer * buf = NULL;
    bool needResp = param->write.need_rsp;

    ESP_LOGD(TAG, "Write request received for WoBLE RX characteristic (con %u, len %u)", param->write.conn_id, param->write.len);

    // Disallow long writes.
    VerifyOrExit(param->write.is_prep == false, err = WEAVE_ERROR_INVALID_ARGUMENT);

    // Copy the data to a PacketBuffer.
    buf = PacketBuffer::New(0);
    VerifyOrExit(buf != NULL, err = WEAVE_ERROR_NO_MEMORY);
    VerifyOrExit(buf->AvailableDataLength() >= param->write.len, err = WEAVE_ERROR_BUFFER_TOO_SMALL);
    memcpy(buf->Start(), param->write.value, param->write.len);
    buf->SetDataLength(param->write.len);

    // Send a response if requested.
    if (needResp)
    {
        esp_ble_gatts_send_response(mAppIf, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        needResp = false;
    }

    // Post an event to the Weave queue to deliver the data into the Weave stack.
    {
        WeaveDeviceEvent event;
        event.Type = DeviceEventType::kWoBLEWriteReceived;
        event.WoBLEWriteReceived.ConId = param->write.conn_id;
        event.WoBLEWriteReceived.Data = buf;
        PlatformMgr().PostEvent(&event);
        buf = NULL;
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
        if (needResp)
        {
            esp_ble_gatts_send_response(mAppIf, param->write.conn_id, param->write.trans_id, ESP_GATT_INTERNAL_ERROR, NULL);
        }
        // TODO: fail connection???
    }
    PacketBuffer::Free(buf);
}

void BLEManagerImpl::HandleTXCharRead(esp_ble_gatts_cb_param_t * param)
{
    WEAVE_ERROR err;
    esp_gatt_rsp_t rsp;

    ESP_LOGD(TAG, "Read request received for WoBLE TX characteristic (con %u)", param->read.conn_id);

    // Send a zero-length response.
    memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
    rsp.attr_value.handle = param->read.handle;
    err = esp_ble_gatts_send_response(mAppIf, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "esp_ble_gatts_send_response() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleTXCharCCCDRead(esp_ble_gatts_cb_param_t * param)
{
    WEAVE_ERROR err;
    WoBLEConState * conState;
    esp_gatt_rsp_t rsp;

    ESP_LOGD(TAG, "Read request received for WoBLE TX characteristic CCCD (con %u)", param->read.conn_id);

    // Find the connection state record.
    conState = GetConnectionState(param->read.conn_id);

    // Send current CCCD value, or an error if we failed to allocate a connection state object.
    memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
    rsp.attr_value.handle = param->read.handle;
    if (conState != NULL)
    {
        rsp.attr_value.len = 2;
        rsp.attr_value.value[0] = conState->Subscribed ? 1 : 0;
    }
    err = esp_ble_gatts_send_response(mAppIf, param->read.conn_id, param->read.trans_id, (conState != NULL) ? ESP_GATT_OK : ESP_GATT_INTERNAL_ERROR, &rsp);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "esp_ble_gatts_send_response() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(esp_ble_gatts_cb_param_t * param)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    WoBLEConState * conState;
    bool needResp = param->write.need_rsp;
    bool indicationsEnabled;

    ESP_LOGD(TAG, "Write request received for WoBLE TX characteristic CCCD (con %u, len %u)", param->write.conn_id, param->write.len);

    // Find the connection state record.
    conState = GetConnectionState(param->read.conn_id);
    VerifyOrExit(conState != NULL, err = WEAVE_ERROR_NO_MEMORY);

    // Disallow long writes.
    VerifyOrExit(param->write.is_prep == false, err = WEAVE_ERROR_INVALID_ARGUMENT);

    // Determine if the client is enabling or disabling indications.
    indicationsEnabled = (param->write.len > 0 && (param->write.value[0] != 0));

    // Send a response to the Write if requested.
    if (needResp)
    {
        esp_ble_gatts_send_response(mAppIf, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        needResp = false;
    }

    // Post an event to the Weave queue to process either a WoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling indications.
    {
        WeaveDeviceEvent event;
        event.Type = (indicationsEnabled) ? DeviceEventType::kWoBLESubscribe : DeviceEventType::kWoBLEUnsubscribe;
        event.WoBLESubscribe.ConId = param->write.conn_id;
        PlatformMgr().PostEvent(&event);
    }

    WeaveLogProgress(DeviceLayer, "WoBLE %s received", indicationsEnabled ? "subscribe" : "unsubscribe");

exit:
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
        if (needResp)
        {
            esp_ble_gatts_send_response(mAppIf, param->write.conn_id, param->write.trans_id, ESP_GATT_INTERNAL_ERROR, NULL);
        }
        // TODO: fail connection???
    }
}

void BLEManagerImpl::HandleTXCharConfirm(WoBLEConState * conState, esp_ble_gatts_cb_param_t * param)
{
    ESP_LOGD(TAG, "Confirm received for WoBLE TX characteristic indication (con %u, status %u)", param->conf.conn_id, param->conf.status);

    // If there is a pending indication buffer for the connection, release it now.
    PacketBuffer::Free(conState->PendingIndBuf);
    conState->PendingIndBuf = NULL;

    // If the confirmation was successful...
    if (param->conf.status == ESP_GATT_OK)
    {
        // Post an event to the Weave queue to process the indicate confirmation.
        WeaveDeviceEvent event;
        event.Type = DeviceEventType::kWoBLEIndicateConfirm;
        event.WoBLEIndicateConfirm.ConId = param->conf.conn_id;
        PlatformMgr().PostEvent(&event);
    }

    else
    {
        WeaveDeviceEvent event;
        event.Type = DeviceEventType::kWoBLEConnectionError;
        event.WoBLEConnectionError.ConId = param->disconnect.conn_id;
        event.WoBLEConnectionError.Reason = BLE_ERROR_WOBLE_PROTOCOL_ABORT;
        PlatformMgr().PostEvent(&event);
    }
}

void BLEManagerImpl::HandleDisconnect(esp_ble_gatts_cb_param_t * param)
{
    WeaveLogProgress(DeviceLayer, "BLE GATT connection closed (con %u, reason %u)", param->disconnect.conn_id, param->disconnect.reason);

    // If this was a WoBLE connection, release the associated connection state record
    // and post an event to deliver a connection error to the WoBLE layer.
    if (ReleaseConnectionState(param->disconnect.conn_id))
    {
        WeaveDeviceEvent event;
        event.Type = DeviceEventType::kWoBLEConnectionError;
        event.WoBLEConnectionError.ConId = param->disconnect.conn_id;
        switch (param->disconnect.reason)
        {
        case ESP_GATT_CONN_TERMINATE_PEER_USER:
            event.WoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;
        case ESP_GATT_CONN_TERMINATE_LOCAL_HOST:
            event.WoBLEConnectionError.Reason = BLE_ERROR_APP_CLOSED_CONNECTION;
            break;
        default:
            event.WoBLEConnectionError.Reason = BLE_ERROR_WOBLE_PROTOCOL_ABORT;
            break;
        }
        PlatformMgr().PostEvent(&event);

        // Force a refresh of the advertising state.
        SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
        ClearFlag(mFlags, kFlag_AdvertisingConfigured);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

BLEManagerImpl::WoBLEConState * BLEManagerImpl::GetConnectionState(uint16_t conId, bool allocate)
{
    uint16_t freeIndex = kMaxConnections;

    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mCons[i].Allocated == 1)
        {
            if (mCons[i].ConId == conId)
            {
                return &mCons[i];
            }
        }

        else if (i < freeIndex)
        {
            freeIndex = i;
        }
    }

    if (allocate)
    {
        if (freeIndex < kMaxConnections)
        {
            memset(&mCons[freeIndex], 0, sizeof(WoBLEConState));
            mCons[freeIndex].Allocated = 1;
            mCons[freeIndex].ConId = conId;
            return &mCons[freeIndex];
        }

        WeaveLogError(DeviceLayer, "Failed to allocate WoBLEConState");
    }

    return NULL;
}

bool BLEManagerImpl::ReleaseConnectionState(uint16_t conId)
{
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mCons[i].Allocated && mCons[i].ConId == conId)
        {
            PacketBuffer::Free(mCons[i].PendingIndBuf);
            mCons[i].Allocated = 0;
            return true;
        }
    }

    return false;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    uint16_t numCons = 0;
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mCons[i].Allocated)
        {
            numCons++;
        }
    }

    return numCons;
}

void BLEManagerImpl::HandleGATTEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param)
{
    ESP_LOGV(TAG, "GATT Event: %d (if %d)", (int)event, (int)gatts_if);

    // This method is invoked on the ESP BLE thread.  Therefore we must hold a lock
    // on the Weave stack while processing the event.
    PlatformMgr().LockWeaveStack();

    sInstance.HandleGATTControlEvent(event, gatts_if, param);
    sInstance.HandleGATTCommEvent(event, gatts_if, param);

    PlatformMgr().UnlockWeaveStack();
}

void BLEManagerImpl::HandleGAPEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    ESP_LOGV(TAG, "GAP Event: %d", (int)event);

    // This method is invoked on the ESP BLE thread.  Therefore we must hold a lock
    // on the Weave stack while processing the event.
    PlatformMgr().LockWeaveStack();

    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:

        if (param->adv_data_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            WeaveLogError(DeviceLayer, "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT error: %d", (int)param->adv_data_cmpl.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        SetFlag(sInstance.mFlags, kFlag_AdvertisingConfigured);
        ClearFlag(sInstance.mFlags, kFlag_ControlOpInProgress);

        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:

        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            WeaveLogError(DeviceLayer, "ESP_GAP_BLE_ADV_START_COMPLETE_EVT error: %d", (int)param->adv_start_cmpl.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        ClearFlag(sInstance.mFlags, kFlag_ControlOpInProgress);
        ClearFlag(sInstance.mFlags, kFlag_AdvertisingRefreshNeeded);

        // Transition to the Advertising state...
        if (!GetFlag(sInstance.mFlags, kFlag_Advertising))
        {
            WeaveLogProgress(DeviceLayer, "WoBLE advertising started");

            SetFlag(sInstance.mFlags, kFlag_Advertising);

            // Post a WoBLEAdvertisingChange(Started) event.
            {
                WeaveDeviceEvent advChange;
                advChange.Type = DeviceEventType::kWoBLEAdvertisingChange;
                advChange.WoBLEAdvertisingChange.Result = kActivity_Started;
                PlatformMgr().PostEvent(&advChange);
            }
        }

        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:

        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            WeaveLogError(DeviceLayer, "ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT error: %d", (int)param->adv_stop_cmpl.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        ClearFlag(sInstance.mFlags, kFlag_ControlOpInProgress);
        ClearFlag(sInstance.mFlags, kFlag_AdvertisingRefreshNeeded);

        // Transition to the not Advertising state...
        if (GetFlag(sInstance.mFlags, kFlag_Advertising))
        {
            ClearFlag(sInstance.mFlags, kFlag_Advertising);

            WeaveLogProgress(DeviceLayer, "WoBLE advertising stopped");

            // Directly inform the ThreadStackManager that WoBLE advertising has stopped.
#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
            ThreadStackMgr().OnWoBLEAdvertisingStop();
#endif // WEAVE_DEVICE_CONFIG_ENABLE_THREAD

            // Post a WoBLEAdvertisingChange(Stopped) event.
            {
                WeaveDeviceEvent advChange;
                advChange.Type = DeviceEventType::kWoBLEAdvertisingChange;
                advChange.WoBLEAdvertisingChange.Result = kActivity_Stopped;
                PlatformMgr().PostEvent(&advChange);
            }
        }

        break;

    default:
        break;
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Disabling WoBLE service due to error: %s", ErrorStr(err));
        sInstance.mServiceMode = ConnectivityManager::kWoBLEServiceMode_Disabled;
    }
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
    PlatformMgr().UnlockWeaveStack();
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}


} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // WEAVE_DEVICE_CONFIG_ENABLE_WOBLE

