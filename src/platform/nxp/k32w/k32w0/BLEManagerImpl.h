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
 *          for the K32W platforms.
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "Messaging.h"

#include "fsl_os_abstraction.h"

#include "ble_conn_manager.h"
#include "ble_controller_task_config.h"
#include "ble_general.h"
#include "ble_host_task_config.h"
#include "ble_host_tasks.h"
#include "controller_interface.h"
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
 * Concrete implementation of the BLEManager singleton object for the K32W platforms.
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

    enum class Flags : uint8_t
    {
        kAdvertisingEnabled      = 0x0001,
        kFastAdvertisingEnabled  = 0x0002,
        kAdvertising             = 0x0004,
        kRestartAdvertising      = 0x0008,
        kK32WBLEStackInitialized = 0x0010,
        kDeviceNameSet           = 0x0020,
    };
    BitFlags<BLEManagerImpl::Flags> mFlags;

    enum
    {
        kMaxConnections      = BLE_LAYER_NUM_BLE_ENDPOINTS,
        kMaxDeviceNameLength = 32,
        kUnusedIndex         = 0xFF,
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
        BLE_INTERNAL_ERROR,
        BLE_KW_MSG_2M_UPGRADE_ERROR,
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
    uint8_t mAdvHandle;
    char mDeviceName[kMaxDeviceNameLength + 1];
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    chip::System::PacketBufferHandle c3AdditionalDataBufferHandle;
#endif

    void DriveBLEState(void);
    CHIP_ERROR ConfigureAdvertising(void);
    CHIP_ERROR StartAdvertising(void);
    CHIP_ERROR StopAdvertising(void);
    void HandleSoftDeviceBLEEvent(const ChipDeviceEvent * event);
    void HandleConnectEvent(blekw_msg_t * msg);
    void HandleConnectionCloseEvent(blekw_msg_t * msg);
    void HandleWriteEvent(blekw_msg_t * msg);
    void HandleRXCharWrite(blekw_msg_t * msg);
    void HandleTXCharCCCDWrite(blekw_msg_t * msg);
    CHIP_ERROR HandleGAPConnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleGAPDisconnect(const ChipDeviceEvent * event);
    CHIP_ERROR HandleRXCharWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXCharCCCDWrite(const ChipDeviceEvent * event);
    CHIP_ERROR HandleTXComplete(const ChipDeviceEvent * event);
    CHIP_ERROR SetSubscribed(uint16_t conId);
    bool UnsetSubscribed(uint16_t conId);
    bool IsSubscribed(uint16_t conId);
    CHIP_ERROR ConfigureAdvertisingData(void);
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    CHIP_ERROR EncodeAdditionalDataTlv();
    void HandleC3ReadRequest(blekw_msg_t * msg);
#endif
    BLEManagerImpl::ble_err_t blekw_send_event(int8_t connection_handle, uint16_t handle, uint8_t * data, uint32_t len);
    bool RemoveConnection(uint8_t connectionHandle);
    void AddConnection(uint8_t connectionHandle);
    BLEManagerImpl::CHIPoBLEConState * GetConnectionState(uint8_t connectionHandle, bool allocate);

    static void DriveBLEState(intptr_t arg);

    static void BLE_SignalFromISRCallback(void);
    static void blekw_connection_timeout_cb(TimerHandle_t timer);
    static CHIP_ERROR blekw_msg_add_u8(blekw_msg_type_t type, uint8_t data);
    static void blekw_new_data_received_notification(uint32_t mask);
    static void BleAdvTimeoutHandler(TimerHandle_t xTimer);
    static void CancelBleAdvTimeoutTimer(void);
    static void StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs);
    static CHIP_ERROR blekw_controller_init(void);
    static CHIP_ERROR blekw_host_init(void);
    static void Host_Task(osaTaskParam_t argument);
    static void blekw_generic_cb(gapGenericEvent_t * pGenericEvent);
    static void blekw_gatt_server_cb(deviceId_t deviceId, gattServerEvent_t * pServerEvent);
    static CHIP_ERROR blekw_msg_add_u16(blekw_msg_type_t type, uint16_t data);
    static CHIP_ERROR blekw_msg_add_att_written(blekw_msg_type_t type, uint8_t device_id, uint16_t handle, uint8_t * data,
                                                uint16_t length);
    static CHIP_ERROR blekw_msg_add_att_read(blekw_msg_type_t type, uint8_t device_id, uint16_t handle);
    static BLEManagerImpl::ble_err_t blekw_start_advertising(gapAdvertisingParameters_t * adv_params, gapAdvertisingData_t * adv,
                                                             gapScanResponseData_t * scnrsp);
    static BLEManagerImpl::ble_err_t blekw_stop_advertising(void);
    static void blekw_gap_advertising_cb(gapAdvertisingEvent_t * pAdvertisingEvent);
    static void blekw_gap_connection_cb(deviceId_t deviceId, gapConnectionEvent_t * pConnectionEvent);
    static void blekw_start_connection_timeout(void);
    static void blekw_stop_connection_timeout(void);

    static void bleAppTask(void * p_arg);

public:
    static bool blekw_stop_connection_internal(BLE_CONNECTION_OBJECT conId);
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
 * that are specific to the K32W platforms.
 */
inline BLEManagerImpl & BLEMgrImpl(void)
{
    return BLEManagerImpl::sInstance;
}

inline BleLayer * BLEManagerImpl::_GetBleLayer()
{
    return this;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
