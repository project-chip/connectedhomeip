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

#include <type_traits>
#include <utility>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "CHIPBluezHelper.h"

using namespace ::nl;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {
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

    err = BleLayer::Init(this, this, this, &SystemLayer);
    SuccessOrExit(err);

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags       = (CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART && !mIsCentral) ? kFlag_AdvertisingEnabled : 0;
    mAppState    = nullptr;

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
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (deviceName != nullptr && deviceName[0] != 0)
    {
        VerifyOrExit(strlen(deviceName) < kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);
        strcpy(mDeviceName, deviceName);
        SetFlag(mFlags, kFlag_UseCustomDeviceName);
    }
    else
    {
        uint16_t discriminator;
        SuccessOrExit(err = ConfigurationMgr().GetSetupDiscriminator(discriminator));
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
        mDeviceName[kMaxDeviceNameLength] = 0;
        ClearFlag(mFlags, kFlag_UseCustomDeviceName);
    }

exit:
    return err;
}

uint16_t BLEManagerImpl::_NumConnections()
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

    mIsCentral = aIsCentral;

    return err;
}

CHIP_ERROR BLEManagerImpl::StartBLEAdvertising()
{
    return StartBluezAdv(static_cast<BluezEndpoint *>(mpAppState));
}

CHIP_ERROR BLEManagerImpl::StopBLEAdvertising()
{
    return StopBluezAdv(static_cast<BluezEndpoint *>(mpAppState));
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
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
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
        break;
    default:
        HandlePlatformSpecificBLEEvent(event);
        break;
    }
}

void BLEManagerImpl::HandlePlatformSpecificBLEEvent(const ChipDeviceEvent * apEvent)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    bool controlOpComplete = false;
    ChipLogProgress(DeviceLayer, "HandlePlatformSpecificBLEEvent %d", apEvent->Type);
    switch (apEvent->Type)
    {
    case DeviceEventType::kPlatformLinuxBLECentralConnected:
        if (OnConnectionComplete != nullptr)
            OnConnectionComplete(mBLEScanConfig.mAppState, apEvent->Platform.BLECentralConnected.mConnection);
        break;
    case DeviceEventType::kPlatformLinuxBLEWriteComplete:
        HandleWriteConfirmation(apEvent->Platform.BLEWriteComplete.mConnection, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_RX);
        break;
    case DeviceEventType::kPlatformLinuxBLESubscribeOpComplete:
        if (apEvent->Platform.BLESubscribeOpComplete.mIsSubscribed)
            HandleSubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &CHIP_BLE_SVC_ID,
                                    &ChipUUID_CHIPoBLEChar_TX);
        else
            HandleUnsubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &CHIP_BLE_SVC_ID,
                                      &ChipUUID_CHIPoBLEChar_TX);
        break;
    case DeviceEventType::kPlatformLinuxBLEIndicationReceived:
        HandleIndicationReceived(apEvent->Platform.BLEIndicationReceived.mConnection, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX,
                                 PacketBufferHandle::Adopt(apEvent->Platform.BLEIndicationReceived.mData));
        break;
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
    BluezConnection * connection = static_cast<BluezConnection *>(conId);
    return (connection != nullptr) ? connection->mMtu : 0;
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    bool result = false;

    VerifyOrExit(Ble::UUIDsMatch(svcId, &CHIP_BLE_SVC_ID),
                 ChipLogError(DeviceLayer, "SubscribeCharacteristic() called with invalid service ID"));
    VerifyOrExit(Ble::UUIDsMatch(charId, &ChipUUID_CHIPoBLEChar_TX),
                 ChipLogError(DeviceLayer, "SubscribeCharacteristic() called with invalid characteristic ID"));

    result = BluezSubscribeCharacteristic(conId);
exit:
    return result;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    bool result = false;

    VerifyOrExit(Ble::UUIDsMatch(svcId, &CHIP_BLE_SVC_ID),
                 ChipLogError(DeviceLayer, "UnsubscribeCharacteristic() called with invalid service ID"));
    VerifyOrExit(Ble::UUIDsMatch(charId, &ChipUUID_CHIPoBLEChar_TX),
                 ChipLogError(DeviceLayer, "UnsubscribeCharacteristic() called with invalid characteristic ID"));

    result = BluezUnsubscribeCharacteristic(conId);
exit:
    return result;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %p)", conId);
    return CloseBluezConnection(conId);
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                    chip::System::PacketBufferHandle pBuf)
{
    return SendBluezIndication(conId, std::move(pBuf));
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                      chip::System::PacketBufferHandle pBuf)
{
    bool result = false;

    VerifyOrExit(Ble::UUIDsMatch(svcId, &CHIP_BLE_SVC_ID),
                 ChipLogError(DeviceLayer, "SendWriteRequest() called with invalid service ID"));
    VerifyOrExit(Ble::UUIDsMatch(charId, &ChipUUID_CHIPoBLEChar_RX),
                 ChipLogError(DeviceLayer, "SendWriteRequest() called with invalid characteristic ID"));

    result = BluezSendWriteRequest(conId, std::move(pBuf));
exit:
    return result;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                     chip::System::PacketBufferHandle pBuf)
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

