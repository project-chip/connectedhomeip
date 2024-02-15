/*
 *
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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#undef BT_ENABLE_HCI_SNOOP_LOG

#include <platform/CommissionableDataProvider.h>
#include <platform/internal/BLEManager.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "timers.h"
#include <ble/CHIPBleServiceData.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include "bt_callback_manager.h"
#include "bt_gap_le.h"
#include "bt_gatts.h"
#include "bt_init.h"
#include "bt_platform.h"
#include "bt_uuid.h"
#include "connection_info.h"
#ifdef BT_ENABLE_HCI_SNOOP_LOG
#include "bt_driver_btsnoop.h"
#endif
#include "gatt_service.h"

using namespace ::chip;
using namespace ::chip::Ble;

extern void (*CHIPoBLEProfile_read_callback)(uint16_t handle, void * data, uint16_t size);
extern void (*CHIPoBLEProfile_write_callback)(uint16_t handle, void * data, uint16_t size);
extern void (*CHIPoBLEProfile_ccc_callback)(uint16_t handle, void * data, uint16_t size);

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {
#define CHIP_ADV_DATA_TYPE_FLAGS 0x01
#define CHIP_ADV_DATA_FLAGS 0x06
#define CHIP_ADV_DATA_TYPE_NAME 0x09
#define CHIP_ADV_DATA_TYPE_SERVICE_DATA 0x16
#define CHIP_ADV_SHORT_UUID_LEN 2

#define MAX_ADV_DATA_LEN (31)
#define BLE_ADV_OTHER_LEN (9)

const uint8_t ShortUUID_CHIPoBLEService[] = { 0xF6, 0xFF };

#define EG_EVENT_BLE_POWER_ON_CNF (0x01)
#define EG_EVENT_BLE_ADV_CNF (0x02)

TimerHandle_t sbleAdvTimeoutTimer; // FreeRTOS sw timer.
EventGroupHandle_t xBleEventGroup;

const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

/***************************************************************************/
/**
 * Setup the bluetooth init function.
 *
 * @return none
 *
 * All bluetooth specific initialization
 ******************************************************************************/

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Initialize the CHIP BleLayer.
    ChipLogError(DeviceLayer, "BLE init start");
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
    ChipLogError(DeviceLayer, "BleLayer init complete");
    SuccessOrExit(err);

    mServiceMode                   = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    CHIPoBLEProfile_write_callback = BLEManagerImpl::HandleRXCharWrite;
    CHIPoBLEProfile_ccc_callback   = BLEManagerImpl::HandleTXCharCCCDWrite;

    xBleEventGroup = xEventGroupCreate();
    if (xBleEventGroup == NULL)
    {
        ChipLogError(DeviceLayer, "Cannot create xBleEventGroup");
        err = CHIP_ERROR_NO_MEMORY;
    }
    SuccessOrExit(err);

    init_connection_info();
    bt_create_task();

    bt_callback_manager_register_callback(bt_callback_type_app_event,
                                          (uint32_t) (MODULE_MASK_GAP | MODULE_MASK_GATT | MODULE_MASK_SYSTEM),
                                          (void *) BleMatterAppEventCallback);
#ifdef BT_ENABLE_HCI_SNOOP_LOG
    bt_driver_btsnoop_ctrl(1);
#endif
    // Create FreeRTOS sw timer for BLE timeouts and interval change.
    sbleAdvTimeoutTimer = xTimerCreate("BleAdvTimer",       // Just a text name, not used by the RTOS kernel
                                       1,                   // == default timer period (mS)
                                       false,               // no timer reload (==one-shot)
                                       (void *) this,       // init timer id = ble obj context
                                       BleAdvTimeoutHandler // timer callback handler
    );

    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    return num_connection_info();
}

