/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "sdkconfig.h"

#if CONFIG_BT_BLUEDROID_ENABLED

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include <lib/core/CHIPCallback.h>
#elif CONFIG_BT_NIMBLE_ENABLED

/* min max macros in NimBLE can cause build issues with generic min max
 * functions defined in CHIP.*/
#define min
#define max
#include "host/ble_hs.h"
#undef min
#undef max

/* GATT context */
struct ble_gatt_char_context
{
    uint16_t conn_handle;
    uint16_t attr_handle;
    struct ble_gatt_access_ctxt * ctxt;
    void * arg;
};

#endif

#include "ble/Ble.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

/**
 * Concrete implementation of the BLEManager singleton object for the ESP32 platform.
 */
class BLEManagerImpl final : public BLEManager,
                             private Ble::BleLayer,
                             private Ble::BlePlatformDelegate,
                             private Ble::BleApplicationDelegate
{
public:
    BLEManagerImpl() {}

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
    ::chip::Ble::BleLayer * _GetBleLayer(void);

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
        kAsyncInitCompleted       = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kESPBLELayerInitialized   = 0x0002, /**< The ESP BLE layer has been initialized. */
        kAppRegistered            = 0x0004, /**< The CHIPoBLE application has been registered with the ESP BLE layer. */
        kAttrsRegistered          = 0x0008, /**< The CHIPoBLE GATT attributes have been registered with the ESP BLE layer. */
        kGATTServiceStarted       = 0x0010, /**< The CHIPoBLE GATT service has been started. */
        kAdvertisingConfigured    = 0x0020, /**< CHIPoBLE advertising has been configured in the ESP BLE layer. */
        kAdvertising              = 0x0040, /**< The system is currently CHIPoBLE advertising. */
        kControlOpInProgress      = 0x0080, /**< An async control operation has been issued to the ESP BLE layer. */
        kAdvertisingEnabled       = 0x0100, /**< The application has enabled CHIPoBLE advertising. */
        kFastAdvertisingEnabled   = 0x0200, /**< The application has enabled fast advertising. */
        kUseCustomDeviceName      = 0x0400, /**< The application has configured a custom BLE device name. */
        kAdvertisingRefreshNeeded = 0x0800, /**< The advertising configuration/state in ESP BLE layer needs to be updated. */
    };

    enum
    {
        kMaxConnections      = BLE_LAYER_NUM_BLE_ENDPOINTS,
        kMaxDeviceNameLength = 16
    };

#if CONFIG_BT_NIMBLE_ENABLED
    uint16_t mSubscribedConIds[kMaxConnections];
#endif

    struct CHIPoBLEConState
    {
        System::PacketBufferHandle PendingIndBuf;
        uint16_t ConId;
        uint16_t MTU : 10;
        uint16_t Allocated : 1;
        uint16_t Subscribed : 1;
        uint16_t Unused : 4;

        void Set(uint16_t conId)
        {
            PendingIndBuf = nullptr;
            ConId         = conId;
            MTU           = 0;
            Allocated     = 1;
            Subscribed    = 0;
            Unused        = 0;
        }
        void Reset()
        {
            PendingIndBuf = nullptr;
            ConId         = BLE_CONNECTION_UNINITIALIZED;
            MTU           = 0;
            Allocated     = 0;
            Subscribed    = 0;
            Unused        = 0;
        }
    };

    CHIPoBLEConState mCons[kMaxConnections];
    CHIPoBLEServiceMode mServiceMode;
#if CONFIG_BT_BLUEDROID_ENABLED
    esp_gatt_if_t mAppIf;
#elif CONFIG_BT_NIMBLE_ENABLED
    uint16_t mNumGAPCons;
#endif
    uint16_t mServiceAttrHandle;
    uint16_t mRXCharAttrHandle;
    uint16_t mTXCharAttrHandle;
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    uint16_t mC3CharAttrHandle;
#endif
    uint16_t mTXCharCCCDAttrHandle;
    BitFlags<Flags> mFlags;
    char mDeviceName[kMaxDeviceNameLength + 1];
    CHIP_ERROR MapBLEError(int bleErr);

    void DriveBLEState(void);
    CHIP_ERROR InitESPBleLayer(void);
    CHIP_ERROR ConfigureAdvertisingData(void);
    CHIP_ERROR StartAdvertising(void);

    static constexpr System::Clock::Timeout kFastAdvertiseTimeout =
        System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
    System::Clock::Timestamp mAdvertiseStartTime;

    static void HandleFastAdvertisementTimer(System::Layer * systemLayer, void * context);
    void HandleFastAdvertisementTimer();

#if CONFIG_BT_BLUEDROID_ENABLED
    void HandleGATTControlEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param);
    void HandleGATTCommEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param);
    void HandleRXCharWrite(esp_ble_gatts_cb_param_t * param);
    void HandleTXCharRead(esp_ble_gatts_cb_param_t * param);
    void HandleTXCharCCCDRead(esp_ble_gatts_cb_param_t * param);
    void HandleTXCharCCCDWrite(esp_ble_gatts_cb_param_t * param);
    void HandleTXCharConfirm(CHIPoBLEConState * conState, esp_ble_gatts_cb_param_t * param);
    void HandleDisconnect(esp_ble_gatts_cb_param_t * param);
    CHIPoBLEConState * GetConnectionState(uint16_t conId, bool allocate = false);
    bool ReleaseConnectionState(uint16_t conId);

    static void HandleGATTEvent(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t * param);
    static void HandleGAPEvent(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t * param);

#elif CONFIG_BT_NIMBLE_ENABLED
    void HandleRXCharRead(struct ble_gatt_char_context * param);
    void HandleRXCharWrite(struct ble_gatt_char_context * param);
    void HandleTXCharWrite(struct ble_gatt_char_context * param);
    void HandleTXCharRead(struct ble_gatt_char_context * param);
    void HandleTXCharCCCDRead(void * param);
    void HandleTXCharCCCDWrite(struct ble_gap_event * gapEvent);
    CHIP_ERROR HandleTXComplete(struct ble_gap_event * gapEvent);
    CHIP_ERROR HandleGAPConnect(struct ble_gap_event * gapEvent);
    CHIP_ERROR HandleGAPDisconnect(struct ble_gap_event * gapEvent);
    CHIP_ERROR SetSubscribed(uint16_t conId);
    bool UnsetSubscribed(uint16_t conId);
    bool IsSubscribed(uint16_t conId);

    static CHIP_ERROR bleprph_set_random_addr(void);
    static void bleprph_host_task(void * param);
    static void bleprph_on_sync(void);
    static void bleprph_on_reset(int);
    static const struct ble_gatt_svc_def CHIPoBLEGATTAttrs[];
    static int ble_svr_gap_event(struct ble_gap_event * event, void * arg);

    static int gatt_svr_chr_access(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt * ctxt, void * arg);
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    static int gatt_svr_chr_access_additional_data(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt * ctxt,
                                                   void * arg);
    void HandleC3CharRead(struct ble_gatt_char_context * param);
#endif /* CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING */
#endif

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

inline ::chip::Ble::BleLayer * BLEManagerImpl::_GetBleLayer()
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
