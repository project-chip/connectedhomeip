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

/**
 *    @file
 *          Provides an implementation of the BLEManager singleton object
 *          for the Beken platforms.
 */

/* this file behaves like a config.h, comes first */
#include <crypto/CHIPCryptoPAL.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/internal/CHIPDeviceLayerInternal.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
#include <ble/CHIPBleServiceData.h>
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#endif
#include "stdio.h"
#include "timers.h"
#include <string.h>

// BK7231n BLE related header files
#include "matter_pal.h"

/*******************************************************************************
 * Local data types
 *******************************************************************************/
using namespace ::chip;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

/*******************************************************************************
 * Macros & Constants definitions
 *******************************************************************************/
#ifndef MAX_ADV_DATA_LEN
#define MAX_ADV_DATA_LEN 31
#endif
#define CHIP_ADV_DATA_TYPE_FLAGS 0x01
#define CHIP_ADV_DATA_FLAGS 0x06

/* advertising configuration */
#define CHIP_ADV_SHORT_UUID_LEN (2)

/* FreeRTOS sw timer */
TimerHandle_t bleFastAdvTimer;

enum
{
    DriveBLEExtPerfEvt_DISCONNECT = 0,
};

const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };
static const uint8_t _svc_uuid[16]         = { 0xF6, 0xFF, 0, 0, 0x0, 0x0, 0, 0, 0, 0, 0x0, 0x0, 0, 0, 0, 0 };

#define UUID_CHIPoBLECharact_RX                                                                                                    \
    {                                                                                                                              \
        0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18                             \
    }
//#define UUID_CHIPoBLECharact_RX   { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D, 0x11
//}
#define ChipUUID_CHIPoBLECharact_TX                                                                                                \
    {                                                                                                                              \
        0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18                             \
    }
//#define ChipUUID_CHIPoBLECharact_TX   { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D,
// 0x12 }
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
#define UUID_CHIPoBLEChar_C3                                                                                                       \
    {                                                                                                                              \
        0x04, 0x8F, 0x21, 0x83, 0x8A, 0x74, 0x7D, 0xB8, 0xF2, 0x45, 0x72, 0x87, 0x38, 0x02, 0x63, 0x64                             \
    }
#endif
#define BEKEN_ATT_DECL_PRIMARY_SERVICE_128                                                                                         \
    {                                                                                                                              \
        0x00, 0x28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                       \
    }
#define BEKEN_ATT_DECL_CHARACTERISTIC_128                                                                                          \
    {                                                                                                                              \
        0x03, 0x28, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                       \
    }
#define BEKEN_ATT_DESC_CLIENT_CHAR_CFG_128                                                                                         \
    {                                                                                                                              \
        0x02, 0x29, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                                                       \
    }

enum
{
    SVR_FFF6_IDX_SVC,
    SVR_FFF6_RX_DECL,
    SVR_FFF6_RX_VALUE,
    SVR_FFF6_TX_DECL,
    SVR_FFF6_TX_VALUE,
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    SVR_FFF6_C3_VALUE,
#endif
    SVR_FFF6_TX_CFG,
    SVR_FFF6_MAX,
};

ble_attm_desc_t svr_fff6_att_db[SVR_FFF6_MAX] = {
    //  Service Declaration
    [SVR_FFF6_IDX_SVC] = { BEKEN_ATT_DECL_PRIMARY_SERVICE_128, BK_BLE_PERM_SET(RD, ENABLE), 0, 2 },

    //  Level Characteristic Declaration
    [SVR_FFF6_RX_DECL] = { BEKEN_ATT_DECL_CHARACTERISTIC_128, BK_BLE_PERM_SET(RD, ENABLE), 0, 0 },
    //  Level Characteristic Value  BK_PERM_RIGHT_ENABLE WRITE_REQ_POS  RI_POS  UUID_LEN_POS  BK_PERM_RIGHT_UUID_128
    [SVR_FFF6_RX_VALUE] = { UUID_CHIPoBLECharact_RX, BK_BLE_PERM_SET(WRITE_REQ, ENABLE),
                            BK_BLE_PERM_SET(RI, ENABLE) | BK_BLE_PERM_SET(UUID_LEN, UUID_128), 512 },

    /// RD_POS  NTF_POS
    [SVR_FFF6_TX_DECL] = { BEKEN_ATT_DECL_CHARACTERISTIC_128, BK_BLE_PERM_SET(RD, ENABLE) | BK_BLE_PERM_SET(NTF, ENABLE), 0, 0 },
    ////  UUID_LEN_POS   BK_PERM_RIGHT_UUID_128 RD_POS
    [SVR_FFF6_TX_VALUE] = { ChipUUID_CHIPoBLECharact_TX, BK_BLE_PERM_SET(RD, ENABLE) | BK_BLE_PERM_SET(NTF, ENABLE),
                            BK_BLE_PERM_SET(RI, ENABLE) | BK_BLE_PERM_SET(UUID_LEN, UUID_128), 512 },
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    [SVR_FFF6_C3_VALUE] = { UUID_CHIPoBLEChar_C3, BK_BLE_PERM_SET(RD, ENABLE),
                            BK_BLE_PERM_SET(RI, ENABLE) | BK_BLE_PERM_SET(UUID_LEN, UUID_128), 512 },
#endif
    [SVR_FFF6_TX_CFG] = { BEKEN_ATT_DESC_CLIENT_CHAR_CFG_128, BK_BLE_PERM_SET(RD, ENABLE) | BK_BLE_PERM_SET(WRITE_REQ, ENABLE), 0,
                          2 },
};

