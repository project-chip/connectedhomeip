/*
 *
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

/**
 *    @file
 *          Provides an implementation of the BLEManager singleton object
 *          for the nRF5 platforms.
 */

#ifndef BLE_MANAGER_IMPL_H
#define BLE_MANAGER_IMPL_H

#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE

#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

/**
 * Concrete implementation of the NetworkProvisioningServer singleton object for the nRF5 platforms.
 */
class BLEManagerImpl final
    : public BLEManager,
      private ::nl::Ble::BleLayer,
      private BlePlatformDelegate,
      private BleApplicationDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

public:

    // ===== Platform-specific members available for use by the application.

    uint8_t GetAdvertisingHandle(void);
    void SetAdvertisingHandle(uint8_t handle);

private:

    // ===== Members that implement the BLEManager internal interface.

    WEAVE_ERROR _Init(void);
    WoBLEServiceMode _GetWoBLEServiceMode(void);
    WEAVE_ERROR _SetWoBLEServiceMode(WoBLEServiceMode val);
    bool _IsAdvertisingEnabled(void);
    WEAVE_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsFastAdvertisingEnabled(void);
    WEAVE_ERROR _SetFastAdvertisingEnabled(bool val);
    bool _IsAdvertising(void);
    WEAVE_ERROR _GetDeviceName(char * buf, size_t bufSize);
    WEAVE_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections(void);
    void _OnPlatformEvent(const WeaveDeviceEvent * event);
    ::nl::Ble::BleLayer * _GetBleLayer(void) const;

    // ===== Members that implement virtual methods on BlePlatformDelegate.

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId) override;
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId) override;
    bool CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;
    bool SendIndication(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId, PacketBuffer * pBuf) override;
    bool SendWriteRequest(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId, PacketBuffer * pBuf) override;
    bool SendReadRequest(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId, PacketBuffer * pBuf) override;
    bool SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const WeaveBleUUID * svcId, const WeaveBleUUID * charId) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyWeaveConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    enum
    {
        kFlag_AsyncInitCompleted                = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kFlag_AdvertisingEnabled                = 0x0002, /**< The application has enabled WoBLE advertising. */
        kFlag_FastAdvertisingEnabled            = 0x0004, /**< The application has enabled fast advertising. */
        kFlag_Advertising                       = 0x0008, /**< The system is currently WoBLE advertising. */
        kFlag_AdvertisingRefreshNeeded          = 0x0010, /**< The advertising state/configuration has changed, but the SoftDevice has yet to be updated. */
    };

    enum
    {
        kMaxConnections = MIN(BLE_LAYER_NUM_BLE_ENDPOINTS, NRF_SDH_BLE_PERIPHERAL_LINK_COUNT),
        kMaxDeviceNameLength = 20, // TODO: right-size this
        kMaxAdvertismentDataSetSize = 31 // TODO: verify this
    };

    ble_gatts_char_handles_t mWoBLECharHandle_RX;
    ble_gatts_char_handles_t mWoBLECharHandle_TX;
    WoBLEServiceMode mServiceMode;
    uint16_t mFlags;
    uint16_t mNumGAPCons;
    uint16_t mSubscribedConIds[kMaxConnections];
    uint8_t mAdvHandle;
    uint8_t mAdvDataBuf[kMaxAdvertismentDataSetSize];
    uint8_t mScanRespDataBuf[kMaxAdvertismentDataSetSize];

    void DriveBLEState(void);
    WEAVE_ERROR ConfigureAdvertising(void);
    WEAVE_ERROR EncodeAdvertisingData(ble_gap_adv_data_t & gapAdvData);
    WEAVE_ERROR StartAdvertising(void);
    WEAVE_ERROR StopAdvertising(void);
    void HandleSoftDeviceBLEEvent(const WeaveDeviceEvent * event);
    WEAVE_ERROR HandleGAPConnect(const WeaveDeviceEvent * event);
    WEAVE_ERROR HandleGAPDisconnect(const WeaveDeviceEvent * event);
    WEAVE_ERROR HandleRXCharWrite(const WeaveDeviceEvent * event);
    WEAVE_ERROR HandleTXCharCCCDWrite(const WeaveDeviceEvent * event);
    WEAVE_ERROR HandleTXComplete(const WeaveDeviceEvent * event);
    WEAVE_ERROR SetSubscribed(uint16_t conId);
    bool UnsetSubscribed(uint16_t conId);
    bool IsSubscribed(uint16_t conId);

    static void DriveBLEState(intptr_t arg);
    static void SoftDeviceBLEEventCallback(const ble_evt_t * bleEvent, void * context);
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
 * that are specific to the NRF5* platforms.
 */
inline BLEManagerImpl & BLEMgrImpl(void)
{
    return BLEManagerImpl::sInstance;
}

inline uint8_t BLEManagerImpl::GetAdvertisingHandle(void)
{
    return mAdvHandle;
}

inline void BLEManagerImpl::SetAdvertisingHandle(uint8_t handle)
{
    mAdvHandle = handle;
}

inline ::nl::Ble::BleLayer * BLEManagerImpl::_GetBleLayer() const
{
    return (BleLayer *)(this);
}

inline BLEManager::WoBLEServiceMode BLEManagerImpl::_GetWoBLEServiceMode(void)
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
} // namespace Weave
} // namespace nl

#endif // WEAVE_DEVICE_CONFIG_ENABLE_WOBLE

#endif // BLE_MANAGER_IMPL_H
