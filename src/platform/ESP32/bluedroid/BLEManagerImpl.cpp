/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "sdkconfig.h"

#if CONFIG_BT_BLUEDROID_ENABLED

#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>
#include <system/SystemTimer.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "esp_gatts_api.h"
#include "esp_log.h"

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

const uint16_t CHIPoBLEAppId = 0x235A;

const uint8_t UUID_PrimaryService[]        = { 0x00, 0x28 };
const uint8_t UUID_CharDecl[]              = { 0x03, 0x28 };
const uint8_t UUID_ClientCharConfigDesc[]  = { 0x02, 0x29 };
const uint8_t UUID_CHIPoBLEService[]       = { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
                                         0x00, 0x10, 0x00, 0x00, 0xF6, 0xFF, 0x00, 0x00 };
const uint8_t ShortUUID_CHIPoBLEService[]  = { 0xF6, 0xFF };
const uint8_t UUID_CHIPoBLEChar_RX[]       = { 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95,
                                         0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 };
const uint8_t UUID_CHIPoBLEChar_TX[]       = { 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95,
                                         0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 };
const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

const uint8_t CharProps_ReadNotify = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
const uint8_t CharProps_Write      = ESP_GATT_CHAR_PROP_BIT_WRITE;

// Offsets into CHIPoBLEGATTAttrs for specific attributes.
enum
{
    kAttrIndex_ServiceDeclaration = 0,
    kAttrIndex_RXCharValue        = 2,
    kAttrIndex_TXCharValue        = 4,
    kAttrIndex_TXCharCCCDValue    = 5,
};

// Table of attribute definitions for Chip over BLE GATT service.
const esp_gatts_attr_db_t CHIPoBLEGATTAttrs[] = {
    // Service Declaration for Chip over BLE Service
    { { ESP_GATT_AUTO_RSP },
      { ESP_UUID_LEN_16, (uint8_t *) UUID_PrimaryService, ESP_GATT_PERM_READ, ESP_UUID_LEN_16, ESP_UUID_LEN_16,
        (uint8_t *) ShortUUID_CHIPoBLEService } },

    // ----- Chip over BLE RX Characteristic -----

    // Characteristic declaration
    { { ESP_GATT_AUTO_RSP },
      { ESP_UUID_LEN_16, (uint8_t *) UUID_CharDecl, ESP_GATT_PERM_READ, 1, 1, (uint8_t *) &CharProps_Write } },
    // Characteristic value
    { { ESP_GATT_RSP_BY_APP }, { ESP_UUID_LEN_128, (uint8_t *) UUID_CHIPoBLEChar_RX, ESP_GATT_PERM_WRITE, 512, 0, NULL } },

    // ----- Chip over BLE TX Characteristic -----

    // Characteristic declaration
    { { ESP_GATT_AUTO_RSP },
      { ESP_UUID_LEN_16, (uint8_t *) UUID_CharDecl, ESP_GATT_PERM_READ, 1, 1, (uint8_t *) &CharProps_ReadNotify } },
    // Characteristic value
    { { ESP_GATT_RSP_BY_APP }, { ESP_UUID_LEN_128, (uint8_t *) UUID_CHIPoBLEChar_TX, ESP_GATT_PERM_READ, 512, 0, NULL } },
    // Client characteristic configuration description (CCCD) value
    { { ESP_GATT_RSP_BY_APP },
      { ESP_UUID_LEN_16, (uint8_t *) UUID_ClientCharConfigDesc, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, 2, 0, NULL } },
};

const uint16_t CHIPoBLEGATTAttrCount = sizeof(CHIPoBLEGATTAttrs) / sizeof(CHIPoBLEGATTAttrs[0]);

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;

