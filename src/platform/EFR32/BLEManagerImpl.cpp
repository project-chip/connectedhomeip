/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          for the Silicon Labs EFR32 platforms.
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <platform/internal/BLEManager.h>

#include <ble/CHIPBleServiceData.h>
#include <platform/EFR32/freertos_bluetooth.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include "rtos_gecko.h"

using namespace ::chip;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

#define CHIP_ADV_DATA_TYPE_FLAGS 0x01
#define CHIP_ADV_DATA_TYPE_UUID 0x03
#define CHIP_ADV_DATA_TYPE_NAME 0x09
#define CHIP_ADV_DATA_TYPE_SERVICE_DATA 0x16

#define CHIP_ADV_DATA_FLAGS 0x06
#define CHIP_ADV_CHIPOBLE_SERVICE_HANDLE 0

#define CHIP_ADV_DATA 0
#define CHIP_ADV_SCAN_RESPONSE_DATA 1
#define CHIP_ADV_SHORT_UUID_LEN 2

#define MAX_RESPONSE_DATA_LEN 31
#define MAX_ADV_DATA_LEN 31

// Timer Frequency used.
#define TIMER_CLK_FREQ ((uint32) 32768)
// Convert msec to timer ticks.
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)
#define TIMER_S_2_TIMERTICK(s) (TIMER_CLK_FREQ * s)

uint8_t bluetooth_stack_heap[DEFAULT_BLUETOOTH_HEAP(BLE_LAYER_NUM_BLE_ENDPOINTS)];

/* Bluetooth stack configuration parameters (see "UG136: Silicon Labs Bluetooth C Application Developer's Guide" for
 * details on each parameter) */
static gecko_configuration_t config;

const uint8_t UUID_CHIPoBLEService[]       = { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
                                         0x00, 0x10, 0x00, 0x00, 0xAF, 0xFE, 0x00, 0x00 };
const uint8_t ShortUUID_CHIPoBLEService[]  = { 0xAF, 0xFE };
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
 * @return error code for the gecko_init
 * function
 *
 * All bluetooth specific initialization
 * code should be here like gecko_init(),
 * gecko_init_whitelisting(),
 * gecko_init_multiprotocol() and so on.
 ******************************************************************************/
extern "C" errorcode_t initialize_bluetooth()
{
    errorcode_t err = gecko_init(&config);

    if (err == bg_err_success)
    {
        gecko_init_multiprotocol(NULL);
    }
    return err;
}

static void initBleConfig(void)
{
    memset(&config, 0, sizeof(gecko_configuration_t));
    config.config_flags              = GECKO_CONFIG_FLAG_RTOS;       /* Check flag options from UG136 */
    config.bluetooth.max_connections = BLE_LAYER_NUM_BLE_ENDPOINTS;  /* Maximum number of simultaneous connections */
    config.bluetooth.heap            = bluetooth_stack_heap;         /* Bluetooth stack memory for connection management */
    config.bluetooth.heap_size       = sizeof(bluetooth_stack_heap); /* Size of Heap */
    config.gattdb                    = &bg_gattdb_data;              /* Pointer to GATT database */
    config.scheduler_callback        = BluetoothLLCallback;
    config.stack_schedule_callback   = BluetoothUpdate;
#if (HAL_PA_ENABLE)
    config.pa.config_enable = 1; /* Set this to be a valid PA config */
#if defined(FEATURE_PA_INPUT_FROM_VBAT)
    config.pa.input = GECKO_RADIO_PA_INPUT_VBAT; /* Configure PA input to VBAT */
#else
    config.pa.input = GECKO_RADIO_PA_INPUT_DCDC; /* Configure PA input to DCDC */
#endif // defined(FEATURE_PA_INPUT_FROM_VBAT)
#endif // (HAL_PA_ENABLE)
    config.mbedtls.flags      = GECKO_MBEDTLS_FLAGS_NO_MBEDTLS_DEVICE_INIT;
    config.mbedtls.dev_number = 0;
}

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;
    errorcode_t ret;

    // Initialize the CHIP BleLayer.
    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);

    memset(mBleConnections, 0, sizeof(mBleConnections));
    memset(mIndConfId, kUnusedIndex, sizeof(mIndConfId));
    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;

    initBleConfig();

    // Start Bluetooth Link Layer and stack tasks
    ret =
        bluetooth_start(CHIP_DEVICE_CONFIG_BLE_LL_TASK_PRIORITY, CHIP_DEVICE_CONFIG_BLE_STACK_TASK_PRIORITY, initialize_bluetooth);

    VerifyOrExit(ret == bg_err_success, err = MapBLEError(ret));

    // Create the Bluetooth Application task
    xTaskCreate(bluetoothStackEventHandler,                                       /* Function that implements the task. */
                CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME,                             /* Text name for the task. */
                CHIP_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE / sizeof(StackType_t), /* Number of indexes in the xStack array. */
                this,                                                             /* Parameter passed into the task. */
                CHIP_DEVICE_CONFIG_BLE_APP_TASK_PRIORITY,                         /* Priority at which the task is created. */
                NULL);                                                            /* Variable to hold the task's data structure. */

    mFlags = kFlag_AdvertisingEnabled;
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    uint16_t numCons = 0;
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mBleConnections[i].allocated)
        {
            numCons++;
        }
    }

    return numCons;
}

