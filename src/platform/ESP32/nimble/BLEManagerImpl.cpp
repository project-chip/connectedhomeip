/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
#include <ble/Ble.h>
#endif // CONFIG_ENABLE_ESP32_BLE_CONTROLLER

#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
#include <platform/ESP32/BLEManagerImpl.h>
#endif // CONFIG_ENABLE_ESP32_BLE_CONTROLLER

#include <platform/internal/BLEManager.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#include <system/SystemTimer.h>

#ifndef CONFIG_IDF_TARGET_ESP32P4
#include "esp_bt.h"
#endif
#include "esp_log.h"

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#include "esp_nimble_hci.h"
#endif

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
#include "blecent.h"
#endif // CONFIG_ENABLE_ESP32_BLE_CONTROLLER

#include "host/ble_hs.h"
#include "host/ble_hs_pvcy.h"
#include "host/ble_uuid.h"
#include "host/util/util.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

// Not declared in any header file, hence requires a forward declaration.
extern "C" void ble_store_config_init(void);

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

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
static constexpr uint16_t kNewConnectionScanTimeout = 60;
static constexpr uint16_t kConnectTimeout           = 20;
#endif // CONFIG_ENABLE_ESP32_BLE_CONTROLLER

struct ESP32ChipServiceData
{
    uint8_t ServiceUUID[2];
    ChipBLEDeviceIdentificationInfo DeviceIdInfo;
};

const ble_uuid16_t ShortUUID_CHIPoBLEService = { BLE_UUID_TYPE_16, 0xFFF6 };

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
const ble_uuid16_t ShortUUID_CHIPoBLE_CharTx_Desc = { BLE_UUID_TYPE_16, 0x2902 };
#endif // CONFIG_ENABLE_ESP32_BLE_CONTROLLER

const ble_uuid128_t UUID128_CHIPoBLEChar_RX = {
    BLE_UUID_TYPE_128, { 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 }
};

const ble_uuid128_t UUID_CHIPoBLEChar_TX = {
    { BLE_UUID_TYPE_128 }, { 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 }
};

#ifdef CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
const ble_uuid128_t UUID_CHIPoBLEChar_C3 = {
    { BLE_UUID_TYPE_128 }, { 0x04, 0x8F, 0x21, 0x83, 0x8A, 0x74, 0x7D, 0xB8, 0xF2, 0x45, 0x72, 0x87, 0x38, 0x02, 0x63, 0x64 }
};
#endif // CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
const struct ble_gatt_svc_def kNullSvc = { 0 };

SemaphoreHandle_t semaphoreHandle = NULL;

// LE Random Device Address
// (see Bluetooth® Core Specification 4.2 Vol 6, Part B, Section 1.3.2.1 "Static device address")
uint8_t own_addr_type = BLE_OWN_ADDR_RANDOM;

} // unnamed namespace

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
ChipDeviceScanner & mDeviceScanner = Internal::ChipDeviceScanner::GetInstance();
#endif
BLEManagerImpl BLEManagerImpl::sInstance;
const struct ble_gatt_svc_def BLEManagerImpl::CHIPoBLEGATTSvc = { .type            = BLE_GATT_SVC_TYPE_PRIMARY,
                                                                  .uuid            = (ble_uuid_t *) (&ShortUUID_CHIPoBLEService),
                                                                  .characteristics = (struct ble_gatt_chr_def[]){
                                                                      {
                                                                          .uuid       = (ble_uuid_t *) (&UUID128_CHIPoBLEChar_RX),
                                                                          .access_cb  = gatt_svr_chr_access,
                                                                          .flags      = BLE_GATT_CHR_F_WRITE,
                                                                          .val_handle = &sInstance.mRXCharAttrHandle,
                                                                      },
                                                                      {
                                                                          .uuid      = (ble_uuid_t *) (&UUID_CHIPoBLEChar_TX),
                                                                          .access_cb = gatt_svr_chr_access,
                                                                          .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_INDICATE,
                                                                          .val_handle = &sInstance.mTXCharCCCDAttrHandle,
                                                                      },
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
                                                                      {
                                                                          .uuid       = (ble_uuid_t *) (&UUID_CHIPoBLEChar_C3),
                                                                          .access_cb  = gatt_svr_chr_access_additional_data,
                                                                          .flags      = BLE_GATT_CHR_F_READ,
                                                                          .val_handle = &sInstance.mC3CharAttrHandle,
                                                                      },
#endif
                                                                      {
                                                                          0, /* No more characteristics in this service */
                                                                      },
                                                                  } };

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
void BLEManagerImpl::HandleConnectFailed(CHIP_ERROR error)
{
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event;
        event.Type                                    = DeviceEventType::kPlatformESP32BLECentralConnectFailed;
        event.Platform.BLECentralConnectFailed.mError = error;
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::CancelConnect(void)
{
    int rc = ble_gap_conn_cancel();
    VerifyOrReturn(rc == 0, ChipLogError(Ble, "Failed to cancel connection rc=%d", rc));
}

void BLEManagerImpl::HandleConnectTimeout(chip::System::Layer *, void * context)
{
    CancelConnect();
    BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_TIMEOUT);
}

void BLEManagerImpl::ConnectDevice(const ble_addr_t & addr, uint16_t timeout)
{
    int rc;
    uint8_t ownAddrType;

    rc = ble_hs_id_infer_auto(0, &ownAddrType);
    if (rc != 0)
    {
        ChipLogError(Ble, "Failed to infer own address type rc=%d", rc);
        return;
    }

    rc = ble_gap_connect(ownAddrType, &addr, (timeout * 1000), NULL, ble_svr_gap_event, NULL);
    if (rc != 0)
    {
        ChipLogError(Ble, "Failed to connect to rc=%d", rc);
    }
}
#endif

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    // Initialize the Chip BleLayer.
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    err = BleLayer::Init(this, this, this, &DeviceLayer::SystemLayer());
#else
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
#endif
    SuccessOrExit(err);

    mRXCharAttrHandle = 0;
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    mC3CharAttrHandle = 0;
#endif
    mTXCharCCCDAttrHandle = 0;
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART && !mIsCentral);
    mFlags.Set(Flags::kFastAdvertisingEnabled, !mIsCentral);
#else
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);
#endif

    mNumGAPCons = 0;
    memset(reinterpret_cast<void *>(mCons), 0, sizeof(mCons));
    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    memset(mDeviceName, 0, sizeof(mDeviceName));

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

