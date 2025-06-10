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
 *          for the Realtek platforms.
 */

/* this file behaves like a config.h, comes first */
#include <crypto/CHIPCryptoPAL.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <ble/Ble.h>

#include "stdio.h"

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#endif

#include "bt_types.h"
#include "gap_msg.h"
#include "matter_ble.h"
#include "matter_ble_service.h"

/*******************************************************************************
 * Local data types
 *******************************************************************************/
using namespace ::chip;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

/*******************************************************************************
 * Macros & Constants definitions
 *******************************************************************************/
#define APP_MAX_LINKS 1
#define MAX_ADV_DATA_LEN 31
#define MAX_RSP_DATA_LEN 31
#define CHIP_ADV_DATA_TYPE_FLAGS 0x01
#define CHIP_ADV_DATA_FLAGS 0x06
#define CHIP_ADV_DATA_TYPE_SERVICE_DATA 0x16

#define LOOP_EV_BLE (0x08)

/* ble app task configuration */
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_PRIORITY (HOST_TASK_PRIORITY - 1)
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE (1024)

/* advertising configuration */
#define CHIP_ADV_SHORT_UUID_LEN (2)

/** Type of UUID */
enum
{
    /** 16-bit UUID (BT SIG assigned) */
    BLE_UUID_TYPE_16 = 16,

    /** 32-bit UUID (BT SIG assigned) */
    BLE_UUID_TYPE_32 = 32,

    /** 128-bit UUID */
    BLE_UUID_TYPE_128 = 128,
};

typedef struct
{
    /** Type of the UUID */
    uint8_t type;
} ble_uuid_t;

/** 16-bit UUID */
typedef struct
{
    ble_uuid_t u;
    uint16_t value;
} ble_uuid16_t;

const ble_uuid16_t ShortUUID_CHIPoBLEService = { BLE_UUID_TYPE_16, 0xFFF6 };

static constexpr System::Clock::Timeout kFastAdvertiseTimeout =
    System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
System::Clock::Timestamp mAdvertiseStartTime;
} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    // Initialize the CHIP BleLayer.
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;

    // Check if BLE stack is initialized
    VerifyOrExit(!mFlags.Has(Flags::kAMEBABLEStackInitialized), err = CHIP_ERROR_INCORRECT_STATE);

    err = MapBLEError(matter_ble_init(APP_MAX_LINKS));
    matter_ble_cback_register((P_MATTER_BLE_CBACK) (ble_callback_dispatcher));

    SuccessOrExit(err);

    // Set related flags
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kAMEBABLEStackInitialized);
    mFlags.Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART ? true : false);
    mFlags.Set(Flags::kFastAdvertisingEnabled);

    InitSubscribed();

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

void BLEManagerImpl::HandleTXCharRead(void * param)
{
    /* Not supported */
    ChipLogError(DeviceLayer, "BLEManagerImpl::HandleTXCharRead() not supported");
}

void BLEManagerImpl::HandleTXCharCCCDRead(void * param)
{
    /* Not Supported */
    ChipLogError(DeviceLayer, "BLEManagerImpl::HandleTXCharCCCDRead() not supported");
}

void BLEManagerImpl::HandleTXCharCCCDWrite(int conn_id, int indicationsEnabled)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // If the client has requested to enabled indications/notifications
    if (indicationsEnabled)
    {
        // If indications are not already enabled for the connection...
        if (!IsSubscribed(conn_id))
        {
            // Record that indications have been enabled for this connection.
            err = SetSubscribed(conn_id);
            VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, err = CHIP_NO_ERROR);
            SuccessOrExit(err);
        }
    }
    else
    {
        // If indications had previously been enabled for this connection, record that they are no longer
        // enabled.
        UnsetSubscribed(conn_id);
    }

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling indications.
    {
        ChipDeviceEvent event;
        event.Type = indicationsEnabled ? DeviceEventType::kCHIPoBLESubscribe : DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = conn_id;
        PlatformMgr().PostEventOrDie(&event);
    }

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", indicationsEnabled ? "subscribe" : "unsubscribe");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
    }

    return;
}

