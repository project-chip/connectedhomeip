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
 *          for the Qorvo QPG platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/BleUUID.h>
#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

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

#define CHIP_ADV_SHORT_UUID_LEN 2

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
    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);

    appCbacks.stackCback    = ExternalCbHandler;
    appCbacks.chrReadCback  = HandleTXCharRead;
    appCbacks.chrWriteCback = HandleRXCharWrite;
    appCbacks.cccCback      = _handleTXCharCCCDWrite;
    qvCHIP_BleSetUUIDs((uint8_t *) chipUUID_CHIPoBLE_Service, (uint8_t *) chipUUID_CHIPoBLEChar_TX.bytes,
                       (uint8_t *) chipUUID_CHIPoBLEChar_RX.bytes);
    qvCHIP_BleInit(&appCbacks);

    // Create FreeRTOS sw timer for BLE timeouts and interval change.
    sbleAdvTimeoutTimer = xTimerCreate("BleAdvTimer",       // Just a text name, not used by the RTOS kernel
                                       1,                   // == default timer period (mS)
                                       false,               // no timer reload (==one-shot)
                                       (void *) this,       // init timer id = ble obj context
                                       BleAdvTimeoutHandler // timer callback handler
    );
    VerifyOrExit(sbleAdvTimeoutTimer != NULL, err = CHIP_ERROR_INCORRECT_STATE);

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

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
    CHIP_ERROR err;

    err = MapBLEError(qvCHIP_BleGetDeviceName(buf, bufSize));

    return err;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * devName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (devName != nullptr && devName[0] != 0)
    {
        err = MapBLEError(qvCHIP_BleSetDeviceName(devName));
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

        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLESubscribe");
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
        connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
        PlatformMgr().PostEvent(&connEstEvent);
    }
    break;

    case DeviceEventType::kCHIPoBLEUnsubscribe: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEUnsubscribe");
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kCHIPoBLEWriteReceived: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEWriteReceived");
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
    }
    break;

    case DeviceEventType::kCHIPoBLEConnectionError: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEConnectionError");
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
    }
    break;

    // Generic CHIP events
    case DeviceEventType::kFabricMembershipChange:
    case DeviceEventType::kServiceProvisioningChange:
    case DeviceEventType::kAccountPairingChange:

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

        // Force the advertising state to be refreshed to reflect new provisioning state.
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);

        DriveBLEState();

        break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEIndicateConfirm");
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
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
    CHIP_ERROR err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;

    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    err = MapBLEError(qvCHIP_BleCloseConnection(conId));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "qvCHIP_BleCloseConnection() failed: %s", ErrorStr(err));
    }

    return (err == CHIP_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    uint16_t retVal = 0;
    CHIP_ERROR err  = MapBLEError(qvCHIP_BleGetMTU(conId, &retVal));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "qvCHIP_BleGetMTU() failed: %s", ErrorStr(err));
    }

    return retVal;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool isRxHandle;
    uint16_t cId;
    uint16_t dataLen = data->DataLength();

    VerifyOrExit(IsSubscribed(conId), err = CHIP_ERROR_INVALID_ARGUMENT);
    ChipLogDetail(DeviceLayer, "Sending notification for CHIPoBLE Client TX (con %u, len %u)", conId, dataLen);

    isRxHandle = UUIDsMatch(&chipUUID_CHIPoBLEChar_RX, charId);
    cId        = qvCHIP_BleGetHandle(isRxHandle);

    qvCHIP_BleSendNotification(conId, cId, dataLen, data->Start());

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManagerImpl::SendNotification() failed: %s", ErrorStr(err));
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

