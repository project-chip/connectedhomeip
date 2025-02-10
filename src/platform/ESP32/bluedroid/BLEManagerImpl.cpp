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
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
#include <lib/support/CodeUtils.h>
#endif

#ifdef CONFIG_BT_BLUEDROID_ENABLED

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
#include <ble/Ble.h>
#endif
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CommissionableDataProvider.h>
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/ESP32/BLEManagerImpl.h>
#include <platform/ESP32/ChipDeviceScanner.h>
#endif
#include <platform/internal/BLEManager.h>
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#include <system/SystemTimer.h>
#endif

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
#define CHIP_MAX_MTU_SIZE 256

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

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
static constexpr uint16_t kNewConnectionScanTimeout = 60;
static constexpr uint16_t kConnectTimeout           = 20;
#endif

const uint16_t CHIPoBLEAppId = 0x235A;

const uint8_t UUID_PrimaryService[]       = { 0x00, 0x28 };
const uint8_t UUID_CharDecl[]             = { 0x03, 0x28 };
const uint8_t UUID_ClientCharConfigDesc[] = { 0x02, 0x29 };
const uint8_t UUID_CHIPoBLEService[]      = { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
                                              0x00, 0x10, 0x00, 0x00, 0xF6, 0xFF, 0x00, 0x00 };
const uint8_t ShortUUID_CHIPoBLEService[] = { 0xF6, 0xFF };
const uint8_t UUID_CHIPoBLEChar_RX[]      = { 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95,
                                              0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 };
const uint8_t UUID_CHIPoBLEChar_TX[]      = { 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95,
                                              0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 };
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
const uint8_t ShortUUID_CHIPoBLE_CharTx_Desc[] = { 0x02, 0x29 };
#endif

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

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
ChipDeviceScanner & mDeviceScanner = Internal::ChipDeviceScanner::GetInstance();
#endif
BLEManagerImpl BLEManagerImpl::sInstance;
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
static esp_gattc_char_elem_t * char_elem_result   = NULL;
static esp_gattc_descr_elem_t * descr_elem_result = NULL;

/// Declare static functions
static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t * param);

static esp_bt_uuid_t remote_filter_service_uuid = {
    .len = ESP_UUID_LEN_16,
    .uuid = {.uuid16 = 0xFFF6,},
};
static esp_bt_uuid_t notify_descr_uuid = {
    .len = ESP_UUID_LEN_16,
};

static bool connect    = false;
static bool get_server = false;
static uint16_t connId;

#define PROFILE_NUM 1
#define PROFILE_A_APP_ID 0
#define INVALID_HANDLE 0

struct gattc_profile_inst
{
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_start_handle;
    uint16_t service_end_handle;
    uint16_t notify_char_handle;
    uint16_t write_char_handle;
    esp_bd_addr_t remote_bda;
};

/* One gatt-based profile one app_id and one , this array will store the gattc_if returned by ESP_GATTS_REG_EVT */
static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_A_APP_ID] = {
        .gattc_cb = BLEManagerImpl::gattc_profile_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};
static esp_gatt_if_t chip_ctrl_gattc_if = 0;
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

    memset(reinterpret_cast<void *>(mCons), 0, sizeof(mCons));
    mServiceMode          = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mAppIf                = ESP_GATT_IF_NONE;
    mServiceAttrHandle    = 0;
    mRXCharAttrHandle     = 0;
    mTXCharAttrHandle     = 0;
    mTXCharCCCDAttrHandle = 0;
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART && !mIsCentral);
    mFlags.Set(Flags::kFastAdvertisingEnabled, !mIsCentral);
#else
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);
#endif
    memset(mDeviceName, 0, sizeof(mDeviceName));

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

