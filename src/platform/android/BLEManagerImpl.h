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

#include <jni.h>

#include <ble/BleLayer.h>
#include <platform/internal/BLEManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Concrete implementation of the BLEManagerImpl singleton object for the Linux platforms.
 */
class BLEManagerImpl final : public BLEManager,
                             public Ble::BleLayer,
                             private Ble::BlePlatformDelegate,
                             private Ble::BleApplicationDelegate,
                             private Ble::BleConnectionDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

public:
    CHIP_ERROR ConfigureBle(uint32_t aNodeId, bool aIsCentral);

    void InitializeWithObject(jobject managerObject);

private:
    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init();
    void _Shutdown() {}
    bool _IsAdvertisingEnabled();
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsAdvertising();
    CHIP_ERROR _SetAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections();
    void _OnPlatformEvent(const ChipDeviceEvent * event);
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
    CHIP_ERROR CancelConnection() override;

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
        kServiceModeEnabled       = 0x0400, /**< CHIPoBLEServiceMode is enabled or not*/
    };

    // ===== helpers
    CHIP_ERROR HasFlag(Flags flag, bool & has);
    CHIP_ERROR SetFlag(Flags flag, bool isSet);

    jobject mBLEManagerObject = nullptr;

    jmethodID mInitMethod    = nullptr;
    jmethodID mSetFlagMethod = nullptr;
    jmethodID mHasFlagMethod = nullptr;

    jmethodID mOnSubscribeCharacteristicMethod    = nullptr;
    jmethodID mOnUnsubscribeCharacteristicMethod  = nullptr;
    jmethodID mOnCloseConnectionMethod            = nullptr;
    jmethodID mOnGetMTUMethod                     = nullptr;
    jmethodID mOnSendWriteRequestMethod           = nullptr;
    jmethodID mOnNotifyChipConnectionClosedMethod = nullptr;
    jmethodID mOnNewConnectionMethod              = nullptr;
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

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
