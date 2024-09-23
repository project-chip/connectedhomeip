/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
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
#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Concrete implementation of the NetworkProvisioningServer singleton object for the  PSoC 6 platform.
 */
class BLEManagerImpl final : public BLEManager,
                             private Ble::BleLayer,
                             private Ble::BlePlatformDelegate,
                             private Ble::BleApplicationDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    void _Shutdown() {}
    bool _IsAdvertisingEnabled(void);
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsFastAdvertisingEnabled(void);
    bool _IsAdvertising(void);
    CHIP_ERROR _SetAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    ::chip::Ble::BleLayer * _GetBleLayer(void);

    // ===== Members that implement virtual methods on BlePlatformDelegate.

    CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                       const Ble::ChipBleUUID * charId) override;
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                         const Ble::ChipBleUUID * charId) override;
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                              System::PacketBufferHandle data) override;
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                System::PacketBufferHandle data) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

public:
    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.
    enum class Flags : uint16_t
    {
        kFlag_StackInitialized       = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kFlag_AdvertisingEnabled     = 0x0002, /**< The application has enabled CHIPoBLE advertising. */
        kFlag_FastAdvertisingEnabled = 0x0004, /**< The application has enabled fast advertising. */
        kFlag_Advertising            = 0x0008, /**< The system is currently CHIPoBLE advertising. */
        kFlag_AdvertisingRestarted =
            0x0010, /**< The advertising state/configuration has changed, but the SoftDevice has yet to be updated. */
    };

    enum
    {
        kMaxConnections = BLE_LAYER_NUM_BLE_ENDPOINTS,
    };

    struct CHIPoBLEConState
    {
        // System::PacketBuffer * PendingIndBuf;
        uint16_t ConId;
        uint16_t Mtu;
        bool connected;
        bool Subscribed;
    };

    CHIPoBLEConState mCons[3];
    uint16_t mNumCons;
    CHIPoBLEServiceMode mServiceMode;
    BitFlags<Flags> mFlags;
    char mDeviceName[32 + 1];

    void DriveBLEState(void);
    void SetAdvertisingData(uint8_t * data, uint8_t * len);
    void SetScanRspData(uint8_t * data, uint8_t * len);
    void SetAdvStartFlag(void);
    void SetAdvEndFlag(void);
    void SetStackInit(void);
    CHIPoBLEConState * AllocConnectionState(uint16_t conId);
    CHIPoBLEConState * GetConnectionState(uint16_t conId);
    bool ReleaseConnectionState(uint16_t conId);
    void SetConnectionMtu(uint16_t conId, uint16_t mtu);
    static void DriveBLEState(intptr_t arg);
    void HandleTXCharCCCDRead(uint8_t connection_id, uint16_t * length, uint8_t * value);
    bool HandleRXCharWrite(uint8_t connection_id, uint16_t length, uint8_t * value);
    bool HandleTXCharCCCDWrite(uint8_t connection_id, uint16_t length, uint8_t * value);
    void SendIndicationConfirm(uint16_t conId);
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    void HandleC3CharRead(uint8_t connection_id, uint16_t * p_len, uint8_t * p_value);
#endif
    static void HandleFastAdvertisementTimer(System::Layer * systemLayer, void * context);
    void HandleFastAdvertisementTimer();
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
 * that are specific to the PSoC 6 platform.
 */
inline BLEManagerImpl & BLEMgrImpl(void)
{
    return BLEManagerImpl::sInstance;
}

inline Ble::BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
