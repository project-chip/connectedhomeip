/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2020 Nest Labs, Inc.
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
 *          for NXP platforms.
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <set>

#include "fsl_os_abstraction.h"

#include "ble_conn_manager.h"
#include "ble_general.h"
#include "ble_host_task_config.h"
#include "ble_host_tasks.h"
#include "gap_interface.h"
#include "gatt_db_dynamic.h"
#include "gatt_server_interface.h"

#include "FreeRTOS.h"
#include "event_groups.h"
#include "timers.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

/**
 * A delegate class that can be used by the application to subscribe to BLE events.
 */
struct BLECallbackDelegate
{
    using GapGenericCallback = void (*)(gapGenericEvent_t * event);
    using GattServerCallback = void (*)(deviceId_t id, gattServerEvent_t * event);

    GapGenericCallback gapCallback  = nullptr;
    GattServerCallback gattCallback = nullptr;
};

typedef enum service_mode_t
{
    kCHIPoBLE_NotSupported = 0,
    kCHIPoBLE_Enabled,
    kCHIPoBLE_Disabled,
    kMultipleBLE_Enabled,
    kMultipleBLE_Disabled,
} service_mode_t;

/**
 * Base class for different platform implementations (K32W0 and K32W1 for now).
 */
class BLEManagerCommon : public BLEManager, protected BleLayer, private BlePlatformDelegate, private BleApplicationDelegate
{
protected:
    // ===== Members that implement the BLEManager internal interface.

    CHIP_ERROR _Init(void);
    CHIP_ERROR _Shutdown() { return CHIP_NO_ERROR; }
    CHIP_ERROR _SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val);
    bool _IsAdvertisingEnabled(void);
    CHIP_ERROR _SetAdvertisingEnabled(bool val);
    bool _IsAdvertising(void);
    CHIP_ERROR _SetAdvertisingMode(BLEAdvertisingMode mode);
    CHIP_ERROR _GetDeviceName(char * buf, size_t bufSize);
    CHIP_ERROR _SetDeviceName(const char * deviceName);
    uint16_t _NumConnections(void);
    void _OnPlatformEvent(const ChipDeviceEvent * event);

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

    // ===== Private members reserved for use by this class only.

    enum class Flags : uint8_t
    {
        kAdvertisingEnabled      = 0x0001,
        kFastAdvertisingEnabled  = 0x0002,
        kAdvertising             = 0x0004,
        kRestartAdvertising      = 0x0008,
        kK32WBLEStackInitialized = 0x0010,
        kDeviceNameSet           = 0x0020,
    };
    BitFlags<BLEManagerCommon::Flags> mFlags;

    enum
    {
        kMaxDeviceNameLength = 16,
        kUnusedIndex         = 0xFF,
    };

    enum
    {
        kMaxHandles = 5,
    };

    typedef enum
    {
        BLE_KW_MSG_ERROR = 0x01,
        BLE_KW_MSG_CONNECTED,
        BLE_KW_MSG_DISCONNECTED,
        BLE_KW_MSG_MTU_CHANGED,
        BLE_KW_MSG_ATT_WRITTEN,
        BLE_KW_MSG_ATT_LONG_WRITTEN,
        BLE_KW_MSG_ATT_READ,
        BLE_KW_MSG_ATT_CCCD_WRITTEN,
        BLE_KW_MSG_FORCE_DISCONNECT,
    } blekw_msg_type_t;

    typedef struct hk_ble_kw_msg_s
    {
        blekw_msg_type_t type;
        uint16_t length;
        union
        {
            uint8_t u8;
            uint16_t u16;
            uint32_t u32;
            uint8_t data[1];
            char * str;
        } data;
    } blekw_msg_t;

    typedef enum ble_err_t
    {
        BLE_OK = 0,
        BLE_INTERNAL_GATT_ERROR,
        BLE_E_SET_ADV_PARAMS,
        BLE_E_ADV_PARAMS_FAILED,
        BLE_E_SET_ADV_DATA,
        BLE_E_ADV_CHANGED,
        BLE_E_ADV_FAILED,
        BLE_E_ADV_SETUP_FAILED,
        BLE_E_START_ADV,
        BLE_E_STOP,
        BLE_E_FAIL,
        BLE_E_START_ADV_FAILED,
        BLE_KW_MSG_2M_UPGRADE_ERROR,
        BLE_INTERNAL_ERROR,
    } ble_err_t;

    typedef struct ble_att_written_data_s
    {
        uint8_t device_id;
        uint16_t handle;
        uint16_t length;
        uint8_t data[1];
    } blekw_att_written_data_t;

    typedef struct hk_ble_att_read_data_s
    {
        uint8_t device_id;
        uint16_t handle;
    } blekw_att_read_data_t;

    service_mode_t mServiceMode;
    char mDeviceName[kMaxDeviceNameLength + 1];
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    chip::System::PacketBufferHandle c3AdditionalDataBufferHandle;
#endif
    std::set<uint8_t> mDeviceIds;
    bool mDeviceSubscribed = false;

    uint8_t mReadHandleSize  = 0;
    uint8_t mWriteHandleSize = 0;
    uint16_t mReadNotificationHandle[kMaxHandles];
    uint16_t mWriteNotificationHandle[kMaxHandles];

    void DriveBLEState(void);
    CHIP_ERROR ConfigureAdvertising(void);
    CHIP_ERROR ConfigureAdvertisingData(void);
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);

    void HandleConnectEvent(blekw_msg_t * msg);
    void HandleConnectionCloseEvent(blekw_msg_t * msg);
    void HandleWriteEvent(blekw_msg_t * msg);
    void HandleRXCharWrite(blekw_msg_t * msg);
    void HandleTXCharCCCDWrite(blekw_msg_t * msg);
    void HandleForceDisconnect();

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    CHIP_ERROR EncodeAdditionalDataTlv();
    void HandleC3ReadRequest(blekw_msg_t * msg);
