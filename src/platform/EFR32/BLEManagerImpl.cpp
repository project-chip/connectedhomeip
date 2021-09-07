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
 *          for the Silicon Labs EFR32 platforms.
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <platform/internal/BLEManager.h>

#include "FreeRTOS.h"
#include "rail.h"
#include "sl_bt_api.h"
#include "sl_bt_stack_config.h"
#include "sl_bt_stack_init.h"
#include "timers.h"
#include <ble/CHIPBleServiceData.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/EFR32/freertos_bluetooth.h>
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

#define CHIP_ADV_DATA 0
#define CHIP_ADV_SCAN_RESPONSE_DATA 1
#define CHIP_ADV_SHORT_UUID_LEN 2

#define MAX_RESPONSE_DATA_LEN 31
#define MAX_ADV_DATA_LEN 31

// Timer Frequency used.
#define TIMER_CLK_FREQ ((uint32_t) 32768)
// Convert msec to timer ticks.
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)
#define TIMER_S_2_TIMERTICK(s) (TIMER_CLK_FREQ * s)

#define BLE_MAX_BUFFER_SIZE (3076)
#define BLE_MAX_ADVERTISERS (1)
#define BLE_CONFIG_MAX_PERIODIC_ADVERTISING_SYNC (0)
#define BLE_CONFIG_MAX_SOFTWARE_TIMERS (4)
#define BLE_CONFIG_MIN_TX_POWER (-30)
#define BLE_CONFIG_MAX_TX_POWER (80)
#define BLE_CONFIG_RF_PATH_GAIN_TX (0)
#define BLE_CONFIG_RF_PATH_GAIN_RX (0)

TimerHandle_t sbleAdvTimeoutTimer; // FreeRTOS sw timer.

/* Bluetooth stack configuration parameters (see "UG136: Silicon Labs Bluetooth C Application Developer's Guide" for
 * details on each parameter) */
static sl_bt_configuration_t config;

/** @brief Table of used BGAPI classes */
static const struct sli_bgapi_class * const bt_class_table[] = { SL_BT_BGAPI_CLASS(system),      SL_BT_BGAPI_CLASS(advertiser),
                                                                 SL_BT_BGAPI_CLASS(gap),         SL_BT_BGAPI_CLASS(scanner),
                                                                 SL_BT_BGAPI_CLASS(connection),  SL_BT_BGAPI_CLASS(gatt),
                                                                 SL_BT_BGAPI_CLASS(gatt_server), SL_BT_BGAPI_CLASS(nvm),
                                                                 SL_BT_BGAPI_CLASS(sm),          NULL };

StackType_t bluetoothEventStack[CHIP_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE / sizeof(StackType_t)];
StaticTask_t bluetoothEventTaskStruct;
static TaskHandle_t BluetoothEventTaskHandle;

const uint8_t UUID_CHIPoBLEService[]       = { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
                                         0x00, 0x10, 0x00, 0x00, 0xF6, 0xFF, 0x00, 0x00 };
const uint8_t ShortUUID_CHIPoBLEService[]  = { 0xF6, 0xFF };
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
 * code should be here like sl_bt_init_stack(),
 * sl_bt_init_multiprotocol() and so on.
 ******************************************************************************/
extern "C" sl_status_t initialize_bluetooth()
{
#if !defined(SL_CATALOG_KERNEL_PRESENT)
    NVIC_ClearPendingIRQ(PendSV_IRQn);
    NVIC_EnableIRQ(PendSV_IRQn);
#endif
    sl_status_t ret = sl_bt_init_stack(&config);
    sl_bt_init_classes(bt_class_table);
    sl_bt_init_multiprotocol();
    return ret;
}