BLEManagerImpl::BLEManagerImpl() :
    mAdvertiseTimerCallback(HandleAdvertisementTimer, this), mFastAdvertiseTimerCallback(HandleFastAdvertisementTimer, this)
{}

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    // Initialize the Chip BleLayer.
    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);

    memset(mCons, 0, sizeof(mCons));
    mServiceMode          = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mAppIf                = ESP_GATT_IF_NONE;
    mServiceAttrHandle    = 0;
    mRXCharAttrHandle     = 0;
    mTXCharAttrHandle     = 0;
    mTXCharCCCDAttrHandle = 0;
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);
    memset(mDeviceName, 0, sizeof(mDeviceName));

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

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

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (val)
    {
        mAdvertiseStartTime = System::Timer::GetCurrentEpoch();
        SystemLayer.StartTimer(kAdvertiseTimeout, &mAdvertiseTimerCallback);
        SystemLayer.StartTimer(kFastAdvertiseTimeout, &mFastAdvertiseTimerCallback);
    }
    mFlags.Set(Flags::kFastAdvertisingEnabled, val);
    mFlags.Set(Flags::kAdvertisingRefreshNeeded, 1);
    mFlags.Set(Flags::kAdvertisingEnabled, val);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
exit:
    return err;
}

void BLEManagerImpl::HandleAdvertisementTimer(void * context)
{
    static_cast<BLEManagerImpl *>(context)->HandleAdvertisementTimer();
}

