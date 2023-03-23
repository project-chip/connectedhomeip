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
#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
#include "esp_gattc_api.h"
#endif
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

#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER && CONFIG_BT_NIMBLE_ENABLED
#include "nimble/blecent.h"
#endif

#include "ble/Ble.h"
#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
#include <ble/BleLayer.h>
#include <ble/BleUUID.h>
#include <platform/ESP32/ChipDeviceScanner.h>
#endif

namespace chip {
namespace DeviceLayer {
namespace Internal {

#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
void HandleIncomingBleConnection(Ble::BLEEndPoint * bleEP);

enum class BleScanState : uint8_t
{
    kNotScanning,
    kScanForDiscriminator,
    kScanForAddress,
    kConnecting,
};

struct BLEAdvConfig
{
    char * mpBleName;
    uint32_t mAdapterId;
    uint8_t mMajor;
    uint8_t mMinor;
    uint16_t mVendorId;
    uint16_t mProductId;
    uint64_t mDeviceId;
    uint8_t mPairingStatus;
    uint8_t mType;
    uint16_t mDuration;
    const char * mpAdvertisingUUID;
};

struct BLEScanConfig
{
    // If an active scan for connection is being performed
    BleScanState mBleScanState = BleScanState::kNotScanning;

    // If scanning by discriminator, what are we scanning for
    SetupDiscriminator mDiscriminator;

    // If scanning by address, what address are we searching for
    std::string mAddress;

    // Optional argument to be passed to callback functions provided by the BLE scan/connect requestor
    void * mAppState = nullptr;
};

#endif
/**
 * Concrete implementation of the BLEManager singleton object for the ESP32 platform.
 */
class BLEManagerImpl final : public BLEManager,
                             private Ble::BleLayer,
                             private Ble::BlePlatformDelegate,
#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
                             private Ble::BleApplicationDelegate,
                             private Ble::BleConnectionDelegate,
                             private ChipDeviceScannerDelegate
#else
                             private Ble::BleApplicationDelegate
#endif
{
public:
    BLEManagerImpl() {}
#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    CHIP_ERROR ConfigureBle(uint32_t aAdapterId, bool aIsCentral);
#if CONFIG_BT_BLUEDROID_ENABLED
    static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t * param);
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
#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    void HandlePlatformSpecificBLEEvent(const ChipDeviceEvent * event);
    CHIP_ERROR _SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val);
#endif
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
    // ===== Members that implement virtual methods on BleConnectionDelegate.
#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER

    void NewConnection(chip::Ble::BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator) override;
    CHIP_ERROR CancelConnection() override;

    // ===== Members that implement virtual methods on ChipDeviceScannerDelegate
#if CONFIG_BT_NIMBLE_ENABLED
    virtual void OnDeviceScanned(const struct ble_hs_adv_fields & fields, const ble_addr_t & addr,
                                 const chip::Ble::ChipBLEDeviceIdentificationInfo & info) override;
#elif CONFIG_BT_BLUEDROID_ENABLED
    virtual void OnDeviceScanned(esp_ble_addr_type_t & addr_type, esp_bd_addr_t & addr,
                                 const chip::Ble::ChipBLEDeviceIdentificationInfo & info) override;
#endif

    void OnScanComplete() override;
#endif
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

#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    BLEAdvConfig mBLEAdvConfig;
#endif
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
#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    CHIP_ERROR HandleGAPConnect(esp_ble_gattc_cb_param_t p_data);
    CHIP_ERROR HandleGAPCentralConnect(esp_ble_gattc_cb_param_t p_data);

    static void HandleConnectFailed(CHIP_ERROR error);
    static void ConnectDevice(esp_bd_addr_t & addr, esp_ble_addr_type_t addr_type, uint16_t timeout);
    void HandleGAPConnectionFailed();
    CHIP_ERROR HandleRXNotify(esp_ble_gattc_cb_param_t p_data);
#endif
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
    CHIP_ERROR HandleGAPPeripheralConnect(struct ble_gap_event * gapEvent);
    CHIP_ERROR HandleGAPDisconnect(struct ble_gap_event * gapEvent);
    CHIP_ERROR SetSubscribed(uint16_t conId);
    bool UnsetSubscribed(uint16_t conId);
    bool IsSubscribed(uint16_t conId);
    static void ConnectDevice(const ble_addr_t & addr, uint16_t timeout);
    CHIP_ERROR HandleGAPCentralConnect(struct ble_gap_event * gapEvent);
    void HandleGAPConnectionFailed(struct ble_gap_event * gapEvent, CHIP_ERROR error);

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

#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    static int btshell_on_mtu(uint16_t conn_handle, const struct ble_gatt_error * error, uint16_t mtu, void * arg);

    bool SubOrUnsubChar(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                        bool subscribe);

    static void OnGattDiscComplete(const struct peer * peer, int status, void * arg);
    static void HandleConnectFailed(CHIP_ERROR error);
    CHIP_ERROR HandleRXNotify(struct ble_gap_event * event);
#endif
#endif
#if CONFIG_ENABLE_ESP32_BLE_CONTROLLER
    static void CancelConnect(void);
    static void HandleConnectTimeout(chip::System::Layer *, void * context);
    void InitiateScan(BleScanState scanType);
    static void InitiateScan(intptr_t arg);
    void HandleAdvertisementTimer(System::Layer * systemLayer, void * context);
    void HandleAdvertisementTimer();
    void CleanScanConfig();
    BLEScanConfig mBLEScanConfig;
    bool mIsCentral;
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