#if 0
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
#endif

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (mFlags.Has(Flags::kAdvertisingEnabled) != val)
    {
        mFlags.Set(Flags::kAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
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

    mFlags.Set(Flags::kRestartAdvertising);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    if (strlen(gatts_device_name) >= bufSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    strcpy(buf, gatts_device_name);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

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
        strcpy(gatts_device_name, deviceName);
    }
    else
    {
        gatts_device_name[0] = 0;
    }

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe: {
        ChipDeviceEvent connEstEvent;

        ChipLogProgress(DeviceLayer, "_OnBlePlatformEvent kCHIPoBLESubscribe");
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
        PlatformMgr().PostEventOrDie(&connEstEvent);
    }
    break;

    case DeviceEventType::kCHIPoBLEUnsubscribe: {
        ChipLogProgress(DeviceLayer, "_OnBlePlatformEvent kCHIPoBLEUnsubscribe");
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kCHIPoBLEWriteReceived: {
        ChipLogProgress(DeviceLayer, "_OnBlePlatformEvent kCHIPoBLEWriteReceived");
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
    }
    break;

    case DeviceEventType::kCHIPoBLEConnectionError: {
        ChipLogProgress(DeviceLayer, "_OnBlePlatformEvent kCHIPoBLEConnectionError");
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
    }
    break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm: {
        ChipLogProgress(DeviceLayer, "_OnBlePlatformEvent kCHIPoBLEIndicateConfirm, ConId %04x",
                        event->CHIPoBLEIndicateConfirm.ConId);
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kCHIPoBLENotifyConfirm: {
        ChipLogProgress(DeviceLayer, "_OnBlePlatformEvent kCHIPoBLENotifyConfirm");
        HandleTxConfirmationEvent(event->CHIPoBLENotifyConfirm.ConId);
    }
    break;

    default:
        ChipLogProgress(DeviceLayer, "_OnBlePlatformEvent default:  event->Type = %d", event->Type);
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    bt_hci_cmd_disconnect_t disconnect_para;
    bt_status_t ret;

    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    disconnect_para.connection_handle = conId;
    disconnect_para.reason            = BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION;

    ret = bt_gap_le_disconnect(&disconnect_para);
    err = MapBLEError(ret);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "bt_gap_le_disconnect() failed: %s", ErrorStr(err));
    }

    return (err == CHIP_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    uint16_t mtu = (uint16_t) bt_gattc_get_mtu(conId);

    ChipLogProgress(DeviceLayer, "GetMTU (con %u), returning %u", conId, mtu);

    return mtu;
}

#define INDICATION_BUFFER_LENGTH (300)
bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    uint8_t buf[INDICATION_BUFFER_LENGTH + 3] = { 0 };
    bt_gattc_charc_value_notification_indication_t * req;
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipDeviceEvent event;
    bt_status_t ret;

    VerifyOrExit(UUIDsMatch(&ChipUUID_CHIPoBLEChar_TX, charId), err = CHIP_ERROR_INVALID_MESSAGE_TYPE);
    VerifyOrExit(UUIDsMatch(&ChipUUID_CHIPoBLEChar_TX, charId), err = CHIP_ERROR_INVALID_MESSAGE_TYPE);

    ChipLogProgress(DeviceLayer, "SendIndication(): conId %d, len %d", conId, data->DataLength());

    if (data->DataLength() > INDICATION_BUFFER_LENGTH)
    {
        ChipLogError(DeviceLayer, "SendIndication(): Exceed buffer length! conId %d, len %d", conId, data->DataLength());
        err = CHIP_ERROR_NO_MEMORY;
        goto exit;
    }

    req                         = (bt_gattc_charc_value_notification_indication_t *) buf;
    req->attribute_value_length = 3 + data->DataLength();
    req->att_req.opcode         = BT_ATT_OPCODE_HANDLE_VALUE_INDICATION;
    req->att_req.handle         = 24;
    memcpy(&req->att_req.attribute_value[0], data->Start(), data->DataLength());
    ret = bt_gatts_send_charc_value_notification_indication(conId, req);
    err = MapBLEError(ret);
    SuccessOrExit(err);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManagerImpl::SendIndication() failed: %s", ErrorStr(err));
        return false;
    }

    return true;
}