/// const static uint8_t svr_fff6_att_db_item = SVR_FFF6_MAX;
} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

int BLEManagerImpl::beken_ble_init(void)
{
    int status;

    struct bk_ble_db_cfg ble_db_cfg;

    ble_db_cfg.att_db      = svr_fff6_att_db;
    ble_db_cfg.att_db_nb   = SVR_FFF6_MAX;
    ble_db_cfg.prf_task_id = 0;
    ble_db_cfg.start_hdl   = 0;
    ble_db_cfg.svc_perm    = BK_BLE_PERM_SET(SVC_UUID_LEN, UUID_16);
    memcpy(&(ble_db_cfg.uuid[0]), &_svc_uuid[0], 16);
    status = bk_ble_create_db(&ble_db_cfg);

    if (status != BK_ERR_BLE_SUCCESS)
    {
        return -1;
    }
    return 0;
}

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    // Initialize the CHIP BleLayer.
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    memset(mBleConnections, 0, sizeof(mBleConnections));
    // Check if BLE stack is initialized
    VerifyOrExit(!mFlags.Has(Flags::kAMEBABLEStackInitialized), err = CHIP_ERROR_INCORRECT_STATE);
    bk_ble_set_notice_cb(ble_event_notice);

    // Set related flags
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kAMEBABLEStackInitialized);
    mFlags.Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART ? true : false);
    mFlags.Set(Flags::kFastAdvertisingEnabled);
    mFlags.Set(Flags::kSlowAdvertisingEnabled);
    if (!mFlags.Has(Flags::kBEKENBLEAdvTimer))
    {
        uint32_t bleAdvTimeoutMs = CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME;
        bleFastAdvTimer          = xTimerCreate("", (bleAdvTimeoutMs / 2), pdFALSE, NULL, ble_adv_timer_timeout_handle);
        mFlags.Set(Flags::kBEKENBLEAdvTimer);
    }

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

void BLEManagerImpl::HandleTXCharRead(void * param)
{
    CHIPoBLEConState * conState;
    ble_read_req_t * r_req = (ble_read_req_t *) param;
    if (param == NULL)
    {
        ChipLogError(DeviceLayer, "HandleTXCharRead failed: %p", param);
    }
    r_req->length = 0;
}

void BLEManagerImpl::HandleTXCharCCCDRead(void * param)
{
    CHIPoBLEConState * conState;
    ble_read_req_t * r_req = (ble_read_req_t *) param;
    conState               = GetConnectionState(r_req->conn_idx);

    if (param == NULL)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDRead failed");
        return;
    }

    if (conState != NULL)
    {
        r_req->value[0] = conState->subscribed ? 1 : 0;
        r_req->value[1] = 0;
        r_req->length   = 2;
    }
    else
    {
        ChipLogError(DeviceLayer, "conState failed: %p", conState);
        r_req->length = 0;
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(int conn_id, int notificationsEnabled, int indicationsEnabled)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // If the client has requested to enabled indications/notifications
    if (indicationsEnabled || notificationsEnabled)
    {
        // If indications are not already enabled for the connection...
        if (!IsSubscribed(conn_id))
        {
            // Record that indications have been enabled for this connection.
            err = SetSubscribed(conn_id);
            VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, err = CHIP_NO_ERROR);
            SuccessOrExit(err);
        }
    }
    else
    {
        // If indications had previously been enabled for this connection, record that they are no longer enabled.
        UnsetSubscribed(conn_id);
    }

    // Post an event to the Chip queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
    // whether the client is enabling or disabling indications.
    {
        ChipDeviceEvent event;
        event.Type = (indicationsEnabled || notificationsEnabled) ? DeviceEventType::kCHIPoBLESubscribe
                                                                  : DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = conn_id;
        PlatformMgr().PostEventOrDie(&event);
    }

    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received",
                    (indicationsEnabled || notificationsEnabled) ? "subscribe" : "unsubscribe");

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
    }

    return;
}

