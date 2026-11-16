/*
 *
 *    Copyright (c) 2022-2026 Project CHIP Authors
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
 *          Provides an implementation of the BLEManager object
 *          for Telink platforms, by including Zephyr platform
 *          implementation.
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <platform/NetworkCommissioning.h>
#include <platform/Zephyr/BLEAdvertisingArbiter.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/gatt.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

/**
 * Concrete implementation of the BLEManager singleton object for the Zephyr platforms.
 */
class BLEManagerImpl final : public BLEManager, private BleLayer, private BlePlatformDelegate, private BleApplicationDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

private:
    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    void _Shutdown();
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

    CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId) override;
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                         const ChipBleUUID * charId) override;
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;
    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                              PacketBufferHandle pBuf) override;
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                PacketBufferHandle pBuf) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;
    void CheckNonConcurrentBleClosing() override;

    // ===== Private members reserved for use by this class only.

    enum class Flags : uint8_t
    {
        kAsyncInitCompleted     = 0x0001, /**< One-time asynchronous initialization actions have been performed. */
        kAdvertisingEnabled     = 0x0002, /**< The application has enabled CHIPoBLE advertising. */
        kFastAdvertisingEnabled = 0x0004, /**< The application has enabled fast advertising. */
        kAdvertising            = 0x0008, /**< The system is currently CHIPoBLE advertising. */
        kAdvertisingRefreshNeeded =
            0x0010, /**< The advertising state/configuration has changed, but the SoftDevice has yet to be updated. */
        kChipoBleGattServiceRegister = 0x0020, /**< The system has currently CHIPoBLE GATT service registered. */
        kExtendedAdvertisingEnabled  = 0x0040, /**< The application has enabled extended advertising. */
    };

    struct ServiceData;

    BitFlags<Flags> mFlags;
    uint16_t mMatterConnNum;
    CHIPoBLEServiceMode mServiceMode;
    bool mSubscribedConns[CONFIG_BT_MAX_CONN];
    bt_gatt_indicate_params mIndicateParams[CONFIG_BT_MAX_CONN];
    bt_conn_cb mConnCallbacks;
    BLEAdvertisingArbiter::Request mAdvertisingRequest = {};
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    PacketBufferHandle c3CharDataBufferHandle;
#endif
    // The summarized number of Bluetooth LE connections related to the device (including these not related to Matter service).
    uint16_t mTotalConnNum;
#ifdef CONFIG_CHIP_CUSTOM_BLE_ADV_DATA
    Span<bt_data> mCustomAdvertising  = {};
    Span<bt_data> mCustomScanResponse = {};
#endif

    void DriveBLEState(void);
    CHIP_ERROR PrepareAdvertisingRequest();
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);
    CHIP_ERROR HandleGAPConnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleGAPDisconnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleRXCharWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXCharCCCDWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXCharComplete(const ChipDeviceEvent * event);
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    CHIP_ERROR PrepareC3CharData();
#endif
    bool IsSubscribed(bt_conn * conn);
    bool SetSubscribed(bt_conn * conn);
    bool UnsetSubscribed(bt_conn * conn);
    uint32_t GetAdvertisingInterval();
    CHIP_ERROR RegisterGattService();
    CHIP_ERROR UnregisterGattService();

    static void DriveBLEState(intptr_t arg);
#if defined(CONFIG_CHIP_CONCURRENT_MODE) && !defined(CONFIG_CHIP_CONCURRENT_BLE_IDLE) &&                                           \
    !defined(CONFIG_CHIP_CONCURRENT_BLE_OFF_AFTER_COMMISSIONING)
    static void HandleConcurrentModeReAdv(intptr_t arg);
#endif
#if defined(CONFIG_CHIP_CONCURRENT_MODE) && defined(CONFIG_CHIP_CONCURRENT_BLE_OFF_AFTER_COMMISSIONING)
    /* k_work handler: runs in Zephyr sysworkq; only a delayed retry trigger
     * that defers the teardown state machine to the CHIP task. */
    static void HandleBleTeardownWork(struct k_work * work);
    /* Runs on the CHIP task with the CHIP stack lock held: performs the state
     * checks and the BLE/RF teardown transitions. */
    static void DoBleTeardown(intptr_t arg);
#endif

    // Below callbacks run from the system workqueue context and have a limited stack capacity.
    static void HandleTXIndicated(bt_conn * conn, bt_gatt_indicate_params * attr, uint8_t err);
    static void HandleConnect(bt_conn * conn, uint8_t err);
    static void HandleDisconnect(bt_conn * conn, uint8_t reason);
    static void HandleSlowBLEAdvertisementInterval(System::Layer * layer, void * param);
    static void HandleExtendedBLEAdvertisementInterval(System::Layer * layer, void * param);

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
    friend class InitScanCallback;
    bool mBLERadioInitialized = false;

    enum class PrescanState : uint8_t
    {
        kIdle,
        kInProgress,
        kCompleted,
    };
    PrescanState mPrescanState = PrescanState::kIdle;
#endif

    static BLEManagerImpl sInstance;

public:
    // Below callbacks are public in order to be visible from the global scope.
    static ssize_t HandleRXWrite(bt_conn * conn, const bt_gatt_attr * attr, const void * buf, uint16_t len, uint16_t offset,
                                 uint8_t flags);
    static ssize_t HandleTXCCCWrite(bt_conn * conn, const bt_gatt_attr * attr, uint16_t value);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    static ssize_t HandleC3Read(struct bt_conn * conn, const struct bt_gatt_attr * attr, void * buf, uint16_t len, uint16_t offset);
#endif

#ifdef CONFIG_EXPOSE_CHIP_ID_VIA_BLE
    static ssize_t HandleChipIDRead(struct bt_conn * conn, const struct bt_gatt_attr * attr, void * buf, uint16_t len,
                                    uint16_t offset);
#endif

#if defined(CONFIG_CHIP_CONCURRENT_BLE_RESUME_AFTER_COMMISSIONING)
    /* Re-enable BLE after the post-commissioning teardown: restores the
     * BLE/802.15.4 RF time-sharing, re-initializes the BLE stack and starts
     * a connectable advertisement. Returns false if BLE has not been torn
     * down yet or the resume failed. Must run in application task context
     * (blocking). Port of the ot_ble_pm sample ble_enable(). */
    bool ResumeBleAfterTeardown(void);
#endif

#ifdef CONFIG_CHIP_CUSTOM_BLE_ADV_DATA
    void SetCustomAdvertising(Span<bt_data> CustomAdvertising);
    void SetCustomScanResponse(Span<bt_data> CustomScanResponse);
#endif
};

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD && !CHIP_DEVICE_CONFIG_SUPPORTS_CONCURRENT_CONNECTION
class InitScanCallback : public DeviceLayer::NetworkCommissioning::ThreadDriver::ScanCallback
{
public:
    void OnFinished(NetworkCommissioning::Status err, CharSpan debugText,
                    NetworkCommissioning::ThreadScanResponseIterator * networks) override
    {
        BLEManagerImpl & mBLEManagerImpl = BLEMgrImpl();
        if (mBLEManagerImpl.mPrescanState != BLEManagerImpl::PrescanState::kInProgress ||
            !mBLEManagerImpl._IsAdvertisingEnabled() ||
            mBLEManagerImpl.mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_Enabled)
        {
            return;
        }
        mBLEManagerImpl.mPrescanState = BLEManagerImpl::PrescanState::kCompleted;
        TEMPORARY_RETURN_IGNORED mBLEManagerImpl.StartAdvertising();
    };
};
#endif

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
 * that are specific to the Zephyr platforms.
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