void BLEManagerImpl::HandleRXCharWrite(uint16_t handle, void * data, uint16_t size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;
    uint16_t writeLen = size;

    // Copy the data to a packet buffer.
    buf = System::PacketBufferHandle::NewWithData(data, writeLen, 0, 0);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    ChipLogDetail(DeviceLayer, "Write request/command received for CHIPoBLE RX characteristic (con %u, len %u)", handle,
                  buf->DataLength());

    // Post an event to the CHIP queue to deliver the data into the CHIP stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = handle;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        err                               = PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(uint16_t handle, void * data, uint16_t size)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    app_bt_connection_cb_t * bleConnState;
    bool isDisabled;
    ChipDeviceEvent event;
    bleConnState = find_connection_info_by_handle(handle);
    VerifyOrExit(bleConnState != NULL, err = CHIP_ERROR_NO_MEMORY);

    VerifyOrExit(size == sizeof(uint16_t), err = CHIP_ERROR_INVALID_MESSAGE_LENGTH);

    // Determine if the client is enabling or disabling notification/indication.
    isDisabled = (*(uint16_t *) data != 0x0002);

    ChipLogProgress(DeviceLayer, "HandleTXcharCCCDWrite - Config Flags value : %d", *(uint16_t *) data);
    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", isDisabled ? "unsubscribe" : "subscribe");

    if (!isDisabled)
    {
        // If indications are not already enabled for the connection...
        if (!bleConnState->subscribed)
        {
            bleConnState->subscribed = 1;
            // Post an event to the CHIP queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
            // whether the client is enabling or disabling indications.
            {
                event.Type                    = DeviceEventType::kCHIPoBLESubscribe;
                event.CHIPoBLESubscribe.ConId = handle;
                err                           = PlatformMgr().PostEvent(&event);
            }
        }
    }
    else
    {
        bleConnState->subscribed      = 0;
        event.Type                    = DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = handle;
        err                           = PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
    }
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
    // Nothing to do
}

void BLEManagerImpl::HandleTxConfirmationEvent(BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event;
    ChipLogProgress(DeviceLayer, "Tx Confirmation received!!!");

    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = conId;
    PlatformMgr().PostEventOrDie(&event);
}

