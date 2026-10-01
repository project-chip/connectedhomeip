/*
 *
 *    Copyright (c) 2020-2025 Project CHIP Authors
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
 *          for webOS platforms.
 */

/**
 * Note: BLEManager requires ConnectivityManager to be defined beforehand,
 *       otherwise we will face circular dependency between them. */
#include <platform/ConnectivityManager.h>

/**
 * Note: Use public include for BLEManager which includes our local
 *       platform/<PLATFORM>/BLEManagerImpl.h after defining interface class. */
#include "platform/internal/BLEManager.h"

#include <cassert>
#include <type_traits>
#include <utility>

#include <glib.h>

#include <ble/Ble.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/CommissionableDataProvider.h>

#include "wbs/ChipDeviceScanner.h"
#include "wbs/Helper.h"

#if !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
#include <platform/DeviceControlServer.h>
#endif

using namespace ::nl;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

#if defined(LGE_BLE_NEW_CONNECTION_SCAN_TIMEOUT)
static constexpr System::Clock::Timeout kNewConnectionScanTimeout = System::Clock::Seconds16(LGE_BLE_NEW_CONNECTION_SCAN_TIMEOUT);
#else
static constexpr System::Clock::Timeout kNewConnectionScanTimeout = System::Clock::Seconds16(20);
#endif
// wbs connection cannot be cancelled (CancelConnect is not implemented) and includes retries,
// so the timeout is longer than on webOS (20 s).
static constexpr System::Clock::Timeout kConnectTimeout = System::Clock::Seconds16(40);

// Parameters of a connection attempt executed on the Matter GLib context.
struct ConnectRequest
{
    ConnectRequest(const std::string & address, WbsEndpoint * endpoint) : mAddress(address), mEndpoint(endpoint) {}
    std::string mAddress;
    WbsEndpoint * mEndpoint;
};

} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    ReturnErrorOnFailure(BleLayer::Init(this, this, this, &DeviceLayer::SystemLayer()));

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    // Advertising (peripheral role) is not supported by the wbs layer.
    mFlags.ClearAll();

    memset(mDeviceName, 0, sizeof(mDeviceName));

    return DeviceLayer::SystemLayer().ScheduleLambda([this] { DriveBLEState(); });
}

void BLEManagerImpl::_Shutdown()
{
    // Make sure that timers are stopped before shutting down the BLE layer.
    DeviceLayer::SystemLayer().CancelTimer(HandleScanTimer, this);
    DeviceLayer::SystemLayer().CancelTimer(HandleConnectTimer, this);
    CancelConnect(mpEndpoint);

    mDeviceScanner.Shutdown();
    // Release BLE connection resources
    ReleaseEndpoint();
    mFlags.Clear(Flags::kWBSManagerInitialized).Clear(Flags::kWBSAdapterAvailable).Clear(Flags::kWBSBLELayerInitialized);
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    VerifyOrReturnError(!val, CHIP_ERROR_NOT_IMPLEMENTED, ChipLogError(DeviceLayer, "BLE advertising is not supported on webOS"));
    mFlags.Clear(Flags::kAdvertisingEnabled);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    ChipLogError(DeviceLayer, "BLE advertising is not supported on webOS");
    return CHIP_ERROR_NOT_IMPLEMENTED;
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
    return (mpEndpoint != nullptr && mpEndpoint->mConnectionMap != nullptr)
        ? static_cast<uint16_t>(g_hash_table_size(mpEndpoint->mConnectionMap))
        : 0;
}

void BLEManagerImpl::ReleaseEndpoint()
{
    VerifyOrReturn(mpEndpoint != nullptr);
    // The connection table is used on the Matter GLib context (Helper), so release it there.
    TEMPORARY_RETURN_IGNORED PlatformMgrImpl().GLibMatterContextInvokeSync(
        +[](WbsEndpoint * endpoint) { return ShutdownWbsLayer(endpoint); }, mpEndpoint);
    mpEndpoint = nullptr;
}