void BLEManagerImpl::HandleNewConnection(BLE_CONNECTION_OBJECT conId)
{
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event;
        event.Type                                     = DeviceEventType::kPlatformLinuxBLECentralConnected;
        event.Platform.BLECentralConnected.mConnection = conId;
        PlatformMgr().PostEvent(&event);
    }
}

void BLEManagerImpl::HandleWriteComplete(BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event;
    event.Type                                  = DeviceEventType::kPlatformLinuxBLEWriteComplete;
    event.Platform.BLEWriteComplete.mConnection = conId;
    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::HandleSubscribeOpComplete(BLE_CONNECTION_OBJECT conId, bool subscribed)
{
    ChipDeviceEvent event;
    event.Type                                          = DeviceEventType::kPlatformLinuxBLESubscribeOpComplete;
    event.Platform.BLESubscribeOpComplete.mConnection   = conId;
    event.Platform.BLESubscribeOpComplete.mIsSubscribed = subscribed;
    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::HandleTXCharChanged(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    using DataLength = decltype(std::declval<System::PacketBuffer>().AvailableDataLength());

    CHIP_ERROR err                 = CHIP_NO_ERROR;
    System::PacketBufferHandle buf = System::PacketBuffer::New();

    ChipLogProgress(DeviceLayer, "Indication received, conn = %p", conId);

    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(buf->AvailableDataLength() >= len, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buf->Start(), value, len);
    buf->SetDataLength(static_cast<DataLength>(len));

    ChipDeviceEvent event;
    event.Type                                       = DeviceEventType::kPlatformLinuxBLEIndicationReceived;
    event.Platform.BLEIndicationReceived.mConnection = conId;
    event.Platform.BLEIndicationReceived.mData       = std::move(buf).UnsafeRelease();
    PlatformMgr().PostEvent(&event);

exit:
    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "HandleTXCharChanged() failed: %s", ErrorStr(err));
}

void BLEManagerImpl::HandleRXCharWrite(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;

    // Copy the data to a packet buffer.
    buf = System::PacketBuffer::New();
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(buf->AvailableDataLength() >= len, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buf->Start(), value, len);
    static_assert(std::is_same<decltype(buf->AvailableDataLength()), uint16_t>::value,
                  "Unexpected available data length type; fix the casting below");
    // Cast is safe, since we just verified that "len" fits in uint16_t.
    buf->SetDataLength(static_cast<uint16_t>(len));

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type = DeviceEventType::kCHIPoBLEWriteReceived;
        ChipLogProgress(Ble, "Write request received debug %p", conId);
        event.CHIPoBLEWriteReceived.ConId = conId;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::CHIPoBluez_ConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "Bluez notify CHIPoBluez connection disconnected");

    // If this was a CHIPoBLE connection, post an event to deliver a connection error to the CHIPoBLE layer.
    {
        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId  = conId;
        event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
        PlatformMgr().PostEvent(&event);
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(BLE_CONNECTION_OBJECT conId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    BluezConnection * connection = static_cast<BluezConnection *>(conId);

    VerifyOrExit(connection != nullptr, ChipLogError(DeviceLayer, "Connection is NULL in HandleTXCharCCCDWrite"));
    VerifyOrExit(connection->mpC2 != nullptr, ChipLogError(DeviceLayer, "C2 is NULL in HandleTXCharCCCDWrite"));

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling indications.
    {
        ChipDeviceEvent event;
        event.Type = connection->mIsNotify ? DeviceEventType::kCHIPoBLESubscribe : DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = connection;
        PlatformMgr().PostEvent(&event);
    }

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", connection->mIsNotify ? "subscribe" : "unsubscribe");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
        // TODO: fail connection
    }
}

void BLEManagerImpl::HandleTXComplete(BLE_CONNECTION_OBJECT conId)
{
    // Post an event to the Chip queue to process the indicate confirmation.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = conId;
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
        err = InitBluezBleLayer(mIsCentral, nullptr, mBLEAdvConfig, mpAppState);
        SuccessOrExit(err);
        SetFlag(mFlags, kFlag_BluezBLELayerInitialized);
    }

    // Register the CHIPoBLE application with the Bluez BLE layer if needed.
    if (!mIsCentral && mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !GetFlag(mFlags, kFlag_AppRegistered))
    {
        err = BluezGattsAppRegister(static_cast<BluezEndpoint *>(mpAppState));
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
                err = BluezAdvertisementSetup(static_cast<BluezEndpoint *>(mpAppState));
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

    // Configure BLE scanning
    if (mBLEScanConfig.mDiscriminator && !GetFlag(mFlags, kFlag_Scanning))
    {
        err = StartDiscovery(static_cast<BluezEndpoint *>(mpAppState), { mBLEScanConfig.mDiscriminator });
        SuccessOrExit(err);
        SetFlag(mFlags, kFlag_Scanning);
    }
    else if (!mBLEScanConfig.mDiscriminator && GetFlag(mFlags, kFlag_Scanning))
    {
        err = StopDiscovery(static_cast<BluezEndpoint *>(mpAppState));
        SuccessOrExit(err);
        ClearFlag(mFlags, kFlag_Scanning);
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

void BLEManagerImpl::NewConnection(BleLayer * bleLayer, void * appState, const uint16_t connDiscriminator)
{
    mBLEScanConfig.mDiscriminator = connDiscriminator;
    mBLEScanConfig.mAppState      = appState;
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
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