CHIP_ERROR BLEManagerImpl::MapBLEError(int bleErr)
{
    switch (bleErr)
    {
    case BT_STATUS_SUCCESS:
        return CHIP_NO_ERROR;
    default:
        return CHIP_ERROR(ChipError::Range::kPlatform, bleErr + CHIP_DEVICE_CONFIG_BLE_ERROR_MIN);
    }
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    bt_hci_cmd_le_set_advertising_enable_t enable;
    uint32_t deviceNameLength = 0;
    ChipBLEDeviceIdentificationInfo deviceIdInfo;
    uint8_t deviceIdInfoLength = sizeof(deviceIdInfo);
    CHIP_ERROR err             = CHIP_NO_ERROR;
    int adv_name_len;
    uint32_t index = 0;
    bt_status_t ret;

    bt_hci_cmd_le_set_advertising_parameters_t adv_param = { .advertising_interval_min =
                                                                 CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN,
                                                             .advertising_interval_max =
                                                                 CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX,
                                                             .advertising_type          = BT_HCI_ADV_TYPE_CONNECTABLE_UNDIRECTED,
                                                             .own_address_type          = BT_ADDR_PUBLIC,
                                                             .advertising_channel_map   = 7,
                                                             .advertising_filter_policy = 0 };

    bt_hci_cmd_le_set_advertising_data_t adv_data = {
        .advertising_data_length = MAX_ADV_DATA_LEN,
    };

    if (mFlags.Has(Flags::kRestartAdvertising))
    {
        ChipLogProgress(DeviceLayer, "Stop advertising..");
        enable.advertising_enable = BT_HCI_DISABLE;
        ret                       = bt_gap_le_set_advertising(&enable, NULL, NULL, NULL);

        if (BT_STATUS_SUCCESS == ret)
        {
            xEventGroupWaitBits(xBleEventGroup, EG_EVENT_BLE_ADV_CNF, pdTRUE, pdFALSE, pdMS_TO_TICKS(10000));
            ChipLogProgress(DeviceLayer, "Advertising stopped.");
        }

        mFlags.Clear(Flags::kRestartAdvertising);
    }

    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        adv_param.advertising_interval_min = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
        adv_param.advertising_interval_max = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
    }

    if (!mFlags.Has(Flags::kDeviceNameSet))
    {
        uint16_t discriminator = 0;
        err                    = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator);

        snprintf(gatts_device_name, sizeof(gatts_device_name), "%s%04" PRIX32, CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX,
                 static_cast<uint32_t>(discriminator));

        gatts_device_name[kMaxDeviceNameLength] = 0;
    }

    deviceNameLength = strlen(gatts_device_name);

    VerifyOrExit(deviceNameLength < kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);
    deviceNameLength =
        deviceNameLength > MAX_ADV_DATA_LEN - BLE_ADV_OTHER_LEN - 1 ? MAX_ADV_DATA_LEN - BLE_ADV_OTHER_LEN - 1 : deviceNameLength;

    ChipLogProgress(DeviceLayer, "Beginning advertising, interval(min,max)=(%d, %d), devName=%s, len=%lu",
                    adv_param.advertising_interval_min, adv_param.advertising_interval_max, gatts_device_name, deviceNameLength);

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    SuccessOrExit(err);

    static_assert(sizeof(deviceIdInfo) + CHIP_ADV_SHORT_UUID_LEN + 1 <= UINT8_MAX, "Our length won't fit in a uint8_t");
    static_assert(2 + CHIP_ADV_SHORT_UUID_LEN + sizeof(deviceIdInfo) + 1 <= MAX_ADV_DATA_LEN, "Our buffer is not big enough");

    adv_data.advertising_data[index++] = 0x02;                     // AD length
    adv_data.advertising_data[index++] = CHIP_ADV_DATA_TYPE_FLAGS; // AD type : flags
    adv_data.advertising_data[index++] = CHIP_ADV_DATA_FLAGS;      // AD value

    adv_data.advertising_data[index++] = static_cast<uint8_t>(deviceIdInfoLength + CHIP_ADV_SHORT_UUID_LEN + 1); // AD length
    adv_data.advertising_data[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA; // AD type : Service Data
    adv_data.advertising_data[index++] = ShortUUID_CHIPoBLEService[0];    // AD value
    adv_data.advertising_data[index++] = ShortUUID_CHIPoBLEService[1];

    memcpy(&adv_data.advertising_data[index], (void *) &deviceIdInfo, deviceIdInfoLength); // AD value
    index += deviceIdInfoLength;

    adv_data.advertising_data[index++] = static_cast<uint8_t>(deviceNameLength + 1); // AD length
    adv_data.advertising_data[index++] = CHIP_ADV_DATA_TYPE_NAME;                    // AD type : name
    memcpy(&adv_data.advertising_data[index], gatts_device_name, deviceNameLength);  // AD value
    index += deviceNameLength;

    enable.advertising_enable = BT_HCI_ENABLE;
    ret                       = bt_gap_le_set_advertising(&enable, &adv_param, &adv_data, NULL);

    err = MapBLEError(ret);
    SuccessOrExit(err);

    if ((xEventGroupWaitBits(xBleEventGroup, EG_EVENT_BLE_ADV_CNF, pdTRUE, pdFALSE, pdMS_TO_TICKS(10000)) & EG_EVENT_BLE_ADV_CNF) ==
        EG_EVENT_BLE_ADV_CNF)
    {
        ChipLogProgress(DeviceLayer, "Advertising started.");
        if (mFlags.Has(Flags::kFastAdvertisingEnabled))
        {
            StartBleAdvTimeoutTimer(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
        }
        mFlags.Set(Flags::kAdvertising);
    }
    else
    {
        err = CHIP_ERROR_TIMEOUT;
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bt_status_t ret;

    if (mFlags.Has(Flags::kAdvertising))
    {
        mFlags.Clear(Flags::kAdvertising).Clear(Flags::kRestartAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled, true);

        ChipLogProgress(DeviceLayer, "Stop advertising..");
        bt_hci_cmd_le_set_advertising_enable_t enable = { BT_HCI_DISABLE };
        ret                                           = bt_gap_le_set_advertising(&enable, NULL, NULL, NULL);

        err = MapBLEError(ret);
        SuccessOrExit(err);

        if ((xEventGroupWaitBits(xBleEventGroup, EG_EVENT_BLE_ADV_CNF, pdTRUE, pdFALSE, pdMS_TO_TICKS(10000)) &
             EG_EVENT_BLE_ADV_CNF) != EG_EVENT_BLE_ADV_CNF)
        {
            err = CHIP_ERROR_TIMEOUT;
        }
        SuccessOrExit(err);

        ChipLogProgress(DeviceLayer, "Advertising stopped.");

        CancelBleAdvTimeoutTimer();
    }

exit:
    return err;
}

void BLEManagerImpl::BleAdvTimeoutHandler(TimerHandle_t xTimer)
{
    if (BLEMgrImpl().mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Start slow advertisement");
        BLEMgr().SetAdvertisingMode(BLEAdvertisingMode::kSlowAdvertising);
    }
    else if (BLEMgrImpl().mFlags.Has(Flags::kAdvertising))
    {
        // Advertisement time expired. Stop advertising
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Stop advertisement");
        BLEMgr().SetAdvertisingEnabled(false);
    }
}

void BLEManagerImpl::CancelBleAdvTimeoutTimer(void)
{
    if (xTimerStop(sbleAdvTimeoutTimer, 0) == pdFAIL)
    {
        ChipLogError(DeviceLayer, "Failed to stop BledAdv timeout timer");
    }
}

void BLEManagerImpl::StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sbleAdvTimeoutTimer))
    {
        CancelBleAdvTimeoutTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sbleAdvTimeoutTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        ChipLogError(DeviceLayer, "Failed to start BledAdv timeout timer");
    }
}