CHIP_ERROR BLEManagerImpl::MapBLEError(int bleErr) const
{
    CHIP_ERROR err;

    switch (bleErr)
    {
    case QV_STATUS_NO_ERROR: {
        err = CHIP_NO_ERROR;
        break;
    }
    case QV_STATUS_BUFFER_TOO_SMALL: {
        err = CHIP_ERROR_BUFFER_TOO_SMALL;
        break;
    }
    case QV_STATUS_INVALID_ARGUMENT: {
        err = CHIP_ERROR_INVALID_ARGUMENT;
        break;
    }
    default: {
        err = CHIP_ERROR_INCORRECT_STATE;
    }
    }
    return err;
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the CHIP task is running.
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
    uint8_t index               = 0;
    uint8_t mDeviceNameLength   = 0;
    uint8_t mDeviceIdInfoLength = 0;

    memset(mAdvDataBuf, 0, kMaxAdvertisementDataSetSize);
    memset(mScanRespDataBuf, 0, kMaxAdvertisementDataSetSize);

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(mDeviceIdInfo);
    SuccessOrExit(err);

    if (!mFlags.Has(Flags::kDeviceNameSet))
    {
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04" PRIX32, CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, (uint32_t) 0);

        mDeviceName[kMaxDeviceNameLength] = 0;
        err                               = MapBLEError(qvCHIP_BleSetDeviceName(mDeviceName));
        SuccessOrExit(err);
    }

    mDeviceNameLength   = static_cast<uint8_t>(strlen(mDeviceName));
    mDeviceIdInfoLength = sizeof(mDeviceIdInfo);

    // Check sizes
    static_assert(sizeof(mDeviceIdInfo) + CHIP_ADV_SHORT_UUID_LEN + 1 <= UINT8_MAX, "Our length won't fit in a uint8_t");
    static_assert(2 + CHIP_ADV_SHORT_UUID_LEN + sizeof(mDeviceIdInfo) + 1 <= kMaxAdvertisementDataSetSize,
                  "Advertisement data buffer is not big enough");

    // Fill in advertising data
    index                = 0;
    mAdvDataBuf[index++] = 0x02;                     // length
    mAdvDataBuf[index++] = CHIP_ADV_DATA_TYPE_FLAGS; // AD type : flags
    mAdvDataBuf[index++] = CHIP_ADV_DATA_FLAGS;      // AD value

    mAdvDataBuf[index++] = static_cast<uint8_t>(mDeviceIdInfoLength + CHIP_ADV_SHORT_UUID_LEN + 1); // AD length
    mAdvDataBuf[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;                                         // AD type : Service Data
    mAdvDataBuf[index++] = chipUUID_CHIPoBLE_Service[1];                                            // AD value
    mAdvDataBuf[index++] = chipUUID_CHIPoBLE_Service[0];
    memcpy(&mAdvDataBuf[index], (void *) &mDeviceIdInfo, mDeviceIdInfoLength); // AD value
    index = static_cast<uint8_t>(index + mDeviceIdInfoLength);

    mAdvDataBuf[index++] = static_cast<uint8_t>(mDeviceNameLength + 1); // length
    mAdvDataBuf[index++] = CHIP_ADV_DATA_TYPE_NAME;                     // AD type : name
    memcpy(&mAdvDataBuf[index], mDeviceName, mDeviceNameLength);        // AD value
    index = static_cast<uint8_t>(index + mDeviceNameLength);

    qvCHIP_BleSetAdvData(QV_ADV_DATA_LOC_ADV, index, mAdvDataBuf);

    // Fill in scan response data
    index                     = 0;
    mScanRespDataBuf[index++] = CHIP_ADV_SHORT_UUID_LEN + 1;  // AD length
    mScanRespDataBuf[index++] = CHIP_ADV_DATA_TYPE_UUID;      // AD type : uuid
    mScanRespDataBuf[index++] = chipUUID_CHIPoBLE_Service[1]; // AD value
    mScanRespDataBuf[index++] = chipUUID_CHIPoBLE_Service[0];

    qvCHIP_BleSetAdvData(QV_ADV_DATA_LOC_SCAN, index, mScanRespDataBuf);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err;
    uint32_t bleAdvTimeout;
    uint16_t intervalMin;
    uint16_t intervalMax;

    // If already advertising, stop it, before changing values
    if (mFlags.Has(Flags::kAdvertising))
    {
        err = MapBLEError(qvCHIP_BleStopAdvertising());
        SuccessOrExit(err);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "CHIPoBLE start advertising");
    }

    err = ConfigureAdvertisingData();
    SuccessOrExit(err);

    mFlags.Clear(Flags::kRestartAdvertising);

    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        intervalMin   = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
        intervalMax   = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
        bleAdvTimeout = CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME;
    }
    else
    {
        intervalMin   = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        intervalMax   = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
        bleAdvTimeout = CHIP_DEVICE_CONFIG_BLE_ADVERTISING_TIMEOUT - CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME;
    }

    qvCHIP_BleSetAdvInterval(intervalMin, intervalMax);

    err = MapBLEError(qvCHIP_BleStartAdvertising());
    SuccessOrExit(err);

    mFlags.Set(Flags::kAdvertising);
    StartBleAdvTimeoutTimer(bleAdvTimeout);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    CHIP_ERROR err;

    err = MapBLEError(qvCHIP_BleStopAdvertising());
    SuccessOrExit(err);

    CancelBleAdvTimeoutTimer();
    // Transition to the not Advertising state...
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
            PlatformMgr().PostEvent(&advChange);
        }
    }

