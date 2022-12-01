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
#define RSI_BLE_ENABLE 1

//#include "rail.h"
extern "C" {
#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"
#include "timers.h"
#include "wfx_host_events.h"
#include "wfx_rsi.h"
#include "wfx_sl_ble_init.h"
#include <rsi_driver.h>
#include <rsi_utils.h>
#include <stdbool.h>
}
#include <ble/CHIPBleServiceData.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <string.h>

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#endif

// static int32_t handleTxConfirmationFlag = 0;
extern uint16_t rsi_ble_measurement_hndl;
extern rsi_ble_event_conn_status_t conn_event_to_app;
extern sl_wfx_msg_t event_msg;

// StaticTask_t busInitTaskStruct;

/* wfxRsi Task will use as its stack */
// StackType_t wfxRsiInitTaskStack[WFX_RSI_TASK_SZ] = { 0 };

using namespace ::chip;
using namespace ::chip::Ble;

void rsi_ble_event_handling_task(void)
{
    int32_t event_id;

    // int32_t event_id;
    WFX_RSI_LOG("StartAdvertising");
    //    chip::DeviceLayer::Internal::BLEManagerImpl().StartAdvertising(); //TODO:: Called on after init of module

    // Application event map
    while (1)
    {
        // checking for events list
        event_id = rsi_ble_app_get_event();
        if (event_id == -1)
        {
            continue;
        }
        switch (event_id)
        {
        case RSI_BLE_CONN_EVENT: {
            rsi_ble_app_clear_event(RSI_BLE_CONN_EVENT);
            chip::DeviceLayer::Internal::BLEMgrImpl().HandleConnectEvent();
            WFX_RSI_LOG(" RSI_BLE : Module got connected");
        }
        break;
        case RSI_BLE_DISCONN_EVENT: {
            // event invokes when disconnection was completed
            WFX_RSI_LOG(" RSI_BLE : Module got Disconnected");
            chip::DeviceLayer::Internal::BLEMgrImpl().HandleConnectionCloseEvent(event_msg.reason);
            // clear the served event
            rsi_ble_app_clear_event(RSI_BLE_DISCONN_EVENT);
        }
        break;
        case RSI_BLE_MTU_EVENT: {
            // event invokes when write/notification events received
            WFX_RSI_LOG("RSI_BLE:: RSI_BLE_MTU_EVENT");
            chip::DeviceLayer::Internal::BLEMgrImpl().UpdateMtu(event_msg.rsi_ble_mtu);
            // clear the served event
            rsi_ble_app_clear_event(RSI_BLE_MTU_EVENT);
        }
        break;
        case RSI_BLE_GATT_WRITE_EVENT: {
            // event invokes when write/notification events received
            WFX_RSI_LOG("RSI_BLE : RSI_BLE_GATT_WRITE_EVENT");
            chip::DeviceLayer::Internal::BLEMgrImpl().HandleWriteEvent(event_msg.rsi_ble_write);
            // clear the served event
            rsi_ble_app_clear_event(RSI_BLE_GATT_WRITE_EVENT);
        }
        break;
        case RSI_BLE_GATT_INDICATION_CONFIRMATION: {
            WFX_RSI_LOG("RSI_BLE : indication confirmation");
            chip::DeviceLayer::Internal::BLEMgrImpl().HandleTxConfirmationEvent(1);
            rsi_ble_app_clear_event(RSI_BLE_GATT_INDICATION_CONFIRMATION);
        }
        break;

        case RSI_BLE_RESP_ATT_VALUE: {
            WFX_RSI_LOG("RSI_BLE : RESP_ATT confirmation");
        }
        default:
            break;
        }
    }

    WFX_RSI_LOG("%s  END", __func__);
}

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

// Default Connection  parameters
#define BLE_CONFIG_MIN_INTERVAL (16) // Time = Value x 1.25 ms = 30ms
#define BLE_CONFIG_MAX_INTERVAL (80) // Time = Value x 1.25 ms = 100ms
#define BLE_CONFIG_LATENCY (0)
#define BLE_CONFIG_TIMEOUT (100)          // Time = Value x 10 ms = 1s
#define BLE_CONFIG_MIN_CE_LENGTH (0)      // Leave to min value
#define BLE_CONFIG_MAX_CE_LENGTH (0xFFFF) // Leave to max value