CHIP_ERROR BLEManagerImpl::ConfigureBle(uint32_t aAdapterId, bool aIsCentral)
{
    mAdapterId = aAdapterId;
    mIsCentral = aIsCentral;
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLEConnectionError:
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
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
    case DeviceEventType::kPlatformWebOSBLEAdapterAdded:
        ChipLogDetail(DeviceLayer, "BLE adapter added: id=%u address=%s", apEvent->Platform.BLEAdapter.mAdapterId,
                      apEvent->Platform.BLEAdapter.mAdapterAddress);
        if (apEvent->Platform.BLEAdapter.mAdapterId == mAdapterId)
        {
            mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
            DriveBLEState();
        }
        break;
    case DeviceEventType::kPlatformWebOSBLEAdapterRemoved:
        ChipLogDetail(DeviceLayer, "BLE adapter removed: id=%u address=%s", apEvent->Platform.BLEAdapter.mAdapterId,
                      apEvent->Platform.BLEAdapter.mAdapterAddress);
        if (apEvent->Platform.BLEAdapter.mAdapterId == mAdapterId)
        {
            // Shutdown all BLE operations and release resources
            mDeviceScanner.Shutdown();
            ReleaseEndpoint();
            // Clear all flags related to WBS BLE operations
            mFlags.Clear(Flags::kWBSAdapterAvailable);
            mFlags.Clear(Flags::kWBSBLELayerInitialized);
            CleanScanConfig();
            // Indicate that the adapter is no longer available
            err = BLE_ERROR_ADAPTER_UNAVAILABLE;
        }
        break;
    case DeviceEventType::kPlatformWebOSBLECentralConnected:
        if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        {
            BleConnectionDelegate::OnConnectionComplete(mBLEScanConfig.mAppState,
                                                        apEvent->Platform.BLECentralConnected.mConnection);
            CleanScanConfig();
        }
        else
        {
            // The connection completed after the attempt timed out or was cancelled: nobody will use it.
            ChipLogProgress(Ble, "Releasing BLE connection completed after timeout/cancel");
            TEMPORARY_RETURN_IGNORED CloseConnection(apEvent->Platform.BLECentralConnected.mConnection);
        }
        break;
    case DeviceEventType::kPlatformWebOSBLECentralConnectFailed:
        if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        {
            BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, apEvent->Platform.BLECentralConnectFailed.mError);
            CleanScanConfig();
        }
        break;
    case DeviceEventType::kPlatformWebOSBLEWriteComplete:
        HandleWriteConfirmation(apEvent->Platform.BLEWriteComplete.mConnection, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID);
        break;
    case DeviceEventType::kPlatformWebOSBLESubscribeOpComplete:
        if (apEvent->Platform.BLESubscribeOpComplete.mIsSubscribed)
            HandleSubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &CHIP_BLE_SVC_ID,
                                    &Ble::CHIP_BLE_CHAR_2_UUID);
        else
            HandleUnsubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &CHIP_BLE_SVC_ID,
                                      &Ble::CHIP_BLE_CHAR_2_UUID);
        break;
    case DeviceEventType::kPlatformWebOSBLEIndicationReceived:
        HandleIndicationReceived(apEvent->Platform.BLEIndicationReceived.mConnection, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID,
                                 PacketBufferHandle::Adopt(apEvent->Platform.BLEIndicationReceived.mData));
        break;
    default:
        break;
    }

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
    mtu = conId->mMtu;
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
    err = WbsSubscribeCharacteristic(conId);

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
    err = WbsUnsubscribeCharacteristic(conId);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    CHIP_ERROR err = CHIP_ERROR_INTERNAL;

    VerifyOrExit(conId != BLE_CONNECTION_UNINITIALIZED,
                 ChipLogError(DeviceLayer, "BLE connection is not initialized in %s", __func__));
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %p)", conId);
    err = CloseWbsConnection(conId);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                          chip::System::PacketBufferHandle pBuf)
{
    // Peripheral (GATT server) role is not supported by the wbs layer.
    ChipLogError(Ble, "SendIndication: Not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
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
    err = WbsSendWriteRequest(conId, std::move(pBuf));

exit:
    return err;
}

void BLEManagerImpl::HandleNewConnection(BLE_CONNECTION_OBJECT conId)
{
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformWebOSBLECentralConnected,
                               .Platform = { .BLECentralConnected = { .mConnection = conId } } };
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::HandleConnectFailed(CHIP_ERROR error)
{
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformWebOSBLECentralConnectFailed,
                               .Platform = { .BLECentralConnectFailed = { .mError = error } } };
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::HandleWriteComplete(BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformWebOSBLEWriteComplete,
                           .Platform = { .BLEWriteComplete = { .mConnection = conId } } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleSubscribeOpComplete(BLE_CONNECTION_OBJECT conId, bool subscribed)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformWebOSBLESubscribeOpComplete,
                           .Platform = { .BLESubscribeOpComplete = { .mConnection = conId, .mIsSubscribed = subscribed } } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleTXCharChanged(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    System::PacketBufferHandle buf(System::PacketBufferHandle::NewWithData(value, len));
    VerifyOrReturn(!buf.IsNull(), ChipLogError(DeviceLayer, "Failed to allocate packet buffer in %s", __func__));

    // ChipLogDetail(DeviceLayer, "Indication received, conn = %p", conId);

    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformWebOSBLEIndicationReceived,
                           .Platform = {
                               .BLEIndicationReceived = { .mConnection = conId, .mData = std::move(buf).UnsafeRelease() } } };
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::CHIPoWbs_ConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // If this was a CHIPoBLE connection, post an event to deliver a connection error to the CHIPoBLE layer.
    ChipDeviceEvent event{ .Type                    = DeviceEventType::kCHIPoBLEConnectionError,
                           .CHIPoBLEConnectionError = { .ConId = conId, .Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED } };
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

    // Initializes the WBS BLE layer if needed.
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled)
    {
        if (!mFlags.Has(Flags::kWBSManagerInitialized))
        {
            // LsRequester (LS2 client) is created lazily on first use.
            mFlags.Set(Flags::kWBSManagerInitialized);
        }
        if (!mFlags.Has(Flags::kWBSAdapterAvailable))
        {
            // The wbs layer does not track the adapter state: it is assumed to be available,
            // failures are reported by the individual bluetooth2 calls.
            mFlags.Set(Flags::kWBSAdapterAvailable);
        }
        if (!mFlags.Has(Flags::kWBSBLELayerInitialized))
        {
            SuccessOrExit(err = InitConnectionData(mIsCentral, mpEndpoint));
            mFlags.Set(Flags::kWBSBLELayerInitialized);
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
    // the nearest future (e.g. WBS restart due to crash). By doing that we will ensure that BLE
    // adapter reappearance will not extend timeouts for the ongoing operations.
    if (err != BLE_ERROR_ADAPTER_UNAVAILABLE)
    {
        DeviceLayer::SystemLayer().CancelTimer(HandleScanTimer, this);
        DeviceLayer::SystemLayer().CancelTimer(HandleConnectTimer, this);
    }
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(Ble, "Got notification regarding chip connection closure");
#if CHIP_DEVICE_CONFIG_ENABLE_WPA && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    if (mState == kState_NotInitialized)
    {
        // Close BLE GATT connections to disconnect WBS
        TEMPORARY_RETURN_IGNORED CloseConnection(conId);
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

void BLEManagerImpl::InitiateScan(BleScanState scanType)
{
    CHIP_ERROR err = CHIP_ERROR_INCORRECT_STATE;

    DriveBLEState();

    VerifyOrExit(scanType != BleScanState::kNotScanning,
                 ChipLogError(Ble, "Invalid scan type requested: %d", to_underlying(scanType)));
    VerifyOrExit(!mDeviceScanner.IsScanning(), ChipLogError(Ble, "BLE scan already in progress"));
    VerifyOrExit(mFlags.Has(Flags::kWBSAdapterAvailable), err = BLE_ERROR_ADAPTER_UNAVAILABLE);
    VerifyOrExit(mpEndpoint != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    ChipLogProgress(Ble, "mBleScanState (%d -> %d)", to_underlying(mBLEScanConfig.mBleScanState), to_underlying(scanType));
    mBLEScanConfig.mBleScanState = scanType;

    err = mDeviceScanner.Init(this);
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
        TEMPORARY_RETURN_IGNORED mDeviceScanner.StopScan();
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
    TEMPORARY_RETURN_IGNORED manager->mDeviceScanner.StopScan();
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
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().ScheduleLambda([this] { InitiateScan(BleScanState::kScanForDiscriminator); });
}

CHIP_ERROR BLEManagerImpl::CancelConnection()
{
    if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
    {
        CancelConnect(mpEndpoint);
    }
    // If in discovery mode, stop scan.
    else if (mBLEScanConfig.mBleScanState != BleScanState::kNotScanning)
    {
        DeviceLayer::SystemLayer().CancelTimer(HandleScanTimer, this);
        TEMPORARY_RETURN_IGNORED mDeviceScanner.StopScan();
    }
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::NotifyBLEAdapterAdded(unsigned int aAdapterId, const char * aAdapterAddress)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformWebOSBLEAdapterAdded,
                           .Platform = { .BLEAdapter = { .mAdapterId = aAdapterId } } };
    Platform::CopyString(event.Platform.BLEAdapter.mAdapterAddress, aAdapterAddress);
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEAdapterRemoved(unsigned int aAdapterId, const char * aAdapterAddress)
{
    ChipDeviceEvent event{ .Type     = DeviceEventType::kPlatformWebOSBLEAdapterRemoved,
                           .Platform = { .BLEAdapter = { .mAdapterId = aAdapterId } } };
    Platform::CopyString(event.Platform.BLEAdapter.mAdapterAddress, aAdapterAddress);
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::OnDeviceScanned(const pbnjson::JValue & device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info)
{
    // Called on the LsRequester (lsTask) thread. Hand the result over to the Matter thread: connecting here would
    // block the thread that delivers LS2 replies, and the scan state must not be touched without the stack lock.
    {
        std::lock_guard<std::mutex> lock(mScanResultMutex);
        // The previous result is still being processed; the device keeps advertising, so it will be reported again.
        VerifyOrReturn(!mScanResultPending);
        mScanResultAddress       = device["address"].asString();
        mScanResultDiscriminator = info.GetDeviceDiscriminator();
        mScanResultPending       = true;
    }

    CHIP_ERROR err = PlatformMgr().ScheduleWork(
        [](intptr_t arg) { reinterpret_cast<BLEManagerImpl *>(arg)->ProcessScanResult(); }, reinterpret_cast<intptr_t>(this));
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(Ble, "Failed to schedule scan result processing: %" CHIP_ERROR_FORMAT, err.Format());
        std::lock_guard<std::mutex> lock(mScanResultMutex);
        mScanResultPending = false;
    }
}

void BLEManagerImpl::ProcessScanResult()
{
    std::string address;
    uint16_t discriminator;
    {
        std::lock_guard<std::mutex> lock(mScanResultMutex);
        address            = std::move(mScanResultAddress);
        discriminator      = mScanResultDiscriminator;
        mScanResultPending = false;
    }

    ChipLogProgress(Ble, "New device scanned: %s discriminator : %u", address.c_str(), discriminator);

    if (mBLEScanConfig.mBleScanState == BleScanState::kScanForDiscriminator)
    {
        auto isMatch = mBLEScanConfig.mDiscriminator.MatchesLongDiscriminator(discriminator);
        VerifyOrReturn(
            isMatch,
            ChipLogError(Ble, "Skip connection: Device discriminator does not match: %u != %u", discriminator,
                         mBLEScanConfig.mDiscriminator.IsShortDiscriminator() ? mBLEScanConfig.mDiscriminator.GetShortValue()
                                                                              : mBLEScanConfig.mDiscriminator.GetLongValue()));
        ChipLogProgress(Ble, "Device discriminator match. Attempting to connect.");
    }
    else if (mBLEScanConfig.mBleScanState == BleScanState::kScanForAddress)
    {
        auto isMatch = address == mBLEScanConfig.mAddress;
        VerifyOrReturn(isMatch,
                       ChipLogError(Ble, "Skip connection: Device address does not match: %s != %s", address.c_str(),
                                    mBLEScanConfig.mAddress.c_str()));
        ChipLogProgress(Ble, "Device address match. Attempting to connect.");
    }
    else
    {
        // Late result of a scan that has already been stopped.
        ChipLogDetail(Ble, "No active discovery. Ignoring scanned device.");
        return;
    }

    mBLEScanConfig.mBleScanState = BleScanState::kConnecting;

    DeviceLayer::SystemLayer().CancelTimer(HandleScanTimer, this);
    TEMPORARY_RETURN_IGNORED mDeviceScanner.StopScan();
    // Stop scanning and then start connecting timer
    TEMPORARY_RETURN_IGNORED DeviceLayer::SystemLayer().StartTimer(kConnectTimeout, HandleConnectTimer, this);

    StartConnect(address);
}

void BLEManagerImpl::StartConnect(const std::string & address)
{
    // ConnectDevice() is blocking (synchronous LS2 calls with retries), so run it on the Matter GLib
    // context instead of the Matter thread. The result is reported via HandleNewConnection() or
    // HandleConnectFailed(). The source has the same priority as GLibMatterContextInvokeSync(), so a
    // later ReleaseEndpoint() is executed after this connection attempt.
    auto * request = Platform::New<ConnectRequest>(address, mpEndpoint);
    VerifyOrReturn(request != nullptr, HandleConnectFailed(CHIP_ERROR_NO_MEMORY));

    GSource * source = g_idle_source_new();
    g_source_set_priority(source, G_PRIORITY_HIGH_IDLE);
    g_source_set_callback(
        source,
        [](gpointer userData) -> gboolean {
            auto * req     = static_cast<ConnectRequest *>(userData);
            CHIP_ERROR err = ConnectDevice(req->mAddress, req->mEndpoint);
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(Ble, "Device connection failed: %" CHIP_ERROR_FORMAT, err.Format());
            }
            return G_SOURCE_REMOVE;
        },
        request, [](gpointer userData) { Platform::Delete(static_cast<ConnectRequest *>(userData)); });
    PlatformMgrImpl().GLibMatterContextAttachSource(source);
    g_source_unref(source);
}

void BLEManagerImpl::HandleConnectTimer(chip::System::Layer *, void * appState)
{
    ChipLogProgress(Ble, "BLE connect timeout");
    auto * manager = static_cast<BLEManagerImpl *>(appState);
    CancelConnect(manager->mpEndpoint);
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