CHIP_ERROR BLEManagerImpl::HandleTXComplete(int conn_id)
{
    // Post an event to the Chip queue to process the indicate confirmation.
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = conn_id;
    PlatformMgr().PostEventOrDie(&event);
    return CHIP_NO_ERROR;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    uint16_t numCons = 0;

    for (uint8_t i = 0; i < kMaxConnections; i++)
    {
        if (mBleConnections[i].allocated == 1)
        {
            numCons += 1;
        }
    }

    return numCons;
}

CHIP_ERROR BLEManagerImpl::HandleGAPConnect(uint16_t conn_id)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    // Track the number of active GAP connections.
    mNumGAPCons++;
    err = SetSubscribed(conn_id);
    VerifyOrExit(err != CHIP_ERROR_NO_MEMORY, err = CHIP_NO_ERROR);
    SuccessOrExit(err);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::HandleGAPDisconnect(uint16_t conn_id, uint16_t disc_cause)
{
    // Update the number of GAP connections.
    if (mNumGAPCons > 0)
    {
        mNumGAPCons--;
    }

    if (RemoveConnection(conn_id))
    {
        CHIP_ERROR disconReason;
        switch (disc_cause)
        {
        case 0x13:
        case 0x08:
            disconReason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;
        case 0x16:
            disconReason = BLE_ERROR_APP_CLOSED_CONNECTION;
            break;
        default:
            disconReason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
            break;
        }
        HandleConnectionError(conn_id, disconReason);
    }

    // Force a reconfiguration of advertising in case we switched to non-connectable mode when
    // the BLE connection was established.
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    return CHIP_NO_ERROR;
}

bool BLEManagerImpl::RemoveConnection(uint8_t connectionHandle)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(connectionHandle);
    bool status                     = false;

    if (bleConnState != NULL)
    {
        bleConnState->Reset();
        memset(bleConnState, 0, sizeof(CHIPoBLEConState));
        status = true;
    }
    return status;
}

BLEManagerImpl::CHIPoBLEConState * BLEManagerImpl::GetConnectionState(uint8_t connectionHandle, bool allocate)
{
    uint8_t freeIndex = kMaxConnections;

    for (uint8_t i = 0; i < kMaxConnections; i++)
    {
        if (mBleConnections[i].allocated == 1)
        {
            if (mBleConnections[i].conn_idx == connectionHandle)
            {
                return &mBleConnections[i];
            }
        }

        else if (i < freeIndex)
        {
            freeIndex = i;
        }
    }

    if (allocate)
    {
        if (freeIndex < kMaxConnections)
        {
            mBleConnections[freeIndex].Set(connectionHandle);
            return &mBleConnections[freeIndex];
        }

        ChipLogError(DeviceLayer, "Failed to allocate CHIPoBLEConState");
    }

    return NULL;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (val != mServiceMode)
    {
        mServiceMode = val;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (mFlags.Has(Flags::kAdvertisingEnabled) != val)
    {
        mFlags.Set(Flags::kAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    switch (mode)
    {
    case BLEAdvertisingMode::kFastAdvertising:
        mFlags.Set(Flags::kFastAdvertisingEnabled, true);
        break;
    case BLEAdvertisingMode::kSlowAdvertising:
        mFlags.Set(Flags::kFastAdvertisingEnabled, false);
        break;
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    if (strlen(mDeviceName) >= bufSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    strcpy(buf, mDeviceName);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (deviceName != NULL && deviceName[0] != 0)
    {
        VerifyOrExit(strlen(deviceName) >= kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);
        strcpy(mDeviceName, deviceName);
        mFlags.Set(Flags::kDeviceNameSet);
        ChipLogProgress(DeviceLayer, "Setting device name to : \"%s\"", deviceName);
    }
    else
    {
        mDeviceName[0] = 0;
        mFlags.Clear(Flags::kDeviceNameSet);
    }

exit:
    return err;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    // Platform specific events
    case DeviceEventType::kCHIPoBLESubscribe:
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        {
            ChipDeviceEvent connEstEvent;
            connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
            PlatformMgr().PostEventOrDie(&connEstEvent);
        }
        break;

    case DeviceEventType::kCHIPoBLEUnsubscribe: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEUnsubscribe");
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kCHIPoBLEWriteReceived: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEWriteReceived");
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
    }
    break;

    case DeviceEventType::kCHIPoBLEConnectionError: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEConnectionError");
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
    }
    break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEIndicateConfirm");
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kServiceProvisioningChange:
    case DeviceEventType::kWiFiConnectivityChange:
        ChipLogProgress(DeviceLayer, "Updating advertising data");
        StartAdvertising();
        break;

    default:
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent default:  event->Type = %d", event->Type);
        break;
    }
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    CHIP_ERROR err;
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);
    uint32_t delay_time = 20;
    int ret;

    do
    {
        ret = bk_ble_disconnect(conId, beken_ble_cmd_cb);
        if (BK_ERR_BLE_SUCCESS == ret)
        {
            break;
        }

        rtos_delay_milliseconds(10);
        delay_time--;
    } while (delay_time);
    // Beken Ble close function
    err = MapBLEError(ret);
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "bk_ble_disconnect() failed: %s", ErrorStr(err));
    }

    return false;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    CHIPoBLEConState * conState = const_cast<BLEManagerImpl *>(this)->GetConnectionState(conId);
    return (conState != NULL) ? conState->mtu : 0;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBufferHandle pBuf)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     PacketBufferHandle pBuf)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SendReadRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "BLEManagerImpl::SendReadResponse() not supported");
    return false;
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // Nothing to do
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int ret;

    VerifyOrExit(IsSubscribed(conId), err = CHIP_ERROR_INVALID_ARGUMENT);
    ret = bk_ble_send_noti_value(conId, data->DataLength(), data->Start(), 0, SVR_FFF6_TX_VALUE);
    err = MapBLEError(ret);
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManagerImpl::SendIndication() failed: %s", ErrorStr(err));
        return false;
    }

    return true;
}