exit:
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

void BLEManagerImpl::HandleTXCharRead(uint16_t connId, uint16_t handle, uint8_t operation, uint16_t offset,
                                      qvCHIP_Ble_Attr_t * pAttr)
{
    uint8_t rsp = 0;

    ChipLogProgress(DeviceLayer, "Read request received for CHIPoBLE Client TX characteristic (con %u)", connId);

    // Send a zero-length response.
    qvCHIP_BleWriteAttr(connId, handle, 0, &rsp);
}

void BLEManagerImpl::HandleRXCharWrite(uint16_t connId, uint16_t handle, uint8_t operation, uint16_t offset, uint16_t len,
                                       uint8_t * pValue, qvCHIP_Ble_Attr_t * pAttr)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(DeviceLayer, "Write request received for CHIPoBLE Client RX characteristic (con %u, len %u)", connId, len);

    // Copy the data to a packet buffer.
    PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(pValue, len, 0, 0);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = connId;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
        // TODO: fail connection???
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(qvCHIP_Ble_AttsCccEvt_t * pEvt)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    bool notificationsEnabled;

    ChipLogProgress(DeviceLayer, "Write request received for CHIPoBLE TX characteristic CCCD (con %u, len %u)", pEvt->hdr.param, 0);

    // Determine if the client is enabling or disabling notifications
    notificationsEnabled = (pEvt->value != 0);

    // If the client has requested to enable notifications
    if (notificationsEnabled)
    {
        // Set subcription only the first time
        if (!IsSubscribed(pEvt->hdr.param))
        {
            // Record that notifications have been enabled for this connection.
            err = SetSubscribed(pEvt->hdr.param);
            VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, err = CHIP_NO_ERROR);
            SuccessOrExit(err);
        }
    }

    else
    {
        // If notifications had previously been enabled for this connection, record that they are no longer enabled
        UnsetSubscribed(pEvt->hdr.param);
    }

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling notifications
    {
        ChipDeviceEvent event;
        event.Type = (notificationsEnabled) ? DeviceEventType::kCHIPoBLESubscribe : DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = pEvt->hdr.param;
        PlatformMgr().PostEvent(&event);
    }

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", notificationsEnabled ? "subscribe" : "unsubscribe");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
        // TODO: fail connection???
    }
}

/* Process DM Messages */
void BLEManagerImpl::HandleDmMsg(qvCHIP_Ble_DmEvt_t * pDmEvt)
{
    switch (pDmEvt->hdr.event)
    {
    case QVCHIP_DM_RESET_CMPL_IND: {
        break;
    }
    case QVCHIP_DM_SCAN_START_IND: {
        break;
    }
    case QVCHIP_DM_SCAN_STOP_IND: {
        break;
    }
    case QVCHIP_DM_ADV_START_IND: {
        if (pDmEvt->hdr.status != QVCHIP_HCI_SUCCESS)
        {
            ChipLogError(DeviceLayer, "QVCHIP_DM_ADV_START_IND error: %d", (int) pDmEvt->advSetStart.hdr.status);
            return;
        }

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
    }
    case QVCHIP_DM_ADV_STOP_IND: {
        if (pDmEvt->hdr.status != QVCHIP_HCI_SUCCESS)
        {
            ChipLogError(DeviceLayer, "QVCHIP_DM_ADV_STOP_IND error: %d", (int) pDmEvt->advSetStop.status);
            return;
        }
        if (mFlags.Has(Flags::kRestartAdvertising))
        {
            BLEMgr().SetAdvertisingMode(BLEAdvertisingMode::kSlowAdvertising);
        }
        break;
    }
    case QVCHIP_DM_CONN_OPEN_IND: {
        ChipLogProgress(DeviceLayer, "BLE GATT connection established (con %u)", pDmEvt->connOpen.hdr.param);

        // Allocate a connection state record for the new connection.
        mNumGAPCons++;

        // Receiving a connection stops the advertising processes.  So force a refresh of the advertising
        // state.
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    }
    case QVCHIP_DM_CONN_CLOSE_IND: {
        ChipLogProgress(DeviceLayer, "BLE GATT connection closed (con %u, reason %u)", pDmEvt->connClose.hdr.param,
                        pDmEvt->connClose.reason);

        // Force a refresh of the advertising state.
        if (mNumGAPCons > 0)
        {
            mNumGAPCons--;
        }

        // If this was a CHIPoBLE connection, release the associated connection state record
        // and post an event to deliver a connection error to the CHIPoBLE layer.
        if (UnsetSubscribed(pDmEvt->connClose.hdr.param))
        {
            ChipDeviceEvent event;
            event.Type                          = DeviceEventType::kCHIPoBLEConnectionError;
            event.CHIPoBLEConnectionError.ConId = pDmEvt->connClose.hdr.param;
            switch (pDmEvt->connClose.reason)
            {
            case QVCHIP_HCI_ERR_REMOTE_TERMINATED:
                event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
                break;
            case QVCHIP_HCI_ERR_LOCAL_TERMINATED:
                event.CHIPoBLEConnectionError.Reason = BLE_ERROR_APP_CLOSED_CONNECTION;
                break;
            default:
                event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
                break;
            }
            PlatformMgr().PostEvent(&event);
        }

        mFlags.Set(Flags::kAdvertisingRefreshNeeded);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    }
    case QVCHIP_DM_CONN_UPDATE_IND: {
        break;
    }
    default: {
        break;
    }
    }
}

