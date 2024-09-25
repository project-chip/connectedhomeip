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

/**
 * Note: BLEManager requires ConnectivityManager to be defined beforehand,
 *       otherwise we will face circular dependency between them. */
#include <platform/ConnectivityManager.h>

/**
 * Note: Use public include for BLEManager which includes our local
 *       platform/<PLATFORM>/BLEManagerImpl.h after defining interface class. */
#include "platform/internal/BLEManager.h"

#include <type_traits>
#include <utility>

#include <ble/CHIPBleServiceData.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>

#include "bluez/BluezEndpoint.h"

#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
#include <platform/DeviceControlServer.h>
#endif

using namespace ::nl;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

static constexpr System::Clock::Timeout kNewConnectionScanTimeout = System::Clock::Seconds16(20);
static constexpr System::Clock::Timeout kConnectTimeout           = System::Clock::Seconds16(20);
static constexpr System::Clock::Timeout kFastAdvertiseTimeout =
    System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
// The CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_CHANGE_TIME_MS specifies the transition time
// starting from advertisement commencement. Since the extended advertisement timer is started after
// the fast-to-slow transition, we have to subtract the time spent in fast advertising.
static constexpr System::Clock::Timeout kSlowAdvertiseTimeout = System::Clock::Milliseconds32(
    CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_CHANGE_TIME_MS - CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
static_assert(CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_CHANGE_TIME_MS >=
                  CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME,
              "The extended advertising interval change time must be greater than the fast advertising interval change time");
#endif

void HandleConnectTimeout(chip::System::Layer *, void * apEndpoint)
{
    VerifyOrDie(apEndpoint != nullptr);
    static_cast<BluezEndpoint *>(apEndpoint)->CancelConnect();
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

    DeviceLayer::SystemLayer().ScheduleLambda([this] { DriveBLEState(); });

exit:
    return err;
}

void BLEManagerImpl::_Shutdown()
{
    // Ensure scan resources are cleared (e.g. timeout timers).
    mDeviceScanner.Shutdown();
    // Stop advertising and free resources.
    mBLEAdvertisement.Shutdown();
    // Make sure that the endpoint is not used by the timer.
    DeviceLayer::SystemLayer().CancelTimer(HandleConnectTimeout, &mEndpoint);
    // Release BLE connection resources (unregister from BlueZ).
    mEndpoint.Shutdown();
    mFlags.Clear(Flags::kBluezBLELayerInitialized);
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (mFlags.Has(Flags::kAdvertisingEnabled) != val)
    {
        mFlags.Set(Flags::kAdvertisingEnabled, val);
    }

    DeviceLayer::SystemLayer().ScheduleLambda([this] { DriveBLEState(); });

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
    DeviceLayer::SystemLayer().ScheduleLambda([this] { DriveBLEState(); });
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
    mAdapterId   = aAdapterId;
    mIsCentral   = aIsCentral;
    mpBLEAdvUUID = "0xFFF6";
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe:
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        {
            ChipDeviceEvent connectionEvent;
            connectionEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
            PlatformMgr().PostEventOrDie(&connectionEvent);
        }
        break;

    case DeviceEventType::kCHIPoBLEUnsubscribe:
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        break;

    case DeviceEventType::kCHIPoBLEWriteReceived:
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
        break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm:
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
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
        HandleWriteConfirmation(apEvent->Platform.BLEWriteComplete.mConnection, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID);
        break;
    case DeviceEventType::kPlatformLinuxBLESubscribeOpComplete:
        if (apEvent->Platform.BLESubscribeOpComplete.mIsSubscribed)
            HandleSubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &CHIP_BLE_SVC_ID,
                                    &Ble::CHIP_BLE_CHAR_2_UUID);
        else
            HandleUnsubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &CHIP_BLE_SVC_ID,
                                      &Ble::CHIP_BLE_CHAR_2_UUID);
        break;
    case DeviceEventType::kPlatformLinuxBLEIndicationReceived:
        HandleIndicationReceived(apEvent->Platform.BLEIndicationReceived.mConnection, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID,
                                 PacketBufferHandle::Adopt(apEvent->Platform.BLEIndicationReceived.mData));
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralAdvStartComplete:
        SuccessOrExit(err = apEvent->Platform.BLEPeripheralAdvStartComplete.mError);
        sInstance.mFlags.Clear(Flags::kControlOpInProgress).Clear(Flags::kAdvertisingRefreshNeeded);
        // Do not restart the timer if it is still active. This is to avoid the timer from being restarted
        // if the advertising is stopped due to a premature release.
        if (!DeviceLayer::SystemLayer().IsTimerActive(HandleAdvertisingTimer, this))
        {
            // Start a timer to make sure that the fast advertising is stopped after specified timeout.
            SuccessOrExit(err = DeviceLayer::SystemLayer().StartTimer(kFastAdvertiseTimeout, HandleAdvertisingTimer, this));
        }
        sInstance.mFlags.Set(Flags::kAdvertising);
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralAdvStopComplete:
        SuccessOrExit(err = apEvent->Platform.BLEPeripheralAdvStopComplete.mError);
        sInstance.mFlags.Clear(Flags::kControlOpInProgress).Clear(Flags::kAdvertisingRefreshNeeded);
        DeviceLayer::SystemLayer().CancelTimer(HandleAdvertisingTimer, this);

        // Transition to the not Advertising state...
        if (sInstance.mFlags.Has(Flags::kAdvertising))
        {
            sInstance.mFlags.Clear(Flags::kAdvertising);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");
        }
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralAdvReleased:
        // If the advertising was stopped due to a premature release, check if it needs to be restarted.
        sInstance.mFlags.Clear(Flags::kAdvertising);
        DriveBLEState();
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralRegisterAppComplete:
        SuccessOrExit(err = apEvent->Platform.BLEPeripheralRegisterAppComplete.mError);
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
        DeviceLayer::SystemLayer().CancelTimer(HandleAdvertisingTimer, this);
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
    uint16_t mtu = 0;
    VerifyOrExit(conId != BLE_CONNECTION_UNINITIALIZED,
                 ChipLogError(DeviceLayer, "BLE connection is not initialized in %s", __func__));
    mtu = conId->GetMTU();