static void initBleConfig(void)
{
    memset(&config, 0, sizeof(sl_bt_configuration_t));
    config.config_flags                = SL_BT_CONFIG_FLAG_RTOS;      /* Check flag options from UG136 */
    config.bluetooth.max_connections   = BLE_LAYER_NUM_BLE_ENDPOINTS; /* Maximum number of simultaneous connections */
    config.bluetooth.max_advertisers   = BLE_MAX_ADVERTISERS;
    config.bluetooth.max_periodic_sync = BLE_CONFIG_MAX_PERIODIC_ADVERTISING_SYNC;
    config.bluetooth.max_buffer_memory = BLE_MAX_BUFFER_SIZE;
    config.gattdb                      = &gattdb; /* Pointer to GATT database */
    config.scheduler_callback          = BluetoothLLCallback;
    config.stack_schedule_callback     = BluetoothUpdate;
    config.max_timers                  = BLE_CONFIG_MAX_SOFTWARE_TIMERS;
    config.rf.tx_gain                  = BLE_CONFIG_RF_PATH_GAIN_TX;
    config.rf.rx_gain                  = BLE_CONFIG_RF_PATH_GAIN_RX;
    config.rf.tx_min_power             = BLE_CONFIG_MIN_TX_POWER;
    config.rf.tx_max_power             = BLE_CONFIG_MAX_TX_POWER;
#if (HAL_PA_ENABLE)
    config.pa.config_enable = 1; /* Set this to be a valid PA config */
#if defined(FEATURE_PA_INPUT_FROM_VBAT)
    config.pa.input = SL_BT_RADIO_PA_INPUT_VBAT; /* Configure PA input to VBAT */
#else
    config.pa.input = SL_BT_RADIO_PA_INPUT_DCDC; /* Configure PA input to DCDC */
#endif // defined(FEATURE_PA_INPUT_FROM_VBAT)
#endif // (HAL_PA_ENABLE)
}

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;
    sl_status_t ret;

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
    BluetoothEventTaskHandle =
        xTaskCreateStatic(bluetoothStackEventHandler,               /* Function that implements the task. */
                          CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME,     /* Text name for the task. */
                          ArraySize(bluetoothEventStack),           /* Number of indexes in the xStack array. */
                          this,                                     /* Parameter passed into the task. */
                          CHIP_DEVICE_CONFIG_BLE_APP_TASK_PRIORITY, /* Priority at which the task is created. */
                          bluetoothEventStack,                      /* Pointer to task heap */
                          &bluetoothEventTaskStruct);               /* Variable that holds the task struct */

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
        flags = xEventGroupWaitBits(bluetooth_event_flags,            /* The event group being tested. */
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
            switch (SL_BT_MSG_ID(bluetooth_evt->header))
            {
            case sl_bt_evt_system_boot_id: {
                ChipLogProgress(DeviceLayer, "Bluetooth stack booted: v%d.%d.%d-b%d", bluetooth_evt->data.evt_system_boot.major,
                                bluetooth_evt->data.evt_system_boot.minor, bluetooth_evt->data.evt_system_boot.patch,
                                bluetooth_evt->data.evt_system_boot.build);
                sInstance.HandleBootEvent();

                RAIL_Version_t railVer;
                RAIL_GetVersion(&railVer, true);
                ChipLogProgress(DeviceLayer, "RAIL version:, v%d.%d.%d-b%d", railVer.major, railVer.minor, railVer.rev,
                                railVer.build);
            }
            break;

            case sl_bt_evt_connection_opened_id: {
                sInstance.HandleConnectEvent(bluetooth_evt);
            }
            break;
            case sl_bt_evt_connection_parameters_id: {
                // ChipLogProgress(DeviceLayer, "Connection parameter ID received. Nothing to do");
            }
            break;
            case sl_bt_evt_connection_phy_status_id: {
                // ChipLogProgress(DeviceLayer, "PHY update procedure is completed");
            }
            break;
            case sl_bt_evt_connection_closed_id: {
                sInstance.HandleConnectionCloseEvent(bluetooth_evt);
            }
            break;

            /* This event indicates that a remote GATT client is attempting to write a value of an
             * attribute in to the local GATT database, where the attribute was defined in the GATT
             * XML firmware configuration file to have type="user".  */
            case sl_bt_evt_gatt_server_attribute_value_id: {
                sInstance.HandleWriteEvent(bluetooth_evt);
            }
            break;

            case sl_bt_evt_gatt_mtu_exchanged_id: {
                sInstance.UpdateMtu(bluetooth_evt);
            }
            break;

            // confirmation of indication received from remote GATT client
            case sl_bt_evt_gatt_server_characteristic_status_id: {
                sl_bt_gatt_server_characteristic_status_flag_t StatusFlags;

                StatusFlags = (sl_bt_gatt_server_characteristic_status_flag_t)
                                  bluetooth_evt->data.evt_gatt_server_characteristic_status.status_flags;

                if (sl_bt_gatt_server_confirmation == StatusFlags)
                {
                    sInstance.HandleTxConfirmationEvent(bluetooth_evt->data.evt_gatt_server_characteristic_status.connection);
                }
                else if ((bluetooth_evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_CHIPoBLEChar_Tx) &&
                         (bluetooth_evt->data.evt_gatt_server_characteristic_status.status_flags == gatt_server_client_config))
                {
                    sInstance.HandleTXCharCCCDWrite(bluetooth_evt);
                }
            }
            break;

            /* Software Timer event */
            case sl_bt_evt_system_soft_timer_id: {
                sInstance.HandleSoftTimerEvent(bluetooth_evt);
            }
            break;

            default:
                ChipLogProgress(DeviceLayer, "evt_UNKNOWN id = %08" PRIx32, SL_BT_MSG_ID(bluetooth_evt->header));
                break;
            }
        }

        PlatformMgr().UnlockChipStack();

        vRaiseEventFlagBasedOnContext(bluetooth_event_flags, BLUETOOTH_EVENT_FLAG_EVT_HANDLED);
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
    if (strlen(mDeviceName) >= bufSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    strcpy(buf, mDeviceName);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    sl_status_t ret;
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
        strcpy(mDeviceName, deviceName);
        mFlags.Set(Flags::kDeviceNameSet);
        ChipLogProgress(DeviceLayer, "Setting device name to : \"%s\"", deviceName);
        static_assert(kMaxDeviceNameLength <= UINT16_MAX, "deviceName length might not fit in a uint8_t");
        ret = sl_bt_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(deviceName), (uint8_t *) deviceName);
        if (ret != SL_STATUS_OK)
        {
            err = MapBLEError(ret);
            ChipLogError(DeviceLayer, "sl_bt_gatt_server_write_attribute_value() failed: %s", ErrorStr(err));
            return err;
        }
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
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
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

    case DeviceEventType::kCHIPoBLENotifyConfirm: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLENotifyConfirm");
        HandleTxConfirmationEvent(event->CHIPoBLENotifyConfirm.ConId);
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
    sl_status_t ret;

    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    ret = sl_bt_connection_close(conId);
    err = MapBLEError(ret);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "sl_bt_connection_close() failed: %s", ErrorStr(err));
    }

    return (err == CHIP_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    CHIPoBLEConState * conState = const_cast<BLEManagerImpl *>(this)->GetConnectionState(conId);
    return (conState != NULL) ? conState->mtu : 0;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    CHIP_ERROR err              = CHIP_NO_ERROR;
    CHIPoBLEConState * conState = GetConnectionState(conId);
    sl_status_t ret;
    uint16_t cId        = (UUIDsMatch(&ChipUUID_CHIPoBLEChar_RX, charId) ? gattdb_CHIPoBLEChar_Rx : gattdb_CHIPoBLEChar_Tx);
    uint8_t timerHandle = GetTimerHandle(conId, true);
    ChipDeviceEvent event;

    VerifyOrExit(((conState != NULL) && (conState->subscribed != 0)), err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(timerHandle != kMaxConnections, err = CHIP_ERROR_NO_MEMORY);

    // start timer for light notification confirmation. Long delay for spake2 indication
    sl_bt_system_set_soft_timer(TIMER_S_2_TIMERTICK(6), timerHandle, true);

    ret = sl_bt_gatt_server_send_notification(conId, cId, (data->DataLength()), data->Start());
    err = MapBLEError(ret);

    if (err == CHIP_NO_ERROR)
    {
        event.Type                        = DeviceEventType::kCHIPoBLENotifyConfirm;
        event.CHIPoBLENotifyConfirm.ConId = conId;
        PlatformMgr().PostEvent(&event);
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

CHIP_ERROR BLEManagerImpl::MapBLEError(int bleErr)
{
    switch (bleErr)
    {
    case SL_STATUS_OK:
        return CHIP_NO_ERROR;
    case SL_STATUS_BT_ATT_INVALID_ATT_LENGTH:
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    case SL_STATUS_INVALID_PARAMETER:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case SL_STATUS_INVALID_STATE:
        return CHIP_ERROR_INCORRECT_STATE;
    default:
        return CHIP_ERROR(ChipError::Range::kPlatform, bleErr + CHIP_DEVICE_CONFIG_EFR32_BLE_ERROR_MIN);
    }
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if BLE stack is initialized
    VerifyOrExit(mFlags.Has(Flags::kEFRBLEStackInitialized), /* */);

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

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    sl_status_t ret;
    ChipBLEDeviceIdentificationInfo mDeviceIdInfo;
    CHIP_ERROR err;
    uint8_t responseData[MAX_RESPONSE_DATA_LEN];
    uint8_t advData[MAX_ADV_DATA_LEN];
    uint32_t index              = 0;
    uint32_t mDeviceNameLength  = 0;
    uint8_t mDeviceIdInfoLength = 0;

    VerifyOrExit((kMaxDeviceNameLength + 1) < UINT8_MAX, err = CHIP_ERROR_INVALID_ARGUMENT);

    memset(responseData, 0, MAX_RESPONSE_DATA_LEN);
    memset(advData, 0, MAX_ADV_DATA_LEN);

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(mDeviceIdInfo);
    SuccessOrExit(err);

    if (!mFlags.Has(Flags::kDeviceNameSet))
    {
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04" PRIX32, CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, (uint32_t) 0);

        mDeviceName[kMaxDeviceNameLength] = 0;
        mDeviceNameLength                 = strlen(mDeviceName);

        VerifyOrExit(mDeviceNameLength < kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);

        ret = sl_bt_gatt_server_write_attribute_value(gattdb_device_name, 0, mDeviceNameLength, (uint8_t *) mDeviceName);
        if (ret != SL_STATUS_OK)
        {
            err = MapBLEError(ret);
            ChipLogError(DeviceLayer, "sl_bt_gatt_server_write_attribute_value() failed: %s", ErrorStr(err));
            return err;
        }
    }

    mDeviceNameLength = strlen(mDeviceName); // Device Name length + length field
    VerifyOrExit(mDeviceNameLength < kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);

    mDeviceIdInfoLength = sizeof(mDeviceIdInfo); // Servicedatalen + length+ UUID (Short)
    static_assert(sizeof(mDeviceIdInfo) + CHIP_ADV_SHORT_UUID_LEN + 1 <= UINT8_MAX, "Our length won't fit in a uint8_t");
    static_assert(2 + CHIP_ADV_SHORT_UUID_LEN + sizeof(mDeviceIdInfo) + 1 <= MAX_ADV_DATA_LEN, "Our buffer is not big enough");

    index            = 0;
    advData[index++] = 0x02;                                                                    // length
    advData[index++] = CHIP_ADV_DATA_TYPE_FLAGS;                                                // AD type : flags
    advData[index++] = CHIP_ADV_DATA_FLAGS;                                                     // AD value
    advData[index++] = static_cast<uint8_t>(mDeviceIdInfoLength + CHIP_ADV_SHORT_UUID_LEN + 1); // AD length
    advData[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;                                         // AD type : Service Data
    advData[index++] = ShortUUID_CHIPoBLEService[0];                                            // AD value
    advData[index++] = ShortUUID_CHIPoBLEService[1];
    memcpy(&advData[index], (void *) &mDeviceIdInfo, mDeviceIdInfoLength); // AD value
    index += mDeviceIdInfoLength;

    advData[index++] = static_cast<uint8_t>(mDeviceNameLength + 1); // length
    advData[index++] = CHIP_ADV_DATA_TYPE_NAME;                     // AD type : name
    memcpy(&advData[index], mDeviceName, mDeviceNameLength);        // AD value
    index += mDeviceNameLength;

    if (0xff != advertising_set_handle)
    {
        sl_bt_advertiser_delete_set(advertising_set_handle);
        advertising_set_handle = 0xff;
    }

    ret = sl_bt_advertiser_create_set(&advertising_set_handle);
    if (ret != SL_STATUS_OK)
    {
        err = MapBLEError(ret);
        ChipLogError(DeviceLayer, "sl_bt_advertiser_create_set() failed: %s", ErrorStr(err));
        ExitNow();
    }
    ret = sl_bt_advertiser_set_data(advertising_set_handle, CHIP_ADV_DATA, index, (uint8_t *) advData);

    if (ret != SL_STATUS_OK)
    {
        err = MapBLEError(ret);
        ChipLogError(DeviceLayer, "sl_bt_advertiser_set_data() failed: %s", ErrorStr(err));
        ExitNow();
    }

    index = 0;

    responseData[index++] = CHIP_ADV_SHORT_UUID_LEN + 1;  // AD length
    responseData[index++] = CHIP_ADV_DATA_TYPE_UUID;      // AD type : uuid
    responseData[index++] = ShortUUID_CHIPoBLEService[0]; // AD value
    responseData[index++] = ShortUUID_CHIPoBLEService[1];

    ret = sl_bt_advertiser_set_data(advertising_set_handle, CHIP_ADV_SCAN_RESPONSE_DATA, index, (uint8_t *) responseData);

    if (ret != SL_STATUS_OK)
    {
        err = MapBLEError(ret);
        ChipLogError(DeviceLayer, "sl_bt_advertiser_set_data() failed: %s", ErrorStr(err));
        ExitNow();
    }

    err = MapBLEError(ret);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err;
    sl_status_t ret;
    uint32_t interval_min;
    uint32_t interval_max;
    uint32_t BleAdvTimeoutMs;
    uint16_t numConnectionss = NumConnections();
    uint8_t connectableAdv =
        (numConnectionss < kMaxConnections) ? sl_bt_advertiser_connectable_scannable : sl_bt_advertiser_scannable_non_connectable;

    // If already advertising, stop it, before changing values
    if (mFlags.Has(Flags::kAdvertising))
    {
        sl_bt_advertiser_stop(advertising_set_handle);
    }
    else
    {
        ChipLogDetail(DeviceLayer, "Start BLE advertissement");
    }

#ifndef EFR32MG24
    // set_random_address call causes problems with MG24 family.
    // Todo fix in GSDK.
    const uint8_t kResolvableRandomAddrType = 2; // Private resolvable random address type
    bd_addr unusedBdAddr;                        // We can ignore this field when setting random address.
    sl_bt_advertiser_set_random_address(advertising_set_handle, kResolvableRandomAddrType, unusedBdAddr, &unusedBdAddr);
    (void) unusedBdAddr;
#endif

    err = ConfigureAdvertisingData();
    SuccessOrExit(err);

    mFlags.Clear(Flags::kRestartAdvertising);

    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        interval_min    = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
        interval_max    = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
        BleAdvTimeoutMs = CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME;
    }
    else
    {
        interval_min    = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        interval_max    = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
        BleAdvTimeoutMs = CHIP_DEVICE_CONFIG_BLE_ADVERTISING_TIMEOUT;
    }

    ret = sl_bt_advertiser_set_timing(advertising_set_handle, interval_min, interval_max, 0, 0);
    err = MapBLEError(ret);
    SuccessOrExit(err);

    sl_bt_advertiser_set_configuration(advertising_set_handle, 1);
    ret = sl_bt_advertiser_start(advertising_set_handle, sl_bt_advertiser_user_data, connectableAdv);

    if (SL_STATUS_OK == ret)
    {
        StartBleAdvTimeoutTimer(BleAdvTimeoutMs);
        mFlags.Set(Flags::kAdvertising);
    }

    err = MapBLEError(ret);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    sl_status_t ret;

    if (mFlags.Has(Flags::kAdvertising))
    {
        mFlags.Clear(Flags::kAdvertising).Clear(Flags::kRestartAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled, true);

        ret = sl_bt_advertiser_stop(advertising_set_handle);
        sl_bt_advertiser_delete_set(advertising_set_handle);
        advertising_set_handle = 0xff;
        err                    = MapBLEError(ret);
        SuccessOrExit(err);

        CancelBleAdvTimeoutTimer();
    }

exit:
    return err;
}

void BLEManagerImpl::UpdateMtu(volatile sl_bt_msg_t * evt)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(evt->data.evt_gatt_mtu_exchanged.connection);
    if (bleConnState != NULL)
    {
        // bleConnState->MTU is a 10-bit field inside a uint16_t.  We're
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
        bleConnState->mtu = evt->data.evt_gatt_mtu_exchanged.mtu;
#pragma GCC diagnostic pop
        ;
    }
}

void BLEManagerImpl::HandleBootEvent(void)
{
    mFlags.Set(Flags::kEFRBLEStackInitialized);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerImpl::HandleConnectEvent(volatile sl_bt_msg_t * evt)
{
    sl_bt_evt_connection_opened_t * conn_evt = (sl_bt_evt_connection_opened_t *) &(evt->data);
    uint8_t connHandle                       = conn_evt->connection;
    uint8_t bondingHandle                    = conn_evt->bonding;

    ChipLogProgress(DeviceLayer, "Connect Event for handle : %d", connHandle);

    AddConnection(connHandle, bondingHandle);

    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerImpl::HandleConnectionCloseEvent(volatile sl_bt_msg_t * evt)
{
    sl_bt_evt_connection_closed_t * conn_evt = (sl_bt_evt_connection_closed_t *) &(evt->data);
    uint8_t connHandle                       = conn_evt->connection;

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
        mFlags.Set(Flags::kRestartAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

void BLEManagerImpl::HandleWriteEvent(volatile sl_bt_msg_t * evt)
{
    uint16_t attribute = evt->data.evt_gatt_server_user_write_request.characteristic;

    ChipLogProgress(DeviceLayer, "Char Write Req, char : %d", attribute);

    if (gattdb_CHIPoBLEChar_Rx == attribute)
    {
        HandleRXCharWrite(evt);
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(volatile sl_bt_msg_t * evt)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CHIPoBLEConState * bleConnState;
    bool isDisabled;
    ChipDeviceEvent event;

    bleConnState = GetConnectionState(evt->data.evt_gatt_server_user_write_request.connection);
    VerifyOrExit(bleConnState != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Determine if the client is enabling or disabling notification/indication.
    isDisabled = (evt->data.evt_gatt_server_characteristic_status.client_config_flags == sl_bt_gatt_disable);

    ChipLogProgress(DeviceLayer, "HandleTXcharCCCDWrite - Config Flags value : %d",
                    evt->data.evt_gatt_server_characteristic_status.client_config_flags);
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

void BLEManagerImpl::HandleRXCharWrite(volatile sl_bt_msg_t * evt)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;
    uint16_t writeLen = evt->data.evt_gatt_server_user_write_request.value.len;
    uint8_t * data    = (uint8_t *) evt->data.evt_gatt_server_user_write_request.value.data;

    // Copy the data to a packet buffer.
    buf = System::PacketBufferHandle::NewWithData(data, writeLen, 0, 0);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    ChipLogDetail(DeviceLayer, "Write request/command received for CHIPoBLE RX characteristic (con %" PRIu16 ", len %" PRIu16 ")",
                  evt->data.evt_gatt_server_user_write_request.connection, buf->DataLength());

    // Post an event to the CHIP queue to deliver the data into the CHIP stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = evt->data.evt_gatt_server_user_write_request.connection;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleTxConfirmationEvent(BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event;
    uint8_t timerHandle = sInstance.GetTimerHandle(conId);

    ChipLogProgress(DeviceLayer, "Tx Confirmation received");

    // stop indication confirmation timer
    if (timerHandle < kMaxConnections)
    {
        ChipLogProgress(DeviceLayer, " stop soft timer");
        sl_bt_system_set_soft_timer(0, timerHandle, false);
    }

    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = conId;
    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::HandleSoftTimerEvent(volatile sl_bt_msg_t * evt)
{
    // BLE Manager starts soft timers with timer handles less than kMaxConnections
    // If we receive a callback for unknown timer handle ignore this.
    if (evt->data.evt_system_soft_timer.handle < kMaxConnections)
    {
        ChipLogProgress(DeviceLayer, "BLEManagerImpl::HandleSoftTimerEvent CHIPOBLE_PROTOCOL_ABORT");
        ChipDeviceEvent event;
        event.Type                                                   = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId                          = mIndConfId[evt->data.evt_system_soft_timer.handle];
        sInstance.mIndConfId[evt->data.evt_system_soft_timer.handle] = kUnusedIndex;
        event.CHIPoBLEConnectionError.Reason                         = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
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

void BLEManagerImpl::BleAdvTimeoutHandler(TimerHandle_t xTimer)
{
    if (BLEMgrImpl().mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Start slow advertissment");
        BLEMgr().SetAdvertisingMode(BLEAdvertisingMode::kSlowAdvertising);
    }
    else if (BLEMgrImpl().mFlags.Has(Flags::kAdvertising))
    {
        // Advertisement time expired. Stop advertising
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Stop advertissement");
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

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