bt_status_t BLEManagerImpl::BleMatterAppEventCallback(bt_msg_type_t msg, bt_status_t status, void * buff)
{
    ChipLogProgress(DeviceLayer, "BleMatterAppEventCallback: msg %08x, status %08x", (unsigned int) msg, (unsigned int) status);

    // PlatformMgr().LockChipStack();

    switch (msg)
    {
    case BT_POWER_ON_CNF:
        if (BT_STATUS_SUCCESS != bt_gatts_set_max_mtu(247))
        {
            ChipLogError(DeviceLayer, "Unable to set BT GATTS maximum mtu size!");
        }

        sInstance.mFlags.Set(Flags::kBLEStackInitialized);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;

    case BT_GAP_LE_SET_ADVERTISING_CNF:
        ChipLogProgress(DeviceLayer, "BT_GAP_LE_SET_ADVERTISING_CNF: Raise EG_EVENT_BLE_ADV_CNF");
        xEventGroupSetBits(xBleEventGroup, EG_EVENT_BLE_ADV_CNF);
        break;

    case BT_GAP_LE_CONNECT_IND:
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;

    case BT_GAP_LE_DISCONNECT_IND: {
        bt_hci_evt_disconnect_complete_t * conn_evt = (bt_hci_evt_disconnect_complete_t *) buff;
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId = conn_evt->connection_handle;

        switch (conn_evt->reason)
        {
        case BT_HCI_STATUS_REMOTE_USER_TERMINATED_CONNECTION:
        case BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES:
        case BT_HCI_STATUS_REMOTE_TERMINATED_CONNECTION_DUE_TO_POWER_OFF:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;

        case BT_HCI_STATUS_CONNECTION_TERMINATED_BY_LOCAL_HOST:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_APP_CLOSED_CONNECTION;
            break;

        default:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
            break;
        }

        ChipLogProgress(DeviceLayer, "BLE GATT connection closed (con %u, reason %u)", conn_evt->connection_handle,
                        conn_evt->reason);

        PlatformMgr().PostEventOrDie(&event);

        // Arrange to re-enable connectable advertising in case it was disabled due to the
        // maximum connection limit being reached.

        sInstance.mFlags.Set(Flags::kRestartAdvertising);
        sInstance.mFlags.Set(Flags::kFastAdvertisingEnabled);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
    break;
    case BT_GATTC_CHARC_VALUE_CONFIRMATION: {
        bt_handle_t * connection_handle_p = (bt_handle_t *) buff;
        ChipDeviceEvent event;

        ChipLogProgress(DeviceLayer, "Tx Confirmation received");

        event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
        event.CHIPoBLEIndicateConfirm.ConId = *connection_handle_p;
        PlatformMgr().PostEventOrDie(&event);
    }
    break;

    default:
        break;
    }

    // PlatformMgr().UnlockChipStack();

    return BT_STATUS_SUCCESS;
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if BLE stack is initialized
    VerifyOrExit(mFlags.Has(Flags::kBLEStackInitialized), /* */);

    // Start advertising if needed...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && mFlags.Has(Flags::kAdvertisingEnabled) &&
        NumConnections() < kMaxConnections)
    {
        // Start/re-start advertising if not already started, or if there is a pending change
        // to the advertising configuration.
        if (!mFlags.Has(Flags::kAdvertising) || mFlags.Has(Flags::kRestartAdvertising))
        {
            err = StartAdvertising();
            SuccessOrExit(err);
        }
    }

    // Otherwise, stop advertising if it is enabled.
    else if (mFlags.Has(Flags::kAdvertising))
    {
        err = StopAdvertising();
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
