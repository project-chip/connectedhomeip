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
 *          for the Linux platforms.
 */

#pragma once

#include <ble/BleLayer.h>
#include <platform/internal/BLEManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "bluez/ChipDeviceScanner.h"
#include "bluez/Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

void HandleIncomingBleConnection(Ble::BLEEndPoint * bleEP);

struct BLEAdvConfig
{
    char * mpBleName;
    uint32_t mAdapterId;
    uint8_t mMajor;
    uint8_t mMinor;
    uint16_t mVendorId;
    uint16_t mProductId;
    uint64_t mDeviceId;
    uint8_t mPairingStatus;
    ChipAdvType mType;
    uint16_t mDuration;
    const char * mpAdvertisingUUID;
};

enum class BleScanState : uint8_t
{
    kNotScanning,
    kScanForDiscriminator,
    kScanForAddress,
    kConnecting,
};

struct BLEScanConfig
{
    // If an active scan for connection is being performed
    BleScanState mBleScanState = BleScanState::kNotScanning;

    // If scanning by discriminator, what are we scanning for
    SetupDiscriminator mDiscriminator;

    // If scanning by address, what address are we searching for
    std::string mAddress;

    // Optional argument to be passed to callback functions provided by the BLE scan/connect requestor
    void * mAppState = nullptr;
};

/**
 * Concrete implementation of the BLEManagerImpl singleton object for the Linux platforms.
 */
class BLEManagerImpl final : public BLEManager,
                             private Ble::BleLayer,
                             private Ble::BlePlatformDelegate,
                             private Ble::BleApplicationDelegate,
                             private Ble::BleConnectionDelegate,
                             private ChipDeviceScannerDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

public:
    CHIP_ERROR ConfigureBle(uint32_t aAdapterId, bool aIsCentral);
    void OnScanError(CHIP_ERROR error) override;

    // Driven by BlueZ IO
    static void HandleNewConnection(BLE_CONNECTION_OBJECT conId);
    static void HandleConnectFailed(CHIP_ERROR error);
    static void HandleWriteComplete(BLE_CONNECTION_OBJECT conId);
    static void HandleSubscribeOpComplete(BLE_CONNECTION_OBJECT conId, bool subscribed);
    static void HandleTXCharChanged(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len);
    static void HandleRXCharWrite(BLE_CONNECTION_OBJECT user_data, const uint8_t * value, size_t len);
    static void CHIPoBluez_ConnectionClosed(BLE_CONNECTION_OBJECT user_data);
    static void HandleTXCharCCCDWrite(BLE_CONNECTION_OBJECT user_data);
    static void HandleTXComplete(BLE_CONNECTION_OBJECT user_data);

    static void NotifyBLEPeripheralRegisterAppComplete(bool aIsSuccess, void * apAppstate);
    static void NotifyBLEPeripheralAdvConfiguredComplete(bool aIsSuccess, void * apAppstate);
    static void NotifyBLEPeripheralAdvStartComplete(bool aIsSuccess, void * apAppstate);
    static void NotifyBLEPeripheralAdvStopComplete(bool aIsSuccess, void * apAppstate);

private:
    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init();
    void _Shutdown();
    bool _IsAdvertisingEnabled();
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsAdvertising();
    CHIP_ERROR _SetAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections();

    void _OnPlatformEvent(const ChipDeviceEvent * event);
    void HandlePlatformSpecificBLEEvent(const ChipDeviceEvent * event);
    BleLayer * _GetBleLayer();

    // ===== Members that implement virtual methods on BlePlatformDelegate.

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                 const Ble::ChipBleUUID * charId) override;
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                   const Ble::ChipBleUUID * charId) override;
    bool CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;
    bool SendIndication(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                        System::PacketBufferHandle pBuf) override;
    bool SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                          System::PacketBufferHandle pBuf) override;
    bool SendReadRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                         System::PacketBufferHandle pBuf) override;
    bool SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const Ble::ChipBleUUID * svcId,
                          const Ble::ChipBleUUID * charId) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

    // ===== Members that implement virtual methods on BleConnectionDelegate.

    void NewConnection(BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator) override;
    void NewConnection(BleLayer * bleLayer, void * appState, BLE_CONNECTION_OBJECT connObj) override{};
    CHIP_ERROR CancelConnection() override;

    // ===== Members that implement virtual methods on ChipDeviceScannerDelegate
    void OnDeviceScanned(BluezDevice1 * device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) override;
    void OnScanComplete() override;

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr();
    friend BLEManagerImpl & BLEMgrImpl();

    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.
    enum class Flags : uint16_t
    {
        kAsyncInitCompleted       = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kBluezBLELayerInitialized = 0x0002, /**< The Bluez layer has been initialized. */
        kAppRegistered            = 0x0004, /**< The CHIPoBLE application has been registered with the Bluez layer. */
        kAdvertisingConfigured    = 0x0008, /**< CHIPoBLE advertising has been configured in the Bluez layer. */
        kAdvertising              = 0x0010, /**< The system is currently CHIPoBLE advertising. */
        kControlOpInProgress      = 0x0020, /**< An async control operation has been issued to the ESP BLE layer. */
        kAdvertisingEnabled       = 0x0040, /**< The application has enabled CHIPoBLE advertising. */
        kFastAdvertisingEnabled   = 0x0080, /**< The application has enabled fast advertising. */
        kUseCustomDeviceName      = 0x0100, /**< The application has configured a custom BLE device name. */
        kAdvertisingRefreshNeeded = 0x0200, /**< The advertising configuration/state in BLE layer needs to be updated. */
    };

    enum
    {
        kMaxConnections             = 1,  // TODO: right max connection
        kMaxDeviceNameLength        = 20, // TODO: right-size this
        kMaxAdvertismentDataSetSize = 31  // TODO: verify this
    };

    CHIP_ERROR StartBLEAdvertising();
    CHIP_ERROR StopBLEAdvertising();

    void DriveBLEState();
    static void DriveBLEState(intptr_t arg);

    void InitiateScan(BleScanState scanType);
    static void InitiateScan(intptr_t arg);
    void CleanScanConfig();

    CHIPoBLEServiceMode mServiceMode;
    BLEAdvConfig mBLEAdvConfig;
    BLEScanConfig mBLEScanConfig;
    BitFlags<Flags> mFlags;
    char mDeviceName[kMaxDeviceNameLength + 1];
    bool mIsCentral            = false;
    BluezEndpoint * mpEndpoint = nullptr;
    std::unique_ptr<ChipDeviceScanner> mDeviceScanner;
};

/**
 * Returns a reference to the public interface of the BLEManager singleton object.
 *
 * Internal components should use this to access features of the BLEManager object
 * that are common to all platforms.
 */
inline BLEManager & BLEMgr()
{
    return BLEManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the BLEManager singleton object.
 *
 * Internal components can use this to gain access to features of the BLEManager
 * that are specific to the Linux platforms.
 */
inline BLEManagerImpl & BLEMgrImpl()
{
    return BLEManagerImpl::sInstance;
}

inline Ble::BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
}

inline bool BLEManagerImpl::_IsAdvertisingEnabled()
{
    return mFlags.Has(Flags::kAdvertisingEnabled);
}

inline bool BLEManagerImpl::_IsAdvertising()
{
    return mFlags.Has(Flags::kAdvertising);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