void BLEManagerImpl::HandleAdvertisementTimer()
{
    uint64_t currentTimestamp = System::Timer::GetCurrentEpoch();

    if (currentTimestamp - mAdvertiseStartTime >= kAdvertiseTimeout)
    {
        mFlags.Clear(Flags::kAdvertisingEnabled);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

void BLEManagerImpl::HandleFastAdvertisementTimer(void * context)
{
    static_cast<BLEManagerImpl *>(context)->HandleFastAdvertisementTimer();
}

void BLEManagerImpl::HandleFastAdvertisementTimer()
{
    uint64_t currentTimestamp = System::Timer::GetCurrentEpoch();

    if (currentTimestamp - mAdvertiseStartTime >= kFastAdvertiseTimeout)
    {
        mFlags.Clear(Flags::kFastAdvertisingEnabled);
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    switch (mode)
    {
    case BLEAdvertisingMode::kFastAdvertising:
        mFlags.Set(Flags::kFastAdvertisingEnabled);
        break;
    case BLEAdvertisingMode::kSlowAdvertising:
        mFlags.Clear(Flags::kFastAdvertisingEnabled);
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
    return CHIP_NO_ERROR;
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
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    if (deviceName != NULL && deviceName[0] != 0)
    {
        if (strlen(deviceName) >= kMaxDeviceNameLength)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        strcpy(mDeviceName, deviceName);
        mFlags.Set(Flags::kUseCustomDeviceName);
    }
    else
    {
        mDeviceName[0] = 0;
        mFlags.Clear(Flags::kUseCustomDeviceName);
    }
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe:
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        {
            ChipDeviceEvent connectionEvent;
            connectionEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
            PlatformMgr().PostEvent(&connectionEvent);
        }
        break;

    case DeviceEventType::kCHIPoBLEUnsubscribe:
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        break;

    case DeviceEventType::kCHIPoBLEWriteReceived:
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
        break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm:
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
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
            mFlags.Clear(Flags::kAdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED

        // Force the advertising configuration to be refreshed to reflect new provisioning state.
        ChipLogProgress(DeviceLayer, "Updating advertising data");
        mFlags.Clear(Flags::kAdvertisingConfigured);
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);

        DriveBLEState();

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

    // Signal the ESP BLE layer to close the conntion.
    err = MapBLEError(esp_ble_gatts_close(mAppIf, conId));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gatts_close() failed: %s", ErrorStr(err));
    }

    // Release the associated connection state record.
    ReleaseConnectionState(conId);

    // Force a refresh of the advertising state.
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    mFlags.Clear(Flags::kAdvertisingConfigured);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return (err == CHIP_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    CHIPoBLEConState * conState = const_cast<BLEManagerImpl *>(this)->GetConnectionState(conId);
    return (conState != NULL) ? conState->MTU : 0;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    CHIPoBLEConState * conState = GetConnectionState(conId);

    ESP_LOGD(TAG, "Sending indication for CHIPoBLE TX characteristic (con %u, len %u)", conId, data->DataLength());

    VerifyOrExit(conState != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(conState->PendingIndBuf.IsNull(), err = CHIP_ERROR_INCORRECT_STATE);

    err = MapBLEError(esp_ble_gatts_send_indicate(mAppIf, conId, mTXCharAttrHandle, data->DataLength(), data->Start(), false));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gatts_send_indicate() failed: %s", ErrorStr(err));
        ExitNow();
    }

    // Save a reference to the buffer until we get a indication from the ESP BLE layer that it
    // has been sent.
    conState->PendingIndBuf = std::move(data);

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

CHIP_ERROR BLEManagerImpl::MapBLEError(int bleErr)
{
    switch (bleErr)
    {
    case ESP_OK:
        return CHIP_NO_ERROR;
    case ESP_ERR_INVALID_ARG:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case ESP_ERR_INVALID_STATE:
        return CHIP_ERROR_INCORRECT_STATE;
    case ESP_ERR_NO_MEM:
        return CHIP_ERROR_NO_MEMORY;
    default:
        return CHIP_DEVICE_CONFIG_ESP32_BLE_ERROR_MIN + static_cast<CHIP_ERROR>(bleErr);
    }
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the Chip task is running.
    if (!mFlags.Has(Flags::kAsyncInitCompleted))
    {
        mFlags.Set(Flags::kAsyncInitCompleted);

        // If CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled,
        // disable CHIPoBLE advertising if the device is fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            mFlags.Clear(Flags::kAdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    }

    // If there's already a control operation in progress, wait until it completes.
    VerifyOrExit(!mFlags.Has(Flags::kControlOpInProgress), /* */);

    // Initializes the ESP BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !mFlags.Has(Flags::kESPBLELayerInitialized))
    {
        err = InitESPBleLayer();
        SuccessOrExit(err);
    }

    // Register the CHIPoBLE application with the ESP BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !mFlags.Has(Flags::kAppRegistered))
    {
        err = MapBLEError(esp_ble_gatts_app_register(CHIPoBLEAppId));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "esp_ble_gatts_app_register() failed: %s", ErrorStr(err));
            ExitNow();
        }

        mFlags.Set(Flags::kControlOpInProgress);

        ExitNow();
    }

    // Register the CHIPoBLE GATT attributes with the ESP BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !mFlags.Has(Flags::kAttrsRegistered))
    {
        err = MapBLEError(esp_ble_gatts_create_attr_tab(CHIPoBLEGATTAttrs, mAppIf, CHIPoBLEGATTAttrCount, 0));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "esp_ble_gatts_create_attr_tab() failed: %s", ErrorStr(err));
            ExitNow();
        }

        mFlags.Set(Flags::kControlOpInProgress);

        ExitNow();
    }

    // Start the CHIPoBLE GATT service if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !mFlags.Has(Flags::kGATTServiceStarted))
    {
        err = MapBLEError(esp_ble_gatts_start_service(mServiceAttrHandle));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "esp_ble_gatts_start_service() failed: %s", ErrorStr(err));
            ExitNow();
        }

        mFlags.Set(Flags::kControlOpInProgress);

        ExitNow();
    }

    // If the application has enabled CHIPoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled &&
        mFlags.Has(Flags::kAdvertisingEnabled)
#if CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION
        // and no connections are active...
        && (_NumConnections() == 0)
#endif
    )
    {
        // Start/re-start advertising if not already advertising, or if the advertising state of the
        // ESP BLE layer needs to be refreshed.
        if (!mFlags.Has(Flags::kAdvertising) || mFlags.Has(Flags::kAdvertisingRefreshNeeded))
        {
            // Configure advertising data if it hasn't been done yet.  This is an asynchronous step which
            // must complete before advertising can be started.  When that happens, this method will
            // be called again, and execution will proceed to the code below.
            if (!mFlags.Has(Flags::kAdvertisingConfigured))
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
        if (mFlags.Has(Flags::kAdvertising))
        {
            err = MapBLEError(esp_ble_gap_stop_advertising());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "esp_ble_gap_stop_advertising() failed: %s", ErrorStr(err));
                ExitNow();
            }

            mFlags.Set(Flags::kControlOpInProgress);

            ExitNow();
        }
    }

    // Stop the CHIPoBLE GATT service if needed.
    if (mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_Enabled && mFlags.Has(Flags::kGATTServiceStarted))
    {
        // TODO: what to do about existing connections??

        err = MapBLEError(esp_ble_gatts_stop_service(mServiceAttrHandle));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "esp_ble_gatts_stop_service() failed: %s", ErrorStr(err));
            ExitNow();
        }

        mFlags.Set(Flags::kControlOpInProgress);

        ExitNow();
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

