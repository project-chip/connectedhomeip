/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          for Linux platforms.
 */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <ble/CHIPBleServiceData.h>
#include <new>
#include <platform/internal/BLEManager.h>
#include <support/CodeUtils.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "CHIPBluezHelper.h"

using namespace ::nl;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {
const uint8_t UUID_CHIPoBLEService[]       = { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
                                         0x00, 0x10, 0x00, 0x00, 0xAF, 0xFE, 0x00, 0x00 };
const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

void HandleIncomingBleConnection(BLEEndPoint * bleEP)
{
    ChipLogProgress(DeviceLayer, "CHIPoBluez con rcvd");
}

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags       = kFlag_AdvertisingEnabled;
    mAppState    = NULL;

    memset(mDeviceName, 0, sizeof(mDeviceName));

    OnChipBleConnectReceived = HandleIncomingBleConnection;

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

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

    if (GetFlag(mFlags, kFlag_AdvertisingEnabled) != val)
    {
        SetFlag(mFlags, kFlag_AdvertisingEnabled, val);
    }

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

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
        SetFlag(mFlags, kFlag_UseCustomDeviceName);
    }
    else
    {
        mDeviceName[0] = 0;
        ClearFlag(mFlags, kFlag_UseCustomDeviceName);
    }

    return CHIP_NO_ERROR;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    uint16_t numCons = 0;
    return numCons;
}

CHIP_ERROR BLEManagerImpl::ConfigureBle(uint32_t aNodeId, bool aIsCentral)
{
    CHIP_ERROR err                  = CHIP_NO_ERROR;
    mBLEAdvConfig.mpBleName         = mDeviceName;
    mBLEAdvConfig.mNodeId           = aNodeId;
    mBLEAdvConfig.mMajor            = 1;
    mBLEAdvConfig.mMinor            = 1;
    mBLEAdvConfig.mVendorId         = 1;
    mBLEAdvConfig.mProductId        = 1;
    mBLEAdvConfig.mDeviceId         = 1;
    mBLEAdvConfig.mDuration         = 2;
    mBLEAdvConfig.mPairingStatus    = 0;
    mBLEAdvConfig.mType             = ChipAdvType::BLUEZ_ADV_TYPE_UNDIRECTED_CONNECTABLE_SCANNABLE;
    mBLEAdvConfig.mpAdvertisingUUID = "0xFEAF";

    mpBleAddr  = NULL;
    mIsCentral = aIsCentral;

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StartBLEAdvertising(void)
{
    return StartBluezAdv(mpAppState);
}

CHIP_ERROR BLEManagerImpl::StopBLEAdvertising(void)
{
    return StopBluezAdv(mpAppState);
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
                            event->CHIPoBLEWriteReceived.Data);
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

        // If CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled, and there is a change to the
        // device's provisioning state, then automatically disable CHIPoBLE advertising if the device
        // is now fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED

        // Force the advertising configuration to be refreshed to reflect new provisioning state.
        ClearFlag(mFlags, kFlag_AdvertisingConfigured);

        DriveBLEState();

    default:
        break;
    }

    HandlePlatformSpecificBLEEvent(event);
}

void BLEManagerImpl::HandlePlatformSpecificBLEEvent(const ChipDeviceEvent * apEvent)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    bool controlOpComplete = false;
    ChipLogProgress(DeviceLayer, "HandlePlatformSpecificBLEEvent , %d", apEvent->Type);
    switch (apEvent->Type)
    {
    case DeviceEventType::kPlatformLinuxBLEPeripheralAdvConfiguredComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralAdvConfiguredComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);
        SetFlag(sInstance.mFlags, kFlag_AdvertisingConfigured);
        ClearFlag(sInstance.mFlags, kFlag_ControlOpInProgress);
        controlOpComplete = true;
        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising config complete");
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralAdvStartComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralAdvStartComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);
        ClearFlag(sInstance.mFlags, kFlag_ControlOpInProgress);
        ClearFlag(sInstance.mFlags, kFlag_AdvertisingRefreshNeeded);

        if (!GetFlag(sInstance.mFlags, kFlag_Advertising))
        {
            SetFlag(sInstance.mFlags, kFlag_Advertising);
        }

        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralAdvStopComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralAdvStopComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);

        ClearFlag(sInstance.mFlags, kFlag_ControlOpInProgress);
        ClearFlag(sInstance.mFlags, kFlag_AdvertisingRefreshNeeded);

        // Transition to the not Advertising state...
        if (GetFlag(sInstance.mFlags, kFlag_Advertising))
        {
            ClearFlag(sInstance.mFlags, kFlag_Advertising);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");
        }
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralRegisterAppComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralRegisterAppComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);
        SetFlag(mFlags, kFlag_AppRegistered);
        controlOpComplete = true;
        break;
    default:
        break;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
        ClearFlag(sInstance.mFlags, kFlag_ControlOpInProgress);
    }

    if (controlOpComplete)
    {
        ClearFlag(mFlags, kFlag_ControlOpInProgress);
        DriveBLEState();
    }
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    BluezConnection * con = static_cast<BluezConnection *>(conId);
    return (con != NULL) ? con->mMtu : 0;
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic() not supported");
    return true;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic() not supported");
    return true;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    BluezConnection * con = static_cast<BluezConnection *>(conId);
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);
    return CloseBluezConnection(con);
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                    chip::System::PacketBuffer * pBuf)
{
    return SendBluezIndication((void *) conId, pBuf);
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                      chip::System::PacketBuffer * pBuf)
{
    ChipLogError(Ble, "SendWriteRequest: Not implemented");
    return true;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                     chip::System::PacketBuffer * pBuf)
{
    ChipLogError(Ble, "SendReadRequest: Not implemented");
    return true;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId)
{
    ChipLogError(Ble, "SendReadRBluezonse: Not implemented");
    return true;
}