/*******************************************************************************
 * Private functions
 *******************************************************************************/

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // If the device name is not specified, generate a CHIP-standard name based on the bottom digits of the Chip device id.
    uint16_t discriminator;
    SuccessOrExit(err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));

    if (!mFlags.Has(Flags::kDeviceNameSet))
    {
        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
        mDeviceName[kMaxDeviceNameLength] = 0;
    }

    // Configure the BLE device name.
    sInstance.mFlags.Set(Flags::kDeviceNameDefSet);
    bk_ble_appm_set_dev_name(kMaxDeviceNameLength, (uint8_t *) mDeviceName);
exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    ble_err_t bk_err;
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(DeviceLayer, "StartAdvertising...");
    ChipLogProgress(DeviceLayer, "BLE flag  = %x", mFlags.Raw());
    if ((!mFlags.Has(Flags::kDeviceNameSet)) && (!mFlags.Has(Flags::kDeviceNameDefSet)))
    {
        err = sInstance.ConfigureAdvertisingData();
        SuccessOrExit(err);
    }
    // Post a CHIPoBLEAdvertisingChange(Stopped) event.
    if (mFlags.Has(Flags::kBEKENBLEADVStop))
    {
        bk_ble_delete_advertising(adv_actv_idx, beken_ble_cmd_cb);
        mFlags.Clear(Flags::kBEKENBLEADVStop);
        ChipDeviceEvent advChange;
        advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
        advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
        PlatformMgr().PostEventOrDie(&advChange);
        goto exit;
    }
    else if (!mFlags.Has(Flags::kBEKENBLEADVCreate))
    {
        ChipLogProgress(DeviceLayer, "BLE ADVCreate...");
        uint16_t adv_int_min;
        uint16_t adv_int_max;
        ble_adv_param_t adv_param;

        memset(&adv_param, 0, sizeof(adv_param));

        if (mFlags.Has(Flags::kFastAdvertisingEnabled))
        {
            adv_int_min = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
            adv_int_max = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
            mFlags.Set(Flags::kAdvertisingIsFastADV);
        }
        else
        {
            adv_int_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
            adv_int_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
            mFlags.Clear(Flags::kAdvertisingIsFastADV);
        }

        adv_actv_idx = bk_ble_get_idle_actv_idx_handle();
        if (UNKNOW_ACT_IDX == adv_actv_idx)
        {
            ChipLogError(DeviceLayer, "adv act-idx:%d error", adv_actv_idx);
            return MapBLEError(-1);
        }
        adv_param.chnl_map      = 7;
        adv_param.adv_intv_max  = 160;
        adv_param.adv_intv_min  = 160;
        adv_param.own_addr_type = 0;
        adv_param.adv_type      = 0;
        adv_param.adv_prop      = 3;
        adv_param.prim_phy      = 1;
        // bk_ble_create_advertising(adv_actv_idx, 7, adv_int_min, adv_int_max, beken_ble_cmd_cb);
        bk_err = bk_ble_create_advertising(adv_actv_idx, &adv_param, beken_ble_cmd_cb);
        if (bk_err != BK_OK)
        {
            ChipLogProgress(DeviceLayer, "BLE ADVCreate ret= %x.", bk_err);
        }
        ChipLogProgress(DeviceLayer, "BLE ADVCreate ret= %x. adv_actv_idx = %x", bk_err, adv_actv_idx);
        sInstance.mFlags.Clear(Flags::kAdvertisingRefreshNeeded);
        goto exit;
    }
    else if (!mFlags.Has(Flags::kBEKENBLEADVSetData))
    {
        ChipLogProgress(DeviceLayer, "BLE SetData...");
        ChipBLEDeviceIdentificationInfo deviceIdInfo;
        uint8_t advData[MAX_ADV_DATA_LEN] = { 0 };
        uint8_t index                     = 0;
        /**************** Prepare advertising data *******************************************/
        memset(advData, 0, sizeof(advData));
        advData[index++] = 0x02;                     // length
        advData[index++] = CHIP_ADV_DATA_TYPE_FLAGS; // AD type : flags
        advData[index++] = CHIP_ADV_DATA_FLAGS;      // AD value
        advData[index++] = static_cast<uint8_t>(sizeof(deviceIdInfo) + CHIP_ADV_SHORT_UUID_LEN + 1);
        ;                                                             // length
        advData[index++] = 0x16;                                      // AD type: (Service Data - 16-bit UUID)
        advData[index++] = static_cast<uint8_t>(_svc_uuid[0] & 0xFF); // AD value
        advData[index++] = static_cast<uint8_t>(_svc_uuid[1] & 0xFF); // AD value
        err              = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "GetBLEDeviceIdentificationInfo(): %s", ErrorStr(err));
            ExitNow();
        }

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
        deviceIdInfo.SetAdditionalDataFlag(true);
