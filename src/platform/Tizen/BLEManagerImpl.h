/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
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
 *          for the Tizen platforms.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include <bluetooth.h>
#include <glib.h>

#include <ble/Ble.h>
#include <ble/CHIPBleServiceData.h>
#include <lib/core/CHIPError.h>
#include <lib/support/BitFlags.h>
#include <lib/support/SetupDiscriminator.h>
#include <platform/CHIPDeviceEvent.h>
#include <system/SystemLayer.h>
#include <system/SystemPacketBuffer.h>

#include "ChipDeviceScanner.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * enum Class for BLE Scanning state. CHIP supports Scanning by Discriminator or Address
 */
enum class BleScanState : uint8_t
{
    kNotScanning,
    kScanForDiscriminator,
    kScanForAddress,
    kConnecting,
};

/**
 * Structure for BLE Scanning Configuration
 */
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
 * Concrete implementation of the BLEManagerImpl singleton object for the Tizen platforms.
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

    //  ===== Members that implement virtual methods on ChipDeviceScannerDelegate
    void OnChipDeviceScanned(void * device, const Ble::ChipBLEDeviceIdentificationInfo & info) override;
    void OnScanComplete() override;
    void OnScanError(CHIP_ERROR err) override;

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr();
    friend BLEManagerImpl & BLEMgrImpl();

    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.
    enum class Flags : uint16_t
    {
        kAsyncInitCompleted       = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kTizenBLELayerInitialized = 0x0002, /**< The Tizen Platform BLE layer has been initialized. */
        kAppRegistered            = 0x0004, /**< The CHIPoBLE application has been registered with the Bluez layer. */
        kAdvertisingConfigured    = 0x0008, /**< CHIPoBLE advertising has been configured in the Bluez layer. */
        kAdvertising              = 0x0010, /**< The system is currently CHIPoBLE advertising. */
        kControlOpInProgress      = 0x0020, /**< An async control operation has been issued to the ESP BLE layer. */
        kAdvertisingEnabled       = 0x0040, /**< The application has enabled CHIPoBLE advertising. */
        kFastAdvertisingEnabled   = 0x0080, /**< The application has enabled fast advertising. */
        kUseCustomDeviceName      = 0x0100, /**< The application has configured a custom BLE device name. */
        kAdvertisingRefreshNeeded = 0x0200, /**< The advertising configuration/state in BLE layer needs to be updated. */
    };

    static CHIP_ERROR _BleInitialize(void * userData);
    void DriveBLEState();
    static void DriveBLEState(intptr_t arg);

    void InitiateScan(BleScanState scanType);
    static void InitiateScan(intptr_t arg);

    static void AdvertisingStateChangedCb(int result, bt_advertiser_h advertiser, bt_adapter_le_advertising_state_e advState,
                                          void * userData);
    static void NotificationStateChangedCb(bool notify, bt_gatt_server_h server, bt_gatt_h gattHandle, void * userData);
    static void ReadValueRequestedCb(const char * remoteAddress, int requestId, bt_gatt_server_h server, bt_gatt_h gattHandle,
                                     int offset, void * userData);
    static void WriteValueRequestedCb(const char * remoteAddress, int requestId, bt_gatt_server_h server, bt_gatt_h gattHandle,
                                      bool responseNeeded, int offset, const char * value, int len, void * userData);
    static void IndicationConfirmationCb(int result, const char * remoteAddress, bt_gatt_server_h server, bt_gatt_h characteristic,
                                         bool completed, void * userData);
    static void IndicationConfirmationCb(bt_gatt_h characteristic, bt_gatt_server_notification_sent_cb callback,
                                         const char * device_address, void * userData);
    static void GattConnectionStateChangedCb(int result, bool connected, const char * remoteAddress, void * userData);
    static void WriteCompletedCb(int result, bt_gatt_h gattHandle, void * userData);
    static void CharacteristicNotificationCb(bt_gatt_h characteristic, char * value, int len, void * userData);

    // ==== Connection.
    void InitConnectionData();
    void AddConnectionData(const char * remoteAddr);
    void RemoveConnectionData(const char * remoteAddr);

    void HandleC1CharWriteEvent(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len);
    void HandleRXCharChanged(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len);
    void HandleConnectionEvent(bool connected, const char * remoteAddress);
    static void HandleConnectionTimeout(System::Layer * layer, void * data);
    static bool IsDeviceChipPeripheral(BLE_CONNECTION_OBJECT conId);

    // ==== BLE Adv & GATT Server.
    void NotifyBLEPeripheralGATTServerRegisterComplete(bool aIsSuccess, void * apAppstate);
    void NotifyBLEPeripheralAdvConfiguredComplete(bool aIsSuccess, void * apAppstate);
    void NotifyBLEPeripheralAdvStartComplete(bool aIsSuccess, void * apAppstate);
    void NotifyBLEPeripheralAdvStopComplete(bool aIsSuccess, void * apAppstate);
    void NotifyBLESubscribed(bool indicationsEnabled, BLE_CONNECTION_OBJECT conId);
    void NotifyBLEIndicationConfirmation(BLE_CONNECTION_OBJECT conId);
    void NotifyBLEWriteReceived(System::PacketBufferHandle & buf, BLE_CONNECTION_OBJECT conId);

    // ==== Connection.
    static CHIP_ERROR ConnectChipThing(const char * userData);
    void NotifyBLEConnectionEstablished(BLE_CONNECTION_OBJECT conId, CHIP_ERROR error);
    void NotifyBLEDisconnection(BLE_CONNECTION_OBJECT conId, CHIP_ERROR error);
    void NotifyHandleNewConnection(BLE_CONNECTION_OBJECT conId);
    void NotifyHandleConnectFailed(CHIP_ERROR error);
    void NotifyHandleWriteComplete(BLE_CONNECTION_OBJECT conId);
    void NotifySubscribeOpComplete(BLE_CONNECTION_OBJECT conId, bool isSubscribed);
    void NotifyBLENotificationReceived(System::PacketBufferHandle & buf, BLE_CONNECTION_OBJECT conId);

    int RegisterGATTServer();
    int StartBLEAdvertising();
    int StopBLEAdvertising();
    void CleanScanConfig();

    CHIPoBLEServiceMode mServiceMode;
    BitFlags<Flags> mFlags;
    bool mIsCentral          = false;
    void * mGattCharC1Handle = nullptr;
    void * mGattCharC2Handle = nullptr;
    uint32_t mAdapterId;
    bt_advertiser_h mAdvertiser = nullptr;
    bool mAdvReqInProgress      = false;
    /* Connection Hash Table Map */
    GHashTable * mConnectionMap = nullptr;

    BLEScanConfig mBLEScanConfig;
    std::unique_ptr<ChipDeviceScanner> mDeviceScanner;
    bt_gatt_client_h mGattClient = nullptr;
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
 * that are specific to the Tizen platforms.
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