CHIP_ERROR BLEManagerImpl::HandleTXComplete(int conn_id)
{
    // Post an event to the Chip queue to process the indicate confirmation.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = conn_id;
    PlatformMgr().PostEventOrDie(&event);
    return CHIP_NO_ERROR;
}

uint16_t BLEManagerImpl::_NumConnections()
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

CHIP_ERROR BLEManagerImpl::HandleGAPConnect(uint16_t conn_id)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Track the number of active GAP connections.
    mNumGAPCons++;
    VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, err = CHIP_NO_ERROR);
    SuccessOrExit(err);

    mFlags.Set(Flags::kRestartAdvertising);
    mFlags.Clear(Flags::kRestartAdvertising);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::HandleGAPDisconnect(uint16_t conn_id, uint16_t disc_cause)
{
    // Update the number of GAP connections.
    if (mNumGAPCons > 0)
    {
        mNumGAPCons--;
    }

    CHIP_ERROR disconReason;
    switch (disc_cause)
    {
    case HCI_ERR | HCI_ERR_REMOTE_USER_TERMINATE:
        disconReason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
        break;
    case HCI_ERR | HCI_ERR_LOCAL_HOST_TERMINATE:
        disconReason = BLE_ERROR_APP_CLOSED_CONNECTION;
        break;
    default:
        disconReason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
        break;
    }

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
    event.CHIPoBLEConnectionError.ConId  = conn_id;
    event.CHIPoBLEConnectionError.Reason = disconReason;
    PlatformMgr().PostEventOrDie(&event);

    // Force a reconfiguration of advertising in case we switched to non-connectable mode when
    // the BLE connection was established.
    mFlags.Set(Flags::kRestartAdvertising);
    mFlags.Clear(Flags::kAdvertisingConfigured);

    return CHIP_NO_ERROR;
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

void BLEManagerImpl::AddConnection(uint8_t connectionHandle)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(connectionHandle, true);

    if (bleConnState != NULL)
    {
        memset(bleConnState, 0, sizeof(CHIPoBLEConState));
        bleConnState->allocated        = 1;
        bleConnState->connectionHandle = connectionHandle;
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

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (mFlags.Has(Flags::kAdvertisingEnabled) != val)
    {
        mFlags.Set(Flags::kAdvertisingEnabled, val);
        mFlags.Set(Flags::kFastAdvertisingEnabled, val);
        mFlags.Set(Flags::kRestartAdvertising, 1);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

void BLEManagerImpl::HandleFastAdvertisementTimer(System::Layer * systemLayer, void * context)
{
    static_cast<BLEManagerImpl *>(context)->HandleFastAdvertisementTimer();
}

void BLEManagerImpl::HandleFastAdvertisementTimer()
{
    System::Clock::Timestamp currentTimestamp = System::SystemClock().GetMonotonicTimestamp();

    if (currentTimestamp - mAdvertiseStartTime >= kFastAdvertiseTimeout)
    {
        mFlags.Set(Flags::kFastAdvertisingEnabled, 0);
        mFlags.Set(Flags::kRestartAdvertising, 1);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (deviceName != NULL && deviceName[0] != 0)
    {
        if (strlen(deviceName) >= kMaxDeviceNameLength)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        strcpy(mDeviceName, deviceName);
        // Configure the BLE device name.
        matter_ble_device_name_set(mDeviceName, kMaxDeviceNameLength);

        mFlags.Set(Flags::kDeviceNameSet);
        ChipLogProgress(DeviceLayer, "Setting device name to : \"%s\"", deviceName);
    }
    else
    {
        mDeviceName[0] = 0;
        mFlags.Clear(Flags::kDeviceNameSet);
    }

exit:
    return err;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    // Platform specific events
    case DeviceEventType::kCHIPoBLESubscribe:
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        {
            ChipDeviceEvent connEstEvent;
            connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
            PlatformMgr().PostEventOrDie(&connEstEvent);
        }
        break;

    case DeviceEventType::kCHIPoBLEUnsubscribe: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEUnsubscribe");
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
    }
    break;

    case DeviceEventType::kCHIPoBLEWriteReceived: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEWriteReceived");
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID,
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
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
    }
    break;

    case DeviceEventType::kServiceProvisioningChange:
    case DeviceEventType::kWiFiConnectivityChange:
        ChipLogProgress(DeviceLayer, "Updating advertising data");
        mFlags.Clear(Flags::kAdvertisingConfigured);
        mFlags.Set(Flags::kRestartAdvertising);

        DriveBLEState();
        break;

    default:
        break;
    }
}

CHIP_ERROR BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                   const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic() not supported");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                     const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic() not supported");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    CHIP_ERROR err;
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    // Ble close function
    err = MapBLEError(matter_ble_disconnect(conId));

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Close connection failed: %s", ErrorStr(err));
    }

    mFlags.Set(Flags::kRestartAdvertising);
    mFlags.Clear(Flags::kAdvertisingConfigured);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return err;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return matter_ble_mtu_get(conId);
}

