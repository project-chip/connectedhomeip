/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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

#ifndef BLE_MANAGER_IMPL_H
#define BLE_MANAGER_IMPL_H

#include <platform/internal/BLEManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

void HandleIncomingBleConnection(BLEEndPoint * bleEP);

enum ChipAdvType
{
    BLUEZ_ADV_TYPE_CONNECTABLE = 0x01,
    BLUEZ_ADV_TYPE_SCANNABLE   = 0x02,
    BLUEZ_ADV_TYPE_DIRECTED    = 0x04,

    BLUEZ_ADV_TYPE_UNDIRECTED_NONCONNECTABLE_NONSCANNABLE = 0,
    BLUEZ_ADV_TYPE_UNDIRECTED_CONNECTABLE_NONSCANNABLE    = BLUEZ_ADV_TYPE_CONNECTABLE,
    BLUEZ_ADV_TYPE_UNDIRECTED_NONCONNECTABLE_SCANNABLE    = BLUEZ_ADV_TYPE_SCANNABLE,
    BLUEZ_ADV_TYPE_UNDIRECTED_CONNECTABLE_SCANNABLE       = BLUEZ_ADV_TYPE_CONNECTABLE | BLUEZ_ADV_TYPE_SCANNABLE,

    BLUEZ_ADV_TYPE_DIRECTED_NONCONNECTABLE_NONSCANNABLE = BLUEZ_ADV_TYPE_DIRECTED,
    BLUEZ_ADV_TYPE_DIRECTED_CONNECTABLE_NONSCANNABLE    = BLUEZ_ADV_TYPE_DIRECTED | BLUEZ_ADV_TYPE_CONNECTABLE,
    BLUEZ_ADV_TYPE_DIRECTED_NONCONNECTABLE_SCANNABLE    = BLUEZ_ADV_TYPE_DIRECTED | BLUEZ_ADV_TYPE_SCANNABLE,
    BLUEZ_ADV_TYPE_DIRECTED_CONNECTABLE_SCANNABLE = BLUEZ_ADV_TYPE_DIRECTED | BLUEZ_ADV_TYPE_CONNECTABLE | BLUEZ_ADV_TYPE_SCANNABLE,
};

struct BLEAdvConfig
{
    char * mpBleName;
    uint32_t mNodeId;
    uint8_t mMajor;
    uint8_t mMinor;
    uint16_t mVendorId;
    uint16_t mProductId;
    uint64_t mDeviceId;
    uint8_t mPairingStatus;
    ChipAdvType mType;
    uint16_t mDuration;
    char * mpAdvertisingUUID;
};

/**
 * Concrete implementation of the BLEManagerImpl singleton object for the Linux platforms.
 */
class BLEManagerImpl final : public BLEManager, private BleLayer, private BlePlatformDelegate, private BleApplicationDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

public:
    CHIP_ERROR ConfigureBle(uint32_t aNodeId, bool aIsCentral);

    // Driven by BlueZ IO
    static void CHIPoBluez_NewConnection(void * user_data);
    static void HandleRXCharWrite(void * user_data, const uint8_t * value, size_t len);
    static void CHIPoBluez_ConnectionClosed(void * user_data);
    static void HandleTXCharCCCDWrite(void * user_data);
    static void HandleTXComplete(void * user_data);
    static bool WoBLEz_TimerCb(void * user_data);

    static void NotifyBLEPeripheralRegisterAppComplete(bool aIsSuccess, void * apAppstate);
    static void NotifyBLEPeripheralAdvConfiguredComplete(bool aIsSuccess, void * apAppstate);
    static void NotifyBLEPeripheralAdvStartComplete(bool aIsSuccess, void * apAppstate);
    static void NotifyBLEPeripheralAdvStopComplete(bool aIsSuccess, void * apAppstate);

private:
    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    CHIPoBLEServiceMode _GetCHIPoBLEServiceMode(void);
    CHIP_ERROR _SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val);
    bool _IsAdvertisingEnabled(void);
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsFastAdvertisingEnabled(void);
    CHIP_ERROR _SetFastAdvertisingEnabled(bool val);
    bool _IsAdvertising(void);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections(void);

    void _OnPlatformEvent(const ChipDeviceEvent * event);
    void HandlePlatformSpecificBLEEvent(const ChipDeviceEvent * event);
    BleLayer * _GetBleLayer(void) const;

    // ===== Members that implement virtual methods on BlePlatformDelegate.

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId) override;
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId) override;
    bool CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;
    bool SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                        PacketBuffer * pBuf) override;
    bool SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                          PacketBuffer * pBuf) override;
    bool SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                         PacketBuffer * pBuf) override;
    bool SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const ChipBleUUID * svcId,
                          const ChipBleUUID * charId) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.
    enum
    {
        kFlag_AsyncInitCompleted       = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kFlag_BluezBLELayerInitialized = 0x0002, /**< The Bluez layer has been initialized. */
        kFlag_AppRegistered            = 0x0004, /**< The CHIPoBLE application has been registered with the Bluez layer. */
        kFlag_AdvertisingConfigured    = 0x0008, /**< CHIPoBLE advertising has been configured in the Bluez layer. */
        kFlag_Advertising              = 0x0010, /**< The system is currently CHIPoBLE advertising. */
        kFlag_ControlOpInProgress      = 0x0020, /**< An async control operation has been issued to the ESP BLE layer. */
        kFlag_AdvertisingEnabled       = 0x0040, /**< The application has enabled CHIPoBLE advertising. */
        kFlag_FastAdvertisingEnabled   = 0x0080, /**< The application has enabled fast advertising. */
        kFlag_UseCustomDeviceName      = 0x0100, /**< The application has configured a custom BLE device name. */
        kFlag_AdvertisingRefreshNeeded = 0x0200, /**< The advertising configuration/state in ESP BLE layer needs to be updated. */
    };

    enum
    {
        kMaxConnections             = 1,  // TODO: right max connection
        kMaxDeviceNameLength        = 20, // TODO: right-size this
        kMaxAdvertismentDataSetSize = 31  // TODO: verify this
    };

    CHIP_ERROR StartBLEAdvertising(void);
    CHIP_ERROR StopBLEAdvertising(void);

    void DriveBLEState();
    static void DriveBLEState(intptr_t arg);

    CHIPoBLEServiceMode mServiceMode;
    BLEAdvConfig mBLEAdvConfig;
    uint16_t mFlags;
    char mDeviceName[kMaxDeviceNameLength + 1];
    bool mIsCentral;
    char * mpBleAddr;
    void * mpAppState;
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
 * that are specific to the Linux platforms.
 */
inline BLEManagerImpl & BLEMgrImpl(void)
{
    return BLEManagerImpl::sInstance;
}

inline BleLayer * BLEManagerImpl::_GetBleLayer() const
{
    return (BleLayer *) (this);
}

inline BLEManager::CHIPoBLEServiceMode BLEManagerImpl::_GetCHIPoBLEServiceMode(void)
{
    return mServiceMode;
}

inline bool BLEManagerImpl::_IsAdvertisingEnabled(void)
{
    return GetFlag(mFlags, kFlag_AdvertisingEnabled);
}

inline bool BLEManagerImpl::_IsFastAdvertisingEnabled(void)
{
    return GetFlag(mFlags, kFlag_FastAdvertisingEnabled);
}

inline bool BLEManagerImpl::_IsAdvertising(void)
{
    return GetFlag(mFlags, kFlag_Advertising);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#endif // BLE_MANAGER_IMPL_H
