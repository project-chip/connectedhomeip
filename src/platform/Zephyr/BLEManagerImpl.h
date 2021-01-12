/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          for the Zephyr platforms.
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>

#include <support/logging/CHIPLogging.h>

#include <type_traits>

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
    // As a result of https://github.com/zephyrproject-rtos/zephyr/issues/29357, BLE indication
    // callback parameter type has changed in recent Zephyr revisions. Select the compatible type
    // below to support both versions for now.
    using IndicationAttrType =
        std::conditional_t<std::is_same<bt_gatt_indicate_func_t, void (*)(bt_conn *, bt_gatt_indicate_params *, uint8_t)>::value,
                           bt_gatt_indicate_params *, const bt_gatt_attr *>;

    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
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

    bool SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId);
    bool UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId);
    bool CloseConnection(BLE_CONNECTION_OBJECT conId);
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const;
    bool SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                        PacketBufferHandle pBuf);
    bool SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                          PacketBufferHandle pBuf);
    bool SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                         PacketBufferHandle pBuf);
    bool SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const ChipBleUUID * svcId,
                          const ChipBleUUID * charId);

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId);

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

    struct ServiceData;

    uint16_t mFlags;
    uint16_t mGAPConns;
    CHIPoBLEServiceMode mServiceMode;
    bool mSubscribedConns[CONFIG_BT_MAX_CONN];
    bt_gatt_indicate_params mIndicateParams[CONFIG_BT_MAX_CONN];
    bt_conn_cb mConnCallbacks;

    void DriveBLEState(void);
    CHIP_ERROR ConfigureAdvertising(void);
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);
    CHIP_ERROR HandleGAPConnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleGAPDisconnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleRXCharWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXCharCCCDWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXComplete(const ChipDeviceEvent * event);
    bool IsSubscribed(bt_conn * conn);
    bool SetSubscribed(bt_conn * conn);
    bool UnsetSubscribed(bt_conn * conn);
    uint32_t GetAdvertisingInterval();

    static void DriveBLEState(intptr_t arg);

    // Below callbacks run from the system workqueue context and have a limited stack capacity.
    static void HandleTXIndicated(bt_conn * conn, IndicationAttrType attr, uint8_t err);
    static void HandleConnect(bt_conn * conn, uint8_t err);
    static void HandleDisconnect(bt_conn * conn, uint8_t reason);
    static void HandleBLEAdvertisementTimeout(System::Layer * layer, void * param, System::Error error);

    // ===== Members for internal use by the following friends.

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static BLEManagerImpl sInstance;

public:
    // Below callbacks are public in order to be visible from the global scope.
    static ssize_t HandleRXWrite(bt_conn * conn, const bt_gatt_attr * attr, const void * buf, uint16_t len, uint16_t offset,
                                 uint8_t flags);
    static ssize_t HandleTXCCCWrite(bt_conn * conn, const bt_gatt_attr * attr, uint16_t value);
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