void BLEManagerImpl::bluetoothStackEventHandler(void * p_arg)
{
    EventBits_t flags = 0;

    while (1)
    {
        // wait for Bluetooth stack events, do not consume set flag
        flags |= xEventGroupWaitBits(bluetooth_event_flags,            /* The event group being tested. */
                                     BLUETOOTH_EVENT_FLAG_EVT_WAITING, /* The bits within the event group to wait for. */
                                     pdFALSE,                          /* Dont clear flags before returning */
                                     pdFALSE,                          /* Any flag will do, dont wait for all flags to be set */
                                     portMAX_DELAY);                   /* Wait for maximum duration for bit to be set */

        if (flags & BLUETOOTH_EVENT_FLAG_EVT_WAITING)
        {
            flags &= ~BLUETOOTH_EVENT_FLAG_EVT_WAITING;
            xEventGroupClearBits(bluetooth_event_flags, BLUETOOTH_EVENT_FLAG_EVT_WAITING);

            // As this is running in a separate thread, we need to block CHIP from operating,
            // until the events are handled.
            PlatformMgr().LockChipStack();

            // handle bluetooth events
            switch (BGLIB_MSG_ID(bluetooth_evt->header))
            {
            case gecko_evt_system_boot_id: {
                sInstance.HandleBootEvent();
            }
            break;

            case gecko_evt_le_connection_opened_id: {
                sInstance.HandleConnectEvent(bluetooth_evt);
            }
            break;

            case gecko_evt_le_connection_closed_id: {
                sInstance.HandleConnectionCloseEvent(bluetooth_evt);
            }
            break;

            /* This event indicates that a remote GATT client is attempting to write a value of an
             * attribute in to the local GATT database, where the attribute was defined in the GATT
             * XML firmware configuration file to have type="user".  */
            case gecko_evt_gatt_server_attribute_value_id: {
                sInstance.HandleWriteEvent(bluetooth_evt);
            }
            break;

            case gecko_evt_gatt_mtu_exchanged_id: {
                sInstance.UpdateMtu(bluetooth_evt);
            }
            break;

            // confirmation of indication received from remote GATT client
            case gecko_evt_gatt_server_characteristic_status_id: {
                enum gatt_server_characteristic_status_flag StatusFlags;

                StatusFlags = (enum gatt_server_characteristic_status_flag)
                                  bluetooth_evt->data.evt_gatt_server_characteristic_status.status_flags;

                if (gatt_server_confirmation == StatusFlags)
                {
                    sInstance.HandleTxConfirmationEvent(bluetooth_evt);
                }
                else if ((bluetooth_evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_CHIPoBLEChar_Tx) &&
                         (bluetooth_evt->data.evt_gatt_server_characteristic_status.status_flags == gatt_server_client_config))
                {
                    sInstance.HandleTXCharCCCDWrite(bluetooth_evt);
                }
            }
            break;

            /* Software Timer event */
            case gecko_evt_hardware_soft_timer_id: {
                sInstance.HandleSoftTimerEvent(bluetooth_evt);
            }
            break;

            default:
                ChipLogProgress(DeviceLayer, "evt_UNKNOWN id = %08x", BGLIB_MSG_ID(bluetooth_evt->header));
                break;
            }
        }

        PlatformMgr().UnlockChipStack();

        flags = vRaiseEventFlagBasedOnContext(bluetooth_event_flags, BLUETOOTH_EVENT_FLAG_EVT_HANDLED, NULL);
    }
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
        SetFlag(mFlags, kFlag_DeviceNameSet, true);
        ChipLogProgress(DeviceLayer, "Setting device name to : \"%s\"", deviceName);
        gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(deviceName), (uint8_t *) deviceName);
    }
    else
    {
        mDeviceName[0] = 0;
    }
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe: {
        ChipDeviceEvent connEstEvent;

        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLESubscribe");
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
        PlatformMgr().PostEvent(&connEstEvent);
    }
    break;

    case DeviceEventType::kCHIPoBLEUnsubscribe: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEUnsubscribe");
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kCHIPoBLEWriteReceived: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEWriteReceived");
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_RX,
                            event->CHIPoBLEWriteReceived.Data);
    }
    break;

    case DeviceEventType::kCHIPoBLEConnectionError: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEConnectionError");
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
    }
    break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEIndicateConfirm");
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
    }
    break;

    default:
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent default:  event->Type = %d", event->Type);
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
    gecko_msg_le_connection_close_rsp_t * rsp;

    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    rsp = gecko_cmd_le_connection_close(conId);
    err = MapBLEError(rsp->result);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "gecko_cmd_le_connection_close() failed: %s", ErrorStr(err));
    }

    return (err == CHIP_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    CHIPoBLEConState * conState = const_cast<BLEManagerImpl *>(this)->GetConnectionState(conId);
    return (conState != NULL) ? conState->mtu : 0;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBuffer * data)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    CHIPoBLEConState * conState = GetConnectionState(conId);
    gecko_msg_gatt_server_send_characteristic_notification_rsp_t * rsp;
    uint16_t cId        = (UUIDsMatch(&ChipUUID_CHIPoBLEChar_RX, charId) ? gattdb_CHIPoBLEChar_Rx : gattdb_CHIPoBLEChar_Tx);
    uint8_t timerHandle = GetTimerHandle(conId, true);

    VerifyOrExit(((conState != NULL) && (conState->subscribed != 0)), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(timerHandle != kMaxConnections, err = CHIP_ERROR_NO_MEMORY);

    // start timer for light indication confirmation
    gecko_cmd_hardware_set_soft_timer(TIMER_S_2_TIMERTICK(1), timerHandle, true);

    rsp = gecko_cmd_gatt_server_send_characteristic_notification(conId, cId, data->DataLength(), data->Start());

    err = MapBLEError(rsp->result);

exit:
    PacketBuffer::Free(data);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManagerImpl::SendIndication() failed: %s", ErrorStr(err));
        return false;
    }
    return true;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBuffer * pBuf)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     PacketBuffer * pBuf)
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