void BLEManagerImpl::_Shutdown()
{
    if (mFlags.Has(Flags::kBleDeinitAndMemReleased))
    {
        ChipLogProgress(DeviceLayer, "Ble already deinitialized, returning from ShutDown flow");
        return;
    }

    CancelBleAdvTimeoutTimer();

    BleLayer::Shutdown();

    // selectively setting kGATTServiceStarted flag, in order to notify the state machine to stop the CHIPoBLE GATT service
    mFlags.ClearAll().Set(Flags::kGATTServiceStarted);
    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;

    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (val)
    {
        StartBleAdvTimeoutTimer(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
    }

    mFlags.Set(Flags::kFastAdvertisingEnabled, val);
    mFlags.Set(Flags::kAdvertisingRefreshNeeded, 1);
    mFlags.Set(Flags::kAdvertisingEnabled, val);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

void BLEManagerImpl::BleAdvTimeoutHandler(System::Layer *, void *)
{
    if (BLEMgrImpl().mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        ChipLogProgress(DeviceLayer, "bleAdv Timeout : Start slow advertisement");
        BLEMgrImpl().mFlags.Set(Flags::kFastAdvertisingEnabled, 0);
        BLEMgrImpl().mFlags.Set(Flags::kAdvertisingRefreshNeeded, 1);

#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
        BLEMgrImpl().mFlags.Clear(Flags::kExtAdvertisingEnabled);
        BLEMgrImpl().StartBleAdvTimeoutTimer(CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_CHANGE_TIME_MS);
#endif
    }
#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
    else
    {
        ChipLogProgress(DeviceLayer, "bleAdv Timeout : Start extended advertisement");
        BLEMgrImpl().mFlags.Set(Flags::kAdvertising);
        BLEMgrImpl().mFlags.Set(Flags::kExtAdvertisingEnabled);
        BLEMgr().SetAdvertisingMode(BLEAdvertisingMode::kSlowAdvertising);
        BLEMgrImpl().mFlags.Set(Flags::kAdvertisingRefreshNeeded, 1);
    }
#endif
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    switch (mode)
    {
    case BLEAdvertisingMode::kFastAdvertising:
        mFlags.Set(Flags::kFastAdvertisingEnabled, true);
        break;
    case BLEAdvertisingMode::kSlowAdvertising:
        mFlags.Set(Flags::kFastAdvertisingEnabled, false);
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);
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

exit:
    return err;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe:
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        {
            ChipDeviceEvent connectionEvent;
            connectionEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
            PlatformMgr().PostEventOrDie(&connectionEvent);
        }
        break;

    case DeviceEventType::kCHIPoBLEUnsubscribe:
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        break;

    case DeviceEventType::kCHIPoBLEWriteReceived:
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
        break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm:
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        break;

    case DeviceEventType::kCHIPoBLEConnectionError:
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
        break;

    case DeviceEventType::kServiceProvisioningChange:
    case DeviceEventType::kWiFiConnectivityChange:
        // Force the advertising configuration to be refreshed to reflect new provisioning state.
        ChipLogProgress(DeviceLayer, "Updating advertising data");
        mFlags.Clear(Flags::kAdvertisingConfigured);
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);

        DriveBLEState();
        break;

    default:
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
        HandlePlatformSpecificBLEEvent(event);
#endif
        break;
    }
}

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
void BLEManagerImpl::HandlePlatformSpecificBLEEvent(const ChipDeviceEvent * apEvent)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(DeviceLayer, "HandlePlatformSpecificBLEEvent %d", apEvent->Type);

    switch (apEvent->Type)
    {
    case DeviceEventType::kPlatformESP32BLECentralConnected:
        if (BLEManagerImpl::mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        {
            BleConnectionDelegate::OnConnectionComplete(mBLEScanConfig.mAppState,
                                                        apEvent->Platform.BLECentralConnected.mConnection);
            CleanScanConfig();
        }
        break;

    case DeviceEventType::kPlatformESP32BLECentralConnectFailed:
        if (BLEManagerImpl::mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        {
            BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, apEvent->Platform.BLECentralConnectFailed.mError);
            CleanScanConfig();
        }
        break;

    case DeviceEventType::kPlatformESP32BLEWriteComplete:
        HandleWriteConfirmation(apEvent->Platform.BLEWriteComplete.mConnection, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID);
        break;

    case DeviceEventType::kPlatformESP32BLESubscribeOpComplete:
        if (apEvent->Platform.BLESubscribeOpComplete.mIsSubscribed)
            HandleSubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &CHIP_BLE_SVC_ID,
                                    &Ble::CHIP_BLE_CHAR_2_UUID);
        else
            HandleUnsubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &CHIP_BLE_SVC_ID,
                                      &Ble::CHIP_BLE_CHAR_2_UUID);
        break;

    case DeviceEventType::kPlatformESP32BLEIndicationReceived:
        HandleIndicationReceived(apEvent->Platform.BLEIndicationReceived.mConnection, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID,
                                 PacketBufferHandle::Adopt(apEvent->Platform.BLEIndicationReceived.mData));
        break;

    default:
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %" CHIP_ERROR_FORMAT, err.Format());
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

static int OnUnsubscribeCharComplete(uint16_t conn_handle, const struct ble_gatt_error * error, struct ble_gatt_attr * attr,
                                     void * arg)
{
    ChipLogProgress(DeviceLayer, "Subscribe complete: conn_handle=%d, error=%d, attr_handle=%d", conn_handle, error->status,
                    attr->handle);

    ChipDeviceEvent event;
    event.Type                                          = DeviceEventType::kPlatformESP32BLESubscribeOpComplete;
    event.Platform.BLESubscribeOpComplete.mConnection   = conn_handle;
    event.Platform.BLESubscribeOpComplete.mIsSubscribed = false;
    PlatformMgr().PostEventOrDie(&event);

    return 0;
}

static int OnSubscribeCharComplete(uint16_t conn_handle, const struct ble_gatt_error * error, struct ble_gatt_attr * attr,
                                   void * arg)
{
    ChipLogProgress(DeviceLayer, "Subscribe complete: conn_handle=%d, error=%d, attr_handle=%d", conn_handle, error->status,
                    attr->handle);

    ChipDeviceEvent event;
    event.Type                                          = DeviceEventType::kPlatformESP32BLESubscribeOpComplete;
    event.Platform.BLESubscribeOpComplete.mConnection   = conn_handle;
    event.Platform.BLESubscribeOpComplete.mIsSubscribed = true;
    PlatformMgr().PostEventOrDie(&event);

    return 0;
}
#endif

CHIP_ERROR BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                   const ChipBleUUID * charId)
{
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    const struct peer_dsc * dsc;
    uint8_t value[2];
    int rc;
    struct peer * peer = peer_find(conId);
    if (peer == nullptr)
    {
        return BLE_ERROR_GATT_SUBSCRIBE_FAILED;
    }

    dsc = peer_dsc_find_uuid(peer, (ble_uuid_t *) (&ShortUUID_CHIPoBLEService), (ble_uuid_t *) (&UUID_CHIPoBLEChar_TX),
                             (ble_uuid_t *) (&ShortUUID_CHIPoBLE_CharTx_Desc));

    if (dsc == nullptr)
    {
        ChipLogError(Ble, "Peer does not have CCCD");
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return BLE_ERROR_GATT_SUBSCRIBE_FAILED;
    }

    value[0] = 0x02;
    value[1] = 0x00;

    rc = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle, value, sizeof(value), OnSubscribeCharComplete, NULL);
    if (rc != 0)
    {
        ChipLogError(Ble, "ble_gattc_write_flat failed: %d", rc);
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return BLE_ERROR_GATT_SUBSCRIBE_FAILED;
    }
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                     const ChipBleUUID * charId)
{
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    const struct peer_dsc * dsc;
    uint8_t value[2];
    int rc;
    struct peer * peer = peer_find(conId);
    if (peer == nullptr)
    {
        return BLE_ERROR_GATT_UNSUBSCRIBE_FAILED;
    }

    dsc = peer_dsc_find_uuid(peer, (ble_uuid_t *) (&ShortUUID_CHIPoBLEService), (ble_uuid_t *) (&UUID_CHIPoBLEChar_TX),
                             (ble_uuid_t *) (&ShortUUID_CHIPoBLE_CharTx_Desc));

    if (dsc == nullptr)
    {
        ChipLogError(Ble, "Peer does not have CCCD");
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return BLE_ERROR_GATT_UNSUBSCRIBE_FAILED;
    }

    value[0] = 0x00;
    value[1] = 0x00;

    rc = ble_gattc_write_flat(peer->conn_handle, dsc->dsc.handle, value, sizeof(value), OnUnsubscribeCharComplete, NULL);
    if (rc != 0)
    {
        ChipLogError(Ble, "ble_gattc_write_flat failed: %d", rc);
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return BLE_ERROR_GATT_UNSUBSCRIBE_FAILED;
    }
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    CHIP_ERROR err;

    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    // Signal the ESP BLE layer to close the conneecction.
    err = MapBLEError(ble_gap_terminate(conId, BLE_ERR_REM_USER_CONN_TERM));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ble_gap_terminate() failed: %" CHIP_ERROR_FORMAT, err.Format());
    }

