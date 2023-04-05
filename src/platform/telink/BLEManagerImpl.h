/*
 *
 *    Copyright (c) 2022 Project CHIP Authors
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

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId) override;
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId) override;
    bool CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;
    bool SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                        PacketBufferHandle pBuf) override;
    bool SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                          PacketBufferHandle pBuf) override;
    bool SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                         PacketBufferHandle pBuf) override;
    bool SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const ChipBleUUID * svcId,
                          const ChipBleUUID * charId) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

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
    };

    struct ServiceData;

    BitFlags<Flags> mFlags;
    uint16_t mGAPConns;
    CHIPoBLEServiceMode mServiceMode;
    bool mSubscribedConns[CONFIG_BT_MAX_CONN];
    bt_gatt_indicate_params mIndicateParams[CONFIG_BT_MAX_CONN];
    bt_conn_cb mConnCallbacks;
    bt_conn * mconId;
    BLEAdvertisingArbiter::Request mAdvertisingRequest = {};
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    PacketBufferHandle c3CharDataBufferHandle;
#endif
    bool mBLERadioInitialized;

    void DriveBLEState(void);
    CHIP_ERROR PrepareAdvertisingRequest(void);
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);
    CHIP_ERROR HandleGAPConnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleGAPDisconnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleRXCharWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXCharCCCDWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXCharComplete(const ChipDeviceEvent * event);
    CHIP_ERROR HandleBleConnectionClosed(const ChipDeviceEvent * event);

    /*
        @todo WORKAROUND: Due to abscense of non-cuncurrent mode in Matter
        we are emulating connection to Thread with this events and manually
        disconnect BLE ass soon as OperationalNetworkEnabled occures.
        This functionality shall be removed as soon as non-cuncurrent mode
        would be implemented
     */
    CHIP_ERROR HandleThreadStateChange(const ChipDeviceEvent * event);
    CHIP_ERROR HandleOperationalNetworkEnabled(const ChipDeviceEvent * event);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    CHIP_ERROR PrepareC3CharData(void);
#endif
    bool IsSubscribed(bt_conn * conn);
    bool SetSubscribed(bt_conn * conn);
    bool UnsetSubscribed(bt_conn * conn);
    uint32_t GetAdvertisingInterval();

    static void DriveBLEState(intptr_t arg);

    // Below callbacks run from the system workqueue context and have a limited stack capacity.
    static void HandleTXIndicated(bt_conn * conn, bt_gatt_indicate_params * attr, uint8_t err);
    static void HandleConnect(bt_conn * conn, uint8_t err);
    static void HandleDisconnect(bt_conn * conn, uint8_t reason);
    static void HandleBLEAdvertisementIntervalChange(System::Layer * layer, void * param);

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static BLEManagerImpl sInstance;

public:
    // Below callbacks are public in order to be visible from the global scope.
    static ssize_t HandleRXWrite(bt_conn * conn, const bt_gatt_attr * attr, const void * buf, uint16_t len, uint16_t offset,
                                 uint8_t flags);
    static ssize_t HandleTXCCCWrite(bt_conn * conn, const bt_gatt_attr * attr, uint16_t value);

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    static ssize_t HandleC3Read(struct bt_conn * conn, const struct bt_gatt_attr * attr, void * buf, uint16_t len, uint16_t offset);
#endif

    /* Switch to IEEE802154 interface. @todo: remove to other module? */
    void SwitchToIeee802154(void);
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