#define BLE__DEFAULT_TIMER_PERIOD 1

TimerHandle_t sbleAdvTimeoutTimer; // FreeRTOS sw timer.

const uint8_t UUID_CHIPoBLEService[]       = { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
                                         0x00, 0x10, 0x00, 0x00, 0xF6, 0xFF, 0x00, 0x00 };
const uint8_t ShortUUID_CHIPoBLEService[]  = { 0xF6, 0xFF };
const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;
    ChipLogProgress(DeviceLayer, "%s Start ", __func__);

    //    wfx_rsi.init_task = xTaskCreateStatic((TaskFunction_t) wfx_sl_module_init, "init_task", WFX_RSI_TASK_SZ, NULL, 1,
    //    wfxRsiInitTaskStack, &busInitTaskStruct);

    if (NULL == wfx_rsi.init_task)
    {
        WFX_RSI_LOG("%s: error: failed to create task.", __func__);
    }

    // Initialize the CHIP BleLayer.
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    return err;

    ble_rsi_task();
    memset(mBleConnections, 0, sizeof(mBleConnections));
    memset(mIndConfId, kUnusedIndex, sizeof(mIndConfId));
    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;

    // Create FreeRTOS sw timer for BLE timeouts and interval change.
    sbleAdvTimeoutTimer = xTimerCreate("BleAdvTimer",             // Just a text name, not used by the RTOS kernel
                                       BLE__DEFAULT_TIMER_PERIOD, // == default timer period (mS)
                                       false,                     // no timer reload (==one-shot)
                                       (void *) this,             // init timer id = ble obj context
                                       BleAdvTimeoutHandler       // timer callback handler
    );

    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    ChipLogProgress(DeviceLayer, "%s END ", __func__);
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
    ChipLogProgress(DeviceLayer, "_SetDeviceName Started");
    rsi_bt_set_local_name((uint8_t *) RSI_BLE_DEV_NAME);
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported)
    {
        ChipLogProgress(DeviceLayer, "_SetDeviceName CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    if (deviceName != NULL && deviceName[0] != 0)
    {
        if (strlen(deviceName) >= kMaxDeviceNameLength)
        {
            ChipLogProgress(DeviceLayer, "_SetDeviceName CHIP_ERROR_INVALID_ARGUMENT");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        strcpy(mDeviceName, deviceName);
        mFlags.Set(Flags::kDeviceNameSet);
        mFlags.Set(Flags::kRestartAdvertising);
        ChipLogProgress(DeviceLayer, "Setting device name to : \"%s\"", mDeviceName);
    }
    else
    {
        mDeviceName[0] = 0;
    }
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
    ChipLogProgress(DeviceLayer, "_SetDeviceName Ended");
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
        PlatformMgr().PostEventOrDie(&connEstEvent);
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
    //    int32_t ret;

    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    // ret = rsi_ble_disconnect(1);
    //    err = MapBLEError(ret);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "sl_bt_connection_close() failed: %s", ErrorStr(err));
    }

    return true; //(err == CHIP_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    CHIPoBLEConState * conState = const_cast<BLEManagerImpl *>(this)->GetConnectionState(conId);
    return (conState != NULL) ? conState->mtu : 0;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    int32_t status = 0;
    WFX_RSI_LOG("In send indication");
    status = rsi_ble_indicate_value(event_msg.resp_enh_conn.dev_addr, event_msg.rsi_ble_measurement_hndl, (data->DataLength()),
                                    data->Start());
    if (status != RSI_SUCCESS)
    {
        WFX_RSI_LOG("indication %d failed with error code %lx ", status);
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
    case SL_STATUS_NOT_SUPPORTED:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    default:
        return CHIP_ERROR(ChipError::Range::kPlatform, bleErr + CHIP_DEVICE_CONFIG_SILABS_BLE_ERROR_MIN);
    }
}

void BLEManagerImpl::DriveBLEState(void)
{

    ChipLogProgress(DeviceLayer, "DriveBLEState starting");
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if BLE stack is initialized
    // VerifyOrExit(mFlags.Has(Flags::kEFRBLEStackInitialized), /* */);

    ChipLogProgress(DeviceLayer, "Start advertising if needed...");
    // Start advertising if needed...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && mFlags.Has(Flags::kAdvertisingEnabled) &&
        NumConnections() < kMaxConnections)
    {

        ChipLogProgress(DeviceLayer, "Start/re-start advertising if not already started, or if there is a pending change");
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
    ChipLogProgress(DeviceLayer, "DriveBLEState End");
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{

    ChipBLEDeviceIdentificationInfo mDeviceIdInfo;
    CHIP_ERROR err;
    int32_t result;
    uint8_t responseData[MAX_RESPONSE_DATA_LEN];
    uint8_t advData[MAX_ADV_DATA_LEN];
    uint32_t index              = 0;
    uint32_t mDeviceNameLength  = 0;
    uint8_t mDeviceIdInfoLength = 0;
    err                         = ConfigurationMgr().GetBLEDeviceIdentificationInfo(mDeviceIdInfo);

    ChipLogProgress(DeviceLayer, "ConfigureAdvertisingData start");

    VerifyOrExit((kMaxDeviceNameLength + 1) < UINT8_MAX, err = CHIP_ERROR_INVALID_ARGUMENT);

    memset(responseData, 0, MAX_RESPONSE_DATA_LEN);
    memset(advData, 0, MAX_ADV_DATA_LEN);

    SuccessOrExit(err);

    if (!mFlags.Has(Flags::kDeviceNameSet))
    {
        uint16_t discriminator;
        SuccessOrExit(err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));

        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);

        mDeviceName[kMaxDeviceNameLength] = 0;
        mDeviceNameLength                 = strlen(mDeviceName);

        VerifyOrExit(mDeviceNameLength < kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);
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

    // TODO:: replace the hardcoded values by calling the GetBLEDeviceIdentificationInfo
    advData[index++] = 0;   // OpCode
    advData[index++] = 0;   // DeviceDiscriminatorAndAdvVersion []
    advData[index++] = 15;  // DeviceDiscriminatorAndAdvVersion []
    advData[index++] = 241; // DeviceVendorId []
    advData[index++] = 255; // DeviceVendorId []
    advData[index++] = 5;   // DeviceProductId[]
    advData[index++] = 128; // DeviceProductId[]
    advData[index++] = 0;   // AdditionalDataFlag

    //! prepare advertise data //local/device name
    advData[index++] = strlen(RSI_BLE_DEV_NAME) + 1;
    advData[index++] = 9;

    memcpy(&advData[index], RSI_BLE_DEV_NAME, strlen(RSI_BLE_DEV_NAME)); // AD value
    index += strlen(RSI_BLE_DEV_NAME);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    ReturnErrorOnFailure(EncodeAdditionalDataTlv());
#endif

    result = rsi_ble_set_advertise_data(advData, index);
    if (result != SL_STATUS_OK)
    {
        err = MapBLEError(result);
        ChipLogError(DeviceLayer, "rsi_ble_set_advertise_data() failed: %ld", result);
        ExitNow();
    }
    else
    {
        ChipLogError(DeviceLayer, "rsi_ble_set_advertise_data() success: %ld", result);
    }

    err = MapBLEError(result);

    ChipLogProgress(DeviceLayer, "ConfigureAdvertisingData End");
exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err;
    int32_t status = 0;

    ChipLogProgress(DeviceLayer, "StartAdvertising start");

    // If already advertising, stop it, before changing values
    if (mFlags.Has(Flags::kAdvertising))
    {
        // sl_bt_advertiser_stop(advertising_set_handle);
    }
    else
    {
        ChipLogDetail(DeviceLayer, "Start BLE advertissement");
    }

    //    bd_addr unusedBdAddr; // We can ignore this field when setting random address.

    //    (void) unusedBdAddr;

    err = ConfigureAdvertisingData();
    SuccessOrExit(err);

    mFlags.Clear(Flags::kRestartAdvertising);

    sl_wfx_mac_address_t macaddr;
    wfx_get_wifi_mac_addr(SL_WFX_STA_INTERFACE, &macaddr);

    //! Set local name
    status = rsi_ble_start_advertising();
    if (status == RSI_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "rsi_ble_start_advertising Success");

        if (mFlags.Has(Flags::kFastAdvertisingEnabled))
        {
            StartBleAdvTimeoutTimer(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
        }
        mFlags.Set(Flags::kAdvertising);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "rsi_ble_start_advertising Failed with status: %lx", status);
    }

exit:
    ChipLogError(DeviceLayer, "StartAdvertising() End error: %s", ErrorStr(err));
    return CHIP_NO_ERROR; // err;
}