#ifndef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    // Force a refresh of the advertising state.
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    mFlags.Clear(Flags::kAdvertisingConfigured);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
#endif

    return err;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return ble_att_mtu(conId);
}

CHIP_ERROR BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                          chip::System::PacketBufferHandle data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    struct os_mbuf * om;

    VerifyOrExit(IsSubscribed(conId), err = CHIP_ERROR_INVALID_ARGUMENT);

    ESP_LOGD(TAG, "Sending indication for CHIPoBLE TX characteristic (con %u, len %u)", conId, data->DataLength());

    // For BLE, the buffer is capped at UINT16_MAX. Nevertheless, have a verify
    // check before the cast to uint16_t.
    VerifyOrExit(CanCastTo<uint16_t>(data->DataLength()), err = CHIP_ERROR_MESSAGE_TOO_LONG);

    om = ble_hs_mbuf_from_flat(data->Start(), static_cast<uint16_t>(data->DataLength()));
    if (om == NULL)
    {
        ChipLogError(DeviceLayer, "ble_hs_mbuf_from_flat failed:");
        err = CHIP_ERROR_NO_MEMORY;
        ExitNow();
    }

    err = MapBLEError(ble_gattc_indicate_custom(conId, mTXCharCCCDAttrHandle, om));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ble_gattc_indicate_custom() failed: %" CHIP_ERROR_FORMAT, err.Format());
        ExitNow();
    }

exit:
    return err;
}

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
static int OnWriteComplete(uint16_t conn_handle, const struct ble_gatt_error * error, struct ble_gatt_attr * attr, void * arg)
{
    ChipLogDetail(Ble, "Write complete; status:%d conn_handle:%d attr_handle:%d", error->status, conn_handle, attr->handle);
    ChipDeviceEvent event;
    event.Type                                  = DeviceEventType::kPlatformESP32BLEWriteComplete;
    event.Platform.BLEWriteComplete.mConnection = conn_handle;
    CHIP_ERROR err                              = PlatformMgr().PostEvent(&event);
    if (err != CHIP_NO_ERROR)
    {
        return 1;
    }

    return 0;
}
#endif