#endif
    BLEManagerCommon::ble_err_t blekw_send_event(int8_t connection_handle, uint16_t handle, uint8_t * data, uint32_t len);

    static void DriveBLEState(intptr_t arg);
    static void StopAdvertisingPriorToSwitchingMode(intptr_t arg);
    static void BleAdvTimeoutHandler(TimerHandle_t xTimer);
    static void CancelBleAdvTimeoutTimer(void);
    static void StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs);

    static void blekw_connection_timeout_cb(TimerHandle_t timer);
    static void blekw_generic_cb(gapGenericEvent_t * pGenericEvent);
    static void blekw_gatt_server_cb(deviceId_t deviceId, gattServerEvent_t * pServerEvent);
    static CHIP_ERROR blekw_msg_add_u8(blekw_msg_type_t type, uint8_t data);
    static CHIP_ERROR blekw_msg_add_u16(blekw_msg_type_t type, uint16_t data);
    static CHIP_ERROR blekw_msg_add_att_written(blekw_msg_type_t type, uint8_t device_id, uint16_t handle, uint8_t * data,
                                                uint16_t length);
    static CHIP_ERROR blekw_msg_add_att_read(blekw_msg_type_t type, uint8_t device_id, uint16_t handle);
    static BLEManagerCommon::ble_err_t blekw_start_advertising(gapAdvertisingParameters_t * adv_params, gapAdvertisingData_t * adv,
                                                               gapScanResponseData_t * scnrsp);
    static BLEManagerCommon::ble_err_t blekw_stop_advertising(void);
    static void blekw_gap_advertising_cb(gapAdvertisingEvent_t * pAdvertisingEvent);
    static void blekw_gap_connection_cb(deviceId_t deviceId, gapConnectionEvent_t * pConnectionEvent);
    static void blekw_start_connection_timeout(void);
    static void blekw_stop_connection_timeout(void);
    static CHIP_ERROR blekw_stop_connection_internal(BLE_CONNECTION_OBJECT conId);

public:
    virtual CHIP_ERROR InitHostController(BLECallbackDelegate::GapGenericCallback cb_fp) = 0;
    virtual BLEManagerCommon * GetImplInstance()                                         = 0;
    virtual CHIP_ERROR ResetController() { return CHIP_NO_ERROR; }
    void DoBleProcessing(void);

    BLECallbackDelegate callbackDelegate;
    void RegisterAppCallbacks(BLECallbackDelegate::GapGenericCallback gapCallback,
                              BLECallbackDelegate::GattServerCallback gattCallback);

    CHIP_ERROR AddWriteNotificationHandle(uint16_t name);
    CHIP_ERROR AddReadNotificationHandle(uint16_t name);

    service_mode_t GetBLEServiceMode(void) { return mServiceMode; }
    void SetBLEServiceMode(service_mode_t mode) { mServiceMode = mode; };
};

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
