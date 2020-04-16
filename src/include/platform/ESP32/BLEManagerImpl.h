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
 *          for the ESP32 platform.
 */

#ifndef BLE_MANAGER_IMPL_H
#define BLE_MANAGER_IMPL_H

#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

/**
 * Concrete implementation of the NetworkProvisioningServer singleton object for the ESP32 platform.
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
        kFlag_AsyncInitCompleted        = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kFlag_ESPBLELayerInitialized    = 0x0002, /**< The ESP BLE layer has been initialized. */
        kFlag_AppRegistered             = 0x0004, /**< The WoBLE application has been registered with the ESP BLE layer. */
        kFlag_AttrsRegistered           = 0x0008, /**< The WoBLE GATT attributes have been registered with the ESP BLE layer. */
        kFlag_GATTServiceStarted        = 0x0010, /**< The WoBLE GATT service has been started. */
        kFlag_AdvertisingConfigured     = 0x0020, /**< WoBLE advertising has been configured in the ESP BLE layer. */
        kFlag_Advertising               = 0x0040, /**< The system is currently WoBLE advertising. */
        kFlag_ControlOpInProgress       = 0x0080, /**< An async control operation has been issued to the ESP BLE layer. */
        kFlag_AdvertisingEnabled        = 0x0100, /**< The application has enabled WoBLE advertising. */
        kFlag_FastAdvertisingEnabled    = 0x0200, /**< The application has enabled fast advertising. */
        kFlag_UseCustomDeviceName       = 0x0400, /**< The application has configured a custom BLE device name. */
        kFlag_AdvertisingRefreshNeeded  = 0x0800, /**< The advertising configuration/state in ESP BLE layer needs to be updated. */
    };

    enum
    {
        kMaxConnections = BLE_LAYER_NUM_BLE_ENDPOINTS,
        kMaxDeviceNameLength = 16
    };

    struct WoBLEConState
    {
        PacketBuffer * PendingIndBuf;
        uint16_t ConId;
        uint16_t MTU : 10;
        uint16_t Allocated : 1;
        uint16_t Subscribed : 1;
        uint16_t Unused : 4;
    };

    WoBLEConState mCons[kMaxConnections];
    WoBLEServiceMode mServiceMode;
    esp_gatt_if_t mAppIf;
    uint16_t mServiceAttrHandle;
    uint16_t mRXCharAttrHandle;
    uint16_t mTXCharAttrHandle;
    uint16_t mTXCharCCCDAttrHandle;
    uint16_t mFlags;
    char mDeviceName[kMaxDeviceNameLength + 1];

    void DriveBLEState(void);
    WEAVE_ERROR InitESPBleLayer(void);
    WEAVE_ERROR ConfigureAdvertisingData(void);
    WEAVE_ERROR StartAdvertising(void);
    void HandleGATTControlEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param);
    void HandleGATTCommEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param);
    void HandleRXCharWrite(esp_ble_gatts_cb_param_t * param);
    void HandleTXCharRead(esp_ble_gatts_cb_param_t * param);
    void HandleTXCharCCCDRead(esp_ble_gatts_cb_param_t * param);
    void HandleTXCharCCCDWrite(esp_ble_gatts_cb_param_t * param);
    void HandleTXCharConfirm(WoBLEConState * conState, esp_ble_gatts_cb_param_t * param);
    void HandleDisconnect(esp_ble_gatts_cb_param_t * param);
    WoBLEConState * GetConnectionState(uint16_t conId, bool allocate = false);
    bool ReleaseConnectionState(uint16_t conId);

    static void HandleGATTEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param);
    static void HandleGAPEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t * param);
    static void DriveBLEState(intptr_t arg);
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
 * that are specific to the ESP32 platform.
 */
inline BLEManagerImpl & BLEMgrImpl(void)
{
    return BLEManagerImpl::sInstance;
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