CHIP_ERROR BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                            chip::System::PacketBufferHandle pBuf)
{
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    const struct peer_chr * chr;
    int rc;
    const struct peer * peer = peer_find(conId);

    chr = peer_chr_find_uuid(peer, (ble_uuid_t *) (&ShortUUID_CHIPoBLEService), (ble_uuid_t *) (&UUID128_CHIPoBLEChar_RX));
    if (chr == nullptr)
    {
        ChipLogError(Ble, "Peer does not have RX characteristic");
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return BLE_ERROR_GATT_WRITE_FAILED;
    }

    if (pBuf->DataLength() > UINT16_MAX)
    {
        ChipLogError(Ble, "Buffer data Length is too long");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    rc = ble_gattc_write_flat(conId, chr->chr.val_handle, pBuf->Start(), static_cast<uint16_t>(pBuf->DataLength()), OnWriteComplete,
                              this);
    if (rc != 0)
    {
        ChipLogError(Ble, "ble_gattc_write_flat failed: %d", rc);
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return BLE_ERROR_GATT_WRITE_FAILED;
    }
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    ChipLogDetail(Ble, "Received notification of closed CHIPoBLE connection (con %u)", conId);
    CloseConnection(conId);
}

CHIP_ERROR BLEManagerImpl::MapBLEError(int bleErr)
{
    switch (bleErr)
    {
    case ESP_OK:
        return CHIP_NO_ERROR;
    case BLE_HS_EMSGSIZE:
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    case BLE_HS_ENOMEM:
    case ESP_ERR_NO_MEM:
        return CHIP_ERROR_NO_MEMORY;
    case BLE_HS_ENOTCONN:
        return CHIP_ERROR_NOT_CONNECTED;
    case BLE_HS_ENOTSUP:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    case BLE_HS_EAPP:
        return CHIP_ERROR_INCORRECT_STATE;
    case BLE_HS_EBADDATA:
        return CHIP_ERROR_DATA_NOT_ALIGNED;
    case BLE_HS_ETIMEOUT:
        return CHIP_ERROR_TIMEOUT;
    case BLE_HS_ENOADDR:
        return CHIP_ERROR_INVALID_ADDRESS;
    case ESP_ERR_INVALID_ARG:
        return CHIP_ERROR_INVALID_ARGUMENT;
    default:
        return CHIP_ERROR(ChipError::Range::kPlatform, CHIP_DEVICE_CONFIG_ESP32_BLE_ERROR_MIN + bleErr);
    }
}

void BLEManagerImpl::CancelBleAdvTimeoutTimer(void)
{
    if (SystemLayer().IsTimerActive(BleAdvTimeoutHandler, nullptr))
    {
        SystemLayer().CancelTimer(BleAdvTimeoutHandler, nullptr);
    }
}

void BLEManagerImpl::StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs)
{
    CancelBleAdvTimeoutTimer();

    CHIP_ERROR err = SystemLayer().StartTimer(System::Clock::Milliseconds32(aTimeoutInMs), BleAdvTimeoutHandler, nullptr);
    if ((err != CHIP_NO_ERROR))
    {
        ChipLogError(DeviceLayer, "Failed to start BledAdv timeout timer");
    }
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the Chip task is running.
    if (!mFlags.Has(Flags::kAsyncInitCompleted))
    {
        mFlags.Set(Flags::kAsyncInitCompleted);
    }

    if (mFlags.Has(Flags::kBleDeinitAndMemReleased))
    {
        return;
    }

    // Initializes the ESP BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !mFlags.Has(Flags::kESPBLELayerInitialized))
    {
        err = InitESPBleLayer();
        SuccessOrExit(err);

        // Add delay of 500msec while NimBLE host task gets up and running
        {
            vTaskDelay(500 / portTICK_PERIOD_MS);
        }
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
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "Configure Adv Data failed: %" CHIP_ERROR_FORMAT, err.Format());
                    ExitNow();
                }
            }

            // Start advertising.  This is also an asynchronous step.
            ESP_LOGD(TAG, "NimBLE start advertising...");
            err = StartAdvertising();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Start advertising failed: %" CHIP_ERROR_FORMAT, err.Format());
                ExitNow();
            }

            mFlags.Clear(Flags::kAdvertisingRefreshNeeded);
            // Transition to the Advertising state...
            if (!mFlags.Has(Flags::kAdvertising))
            {
                ChipLogProgress(DeviceLayer, "CHIPoBLE advertising started");

                mFlags.Set(Flags::kAdvertising);

                // Post a CHIPoBLEAdvertisingChange(Started) event.
                {
                    ChipDeviceEvent advChange;
                    advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
                    advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Started;
                    err                                        = PlatformMgr().PostEvent(&advChange);
                }
            }
        }
    }

    // Otherwise stop advertising if needed...
    else
    {
        if (mFlags.Has(Flags::kAdvertising))
        {
#ifdef CONFIG_BT_NIMBLE_EXT_ADV
            if (ble_gap_ext_adv_active(kMatterAdvInstance))
            {
                err = MapBLEError(ble_gap_ext_adv_stop(kMatterAdvInstance));
#else
            if (ble_gap_adv_active())
            {
                err = MapBLEError(ble_gap_adv_stop());
#endif // CONFIG_BT_NIMBLE_EXT_ADV
                if (err != CHIP_NO_ERROR)
                {
                    ChipLogError(DeviceLayer, "BLE advertising stop failed: %" CHIP_ERROR_FORMAT, err.Format());
                    ExitNow();
                }
            }

            // Transition to the not Advertising state...
            mFlags.Clear(Flags::kAdvertising);
            mFlags.Set(Flags::kFastAdvertisingEnabled, true);

            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");

            CancelBleAdvTimeoutTimer();

            // Post a CHIPoBLEAdvertisingChange(Stopped) event.
            {
                ChipDeviceEvent advChange;
                advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
                advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
                err                                        = PlatformMgr().PostEvent(&advChange);
            }

            ExitNow();
        }
    }

    // Stop the CHIPoBLE GATT service if needed.
    if (mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_Enabled && mFlags.Has(Flags::kGATTServiceStarted))
    {
        DeinitESPBleLayer();
        mFlags.ClearAll().Set(Flags::kBleDeinitAndMemReleased);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %" CHIP_ERROR_FORMAT, err.Format());
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

void BLEManagerImpl::bleprph_on_reset(int reason)
{
    ESP_LOGE(TAG, "Resetting state; reason=%d\n", reason);
}

CHIP_ERROR BLEManagerImpl::bleprph_set_random_addr(void)
{
    ble_addr_t addr;

    // Generates a new static random address
    int rc = ble_hs_id_gen_rnd(0, &addr);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to generate random address err: %d", rc);
        return CHIP_ERROR_INTERNAL;
    }
    // Set generated address
    rc = ble_hs_id_set_rnd(addr.val);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to set random address err: %d", rc);
        return CHIP_ERROR_INTERNAL;
    }
    // Try to configure the device with random static address
    rc = ble_hs_util_ensure_addr(1);
    if (rc != 0)
    {
        ESP_LOGE(TAG, "Failed to configure random address err: %d", rc);
        return CHIP_ERROR_INTERNAL;
    }
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::bleprph_on_sync(void)
{
    ESP_LOGI(TAG, "BLE host-controller synced");
    xSemaphoreGive(semaphoreHandle);
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

    VerifyOrExit(!mFlags.Has(Flags::kESPBLELayerInitialized), /* */);

    semaphoreHandle = xSemaphoreCreateBinary();
    if (semaphoreHandle == NULL)
    {
        err = CHIP_ERROR_NO_MEMORY;
        ESP_LOGE(TAG, "Failed to create semaphore");
        ExitNow();
    }

    for (int i = 0; i < kMaxConnections; i++)
    {
        mSubscribedConIds[i] = BLE_CONNECTION_UNINITIALIZED;
    }
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    err = MapBLEError(esp_nimble_hci_and_controller_init());
    SuccessOrExit(err);
#endif

// For ESP-IDF 5.0.1 and below, nimble_port_init() returns void
#if ESP_IDF_VERSION <= ESP_IDF_VERSION_VAL(5, 0, 1)
    nimble_port_init();
#else
    err = MapBLEError(nimble_port_init());
    SuccessOrExit(err);
#endif

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb          = bleprph_on_reset;
    ble_hs_cfg.sync_cb           = bleprph_on_sync;
    ble_hs_cfg.store_status_cb   = ble_store_util_status_rr;
    ble_hs_cfg.sm_bonding        = 1;
    ble_hs_cfg.sm_our_key_dist   = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;
    ble_hs_cfg.sm_their_key_dist = BLE_SM_PAIR_KEY_DIST_ENC | BLE_SM_PAIR_KEY_DIST_ID;

    // Register the CHIPoBLE GATT attributes with the ESP BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled)
    {
        ble_svc_gap_init();
        ble_svc_gatt_init();

        if (mGattSvcs.size() == 0)
        {
            mGattSvcs.push_back(CHIPoBLEGATTSvc);
            mGattSvcs.push_back(kNullSvc);
        }
        err = MapBLEError(ble_gatts_count_cfg(&mGattSvcs[0]));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ble_gatts_count_cfg failed: %" CHIP_ERROR_FORMAT, err.Format());
            ExitNow();
        }

        err = MapBLEError(ble_gatts_add_svcs(&mGattSvcs[0]));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ble_gatts_add_svcs failed: %" CHIP_ERROR_FORMAT, err.Format());
            ExitNow();
        }
    }

    ble_store_config_init();
    nimble_port_freertos_init(bleprph_host_task);

    xSemaphoreTake(semaphoreHandle, portMAX_DELAY);
    vSemaphoreDelete(semaphoreHandle);
    semaphoreHandle = NULL;

    sInstance.mFlags.Set(Flags::kESPBLELayerInitialized);
    sInstance.mFlags.Set(Flags::kGATTServiceStarted);

    err = bleprph_set_random_addr();
exit:
    return err;
}

void BLEManagerImpl::DeinitESPBleLayer()
{
    VerifyOrReturn(DeinitBLE() == CHIP_NO_ERROR);
#ifdef CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING
    BLEManagerImpl::ClaimBLEMemory(nullptr, nullptr);
#endif /* CONFIG_USE_BLE_ONLY_FOR_COMMISSIONING */
}

void BLEManagerImpl::ClaimBLEMemory(System::Layer *, void *)
{
    TaskHandle_t handle = xTaskGetHandle("nimble_host");
    if (handle)
    {
        ChipLogDetail(DeviceLayer, "Schedule ble memory reclaiming since nimble host is still running");

        // Rescheduling it for later, 2 seconds is an arbitrary value, keeping it a bit more so that
        // we dont have to reschedule it again
        SystemLayer().StartTimer(System::Clock::Seconds32(2), ClaimBLEMemory, nullptr);
    }
    else
    {
        // Free up all the space occupied by ble and add it to heap
        esp_err_t err = ESP_OK;

#ifdef CONFIG_IDF_TARGET_ESP32
        err = esp_bt_mem_release(ESP_BT_MODE_BTDM);
#elif defined(CONFIG_IDF_TARGET_ESP32C2) || defined(CONFIG_IDF_TARGET_ESP32C3) || defined(CONFIG_IDF_TARGET_ESP32S3) ||            \
    defined(CONFIG_IDF_TARGET_ESP32H2) || defined(CONFIG_IDF_TARGET_ESP32C6)
        err = esp_bt_mem_release(ESP_BT_MODE_BLE);
#endif

        VerifyOrReturn(err == ESP_OK, ChipLogError(DeviceLayer, "BLE deinit failed"));
        ChipLogProgress(DeviceLayer, "BLE deinit successful and memory reclaimed");

        ChipDeviceEvent event;
        event.Type = DeviceEventType::kBLEDeinitialized;
        VerifyOrDo(CHIP_NO_ERROR == PlatformMgr().PostEvent(&event), ChipLogError(DeviceLayer, "Failed to post BLE deinit event"));
    }
}

CHIP_ERROR BLEManagerImpl::DeinitBLE()
{
    esp_err_t err = ESP_OK;
    VerifyOrReturnError(ble_hs_is_enabled(), CHIP_ERROR_INCORRECT_STATE, ChipLogProgress(DeviceLayer, "BLE already deinited"));
    VerifyOrReturnError(0 == nimble_port_stop(), MapBLEError(ESP_FAIL), ChipLogError(DeviceLayer, "nimble_port_stop() failed"));

    nimble_port_deinit();

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    err = esp_nimble_hci_and_controller_deinit();
#endif

    return MapBLEError(err);
}

