/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include "FreeRTOS.h"
#include "app_msg.h"
#include "event_groups.h"
#include "matter_ble.h"
#include "matter_ble_service.h"
#include "timers.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

/**
 * Concrete implementation of the BLEManager singleton object for the Realtek platforms.
 */
class BLEManagerImpl final : public BLEManager, private BleLayer, private BlePlatformDelegate, private BleApplicationDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

private:
    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    void _Shutdown() {}
    bool _IsAdvertisingEnabled(void);
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsAdvertising(void);
    CHIP_ERROR _SetAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);
    BleLayer * _GetBleLayer(void);

    // ===== Members that implement virtual methods on BlePlatformDelegate.

    CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                       const Ble::ChipBleUUID * charId) override;
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                         const Ble::ChipBleUUID * charId) override;
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                              System::PacketBufferHandle pBuf) override;
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                System::PacketBufferHandle pBuf) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.

    enum class Flags : uint8_t
    {
        kAdvertisingEnabled       = 0x0001,
        kFastAdvertisingEnabled   = 0x0002,
        kAdvertising              = 0x0004,
        kRestartAdvertising       = 0x0008,
        kAMEBABLEStackInitialized = 0x0010,
        kDeviceNameSet            = 0x0020,
        kAdvertisingRefreshNeeded = 0x030,
        kAdvertisingConfigured    = 0x040,
    };
    BitFlags<BLEManagerImpl::Flags> mFlags;

    enum
    {
        kMaxConnections      = BLE_LAYER_NUM_BLE_ENDPOINTS,
        kMaxDeviceNameLength = 16,
        kUnusedIndex         = 0xFF,
    };

    struct CHIPoBLEConState
    {
        uint16_t mtu : 10;
        uint16_t allocated : 1;
        uint16_t subscribed : 1;
        uint16_t unused : 4;
        uint8_t connectionHandle;
        uint8_t bondingHandle;
    };
    CHIPoBLEConState mBleConnections[kMaxConnections];

    CHIPoBLEServiceMode mServiceMode;

    uint16_t mNumGAPCons;
    uint16_t mTXCharCCCDAttrHandle;
    uint16_t mSubscribedConIds[kMaxConnections];
    char mDeviceName[kMaxDeviceNameLength + 1];
    CHIP_ERROR MapBLEError(int bleErr);

    void DriveBLEState(void);
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);
    CHIP_ERROR ConfigureAdvertisingData(void);

    static void HandleFastAdvertisementTimer(System::Layer * systemLayer, void * context);
    void HandleFastAdvertisementTimer();

    void HandleRXCharWrite(uint8_t *, uint16_t, uint8_t);
    void HandleTXCharRead(void * param);
    void HandleTXCharCCCDRead(void * param);
    void HandleTXCharCCCDWrite(int, int);
    CHIP_ERROR HandleTXComplete(int);
    CHIP_ERROR HandleGAPConnect(uint16_t);
    CHIP_ERROR HandleGAPDisconnect(uint16_t, uint16_t);
    CHIP_ERROR SetSubscribed(uint16_t conId);
    bool UnsetSubscribed(uint16_t conId);
    bool IsSubscribed(uint16_t conId);
    void InitSubscribed(void);
    bool RemoveConnection(uint8_t connectionHandle);
    void AddConnection(uint8_t connectionHandle);

    BLEManagerImpl::CHIPoBLEConState * GetConnectionState(uint8_t connectionHandle, bool allocate);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    void HandleC3CharRead(TBTCONFIG_CALLBACK_DATA * p_data);
#endif
    static CHIP_ERROR HandleGapMsg(T_IO_MSG * p_gap_msg);
    static CHIP_ERROR gatt_svr_chr_access(T_SERVER_ID service_id, TBTCONFIG_CALLBACK_DATA * p_data);
    static int ble_callback_dispatcher(void * p_cb_data, int type, T_CHIP_BLEMGR_CALLBACK_TYPE callback_type);

    static void DriveBLEState(intptr_t arg);
    static void BleAdvTimeoutHandler(TimerHandle_t xTimer);
    static void CancelBleAdvTimeoutTimer(void);
    static void StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs);
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
 * that are specific to the platforms.
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
    return mFlags.Has(Flags::kAdvertisingEnabled);
}

inline bool BLEManagerImpl::_IsAdvertising(void)
{
    return mFlags.Has(Flags::kAdvertising);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