CHIP_ERROR BLEManagerImpl::InitESPBleLayer(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(!mFlags.Has(Flags::kESPBLELayerInitialized), /* */);

    // If the ESP Bluetooth controller has not been initialized...
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE)
    {
        // Since Chip only uses BLE, release memory held by ESP classic bluetooth stack.
        err = MapBLEError(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "esp_bt_controller_mem_release() failed: %s", ErrorStr(err));
            ExitNow();
        }

        // Initialize the ESP Bluetooth controller.
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        err                               = MapBLEError(esp_bt_controller_init(&bt_cfg));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "esp_bt_controller_init() failed: %s", ErrorStr(err));
            ExitNow();
        }
    }

    // If the ESP Bluetooth controller has not been enabled, enable it now.
    if (esp_bt_controller_get_status() != ESP_BT_CONTROLLER_STATUS_ENABLED)
    {
        err = MapBLEError(esp_bt_controller_enable(ESP_BT_MODE_BLE));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "esp_bt_controller_enable() failed: %s", ErrorStr(err));
            ExitNow();
        }
    }

    // If the ESP Bluedroid stack has not been initialized, initialize it now.
    if (esp_bluedroid_get_status() == ESP_BLUEDROID_STATUS_UNINITIALIZED)
    {
        err = MapBLEError(esp_bluedroid_init());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "esp_bluedroid_init() failed: %s", ErrorStr(err));
            ExitNow();
        }
    }

    // If the ESP Bluedroid stack has not been enabled, enable it now.
    if (esp_bluedroid_get_status() != ESP_BLUEDROID_STATUS_ENABLED)
    {
        err = MapBLEError(esp_bluedroid_enable());
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "esp_bluedroid_enable() failed: %s", ErrorStr(err));
            ExitNow();
        }
    }

    // Register a callback to receive GATT events.
    err = MapBLEError(esp_ble_gatts_register_callback(HandleGATTEvent));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gatts_register_callback() failed: %s", ErrorStr(err));
        ExitNow();
    }

    // Register a callback to receive GAP events.
    err = MapBLEError(esp_ble_gap_register_callback(HandleGAPEvent));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gap_register_callback() failed: %s", ErrorStr(err));
        ExitNow();
    }

    // Set the maximum supported MTU size.
    err = MapBLEError(esp_ble_gatt_set_local_mtu(ESP_GATT_MAX_MTU_SIZE));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gatt_set_local_mtu() failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    mFlags.Set(Flags::kESPBLELayerInitialized);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    CHIP_ERROR err;
    uint8_t advData[MAX_ADV_DATA_LEN];
    uint8_t index = 0;

    // If a custom device name has not been specified, generate a CHIP-standard name based on the
    // discriminator value
    uint16_t discriminator;
    SuccessOrExit(err = ConfigurationMgr().GetSetupDiscriminator(discriminator));

    if (!mFlags.Has(Flags::kUseCustomDeviceName))
    {
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
        mDeviceName[kMaxDeviceNameLength] = 0;
    }

    // Configure the BLE device name.
    err = MapBLEError(esp_ble_gap_set_device_name(mDeviceName));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gap_set_device_name() failed: %s", ErrorStr(err));
        ExitNow();
    }

    memset(advData, 0, sizeof(advData));
    advData[index++] = 0x02;                            // length
    advData[index++] = CHIP_ADV_DATA_TYPE_FLAGS;        // AD type : flags
    advData[index++] = CHIP_ADV_DATA_FLAGS;             // AD value
    advData[index++] = 0x0A;                            // length
    advData[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA; // AD type: (Service Data - 16-bit UUID)
    advData[index++] = ShortUUID_CHIPoBLEService[0];    // AD value
    advData[index++] = ShortUUID_CHIPoBLEService[1];    // AD value

    chip::Ble::ChipBLEDeviceIdentificationInfo deviceIdInfo;
    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetBLEDeviceIdentificationInfo(): %s", ErrorStr(err));
        ExitNow();
    }

    VerifyOrExit(index + sizeof(deviceIdInfo) <= sizeof(advData), err = CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG);
    memcpy(&advData[index], &deviceIdInfo, sizeof(deviceIdInfo));
    index = static_cast<uint8_t>(index + sizeof(deviceIdInfo));

    // Construct the Chip BLE Service Data to be sent in the scan response packet.
    err = MapBLEError(esp_ble_gap_config_adv_data_raw(advData, sizeof(advData)));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gap_config_adv_data_raw(<raw_data>) failed: %s", ErrorStr(err));
        ExitNow();
    }

    mFlags.Set(Flags::kControlOpInProgress);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err;
    esp_ble_adv_params_t advertParams = {
        0,                                 // adv_int_min
        0,                                 // adv_int_max
        ADV_TYPE_IND,                      // adv_type
        BLE_ADDR_TYPE_PUBLIC,              // own_addr_type
        { 0 },                             // peer_addr
        BLE_ADDR_TYPE_RANDOM,              // peer_addr_type
        ADV_CHNL_ALL,                      // channel_map
        ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, // adv_filter_policy
    };

    advertParams.own_addr_type = BLE_ADDR_TYPE_RANDOM;

    // Advertise connectable if we haven't reached the maximum number of connections.
    size_t numCons        = NumConnections();
    bool connectable      = (numCons < kMaxConnections);
    advertParams.adv_type = connectable ? ADV_TYPE_IND : ADV_TYPE_NONCONN_IND;

    // Advertise in fast mode if it is connectable advertisement and
    // the application has expressly requested fast advertising.
    if (connectable && mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        advertParams.adv_int_min = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
        advertParams.adv_int_max = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
    }
    else
    {
        advertParams.adv_int_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        advertParams.adv_int_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
    }

    ChipLogProgress(DeviceLayer, "Configuring CHIPoBLE advertising (interval %" PRIu32 " ms, %sconnectable, device name %s)",
                    (((uint32_t) advertParams.adv_int_min) * 10) / 16, (connectable) ? "" : "non-", mDeviceName);

    err = MapBLEError(esp_ble_gap_start_advertising(&advertParams));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gap_start_advertising() failed: %s", ErrorStr(err));
        ExitNow();
    }

    mFlags.Set(Flags::kControlOpInProgress);

