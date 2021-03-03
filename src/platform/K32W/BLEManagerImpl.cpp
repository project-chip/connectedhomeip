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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <crypto/CHIPCryptoPAL.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/CHIPBleServiceData.h>

#include "board.h"
#include "fsl_xcvr.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"
#include "stdio.h"

/*******************************************************************************
 * Local data types
 *******************************************************************************/
extern "C" bool_t Ble_ConfigureHostStackConfig(void);
extern "C" void (*pfBLE_SignalFromISR)(void);
extern "C" bool_t Ble_CheckMemoryStorage(void);

extern osaEventId_t gHost_TaskEvent;
extern msgQueue_t gApp2Host_TaskQueue;
extern msgQueue_t gHci2Host_TaskQueue;

using namespace ::chip;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

/*******************************************************************************
 * Macros & Constants definitions
 *******************************************************************************/
/* Timeout of BLE commands */
#define CHIP_BLE_KW_EVNT_TIMEOUT 300

/** BLE advertisment state changed */
#define CHIP_BLE_KW_EVNT_ADV_CHANGED 0x0001
/** BLE advertisement command failed */
#define CHIP_BLE_KW_EVNT_ADV_FAILED 0x0002
/** BLE advertisement setup failed */
#define CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED 0x0004
/** BLE advertisement parameters setup complete */
#define CHIP_BLE_KW_EVNT_ADV_PAR_SETUP_COMPLETE 0x0008
/** BLE advertisement data setup complete */
#define CHIP_BLE_KW_EVNT_ADV_DAT_SETUP_COMPLETE 0x0010
/** BLE random address set */
#define CHIP_BLE_KW_EVNT_RND_ADDR_SET 0x0020
/** BLE Initialization complete */
#define CHIP_BLE_KW_EVNT_INIT_COMPLETE 0x0040
/** BLE Received a handle value confirmation from the client */
#define CHIP_BLE_KW_EVNT_INDICATION_CONFIRMED 0x0080
/** BLE send indication failed */
#define CHIP_BLE_KW_EVNT_INDICATION_FAILED 0x0100
/** Maximal time of connection without activity */
#define CHIP_BLE_KW_CONN_TIMEOUT 60000

#define LOOP_EV_BLE (0x08)

/* controller task configuration */
#define CONTROLLER_TASK_PRIORITY (6U)
#define CONTROLLER_TASK_STACK_SIZE (gControllerTaskStackSize_c / sizeof(StackType_t))

/* host task configuration */
#define HOST_TASK_PRIORITY (3U)
#define HOST_TASK_STACK_SIZE (gHost_TaskStackSize_c / sizeof(StackType_t))

/* ble app task configuration */
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_PRIORITY (HOST_TASK_PRIORITY - 1)
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE (1024)

/* advertising configuration */
#define BLEKW_ADV_MAX_NO (2)
#define BLEKW_SCAN_RSP_MAX_NO (2)
#define BLEKW_MAX_ADV_DATA_LEN (31)
#define CHIP_ADV_SHORT_UUID_LEN (2)

/* Message list used to synchronize asynchronous messages from the KW BLE tasks */
anchor_t blekw_msg_list;

/* Used to manage asynchronous events from BLE Stack: e.g.: GAP setup finished */
osaEventId_t event_msg;

osaEventId_t mControllerTaskEvent;
TimerHandle_t connectionTimeout;

/* Used by BLE App Task to handle asynchronous GATT events */
EventGroupHandle_t bleAppTaskLoopEvent;

/* keep the device ID of the connected peer */
uint8_t device_id;

