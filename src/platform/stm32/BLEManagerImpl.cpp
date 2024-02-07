/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          for the stm32wb55 platforms.
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <ble/BleUUID.h>
#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

/**** STM32 api files for ble and led support ****/
#include "app_ble.h"
#include "app_matter.h"
#include "ble_gap_aci.h"
#include "cmsis_os.h"

using namespace ::chip;
using namespace ::chip::Ble;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

// Advertising data content definitions
#define CHIP_ADV_DATA_TYPE_FLAGS 0x01
#define CHIP_ADV_DATA_TYPE_UUID 0x03
#define CHIP_ADV_DATA_FLAGS 0x06
#define CHIP_ADV_DATA_TYPE_NAME 0x09
#define CHIP_ADV_DATA_TYPE_SERVICE_DATA 0x16
#define STM32_MTU 247
#define CHIP_ADV_SHORT_UUID_LEN 2
#define CONNECTION_CLOSE 0x13

// FreeeRTOS sw timer
TimerHandle_t sbleAdvTimeoutTimer;

// Full service UUID - CHIP_BLE_SVC_ID - taken from BleUUID.h header
const uint8_t chipUUID_CHIPoBLE_Service[CHIP_ADV_SHORT_UUID_LEN] = { 0xFF, 0xF6 };

const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };

const ChipBleUUID chipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;
uint16_t gconnid = 0;

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled);
    mNumGAPCons = 0;

    for (int i = 0; i < kMaxConnections; i++)
    {
        mSubscribedConIds[i] = BLE_CONNECTION_UNINITIALIZED;
    }

    // Initialize the CHIP BleLayer.
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    // Create FreeRTOS sw timer for BLE timeouts and interval change.
    sbleAdvTimeoutTimer = xTimerCreate("BleAdvTimer",       // Just a text name, not used by the RTOS kernel
                                       pdMS_TO_TICKS(1),    // == default timer period (mS)
                                       false,               // no timer reload (==one-shot)
                                       (void *) this,       // init timer id = ble obj context
                                       BleAdvTimeoutHandler // timer callback handler
    );
    VerifyOrExit(sbleAdvTimeoutTimer != NULL, err = CHIP_ERROR_INCORRECT_STATE);

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    APP_MATTER_BLE_Set_Receive_Callback(HandleRXCharWrite);
    APP_MATTER_BLE_Set_Connection_Callback(HandleGAPConnect);
    APP_MATTER_BLE_Set_Disconnection_Callback(HandleGAPDisconnect);
    APP_MATTER_BLE_Set_TXCharCCCDWrite_Callback(HandleTXCharCCCDWrite);
    APP_MATTER_BLE_Set_Ack_After_Indicate_Callback(HandleAck);

exit:
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::Init() complete");

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
    ChipLogDetail(DeviceLayer, "CHIPoBLE advertising set to %s", val ? "on" : "off");
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
    CHIP_ERROR err = CHIP_NO_ERROR;

    return err;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * devName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (devName != nullptr && devName[0] != 0)
    {
        // TODO set device name
        SuccessOrExit(err);

        mFlags.Set(Flags::kDeviceNameSet);
    }

