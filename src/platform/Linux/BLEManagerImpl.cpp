/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <new>
#include <platform/CommissionableDataProvider.h>
#include <platform/internal/BLEManager.h>

#include <cassert>
#include <type_traits>
#include <utility>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "bluez/Helper.h"

using namespace ::nl;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

static constexpr System::Clock::Timeout kNewConnectionScanTimeout = System::Clock::Seconds16(10);
static constexpr System::Clock::Timeout kConnectTimeout           = System::Clock::Seconds16(10);

const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

void HandleConnectTimeout(chip::System::Layer *, void * apEndpoint)
{
    assert(apEndpoint != nullptr);

    CancelConnect(static_cast<BluezEndpoint *>(apEndpoint));
    BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_TIMEOUT);
}

} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

void HandleIncomingBleConnection(BLEEndPoint * bleEP)
{
    ChipLogProgress(DeviceLayer, "CHIPoBluez con rcvd");
}

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    err = BleLayer::Init(this, this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART && !mIsCentral);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);

    memset(mDeviceName, 0, sizeof(mDeviceName));

    OnChipBleConnectReceived = HandleIncomingBleConnection;

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

void BLEManagerImpl::_Shutdown()
{
    // ensure scan resources are cleared (e.g. timeout timers)
    mDeviceScanner.reset();
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mFlags.Has(Flags::kAdvertisingEnabled) != val)
    {
        mFlags.Set(Flags::kAdvertisingEnabled, val);
    }

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

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
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (deviceName != nullptr && deviceName[0] != 0)
    {
        VerifyOrExit(strlen(deviceName) < kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);
        strcpy(mDeviceName, deviceName);
        mFlags.Set(Flags::kUseCustomDeviceName);
    }
    else
    {
        uint16_t discriminator;
        SuccessOrExit(err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
        mDeviceName[kMaxDeviceNameLength] = 0;
        mFlags.Clear(Flags::kUseCustomDeviceName);
    }

exit:
    return err;
}

uint16_t BLEManagerImpl::_NumConnections()
{
    uint16_t numCons = 0;
    return numCons;
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
    mBLEAdvConfig.mType             = ChipAdvType::BLUEZ_ADV_TYPE_UNDIRECTED_CONNECTABLE_SCANNABLE;
    mBLEAdvConfig.mpAdvertisingUUID = "0xFFF6";

    mIsCentral = aIsCentral;

    return err;
}

CHIP_ERROR BLEManagerImpl::StartBLEAdvertising()
{
    return StartBluezAdv(mpEndpoint);
}