void BLEManagerImpl::_Shutdown()
{
    CancelBleAdvTimeoutTimer();

    BleLayer::Shutdown();
    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;

    // selectively setting kGATTServiceStarted flag, in order to notify the state machine to stop the CHIPoBLE gatt service
    mFlags.ClearAll().Set(Flags::kGATTServiceStarted);

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    OnChipBleConnectReceived = nullptr;
#endif // CONFIG_ENABLE_ESP32_BLE_CONTROLLER

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
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

void BLEManagerImpl::gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if,
                                                 esp_ble_gattc_cb_param_t * param)
{
    esp_ble_gattc_cb_param_t * p_data = (esp_ble_gattc_cb_param_t *) param;
    CHIP_ERROR err                    = CHIP_NO_ERROR;

    switch (event)
    {
    case ESP_GATTC_REG_EVT:
        if (param->reg.status == ESP_GATT_OK)
        {
            chip_ctrl_gattc_if = gattc_if;
        }
        else
        {
            ChipLogProgress(Ble, "Reg app failed, app_id %04x, status 0x%x", param->reg.app_id, param->reg.status);
            return;
        }
        break;
    case ESP_GATTC_CONNECT_EVT:
        err = sInstance.HandleGAPConnect(*p_data);
        SuccessOrExit(err);
        break;
    case ESP_GATTC_OPEN_EVT:
        if (param->open.status != ESP_GATT_OK)
        {
            ChipLogProgress(Ble, "open failed, status %d", p_data->open.status);
            break;
        }
        ChipLogProgress(Ble, "open success");
        break;
    case ESP_GATTC_DIS_SRVC_CMPL_EVT:
        if (param->dis_srvc_cmpl.status != ESP_GATT_OK)
        {
            ChipLogProgress(Ble, "discover service failed, status %d", param->dis_srvc_cmpl.status);
            break;
        }
        ChipLogProgress(Ble, "discover service complete conn_id %d", param->dis_srvc_cmpl.conn_id);
        esp_ble_gattc_search_service(gattc_if, param->cfg_mtu.conn_id, &remote_filter_service_uuid);
        break;
    case ESP_GATTC_CFG_MTU_EVT:
        if (param->cfg_mtu.status != ESP_GATT_OK)
        {
            ChipLogProgress(Ble, "config mtu failed, error status = %x", param->cfg_mtu.status);
        }
        ChipLogProgress(Ble, "ESP_GATTC_CFG_MTU_EVT, Status %d, MTU %d, conn_id %d", param->cfg_mtu.status, param->cfg_mtu.mtu,
                        param->cfg_mtu.conn_id);
        break;
    case ESP_GATTC_SEARCH_RES_EVT: {
        ChipLogProgress(Ble, "SEARCH RES: conn_id = %x is primary service %d", p_data->search_res.conn_id,
                        p_data->search_res.is_primary);
        ChipLogProgress(Ble, "start handle %d end handle %d current handle value %d", p_data->search_res.start_handle,
                        p_data->search_res.end_handle, p_data->search_res.srvc_id.inst_id);
        if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_16 && p_data->search_res.srvc_id.uuid.uuid.uuid16 == 0xFFF6)
        {
            ChipLogProgress(Ble, "service found");
            get_server                                            = true;
            gl_profile_tab[PROFILE_A_APP_ID].service_start_handle = p_data->search_res.start_handle;
            gl_profile_tab[PROFILE_A_APP_ID].service_end_handle   = p_data->search_res.end_handle;
            ChipLogProgress(Ble, "UUID16: %x", p_data->search_res.srvc_id.uuid.uuid.uuid16);
        }
        break;
    }
    case ESP_GATTC_SEARCH_CMPL_EVT: {
        if (p_data->search_cmpl.status != ESP_GATT_OK)
        {
            ChipLogProgress(Ble, "search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_REMOTE_DEVICE)
        {
            ChipLogProgress(Ble, "Get service information from remote device");
        }
        else if (p_data->search_cmpl.searched_service_source == ESP_GATT_SERVICE_FROM_NVS_FLASH)
        {
            ChipLogProgress(Ble, "Get service information from flash");
        }
        else
        {
            ChipLogProgress(Ble, "unknown service source");
        }
        ChipLogProgress(Ble, "ESP_GATTC_SEARCH_CMPL_EVT");
        if (get_server)
        {
            uint16_t count  = 0;
            uint16_t offset = 0;
            esp_gatt_status_t status =
                esp_ble_gattc_get_attr_count(gattc_if, p_data->search_cmpl.conn_id, ESP_GATT_DB_CHARACTERISTIC,
                                             gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                                             gl_profile_tab[PROFILE_A_APP_ID].service_end_handle, INVALID_HANDLE, &count);
            if (status != ESP_GATT_OK)
            {
                ChipLogProgress(Ble, "esp_ble_gattc_get_attr_count error");
            }
            ChipLogProgress(Ble, "Count : %d", count);

            if (count > 0)
            {
                char_elem_result = (esp_gattc_char_elem_t *) malloc(sizeof(esp_gattc_char_elem_t) * count);
                // memset(char_elem_result, 0xff, sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result)
                {
                    ChipLogProgress(Ble, "gattc no mem");
                    break;
                }
                else
                {
                    status = esp_ble_gattc_get_all_char(
                        gattc_if, p_data->search_cmpl.conn_id, gl_profile_tab[PROFILE_A_APP_ID].service_start_handle,
                        gl_profile_tab[PROFILE_A_APP_ID].service_end_handle, char_elem_result, &count, offset);
                    if (status != 0)
                    {
                        ChipLogProgress(Ble, "esp_ble_gattc_get_char_by_uuid error");
                    }

                    /*  Every service have only one char in our 'ESP_GATTS_DEMO' demo, so we used first 'char_elem_result' */
                }
            }
            if (count > 0)
            {
                for (int i = 0; i < count; i++)
                {
                    if (char_elem_result[i].uuid.len == ESP_UUID_LEN_128)
                    {
                        if (char_elem_result[i].properties & CharProps_Write)
                        {
                            gl_profile_tab[PROFILE_A_APP_ID].write_char_handle = char_elem_result[i].char_handle;
                        }
                        else if (char_elem_result[i].properties & CharProps_ReadNotify)
                        {
                            gl_profile_tab[PROFILE_A_APP_ID].notify_char_handle = char_elem_result[i].char_handle;
                            esp_ble_gattc_register_for_notify(gattc_if, gl_profile_tab[PROFILE_A_APP_ID].remote_bda,
                                                              char_elem_result[i].char_handle);
                        }
                    }
                }
            }
            free(char_elem_result);
        }
        else
        {
            ChipLogProgress(Ble, "no char found");
        }
    }
    break;
    case ESP_GATTC_REG_FOR_NOTIFY_EVT: {
        ChipLogProgress(Ble, "ESP_GATTC_REG_FOR_NOTIFY_EVT");
        if (p_data->reg_for_notify.status != ESP_GATT_OK)
        {
            ChipLogProgress(Ble, "REG FOR NOTIFY failed: error status = %d", p_data->reg_for_notify.status);
        }
        else
        {
            uint16_t count               = 0;
            esp_gatt_status_t ret_status = esp_ble_gattc_get_attr_count(
                gattc_if, gl_profile_tab[PROFILE_A_APP_ID].conn_id, ESP_GATT_DB_DESCRIPTOR,
                gl_profile_tab[PROFILE_A_APP_ID].service_start_handle, gl_profile_tab[PROFILE_A_APP_ID].service_end_handle,
                gl_profile_tab[PROFILE_A_APP_ID].notify_char_handle, &count);
            if (ret_status != ESP_GATT_OK)
            {
                ChipLogProgress(Ble, "esp_ble_gattc_get_attr_count error");
            }
            if (count > 0)
            {
                descr_elem_result = (esp_gattc_descr_elem_t *) malloc(sizeof(esp_gattc_descr_elem_t) * count);
                if (!descr_elem_result)
                {
                    ChipLogProgress(Ble, "malloc error, gattc no mem");
                }
                else
                {
                    memcpy(&notify_descr_uuid.uuid.uuid16, ShortUUID_CHIPoBLE_CharTx_Desc, 2);
                    ret_status = esp_ble_gattc_get_descr_by_char_handle(gattc_if, gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                                                        p_data->reg_for_notify.handle, notify_descr_uuid,
                                                                        descr_elem_result, &count);
                    ChipLogProgress(Ble, "discoverd all chars and discr.........\n\n");

                    ChipDeviceEvent chipEvent;
                    chipEvent.Type                                     = DeviceEventType::kPlatformESP32BLECentralConnected;
                    chipEvent.Platform.BLECentralConnected.mConnection = connId;
                    PlatformMgr().PostEventOrDie(&chipEvent);
                    if (ret_status != ESP_GATT_OK)
                    {
                        ChipLogProgress(Ble, "esp_ble_gattc_get_descr_by_char_handle error");
                    }
                    /* Every char has only one descriptor in our 'ESP_GATTS_DEMO' demo, so we used first 'descr_elem_result' */
                    if (count > 0 && descr_elem_result[0].uuid.len == ESP_UUID_LEN_16 &&
                        descr_elem_result[0].uuid.uuid.uuid16 == ESP_GATT_UUID_CHAR_CLIENT_CONFIG)
                    {
                    }

                    if (ret_status != ESP_GATT_OK)
                    {
                        ChipLogProgress(Ble, "esp_ble_gattc_write_char_descr error");
                    }
                    free(descr_elem_result);
                }
            }
        }
        ChipLogProgress(Ble, "decsr not found");
    }
    break;
    case ESP_GATTC_NOTIFY_EVT:
        if (p_data->notify.is_notify)
        {
            ChipLogProgress(Ble, "ESP_GATTC_NOTIFY_EVT, receive notify value:");
        }
        else
        {
            ChipLogProgress(Ble, "ESP_GATTC_NOTIFY_EVT, receive indicate value:");
        }
        err = sInstance.HandleRXNotify(*p_data);
        SuccessOrExit(err);

        break;
    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK)
        {
            ChipLogProgress(Ble, "write descr failed, error status = %x", p_data->write.status);
            break;
        }
        ChipLogProgress(Ble, "write descr success ");
        break;
    case ESP_GATTC_SRVC_CHG_EVT: {
        esp_bd_addr_t bda;
        memcpy(bda, p_data->srvc_chg.remote_bda, sizeof(esp_bd_addr_t));
        ChipLogProgress(Ble, "ESP_GATTC_SRVC_CHG_EVT, bd_addr:");
        break;
    }
    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK)
        {
            ChipLogProgress(Ble, "write char failed, error status = %x", p_data->write.status);
            break;
        }
        ChipLogProgress(Ble, "write char success ");
        break;
    case ESP_GATTC_DISCONNECT_EVT:
        connect    = false;
        get_server = false;
        ChipLogProgress(Ble, "ESP_GATTC_DISCONNECT_EVT, reason = %d", p_data->disconnect.reason);
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
}

