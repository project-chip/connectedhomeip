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
 *          for the Mbed platform.
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

namespace chip {
namespace DeviceLayer {
namespace Internal {

class GapEventHandler;
class CHIPService;

using namespace chip::Ble;

/**
 * Concrete implementation of the BLEManager singleton object for the Mbed platform.
 */
class BLEManagerImpl final : public BLEManager, private BleLayer, private BlePlatformDelegate, private BleApplicationDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    void _Shutdown() {}
    bool _IsAdvertisingEnabled(void);
    CHIP_ERROR _SetAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsFastAdvertisingEnabled(void);
    CHIP_ERROR _SetFastAdvertisingEnabled(bool val);
    bool _IsAdvertising(void);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    BleLayer * _GetBleLayer(void);

    // ===== Members that implement virtual methods on BlePlatformDelegate.

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId);
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId);
    bool CloseConnection(BLE_CONNECTION_OBJECT conId);
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const;
    bool SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                        PacketBufferHandle pBuf);
    bool SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                          PacketBufferHandle pBuf);
    bool SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                         PacketBufferHandle pBuf);
    bool SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const ChipBleUUID * svcId,
                          const ChipBleUUID * charId);

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId);

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);
    friend class GapEventHandler;
    friend class CHIPService;

    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    enum Flags : uint16_t
    {
        kFlag_AsyncInitCompleted       = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kFlag_AdvertisingEnabled       = 0x0002, /**< The application has enabled CHIPoBLE advertising. */
        kFlag_FastAdvertisingEnabled   = 0x0004, /**< The application has enabled fast advertising. */
        kFlag_Advertising              = 0x0008, /**< The system is currently CHIPoBLE advertising. */
        kFlag_AdvertisingRefreshNeeded = 0x0010, /**< The advertising configuration/state in BLE layer needs to be updated. */
        kFlag_DeviceNameSet            = 0x0020,
        kFlag_UseCustomDeviceName      = 0x0040, /**< The application has configured a custom BLE device name. */
    };

    enum : size_t
    {
        kMaxConnections      = BLE_LAYER_NUM_BLE_ENDPOINTS,
        kMaxDeviceNameLength = 16,
        kAdvertisingDataSize = 0x1F, // mbed-os ble::LEGACY_ADVERTISING_MAX_SIZE
    };

    CHIPoBLEServiceMode mServiceMode;
    BitFlags<Flags> mFlags;
    uint16_t mGAPConns;
    char mDeviceName[kMaxDeviceNameLength + 1];
    uint8_t mAdvertisingDataBuffer[kAdvertisingDataSize];
    bool mInitialized = false;

    static void DoBLEProcessing(intptr_t arg);
    void HandleInitComplete(bool no_error);
    static void DriveBLEState(intptr_t arg);
    void DriveBLEState(void);
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);
};

/**
 * Returns a reference to the public interface of the BLEManager singleton object.
 *
 * Internal components should use this to access features of the BLEManager object
 * that are common to all platforms.
 */
inline BLEManager & BLEMgr(void)
{
    return BLEManagerImpl::sInstance;
}

/**
 * Returns the platform-specific implementation of the BLEManager singleton object.
 *
 * Internal components can use this to gain access to features of the BLEManager
 * that are specific to the Mbed platform.
 */
inline BLEManagerImpl & BLEMgrImpl(void)
{
    return BLEManagerImpl::sInstance;
}

inline BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
}

inline bool BLEManagerImpl::_IsAdvertisingEnabled(void)
{
    return mFlags.Has(Flags::kFlag_AdvertisingEnabled);
}

inline bool BLEManagerImpl::_IsFastAdvertisingEnabled(void)
{
    return mFlags.Has(Flags::kFlag_FastAdvertisingEnabled);
}

inline bool BLEManagerImpl::_IsAdvertising(void)
{
    return mFlags.Has(Flags::kFlag_Advertising);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