CHIP_ERROR BLEManagerImpl::MapBLEError(int bleErr)
{
    CHIP_ERROR err;

    if (bleErr == bg_err_success)
    {
        err = CHIP_NO_ERROR;
    }
    else
    {
        err = (CHIP_ERROR) bleErr + CHIP_DEVICE_CONFIG_EFR32_BLE_ERROR_MIN;
    }

    return err;
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if BLE stack is initialized
    VerifyOrExit(GetFlag(mFlags, kFlag_EFRBLEStackInitialized), /* */);

    // Start advertising if needed...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && GetFlag(mFlags, kFlag_AdvertisingEnabled))
    {
        // Start/re-start advertising if not already started, or if there is a pending change
        // to the advertising configuration.
        if (!GetFlag(mFlags, kFlag_Advertising) || GetFlag(mFlags, kFlag_RestartAdvertising))
        {
            err = StartAdvertising();
            SuccessOrExit(err);
        }
    }

    // Otherwise, stop advertising if it is enabled.
    else if (GetFlag(mFlags, kFlag_Advertising))
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

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    struct gecko_msg_le_gap_bt5_set_adv_data_rsp_t * setAdvDataRsp;
    ChipBLEDeviceIdentificationInfo mDeviceIdInfo;
    CHIP_ERROR err;
    uint8_t responseData[MAX_RESPONSE_DATA_LEN];
    uint8_t advData[MAX_ADV_DATA_LEN];
    uint8_t index               = 0;
    uint8_t mDeviceNameLength   = 0;
    uint8_t mDeviceIdInfoLength = 0;

    memset(responseData, 0, MAX_RESPONSE_DATA_LEN);
    memset(advData, 0, MAX_ADV_DATA_LEN);

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(mDeviceIdInfo);
    SuccessOrExit(err);

    if (!GetFlag(mFlags, kFlag_DeviceNameSet))
    {
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04" PRIX32, CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, (uint32_t) 0);

        mDeviceName[kMaxDeviceNameLength] = 0;

        gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(mDeviceName), (uint8_t *) mDeviceName);
    }

    mDeviceNameLength   = strlen(mDeviceName);   // Device Name length + length field
    mDeviceIdInfoLength = sizeof(mDeviceIdInfo); // Servicedatalen + length+ UUID (Short)

    index                 = 0;
    responseData[index++] = 0x02;                     // length
    responseData[index++] = CHIP_ADV_DATA_TYPE_FLAGS; // AD type : flags
    responseData[index++] = CHIP_ADV_DATA_FLAGS;      // AD value

    responseData[index++] = mDeviceNameLength + 1;                // length
    responseData[index++] = CHIP_ADV_DATA_TYPE_NAME;              // AD type : name
    memcpy(&responseData[index], mDeviceName, mDeviceNameLength); // AD value
    index += mDeviceNameLength;

    responseData[index++] = CHIP_ADV_SHORT_UUID_LEN + 1;  // AD length
    responseData[index++] = CHIP_ADV_DATA_TYPE_UUID;      // AD type : uuid
    responseData[index++] = ShortUUID_CHIPoBLEService[0]; // AD value
    responseData[index++] = ShortUUID_CHIPoBLEService[1];

    setAdvDataRsp = gecko_cmd_le_gap_bt5_set_adv_data(CHIP_ADV_CHIPOBLE_SERVICE_HANDLE, CHIP_ADV_SCAN_RESPONSE_DATA, index,
                                                      (uint8_t *) &responseData);

    if (setAdvDataRsp->result != 0)
    {
        err = MapBLEError(setAdvDataRsp->result);
        ChipLogError(DeviceLayer, "gecko_cmd_le_gap_bt5_set_adv_data() failed: %s", ErrorStr(err));
        ExitNow();
    }

    index = 0;

    advData[index++] = mDeviceIdInfoLength + CHIP_ADV_SHORT_UUID_LEN + 1; // AD length
    advData[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;                   // AD type : Service Data
    advData[index++] = ShortUUID_CHIPoBLEService[0];                      // AD value
    advData[index++] = ShortUUID_CHIPoBLEService[1];
    memcpy(&advData[index], (void *) &mDeviceIdInfo, mDeviceIdInfoLength); // AD value
    index += mDeviceIdInfoLength;

    setAdvDataRsp = gecko_cmd_le_gap_bt5_set_adv_data(CHIP_ADV_CHIPOBLE_SERVICE_HANDLE, CHIP_ADV_DATA, index, (uint8_t *) &advData);

    err = MapBLEError(setAdvDataRsp->result);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err;
    struct gecko_msg_le_gap_start_advertising_rsp_t * startAdvRsp;
    uint32_t interval_min;
    uint32_t interval_max;
    uint16_t numConnectionss = NumConnections();
    uint8_t connectableAdv = (numConnectionss < kMaxConnections) ? le_gap_connectable_scannable : le_gap_scannable_non_connectable;

    err = ConfigureAdvertisingData();
    SuccessOrExit(err);

    SetFlag(mFlags, kFlag_Advertising, true);
    ClearFlag(mFlags, kFlag_RestartAdvertising);

    interval_min = interval_max =
        ((numConnectionss == 0 && !ConfigurationMgr().IsPairedToAccount()) || GetFlag(mFlags, kFlag_FastAdvertisingEnabled))
        ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL
        : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL;

    gecko_cmd_le_gap_set_advertise_timing(CHIP_ADV_CHIPOBLE_SERVICE_HANDLE, interval_min, interval_max, 0, 0);

    startAdvRsp = gecko_cmd_le_gap_start_advertising(CHIP_ADV_CHIPOBLE_SERVICE_HANDLE, le_gap_user_data, connectableAdv);

    err = MapBLEError(startAdvRsp->result);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    gecko_msg_le_gap_stop_advertising_rsp_t * rsp;

    if (GetFlag(mFlags, kFlag_Advertising))
    {
        ClearFlag(mFlags, kFlag_Advertising);
        ClearFlag(mFlags, kFlag_RestartAdvertising);

        rsp = gecko_cmd_le_gap_stop_advertising(CHIP_ADV_CHIPOBLE_SERVICE_HANDLE);
        err = MapBLEError(rsp->result);
        SuccessOrExit(err);
    }

exit:
    return err;
}