#endif
        VerifyOrExit(index + sizeof(deviceIdInfo) <= sizeof(advData), err = CHIP_ERROR_OUTBOUND_MESSAGE_TOO_BIG);

        memcpy(&advData[index], &deviceIdInfo, sizeof(deviceIdInfo));
        index = static_cast<uint8_t>(index + sizeof(deviceIdInfo));
        bk_ble_set_adv_data(adv_actv_idx, advData, index, beken_ble_cmd_cb);
        goto exit;
    }
    else if (!mFlags.Has(Flags::kBEKENBLEADVSetRsp))
    {
        /// uint8_t advData[MAX_ADV_DATA_LEN]    = { 0 };
        /// bk_ble_set_scan_rsp_data(adv_actv_idx, advData, 0xF, beken_ble_cmd_cb);
        sInstance.mFlags.Set(Flags::kBEKENBLEADVSetRsp);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        goto exit;
    }
    else if (!mFlags.Has(Flags::kBEKENBLEADVStarted))
    {
        ChipLogProgress(DeviceLayer, "BLE Started...");
        bk_ble_start_advertising(adv_actv_idx, 0, beken_ble_cmd_cb);
        goto exit;
    }

    if (mFlags.Has(Flags::kBEKENBLEADVStarted))
    {
        ChipDeviceEvent advChange;
        advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
        advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Started;
        PlatformMgr().PostEventOrDie(&advChange);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    // Change flag status to the 'not Advertising state'
    if ((mFlags.Has(Flags::kBEKENBLEADVStarted)) && (!mFlags.Has(Flags::kBEKENBLEADVStop)))
    {
        mFlags.Set(Flags::kBEKENBLEADVStop);
        bk_ble_stop_advertising(adv_actv_idx, beken_ble_cmd_cb);
        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopping");
        goto exit;
    }

    // Post a CHIPoBLEAdvertisingChange(Stopped) event.
    if (mFlags.Has(Flags::kBEKENBLEADVStop))
    {
        bk_ble_delete_advertising(adv_actv_idx, beken_ble_cmd_cb);
        mFlags.Clear(Flags::kBEKENBLEADVStop);
        ChipDeviceEvent advChange;
        advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
        advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
        PlatformMgr().PostEventOrDie(&advChange);
    }
exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::MapBLEError(int bleErr)
{
    switch (bleErr)
    {
    case BK_ERR_BLE_SUCCESS:
        return CHIP_NO_ERROR;
    default:
        return CHIP_ERROR_INCORRECT_STATE;
    }
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if BLE stack is initialized
    VerifyOrExit(mFlags.Has(Flags::kAMEBABLEStackInitialized), /* */);
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled)
    {
        if (!mFlags.Has(Flags::kBekenBLESGATTSReady))
        {
            beken_ble_init();
            goto exit;
        }
    }

    if ((!mFlags.Has(Flags::kFastAdvertisingEnabled)) && (!mFlags.Has(Flags::kSlowAdvertisingEnabled)))
    {
        mFlags.Clear(Flags::kAdvertisingEnabled);
        mFlags.Clear(Flags::kAdvertisingRefreshNeeded);
    }

    if (mFlags.Has(Flags::kAdvertisingRefreshNeeded))
    {
        if (mFlags.Has(Flags::kBEKENBLEADVStarted))
        {
            err = StopAdvertising();
            SuccessOrExit(err);
            goto exit;
        }
        else if (mFlags.Has(Flags::kBEKENBLEADVStarted))
        {
            bk_ble_delete_advertising(adv_actv_idx, beken_ble_cmd_cb);
            goto exit;
        }
    }

    // Start advertising if needed...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled &&
        ((mFlags.Has(Flags::kAdvertisingEnabled)) || (mFlags.Has(Flags::kAdvertisingRefreshNeeded))))
    {
        // Start/re-start advertising if not already started, or if there is a pending change
        // to the advertising configuration.
        if (!mFlags.Has(Flags::kBEKENBLEADVStarted))
        {
            err = StartAdvertising();
            SuccessOrExit(err);
        }
        else
        {
            mFlags.Clear(Flags::kAdvertisingRefreshNeeded);
        }
    }
    // Otherwise, stop advertising if it is enabled.
    else if ((mFlags.Has(Flags::kBEKENBLEADVStarted)) || (mFlags.Has(Flags::kBEKENBLEADVStop)))
    {
        err = StopAdvertising();
        SuccessOrExit(err);
        ChipLogProgress(DeviceLayer, "Stopped Advertising");
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

void BLEManagerImpl::DriveBLEExtPerf(intptr_t arg)
{
    int arg_int = static_cast<int>(arg);
    int event   = arg_int & 0xFFU;
    switch (event)
    {
    case DriveBLEExtPerfEvt_DISCONNECT: {
        uint8_t conn_indx = (arg_int >> 8) & 0xFFU;
        sInstance.CloseConnection(conn_indx);
    }
    break;
    default:
        break;
    }
}

/*******************************************************************************
 * FreeRTOS Task Management Functions
 *******************************************************************************/
void BLEManagerImpl::ble_adv_timer_timeout_handle(TimerHandle_t xTimer)
{
    if (sInstance.mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Stop Fast advertisement");

        sInstance.mFlags.Clear(Flags::kFastAdvertisingEnabled);
        // Stop advertising, change interval and restart it;
        sInstance.StopAdvertising();
    }
    else if (sInstance._IsAdvertisingEnabled())
    {
        // Advertisement time expired. Stop advertising
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Stop slow advertisement");
        sInstance.mFlags.Clear(Flags::kSlowAdvertisingEnabled);
        sInstance.StopAdvertising();
    }
}

void BLEManagerImpl::CancelBleAdvTimeoutTimer(void)
{
    if (xTimerIsTimerActive(bleFastAdvTimer))
    {
        if (xTimerStop(bleFastAdvTimer, 0) == pdFAIL)
        {
            ChipLogError(DeviceLayer, "Failed to stop BledAdv timeout timer");
            sInstance.mFlags.Clear(Flags::kBEKENBLEAdvTimerRun);
        }
    }
}

void BLEManagerImpl::StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs)
{
    CancelBleAdvTimeoutTimer();
    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(bleFastAdvTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        ChipLogError(DeviceLayer, "Failed to start BledAdv timeout timer");
    }
    else
    {
        xTimerStart(bleFastAdvTimer, BEKEN_WAIT_FOREVER);
        sInstance.mFlags.Set(Flags::kBEKENBLEAdvTimerRun);
    }
}