#ifdef CONFIG_IDF_TARGET_ESP32P4
// Stub function to avoid link error
extern "C" void ble_transport_ll_deinit(void) {}
#endif

#ifdef CONFIG_BT_NIMBLE_EXT_ADV
CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    CHIP_ERROR err;
    uint8_t index = 0;

    constexpr uint8_t kServiceDataTypeSize = 1;

    chip::Ble::ChipBLEDeviceIdentificationInfo deviceIdInfo;

    // If a custom device name has not been specified, generate a CHIP-standard name based on the
    // bottom digits of the Chip device id.
    uint16_t discriminator;
    SuccessOrExit(err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));

    if (!mFlags.Has(Flags::kUseCustomDeviceName))
    {
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
        mDeviceName[kMaxDeviceNameLength] = 0;
    }

    // Configure the BLE device name.
    err = MapBLEError(ble_svc_gap_device_name_set(mDeviceName));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ble_svc_gap_device_name_set() failed: %" CHIP_ERROR_FORMAT, err.Format());
        ExitNow();
    }

    memset(mMatterAdvData, 0, sizeof(mMatterAdvData));
    mMatterAdvData[index++] = 0x02;                                                         // length
    mMatterAdvData[index++] = CHIP_ADV_DATA_TYPE_FLAGS;                                     // AD type : flags
    mMatterAdvData[index++] = CHIP_ADV_DATA_FLAGS;                                          // AD value
    mMatterAdvData[index++] = kServiceDataTypeSize + sizeof(ESP32ChipServiceData);          // length
    mMatterAdvData[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;                              // AD type: (Service Data - 16-bit UUID)
    mMatterAdvData[index++] = static_cast<uint8_t>(ShortUUID_CHIPoBLEService.value & 0xFF); // AD value
    mMatterAdvData[index++] = static_cast<uint8_t>((ShortUUID_CHIPoBLEService.value >> 8) & 0xFF); // AD value

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetBLEDeviceIdentificationInfo(): %" CHIP_ERROR_FORMAT, err.Format());
        ExitNow();
    }

#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
    // Check for extended advertisement interval and redact VID/PID if past the initial period.
    if (mFlags.Has(Flags::kExtAdvertisingEnabled))
    {
        deviceIdInfo.SetVendorId(0);
        deviceIdInfo.SetProductId(0);
        deviceIdInfo.SetExtendedAnnouncementFlag(true);
    }
#endif

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    if (!mFlags.Has(Flags::kExtAdvertisingEnabled))
    {
        deviceIdInfo.SetAdditionalDataFlag(true);
    }
    else
    {
        deviceIdInfo.SetAdditionalDataFlag(false);
    }
#endif

    VerifyOrExit(index + sizeof(deviceIdInfo) <= sizeof(mMatterAdvData), err = CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG);
    memcpy(&mMatterAdvData[index], &deviceIdInfo, sizeof(deviceIdInfo));
    index = static_cast<uint8_t>(index + sizeof(deviceIdInfo));

    mMatterAdvDataLen = index;
exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err;
    ble_gap_ext_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.scannable     = 1;
    adv_params.own_addr_type = own_addr_type;
    adv_params.legacy_pdu    = 1;

    mFlags.Clear(Flags::kAdvertisingRefreshNeeded);

    // Advertise connectable if we haven't reached the maximum number of connections.
    size_t numCons         = _NumConnections();
    bool connectable       = (numCons < kMaxConnections);
    adv_params.connectable = connectable;

    // Advertise in fast mode if it is connectable advertisement and
    // the application has expressly requested fast advertising.
    if (connectable && mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        adv_params.itvl_min = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
        adv_params.itvl_max = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
    }
    else
    {
#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
        if (!mFlags.Has(Flags::kExtAdvertisingEnabled))
        {
            adv_params.itvl_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
            adv_params.itvl_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
        }
        else
        {
            adv_params.itvl_min = CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MIN;
            adv_params.itvl_max = CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MAX;
        }
#else

        adv_params.itvl_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        adv_params.itvl_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;

#endif
    }

    ChipLogProgress(DeviceLayer, "Configuring CHIPoBLE advertising (interval %" PRIu32 " ms, %sconnectable)",
                    (((uint32_t) adv_params.itvl_min) * 10) / 16, (connectable) ? "" : "non-");

    {
        if (ble_gap_ext_adv_active(kMatterAdvInstance))
        {
            /* Advertising is already active. Stop and restart with the new parameters */
            ChipLogProgress(DeviceLayer, "Device already advertising, stop active advertisement and restart");
            err = MapBLEError(ble_gap_ext_adv_stop(kMatterAdvInstance));
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "ble_gap_ext_adv_stop() failed: %" CHIP_ERROR_FORMAT ", cannot restart", err.Format());
                return err;
            }
        }
        err = MapBLEError(ble_gap_ext_adv_configure(kMatterAdvInstance, &adv_params, NULL, ble_svr_gap_event, NULL));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ble_gap_ext_adv_configure failed: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }

        ble_addr_t addr;

        /* generate new non-resolvable private address */
        err = MapBLEError(ble_hs_id_gen_rnd(1, &addr));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ble_hs_id_gen_rnd failed: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }

        /* Set generated address */
        err = MapBLEError(ble_gap_ext_adv_set_addr(kMatterAdvInstance, &addr));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ble_gap_ext_adv_set_addr failed: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }

        struct os_mbuf * data = os_msys_get_pkthdr(mMatterAdvDataLen, 0);
        assert(data);
        err = MapBLEError(os_mbuf_append(data, mMatterAdvData, mMatterAdvDataLen));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "os_mbuf_append failed: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }

        err = MapBLEError(ble_gap_ext_adv_set_data(kMatterAdvInstance, data));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "os_mbuf_append failed: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }

        if (mScanResponse.HasValue())
        {
            uint16_t resp_size         = static_cast<uint16_t>(mScanResponse.Value().size());
            struct os_mbuf * resp_data = os_msys_get_pkthdr(resp_size, 0);
            assert(resp_data);
            err = MapBLEError(os_mbuf_append(resp_data, mScanResponse.Value().data(), resp_size));
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "os_mbuf_append failed: %" CHIP_ERROR_FORMAT, err.Format());
                return err;
            }
            err = MapBLEError(ble_gap_ext_adv_rsp_set_data(kMatterAdvInstance, resp_data));
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "ble_gap_ext_adv_rsp_set_data failed: %" CHIP_ERROR_FORMAT, err.Format());
                return err;
            }
        }
        err = MapBLEError(ble_gap_ext_adv_start(kMatterAdvInstance, 0, 0));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "ble_gap_ext_adv_start() failed: %" CHIP_ERROR_FORMAT, err.Format());
        }
    }
    return err;
}
#else
CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    CHIP_ERROR err;
    uint8_t advData[MAX_ADV_DATA_LEN];
    uint8_t index = 0;

    constexpr uint8_t kServiceDataTypeSize = 1;

    chip::Ble::ChipBLEDeviceIdentificationInfo deviceIdInfo;

    // If a custom device name has not been specified, generate a CHIP-standard name based on the
    // bottom digits of the Chip device id.
    uint16_t discriminator;
    SuccessOrExit(err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));

    if (!mFlags.Has(Flags::kUseCustomDeviceName))
    {
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
        mDeviceName[kMaxDeviceNameLength] = 0;
    }

    // Configure the BLE device name.
    err = MapBLEError(ble_svc_gap_device_name_set(mDeviceName));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ble_svc_gap_device_name_set() failed: %" CHIP_ERROR_FORMAT, err.Format());
        ExitNow();
    }

    memset(advData, 0, sizeof(advData));
    advData[index++] = 0x02;                                                                // length
    advData[index++] = CHIP_ADV_DATA_TYPE_FLAGS;                                            // AD type : flags
    advData[index++] = CHIP_ADV_DATA_FLAGS;                                                 // AD value
    advData[index++] = kServiceDataTypeSize + sizeof(ESP32ChipServiceData);                 // length
    advData[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;                                     // AD type: (Service Data - 16-bit UUID)
    advData[index++] = static_cast<uint8_t>(ShortUUID_CHIPoBLEService.value & 0xFF);        // AD value
    advData[index++] = static_cast<uint8_t>((ShortUUID_CHIPoBLEService.value >> 8) & 0xFF); // AD value

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetBLEDeviceIdentificationInfo(): %" CHIP_ERROR_FORMAT, err.Format());
        ExitNow();
    }