exit:
    return mtu;
}

CHIP_ERROR BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                   const ChipBleUUID * charId)
{
    CHIP_ERROR err = BLE_ERROR_GATT_SUBSCRIBE_FAILED;

    VerifyOrExit(conId != BLE_CONNECTION_UNINITIALIZED,
                 ChipLogError(DeviceLayer, "BLE connection is not initialized in %s", __func__));
    VerifyOrExit(Ble::UUIDsMatch(svcId, &CHIP_BLE_SVC_ID),
                 ChipLogError(DeviceLayer, "SubscribeCharacteristic() called with invalid service ID"));
    VerifyOrExit(Ble::UUIDsMatch(charId, &Ble::CHIP_BLE_CHAR_2_UUID),
                 ChipLogError(DeviceLayer, "SubscribeCharacteristic() called with invalid characteristic ID"));
    err = conId->SubscribeCharacteristic();

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                     const ChipBleUUID * charId)
{
    CHIP_ERROR err = BLE_ERROR_GATT_UNSUBSCRIBE_FAILED;

    VerifyOrExit(conId != BLE_CONNECTION_UNINITIALIZED,
                 ChipLogError(DeviceLayer, "BLE connection is not initialized in %s", __func__));
    VerifyOrExit(Ble::UUIDsMatch(svcId, &CHIP_BLE_SVC_ID),
                 ChipLogError(DeviceLayer, "UnsubscribeCharacteristic() called with invalid service ID"));
    VerifyOrExit(Ble::UUIDsMatch(charId, &Ble::CHIP_BLE_CHAR_2_UUID),
                 ChipLogError(DeviceLayer, "UnsubscribeCharacteristic() called with invalid characteristic ID"));
    err = conId->UnsubscribeCharacteristic();

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;

    VerifyOrExit(conId != BLE_CONNECTION_UNINITIALIZED,
                 ChipLogError(DeviceLayer, "BLE connection is not initialized in %s", __func__));
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %p)", conId);
    err = conId->CloseConnection();

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                          chip::System::PacketBufferHandle pBuf)
{
    CHIP_ERROR err = BLE_ERROR_GATT_INDICATE_FAILED;

    VerifyOrExit(conId != BLE_CONNECTION_UNINITIALIZED,
                 ChipLogError(DeviceLayer, "BLE connection is not initialized in %s", __func__));
    err = conId->SendIndication(std::move(pBuf));

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                            const Ble::ChipBleUUID * charId, chip::System::PacketBufferHandle pBuf)
{
    CHIP_ERROR err = BLE_ERROR_GATT_WRITE_FAILED;

    VerifyOrExit(conId != BLE_CONNECTION_UNINITIALIZED,
                 ChipLogError(DeviceLayer, "BLE connection is not initialized in %s", __func__));
    VerifyOrExit(Ble::UUIDsMatch(svcId, &CHIP_BLE_SVC_ID),
                 ChipLogError(DeviceLayer, "SendWriteRequest() called with invalid service ID"));
    VerifyOrExit(Ble::UUIDsMatch(charId, &Ble::CHIP_BLE_CHAR_1_UUID),
                 ChipLogError(DeviceLayer, "SendWriteRequest() called with invalid characteristic ID"));
    err = conId->SendWriteRequest(std::move(pBuf));

exit:
    return err;
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
    VerifyOrReturn(conId != BLE_CONNECTION_UNINITIALIZED,
                   ChipLogError(DeviceLayer, "BLE connection is not initialized in %s", __func__));

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling indications.
    ChipDeviceEvent event;
    event.Type = conId->IsNotifyAcquired() ? DeviceEventType::kCHIPoBLESubscribe : DeviceEventType::kCHIPoBLEUnsubscribe;
    event.CHIPoBLESubscribe.ConId = conId;
    PlatformMgr().PostEventOrDie(&event);

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received",
                    (event.Type == DeviceEventType::kCHIPoBLESubscribe) ? "subscribe" : "unsubscribe");
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
        SuccessOrExit(err = mEndpoint.Init(mIsCentral, mAdapterId));
        mFlags.Set(Flags::kBluezBLELayerInitialized);
    }

    // Register the CHIPoBLE application with the Bluez BLE layer if needed.
    if (!mIsCentral && mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !mFlags.Has(Flags::kAppRegistered))
    {
        SuccessOrExit(err = mEndpoint.RegisterGattApplication());
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

            // Configure advertising data if it hasn't been done yet.
            if (!mFlags.Has(Flags::kAdvertisingConfigured))
            {
                SuccessOrExit(err = mBLEAdvertisement.Init(mEndpoint, mpBLEAdvUUID, mDeviceName));
                mFlags.Set(Flags::kAdvertisingConfigured);
            }

            // Setup service data for advertising.
            auto serviceDataFlags = BluezAdvertisement::kServiceDataNone;
#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
            if (mFlags.Has(Flags::kExtAdvertisingEnabled))
                serviceDataFlags |= BluezAdvertisement::kServiceDataExtendedAnnouncement;
#endif
            SuccessOrExit(err = mBLEAdvertisement.SetupServiceData(serviceDataFlags));

            // Set or update the advertising intervals.
            SuccessOrExit(err = mBLEAdvertisement.SetIntervals(GetAdvertisingIntervals()));

            if (!mFlags.Has(Flags::kAdvertising))
            {
                // Start advertising. This is an asynchronous step. BLE manager will be notified of
                // advertising start completion via a call to NotifyBLEPeripheralAdvStartComplete.
                SuccessOrExit(err = mBLEAdvertisement.Start());
                mFlags.Set(Flags::kControlOpInProgress);
                ExitNow();
            }
        }
    }

    // Otherwise stop advertising if needed...
    else
    {
        if (mFlags.Has(Flags::kAdvertising))
        {
            SuccessOrExit(err = mBLEAdvertisement.Stop());
            mFlags.Set(Flags::kControlOpInProgress);

            ExitNow();
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        DeviceLayer::SystemLayer().CancelTimer(HandleAdvertisingTimer, this);
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(Ble, "Got notification regarding chip connection closure");
#if CHIP_DEVICE_CONFIG_ENABLE_WPA && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    if (mState == kState_NotInitialized)
    {
        // Close BLE GATT connections to disconnect BlueZ
        CloseConnection(conId);
        // In Non-Concurrent mode start the Wi-Fi, as BLE has been stopped
        DeviceLayer::ConnectivityMgrImpl().StartNonConcurrentWiFiManagement();
    }
#endif // CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
}

void BLEManagerImpl::CheckNonConcurrentBleClosing()
{
#if CHIP_DEVICE_CONFIG_ENABLE_WPA && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    if (mState == kState_Disconnecting)
    {
        DeviceLayer::DeviceControlServer::DeviceControlSvr().PostCloseAllBLEConnectionsToOperationalNetworkEvent();
    }
#endif
}

BluezAdvertisement::AdvertisingIntervals BLEManagerImpl::GetAdvertisingIntervals() const
{
    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
        return { CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN, CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX };
#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
    if (mFlags.Has(Flags::kExtAdvertisingEnabled))
        return { CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MIN, CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MAX };
#endif
    return { CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN, CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX };
}

void BLEManagerImpl::HandleAdvertisingTimer(chip::System::Layer *, void * appState)
{
    auto * self = static_cast<BLEManagerImpl *>(appState);

    if (self->mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Start slow advertisement");
        self->_SetAdvertisingMode(BLEAdvertisingMode::kSlowAdvertising);
#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
        self->mFlags.Clear(Flags::kExtAdvertisingEnabled);
        DeviceLayer::SystemLayer().StartTimer(kSlowAdvertiseTimeout, HandleAdvertisingTimer, self);
    }
    else
    {
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Start extended advertisement");
        self->mFlags.Set(Flags::kExtAdvertisingEnabled);
        // This will trigger advertising intervals update in the DriveBLEState() function.
        self->_SetAdvertisingMode(BLEAdvertisingMode::kSlowAdvertising);
#endif
    }
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

    if (!mFlags.Has(Flags::kBluezBLELayerInitialized))
    {
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_INCORRECT_STATE);
        ChipLogError(Ble, "BLE Layer is not yet initialized");
        return;
    }

    if (mEndpoint.GetAdapter() == nullptr)
    {
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_INCORRECT_STATE);
        ChipLogError(Ble, "No adapter available for new connection establishment");
        return;
    }

    mBLEScanConfig.mBleScanState = scanType;

    CHIP_ERROR err = mDeviceScanner.Init(mEndpoint.GetAdapter(), this);
    if (err != CHIP_NO_ERROR)
    {
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_INTERNAL);
        ChipLogError(Ble, "Failed to create a BLE device scanner");
        return;
    }

    err = mDeviceScanner.StartScan(kNewConnectionScanTimeout);
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
        DeviceLayer::SystemLayer().CancelTimer(HandleConnectTimeout, &mEndpoint);

    mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
}