exit:
    return err;
}

void BLEManagerImpl::HandleGATTControlEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    bool controlOpComplete = false;

    // Ignore GATT control events that do not pertain to the CHIPoBLE application, except for ESP_GATTS_REG_EVT.
    if (event != ESP_GATTS_REG_EVT && (!mFlags.Has(Flags::kAppRegistered) || gatts_if != mAppIf))
    {
        ExitNow();
    }

    switch (event)
    {
    case ESP_GATTS_REG_EVT:

        if (param->reg.app_id == CHIPoBLEAppId)
        {
            if (param->reg.status != ESP_GATT_OK)
            {
                ChipLogError(DeviceLayer, "ESP_GATTS_REG_EVT error: %d", (int) param->reg.status);
                ExitNow(err = ESP_ERR_INVALID_RESPONSE);
            }

            // Save the 'interface type' assigned to the CHIPoBLE application by the ESP BLE layer.
            mAppIf = gatts_if;

            mFlags.Set(Flags::kAppRegistered);
            controlOpComplete = true;
        }
        esp_ble_gap_config_local_privacy(true);

        break;

    case ESP_GATTS_CREAT_ATTR_TAB_EVT:

        if (param->add_attr_tab.status != ESP_GATT_OK)
        {
            ChipLogError(DeviceLayer, "ESP_GATTS_CREAT_ATTR_TAB_EVT error: %d", (int) param->add_attr_tab.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        // Save the attribute handles assigned by the ESP BLE layer to the CHIPoBLE attributes.
        mServiceAttrHandle    = param->add_attr_tab.handles[kAttrIndex_ServiceDeclaration];
        mRXCharAttrHandle     = param->add_attr_tab.handles[kAttrIndex_RXCharValue];
        mTXCharAttrHandle     = param->add_attr_tab.handles[kAttrIndex_TXCharValue];
        mTXCharCCCDAttrHandle = param->add_attr_tab.handles[kAttrIndex_TXCharCCCDValue];

        mFlags.Set(Flags::kAttrsRegistered);
        controlOpComplete = true;

        break;

    case ESP_GATTS_START_EVT:

        if (param->start.status != ESP_GATT_OK)
        {
            ChipLogError(DeviceLayer, "ESP_GATTS_START_EVT error: %d", (int) param->start.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        ChipLogProgress(DeviceLayer, "CHIPoBLE GATT service started");

        mFlags.Set(Flags::kGATTServiceStarted);
        controlOpComplete = true;

        break;

    case ESP_GATTS_STOP_EVT:

        if (param->stop.status != ESP_GATT_OK)
        {
            ChipLogError(DeviceLayer, "ESP_GATTS_STOP_EVT error: %d", (int) param->stop.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        ChipLogProgress(DeviceLayer, "CHIPoBLE GATT service stopped");

        mFlags.Clear(Flags::kGATTServiceStarted);
        controlOpComplete = true;

        break;

    case ESP_GATTS_RESPONSE_EVT:
        ESP_LOGD(TAG, "ESP_GATTS_RESPONSE_EVT (handle %u, status %d)", param->rsp.handle, (int) param->rsp.status);
        break;

    default:
        // Ignore all other event types.
        break;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
    if (controlOpComplete)
    {
        mFlags.Clear(Flags::kControlOpInProgress);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

void BLEManagerImpl::HandleGATTCommEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param)
{
    // Ignore the event if the CHIPoBLE service hasn't been started, or if the event is for a different
    // BLE application.
    if (!sInstance.mFlags.Has(Flags::kGATTServiceStarted) || gatts_if != sInstance.mAppIf)
    {
        return;
    }

    switch (event)
    {
    case ESP_GATTS_CONNECT_EVT:
        ChipLogProgress(DeviceLayer, "BLE GATT connection established (con %u)", param->connect.conn_id);

        // Allocate a connection state record for the new connection.
        GetConnectionState(param->mtu.conn_id, true);

        // Receiving a connection stops the advertising processes.  So force a refresh of the advertising
        // state.
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);
        mFlags.Clear(Flags::kAdvertisingConfigured);
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

    case ESP_GATTS_CONF_EVT: {
        CHIPoBLEConState * conState = GetConnectionState(param->conf.conn_id);
        if (conState != NULL)
        {
            HandleTXCharConfirm(conState, param);
        }
    }
    break;

    case ESP_GATTS_MTU_EVT: {
        ESP_LOGD(TAG, "MTU for con %u: %u", param->mtu.conn_id, param->mtu.mtu);
        CHIPoBLEConState * conState = GetConnectionState(param->mtu.conn_id);
        if (conState != NULL)
        {
            // conState->MTU is a 10-bit field inside a uint16_t.  We're
            // assigning to it from a uint16_t, and compilers warn about
            // possibly not fitting.  There's no way to suppress that warning
            // via explicit cast; we have to disable the warning around the
            // assignment.
            //
            // TODO: https://github.com/project-chip/connectedhomeip/issues/2569
            // tracks making this safe with a check or explaining why no check
            // is needed.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
            conState->MTU = param->mtu.mtu;
#pragma GCC diagnostic pop
        }
    }
    break;

    default:
        break;
    }
}

void BLEManagerImpl::HandleRXCharWrite(esp_ble_gatts_cb_param_t * param)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool needResp  = param->write.need_rsp;
    PacketBufferHandle buf;

    ESP_LOGD(TAG, "Write request received for CHIPoBLE RX characteristic (con %u, len %u)", param->write.conn_id, param->write.len);

    // Disallow long writes.
    VerifyOrExit(param->write.is_prep == false, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Copy the data to a packet buffer.
    buf = System::PacketBufferHandle::NewWithData(param->write.value, param->write.len, 0, 0);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    // Send a response if requested.
    if (needResp)
    {
        esp_ble_gatts_send_response(mAppIf, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        needResp = false;
    }

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = param->write.conn_id;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
        if (needResp)
        {
            esp_ble_gatts_send_response(mAppIf, param->write.conn_id, param->write.trans_id, ESP_GATT_INTERNAL_ERROR, NULL);
        }
        // TODO: fail connection???
    }
}

void BLEManagerImpl::HandleTXCharRead(esp_ble_gatts_cb_param_t * param)
{
    CHIP_ERROR err;
    esp_gatt_rsp_t rsp;

    ESP_LOGD(TAG, "Read request received for CHIPoBLE TX characteristic (con %u)", param->read.conn_id);

    // Send a zero-length response.
    memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
    rsp.attr_value.handle = param->read.handle;
    err = MapBLEError(esp_ble_gatts_send_response(mAppIf, param->read.conn_id, param->read.trans_id, ESP_GATT_OK, &rsp));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gatts_send_response() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleTXCharCCCDRead(esp_ble_gatts_cb_param_t * param)
{
    CHIP_ERROR err;
    CHIPoBLEConState * conState;
    esp_gatt_rsp_t rsp;

    ESP_LOGD(TAG, "Read request received for CHIPoBLE TX characteristic CCCD (con %u)", param->read.conn_id);

    // Find the connection state record.
    conState = GetConnectionState(param->read.conn_id);

    // Send current CCCD value, or an error if we failed to allocate a connection state object.
    memset(&rsp, 0, sizeof(esp_gatt_rsp_t));
    rsp.attr_value.handle = param->read.handle;
    if (conState != NULL)
    {
        rsp.attr_value.len      = 2;
        rsp.attr_value.value[0] = conState->Subscribed ? 1 : 0;
    }
    err = MapBLEError(esp_ble_gatts_send_response(mAppIf, param->read.conn_id, param->read.trans_id,
                                                  (conState != NULL) ? ESP_GATT_OK : ESP_GATT_INTERNAL_ERROR, &rsp));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gatts_send_response() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(esp_ble_gatts_cb_param_t * param)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CHIPoBLEConState * conState;
    bool needResp = param->write.need_rsp;
    bool indicationsEnabled;

    ESP_LOGD(TAG, "Write request received for CHIPoBLE TX characteristic CCCD (con %u, len %u)", param->write.conn_id,
             param->write.len);

    // Find the connection state record.
    conState = GetConnectionState(param->read.conn_id);
    VerifyOrExit(conState != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Disallow long writes.
    VerifyOrExit(param->write.is_prep == false, err = CHIP_ERROR_INVALID_ARGUMENT);

    // Determine if the client is enabling or disabling indications.
    indicationsEnabled = (param->write.len > 0 && (param->write.value[0] != 0));

    // Send a response to the Write if requested.
    if (needResp)
    {
        esp_ble_gatts_send_response(mAppIf, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        needResp = false;
    }

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling indications.
    {
        ChipDeviceEvent event;
        event.Type = (indicationsEnabled) ? DeviceEventType::kCHIPoBLESubscribe : DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = param->write.conn_id;
        PlatformMgr().PostEvent(&event);
    }

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", indicationsEnabled ? "subscribe" : "unsubscribe");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
        if (needResp)
        {
            esp_ble_gatts_send_response(mAppIf, param->write.conn_id, param->write.trans_id, ESP_GATT_INTERNAL_ERROR, NULL);
        }
        // TODO: fail connection???
    }
}

void BLEManagerImpl::HandleTXCharConfirm(CHIPoBLEConState * conState, esp_ble_gatts_cb_param_t * param)
{
    ESP_LOGD(TAG, "Confirm received for CHIPoBLE TX characteristic indication (con %u, status %u)", param->conf.conn_id,
             param->conf.status);

    // If there is a pending indication buffer for the connection, release it now.
    conState->PendingIndBuf = nullptr;

    // If the confirmation was successful...
    if (param->conf.status == ESP_GATT_OK)
    {
        // Post an event to the Chip queue to process the indicate confirmation.
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
        event.CHIPoBLEIndicateConfirm.ConId = param->conf.conn_id;
        PlatformMgr().PostEvent(&event);
    }

    else
    {
        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId  = param->disconnect.conn_id;
        event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
        PlatformMgr().PostEvent(&event);
    }
}

void BLEManagerImpl::HandleDisconnect(esp_ble_gatts_cb_param_t * param)
{
    ChipLogProgress(DeviceLayer, "BLE GATT connection closed (con %u, reason %u)", param->disconnect.conn_id,
                    param->disconnect.reason);

    // If this was a CHIPoBLE connection, release the associated connection state record
    // and post an event to deliver a connection error to the CHIPoBLE layer.
    if (ReleaseConnectionState(param->disconnect.conn_id))
    {
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId = param->disconnect.conn_id;
        switch (param->disconnect.reason)
        {
        case ESP_GATT_CONN_TERMINATE_PEER_USER:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;
        case ESP_GATT_CONN_TERMINATE_LOCAL_HOST:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_APP_CLOSED_CONNECTION;
            break;
        default:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
            break;
        }
        PlatformMgr().PostEvent(&event);

        // Force a refresh of the advertising state.
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);
        mFlags.Clear(Flags::kAdvertisingConfigured);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

BLEManagerImpl::CHIPoBLEConState * BLEManagerImpl::GetConnectionState(uint16_t conId, bool allocate)
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
            mCons[freeIndex].Set(conId);
            return &mCons[freeIndex];
        }

        ChipLogError(DeviceLayer, "Failed to allocate CHIPoBLEConState");
    }

    return NULL;
}

bool BLEManagerImpl::ReleaseConnectionState(uint16_t conId)
{
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mCons[i].Allocated && mCons[i].ConId == conId)
        {
            mCons[i].Reset();
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
    ESP_LOGV(TAG, "GATT Event: %d (if %d)", (int) event, (int) gatts_if);

    // This method is invoked on the ESP BLE thread.  Therefore we must hold a lock
    // on the Chip stack while processing the event.
    PlatformMgr().LockChipStack();

    sInstance.HandleGATTControlEvent(event, gatts_if, param);
    sInstance.HandleGATTCommEvent(event, gatts_if, param);

    PlatformMgr().UnlockChipStack();
}

void BLEManagerImpl::HandleGAPEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t * param)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ESP_LOGV(TAG, "GAP Event: %d", (int) event);

    // This method is invoked on the ESP BLE thread.  Therefore we must hold a lock
    // on the Chip stack while processing the event.
    PlatformMgr().LockChipStack();

    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:

        if (param->adv_data_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ChipLogError(DeviceLayer, "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT error: %d", (int) param->adv_data_cmpl.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        sInstance.mFlags.Set(Flags::kAdvertisingConfigured);
        sInstance.mFlags.Clear(Flags::kControlOpInProgress);

        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:

        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ChipLogError(DeviceLayer, "ESP_GAP_BLE_ADV_START_COMPLETE_EVT error: %d", (int) param->adv_start_cmpl.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        sInstance.mFlags.Clear(Flags::kControlOpInProgress);
        sInstance.mFlags.Clear(Flags::kAdvertisingRefreshNeeded);

        // Transition to the Advertising state...
        if (!sInstance.mFlags.Has(Flags::kAdvertising))
        {
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising started");

            sInstance.mFlags.Set(Flags::kAdvertising);

            // Post a CHIPoBLEAdvertisingChange(Started) event.
            {
                ChipDeviceEvent advChange;
                advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
                advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Started;
                PlatformMgr().PostEvent(&advChange);
            }
        }

        break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:

        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ChipLogError(DeviceLayer, "ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT error: %d", (int) param->adv_stop_cmpl.status);
            ExitNow(err = ESP_ERR_INVALID_RESPONSE);
        }

        sInstance.mFlags.Clear(Flags::kControlOpInProgress);
        sInstance.mFlags.Clear(Flags::kAdvertisingRefreshNeeded);

        // Transition to the not Advertising state...
        if (sInstance.mFlags.Has(Flags::kAdvertising))
        {
            sInstance.mFlags.Clear(Flags::kAdvertising);
            sInstance.mFlags.Set(Flags::kFastAdvertisingEnabled, true);

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
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
    PlatformMgr().UnlockChipStack();
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CONFIG_BT_BLUEDROID_ENABLED

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