void BLEManagerImpl::CHIPoBluez_NewConnection(void * data)
{
    ChipLogProgress(Ble, "CHIPoBluez_NewConnection: %p", data);
}

void BLEManagerImpl::HandleRXCharWrite(void * data, const uint8_t * value, size_t len)
{
    CHIP_ERROR err     = CHIP_NO_ERROR;
    PacketBuffer * buf = NULL;

    // Copy the data to a PacketBuffer.
    buf = PacketBuffer::New();
    VerifyOrExit(buf != NULL, err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(buf->AvailableDataLength() >= len, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buf->Start(), value, len);
    buf->SetDataLength(len);

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type = DeviceEventType::kCHIPoBLEWriteReceived;
        ChipLogProgress(Ble, "Write request received debug %p", data);
        event.CHIPoBLEWriteReceived.ConId = data;
        event.CHIPoBLEWriteReceived.Data  = buf;
        PlatformMgr().PostEvent(&event);
        buf = NULL;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
    }

    if (NULL != buf)
    {
        chip::System::PacketBuffer::Free(buf);
    }
}

void BLEManagerImpl::CHIPoBluez_ConnectionClosed(void * data)
{
    ChipLogProgress(DeviceLayer, "Bluez notify CHIPoBluez connection disconnected");

    // If this was a CHIPoBLE connection, post an event to deliver a connection error to the CHIPoBLE layer.
    {
        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId  = data;
        event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
        PlatformMgr().PostEvent(&event);
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(void * data)
{
    CHIP_ERROR err               = CHIP_NO_ERROR;
    BluezConnection * connection = static_cast<BluezConnection *>(data);
    const char * msg             = NULL;

    VerifyOrExit(connection != NULL, msg = "Connection is NULL in HandleTXCharCCCDWrite");
    VerifyOrExit(connection->mpC2 != NULL, msg = "C2 is NULL in HandleTXCharCCCDWrite");

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling indications.
    {
        ChipDeviceEvent event;
        event.Type = (connection->mIsNotify) ? DeviceEventType::kCHIPoBLESubscribe : DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = data;
        PlatformMgr().PostEvent(&event);
    }

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", (connection->mIsNotify) ? "subscribe" : "unsubscribe");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
        // TODO: fail connection
    }
}

void BLEManagerImpl::HandleTXComplete(void * data)
{
    // Post an event to the Chip queue to process the indicate confirmation.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = data;
    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::DriveBLEState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the Chip task is running.
    if (!GetFlag(mFlags, kFlag_AsyncInitCompleted))
    {
        SetFlag(mFlags, kFlag_AsyncInitCompleted);

        // If CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled,
        // disable CHIPoBLE advertising if the device is fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        ExitNow();
    }

    // If there's already a control operation in progress, wait until it completes.
    VerifyOrExit(!GetFlag(mFlags, kFlag_ControlOpInProgress), /* */);

    // Initializes the Bluez BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !GetFlag(mFlags, kFlag_BluezBLELayerInitialized))
    {
        err = InitBluezBleLayer(false, NULL, mBLEAdvConfig, mpAppState);
        SuccessOrExit(err);
        SetFlag(mFlags, kFlag_BluezBLELayerInitialized);
    }

    // Register the CHIPoBLE application with the Bluez BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !GetFlag(mFlags, kFlag_AppRegistered))
    {
        err = BluezGattsAppRegister(mpAppState);
        SetFlag(mFlags, kFlag_ControlOpInProgress);
        ExitNow();
    }

    // If the application has enabled CHIPoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && GetFlag(mFlags, kFlag_AdvertisingEnabled))
    {
        // Start/re-start advertising if not already advertising, or if the advertising state of the
        // Bluez BLE layer needs to be refreshed.
        if (!GetFlag(mFlags, kFlag_Advertising) || GetFlag(mFlags, kFlag_AdvertisingRefreshNeeded))
        {
            // Configure advertising data if it hasn't been done yet.  This is an asynchronous step which
            // must complete before advertising can be started.  When that happens, this method will
            // be called again, and execution will proceed to the code below.
            if (!GetFlag(mFlags, kFlag_AdvertisingConfigured))
            {
                err = BluezAdvertisementSetup(mpAppState);
                ExitNow();
            }

            // Start advertising.  This is also an asynchronous step.
            err = StartBLEAdvertising();
            SuccessOrExit(err);

            SetFlag(sInstance.mFlags, kFlag_Advertising);
            ExitNow();
        }
    }

    // Otherwise stop advertising if needed...
    else
    {
        if (GetFlag(mFlags, kFlag_Advertising))
        {
            err = StopBLEAdvertising();
            SuccessOrExit(err);
            SetFlag(mFlags, kFlag_ControlOpInProgress);

            ExitNow();
        }
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

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    ChipLogRetain(Ble, "Got notification regarding chip connection closure");
}

void BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                                 = DeviceEventType::kPlatformLinuxBLEPeripheralRegisterAppComplete;
    event.Platform.BLEPeripheralRegisterAppComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralRegisterAppComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvConfiguredComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type = DeviceEventType::kPlatformLinuxBLEPeripheralAdvConfiguredComplete;
    event.Platform.BLEPeripheralAdvConfiguredComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvConfiguredComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                              = DeviceEventType::kPlatformLinuxBLEPeripheralAdvStartComplete;
    event.Platform.BLEPeripheralAdvStartComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvStartComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                             = DeviceEventType::kPlatformLinuxBLEPeripheralAdvStopComplete;
    event.Platform.BLEPeripheralAdvStopComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvStopComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEvent(&event);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