static void esp_gattc_cb(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t * param)
{
    /* If event is register event, store the  for each profile */
    if (event == ESP_GATTC_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        }
        else
        {
            ChipLogProgress(Ble, "reg app failed, app_id %04x, status %d", param->reg.app_id, param->reg.status);
            return;
        }
    }

    /* If the  equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do
    {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++)
        {
            if (gattc_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb
                                                   function */
                gattc_if == gl_profile_tab[idx].gattc_if)
            {
                if (gl_profile_tab[idx].gattc_cb)
                {
                    gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
                }
            }
        }
    } while (0);
}

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
    int rc = esp_ble_gattc_close(chip_ctrl_gattc_if, connId);
    VerifyOrReturn(rc == 0, ChipLogError(Ble, "Failed to cancel connection rc=%d", rc));
}

void BLEManagerImpl::ConnectDevice(esp_bd_addr_t & addr, esp_ble_addr_type_t addr_type, uint16_t timeout)
{
    int rc;
    rc = esp_ble_gattc_open(chip_ctrl_gattc_if, addr, addr_type, true);
    if (rc != 0)
    {
        ChipLogError(Ble, "Failed to connect to rc=%d", rc);
    }
}
#endif

CHIP_ERROR BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                   const ChipBleUUID * charId)
{
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    uint8_t value[2];
    int rc;

    value[0] = 0x02;
    value[1] = 0x00;

    rc = esp_ble_gattc_write_char_descr(chip_ctrl_gattc_if, gl_profile_tab[PROFILE_A_APP_ID].conn_id, descr_elem_result[0].handle,
                                        sizeof(value), value, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
    if (rc != 0)
    {
        ChipLogError(Ble, "esp_ble_gattc_get_descr_by_char_handle failed: %d", rc);
        esp_ble_gattc_close(chip_ctrl_gattc_if, conId);
        return BLE_ERROR_GATT_SUBSCRIBE_FAILED;
    }
    ChipDeviceEvent event;
    event.Type                                          = DeviceEventType::kPlatformESP32BLESubscribeOpComplete;
    event.Platform.BLESubscribeOpComplete.mConnection   = conId;
    event.Platform.BLESubscribeOpComplete.mIsSubscribed = true;
    PlatformMgr().PostEventOrDie(&event);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                     const ChipBleUUID * charId)
{
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    uint8_t value[2];
    int rc;

    value[0] = 0x00;
    value[1] = 0x00;

    rc = esp_ble_gattc_write_char_descr(chip_ctrl_gattc_if, gl_profile_tab[PROFILE_A_APP_ID].conn_id, descr_elem_result[0].handle,
                                        sizeof(value), value, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
    if (rc != 0)
    {
        ChipLogError(Ble, "ble_gattc_write_flat failed: %d", rc);
        esp_ble_gattc_close(chip_ctrl_gattc_if, conId);
        return BLE_ERROR_GATT_UNSUBSCRIBE_FAILED;
    }
    ChipDeviceEvent event;
    event.Type                                          = DeviceEventType::kPlatformESP32BLESubscribeOpComplete;
    event.Platform.BLESubscribeOpComplete.mConnection   = conId;
    event.Platform.BLESubscribeOpComplete.mIsSubscribed = false;
    PlatformMgr().PostEventOrDie(&event);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

CHIP_ERROR BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
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
    CHIPoBLEConState * conState = const_cast<BLEManagerImpl *>(this)->GetConnectionState(conId);
    return (conState != NULL) ? conState->MTU : 0;
}

CHIP_ERROR BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                          PacketBufferHandle data)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    CHIPoBLEConState * conState = GetConnectionState(conId);

    ESP_LOGD(TAG, "Sending indication for CHIPoBLE TX characteristic (con %u, len %u)", conId, data->DataLength());

    VerifyOrExit(conState != NULL, err = CHIP_ERROR_INVALID_ARGUMENT);

    VerifyOrExit(conState->PendingIndBuf.IsNull(), err = CHIP_ERROR_INCORRECT_STATE);
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    ChipLogDetail(Ble, "Sending indication for CHIPoBLE TX characteristic (con %u, len %u)", conId, data->DataLength());
#endif

    // Set param need_confirm as false will send notification, otherwise indication.
    err = MapBLEError(esp_ble_gatts_send_indicate(mAppIf, conId, mTXCharAttrHandle, static_cast<uint16_t>(data->DataLength()),
                                                  data->Start(), true /* need_confirm */));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gatts_send_indicate() failed: %s", ErrorStr(err));
        ExitNow();
    }

    // Save a reference to the buffer until we get a indication from the ESP BLE layer that it
    // has been sent.
    conState->PendingIndBuf = std::move(data);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                            PacketBufferHandle pBuf)
{
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    ChipLogProgress(Ble, "In send write request\n");
    int rc;

    rc = esp_ble_gattc_write_char(chip_ctrl_gattc_if, gl_profile_tab[PROFILE_A_APP_ID].conn_id,
                                  gl_profile_tab[PROFILE_A_APP_ID].write_char_handle, pBuf->DataLength(), pBuf->Start(),
                                  ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
    if (rc != 0)
    {
        ChipLogError(Ble, "ble_gattc_write_flat failed: %d", rc);
        return BLE_ERROR_GATT_WRITE_FAILED;
    }
    ChipDeviceEvent event;
    event.Type                                  = DeviceEventType::kPlatformESP32BLEWriteComplete;
    event.Platform.BLEWriteComplete.mConnection = conId;
    PlatformMgr().PostEventOrDie(&event);
    return CHIP_NO_ERROR;
#else
    return CHIP_ERROR_NOT_IMPLEMENTED;
#endif
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(Ble, "Got notification regarding chip connection closure");
    CloseConnection(conId);
}

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
        return CHIP_ERROR(ChipError::Range::kPlatform, CHIP_DEVICE_CONFIG_ESP32_BLE_ERROR_MIN + bleErr);
    }
}