CHIP_ERROR BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                            PacketBufferHandle pBuf)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // Nothing to do
    CloseConnection(conId);
}

CHIP_ERROR BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                          PacketBufferHandle data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(IsSubscribed(conId), err = CHIP_ERROR_INVALID_ARGUMENT);

    matter_ble_indication_send(conId, data->Start(), data->DataLength());

exit:
    return err;
}

/*******************************************************************************
 * Private functions
 *******************************************************************************/

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData()
{
    CHIP_ERROR err;
    uint8_t advData[MAX_ADV_DATA_LEN]                                                      = { 0 };
    uint8_t rspData[MAX_RSP_DATA_LEN]                                                      = { 0 };
    char serialNumberString[DeviceLayer::ConfigurationManager::kMaxSerialNumberLength + 1] = { 0 };
    ChipBLEDeviceIdentificationInfo deviceIdInfo;
    uint8_t index = 0;
    uint16_t adv_int_min;
    uint16_t adv_int_max;

    // If the device name is not specified, generate a CHIP-standard name based on the bottom digits of the Chip device id.
    uint16_t discriminator;
    SuccessOrExit(err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));

    if (!mFlags.Has(Flags::kDeviceNameSet))
    {
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
        mDeviceName[kMaxDeviceNameLength] = 0;
    }

    /**************** Prepare advertising data *******************************************/
    memset(advData, 0, sizeof(advData));
    advData[index++] = 0x02;                                                                     // length
    advData[index++] = CHIP_ADV_DATA_TYPE_FLAGS;                                                 // AD type : flags
    advData[index++] = CHIP_ADV_DATA_FLAGS;                                                      // AD value
    advData[index++] = static_cast<uint8_t>(sizeof(deviceIdInfo) + CHIP_ADV_SHORT_UUID_LEN + 1); // length
    advData[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;                                     // AD type: (Service Data - 16-bit UUID)
    advData[index++] = static_cast<uint8_t>(ShortUUID_CHIPoBLEService.value & 0xFF);        // AD value
    advData[index++] = static_cast<uint8_t>((ShortUUID_CHIPoBLEService.value >> 8) & 0xFF); // AD value

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "GetBLEDeviceIdentificationInfo(): %s", ErrorStr(err));
        ExitNow();
    }

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    deviceIdInfo.SetAdditionalDataFlag(true);
#endif

    VerifyOrExit(index + sizeof(deviceIdInfo) <= sizeof(advData), err = CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG);
    memcpy(&advData[index], &deviceIdInfo, sizeof(deviceIdInfo));
    index = static_cast<uint8_t>(index + sizeof(deviceIdInfo));

    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        adv_int_min = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
        adv_int_max = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
    }
    else
    {
        adv_int_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        adv_int_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
    }

    matter_ble_adv_config(adv_int_min, adv_int_max, advData, index);

#if 1
    index = 0;

    /**************** Prepare scan response data *******************************************/
    rspData[index++] = 0x00; // length
    rspData[index++] = 0xFF; // GAP_ADTYPE_MANUFACTURER_SPECIFIC
    rspData[index++] = 0x5D;
    rspData[index++] = 0x00; // Company Id

    err = GetDeviceInstanceInfoProvider()->GetSerialNumber(serialNumberString, sizeof(serialNumberString));
    if (err == CHIP_NO_ERROR)
    {
        uint8_t sn_len = strlen(serialNumberString);

        VerifyOrExit(index + sn_len <= sizeof(rspData), err = CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG);
        memcpy(&rspData[index], serialNumberString, sn_len);
        index += sn_len;

        rspData[0] = 3 + sn_len;
        matter_ble_scan_rsp_data_set(rspData, index);
    }
    else
    {
        ChipLogError(DeviceLayer, "ConfigureAdvertisingData error: %s", ErrorStr(err));
    }
