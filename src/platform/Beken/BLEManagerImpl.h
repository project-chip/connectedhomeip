/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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
#include "event_groups.h"

#include "matter_pal.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

/**
 * Concrete implementation of the BLEManager singleton object for the Ameba platforms.
 */
class BLEManagerImpl final : public BLEManager, private BleLayer, private BlePlatformDelegate, private BleApplicationDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

private:
    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    CHIP_ERROR _Shutdown() { return CHIP_NO_ERROR; }
    CHIPoBLEServiceMode _GetCHIPoBLEServiceMode(void);
    CHIP_ERROR _SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val);
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

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static BLEManagerImpl sInstance;

    // ===== Private members reserved for use by this class only.
    enum class Flags : uint32_t
    {
        kAdvertisingEnabled       = 0x0001,
        kAdvertisingRefreshNeeded = 0x0002,
        kFastAdvertisingEnabled   = 0x0004,
        kSlowAdvertisingEnabled   = 0x0008,
        kAdvertisingIsFastADV     = 0x0010,
        kAMEBABLEStackInitialized = 0x0020,
        kDeviceNameSet            = 0x0040,
        kDeviceNameDefSet         = 0x0080,

        kBekenBLESGATTSReady = 0x00100,
        kBEKENBLEADVCreate   = 0x00200,
        kBEKENBLEADVSetData  = 0x00400,
        kBEKENBLEADVSetRsp   = 0x00800,
        kBEKENBLEADVStarted  = 0x01000,
        kBEKENBLEADVStop     = 0x02000,
        kBEKENBLEADVDelet    = 0x04000,
        kBEKENBLEAdvTimer    = 0x08000,
        kBEKENBLEAdvTimerRun = 0x10000,
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
        uint8_t conn_idx;
        uint16_t mtu : 16;
        uint16_t allocated : 1;
        uint16_t subscribed : 1;
        uint16_t unused : 6;
        void Set(uint16_t conId)
        {
            conn_idx   = conId;
            mtu        = 0;
            allocated  = 1;
            subscribed = 0;
            unused     = 0;
        }
        void Reset()
        {
            conn_idx   = kUnusedIndex;
            mtu        = 0;
            allocated  = 0;
            subscribed = 0;
            unused     = 0;
        }
    };
    CHIPoBLEConState mBleConnections[kMaxConnections];

    CHIPoBLEServiceMode mServiceMode;

    uint16_t mNumGAPCons;
    uint16_t mTXCharCCCDAttrHandle;
    char mDeviceName[kMaxDeviceNameLength + 1];
    CHIP_ERROR MapBLEError(int bleErr);

    void DriveBLEState(void);
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);
    CHIP_ERROR ConfigureAdvertisingData(void);

    void HandleRXCharWrite(uint8_t *, uint16_t, uint8_t);
    void HandleTXCharRead(void * param);
    void HandleTXCharCCCDRead(void * param);
    void HandleTXCharCCCDWrite(int, int, int ind = 0);
    void HandleTXCharConfirm(CHIPoBLEConState * conState, int status);
    CHIP_ERROR HandleTXComplete(int);
    CHIP_ERROR HandleGAPConnect(uint16_t);
    CHIP_ERROR HandleGAPDisconnect(uint16_t, uint16_t);
    CHIP_ERROR SetSubscribed(uint16_t conId);
    bool UnsetSubscribed(uint16_t conId);
    bool IsSubscribed(uint16_t conId);

    bool RemoveConnection(uint8_t connectionHandle);

    uint8_t adv_actv_idx;
    BLEManagerImpl::CHIPoBLEConState * GetConnectionState(uint8_t connectionHandle, bool allocate = false);
    static void ble_event_notice(ble_notice_t notice, void * param);
    static void beken_ble_cmd_cb(ble_cmd_t cmd, ble_cmd_param_t * param);
    static void DriveBLEState(intptr_t arg);
    static void DriveBLEExtPerf(intptr_t arg);
    static int beken_ble_init(void);
    static void ble_adv_timer_timeout_handle(TimerHandle_t xTimer);
    static void CancelBleAdvTimeoutTimer(void);
    static void StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs);
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    void HandleC3CharRead(void * param);
#endif
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

inline BLEManager::CHIPoBLEServiceMode BLEManagerImpl::_GetCHIPoBLEServiceMode(void)
{
    return mServiceMode;
}

inline bool BLEManagerImpl::_IsAdvertisingEnabled(void)
{
    return mFlags.Has(Flags::kAdvertisingEnabled);
}

inline bool BLEManagerImpl::_IsAdvertising(void)
{
    return mFlags.Has(Flags::kBEKENBLEADVStarted);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