CHIP_ERROR BLEManagerImpl::StopBLEAdvertising()
{
    return StopBluezAdv(mpEndpoint);
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
            PlatformMgr().PostEventOrDie(&connectionEvent);
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
    case DeviceEventType::kServiceProvisioningChange:
        // Force the advertising configuration to be refreshed to reflect new provisioning state.
        mFlags.Clear(Flags::kAdvertisingConfigured);

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
    ChipLogDetail(DeviceLayer, "HandlePlatformSpecificBLEEvent %d", apEvent->Type);
    switch (apEvent->Type)
    {
    case DeviceEventType::kPlatformLinuxBLECentralConnected:
        if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        {
            BleConnectionDelegate::OnConnectionComplete(mBLEScanConfig.mAppState,
                                                        apEvent->Platform.BLECentralConnected.mConnection);
            CleanScanConfig();
        }
        break;
    case DeviceEventType::kPlatformLinuxBLECentralConnectFailed:
        if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        {
            BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, apEvent->Platform.BLECentralConnectFailed.mError);
            CleanScanConfig();
        }
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
        sInstance.mFlags.Set(Flags::kAdvertisingConfigured).Clear(Flags::kControlOpInProgress);
        controlOpComplete = true;
        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising config complete");
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralAdvStartComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralAdvStartComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);
        sInstance.mFlags.Clear(Flags::kControlOpInProgress).Clear(Flags::kAdvertisingRefreshNeeded);

        if (!sInstance.mFlags.Has(Flags::kAdvertising))
        {
            sInstance.mFlags.Set(Flags::kAdvertising);
        }

        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralAdvStopComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralAdvStopComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);

        sInstance.mFlags.Clear(Flags::kControlOpInProgress).Clear(Flags::kAdvertisingRefreshNeeded);

        // Transition to the not Advertising state...
        if (sInstance.mFlags.Has(Flags::kAdvertising))
        {
            sInstance.mFlags.Clear(Flags::kAdvertising);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");
        }
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralRegisterAppComplete:
        VerifyOrExit(apEvent->Platform.BLEPeripheralRegisterAppComplete.mIsSuccess, err = CHIP_ERROR_INCORRECT_STATE);
        mFlags.Set(Flags::kAppRegistered);
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
        sInstance.mFlags.Clear(Flags::kControlOpInProgress);
    }

    if (controlOpComplete)
    {
        mFlags.Clear(Flags::kControlOpInProgress);
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
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::HandleConnectFailed(CHIP_ERROR error)
{
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event;
        event.Type                                    = DeviceEventType::kPlatformLinuxBLECentralConnectFailed;
        event.Platform.BLECentralConnectFailed.mError = error;
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::HandleWriteComplete(BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event;
    event.Type                                  = DeviceEventType::kPlatformLinuxBLEWriteComplete;
    event.Platform.BLEWriteComplete.mConnection = conId;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleSubscribeOpComplete(BLE_CONNECTION_OBJECT conId, bool subscribed)
{
    ChipDeviceEvent event;
    event.Type                                          = DeviceEventType::kPlatformLinuxBLESubscribeOpComplete;
    event.Platform.BLESubscribeOpComplete.mConnection   = conId;
    event.Platform.BLESubscribeOpComplete.mIsSubscribed = subscribed;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleTXCharChanged(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    System::PacketBufferHandle buf = System::PacketBufferHandle::NewWithData(value, len);

    ChipLogDetail(DeviceLayer, "Indication received, conn = %p", conId);

    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    ChipDeviceEvent event;
    event.Type                                       = DeviceEventType::kPlatformLinuxBLEIndicationReceived;
    event.Platform.BLEIndicationReceived.mConnection = conId;
    event.Platform.BLEIndicationReceived.mData       = std::move(buf).UnsafeRelease();
    PlatformMgr().PostEventOrDie(&event);

exit:
    if (err != CHIP_NO_ERROR)
        ChipLogError(DeviceLayer, "HandleTXCharChanged() failed: %s", ErrorStr(err));
}

void BLEManagerImpl::HandleRXCharWrite(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;

    // Copy the data to a packet buffer.
    buf = System::PacketBufferHandle::NewWithData(value, len);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type = DeviceEventType::kCHIPoBLEWriteReceived;
        ChipLogProgress(Ble, "Write request received debug %p", conId);
        event.CHIPoBLEWriteReceived.ConId = conId;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        PlatformMgr().PostEventOrDie(&event);
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
        PlatformMgr().PostEventOrDie(&event);
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
        PlatformMgr().PostEventOrDie(&event);
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
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::DriveBLEState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the Chip task is running.
    if (!mFlags.Has(Flags::kAsyncInitCompleted))
    {
        mFlags.Set(Flags::kAsyncInitCompleted);
        ExitNow();
    }

    // If there's already a control operation in progress, wait until it completes.
    VerifyOrExit(!mFlags.Has(Flags::kControlOpInProgress), /* */);

    // Initializes the Bluez BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !mFlags.Has(Flags::kBluezBLELayerInitialized))
    {
        err = InitBluezBleLayer(mIsCentral, nullptr, mBLEAdvConfig, mpEndpoint);
        SuccessOrExit(err);
        mFlags.Set(Flags::kBluezBLELayerInitialized);
    }

    // Register the CHIPoBLE application with the Bluez BLE layer if needed.
    if (!mIsCentral && mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !mFlags.Has(Flags::kAppRegistered))
    {
        err = BluezGattsAppRegister(mpEndpoint);
        mFlags.Set(Flags::kControlOpInProgress);
        ExitNow();
    }

    // If the application has enabled CHIPoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && mFlags.Has(Flags::kAdvertisingEnabled))
    {
        // Start/re-start advertising if not already advertising, or if the advertising state of the
        // Bluez BLE layer needs to be refreshed.
        if (!mFlags.Has(Flags::kAdvertising) || mFlags.Has(Flags::kAdvertisingRefreshNeeded))
        {
            mFlags.Clear(Flags::kAdvertisingRefreshNeeded);

            // Configure advertising data if it hasn't been done yet.  This is an asynchronous step which
            // must complete before advertising can be started.  When that happens, this method will
            // be called again, and execution will proceed to the code below.
            if (!mFlags.Has(Flags::kAdvertisingConfigured))
            {
                err = BluezAdvertisementSetup(mpEndpoint);
                ExitNow();
            }

            // Start advertising.  This is also an asynchronous step.
            err = StartBLEAdvertising();
            SuccessOrExit(err);

            sInstance.mFlags.Set(Flags::kAdvertising);
            ExitNow();
        }
    }

    // Otherwise stop advertising if needed...
    else
    {
        if (mFlags.Has(Flags::kAdvertising))
        {
            err = StopBLEAdvertising();
            SuccessOrExit(err);
            mFlags.Set(Flags::kControlOpInProgress);

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
    ChipLogProgress(Ble, "Got notification regarding chip connection closure");
}

void BLEManagerImpl::InitiateScan(BleScanState scanType)
{
    DriveBLEState();

    if (scanType == BleScanState::kNotScanning)
    {
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_INCORRECT_STATE);
        ChipLogError(Ble, "Invalid scan type requested");
        return;
    }

    if (mpEndpoint == nullptr)
    {
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_INCORRECT_STATE);
        ChipLogError(Ble, "BLE Layer is not yet initialized");
        return;
    }

    if (mpEndpoint->mpAdapter == nullptr)
    {
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_INCORRECT_STATE);
        ChipLogError(Ble, "No adapter available for new connection establishment");
        return;
    }

    mDeviceScanner               = Internal::ChipDeviceScanner::Create(mpEndpoint->mpAdapter, this);
    mBLEScanConfig.mBleScanState = scanType;

    if (!mDeviceScanner)
    {
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_INTERNAL);
        ChipLogError(Ble, "Failed to create a BLE device scanner");
        return;
    }

    CHIP_ERROR err = mDeviceScanner->StartScan(kNewConnectionScanTimeout);
    if (err != CHIP_NO_ERROR)
    {
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        ChipLogError(Ble, "Failed to start a BLE can: %s", chip::ErrorStr(err));
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, err);
        return;
    }
}

void BLEManagerImpl::CleanScanConfig()
{
    if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        DeviceLayer::SystemLayer().CancelTimer(HandleConnectTimeout, mpEndpoint);

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

    // Scan initiation performed async, to ensure that the BLE subsystem is initialized.
    PlatformMgr().ScheduleWork(InitiateScan, static_cast<intptr_t>(BleScanState::kScanForDiscriminator));
}

CHIP_ERROR BLEManagerImpl::CancelConnection()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                                 = DeviceEventType::kPlatformLinuxBLEPeripheralRegisterAppComplete;
    event.Platform.BLEPeripheralRegisterAppComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralRegisterAppComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvConfiguredComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type = DeviceEventType::kPlatformLinuxBLEPeripheralAdvConfiguredComplete;
    event.Platform.BLEPeripheralAdvConfiguredComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvConfiguredComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                              = DeviceEventType::kPlatformLinuxBLEPeripheralAdvStartComplete;
    event.Platform.BLEPeripheralAdvStartComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvStartComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                             = DeviceEventType::kPlatformLinuxBLEPeripheralAdvStopComplete;
    event.Platform.BLEPeripheralAdvStopComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvStopComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::OnDeviceScanned(BluezDevice1 * device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info)
{
    ChipLogProgress(Ble, "New device scanned: %s", bluez_device1_get_address(device));

    if (mBLEScanConfig.mBleScanState == BleScanState::kScanForDiscriminator)
    {
        if (!mBLEScanConfig.mDiscriminator.MatchesLongDiscriminator(info.GetDeviceDiscriminator()))
        {
            return;
        }
        ChipLogProgress(Ble, "Device discriminator match. Attempting to connect.");
    }
    else if (mBLEScanConfig.mBleScanState == BleScanState::kScanForAddress)
    {
        if (strcmp(bluez_device1_get_address(device), mBLEScanConfig.mAddress.c_str()) != 0)
        {
            return;
        }
        ChipLogProgress(Ble, "Device address match. Attempting to connect.");
    }
    else
    {
        // Internal consistency eerror
        ChipLogError(Ble, "Unknown discovery type. Ignoring scanned device.");
        return;
    }

    mBLEScanConfig.mBleScanState = BleScanState::kConnecting;
    DeviceLayer::SystemLayer().StartTimer(kConnectTimeout, HandleConnectTimeout, mpEndpoint);
    mDeviceScanner->StopScan();

    ConnectDevice(device, mpEndpoint);
}

void BLEManagerImpl::OnScanComplete()
{
    if (mBLEScanConfig.mBleScanState != BleScanState::kScanForDiscriminator &&
        mBLEScanConfig.mBleScanState != BleScanState::kScanForAddress)
    {
        ChipLogProgress(Ble, "Scan complete notification without an active scan.");
        return;
    }

    BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_TIMEOUT);
    mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