const uint8_t ShortUUID_CHIPoBLEService[]  = { 0xAF, 0xFE };
const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };
} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    osaEventFlags_t flags;
    BaseType_t bleAppCreated    = errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY;
    uint16_t attChipRxHandle[1] = { (uint16_t) value_chipoble_rx };

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;

    // Check if BLE stack is initialized
    VerifyOrExit(!mFlags.Has(Flags::kK32WBLEStackInitialized), err = CHIP_ERROR_INCORRECT_STATE);

    // Initialize the Chip BleLayer.
    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);

    /* Initialization of message wait events -
     * used for receiving BLE Stack events */
    event_msg = OSA_EventCreate(TRUE);
    VerifyOrExit(event_msg != NULL, err = CHIP_ERROR_INCORRECT_STATE);

    pfBLE_SignalFromISR = BLE_SignalFromISRCallback;

    /* Set the config structure to the host stack */
    VerifyOrExit(Ble_ConfigureHostStackConfig() == TRUE, err = CHIP_ERROR_INCORRECT_STATE);

    /* Prepare callback input queue.*/
    MSG_InitQueue(&blekw_msg_list);

    /* Create the connection timeout timer. */
    connectionTimeout =
        xTimerCreate("bleTimeoutTmr", pdMS_TO_TICKS(CHIP_BLE_KW_CONN_TIMEOUT), pdFALSE, (void *) 0, blekw_connection_timeout_cb);

    /* Create BLE App Task */
    bleAppTaskLoopEvent = xEventGroupCreate();
    VerifyOrExit(bleAppTaskLoopEvent != NULL, err = CHIP_ERROR_INCORRECT_STATE);
    bleAppCreated = xTaskCreate(bleAppTask, CHIP_DEVICE_CONFIG_BLE_APP_TASK_NAME,
                                CHIP_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE / sizeof(StackType_t), this,
                                CHIP_DEVICE_CONFIG_BLE_APP_TASK_PRIORITY, NULL);
    VerifyOrExit(bleAppCreated == pdPASS, err = CHIP_ERROR_INCORRECT_STATE);

    /* BLE Radio Init */
    XCVR_TemperatureUpdate(BOARD_GetTemperature());
    VerifyOrExit(XCVR_Init(BLE_MODE, DR_2MBPS) == gXcvrSuccess_c, err = CHIP_ERROR_INCORRECT_STATE);

    /* Create BLE Controller Task */
    VerifyOrExit(blekw_controller_init() == CHIP_NO_ERROR, err = CHIP_ERROR_INCORRECT_STATE);

    /* Create BLE Host Task */
    VerifyOrExit(blekw_host_init() == CHIP_NO_ERROR, err = CHIP_ERROR_INCORRECT_STATE);

    /* BLE Host Stack Init */
    Ble_HostInitialize(blekw_generic_cb, (hciHostToControllerInterface_t) Hci_SendPacketToController);

    /* Register the GATT server callback */
    VerifyOrExit(GattServer_RegisterCallback(blekw_gatt_server_cb) == gBleSuccess_c, err = CHIP_ERROR_INCORRECT_STATE);

    /* Wait until BLE Stack is ready */
    VerifyOrExit(OSA_EventWait(event_msg, CHIP_BLE_KW_EVNT_INIT_COMPLETE, TRUE, CHIP_BLE_KW_EVNT_TIMEOUT, &flags) ==
                     osaStatus_Success,
                 err = CHIP_ERROR_INCORRECT_STATE);

    GattServer_RegisterHandlesForWriteNotifications(1, attChipRxHandle);

    mFlags.Set(Flags::kK32WBLEStackInitialized);
    mFlags.Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART ? true : false);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    uint16_t numCons = 0;
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mBleConnections[i].allocated)
        {
            numCons++;
        }
    }

    return numCons;
}

bool BLEManagerImpl::_IsAdvertisingEnabled(void)
{
    return mFlags.Has(Flags::kAdvertisingEnabled);
}

bool BLEManagerImpl::_IsFastAdvertisingEnabled(void)
{
    return mFlags.Has(Flags::kFastAdvertisingEnabled);
}

bool BLEManagerImpl::RemoveConnection(uint8_t connectionHandle)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(connectionHandle, true);
    bool status                     = false;

    if (bleConnState != NULL)
    {
        memset(bleConnState, 0, sizeof(CHIPoBLEConState));
        status = true;
    }

    return status;
}

void BLEManagerImpl::AddConnection(uint8_t connectionHandle)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(connectionHandle, true);

    if (bleConnState != NULL)
    {
        memset(bleConnState, 0, sizeof(CHIPoBLEConState));
        bleConnState->allocated        = 1;
        bleConnState->connectionHandle = connectionHandle;
    }
}