#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
    // Check for extended advertisement interval and redact VID/PID if past the initial period.
    if (mFlags.Has(Flags::kExtAdvertisingEnabled))
    {
        deviceIdInfo.SetVendorId(0);
        deviceIdInfo.SetProductId(0);
        deviceIdInfo.SetExtendedAnnouncementFlag(true);
    }
#endif

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    if (!mFlags.Has(Flags::kExtAdvertisingEnabled))
    {
        deviceIdInfo.SetAdditionalDataFlag(true);
    }
    else
    {
        deviceIdInfo.SetAdditionalDataFlag(false);
    }
#endif

    VerifyOrExit(index + sizeof(deviceIdInfo) <= sizeof(advData), err = CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG);
    memcpy(&advData[index], &deviceIdInfo, sizeof(deviceIdInfo));
    index = static_cast<uint8_t>(index + sizeof(deviceIdInfo));

    // Construct the Chip BLE Service Data to be sent in the scan response packet.
    err = MapBLEError(ble_gap_adv_set_data(advData, sizeof(advData)));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "ble_gap_adv_set_data failed: %" CHIP_ERROR_FORMAT " %d", err.Format(), discriminator);
        ExitNow();
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err;
    ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    mFlags.Clear(Flags::kAdvertisingRefreshNeeded);

    // Advertise connectable if we haven't reached the maximum number of connections.
    size_t numCons       = _NumConnections();
    bool connectable     = (numCons < kMaxConnections);
    adv_params.conn_mode = connectable ? BLE_GAP_CONN_MODE_UND : BLE_GAP_CONN_MODE_NON;

    // Advertise in fast mode if it is connectable advertisement and
    // the application has expressly requested fast advertising.
    if (connectable && mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        adv_params.itvl_min = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
        adv_params.itvl_max = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
    }
    else
    {
#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
        if (!mFlags.Has(Flags::kExtAdvertisingEnabled))
        {
            adv_params.itvl_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
            adv_params.itvl_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
        }
        else
        {
            adv_params.itvl_min = CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MIN;
            adv_params.itvl_max = CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MAX;
        }
#else

        adv_params.itvl_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        adv_params.itvl_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;

#endif
    }

    ChipLogProgress(DeviceLayer, "Configuring CHIPoBLE advertising (interval %" PRIu32 " ms, %sconnectable)",
                    (((uint32_t) adv_params.itvl_min) * 10) / 16, (connectable) ? "" : "non-");

    {
        if (ble_gap_adv_active())
        {
            /* Advertising is already active. Stop and restart with the new parameters */
            ChipLogProgress(DeviceLayer, "Device already advertising, stop active advertisement and restart");
            err = MapBLEError(ble_gap_adv_stop());
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "ble_gap_adv_stop() failed: %" CHIP_ERROR_FORMAT ", cannot restart", err.Format());
                return err;
            }
        }
#ifdef CONFIG_BT_NIMBLE_HOST_BASED_PRIVACY
        else
        {
            err = MapBLEError(ble_hs_pvcy_rpa_config(NIMBLE_HOST_ENABLE_RPA));
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "RPA not set: %" CHIP_ERROR_FORMAT, err.Format());
                return err;
            }
        }
#endif
        if (mScanResponse.HasValue())
        {
            err = MapBLEError(ble_gap_adv_rsp_set_data(mScanResponse.Value().data(), mScanResponse.Value().size()));
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "ble_gap_adv_rsp_set_data failed: %" CHIP_ERROR_FORMAT, err.Format());
                return err;
            }
        }
        err = MapBLEError(ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, ble_svr_gap_event, NULL));
        if (err == CHIP_NO_ERROR)
        {
            return CHIP_NO_ERROR;
        }
        else
        {
            ChipLogError(DeviceLayer, "ble_gap_adv_start() failed: %" CHIP_ERROR_FORMAT, err.Format());
            return err;
        }
    }
}
#endif // CONFIG_BT_NIMBLE_EXT_ADV

CHIP_ERROR BLEManagerImpl::ConfigureScanResponseData(ByteSpan data)
{
    if (data.empty() || data.size() > MAX_SCAN_RSP_DATA_LEN)
    {
        ChipLogError(DeviceLayer, "scan response data is invalid");
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    memcpy(scanResponseBuffer, data.data(), data.size());
    ByteSpan scanResponseSpan(scanResponseBuffer, data.size());
    mScanResponse = chip::Optional<ByteSpan>(scanResponseSpan);
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::ClearScanResponseData(void)
{
    mScanResponse.ClearValue();
    ChipLogDetail(DeviceLayer, "scan response data is cleared");
}

CHIP_ERROR BLEManagerImpl::ConfigureExtraServices(std::vector<struct ble_gatt_svc_def> & extGattSvcs, bool afterMatterSvc)
{
    if (!mGattSvcs.empty())
    {
        ChipLogError(DeviceLayer, "Cannot configure the extra GATT services");
        return CHIP_ERROR_INCORRECT_STATE;
    }
    if (afterMatterSvc)
    {
        mGattSvcs = extGattSvcs;
    }
    mGattSvcs.push_back(CHIPoBLEGATTSvc);
    if (!afterMatterSvc)
    {
        mGattSvcs.insert(mGattSvcs.end(), extGattSvcs.begin(), extGattSvcs.end());
    }
    mGattSvcs.push_back(kNullSvc);
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::HandleRXCharWrite(struct ble_gatt_char_context * param)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    uint16_t data_len = 0;

    ESP_LOGI(TAG, "Write request received for CHIPoBLE RX characteristic con %u %u", param->conn_handle, param->attr_handle);

    // Copy the data to a packet buffer.
    data_len               = OS_MBUF_PKTLEN(param->ctxt->om);
    PacketBufferHandle buf = System::PacketBufferHandle::New(data_len, 0);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(buf->AvailableDataLength() >= data_len, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    ble_hs_mbuf_to_flat(param->ctxt->om, buf->Start(), data_len, NULL);
    buf->SetDataLength(data_len);

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = param->conn_handle;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        err                               = PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %" CHIP_ERROR_FORMAT, err.Format());
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

    ChipLogProgress(DeviceLayer,
                    "Write request/command received for CHIPoBLE TX CCCD characteristic (con %u"
                    " ) indicate = %d",
                    gapEvent->subscribe.conn_handle, gapEvent->subscribe.cur_indicate);

    // Determine if the client is enabling or disabling indications.
    indicationsEnabled = gapEvent->subscribe.cur_indicate;

    // If the client has requested to enabled indications
    if (indicationsEnabled)
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
        event.Type = (indicationsEnabled) ? DeviceEventType::kCHIPoBLESubscribe : DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = gapEvent->subscribe.conn_handle;
        err                           = PlatformMgr().PostEvent(&event);
    }

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", (indicationsEnabled) ? "subscribe" : "unsubscribe");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %" CHIP_ERROR_FORMAT, err.Format());
        // TODO: fail connection???
    }

    return;
}

