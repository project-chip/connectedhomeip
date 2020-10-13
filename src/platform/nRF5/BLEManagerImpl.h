/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          for the nRF5 platforms.
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "nrf_ble_gatt.h"
#include "nrf_sdh_ble.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

/**
 * Concrete implementation of the NetworkProvisioningServer singleton object for the nRF5 platforms.
 */
class BLEManagerImpl final : public BLEManager, private BleLayer, private BlePlatformDelegate, private BleApplicationDelegate
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

    CHIP_ERROR _Init(void);
    CHIP_ERROR _PlatformInit(void);
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
    BleLayer * _GetBleLayer(void);

    // ===== Members that implement virtual methods on BlePlatformDelegate.

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                 const Ble::ChipBleUUID * charId) override;
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                   const Ble::ChipBleUUID * charId) override;
    bool CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;
    bool SendIndication(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                        System::PacketBuffer * pBuf) override;
    bool SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                          System::PacketBuffer * pBuf) override;
    bool SendReadRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                         System::PacketBuffer * pBuf) override;
    bool SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const Ble::ChipBleUUID * svcId,
                          const Ble::ChipBleUUID * charId) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    enum
    {
        kFlag_AsyncInitCompleted     = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kFlag_AdvertisingEnabled     = 0x0002, /**< The application has enabled CHIPoBLE advertising. */
        kFlag_FastAdvertisingEnabled = 0x0004, /**< The application has enabled fast advertising. */
        kFlag_Advertising            = 0x0008, /**< The system is currently CHIPoBLE advertising. */
        kFlag_AdvertisingRefreshNeeded =
            0x0010, /**< The advertising state/configuration has changed, but the SoftDevice has yet to be updated. */
    };

    enum
    {
        kMaxConnections             = MIN(BLE_LAYER_NUM_BLE_ENDPOINTS, NRF_SDH_BLE_PERIPHERAL_LINK_COUNT),
        kMaxDeviceNameLength        = 20, // TODO: right-size this
        kMaxAdvertismentDataSetSize = 31  // TODO: verify this
    };

    ble_gatts_char_handles_t mCHIPoBLECharHandle_RX;
    ble_gatts_char_handles_t mCHIPoBLECharHandle_TX;
    CHIPoBLEServiceMode mServiceMode;
    uint16_t mFlags;
    uint16_t mNumGAPCons;
    uint16_t mSubscribedConIds[kMaxConnections];
    uint8_t mAdvHandle;
    uint8_t mAdvDataBuf[kMaxAdvertismentDataSetSize];
    uint8_t mScanRespDataBuf[kMaxAdvertismentDataSetSize];

    void DriveBLEState(void);
    CHIP_ERROR ConfigureAdvertising(void);
    CHIP_ERROR EncodeAdvertisingData(ble_gap_adv_data_t & gapAdvData);
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);
    void HandleSoftDeviceBLEEvent(const ChipDeviceEvent * event);
    CHIP_ERROR HandleGAPConnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleGAPDisconnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleRXCharWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXCharCCCDWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXComplete(const ChipDeviceEvent * event);
    CHIP_ERROR SetSubscribed(uint16_t conId);
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

inline BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
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