void BLEManagerImpl::NewConnection(BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator)
{
    mBLEScanConfig.mDiscriminator = connDiscriminator;
    mBLEScanConfig.mAppState      = appState;

    // Scan initiation performed async, to ensure that the BLE subsystem is initialized.
    DeviceLayer::SystemLayer().ScheduleLambda([this] { InitiateScan(BleScanState::kScanForDiscriminator); });
}

CHIP_ERROR BLEManagerImpl::CancelConnection()
{
    if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        mEndpoint.CancelConnect();
    // If in discovery mode, stop scan.
    else if (mBLEScanConfig.mBleScanState != BleScanState::kNotScanning)
        mDeviceScanner.StopScan();
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(CHIP_ERROR error)
{
    ChipDeviceEvent event;
    event.Type                                             = DeviceEventType::kPlatformLinuxBLEPeripheralRegisterAppComplete;
    event.Platform.BLEPeripheralRegisterAppComplete.mError = error;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(CHIP_ERROR error)
{
    ChipDeviceEvent event;
    event.Type                                          = DeviceEventType::kPlatformLinuxBLEPeripheralAdvStartComplete;
    event.Platform.BLEPeripheralAdvStartComplete.mError = error;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(CHIP_ERROR error)
{
    ChipDeviceEvent event;
    event.Type                                         = DeviceEventType::kPlatformLinuxBLEPeripheralAdvStopComplete;
    event.Platform.BLEPeripheralAdvStopComplete.mError = error;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvReleased()
{
    ChipDeviceEvent event;
    event.Type = DeviceEventType::kPlatformLinuxBLEPeripheralAdvReleased;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::OnDeviceScanned(BluezDevice1 & device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info)
{
    const char * address = bluez_device1_get_address(&device);
    ChipLogProgress(Ble, "New device scanned: %s", address);

    if (mBLEScanConfig.mBleScanState == BleScanState::kScanForDiscriminator)
    {
        auto isMatch = mBLEScanConfig.mDiscriminator.MatchesLongDiscriminator(info.GetDeviceDiscriminator());
        VerifyOrReturn(
            isMatch,
            ChipLogError(Ble, "Skip connection: Device discriminator does not match: %u != %u", info.GetDeviceDiscriminator(),
                         mBLEScanConfig.mDiscriminator.IsShortDiscriminator() ? mBLEScanConfig.mDiscriminator.GetShortValue()
                                                                              : mBLEScanConfig.mDiscriminator.GetLongValue()));
        ChipLogProgress(Ble, "Device discriminator match. Attempting to connect.");
    }
    else if (mBLEScanConfig.mBleScanState == BleScanState::kScanForAddress)
    {
        auto isMatch = strcmp(address, mBLEScanConfig.mAddress.c_str()) == 0;
        VerifyOrReturn(isMatch,
                       ChipLogError(Ble, "Skip connection: Device address does not match: %s != %s", address,
                                    mBLEScanConfig.mAddress.c_str()));
        ChipLogProgress(Ble, "Device address match. Attempting to connect.");
    }
    else
    {
        // Internal consistency error
        ChipLogError(Ble, "Unknown discovery type. Ignoring scanned device.");
        return;
    }

    mBLEScanConfig.mBleScanState = BleScanState::kConnecting;

    chip::DeviceLayer::PlatformMgr().LockChipStack();
    // We StartScan in the ChipStack thread.
    // StopScan should also be performed in the ChipStack thread.
    // At the same time, the scan timer also needs to be canceled in the ChipStack thread.
    mDeviceScanner.StopScan();
    // Stop scanning and then start connecting timer
    DeviceLayer::SystemLayer().StartTimer(kConnectTimeout, HandleConnectTimeout, &mEndpoint);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    CHIP_ERROR err = mEndpoint.ConnectDevice(device);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Ble, "Device connection failed: %" CHIP_ERROR_FORMAT, err.Format()));

    ChipLogProgress(Ble, "New device connected: %s", address);
}

void BLEManagerImpl::OnScanComplete()
{
    switch (mBLEScanConfig.mBleScanState)
    {
    case BleScanState::kNotScanning:
        ChipLogProgress(Ble, "Scan complete notification without an active scan.");
        break;
    case BleScanState::kScanForAddress:
    case BleScanState::kScanForDiscriminator:
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        ChipLogProgress(Ble, "Scan complete. No matching device found.");
        break;
    case BleScanState::kConnecting:
        break;
    }
}

void BLEManagerImpl::OnScanError(CHIP_ERROR err)
{
    BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, err);
    ChipLogError(Ble, "BLE scan error: %" CHIP_ERROR_FORMAT, err.Format());
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