CHIP_ERROR BLEManagerImpl::HandleTXComplete(struct ble_gap_event * gapEvent)
{
    ChipLogProgress(DeviceLayer, "Confirm received for CHIPoBLE TX characteristic indication (con %u) status= %d ",
                    gapEvent->notify_tx.conn_handle, gapEvent->notify_tx.status);

    // Signal the BLE Layer that the outstanding indication is complete.
    if (gapEvent->notify_tx.status == BLE_HS_EDONE)
    {
        // Post an event to the Chip queue to process the indicate confirmation.
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
        event.CHIPoBLEIndicateConfirm.ConId = gapEvent->notify_tx.conn_handle;
        ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
    }

    else
    {
        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId  = gapEvent->notify_tx.conn_handle;
        event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
        ReturnErrorOnFailure(PlatformMgr().PostEvent(&event));
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

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
void BLEManagerImpl::HandleGAPConnectionFailed(struct ble_gap_event * gapEvent, CHIP_ERROR error)
{
    ChipLogError(Ble, "BLE GAP connection failed; status:%d", gapEvent->connect.status);
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event;
        event.Type                                    = DeviceEventType::kPlatformESP32BLECentralConnectFailed;
        event.Platform.BLECentralConnectFailed.mError = error;
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::OnGattDiscComplete(const struct peer * peer, int status, void * arg)
{
    if (status != 0)
    {
        ChipLogError(Ble, "GATT discovery failed; status:%d", status);
        ble_gap_terminate(peer->conn_handle, BLE_ERR_REM_USER_CONN_TERM);
        return;
    }

    ChipLogProgress(Ble, "GATT discovery complete status:%d conn_handle:%d", status, peer->conn_handle);

    ChipDeviceEvent event;
    event.Type                                     = DeviceEventType::kPlatformESP32BLECentralConnected;
    event.Platform.BLECentralConnected.mConnection = peer->conn_handle;
    PlatformMgr().PostEventOrDie(&event);
}

CHIP_ERROR BLEManagerImpl::HandleGAPCentralConnect(struct ble_gap_event * gapEvent)
{
    if (BLEManagerImpl::mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
    {
        if (gapEvent->connect.status == 0)
        {
            // Track the number of active GAP connections.
            mNumGAPCons++;

            ChipLogProgress(DeviceLayer, "BLE GAP connection established (con %u)", gapEvent->connect.conn_handle);

            // remember the peer
            int rc = peer_add(gapEvent->connect.conn_handle);
            if (rc != 0)
            {
                HandleGAPConnectionFailed(gapEvent, CHIP_ERROR_INTERNAL);
                ChipLogError(DeviceLayer, "peer_add failed: %d", rc);
                return CHIP_ERROR_INTERNAL;
            }

            // Start the GATT discovery process
            rc = peer_disc_all(gapEvent->connect.conn_handle, OnGattDiscComplete, NULL);
            if (rc != 0)
            {
                HandleGAPConnectionFailed(gapEvent, CHIP_ERROR_INTERNAL);
                ChipLogError(DeviceLayer, "peer_disc_al failed: %d", rc);
                return CHIP_ERROR_INTERNAL;
            }
        }
        else
        {
            HandleGAPConnectionFailed(gapEvent, CHIP_ERROR_INTERNAL);
            return CHIP_ERROR_INTERNAL;
        }
    }
    return CHIP_NO_ERROR;
}
#endif

CHIP_ERROR BLEManagerImpl::HandleGAPPeripheralConnect(struct ble_gap_event * gapEvent)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogProgress(DeviceLayer, "BLE GAP connection established (con %u)", gapEvent->connect.conn_handle);

    // Track the number of active GAP connections.
    mNumGAPCons++;
    err = SetSubscribed(gapEvent->connect.conn_handle);
    VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, err = CHIP_NO_ERROR);
    SuccessOrExit(err);

    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    mFlags.Clear(Flags::kAdvertisingConfigured);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::HandleGAPConnect(struct ble_gap_event * gapEvent)
{
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    int rc;

    rc = ble_gattc_exchange_mtu(gapEvent->connect.conn_handle, NULL, NULL);
    if (rc != 0)
    {
        return CHIP_ERROR_INTERNAL;
    }

    return HandleGAPCentralConnect(gapEvent);
#else
    return HandleGAPPeripheralConnect(gapEvent);
#endif
}

CHIP_ERROR BLEManagerImpl::HandleGAPDisconnect(struct ble_gap_event * gapEvent)
{
    ChipLogProgress(DeviceLayer, "BLE GAP connection terminated (con %u reason 0x%02x)", gapEvent->disconnect.conn.conn_handle,
                    gapEvent->disconnect.reason);

    // Update the number of GAP connections.
    if (mNumGAPCons > 0)
    {
        mNumGAPCons--;
    }

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    peer_delete(gapEvent->disconnect.conn.conn_handle);
#endif

    // There can be a case where the BLE central disconnects without unsubscribing from the BLE characteristic.
    // In such situations, it is necessary to clear the subscription and post a connection error event.
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

        ChipDeviceEvent connectionErrorEvent;
        connectionErrorEvent.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
        connectionErrorEvent.CHIPoBLEConnectionError.ConId  = gapEvent->disconnect.conn.conn_handle;
        connectionErrorEvent.CHIPoBLEConnectionError.Reason = disconReason;
        ReturnErrorOnFailure(PlatformMgr().PostEvent(&connectionErrorEvent));
    }

    ChipDeviceEvent disconnectEvent;
    disconnectEvent.Type = DeviceEventType::kCHIPoBLEConnectionClosed;
    ReturnErrorOnFailure(PlatformMgr().PostEvent(&disconnectEvent));

    // Force a reconfiguration of advertising in case we switched to non-connectable mode when
    // the BLE connection was established.
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    mFlags.Clear(Flags::kAdvertisingConfigured);

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
        if (event->notify_tx.status != 0)
        {
            err = sInstance.HandleTXComplete(event);
            SuccessOrExit(err);
        }
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGD(TAG, "BLE_GAP_EVENT_MTU = %d channel id = %d", event->mtu.value, event->mtu.channel_id);
        break;

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    case BLE_GAP_EVENT_NOTIFY_RX:
        ESP_LOGD(TAG, "BLE_GAP_EVENT_NOTIFY_RX received %s conn_handle:%d attr_handle:%d attr_len:%d",
                 event->notify_rx.indication ? "indication" : "notification", event->notify_rx.conn_handle,
                 event->notify_rx.attr_handle, OS_MBUF_PKTLEN(event->notify_rx.om));
        err = sInstance.HandleRXNotify(event);
        SuccessOrExit(err);
        break;
#endif
    default:
        break;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %" CHIP_ERROR_FORMAT, err.Format());
        sInstance.mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }

    // Schedule DriveBLEState() to run.
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return err.AsInteger();
}

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
void BLEManagerImpl::HandleC3CharRead(struct ble_gatt_char_context * param)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle bufferHandle;

    BitFlags<AdditionalDataFields> additionalDataFields;
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;

#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    uint8_t rotatingDeviceIdUniqueId[ConfigurationManager::kRotatingDeviceIDUniqueIDLength] = {};
    MutableByteSpan rotatingDeviceIdUniqueIdSpan(rotatingDeviceIdUniqueId);

    err = DeviceLayer::GetDeviceInstanceInfoProvider()->GetRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueIdSpan);
    SuccessOrExit(err);
    err = ConfigurationMgr().GetLifetimeCounter(additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter);
    SuccessOrExit(err);
    additionalDataPayloadParams.rotatingDeviceIdUniqueId = rotatingDeviceIdUniqueIdSpan;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);