CHIP_ERROR BLEManagerImpl::SetSubscribed(uint16_t conId)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(conId, false);

    if (bleConnState != NULL)
    {
        bleConnState->subscribed = 1;
        return CHIP_NO_ERROR;
    }

    return CHIP_ERROR_NO_MEMORY;
}

bool BLEManagerImpl::UnsetSubscribed(uint16_t conId)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(conId, false);

    if (bleConnState != NULL)
    {
        bleConnState->subscribed = 0;
        return true;
    }

    return false;
}

bool BLEManagerImpl::IsSubscribed(uint16_t conId)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(conId, false);

    if (bleConnState != NULL)
    {
        if (bleConnState->subscribed)
        {
            return true;
        }
    }
    return false;
}

void BLEManagerImpl::HandleRXCharWrite(uint8_t * p_value, uint16_t len, uint8_t conn_id)
{
    CHIP_ERROR err         = CHIP_NO_ERROR;
    PacketBufferHandle buf = System::PacketBufferHandle::New(len, 0);
    memcpy(buf->Start(), p_value, len);
    buf->SetDataLength(len);

    // Post an event to the Chip queue to deliver the data into the Chip stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = (uint16_t) conn_id;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        PlatformMgr().PostEventOrDie(&event);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleTXCharConfirm(CHIPoBLEConState * conState, int status)
{
    // If the confirmation was successful...
    if (status == 0)
    {
        // Post an event to the Chip queue to process the indicate confirmation.
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
        event.CHIPoBLEIndicateConfirm.ConId = conState->conn_idx;
        PlatformMgr().PostEventOrDie(&event);
    }
    else
    {
        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId  = conState->conn_idx;
        event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::beken_ble_cmd_cb(ble_cmd_t cmd, ble_cmd_param_t * param)
{
    ChipLogProgress(DeviceLayer, "cmd:%d idx:%d status:%d\r\n", cmd, param->cmd_idx, param->status);
    switch (cmd)
    {
    case BLE_CREATE_ADV:
        sInstance.mFlags.Set(Flags::kBEKENBLEADVCreate);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    case BLE_SET_ADV_DATA:
        sInstance.mFlags.Set(Flags::kBEKENBLEADVSetData);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    case BLE_SET_RSP_DATA:
        sInstance.mFlags.Set(Flags::kBEKENBLEADVSetRsp);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    case BLE_START_ADV: {
        uint32_t bleAdvTimeoutMs;
        sInstance.mFlags.Set(Flags::kBEKENBLEADVStarted);
        // if (sInstance.mFlags.Has(Flags::kAdvertisingIsFastADV)){
        bleAdvTimeoutMs = CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME;
        //}else{
        //    bleAdvTimeoutMs = CHIP_DEVICE_CONFIG_BLE_ADVERTISING_TIMEOUT;
        //}
        StartBleAdvTimeoutTimer(bleAdvTimeoutMs);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
    break;
    case BLE_STOP_ADV:
        sInstance.mFlags.Clear(Flags::kBEKENBLEADVStarted);
        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");
        CancelBleAdvTimeoutTimer();
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    case BLE_DELETE_ADV:
        sInstance.mFlags.Clear(Flags::kBEKENBLEADVCreate);
        sInstance.mFlags.Clear(Flags::kBEKENBLEADVSetData);
        sInstance.mFlags.Clear(Flags::kBEKENBLEADVSetRsp);
        sInstance.mFlags.Clear(Flags::kBEKENBLEADVStarted);
        sInstance.adv_actv_idx = kUnusedIndex;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    case BLE_CONN_DIS_CONN:
        break;
    default:
        break;
    }
}

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
void BLEManagerImpl::HandleC3CharRead(void * param)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    chip::System::PacketBufferHandle bufferHandle;
    ble_read_req_t * r_req = (ble_read_req_t *) param;

    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;
    BitFlags<AdditionalDataFields> additionalDataFields;

#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    uint8_t rotatingDeviceIdUniqueId[ConfigurationManager::kRotatingDeviceIDUniqueIDLength] = {};
    MutableByteSpan rotatingDeviceIdUniqueIdSpan(rotatingDeviceIdUniqueId);

    err = DeviceLayer::GetDeviceInstanceInfoProvider()->GetRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueIdSpan);
    SuccessOrExit(err);
    err = ConfigurationMgr().GetLifetimeCounter(additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter);
    SuccessOrExit(err);
    additionalDataPayloadParams.rotatingDeviceIdUniqueId = rotatingDeviceIdUniqueIdSpan;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);
#endif /* CHIP_ENABLE_ROTATING_DEVICE_ID */

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(additionalDataPayloadParams, bufferHandle,
                                                                         additionalDataFields);
    SuccessOrExit(err);
    if (r_req->value == NULL)
    {
        ChipLogError(DeviceLayer, "param->value == NULL");
        return;
    }
    memcpy(r_req->value, bufferHandle->Start(), bufferHandle->DataLength());
    r_req->length = bufferHandle->DataLength();
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to generate TLV encoded Additional Data");
    }
    return;
}
#endif