BLEManagerImpl::CHIPoBLEConState * BLEManagerImpl::GetConnectionState(uint8_t connectionHandle, bool allocate)
{
    uint8_t freeIndex = kMaxConnections;

    for (uint8_t i = 0; i < kMaxConnections; i++)
    {
        if (mBleConnections[i].allocated == 1)
        {
            if (mBleConnections[i].connectionHandle == connectionHandle)
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

CHIP_ERROR BLEManagerImpl::_SetFastAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (mFlags.Has(Flags::kFastAdvertisingEnabled) != val)
    {
        mFlags.Set(Flags::kFastAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
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
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    if (deviceName != NULL && deviceName[0] != 0)
    {
        if (strlen(deviceName) >= kMaxDeviceNameLength)
        {
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        memset(mDeviceName, 0, kMaxDeviceNameLength);
        strcpy(mDeviceName, deviceName);
        mFlags.Set(Flags::kDeviceNameSet);
        ChipLogProgress(DeviceLayer, "Setting device name to : \"%s\"", deviceName);
    }
    else
    {
        mDeviceName[0] = 0;
        mFlags.Set(Flags::kRestartAdvertising);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

void BLEManagerImpl::HandleWriteEvent(blekw_msg_t * msg)
{
    blekw_att_written_data_t * att_wr_data = (blekw_att_written_data_t *) msg->data.data;
    attErrorCode_t status                  = gAttErrCodeNoError_c;

#if CHIP_DEVICE_CHIP0BLE_DEBUG
    ChipLogProgress(DeviceLayer, "Attribute write request(device: %d,handle: %d).", att_wr_data->device_id, att_wr_data->handle);
#endif

    blekw_start_connection_timeout();

    if (value_chipoble_rx == att_wr_data->handle)
    {
        sInstance.HandleRXCharWrite(msg);
    }
    else if (cccd_chipoble_tx == att_wr_data->handle)
    {
        sInstance.HandleTXCharCCCDWrite(msg);
    }

    /* TODO: do we need to send the status also for CCCD_WRITTEN? */
    if (msg->type != BLE_KW_MSG_ATT_CCCD_WRITTEN)
    {
        bleResult_t res = GattServer_SendAttributeWrittenStatus(att_wr_data->device_id, att_wr_data->handle, status);

        if (res != gBleSuccess_c)
        {
            ChipLogProgress(DeviceLayer, "GattServer_SendAttributeWrittenStatus returned %d", res);
        }
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(blekw_msg_t * msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    CHIPoBLEConState * bleConnState;
    bool indicationsEnabled;
    ChipDeviceEvent event;
    blekw_att_written_data_t * att_wr_data = (blekw_att_written_data_t *) msg->data.data;
    uint16_t writeLen                      = att_wr_data->length;
    uint8_t * data                         = att_wr_data->data;

    VerifyOrExit(writeLen != 0, err = CHIP_ERROR_INCORRECT_STATE);
    bleConnState = GetConnectionState(att_wr_data->device_id, false);
    VerifyOrExit(bleConnState != NULL, err = CHIP_ERROR_NO_MEMORY);

    /* Determine if the client is enabling or disabling indications.
     * TODO: Check the indications corresponding bit
     */
    indicationsEnabled = (*data);

#if CHIP_DEVICE_CHIP0BLE_DEBUG
    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", indicationsEnabled ? "subscribe" : "unsubscribe");
#endif

    if (indicationsEnabled)
    {
        // If indications are not already enabled for the connection...
        if (!bleConnState->subscribed)
        {
            bleConnState->subscribed = 1;
            /* Post an event to the CHIP queue to process either a CHIPoBLE
             * Subscribe or Unsubscribe based on whether the client
             * is enabling or disabling indications. */
            {
                event.Type                    = DeviceEventType::kCHIPoBLESubscribe;
                event.CHIPoBLESubscribe.ConId = att_wr_data->device_id;
                PlatformMgr().PostEvent(&event);
            }
        }
    }
    else
    {
        bleConnState->subscribed      = 0;
        event.Type                    = DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = att_wr_data->device_id;
        PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleRXCharWrite(blekw_msg_t * msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;
    blekw_att_written_data_t * att_wr_data = (blekw_att_written_data_t *) msg->data.data;
    uint16_t writeLen                      = att_wr_data->length;
    uint8_t * data                         = att_wr_data->data;

    // Copy the data to a PacketBuffer.
    buf = System::PacketBufferHandle::New(writeLen);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    VerifyOrExit(buf->AvailableDataLength() >= writeLen, err = CHIP_ERROR_BUFFER_TOO_SMALL);
    memcpy(buf->Start(), data, writeLen);
    buf->SetDataLength(writeLen);

#if CHIP_DEVICE_CHIP0BLE_DEBUG
    ChipLogDetail(DeviceLayer,
                  "Write request/command received for"
                  "CHIPoBLE RX characteristic (con %" PRIu16 ", len %" PRIu16 ")",
                  att_wr_data->device_id, buf->DataLength());
#endif

    // Post an event to the CHIP queue to deliver the data into the CHIP stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = att_wr_data->device_id;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        PlatformMgr().PostEvent(&event);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
    }
}
/*******************************************************************************
 * BLE stack callbacks
 *******************************************************************************/
void BLEManagerImpl::blekw_generic_cb(gapGenericEvent_t * pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    switch (pGenericEvent->eventType)
    {
    case gInternalError_c:
        /* Notify the CHIP that the BLE hardware report fail */
        ChipLogProgress(DeviceLayer, "BLE Internal Error: Code 0x%04X, Source 0x%08X, HCI OpCode %d.\n",
                        pGenericEvent->eventData.internalError.errorCode, pGenericEvent->eventData.internalError.errorSource,
                        pGenericEvent->eventData.internalError.hciCommandOpcode);
        (void) blekw_msg_add_u8(BLE_KW_MSG_ERROR, BLE_INTERNAL_ERROR);
        break;

    case gAdvertisingSetupFailed_c:
        /* Set the local synchronization event */
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED);
        break;

    case gAdvertisingParametersSetupComplete_c:
        /* Set the local synchronization event */
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_ADV_PAR_SETUP_COMPLETE);
        break;

    case gAdvertisingDataSetupComplete_c:
        /* Set the local synchronization event */
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_ADV_DAT_SETUP_COMPLETE);
        break;

    case gRandomAddressSet_c:
        /* Set the local synchronization event */
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_RND_ADDR_SET);
        break;

    case gInitializationComplete_c:
        /* Common GAP configuration */
        BleConnManager_GapCommonConfig();

        /* Set the local synchronization event */
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_INIT_COMPLETE);
        break;
    default:
        break;
    }
}

void BLEManagerImpl::blekw_gap_advertising_cb(gapAdvertisingEvent_t * pAdvertisingEvent)
{
    if (pAdvertisingEvent->eventType == gAdvertisingStateChanged_c)
    {
        /* Set the local synchronization event */
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_ADV_CHANGED);
    }
    else
    {
        /* The advertisement start failed */
        ChipLogProgress(DeviceLayer, "Advertising failed: event=%d reason=0x%04X\n", pAdvertisingEvent->eventType,
                        pAdvertisingEvent->eventData.failReason);

        /* Set the local synchronization event */
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_ADV_FAILED);
    }
}

void BLEManagerImpl::blekw_gap_connection_cb(deviceId_t deviceId, gapConnectionEvent_t * pConnectionEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GapPeripheralEvent(deviceId, pConnectionEvent);

    if (pConnectionEvent->eventType == gConnEvtConnected_c)
    {
        /* Notify App Task that the BLE is connected now */
        (void) blekw_msg_add_u8(BLE_KW_MSG_CONNECTED, (uint8_t) deviceId);
    }
    else if (pConnectionEvent->eventType == gConnEvtDisconnected_c)
    {
        blekw_stop_connection_timeout();

        /* Notify App Task that the BLE is disconnected now */
        (void) blekw_msg_add_u8(BLE_KW_MSG_DISCONNECTED, (uint8_t) deviceId);
    }
    else if (pConnectionEvent->eventType == gConnEvtPairingRequest_c)
    {
        /* Reject request for pairing */
        Gap_RejectPairing(deviceId, gPairingNotSupported_c);
    }
    else if (pConnectionEvent->eventType == gConnEvtAuthenticationRejected_c)
    {
        ChipLogProgress(DeviceLayer, "BLE Authentication rejected (reason:%d).\n",
                        pConnectionEvent->eventData.authenticationRejectedEvent.rejectReason);
    }
}

/* Called by BLE when a connect is received */
void BLEManagerImpl::BLE_SignalFromISRCallback(void)
{
    /* TODO: Low Power */
}

void BLEManagerImpl::blekw_connection_timeout_cb(TimerHandle_t timer)
{
    (void) blekw_msg_add_u8(BLE_KW_MSG_FORCE_DISCONNECT, 0);
}

void BLEManagerImpl::blekw_start_connection_timeout(void)
{
    xTimerReset(connectionTimeout, 0);
}

void BLEManagerImpl::blekw_stop_connection_timeout(void)
{
    ChipLogProgress(DeviceLayer, "Stopped connectionTimeout timer.");
    xTimerStop(connectionTimeout, 0);
}

void BLEManagerImpl::blekw_gatt_server_cb(deviceId_t deviceId, gattServerEvent_t * pServerEvent)
{
    switch (pServerEvent->eventType)
    {
    case gEvtMtuChanged_c: {
        uint16_t tempMtu = 0;

        (void) Gatt_GetMtu(deviceId, &tempMtu);
        blekw_msg_add_u16(BLE_KW_MSG_MTU_CHANGED, tempMtu);
        break;
    }

    case gEvtAttributeWritten_c:
        blekw_msg_add_att_written(BLE_KW_MSG_ATT_WRITTEN, deviceId, pServerEvent->eventData.attributeWrittenEvent.handle,
                                  pServerEvent->eventData.attributeWrittenEvent.aValue,
                                  pServerEvent->eventData.attributeWrittenEvent.cValueLength);
        break;

    case gEvtLongCharacteristicWritten_c:
        blekw_msg_add_att_written(BLE_KW_MSG_ATT_LONG_WRITTEN, deviceId, pServerEvent->eventData.longCharWrittenEvent.handle,
                                  pServerEvent->eventData.longCharWrittenEvent.aValue,
                                  pServerEvent->eventData.longCharWrittenEvent.cValueLength);
        break;

    case gEvtAttributeRead_c:
        blekw_msg_add_att_read(BLE_KW_MSG_ATT_READ, deviceId, pServerEvent->eventData.attributeReadEvent.handle);
        break;

    case gEvtCharacteristicCccdWritten_c: {
        uint16_t cccd_val = pServerEvent->eventData.charCccdWrittenEvent.newCccd;

        blekw_msg_add_att_written(BLE_KW_MSG_ATT_CCCD_WRITTEN, deviceId, pServerEvent->eventData.charCccdWrittenEvent.handle,
                                  (uint8_t *) &cccd_val, 2);
        break;
    }

    case gEvtHandleValueConfirmation_c:
        /* Set the local synchronization event */
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_INDICATION_CONFIRMED);
        break;

    case gEvtError_c:
        if (pServerEvent->eventData.procedureError.procedureType == gSendIndication_c)
        {
            /* Set the local synchronization event */
            OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_INDICATION_FAILED);
        }
        else
        {
            ChipLogProgress(DeviceLayer, "BLE Gatt Server Error: Code 0x%04X, Source %d.\n",
                            pServerEvent->eventData.procedureError.error, pServerEvent->eventData.procedureError.procedureType);

            /* Notify CHIP BLE App Task that the BLE hardware report fail */
            (void) blekw_msg_add_u8(BLE_KW_MSG_ERROR, BLE_INTERNAL_GATT_ERROR);
        }
        break;

    default:
        break;
    }
}
/*******************************************************************************
 * Add to message queue functions
 *******************************************************************************/
CHIP_ERROR BLEManagerImpl::blekw_msg_add_att_written(blekw_msg_type_t type, uint8_t device_id, uint16_t handle, uint8_t * data,
                                                     uint16_t length)
{
    blekw_msg_t * msg = NULL;
    blekw_att_written_data_t * att_wr_data;

    /* Allocate a buffer with enough space to store the packet */
    msg = (blekw_msg_t *) MSG_Alloc(sizeof(blekw_msg_t) + sizeof(blekw_att_written_data_t) + length);

    if (!msg)
    {
        return CHIP_ERROR_NO_MEMORY;
        assert(0);
    }

    msg->type              = type;
    msg->length            = sizeof(blekw_att_written_data_t) + length;
    att_wr_data            = (blekw_att_written_data_t *) msg->data.data;
    att_wr_data->device_id = device_id;
    att_wr_data->handle    = handle;
    att_wr_data->length    = length;
    FLib_MemCpy(att_wr_data->data, data, length);

    /* Put message in the queue */
    if (gListOk_c != MSG_Queue(&blekw_msg_list, msg))
    {
        assert(0);
    }

    /* Notify BLE-APP Task to serve the BLE subsystem */
    blekw_new_data_received_notification(LOOP_EV_BLE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::blekw_msg_add_att_read(blekw_msg_type_t type, uint8_t device_id, uint16_t handle)
{
    blekw_msg_t * msg = NULL;
    blekw_att_read_data_t * att_rd_data;

    /* Allocate a buffer with enough space to store the packet */
    msg = (blekw_msg_t *) MSG_Alloc(sizeof(blekw_msg_t) + sizeof(blekw_att_read_data_t));

    if (!msg)
    {
        return CHIP_ERROR_NO_MEMORY;
        assert(0);
    }

    msg->type              = type;
    msg->length            = sizeof(blekw_att_read_data_t);
    att_rd_data            = (blekw_att_read_data_t *) msg->data.data;
    att_rd_data->device_id = device_id;
    att_rd_data->handle    = handle;

    /* Put message in the queue */
    if (gListOk_c != MSG_Queue(&blekw_msg_list, msg))
    {
        assert(0);
    }

    /* Notify BLE-APP Task to serve the BLE subsystem */
    blekw_new_data_received_notification(LOOP_EV_BLE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::blekw_msg_add_u8(blekw_msg_type_t type, uint8_t data)
{
    blekw_msg_t * msg = NULL;

    /* Allocate a buffer with enough space to store the packet */
    msg = (blekw_msg_t *) MSG_Alloc(sizeof(blekw_msg_t));

    if (!msg)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    msg->type    = type;
    msg->length  = 0;
    msg->data.u8 = data;

    /* Put message in the queue */
    MSG_Queue(&blekw_msg_list, msg);

    /* Notify BLE-APP Task to serve the BLE subsystem */
    blekw_new_data_received_notification(LOOP_EV_BLE);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::blekw_msg_add_u16(blekw_msg_type_t type, uint16_t data)
{
    blekw_msg_t * msg = NULL;

    /* Allocate a buffer with enough space to store the packet */
    msg = (blekw_msg_t *) MSG_Alloc(sizeof(blekw_msg_t));

    if (!msg)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    msg->type     = type;
    msg->length   = 0;
    msg->data.u16 = data;

    /* Put message in the queue */
    MSG_Queue(&blekw_msg_list, msg);

    /* Notify BLE-APP Task to serve the BLE subsystem */
    blekw_new_data_received_notification(LOOP_EV_BLE);

    return CHIP_NO_ERROR;
}

/*******************************************************************************
 * FreeRTOS Task Management Functions
 *******************************************************************************/
void BLEManagerImpl::blekw_new_data_received_notification(uint32_t mask)
{
    portBASE_TYPE taskToWake = pdFALSE;

    if (__get_IPSR())
    {
        if (xEventGroupSetBitsFromISR(bleAppTaskLoopEvent, mask, &taskToWake) == pdPASS)
        {
            /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
                   switch should be requested.  The macro used is port specific and will
                   be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
                   the documentation page for the port being used. */
            portYIELD_FROM_ISR(taskToWake);
        }
    }
    else
    {
        xEventGroupSetBits(bleAppTaskLoopEvent, mask);
    }
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