void BLEManagerImpl::UpdateMtu(volatile struct gecko_cmd_packet * evt)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(evt->data.evt_gatt_mtu_exchanged.connection);
    bleConnState->mtu               = evt->data.evt_gatt_mtu_exchanged.mtu;
    ;
}

void BLEManagerImpl::HandleBootEvent(void)
{
    SetFlag(mFlags, kFlag_EFRBLEStackInitialized, true);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerImpl::HandleConnectEvent(volatile struct gecko_cmd_packet * evt)
{
    struct gecko_msg_le_connection_opened_evt_t * conn_evt = (struct gecko_msg_le_connection_opened_evt_t *) &(evt->data);
    uint8_t connHandle                                     = conn_evt->connection;
    uint8_t bondingHandle                                  = conn_evt->bonding;

    ChipLogProgress(DeviceLayer, "Connect Event for handle : %d", connHandle);

    AddConnection(connHandle, bondingHandle);

    SetFlag(mFlags, kFlag_RestartAdvertising, true);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerImpl::HandleConnectionCloseEvent(volatile struct gecko_cmd_packet * evt)
{
    struct gecko_msg_le_connection_closed_evt_t * conn_evt = (struct gecko_msg_le_connection_closed_evt_t *) &(evt->data);
    uint8_t connHandle                                     = conn_evt->connection;

    ChipLogProgress(DeviceLayer, "Disconnect Event for handle : %d", connHandle);

    if (RemoveConnection(connHandle))
    {
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId = connHandle;

        switch (conn_evt->reason)
        {
        case bg_err_bt_remote_user_terminated:
        case bg_err_bt_remote_device_terminated_connection_due_to_low_resources:
        case bg_err_bt_remote_powering_off:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;

        case bg_err_bt_connection_terminated_by_local_host:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_APP_CLOSED_CONNECTION;
            break;

        default:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
            break;
        }

        ChipLogProgress(DeviceLayer, "BLE GATT connection closed (con %u, reason %u)", connHandle, conn_evt->reason);

        PlatformMgr().PostEvent(&event);

        // Arrange to re-enable connectable advertising in case it was disabled due to the
        // maximum connection limit being reached.
        SetFlag(mFlags, kFlag_RestartAdvertising, true);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

void BLEManagerImpl::HandleWriteEvent(volatile struct gecko_cmd_packet * evt)
{
    uint16_t attribute = evt->data.evt_gatt_server_user_write_request.characteristic;

    ChipLogProgress(DeviceLayer, "Char Write Req, char : %d", attribute);

    if (gattdb_CHIPoBLEChar_Rx == attribute)
    {
        HandleRXCharWrite(evt);
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(volatile struct gecko_cmd_packet * evt)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CHIPoBLEConState * bleConnState;
    bool indicationsEnabled;
    ChipDeviceEvent event;

    bleConnState = GetConnectionState(evt->data.evt_gatt_server_user_write_request.connection);

    VerifyOrExit(bleConnState != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Determine if the client is enabling or disabling indications.
    indicationsEnabled = (evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_indication);

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", indicationsEnabled ? "subscribe" : "unsubscribe");

    if (indicationsEnabled)
    {
        // If indications are not already enabled for the connection...
        if (!bleConnState->subscribed)
        {
            bleConnState->subscribed = 1;
            // Post an event to the CHIP queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
            // whether the client is enabling or disabling indications.
            {
                event.Type                    = DeviceEventType::kCHIPoBLESubscribe;
                event.CHIPoBLESubscribe.ConId = evt->data.evt_gatt_server_user_write_request.connection;
                PlatformMgr().PostEvent(&event);
            }
        }
    }
    else
    {
        bleConnState->subscribed      = 0;
        event.Type                    = DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = evt->data.evt_gatt_server_user_write_request.connection;
        PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleRXCharWrite(volatile struct gecko_cmd_packet * evt)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PacketBuffer * buf;
    uint16_t writeLen = evt->data.evt_gatt_server_user_write_request.value.len;
    uint8_t * data    = (uint8_t *) evt->data.evt_gatt_server_user_write_request.value.data;

    // Copy the data to a PacketBuffer.
    buf = PacketBuffer::New(0);
    VerifyOrExit(buf != NULL, err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(buf->AvailableDataLength() >= writeLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buf->Start(), data, writeLen);
    buf->SetDataLength(writeLen);

    ChipLogDetail(DeviceLayer, "Write request/command received for CHIPoBLE RX characteristic (con %" PRIu16 ", len %" PRIu16 ")",
                  evt->data.evt_gatt_server_user_write_request.connection, buf->DataLength());

    // Post an event to the CHIP queue to deliver the data into the CHIP stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = evt->data.evt_gatt_server_user_write_request.connection;
        event.CHIPoBLEWriteReceived.Data  = buf;
        PlatformMgr().PostEvent(&event);
        buf = NULL;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
    }
    PacketBuffer::Free(buf);
}

void BLEManagerImpl::HandleTxConfirmationEvent(volatile struct gecko_cmd_packet * evt)
{
    ChipDeviceEvent event;
    uint8_t timerHandle = sInstance.GetTimerHandle(evt->data.evt_gatt_server_characteristic_status.connection);

    ChipLogProgress(DeviceLayer, "Tx Confirmation received");

    // stop indication confirmation timer

    if (timerHandle < kMaxConnections)
    {
        ChipLogProgress(DeviceLayer, " stop soft timer");
        gecko_cmd_hardware_set_soft_timer(0, timerHandle, false);
    }

    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = evt->data.evt_gatt_server_characteristic_status.connection;
    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::HandleSoftTimerEvent(volatile struct gecko_cmd_packet * evt)
{
    // BLE Manager starts soft timers with timer handles less than kMaxConnections
    // If we receive a callback for unknown timer handle ignore this.
    if (evt->data.evt_hardware_soft_timer.handle < kMaxConnections)
    {
        ChipLogProgress(DeviceLayer, "BLEManagerImpl::HandleSoftTimerEvent CHIPOBLE_PROTOCOL_ABORT");
        ChipDeviceEvent event;
        event.Type                                                     = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId                            = mIndConfId[evt->data.evt_hardware_soft_timer.handle];
        sInstance.mIndConfId[evt->data.evt_hardware_soft_timer.handle] = kUnusedIndex;
        event.CHIPoBLEConnectionError.Reason                           = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
        PlatformMgr().PostEvent(&event);
    }
}

bool BLEManagerImpl::RemoveConnection(uint8_t connectionHandle)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(connectionHandle, true);
    bool status                     = false;

    if (bleConnState != NULL)
    {
        memset(bleConnState, 0, sizeof(CHIPoBLEConState));
        status = true;
    }

    return status;
}

void BLEManagerImpl::AddConnection(uint8_t connectionHandle, uint8_t bondingHandle)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(connectionHandle, true);

    if (bleConnState != NULL)
    {
        memset(bleConnState, 0, sizeof(CHIPoBLEConState));
        bleConnState->allocated        = 1;
        bleConnState->connectionHandle = connectionHandle;
        bleConnState->bondingHandle    = bondingHandle;
    }
}

BLEManagerImpl::CHIPoBLEConState * BLEManagerImpl::GetConnectionState(uint8_t connectionHandle, bool allocate)
{
    uint8_t freeIndex = kMaxConnections;

    for (uint8_t i = 0; i < kMaxConnections; i++)
    {
        if (mBleConnections[i].allocated == 1)
        {
            if (mBleConnections[i].connectionHandle == connectionHandle)
            {
                return &mBleConnections[i];
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
            return &mBleConnections[freeIndex];
        }

        ChipLogError(DeviceLayer, "Failed to allocate CHIPoBLEConState");
    }

    return NULL;
}

uint8_t BLEManagerImpl::GetTimerHandle(uint8_t connectionHandle, bool allocate)
{
    uint8_t freeIndex = kMaxConnections;

    for (uint8_t i = 0; i < kMaxConnections; i++)
    {
        if (mIndConfId[i] == connectionHandle)
        {
            return i;
        }
        else if (allocate)
        {
            if (i < freeIndex)
            {
                freeIndex = i;
            }
        }
    }

    if (freeIndex < kMaxConnections)
    {
        mIndConfId[freeIndex] = connectionHandle;
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to Save Conn Handle for indication");
    }

    return freeIndex;
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
