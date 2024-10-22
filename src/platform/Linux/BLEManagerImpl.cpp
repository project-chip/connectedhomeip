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

#include <ble/Ble.h>
#include <lib/support/CHIPMemString.h>
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

} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    err = BleLayer::Init(this, this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART && !mIsCentral);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);

    memset(mDeviceName, 0, sizeof(mDeviceName));

    DeviceLayer::SystemLayer().ScheduleLambda([this] { DriveBLEState(); });

exit:
    return err;
}

void BLEManagerImpl::_Shutdown()
{
    // Make sure that timers are stopped before shutting down the BLE layer.
    DeviceLayer::SystemLayer().CancelTimer(HandleScanTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleAdvertisingTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleConnectTimer, this);

    mDeviceScanner.Shutdown();
    mBLEAdvertisement.Shutdown();
    mEndpoint.Shutdown();

    mBluezObjectManager.Shutdown();
    mFlags.Clear(Flags::kBluezManagerInitialized);
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
            ChipDeviceEvent connectionEvent{ .Type = DeviceEventType::kCHIPoBLEConnectionEstablished };
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    ChipLogDetail(DeviceLayer, "HandlePlatformSpecificBLEEvent %d", apEvent->Type);
    switch (apEvent->Type)
    {
    case DeviceEventType::kPlatformLinuxBLEAdapterAdded:
        ChipLogDetail(DeviceLayer, "BLE adapter added: id=%u address=%s", apEvent->Platform.BLEAdapter.mAdapterId,
                      apEvent->Platform.BLEAdapter.mAdapterAddress);
        if (apEvent->Platform.BLEAdapter.mAdapterId == mAdapterId)
        {
            mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
            DriveBLEState();
        }
        break;
    case DeviceEventType::kPlatformLinuxBLEAdapterRemoved:
        ChipLogDetail(DeviceLayer, "BLE adapter removed: id=%u address=%s", apEvent->Platform.BLEAdapter.mAdapterId,
                      apEvent->Platform.BLEAdapter.mAdapterAddress);
        if (apEvent->Platform.BLEAdapter.mAdapterId == mAdapterId)
        {
            // Shutdown all BLE operations and release resources
            mDeviceScanner.Shutdown();
            mBLEAdvertisement.Shutdown();
            mEndpoint.Shutdown();
            // Drop reference to the adapter
            mAdapter.reset();
            // Clear all flags related to BlueZ BLE operations
            mFlags.Clear(Flags::kBluezAdapterAvailable);
            mFlags.Clear(Flags::kBluezBLELayerInitialized);
            mFlags.Clear(Flags::kAdvertisingConfigured);
            mFlags.Clear(Flags::kAppRegistered);
            mFlags.Clear(Flags::kAdvertising);
            CleanScanConfig();
            // Indicate that the adapter is no longer available
            err = BLE_ERROR_ADAPTER_UNAVAILABLE;
        }
        break;
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
        mFlags.Clear(Flags::kControlOpInProgress).Clear(Flags::kAdvertisingRefreshNeeded);
        // Do not restart the timer if it is still active. This is to avoid the timer from being restarted
        // if the advertising is stopped due to a premature release.
        if (!DeviceLayer::SystemLayer().IsTimerActive(HandleAdvertisingTimer, this))
        {
            // Start a timer to make sure that the fast advertising is stopped after specified timeout.
            SuccessOrExit(err = DeviceLayer::SystemLayer().StartTimer(kFastAdvertiseTimeout, HandleAdvertisingTimer, this));
        }
        mFlags.Set(Flags::kAdvertising);
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralAdvStopComplete:
        SuccessOrExit(err = apEvent->Platform.BLEPeripheralAdvStopComplete.mError);
        mFlags.Clear(Flags::kControlOpInProgress).Clear(Flags::kAdvertisingRefreshNeeded);
        DeviceLayer::SystemLayer().CancelTimer(HandleAdvertisingTimer, this);
        // Transition to the not Advertising state...
        if (mFlags.Has(Flags::kAdvertising))
        {
            mFlags.Clear(Flags::kAdvertising);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");
        }
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralAdvReleased:
        // If the advertising was stopped due to a premature release, check if it needs to be restarted.
        mFlags.Clear(Flags::kAdvertising);
        DriveBLEState();
        break;
    case DeviceEventType::kPlatformLinuxBLEPeripheralRegisterAppComplete:
        SuccessOrExit(err = apEvent->Platform.BLEPeripheralRegisterAppComplete.mError);
        mFlags.Clear(Flags::kControlOpInProgress);
        mFlags.Set(Flags::kAppRegistered);
        DriveBLEState();
        break;
    default:
        break;
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        DisableBLEService(err);
        mFlags.Clear(Flags::kControlOpInProgress);
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
        ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLECentralConnected,
                               .Platform = { .BLECentralConnected = { .mConnection = conId } } };
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::HandleConnectFailed(CHIP_ERROR error)
{
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLECentralConnectFailed,
                               .Platform = { .BLECentralConnectFailed = { .mError = error } } };
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::HandleWriteComplete(BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLEWriteComplete,
                           .Platform = { .BLEWriteComplete = { .mConnection = conId } } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleSubscribeOpComplete(BLE_CONNECTION_OBJECT conId, bool subscribed)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLESubscribeOpComplete,
                           .Platform = { .BLESubscribeOpComplete = { .mConnection = conId, .mIsSubscribed = subscribed } } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleTXCharChanged(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    System::PacketBufferHandle buf(System::PacketBufferHandle::NewWithData(value, len));
    VerifyOrReturn(!buf.IsNull(), ChipLogError(DeviceLayer, "Failed to allocate packet buffer in %s", __func__));

    ChipLogDetail(DeviceLayer, "Indication received, conn = %p", conId);

    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLEIndicationReceived,
                           .Platform = {
                               .BLEIndicationReceived = { .mConnection = conId, .mData = std::move(buf).UnsafeRelease() } } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleRXCharWrite(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    // Copy the data to a packet buffer.
    System::PacketBufferHandle buf(System::PacketBufferHandle::NewWithData(value, len));
    VerifyOrReturn(!buf.IsNull(), ChipLogError(DeviceLayer, "Failed to allocate packet buffer in %s", __func__));

    ChipLogProgress(Ble, "Write request received, conn = %p", conId);

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    ChipDeviceEvent event{ .Type                  = DeviceEventType::kCHIPoBLEWriteReceived,
                           .CHIPoBLEWriteReceived = { .ConId = conId, .Data = std::move(buf).UnsafeRelease() } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // If this was a CHIPoBLE connection, post an event to deliver a connection error to the CHIPoBLE layer.
    ChipDeviceEvent event{ .Type                    = DeviceEventType::kCHIPoBLEConnectionError,
                           .CHIPoBLEConnectionError = { .ConId = conId, .Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleTXCharCCCDWrite(BLE_CONNECTION_OBJECT conId)
{
    VerifyOrReturn(conId != BLE_CONNECTION_UNINITIALIZED,
                   ChipLogError(DeviceLayer, "BLE connection is not initialized in %s", __func__));

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", conId->IsNotifyAcquired() ? "subscribe" : "unsubscribe");

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling indications.
    ChipDeviceEvent event{ .Type = conId->IsNotifyAcquired() ? static_cast<uint16_t>(DeviceEventType::kCHIPoBLESubscribe)
                                                             : static_cast<uint16_t>(DeviceEventType::kCHIPoBLEUnsubscribe),
                           .CHIPoBLESubscribe = { .ConId = conId } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleTXComplete(BLE_CONNECTION_OBJECT conId)
{
    // Post an event to the Chip queue to process the indicate confirmation.
    ChipDeviceEvent event{ .Type = DeviceEventType::kCHIPoBLEIndicateConfirm, .CHIPoBLEIndicateConfirm = { .ConId = conId } };
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

    // Initializes the BlueZ BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled)
    {
        if (!mFlags.Has(Flags::kBluezManagerInitialized))
        {
            SuccessOrExit(err = mBluezObjectManager.Init());
            mFlags.Set(Flags::kBluezManagerInitialized);
        }
        if (!mFlags.Has(Flags::kBluezAdapterAvailable))
        {
            mAdapter.reset(mBluezObjectManager.GetAdapter(mAdapterId));
            VerifyOrExit(mAdapter, err = BLE_ERROR_ADAPTER_UNAVAILABLE);
            mFlags.Set(Flags::kBluezAdapterAvailable);
        }
        if (!mFlags.Has(Flags::kBluezBLELayerInitialized))
        {
            SuccessOrExit(err = mEndpoint.Init(mAdapter.get(), mIsCentral));
            mFlags.Set(Flags::kBluezBLELayerInitialized);
        }
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
                SuccessOrExit(err = mBLEAdvertisement.Init(mAdapter.get(), mpBLEAdvUUID, mDeviceName));
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
        DisableBLEService(err);
    }
}

void BLEManagerImpl::DisableBLEService(CHIP_ERROR err)
{
    ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %" CHIP_ERROR_FORMAT, err.Format());
    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    // Stop all timers if the error is other than BLE adapter unavailable. In case of BLE adapter
    // beeing unavailable, we will keep timers running, as the adapter might become available in
    // the nearest future (e.g. BlueZ restart due to crash). By doing that we will ensure that BLE
    // adapter reappearance will not extend timeouts for the ongoing operations.
    if (err != BLE_ERROR_ADAPTER_UNAVAILABLE)
    {
        DeviceLayer::SystemLayer().CancelTimer(HandleScanTimer, this);
        DeviceLayer::SystemLayer().CancelTimer(HandleAdvertisingTimer, this);
        DeviceLayer::SystemLayer().CancelTimer(HandleConnectTimer, this);
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
    CHIP_ERROR err = CHIP_ERROR_INCORRECT_STATE;

    DriveBLEState();

    VerifyOrExit(scanType != BleScanState::kNotScanning,
                 ChipLogError(Ble, "Invalid scan type requested: %d", to_underlying(scanType)));
    VerifyOrExit(!mDeviceScanner.IsScanning(), ChipLogError(Ble, "BLE scan already in progress"));
    VerifyOrExit(mFlags.Has(Flags::kBluezAdapterAvailable), err = BLE_ERROR_ADAPTER_UNAVAILABLE);

    mBLEScanConfig.mBleScanState = scanType;

    err = mDeviceScanner.Init(mAdapter.get(), this);
    VerifyOrExit(err == CHIP_NO_ERROR, {
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        ChipLogError(Ble, "Failed to create BLE device scanner: %" CHIP_ERROR_FORMAT, err.Format());
    });

    err = mDeviceScanner.StartScan();
    VerifyOrExit(err == CHIP_NO_ERROR, {
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        ChipLogError(Ble, "Failed to start BLE scan: %" CHIP_ERROR_FORMAT, err.Format());
    });

    err = DeviceLayer::SystemLayer().StartTimer(kNewConnectionScanTimeout, HandleScanTimer, this);
    VerifyOrExit(err == CHIP_NO_ERROR, {
        mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
        mDeviceScanner.StopScan();
        ChipLogError(Ble, "Failed to start BLE scan timeout: %" CHIP_ERROR_FORMAT, err.Format());
    });

exit:
    if (err != CHIP_NO_ERROR)
    {
        BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, err);
    }
}

void BLEManagerImpl::HandleScanTimer(chip::System::Layer *, void * appState)
{
    auto * manager = static_cast<BLEManagerImpl *>(appState);
    manager->OnScanError(CHIP_ERROR_TIMEOUT);
    manager->mDeviceScanner.StopScan();
}

void BLEManagerImpl::CleanScanConfig()
{
    if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        DeviceLayer::SystemLayer().CancelTimer(HandleConnectTimer, this);

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
    {
        DeviceLayer::SystemLayer().CancelTimer(HandleScanTimer, this);
        mDeviceScanner.StopScan();
    }
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::NotifyBLEAdapterAdded(unsigned int aAdapterId, const char * aAdapterAddress)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLEAdapterAdded,
                           .Platform = { .BLEAdapter = { .mAdapterId = aAdapterId } } };
    Platform::CopyString(event.Platform.BLEAdapter.mAdapterAddress, aAdapterAddress);
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEAdapterRemoved(unsigned int aAdapterId, const char * aAdapterAddress)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLEAdapterRemoved,
                           .Platform = { .BLEAdapter = { .mAdapterId = aAdapterId } } };
    Platform::CopyString(event.Platform.BLEAdapter.mAdapterAddress, aAdapterAddress);
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralRegisterAppComplete(CHIP_ERROR error)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLEPeripheralRegisterAppComplete,
                           .Platform = { .BLEPeripheralRegisterAppComplete = { .mError = error } } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(CHIP_ERROR error)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLEPeripheralAdvStartComplete,
                           .Platform = { .BLEPeripheralAdvStartComplete = { .mError = error } } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(CHIP_ERROR error)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformLinuxBLEPeripheralAdvStopComplete,
                           .Platform = { .BLEPeripheralAdvStopComplete = { .mError = error } } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvReleased()
{
    ChipDeviceEvent event{ .Type = DeviceEventType::kPlatformLinuxBLEPeripheralAdvReleased };
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
    DeviceLayer::SystemLayer().CancelTimer(HandleScanTimer, this);
    mDeviceScanner.StopScan();
    // Stop scanning and then start connecting timer
    DeviceLayer::SystemLayer().StartTimer(kConnectTimeout, HandleConnectTimer, this);
    chip::DeviceLayer::PlatformMgr().UnlockChipStack();

    CHIP_ERROR err = mEndpoint.ConnectDevice(device);
    VerifyOrReturn(err == CHIP_NO_ERROR, ChipLogError(Ble, "Device connection failed: %" CHIP_ERROR_FORMAT, err.Format()));

    ChipLogProgress(Ble, "New device connected: %s", address);
}

void BLEManagerImpl::HandleConnectTimer(chip::System::Layer *, void * appState)
{
    auto * manager = static_cast<BLEManagerImpl *>(appState);
    manager->mEndpoint.CancelConnect();
    BLEManagerImpl::HandleConnectFailed(CHIP_ERROR_TIMEOUT);
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
