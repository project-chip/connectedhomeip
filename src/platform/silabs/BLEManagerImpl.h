/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 *          for the Silicon Labs EFR32 platforms.
 */

#pragma once
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#ifdef RSI_BLE_ENABLE
#define BLE_MIN_CONNECTION_INTERVAL_MS 45 // 45 msec
#define BLE_MAX_CONNECTION_INTERVAL_MS 45 // 45 msec
#define BLE_SLAVE_LATENCY_MS 0
#define BLE_TIMEOUT_MS 400
#endif // RSI_BLE_ENABLE
#include "FreeRTOS.h"
#include "timers.h"
#ifdef RSI_BLE_ENABLE
#ifdef __cplusplus
extern "C" {
#endif
#include <rsi_ble.h>
#include <rsi_ble_apis.h>
#include <rsi_bt_common.h>
#ifdef __cplusplus
}
#endif
#else
#include "gatt_db.h"
#include "sl_bgapi.h"
#include "sl_bt_api.h"
#endif // RSI_BLE_ENABLE

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

/**
 * Concrete implementation of the BLEManager singleton object for the EFR32 platforms.
 */
class BLEManagerImpl final : public BLEManager, private BleLayer, private BlePlatformDelegate, private BleApplicationDelegate
{

public:
    void HandleBootEvent(void);

#ifdef RSI_BLE_ENABLE
    void HandleConnectEvent(void);
    void HandleConnectionCloseEvent(uint16_t reason);
    void HandleWriteEvent(rsi_ble_event_write_t evt);
    void UpdateMtu(rsi_ble_event_mtu_t evt);
    void HandleTxConfirmationEvent(BLE_CONNECTION_OBJECT conId);
    void HandleTXCharCCCDWrite(rsi_ble_event_write_t * evt);
    void HandleSoftTimerEvent(void);
#else
    void HandleConnectEvent(volatile sl_bt_msg_t * evt);
    void HandleConnectionCloseEvent(volatile sl_bt_msg_t * evt);
    void HandleWriteEvent(volatile sl_bt_msg_t * evt);
    void UpdateMtu(volatile sl_bt_msg_t * evt);
    void HandleTxConfirmationEvent(BLE_CONNECTION_OBJECT conId);
    void HandleTXCharCCCDWrite(volatile sl_bt_msg_t * evt);
    void HandleSoftTimerEvent(volatile sl_bt_msg_t * evt);
#endif // RSI_BLE_ENABLE
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
#ifdef RSI_BLE_ENABLE
    static void HandleC3ReadRequest(void);
#else
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    static void HandleC3ReadRequest(volatile sl_bt_msg_t * evt);
#endif
#endif
#endif

private:
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

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

    enum class Flags : uint16_t
    {
        kAdvertisingEnabled     = 0x0001,
        kFastAdvertisingEnabled = 0x0002,
        kAdvertising            = 0x0004,
        kRestartAdvertising     = 0x0008,
        kEFRBLEStackInitialized = 0x0010,
        kDeviceNameSet          = 0x0020,
    };

    enum
    {
        kMaxConnections      = BLE_LAYER_NUM_BLE_ENDPOINTS,
        kMaxDeviceNameLength = 16,
        kUnusedIndex         = 0xFF,
    };

    struct CHIPoBLEConState
    {
#ifndef RSI_BLE_ENABLE
        bd_addr address;
#endif
        uint16_t mtu : 10;
        uint16_t allocated : 1;
        uint16_t subscribed : 1;
        uint16_t unused : 4;
        uint8_t connectionHandle;
        uint8_t bondingHandle;
    };

    CHIPoBLEConState mBleConnections[kMaxConnections];
    uint8_t mIndConfId[kMaxConnections];
    CHIPoBLEServiceMode mServiceMode;
    BitFlags<Flags> mFlags;
    char mDeviceName[kMaxDeviceNameLength + 1];
    // The advertising set handle allocated from Bluetooth stack.
    uint8_t advertising_set_handle = 0xff;
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    PacketBufferHandle c3AdditionalDataBufferHandle;
#endif

    CHIP_ERROR MapBLEError(int bleErr);
    void DriveBLEState(void);
    CHIP_ERROR ConfigureAdvertisingData(void);
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    CHIP_ERROR EncodeAdditionalDataTlv();
#endif

#ifdef RSI_BLE_ENABLE
    void HandleRXCharWrite(rsi_ble_event_write_t * evt);
#else
    void HandleRXCharWrite(volatile sl_bt_msg_t * evt);
#endif
    bool RemoveConnection(uint8_t connectionHandle);
    void AddConnection(uint8_t connectionHandle, uint8_t bondingHandle);
    void StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs);
    void CancelBleAdvTimeoutTimer(void);
    CHIPoBLEConState * GetConnectionState(uint8_t conId, bool allocate = false);
    static void DriveBLEState(intptr_t arg);
    static void BleAdvTimeoutHandler(TimerHandle_t xTimer);
    uint8_t GetTimerHandle(uint8_t connectionHandle, bool allocate);
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
 * that are specific to the EFR32 platforms.
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

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