exit:
    return err;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    // Platform specific events
    case DeviceEventType::kCHIPoBLESubscribe: {
        ChipDeviceEvent connEstEvent;
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
        connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
        PlatformMgr().PostEventOrDie(&connEstEvent);
    }
    break;

    case DeviceEventType::kCHIPoBLEUnsubscribe: {
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kCHIPoBLEWriteReceived: {
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
    }
    break;

    case DeviceEventType::kCHIPoBLEConnectionError: {
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
    }
    break;

        // Generic CHIP events
    case DeviceEventType::kServiceProvisioningChange:
        // Force the advertising state to be refreshed to reflect new provisioning state.
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);

        PlatformMgr().ScheduleWork(DriveBLEState, 0);

        break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm: {
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
    }
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
    CHIP_ERROR err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;

    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection con %u", *(uint16_t *) conId);
    // aci_gap_terminate(gconnid, CONNECTION_CLOSE);
    return (err == CHIP_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    uint16_t retVal = 0;
    // TODO get MTU from stm32 api
    retVal = STM32_MTU;
    return retVal;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    uint16_t dataLen = data->DataLength();

    ChipLogDetail(DeviceLayer, "Sending notification for CHIPoBLE Client TX (con %u, len %u)", *(uint16_t *) conId, dataLen);

    APP_MATTER_Send_Notification(dataLen, data->Start());

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
    CloseConnection(conId);
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the CHIP task is running.
    if (!mFlags.Has(Flags::kAsyncInitCompleted))
    {
        mFlags.Set(Flags::kAsyncInitCompleted);
        ChipLogProgress(DeviceLayer, "kAsyncInitCompleted done");
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

    // If the application has enabled CHIPoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled &&
        mFlags.Has(Flags::kAdvertisingEnabled)
#if CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION
        // and no connections are active...
        && (mNumGAPCons == 0)
#endif
    )
    {
        // Start/re-start BLE advertising if not already advertising, or if the
        // advertising state of the underlying stack needs to be refreshed.
        if (!mFlags.Has(Flags::kAdvertising) || mFlags.Has(Flags::kAdvertisingRefreshNeeded))
        {
            mFlags.Clear(Flags::kAdvertisingRefreshNeeded);
            err = StartAdvertising();
            SuccessOrExit(err);
        }
    }

    // Otherwise, stop advertising if currently active.
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
    ChipBLEDeviceIdentificationInfo mDeviceIdInfo;
    CHIP_ERROR err;
    uint8_t index              = 0;
    uint8_t deviceNameLength   = 0;
    uint8_t deviceIdInfoLength = 0;
    int ret                    = 0;

    char deviceName[kMaxDeviceNameLength + 1];
    uint8_t advDataBuf[kMaxAdvertisementDataSetSize];
    uint8_t scanRespDataBuf[kMaxAdvertisementDataSetSize];

    memset(advDataBuf, 0, kMaxAdvertisementDataSetSize);
    memset(scanRespDataBuf, 0, kMaxAdvertisementDataSetSize);

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(mDeviceIdInfo);
    SuccessOrExit(err);

    if (!mFlags.Has(Flags::kDeviceNameSet))
    {
        snprintf(deviceName, sizeof(deviceName), "%s%04" PRIX32, CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, (uint32_t) 0);
        deviceName[kMaxDeviceNameLength] = 0;
        SuccessOrExit(err);
    }

    deviceNameLength   = static_cast<uint8_t>(strlen(deviceName));
    deviceIdInfoLength = sizeof(mDeviceIdInfo);

    // Check sizes
    static_assert(sizeof(mDeviceIdInfo) + CHIP_ADV_SHORT_UUID_LEN + 1 <= UINT8_MAX, "Our length won't fit in a uint8_t");
    static_assert(2 + CHIP_ADV_SHORT_UUID_LEN + sizeof(mDeviceIdInfo) + 1 <= kMaxAdvertisementDataSetSize,
                  "Advertisement data buffer is not big enough");

    // Fill in advertising data
    index               = 0;
    advDataBuf[index++] = 0x02;                     // length
    advDataBuf[index++] = CHIP_ADV_DATA_TYPE_FLAGS; // AD type : flags
    advDataBuf[index++] = CHIP_ADV_DATA_FLAGS;      // AD value

    advDataBuf[index++] = static_cast<uint8_t>(deviceIdInfoLength + CHIP_ADV_SHORT_UUID_LEN + 1); // AD length
    advDataBuf[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;                                        // AD type : Service Data
    advDataBuf[index++] = chipUUID_CHIPoBLE_Service[1];                                           // AD value
    advDataBuf[index++] = chipUUID_CHIPoBLE_Service[0];
    memcpy(&advDataBuf[index], (void *) &mDeviceIdInfo, deviceIdInfoLength); // AD value

    index = static_cast<uint8_t>(index + deviceIdInfoLength);

    ChipLogError(DeviceLayer, "state return update adv: %d", ret);
    // Fill in scan response data
    index                    = 0;
    scanRespDataBuf[index++] = CHIP_ADV_SHORT_UUID_LEN + 1;  // AD length
    scanRespDataBuf[index++] = CHIP_ADV_DATA_TYPE_UUID;      // AD type : uuid
    scanRespDataBuf[index++] = chipUUID_CHIPoBLE_Service[1]; // AD value
    scanRespDataBuf[index++] = chipUUID_CHIPoBLE_Service[0];

    VerifyOrExit(index + (deviceNameLength + 2) <= kMaxAdvertisementDataSetSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    scanRespDataBuf[index++] = static_cast<uint8_t>(deviceNameLength + 1); // length
    scanRespDataBuf[index++] = CHIP_ADV_DATA_TYPE_NAME;                    // AD type : name
    memcpy(&scanRespDataBuf[index], deviceName, deviceNameLength);         // AD value
    index = static_cast<uint8_t>(index + deviceNameLength);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err;

    // If already advertising, stop it, before changing values
    if (mFlags.Has(Flags::kAdvertising))
    {
        APP_BLE_Adv_Cancel(); // ST ble api
    }
    else
    {
        ChipLogProgress(DeviceLayer, "CHIPoBLE start advertising");
    }

    err = ConfigureAdvertisingData();
    SuccessOrExit(err);

    mFlags.Clear(Flags::kRestartAdvertising);

    APP_BLE_Adv_Request(APP_BLE_FAST_ADV);
    // Flag updated asynchronously by BLE host callback
    mFlags.Set(Flags::kAdvertising);

    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        StartBleAdvTimeoutTimer(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    CHIP_ERROR err;

    APP_BLE_Adv_Cancel(); // ST ble api
    CancelBleAdvTimeoutTimer();
    // Transition to the not Advertising state...
    if (mFlags.Has(Flags::kAdvertising))
    {
        mFlags.Clear(Flags::kAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled);

        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");

        // Post a CHIPoBLEAdvertisingChange(Stopped) event.
        ChipDeviceEvent advChange;
        advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
        advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
        err                                        = PlatformMgr().PostEvent(&advChange);
    }

    return err;
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

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

void BLEManagerImpl::bleConnect(void)
{
    CHIP_ERROR err;
    ChipLogProgress(DeviceLayer, "BLE GATT connection established ");

    mNumGAPCons++;
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    ChipDeviceEvent connectEvent;
    connectEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
    err               = PlatformMgr().PostEvent(&connectEvent);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerImpl::bleDisconnect(uint16_t connid)
{
    CHIP_ERROR err;

    ChipLogDetail(DeviceLayer, "BLE GATT connection closed (con %u)", connid);
    ;
    // Force a refresh of the advertising state.
    if (mNumGAPCons > 0)
    {
        mNumGAPCons--;
    }

    // If this was a CHIPoBLE connection, release the associated connection state record
    // and post an event to deliver a connection error to the CHIPoBLE layer.
    if (!BLEMgrImpl().IsSubscribed(connid))
    {
        if (UnsetSubscribed(connid))
        {
            ChipDeviceEvent event;
            event.Type                          = DeviceEventType::kCHIPoBLEConnectionError;
            event.CHIPoBLEConnectionError.ConId = connid;
            PlatformMgr().PostEventOrDie(&event);
            ChipLogDetail(DeviceLayer, "BLE unuset subscribed (con %u)", connid);
        }
    }

    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    ChipDeviceEvent disconnectEvent;
    disconnectEvent.Type = DeviceEventType::kCHIPoBLEConnectionClosed;
    err                  = PlatformMgr().PostEvent(&disconnectEvent);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerImpl::HandleGAPConnect(void)
{

    ChipLogProgress(DeviceLayer, "Gap connect");
    sInstance.bleConnect();
}
void BLEManagerImpl::HandleGAPDisconnect(uint16_t * connid)
{
    ChipLogProgress(DeviceLayer, "Gap disconnect");
    sInstance.bleDisconnect(gconnid);
}

void BLEManagerImpl::HandleRXCharWrite(BLE_Matter_RX * aMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(DeviceLayer, "Write request received for CHIPoBLE Client RX characteristic (con %u, len %u)", aMessage->connid,
                    aMessage->Length);
    // update connid
    gconnid = aMessage->connid;

    PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(aMessage->Payload, aMessage->Length, 0, 0);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = aMessage->connid;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        err                               = PlatformMgr().PostEvent(&event);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
        // TODO: fail connection???
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(BLE_Matter_TXCharCCCD * aMessage)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool notificationsEnabled;

    ChipLogProgress(DeviceLayer, "Write request received for CHIPoBLE TX characteristic CCCD (con %u, len %u)", aMessage->connid,
                    0);

    // Determine if the client is enabling or disabling notifications
    notificationsEnabled = (aMessage->notif != 0);
    // If the client has requested to enable notifications
    if (notificationsEnabled)
    {
        // Set subcription only the first time
        if (!BLEMgrImpl().IsSubscribed(aMessage->connid)) // conid
        {
            // Record that notifications have been enabled for this connection.
            err = BLEMgrImpl().SetSubscribed(aMessage->connid);
            VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, err = CHIP_NO_ERROR);
            SuccessOrExit(err);
        }
    }

    else
    {
        // If notifications had previously been enabled for this connection, record that they are no longer enabled
        BLEMgrImpl().UnsetSubscribed(aMessage->connid);
    }

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling notifications
    {
        ChipDeviceEvent event;
        event.Type = (notificationsEnabled) ? DeviceEventType::kCHIPoBLESubscribe : DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = aMessage->connid;
        err                           = PlatformMgr().PostEvent(&event);
    }

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", notificationsEnabled ? "subscribe" : "unsubscribe");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
        // TODO: fail connection???
    }
}

void BLEManagerImpl::HandleAck(uint16_t * connid)
{
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = gconnid;
    PlatformMgr().PostEventOrDie(&event);
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

void BLEManagerImpl::BleAdvTimeoutHandler(TimerHandle_t xTimer)
{
    if (BLEMgrImpl().mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        /* Stop advertising and defer restart for when stop confirmation is received from the stack */
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Stop advertisement");
        sInstance.StopAdvertising();
        sInstance.mFlags.Set(Flags::kRestartAdvertising);
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
    if (xTimerChangePeriod(sbleAdvTimeoutTimer, pdMS_TO_TICKS(aTimeoutInMs), 100) != pdPASS)
    {
        ChipLogError(DeviceLayer, "Failed to start BledAdv timeout timer");
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