/* Process ATT Messages */
void BLEManagerImpl::HandleAttMsg(qvCHIP_Ble_AttEvt_t * pAttEvt)
{
    switch (pAttEvt->hdr.event)
    {
    case QVCHIP_ATT_MTU_UPDATE_IND: {
        break;
    }
    case QVCHIP_ATTS_HANDLE_VALUE_CNF: {
        ChipDeviceEvent event;

        event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
        event.CHIPoBLEIndicateConfirm.ConId = pAttEvt->hdr.param;
        PlatformMgr().PostEvent(&event);
        break;
    }
    case QVCHIP_ATTC_FIND_BY_TYPE_VALUE_RSP:
    case QVCHIP_ATTC_READ_BY_GROUP_TYPE_RSP:
    case QVCHIP_ATTC_READ_BY_TYPE_RSP:
    case QVCHIP_ATTC_FIND_INFO_RSP:
        break;
    case QVCHIP_ATTC_READ_RSP:
        break;
    case QVCHIP_ATTC_WRITE_RSP:
    case QVCHIP_ATTC_WRITE_CMD_RSP:
    case QVCHIP_ATTC_PREPARE_WRITE_RSP:
        break;
    case QVCHIP_ATTC_EXECUTE_WRITE_RSP:
        break;
    case QVCHIP_ATTC_HANDLE_VALUE_NTF:
    case QVCHIP_ATTC_HANDLE_VALUE_IND:
        break;
    default: {
        break;
    }
    }
}

void BLEManagerImpl::ExternalCbHandler(qvCHIP_Ble_MsgHdr_t * pMsg)
{
    if (pMsg != nullptr)
    {
        PlatformMgr().LockChipStack();

        /* Process advertising/scanning and connection-related messages */
        if (pMsg->event >= QVCHIP_DM_CBACK_START && pMsg->event <= QVCHIP_DM_CBACK_END)
        {
            ChipLogProgress(DeviceLayer, "DM event %d: status %d", pMsg->event, pMsg->status);
            sInstance.HandleDmMsg((qvCHIP_Ble_DmEvt_t *) pMsg);
        }
        /* Process attribute-related messages */
        else if (pMsg->event >= QVCHIP_ATT_CBACK_START && pMsg->event <= QVCHIP_ATT_CBACK_END)
        {
            ChipLogProgress(DeviceLayer, "ATT event %d: status %d", pMsg->event, pMsg->status);
            sInstance.HandleAttMsg((qvCHIP_Ble_AttEvt_t *) pMsg);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "Unknown event %d", pMsg->event);
        }

        PlatformMgr().UnlockChipStack();
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

void BLEManagerImpl::_handleTXCharCCCDWrite(qvCHIP_Ble_AttsCccEvt_t * event)
{
    sInstance.HandleTXCharCCCDWrite(event);
}

void BLEManagerImpl::BleAdvTimeoutHandler(TimerHandle_t xTimer)
{
    if (BLEMgrImpl().mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        /* Stop advertising and defer restart for when stop confirmation is received from the stack */
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Start slow advertissment");
        sInstance.StopAdvertising();
        sInstance.mFlags.Set(Flags::kRestartAdvertising);
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
    if (xTimerChangePeriod(sbleAdvTimeoutTimer, pdMS_TO_TICKS(aTimeoutInMs), 100) != pdPASS)
    {
        ChipLogError(DeviceLayer, "Failed to start BledAdv timeout timer");
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