#endif /* CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID) */

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(additionalDataPayloadParams, bufferHandle,
                                                                         additionalDataFields);
    SuccessOrExit(err);

    os_mbuf_append(param->ctxt->om, bufferHandle->Start(), static_cast<uint16_t>(bufferHandle->DataLength()));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to generate TLV encoded Additional Data, err:%" CHIP_ERROR_FORMAT, err.Format());
    }
    return;
}

int BLEManagerImpl::gatt_svr_chr_access_additional_data(uint16_t conn_handle, uint16_t attr_handle,
                                                        struct ble_gatt_access_ctxt * ctxt, void * arg)
{
    struct ble_gatt_char_context param;
    int err = 0;

    memset(&param, 0, sizeof(struct ble_gatt_char_context));

    switch (ctxt->op)
    {
    case BLE_GATT_ACCESS_OP_READ_CHR:

        param.conn_handle = conn_handle;
        param.attr_handle = attr_handle;
        param.ctxt        = ctxt;
        param.arg         = arg;
        sInstance.HandleC3CharRead(&param);
        break;

    default:
        err = BLE_ATT_ERR_UNLIKELY;
        break;
    }

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return err;
}
#endif /* CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING */

int BLEManagerImpl::gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt * ctxt, void * arg)
{
    struct ble_gatt_char_context param;
    int err = 0;

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

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
CHIP_ERROR BLEManagerImpl::HandleRXNotify(struct ble_gap_event * ble_event)
{
    uint16_t dataLen               = OS_MBUF_PKTLEN(ble_event->notify_rx.om);
    System::PacketBufferHandle buf = System::PacketBufferHandle::New(dataLen, 0);
    VerifyOrReturnError(!buf.IsNull(), CHIP_ERROR_NO_MEMORY);
    VerifyOrReturnError(buf->AvailableDataLength() >= dataLen, CHIP_ERROR_BUFFER_TOO_SMALL);
    ble_hs_mbuf_to_flat(ble_event->notify_rx.om, buf->Start(), dataLen, NULL);
    buf->SetDataLength(dataLen);

    ChipLogDetail(DeviceLayer, "Indication received, conn = %d", ble_event->notify_rx.conn_handle);

    ChipDeviceEvent event;
    event.Type                                       = DeviceEventType::kPlatformESP32BLEIndicationReceived;
    event.Platform.BLEIndicationReceived.mConnection = ble_event->notify_rx.conn_handle;
    event.Platform.BLEIndicationReceived.mData       = std::move(buf).UnsafeRelease();
    PlatformMgr().PostEventOrDie(&event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::ConfigureBle(uint32_t aAdapterId, bool aIsCentral)
{
    mBLEAdvConfig.mpBleName         = mDeviceName;
    mBLEAdvConfig.mAdapterId        = aAdapterId;
    mBLEAdvConfig.mMajor            = 1;
    mBLEAdvConfig.mMinor            = 1;
    mBLEAdvConfig.mVendorId         = 1;
    mBLEAdvConfig.mProductId        = 1;
    mBLEAdvConfig.mDeviceId         = 1;
    mBLEAdvConfig.mDuration         = 2;
    mBLEAdvConfig.mPairingStatus    = 0;
    mBLEAdvConfig.mType             = 1;
    mBLEAdvConfig.mpAdvertisingUUID = "0xFFF6";

    mIsCentral = aIsCentral;
    if (mIsCentral)
    {
        int rc = peer_init(MYNEWT_VAL(BLE_MAX_CONNECTIONS), 64, 64, 64);
        assert(rc == 0);
    }

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::OnDeviceScanned(const ble_addr_t & addr, const chip::Ble::ChipBLEDeviceIdentificationInfo & info)
{

    if (mBLEScanConfig.mBleScanState == BleScanState::kScanForDiscriminator)
    {
        if (!mBLEScanConfig.mDiscriminator.MatchesLongDiscriminator(info.GetDeviceDiscriminator()))
        {
            ChipLogProgress(Ble, "Discriminator did not match");
            return;
        }
        ChipLogProgress(Ble, "Device Discriminator match. Attempting to connect");
    }
    else if (mBLEScanConfig.mBleScanState == BleScanState::kScanForAddress)
    {
        ChipLogProgress(Ble, "Device Address match. Attempting to connect");
    }
    else
    {
        ChipLogProgress(Ble, "Unknown discovery type. Ignoring");
    }

    mBLEScanConfig.mBleScanState = BleScanState::kConnecting;
    chip::DeviceLayer::PlatformMgr().LockChipStack();
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(kConnectTimeout), HandleConnectTimeout, nullptr);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    mDeviceScanner.StopScan();

    ConnectDevice(addr, kConnectTimeout);
}

void BLEManagerImpl::OnScanComplete()
{
    if (mBLEScanConfig.mBleScanState != BleScanState::kScanForDiscriminator &&
        mBLEScanConfig.mBleScanState != BleScanState::kScanForAddress)
    {
        ChipLogProgress(Ble, "Scan complete notification without an active scan");
        return;
    }

    BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_TIMEOUT);
    mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
}

void BLEManagerImpl::InitiateScan(BleScanState scanType)
{
    DriveBLEState();

    // Check for a valid scan type
    if (scanType == BleScanState::kNotScanning)
    {
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_INCORRECT_STATE);
        ChipLogError(Ble, "Invalid scan type requested");
        return;
    }

    // Initialize the device scanner
    CHIP_ERROR err = mDeviceScanner.Init(this);
    if (err != CHIP_NO_ERROR)
    {
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, err);
        ChipLogError(Ble, "Failed to initialize device scanner: %" CHIP_ERROR_FORMAT, err.Format());
        return;
    }

    // Start scanning
    mBLEScanConfig.mBleScanState = scanType;
    err                          = mDeviceScanner.StartScan(kNewConnectionScanTimeout);
    if (err != CHIP_NO_ERROR)
    {
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        ChipLogError(Ble, "Failed to start a BLE scan: %" CHIP_ERROR_FORMAT, err.Format());
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, err);
        return;
    }
}

void BLEManagerImpl::CleanScanConfig()
{
    if (BLEManagerImpl::mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
    {
        DeviceLayer::SystemLayer().CancelTimer(HandleConnectTimeout, nullptr);
    }
    mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
}

void BLEManagerImpl::InitiateScan(intptr_t arg)
{
    sInstance.InitiateScan(static_cast<BleScanState>(arg));
}

void BLEManagerImpl::NewConnection(BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator)
{
    mBLEScanConfig.mDiscriminator = connDiscriminator;
    mBLEScanConfig.mAppState      = appState;

    // Initiate async scan
    PlatformMgr().ScheduleWork(InitiateScan, static_cast<intptr_t>(BleScanState::kScanForDiscriminator));
}

CHIP_ERROR BLEManagerImpl::CancelConnection()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}
#endif

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