#endif

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = ConfigureAdvertisingData();
    SuccessOrExit(err);

    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        mAdvertiseStartTime = System::SystemClock().GetMonotonicTimestamp();
        ReturnErrorOnFailure(DeviceLayer::SystemLayer().StartTimer(kFastAdvertiseTimeout, HandleFastAdvertisementTimer, this));
    }

    // Start advertising
    matter_ble_adv_stop();
    vTaskDelay(100);
    matter_ble_adv_start();

    mFlags.Set(Flags::kAdvertising);
    mFlags.Clear(Flags::kRestartAdvertising);

    if (err == CHIP_NO_ERROR)
    {
        ChipDeviceEvent advChange;
        advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
        advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Started;
        PlatformMgr().PostEventOrDie(&advChange);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising()
{
    CHIP_ERROR err;

    // Stop advertising
    matter_ble_adv_stop();

    // Change flag status to the 'not Advertising state'
    if (mFlags.Has(Flags::kAdvertising))
    {
        mFlags.Clear(Flags::kAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled);

        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");

        // Post a CHIPoBLEAdvertisingChange(Stopped) event.
        {
            ChipDeviceEvent advChange;
            advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
            advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
            PlatformMgr().PostEventOrDie(&advChange);
        }
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::MapBLEError(int bleErr)
{
    switch (bleErr)
    {
    case 0:
        return CHIP_NO_ERROR;
    default:
        return CHIP_ERROR_INCORRECT_STATE;
    }
}

void BLEManagerImpl::DriveBLEState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if BLE stack is initialized
    VerifyOrExit(mFlags.Has(Flags::kAMEBABLEStackInitialized), /* */);

    // Start advertising if needed...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && mFlags.Has(Flags::kAdvertisingEnabled))
    {
        // Start/re-start advertising if not already started, or if there is a pending change
        // to the advertising configuration.
        if (!mFlags.Has(Flags::kAdvertising) || mFlags.Has(Flags::kRestartAdvertising))
        {
            err = StartAdvertising();
            SuccessOrExit(err);
            ChipLogProgress(DeviceLayer, "Started BLE Advertising");
        }
    }
    // Otherwise, stop advertising if it is enabled.
    else if (mFlags.Has(Flags::kAdvertising))
    {
        err = StopAdvertising();
        SuccessOrExit(err);
        ChipLogProgress(DeviceLayer, "Stopped BLE Advertising");
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

void BLEManagerImpl::InitSubscribed()
{
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        mSubscribedConIds[i] = BLE_CONNECTION_UNINITIALIZED;
    }
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
        else if (i < freeIndex && mSubscribedConIds[i] == BLE_CONNECTION_UNINITIALIZED)
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

void BLEManagerImpl::HandleRXCharWrite(uint8_t * p_value, uint16_t len, uint8_t conn_id)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    PacketBufferHandle buf = System::PacketBufferHandle::New(len, 0);
    memcpy(buf->Start(), p_value, len);
    buf->SetDataLength(len);

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    ChipDeviceEvent event;
    event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
    event.CHIPoBLEWriteReceived.ConId = (uint16_t) conn_id;
    event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
    PlatformMgr().PostEventOrDie(&event);
}

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
void BLEManagerImpl::HandleC3CharRead(TBTCONFIG_CALLBACK_DATA * p_data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    PacketBufferHandle bufferHandle;
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
    p_data->msg_data.write.p_value = bufferHandle->Start();
    p_data->msg_data.write.len     = bufferHandle->DataLength();

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to generate TLV encoded Additional Data (%s)", __func__);
    }
    return;
}
#endif /* CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING */

CHIP_ERROR BLEManagerImpl::HandleGapMsg(T_IO_MSG * p_gap_msg)
{
    T_LE_GAP_MSG gap_msg;
    memcpy(&gap_msg, &p_gap_msg->u.param, sizeof(p_gap_msg->u.param));
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(DeviceLayer, "HandleGapMsg subtype: %d", p_gap_msg->subtype);
    switch (p_gap_msg->subtype)
    {
    case GAP_MSG_LE_CONN_STATE_CHANGE:
        /* A new connection was established or a connection attempt failed */
        {
            uint16_t conn_id    = gap_msg.msg_data.gap_conn_state_change.conn_id;
            uint16_t new_state  = gap_msg.msg_data.gap_conn_state_change.new_state;
            uint16_t disc_cause = gap_msg.msg_data.gap_conn_state_change.disc_cause;
            if (new_state == GAP_CONN_STATE_CONNECTED)
            {
                ChipLogProgress(DeviceLayer, "GAP_CONN_STATE_CONNECTED");
                err = sInstance.HandleGAPConnect(conn_id);
                SuccessOrExit(err);
            }
            else if (new_state == GAP_CONN_STATE_DISCONNECTED)
            {
                ChipLogProgress(DeviceLayer, "GAP_CONN_STATE_DISCONNECTED");
                err = sInstance.HandleGAPDisconnect(conn_id, disc_cause);
                SuccessOrExit(err);
            }
        }
        break;

    case GAP_MSG_LE_DEV_STATE_CHANGE: {
        T_GAP_DEV_STATE new_state = gap_msg.msg_data.gap_dev_state_change.new_state;
        ChipLogProgress(DeviceLayer, "HandleGapMsg: init state %d, adv state %d, cause 0x%x", new_state.gap_init_state,
                        new_state.gap_adv_state, gap_msg.msg_data.gap_dev_state_change.cause);

        if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
        {
            ChipLogProgress(DeviceLayer, "GAP stack ready");
        }

        if (new_state.gap_adv_state == GAP_ADV_STATE_IDLE)
        {
            ChipLogProgress(DeviceLayer, "GAP adv stoped");
        }
        else if (new_state.gap_adv_state == GAP_ADV_STATE_ADVERTISING)
        {
            ChipLogProgress(DeviceLayer, "GAP adv start");
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

    // Schedule DriveBLEState() to run.
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return err;
}

CHIP_ERROR BLEManagerImpl::gatt_svr_chr_access(T_SERVER_ID service_id, TBTCONFIG_CALLBACK_DATA * p_data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (service_id == SERVICE_PROFILE_GENERAL_ID)
    {
        T_SERVER_APP_CB_DATA * p_param = (T_SERVER_APP_CB_DATA *) p_data;
        switch (p_param->eventId)
        {
        case PROFILE_EVT_SEND_DATA_COMPLETE:
            err = sInstance.HandleTXComplete(p_param->event_data.send_data_result.conn_id);
            break;

        default:
            break;
        }
    }
    else
    {
        switch (p_data->msg_type)
        {
        case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
            sInstance.HandleC3CharRead(p_data);
#endif
            break;

        case SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE:
            sInstance.HandleRXCharWrite(p_data->msg_data.write.p_value, p_data->msg_data.write.len, p_data->conn_id);
            break;

        case SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION: {
            T_MATTER_BLE_CBACK_DATA * pp_data = (T_MATTER_BLE_CBACK_DATA *) p_data;

            switch (pp_data->msg_data.indication_index)
            {
            case MATTER_BLE_SERVICE_INDICATE_V3_ENABLE: {
                sInstance.HandleTXCharCCCDWrite(p_data->conn_id, 1);
            }
            break;

            case MATTER_BLE_SERVICE_INDICATE_V3_DISABLE: {
                sInstance.HandleTXCharCCCDWrite(p_data->conn_id, 0);
            }
            break;
            }
        }
        break;

        default:
            break;
        }
    }

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return err;
}

int BLEManagerImpl::ble_callback_dispatcher(void * p_cb_data, int type, T_CHIP_BLEMGR_CALLBACK_TYPE callback_type)
{
    switch (callback_type)
    {
    case CB_PROFILE_CALLBACK:
        sInstance.gatt_svr_chr_access(type, (TBTCONFIG_CALLBACK_DATA *) p_cb_data);
        break;

    case CB_GAP_MSG_CALLBACK:
        sInstance.HandleGapMsg((T_IO_MSG *) p_cb_data);
        break;

    default:
        break;
    }

    return 0;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