void BLEManagerImpl::CancelBleAdvTimeoutTimer(void)
{
    SystemLayer().CancelTimer(BleAdvTimeoutHandler, nullptr);
}

void BLEManagerImpl::StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs)
{
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

        DeinitESPBleLayer();
        mFlags.ClearAll();

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
    err = MapBLEError(esp_ble_gatt_set_local_mtu(CHIP_MAX_MTU_SIZE));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gatt_set_local_mtu() failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    mFlags.Set(Flags::kESPBLELayerInitialized);

exit:
    return err;
}

esp_err_t bluedroid_set_random_address()
{
    esp_bd_addr_t rand_addr;

    esp_fill_random(rand_addr, sizeof(esp_bd_addr_t));
    rand_addr[0] = (rand_addr[0] & 0x3F) | 0xC0;

    esp_err_t ret = esp_ble_gap_set_rand_addr(rand_addr);
    if (ret != ESP_OK)
    {
        ChipLogError(DeviceLayer, "Failed to set random address: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    CHIP_ERROR err;
    uint8_t advData[MAX_ADV_DATA_LEN];
    uint8_t index = 0;

    // If a custom device name has not been specified, generate a CHIP-standard name based on the
    // discriminator value
    uint16_t discriminator;
    SuccessOrExit(err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));

    if (!mFlags.Has(Flags::kUseCustomDeviceName))
    {
        ChipLogProgress(Ble, mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
        mDeviceName[kMaxDeviceNameLength] = 0;
    }

    // Configure the BLE device name.
    err = MapBLEError(esp_ble_gap_set_device_name(mDeviceName));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "esp_ble_gap_set_device_name() failed: %s", ErrorStr(err));
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

    memset(advData, 0, sizeof(advData));
    advData[index++] = 0x02;                            // length
    advData[index++] = CHIP_ADV_DATA_TYPE_FLAGS;        // AD type : flags
    advData[index++] = CHIP_ADV_DATA_FLAGS;             // AD value
    advData[index++] = 0x0B;                            // length
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

    bluedroid_set_random_address();
    mFlags.Set(Flags::kControlOpInProgress);

exit:
    return err;
}

// TODO(#36919): Fix the Bluedroid ShutDown flow for ESP32.
void BLEManagerImpl::DeinitESPBleLayer() {}

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
#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
        if (!mFlags.Has(Flags::kExtAdvertisingEnabled))
        {
            advertParams.adv_int_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
            advertParams.adv_int_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
        }
        else
        {
            advertParams.adv_int_min = CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MIN;
            advertParams.adv_int_max = CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MAX;
        }
#else

        advertParams.adv_int_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        advertParams.adv_int_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;

#endif
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
                ExitNow(err = CHIP_ERROR_INTERNAL);
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
            ExitNow(err = CHIP_ERROR_INTERNAL);
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
            ExitNow(err = CHIP_ERROR_INTERNAL);
        }

        ChipLogProgress(DeviceLayer, "CHIPoBLE GATT service started");

        mFlags.Set(Flags::kGATTServiceStarted);
        controlOpComplete = true;

        break;

    case ESP_GATTS_STOP_EVT:

        if (param->stop.status != ESP_GATT_OK)
        {
            ChipLogError(DeviceLayer, "ESP_GATTS_STOP_EVT error: %d", (int) param->stop.status);
            ExitNow(err = CHIP_ERROR_INTERNAL);
        }

        ChipLogProgress(DeviceLayer, "CHIPoBLE GATT service stopped");

        mFlags.Clear(Flags::kGATTServiceStarted);
        controlOpComplete = true;

        break;

    case ESP_GATTS_RESPONSE_EVT:
        ChipLogDetail(Ble, "ESP_GATTS_RESPONSE_EVT (handle %u, status %d)", param->rsp.handle, (int) param->rsp.status);
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
        ChipLogDetail(Ble, "MTU for con %u: %u", param->mtu.conn_id, param->mtu.mtu);
        CHIPoBLEConState * conState = GetConnectionState(param->mtu.conn_id);
        if (conState != NULL)
        {
            // conState->MTU is a 10-bit field inside a uint16_t.  We're
            // assigning to it from a uint16_t, and compilers warn about
            // possibly not fitting.  There's no way to suppress that warning
            // via explicit cast; we have to disable the warning around the
            // assignment.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
            // As per the BLE specification, the maximum MTU value can be 517 bytes.
            // This can be accomodated in 10 bits
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

    ChipLogDetail(Ble, "Write request received for CHIPoBLE RX characteristic (con %u, len %u)", param->write.conn_id,
                  param->write.len);

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
        err                               = PlatformMgr().PostEvent(&event);
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

    ChipLogDetail(Ble, "Read request received for CHIPoBLE TX characteristic (con %u)", param->read.conn_id);

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

    ChipLogDetail(Ble, "Read request received for CHIPoBLE TX characteristic CCCD (con %u)", param->read.conn_id);

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

    ChipLogDetail(Ble, "Write request received for CHIPoBLE TX characteristic CCCD (con %u, len %u)", param->write.conn_id,
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
        err                           = PlatformMgr().PostEvent(&event);
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
    ChipLogDetail(Ble, "Confirm received for CHIPoBLE TX characteristic indication (con %u, status %u)", param->conf.conn_id,
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
        PlatformMgr().PostEventOrDie(&event);
    }

    else
    {
        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId  = param->disconnect.conn_id;
        event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
        PlatformMgr().PostEventOrDie(&event);
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
        PlatformMgr().PostEventOrDie(&event);

        ChipDeviceEvent disconnectEvent;
        disconnectEvent.Type = DeviceEventType::kCHIPoBLEConnectionClosed;
        PlatformMgr().PostEventOrDie(&disconnectEvent);

        // Force a refresh of the advertising state.
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);
        mFlags.Clear(Flags::kAdvertisingConfigured);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
CHIP_ERROR BLEManagerImpl::HandleRXNotify(esp_ble_gattc_cb_param_t param)
{
    System::PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(param.notify.value, param.notify.value_len);
    VerifyOrReturnError(!buf.IsNull(), CHIP_ERROR_NO_MEMORY);

    ChipLogDetail(DeviceLayer, "Indication received, conn = %d", param.notify.conn_id);

    ChipDeviceEvent event;
    event.Type                                       = DeviceEventType::kPlatformESP32BLEIndicationReceived;
    event.Platform.BLEIndicationReceived.mConnection = param.notify.conn_id;
    event.Platform.BLEIndicationReceived.mData       = std::move(buf).UnsafeRelease();
    PlatformMgr().PostEventOrDie(&event);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::ConfigureBle(uint32_t aAdapterId, bool aIsCentral)
{
    CHIP_ERROR err                  = CHIP_NO_ERROR;
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
        err = MapBLEError(esp_ble_gattc_register_callback(esp_gattc_cb));
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "esp_ble_gattc_register_callback() failed: %s", ErrorStr(err));
            ExitNow();
        }
        ChipLogProgress(Ble, "Before initialising (PROFILE\n");

        int rc = esp_ble_gattc_app_register(PROFILE_A_APP_ID);
        if (rc != 0)
        {
            ChipLogError(DeviceLayer, "esp_ble_gattc_app_register() failed %s", ErrorStr(err));
            ExitNow();
        }
    }

    mFlags.Set(Flags::kESPBLELayerInitialized);

exit:
    if (err != CHIP_NO_ERROR)
        return err;

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::OnDeviceScanned(esp_ble_addr_type_t & addr_type, esp_bd_addr_t & addr,
                                     const chip::Ble::ChipBLEDeviceIdentificationInfo & info)
{
    ChipLogProgress(Ble, "In OnDeviceScanned\n");
    if (mBLEScanConfig.mBleScanState == BleScanState::kScanForDiscriminator)
    {
        if (!mBLEScanConfig.mDiscriminator.MatchesLongDiscriminator(info.GetDeviceDiscriminator()))
        {
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

    connect                      = true;
    mBLEScanConfig.mBleScanState = BleScanState::kConnecting;
    DeviceLayer::SystemLayer().StartTimer(System::Clock::Seconds16(kConnectTimeout), HandleConnectTimeout, nullptr);
    mDeviceScanner.StopScan();
    ChipLogProgress(Ble, "Scanned all devices\n");

    ConnectDevice(addr, addr_type, kConnectTimeout);
}

void BLEManagerImpl::OnScanComplete()
{
    ChipLogProgress(Ble, "Stop scan\n");
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
        ChipLogError(Ble, "Failed to initialize device scanner: %s", ErrorStr(err));
        return;
    }

    // Start scanning
    mBLEScanConfig.mBleScanState = scanType;
    err                          = mDeviceScanner.StartScan(kNewConnectionScanTimeout);
    if (err != CHIP_NO_ERROR)
    {
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        ChipLogError(Ble, "Failed to start a BLE scan: %s", chip::ErrorStr(err));
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, err);
        return;
    }
}

void BLEManagerImpl::HandleConnectTimeout(chip::System::Layer *, void * context)
{
    CancelConnect();
    BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_TIMEOUT);
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

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
void BLEManagerImpl::HandleGAPConnectionFailed()
{
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event;
        event.Type                                    = DeviceEventType::kPlatformESP32BLECentralConnectFailed;
        event.Platform.BLECentralConnectFailed.mError = CHIP_ERROR_INTERNAL;
        PlatformMgr().PostEventOrDie(&event);
    }
}