void BLEManagerImpl::ble_event_notice(ble_notice_t notice, void * param)
{
    /// BLEManagerImpl * blemgr = static_cast<BLEManagerImpl *>(param);
    static int conn_idx = 0; // TODO. will improve this when driver notify the status and channel to app.
    switch (notice)
    {
    case BLE_5_STACK_OK: {
        ChipLogProgress(DeviceLayer, "ble stack ok");
        sInstance.mFlags.Set(Flags::kAMEBABLEStackInitialized);
    }
    break;
    case BLE_5_WRITE_EVENT: {
        ble_write_req_t * w_req = (ble_write_req_t *) param;
        ChipLogProgress(DeviceLayer, "write_cb:conn_idx:%d, prf_id:%d, add_id:%d, len:%d, data[0]:%02x\r\n", w_req->conn_idx,
                        w_req->prf_id, w_req->att_idx, w_req->len, w_req->value[0]);
        if (w_req->att_idx == SVR_FFF6_TX_VALUE)
        {
            sInstance.HandleRXCharWrite((uint8_t *) &w_req->value[0], w_req->len, w_req->conn_idx);
        }
        else if (w_req->att_idx == SVR_FFF6_TX_CFG)
        {
            int notificationsEnabled = w_req->value[0] | (w_req->value[0] << 8);
            sInstance.HandleTXCharCCCDWrite(w_req->conn_idx, notificationsEnabled);
        }
        else if (w_req->att_idx == SVR_FFF6_RX_VALUE)
        {
            sInstance.HandleRXCharWrite((uint8_t *) &w_req->value[0], w_req->len, w_req->conn_idx);
        }
        break;
    }
    case BLE_5_READ_EVENT: {
        ble_read_req_t * r_req = (ble_read_req_t *) param;
        ChipLogProgress(DeviceLayer, "read_cb:conn_idx:%d, prf_id:%d, add_id:%d\r\n", r_req->conn_idx, r_req->prf_id,
                        r_req->att_idx);
        if (r_req->att_idx == SVR_FFF6_RX_VALUE)
        {
            sInstance.HandleTXCharRead(param);
        }
        if (r_req->att_idx == SVR_FFF6_TX_CFG)
        {
            sInstance.HandleTXCharCCCDRead(param);
        }
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
        if (r_req->att_idx == SVR_FFF6_C3_VALUE)
        {
            ChipLogProgress(DeviceLayer, "SVR_FFF6_OPT_VALUE!!!!\r\n");
            sInstance.HandleTXCharRead((void *) param);
        }
#endif
        break;
    }
    case BLE_5_TX_DONE: {
        ChipLogProgress(DeviceLayer, "BLE_5_TX_DONE");
#if 0
        tx_done_rsp_t* txd_rsp = (tx_done_rsp_t*)param;
        if(txd_rsp) {
            CHIPoBLEConState * conState = sInstance.GetConnectionState(txd_rsp->conn_idx,false);
            if (conState != NULL)
            {
                sInstance.HandleTXCharConfirm(conState, txd_rsp->status);
            }
        }
#endif
        CHIPoBLEConState * conState = sInstance.GetConnectionState(conn_idx, false);
        if (conState != NULL)
        {
            sInstance.HandleTXCharConfirm(conState, 0);
        }
    }
    break;
    case BLE_5_MTU_CHANGE: {
        ble_mtu_change_t * m_ind = (ble_mtu_change_t *) param;
        ChipLogProgress(DeviceLayer, "m_ind:conn_idx:%d, mtu_size:%d\r\n", m_ind->conn_idx, m_ind->mtu_size);
        CHIPoBLEConState * conState = sInstance.GetConnectionState(m_ind->conn_idx);
        if (conState != NULL)
        {
            conState->mtu = (m_ind->mtu_size >= (1 << 10)) ? ((1 << 10) - 1) : m_ind->mtu_size;
        }
        break;
    }
    case BLE_5_CONNECT_EVENT: {
        ble_conn_ind_t * c_ind = (ble_conn_ind_t *) param;
        ChipLogProgress(DeviceLayer, "BLE GATT connection established (con %u)", c_ind->conn_idx);
        ChipLogProgress(DeviceLayer, "c_ind:conn_idx:%d, addr_type:%d, peer_addr:%02x:%02x:%02x:%02x:%02x:%02x\r\n",
                        c_ind->conn_idx, c_ind->peer_addr_type, c_ind->peer_addr[0], c_ind->peer_addr[1], c_ind->peer_addr[2],
                        c_ind->peer_addr[3], c_ind->peer_addr[4], c_ind->peer_addr[5]);
        sInstance.mFlags.Clear(Flags::kBEKENBLEADVStarted);
        CHIPoBLEConState * bleConnState = sInstance.GetConnectionState(c_ind->conn_idx, true);
        if (bleConnState == NULL)
        {
            ChipLogError(DeviceLayer, "BLE_5_CONNECT_EVENT failed");
            int ext_evt = DriveBLEExtPerfEvt_DISCONNECT | (c_ind->conn_idx << 8);
            PlatformMgr().ScheduleWork(DriveBLEExtPerf, ext_evt);
        }
        conn_idx = c_ind->conn_idx;
        break;
    }
    case BLE_5_DISCONNECT_EVENT: {
        ble_discon_ind_t * d_ind = (ble_discon_ind_t *) param;
        ChipLogProgress(DeviceLayer, "d_ind:conn_idx:%d,reason:%d\r\n", d_ind->conn_idx, d_ind->reason);
        sInstance.HandleGAPDisconnect(d_ind->conn_idx, d_ind->reason);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    }
    case BLE_5_ATT_INFO_REQ: {
        ble_att_info_req_t * a_ind = (ble_att_info_req_t *) param;
        ChipLogProgress(DeviceLayer, "a_ind:conn_idx:%d\r\n", a_ind->conn_idx);
        if (SVR_FFF6_RX_VALUE == a_ind->att_idx)
        {
            a_ind->length = 512;
            a_ind->status = BK_ERR_BLE_SUCCESS;
        }
        else if (SVR_FFF6_TX_CFG == a_ind->att_idx)
        {
            a_ind->length = 2;
            a_ind->status = BK_ERR_BLE_SUCCESS;
        }
        break;
    }
    case BLE_5_CREATE_DB: {
        ble_create_db_t * cd_ind = (ble_create_db_t *) param;
        ChipLogProgress(DeviceLayer, "cd_ind:prf_id:%d, status:%d\r\n", cd_ind->prf_id, cd_ind->status);
        sInstance.mFlags.Set(Flags::kBekenBLESGATTSReady);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
        break;
    }
    default: {
        ChipLogProgress(DeviceLayer, "Unhandled event:%x", notice);
    }
    break;
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
