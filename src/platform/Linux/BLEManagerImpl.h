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

#include <cstdint>
#include <string>

#include <ble/BleLayer.h>
#include <platform/internal/BLEManager.h>

#include "bluez/BluezAdvertisement.h"
#include "bluez/BluezEndpoint.h"
#include "bluez/BluezObjectManager.h"
#include "bluez/ChipDeviceScanner.h"
#include "bluez/Types.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

void HandleIncomingBleConnection(Ble::BLEEndPoint * bleEP);

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

    static void NotifyBLEPeripheralRegisterAppComplete(CHIP_ERROR error);
    static void NotifyBLEPeripheralAdvStartComplete(CHIP_ERROR error);
    static void NotifyBLEPeripheralAdvStopComplete(CHIP_ERROR error);
    static void NotifyBLEPeripheralAdvReleased();

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
    void CheckNonConcurrentBleClosing() override;

    // ===== Members that implement virtual methods on BleConnectionDelegate.

    void NewConnection(BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator) override;
    void NewConnection(BleLayer * bleLayer, void * appState, BLE_CONNECTION_OBJECT connObj) override{};
    CHIP_ERROR CancelConnection() override;

    // ===== Members that implement virtual methods on ChipDeviceScannerDelegate

    void OnDeviceScanned(BluezDevice1 & device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info) override;
    void OnScanComplete() override;

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr();
    friend BLEManagerImpl & BLEMgrImpl();

    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    enum class Flags : uint16_t
    {
        kAsyncInitCompleted       = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kBluezManagerInitialized  = 0x0002, /**< The BlueZ object manager has been initialized. */
        kBluezAdapterAvailable    = 0x0004, /**< Selected BlueZ adapter is available for use. */
        kBluezBLELayerInitialized = 0x0008, /**< The BlueZ BLE layer has been initialized. */
        kAppRegistered            = 0x0010, /**< The CHIPoBLE application has been registered with the Bluez layer. */
        kAdvertisingConfigured    = 0x0020, /**< CHIPoBLE advertising has been configured in the Bluez layer. */
        kAdvertising              = 0x0040, /**< The system is currently CHIPoBLE advertising. */
        kControlOpInProgress      = 0x0080, /**< An async control operation has been issued to the ESP BLE layer. */
        kAdvertisingEnabled       = 0x0100, /**< The application has enabled CHIPoBLE advertising. */
        kFastAdvertisingEnabled   = 0x0200, /**< The application has enabled fast advertising. */
        kUseCustomDeviceName      = 0x0400, /**< The application has configured a custom BLE device name. */
        kAdvertisingRefreshNeeded = 0x0800, /**< The advertising configuration/state in BLE layer needs to be updated. */
        kExtAdvertisingEnabled    = 0x1000, /**< The application has enabled CHIPoBLE extended advertising. */
    };

    enum
    {
        kMaxConnections              = 1,  // TODO: right max connection
        kMaxDeviceNameLength         = 20, // TODO: right-size this
        kMaxAdvertisementDataSetSize = 31  // TODO: verify this
    };

    void DriveBLEState();
    BluezAdvertisement::AdvertisingIntervals GetAdvertisingIntervals() const;
    static void HandleAdvertisingTimer(chip::System::Layer *, void * appState);
    void InitiateScan(BleScanState scanType);
    void CleanScanConfig();

    CHIPoBLEServiceMode mServiceMode;
    BitFlags<Flags> mFlags;

    BluezObjectManager mBluezObjectManager;
    GAutoPtr<BluezAdapter1> mAdapter;
    uint32_t mAdapterId = 0;

    char mDeviceName[kMaxDeviceNameLength + 1];
    bool mIsCentral = false;
    BluezEndpoint mEndpoint{ mBluezObjectManager };

    BluezAdvertisement mBLEAdvertisement{ mEndpoint };
    const char * mpBLEAdvUUID = nullptr;

    ChipDeviceScanner mDeviceScanner{ mBluezObjectManager };
    BLEScanConfig mBLEScanConfig;
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