CHIP_ERROR BLEManagerImpl::HandleGAPCentralConnect(esp_ble_gattc_cb_param_t p_data)
{
    if (BLEManagerImpl::mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
    {
        {
            ChipLogProgress(DeviceLayer, "BLE GAP connection established (con %u)", p_data.connect.conn_id);

            // remember the peer
            connId                                   = p_data.connect.conn_id;
            gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data.connect.conn_id;
            memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data.connect.remote_bda, sizeof(esp_bd_addr_t));

            // Start the GATT discovery process
            int rc = esp_ble_gattc_search_service(chip_ctrl_gattc_if, connId, &remote_filter_service_uuid);
            if (rc != 0)
            {
                HandleGAPConnectionFailed();
                ChipLogError(DeviceLayer, "peer_disc_al failed: %d", rc);
                return CHIP_ERROR_INTERNAL;
            }
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleGAPConnect(esp_ble_gattc_cb_param_t p_data)
{
    if (mIsCentral)
    {
        int rc;
        gl_profile_tab[PROFILE_A_APP_ID].conn_id = p_data.connect.conn_id;
        connId                                   = p_data.connect.conn_id;
        memcpy(gl_profile_tab[PROFILE_A_APP_ID].remote_bda, p_data.connect.remote_bda, sizeof(esp_bd_addr_t));
        rc = esp_ble_gattc_send_mtu_req(chip_ctrl_gattc_if, p_data.connect.conn_id);

        if (rc != 0)
        {
            ChipLogProgress(Ble, "MTU error\n");
            return CHIP_ERROR_INTERNAL;
        }

        return HandleGAPCentralConnect(p_data);
    }
    return CHIP_NO_ERROR;
}
#endif

void BLEManagerImpl::HandleGATTEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param)
{
    ChipLogProgress(Ble, "GATT Event: %d (if %d)", (int) event, (int) gatts_if);

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
#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    esp_ble_gap_cb_param_t * scan_result = (esp_ble_gap_cb_param_t *) param;
#endif

    ChipLogProgress(Ble, "GAP Event: %d", (int) event);

    // This method is invoked on the ESP BLE thread.  Therefore we must hold a lock
    // on the Chip stack while processing the event.
    PlatformMgr().LockChipStack();

    switch (event)
    {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT: {
        if (param->adv_data_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ChipLogError(DeviceLayer, "ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT error: %d", (int) param->adv_data_cmpl.status);
            ExitNow(err = CHIP_ERROR_INTERNAL);
        }

        sInstance.mFlags.Set(Flags::kAdvertisingConfigured);
        sInstance.mFlags.Clear(Flags::kControlOpInProgress);
    }
    break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT: {
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ChipLogError(DeviceLayer, "ESP_GAP_BLE_ADV_START_COMPLETE_EVT error: %d", (int) param->adv_start_cmpl.status);
            ExitNow(err = CHIP_ERROR_INTERNAL);
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
                err                                        = PlatformMgr().PostEvent(&advChange);
            }
        }
    }
    break;

    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT: {
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ChipLogError(DeviceLayer, "ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT error: %d", (int) param->adv_stop_cmpl.status);
            ExitNow(err = CHIP_ERROR_INTERNAL);
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
                err                                        = PlatformMgr().PostEvent(&advChange);
            }
        }
    }
    break;

#ifdef CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        mDeviceScanner.ReportDevice(*scan_result, scan_result->scan_rst.bda);
    }
    break;

    case ESP_GAP_SEARCH_INQ_CMPL_EVT:
        mDeviceScanner.mIsScanning = false;
        break;
#endif
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