// TODO:: Implementation need to be done.
CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // sl_status_t ret;

    if (mFlags.Has(Flags::kAdvertising))
    {
        mFlags.Clear(Flags::kAdvertising).Clear(Flags::kRestartAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled, true);

        // ret = sl_bt_advertiser_stop(advertising_set_handle);
        // sl_bt_advertiser_delete_set(advertising_set_handle);
        advertising_set_handle = 0xff;
        // err                    = MapBLEError(ret);
        //        SuccessOrExit(err);

        CancelBleAdvTimeoutTimer();
    }

    // exit:
    return err;
}

void BLEManagerImpl::UpdateMtu(rsi_ble_event_mtu_t evt)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(event_msg.connectionHandle);
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
        ChipLogProgress(DeviceLayer, "DriveBLEState UpdateMtu %d", evt.mtu_size);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
        bleConnState->mtu = evt.mtu_size;
#pragma GCC diagnostic pop
        ;
    }
}

void BLEManagerImpl::HandleBootEvent(void)
{
    mFlags.Set(Flags::kEFRBLEStackInitialized);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerImpl::HandleConnectEvent(void)
{
    ChipLogProgress(DeviceLayer, "Connect Event for handle : %d", event_msg.connectionHandle);
    AddConnection(event_msg.connectionHandle, event_msg.bondingHandle);

    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

// TODO:: Implementation need to be done.
void BLEManagerImpl::HandleConnectionCloseEvent(uint16_t reason)
{
    uint8_t connHandle = 1;

    ChipLogProgress(DeviceLayer, "Disconnect Event for handle : %d", connHandle);

    if (RemoveConnection(connHandle))
    {
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId = connHandle;

        //        switch (reason)
        //        {
        //
        //        case RSI_REMOTE_DEV_TERMINATE_CONN:
        //        case RSI_BT_CTRL_REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES:
        //        case RSI_BT_CTRL_REMOTE_POWERING_OFF:
        //           event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
        //        break;
        //        default:
        //           event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
        //        }

        //        ChipLogProgress(DeviceLayer, "BLE GATT connection closed (con %u, reason %u)", connHandle, conn_evt->reason);

        PlatformMgr().PostEventOrDie(&event);

        // Arrange to re-enable connectable advertising in case it was disabled due to the
        // maximum connection limit being reached.
        mFlags.Set(Flags::kRestartAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

void BLEManagerImpl::HandleWriteEvent(rsi_ble_event_write_t evt)
{
    // RSI_BLE_WRITE_REQUEST_EVENT
    ChipLogProgress(DeviceLayer, "Char Write Req, packet type %d", evt.pkt_type);
    uint8_t attribute = (uint8_t) event_msg.rsi_ble_measurement_hndl;

    WFX_RSI_LOG("attribute = %d,rsi_ble_measurement_hndl = %d", attribute, event_msg.rsi_ble_measurement_hndl);

    if (evt.handle[0] == (uint8_t) event_msg.rsi_ble_gatt_server_client_config_hndl) // TODO:: compare the handle exactly
    {
        WFX_RSI_LOG("Inside HandleTXCharCCCDWrite ");
        HandleTXCharCCCDWrite(&evt);
    }
    else
    {
        HandleRXCharWrite(&evt);
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(rsi_ble_event_write_t * evt)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipDeviceEvent event;
    // whether the client is enabling or disabling indications.
    {
        event.Type                    = DeviceEventType::kCHIPoBLESubscribe;
        event.CHIPoBLESubscribe.ConId = 1;
        err                           = PlatformMgr().PostEvent(&event);
    }
}

void BLEManagerImpl::HandleRXCharWrite(rsi_ble_event_write_t * evt)
{
    uint8_t conId  = 1;
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;
    uint16_t writeLen = evt->length;
    uint8_t * data    = (uint8_t *) evt->att_value;

    for (int i = 0; i < evt->length; i++)
    {
        ChipLogDetail(DeviceLayer, "HandleRXCharWrite  value : %d", evt->att_value[i]);
    }

    // Copy the data to a packet buffer.
    buf = System::PacketBufferHandle::NewWithData(data, writeLen, 0, 0);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    ChipLogDetail(DeviceLayer, "Write request/command received for CHIPoBLE RX characteristic ( len %d)", writeLen);

    // Post an event to the CHIP queue to deliver the data into the CHIP stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = conId;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        err                               = PlatformMgr().PostEvent(&event);
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
    //    uint8_t timerHandle = sInstance.GetTimerHandle(conId, false);
    //
    //    ChipLogProgress(DeviceLayer, "Tx Confirmation received");
    //
    //    // stop indication confirmation timer // TODO:: Need to find the proper repleacement
    //    if (timerHandle < kMaxConnections)
    //    {
    //        ChipLogProgress(DeviceLayer, " stop soft timer");
    //        // sl_bt_system_set_lazy_soft_timer(0, 0, timerHandle, false);
    //    }

    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = conId;
    PlatformMgr().PostEventOrDie(&event);
}

// TODO:: Need to Implement
void BLEManagerImpl::HandleSoftTimerEvent(void)
{

    // BLE Manager starts soft timers with timer handles less than kMaxConnections
    // If we receive a callback for unknown timer handle ignore this.
    //    if (evt->data.evt_system_soft_timer.handle < kMaxConnections)
    //    {
    //        ChipLogProgress(DeviceLayer, "BLEManagerImpl::HandleSoftTimerEvent CHIPOBLE_PROTOCOL_ABORT");
    //        ChipDeviceEvent event;
    //        event.Type                                                   = DeviceEventType::kCHIPoBLEConnectionError;
    //        event.CHIPoBLEConnectionError.ConId                          = mIndConfId[evt->data.evt_system_soft_timer.handle];
    //        sInstance.mIndConfId[evt->data.evt_system_soft_timer.handle] = kUnusedIndex;
    //        event.CHIPoBLEConnectionError.Reason                         = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
    //        PlatformMgr().PostEventOrDie(&event);
    //    }
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

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
CHIP_ERROR BLEManagerImpl::EncodeAdditionalDataTlv()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
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

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(additionalDataPayloadParams, c3AdditionalDataBufferHandle,
                                                                         additionalDataFields);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to generate TLV encoded Additional Data (%s)", __func__);
    }

    return err;
}

// TODO:: Need the
void BLEManagerImpl::HandleC3ReadRequest(void)
{
    //    sl_bt_evt_gatt_server_user_read_request_t * readReq =
    //        (sl_bt_evt_gatt_server_user_read_request_t *) &(evt->data.evt_gatt_server_user_read_request);
    //    ChipLogDetail(DeviceLayer, "Read request received for CHIPoBLEChar_C3 - opcode:%d", readReq->att_opcode);
    //    sl_status_t ret = sl_bt_gatt_server_send_user_read_response(readReq->connection, readReq->characteristic, 0,
    //                                                                sInstance.c3AdditionalDataBufferHandle->DataLength(),
    //                                                                sInstance.c3AdditionalDataBufferHandle->Start(), nullptr);

    //    if (ret != SL_STATUS_OK)
    //    {
    //        ChipLogDetail(DeviceLayer, "Failed to send read response, err:%ld", ret);
    //    }
}
#endif // CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING

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
