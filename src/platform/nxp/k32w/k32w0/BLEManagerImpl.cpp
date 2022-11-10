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

#include <platform/CommissionableDataProvider.h>

#include <crypto/CHIPCryptoPAL.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/CHIPBleServiceData.h>

#include "board.h"
#include "fsl_xcvr.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"
#include "stdio.h"
#include "timers.h"

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
#include "PWR_Configuration.h"
#endif

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
#define CHIP_BLE_KW_EVNT_TIMEOUT 1000

/** BLE advertisement state changed */
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
#define HOST_TASK_PRIORITY (4U)
#define HOST_TASK_STACK_SIZE (gHost_TaskStackSize_c / sizeof(StackType_t))

/* ble app task configuration */
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_PRIORITY (HOST_TASK_PRIORITY - 1)
#define CHIP_DEVICE_CONFIG_BLE_APP_TASK_STACK_SIZE (1024)

/* advertising configuration */
#define BLEKW_ADV_MAX_NO (2)
#define BLEKW_SCAN_RSP_MAX_NO (2)
#define BLEKW_MAX_ADV_DATA_LEN (31)
#define CHIP_ADV_SHORT_UUID_LEN (2)

/* FreeRTOS sw timer */
TimerHandle_t sbleAdvTimeoutTimer;

/* Message list used to synchronize asynchronous messages from the KW BLE tasks */
anchor_t blekw_msg_list;

/* Used to manage asynchronous events from BLE Stack: e.g.: GAP setup finished */
osaEventId_t event_msg;

osaEventId_t mControllerTaskEvent;
TimerHandle_t connectionTimeout;

/* Used by BLE App Task to handle asynchronous GATT events */
EventGroupHandle_t bleAppTaskLoopEvent;

/* keep the device ID of the connected peer */
uint8_t g_device_id;

const uint8_t ShortUUID_CHIPoBLEService[]  = { 0xF6, 0xFF };
const ChipBleUUID ChipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };
const ChipBleUUID ChipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
static bool bleAppStopInProgress;
#endif
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
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
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

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    PWR_ChangeDeepSleepMode(cPWR_PowerDown_RamRet);
#endif

    GattServer_RegisterHandlesForWriteNotifications(1, attChipRxHandle);

    mFlags.Set(Flags::kK32WBLEStackInitialized);
    mFlags.Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART ? true : false);
    mFlags.Set(Flags::kFastAdvertisingEnabled);

    // Create FreeRTOS sw timer for BLE timeouts and interval change.
    sbleAdvTimeoutTimer = xTimerCreate("BleAdvTimer",       // Just a text name, not used by the RTOS kernel
                                       1,                   // == default timer period (mS)
                                       false,               // no timer reload (==one-shot)
                                       (void *) this,       // init timer id = ble obj context
                                       BleAdvTimeoutHandler // timer callback handler
    );
    VerifyOrExit(sbleAdvTimeoutTimer != NULL, err = CHIP_ERROR_INCORRECT_STATE);
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

bool BLEManagerImpl::_IsAdvertising(void)
{
    return mFlags.Has(Flags::kAdvertising);
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
    mFlags.Set(Flags::kRestartAdvertising);
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
        mFlags.Clear(Flags::kDeviceNameSet);
    }

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe:
        ChipDeviceEvent connEstEvent;

        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
        PlatformMgr().PostEventOrDie(&connEstEvent);
        break;

    case DeviceEventType::kCHIPoBLEUnsubscribe:
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        break;

    case DeviceEventType::kCHIPoBLEWriteReceived:
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
        break;

    case DeviceEventType::kCHIPoBLEConnectionError:
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
        break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm:
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &ChipUUID_CHIPoBLEChar_TX);
        break;

    default:
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

void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT connObj)
{
    BLEMgrImpl().blekw_stop_connection_internal(connObj);
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    return blekw_stop_connection_internal(conId);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    uint16_t tempMtu = 0;
    (void) Gatt_GetMtu(conId, &tempMtu);

    return tempMtu;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBufferHandle pBuf)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     PacketBufferHandle pBuf)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendReadRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendReadResponse() not supported");
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
    uint16_t cId   = (UUIDsMatch(&ChipUUID_CHIPoBLEChar_TX, charId) ? value_chipoble_tx : 0);
    ChipDeviceEvent event;

    if (cId != 0)
    {
        if (blekw_send_event(conId, cId, data->Start(), data->DataLength()) != BLE_OK)
        {
            err = CHIP_ERROR_SENDING_BLOCKED;
        }
        else
        {
            event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
            event.CHIPoBLEIndicateConfirm.ConId = conId;
            err                                 = PlatformMgr().PostEvent(&event);
        }

        if (err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "BLEManagerImpl::SendIndication() failed: %s", ErrorStr(err));
            return false;
        }
        return true;
    }
    return false;
}

BLEManagerImpl::ble_err_t BLEManagerImpl::blekw_send_event(int8_t connection_handle, uint16_t handle, uint8_t * data, uint32_t len)
{
    osaEventFlags_t event_mask;

#if CHIP_DEVICE_CHIP0BLE_DEBUG
    ChipLogProgress(DeviceLayer, "Trying to send event.");
#endif

    if (connection_handle < 0 || handle <= 0)
    {
        ChipLogProgress(DeviceLayer, "BLE Event - Bad Handle");
        return BLE_E_FAIL;
    }

    if (len > 0 && data == NULL)
    {
        ChipLogProgress(DeviceLayer, "BLE Event - Invalid Data");
        return BLE_E_FAIL;
    }

    /************* Send the indication *************/
    if (OSA_EventClear(event_msg, CHIP_BLE_KW_EVNT_INDICATION_CONFIRMED | CHIP_BLE_KW_EVNT_INDICATION_FAILED) != osaStatus_Success)
    {
        ChipLogProgress(DeviceLayer, "BLE Event - Can't clear OSA Events");
        return BLE_E_FAIL;
    }

    if (GattServer_SendInstantValueIndication(connection_handle, handle, len, data) != gBleSuccess_c)
    {
        ChipLogProgress(DeviceLayer, "BLE Event - Can't sent indication");
        return BLE_E_FAIL;
    }

    if (OSA_EventWait(event_msg, CHIP_BLE_KW_EVNT_INDICATION_CONFIRMED | CHIP_BLE_KW_EVNT_INDICATION_FAILED, FALSE,
                      CHIP_BLE_KW_EVNT_TIMEOUT, &event_mask) != osaStatus_Success)
    {
        ChipLogProgress(DeviceLayer, "BLE Event - OSA Event failed");
        return BLE_E_FAIL;
    }

    if (event_mask & CHIP_BLE_KW_EVNT_INDICATION_FAILED)
    {
        ChipLogProgress(DeviceLayer, "BLE Event - Sent Failed");
        return BLE_E_FAIL;
    }

#if CHIP_DEVICE_CHIP0BLE_DEBUG
    ChipLogProgress(DeviceLayer, "BLE Event - Sent :-) ");
#endif

    return BLE_OK;
}
/*******************************************************************************
 * Private functions
 *******************************************************************************/
CHIP_ERROR BLEManagerImpl::blekw_controller_init(void)
{
    mControllerTaskEvent = OSA_EventCreate(TRUE);

    if (!mControllerTaskEvent)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    Controller_TaskEventInit(mControllerTaskEvent, gUseRtos_c);

    /* Task creation */
    if (pdPASS !=
        xTaskCreate(Controller_TaskHandler, "controllerTask", CONTROLLER_TASK_STACK_SIZE, (void *) 0, CONTROLLER_TASK_PRIORITY,
                    NULL))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Setup Interrupt priorities of Interrupt handlers that are used
     * in application to meet requirements of FreeRTOS */

    // BLE_DP_IRQHandler
    NVIC_SetPriority(BLE_DP_IRQn, configMAX_PRIORITIES - 1);
    // BLE_DP0_IRQHandler
    NVIC_SetPriority(BLE_DP0_IRQn, configMAX_PRIORITIES - 1);
    // BLE_DP1_IRQHandler
    NVIC_SetPriority(BLE_DP1_IRQn, configMAX_PRIORITIES - 1);
    // BLE_DP2_IRQHandler
    NVIC_SetPriority(BLE_DP2_IRQn, configMAX_PRIORITIES - 1);
    // BLE_LL_ALL_IRQHandler
    NVIC_SetPriority(BLE_LL_ALL_IRQn, configMAX_PRIORITIES - 1);

    /* Check for available memory storage */
    if (!Ble_CheckMemoryStorage())
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    /* BLE Controller Init */
    if (osaStatus_Success != Controller_Init(Ble_HciRecv))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::Host_Task(osaTaskParam_t argument)
{
    Host_TaskHandler((void *) NULL);
}

CHIP_ERROR BLEManagerImpl::blekw_host_init(void)
{
    /* Initialization of task related */
    gHost_TaskEvent = OSA_EventCreate(TRUE);
    if (!gHost_TaskEvent)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    /* Initialization of task message queue */
    MSG_InitQueue(&gApp2Host_TaskQueue);
    MSG_InitQueue(&gHci2Host_TaskQueue);

    /* Task creation */
    if (pdPASS != xTaskCreate(Host_Task, "hostTask", HOST_TASK_STACK_SIZE, (void *) 0, HOST_TASK_PRIORITY, NULL))
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    return CHIP_NO_ERROR;
}

BLEManagerImpl::ble_err_t BLEManagerImpl::blekw_start_advertising(gapAdvertisingParameters_t * adv_params,
                                                                  gapAdvertisingData_t * adv, gapScanResponseData_t * scnrsp)
{
    osaEventFlags_t event_mask;

    /************* Set the advertising parameters *************/
    OSA_EventClear(event_msg, (CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED | CHIP_BLE_KW_EVNT_ADV_PAR_SETUP_COMPLETE));

    /* Set the advertising parameters */
    if (Gap_SetAdvertisingParameters(adv_params) != gBleSuccess_c)
    {
        vTaskDelay(1);

        /* Retry, just to make sure before giving up and sending an error. */
        if (Gap_SetAdvertisingParameters(adv_params) != gBleSuccess_c)
        {
            return BLE_E_SET_ADV_PARAMS;
        }
    }

    if (OSA_EventWait(event_msg, (CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED | CHIP_BLE_KW_EVNT_ADV_PAR_SETUP_COMPLETE), FALSE,
                      CHIP_BLE_KW_EVNT_TIMEOUT, &event_mask) != osaStatus_Success)
    {
        return BLE_E_ADV_PARAMS_FAILED;
    }

    if (event_mask & CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED)
    {
        return BLE_E_ADV_PARAMS_FAILED;
    }

    /************* Set the advertising data *************/
    OSA_EventClear(event_msg, (CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED | CHIP_BLE_KW_EVNT_ADV_DAT_SETUP_COMPLETE));

    /* Set the advertising data */
    if (Gap_SetAdvertisingData(adv, scnrsp) != gBleSuccess_c)
    {
        return BLE_E_SET_ADV_DATA;
    }

    if (OSA_EventWait(event_msg, (CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED | CHIP_BLE_KW_EVNT_ADV_DAT_SETUP_COMPLETE), FALSE,
                      CHIP_BLE_KW_EVNT_TIMEOUT, &event_mask) != osaStatus_Success)
    {
        return BLE_E_ADV_SETUP_FAILED;
    }

    if (event_mask & CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED)
    {
        return BLE_E_ADV_SETUP_FAILED;
    }

    /************* Start the advertising *************/
    OSA_EventClear(event_msg, (CHIP_BLE_KW_EVNT_ADV_CHANGED | CHIP_BLE_KW_EVNT_ADV_FAILED));

    if (gBleSuccess_c != Gap_CreateRandomDeviceAddress(NULL, NULL))
    {
        return BLE_E_SET_ADV_PARAMS;
    }

    if (OSA_EventWait(event_msg, CHIP_BLE_KW_EVNT_RND_ADDR_SET, FALSE, CHIP_BLE_KW_EVNT_TIMEOUT, &event_mask) != osaStatus_Success)
    {
        return BLE_E_ADV_PARAMS_FAILED;
    }

    /* Start the advertising */
    if (Gap_StartAdvertising(blekw_gap_advertising_cb, blekw_gap_connection_cb) != gBleSuccess_c)
    {
        return BLE_E_START_ADV;
    }

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    PWR_DisallowDeviceToSleep();
#endif

    if (OSA_EventWait(event_msg, (CHIP_BLE_KW_EVNT_ADV_CHANGED | CHIP_BLE_KW_EVNT_ADV_FAILED), FALSE, CHIP_BLE_KW_EVNT_TIMEOUT,
                      &event_mask) != osaStatus_Success)
    {
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
        PWR_AllowDeviceToSleep();
#endif
        return BLE_E_START_ADV_FAILED;
    }

    if (event_mask & CHIP_BLE_KW_EVNT_ADV_FAILED)
    {
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
        PWR_AllowDeviceToSleep();
#endif
        return BLE_E_START_ADV_FAILED;
    }

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    PWR_AllowDeviceToSleep();
#endif

    return BLE_OK;
}

BLEManagerImpl::ble_err_t BLEManagerImpl::blekw_stop_advertising(void)
{
    osaEventFlags_t event_mask;
    bleResult_t res;

    OSA_EventClear(event_msg, (CHIP_BLE_KW_EVNT_ADV_CHANGED | CHIP_BLE_KW_EVNT_ADV_FAILED));

    /* Stop the advertising data */
    res = Gap_StopAdvertising();
    if (res != gBleSuccess_c)
    {
        ChipLogProgress(DeviceLayer, "Failed to stop advertising %d", res);
        return BLE_E_STOP;
    }

    if (OSA_EventWait(event_msg, (CHIP_BLE_KW_EVNT_ADV_CHANGED | CHIP_BLE_KW_EVNT_ADV_FAILED), FALSE, CHIP_BLE_KW_EVNT_TIMEOUT,
                      &event_mask) != osaStatus_Success)
    {
        ChipLogProgress(DeviceLayer, "Stop advertising event timeout.");
        return BLE_E_ADV_CHANGED;
    }

    if (event_mask & CHIP_BLE_KW_EVNT_ADV_FAILED)
    {
        ChipLogProgress(DeviceLayer, "Stop advertising flat out failed.");
        return BLE_E_ADV_FAILED;
    }

    return BLE_OK;
}

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    ble_err_t err;
    CHIP_ERROR chipErr;
    uint16_t discriminator;
    uint16_t advInterval                                  = 0;
    gapAdvertisingData_t adv                              = { 0 };
    gapAdStructure_t adv_data[BLEKW_ADV_MAX_NO]           = { { 0 } };
    gapAdStructure_t scan_rsp_data[BLEKW_SCAN_RSP_MAX_NO] = { { 0 } };
    uint8_t advPayload[BLEKW_MAX_ADV_DATA_LEN]            = { 0 };
    gapScanResponseData_t scanRsp                         = { 0 };
    gapAdvertisingParameters_t adv_params                 = { 0 };
    uint8_t chipAdvDataFlags                              = (gLeGeneralDiscoverableMode_c | gBrEdrNotSupported_c);
    uint8_t chipOverBleService[2];
    ChipBLEDeviceIdentificationInfo mDeviceIdInfo = { 0 };
    uint8_t mDeviceIdInfoLength                   = 0;

    chipErr = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator);
    if (chipErr != CHIP_NO_ERROR)
    {
        return chipErr;
    }

    if (!mFlags.Has(Flags::kDeviceNameSet))
    {
        memset(mDeviceName, 0, kMaxDeviceNameLength);
        snprintf(mDeviceName, kMaxDeviceNameLength, "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);
    }

    /**************** Prepare advertising data *******************************************/
    adv.cNumAdStructures = BLEKW_ADV_MAX_NO;

    chipErr = ConfigurationMgr().GetBLEDeviceIdentificationInfo(mDeviceIdInfo);
    SuccessOrExit(chipErr);
    mDeviceIdInfoLength = sizeof(mDeviceIdInfo);

    if ((mDeviceIdInfoLength + CHIP_ADV_SHORT_UUID_LEN + 1) > BLEKW_MAX_ADV_DATA_LEN)
    {
        return CHIP_ERROR_INCORRECT_STATE;
    }

    adv_data[0].length = 0x02;
    adv_data[0].adType = gAdFlags_c;
    adv_data[0].aData  = (uint8_t *) (&chipAdvDataFlags);

    adv_data[1].length = static_cast<uint8_t>(mDeviceIdInfoLength + CHIP_ADV_SHORT_UUID_LEN + 1);
    adv_data[1].adType = gAdServiceData16bit_c;
    memcpy(advPayload, ShortUUID_CHIPoBLEService, CHIP_ADV_SHORT_UUID_LEN);
    memcpy(&advPayload[CHIP_ADV_SHORT_UUID_LEN], (void *) &mDeviceIdInfo, mDeviceIdInfoLength);
    adv_data[1].aData = advPayload;

    adv.aAdStructures = adv_data;
    /**************** Prepare scan response data *******************************************/
    scanRsp.cNumAdStructures = BLEKW_SCAN_RSP_MAX_NO;

    scan_rsp_data[0].length = static_cast<uint8_t>(strlen(mDeviceName) + 1);
    scan_rsp_data[0].adType = gAdCompleteLocalName_c;
    scan_rsp_data[0].aData  = (uint8_t *) mDeviceName;

    scan_rsp_data[1].length = sizeof(chipOverBleService) + 1;
    scan_rsp_data[1].adType = gAdComplete16bitServiceList_c;
    chipOverBleService[0]   = ShortUUID_CHIPoBLEService[0];
    chipOverBleService[1]   = ShortUUID_CHIPoBLEService[1];
    scan_rsp_data[1].aData  = (uint8_t *) chipOverBleService;

    scanRsp.aAdStructures = scan_rsp_data;

    /**************** Prepare advertising parameters *************************************/
    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        advInterval = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
    }
    else
    {
        advInterval = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
    }

    adv_params.minInterval = adv_params.maxInterval = (uint16_t)(advInterval / 0.625F);
    adv_params.advertisingType                      = gAdvConnectableUndirected_c;
    adv_params.ownAddressType                       = gBleAddrTypePublic_c;
    adv_params.peerAddressType                      = gBleAddrTypePublic_c;
    memset(adv_params.peerAddress, 0, gcBleDeviceAddressSize_c);
    adv_params.channelMap   = (gapAdvertisingChannelMapFlags_t)(gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c);
    adv_params.filterPolicy = gProcessAll_c;

    err = blekw_start_advertising(&adv_params, &adv, &scanRsp);
    if (err == BLE_OK)
    {
        ChipLogProgress(DeviceLayer, "Started Advertising at %d ms", advInterval);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Advertising error 0x%x!", err);
        return CHIP_ERROR_INCORRECT_STATE;
    }

exit:
    return chipErr;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    mFlags.Set(Flags::kAdvertising);
    mFlags.Clear(Flags::kRestartAdvertising);

    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        StartBleAdvTimeoutTimer(CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_TIMEOUT);
    }

    err = ConfigureAdvertisingData();

    if (err == CHIP_NO_ERROR)
    /* schedule NFC emulation stop */
    {
        ChipDeviceEvent advChange;
        advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
        advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Started;
        err                                        = PlatformMgr().PostEvent(&advChange);
    }

    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    ble_err_t err;
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (mFlags.Has(Flags::kAdvertising))
    {
        mFlags.Clear(Flags::kAdvertising);
        mFlags.Clear(Flags::kFastAdvertisingEnabled);
        mFlags.Clear(Flags::kRestartAdvertising);

        err = blekw_stop_advertising();
        if (err != BLE_OK)
        {
            return CHIP_ERROR_INCORRECT_STATE;
        }
        else
        {
            /* schedule NFC emulation stop */
            {
                ChipDeviceEvent advChange;
                advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
                advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
                error                                      = PlatformMgr().PostEvent(&advChange);
            }
        }
    }
    CancelBleAdvTimeoutTimer();

    return error;
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if BLE stack is initialized
    VerifyOrExit(mFlags.Has(Flags::kK32WBLEStackInitialized), /* */);

    // Start advertising if needed...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && mFlags.Has(Flags::kAdvertisingEnabled))
    {
        // Start/re-start advertising if not already started, or if there is a pending change
        // to the advertising configuration.
        if (!mFlags.Has(Flags::kAdvertising) || mFlags.Has(Flags::kRestartAdvertising))
        {
            err = StartAdvertising();
            SuccessOrExit(err);
        }
    }
    // Otherwise, stop advertising if it is enabled.
    else if (mFlags.Has(Flags::kAdvertising))
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

/*******************************************************************************
 * BLE App Task Processing
 *******************************************************************************/
void BLEManagerImpl::bleAppTask(void * p_arg)
{
    while (true)
    {
        xEventGroupWaitBits(bleAppTaskLoopEvent, LOOP_EV_BLE, true, false, portMAX_DELAY);

        if (MSG_Pending(&blekw_msg_list))
        {
            /* There is message from the BLE tasks to solve */
            blekw_msg_t * msg = (blekw_msg_t *) MSG_DeQueue(&blekw_msg_list);

            assert(msg != NULL);

            if (msg->type == BLE_KW_MSG_ERROR)
            {
                if (msg->data.u8 == BLE_KW_MSG_2M_UPGRADE_ERROR)
                {
                    ChipLogProgress(DeviceLayer,
                                    "Warning. BLE is using 1Mbps. Couldn't upgrade to 2Mbps, "
                                    "maybe the peer is missing 2Mbps support.");
                }
                else
                {
                    ChipLogProgress(DeviceLayer, "BLE Error: %d.\n", msg->data.u8);
                }
            }
            else if (msg->type == BLE_KW_MSG_CONNECTED)
            {
                sInstance.HandleConnectEvent(msg);
            }
            else if (msg->type == BLE_KW_MSG_DISCONNECTED)
            {
                sInstance.HandleConnectionCloseEvent(msg);
            }
            else if (msg->type == BLE_KW_MSG_MTU_CHANGED)
            {
                blekw_start_connection_timeout();
                ChipLogProgress(DeviceLayer, "BLE MTU size has been changed to %d.", msg->data.u16);
            }
            else if (msg->type == BLE_KW_MSG_ATT_WRITTEN || msg->type == BLE_KW_MSG_ATT_LONG_WRITTEN ||
                     msg->type == BLE_KW_MSG_ATT_CCCD_WRITTEN)
            {
                sInstance.HandleWriteEvent(msg);
            }
            else if (msg->type == BLE_KW_MSG_FORCE_DISCONNECT)
            {
                ChipLogProgress(DeviceLayer, "BLE connection timeout: Forcing disconnection.");

                /* Set the advertising parameters */
                if (Gap_Disconnect(g_device_id) != gBleSuccess_c)
                {
                    ChipLogProgress(DeviceLayer, "Gap_Disconnect() failed.");
                }
            }

            /* Freed the message from the queue */
            MSG_Free(msg);
        }
    }
}

void BLEManagerImpl::HandleConnectEvent(blekw_msg_t * msg)
{
    uint8_t device_id_loc = msg->data.u8;
    ChipLogProgress(DeviceLayer, "BLE is connected with device: %d.\n", device_id_loc);

    g_device_id = device_id_loc;
    blekw_start_connection_timeout();
    sInstance.AddConnection(device_id_loc);
    mFlags.Set(Flags::kRestartAdvertising);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerImpl::HandleConnectionCloseEvent(blekw_msg_t * msg)
{
    uint8_t device_id_loc = msg->data.u8;
    ChipLogProgress(DeviceLayer, "BLE is disconnected with device: %d.\n", device_id_loc);

    if (sInstance.RemoveConnection(device_id_loc))
    {
        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId  = device_id_loc;
        event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;

        PlatformMgr().PostEventOrDie(&event);
        mFlags.Set(Flags::kRestartAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled);
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
                err                           = PlatformMgr().PostEvent(&event);
            }
        }
    }
    else
    {
        bleConnState->subscribed      = 0;
        event.Type                    = DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = att_wr_data->device_id;
        err                           = PlatformMgr().PostEvent(&event);
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
                  "CHIPoBLE RX characteristic (con %u, len %u)",
                  att_wr_data->device_id, buf->DataLength());
#endif

    // Post an event to the CHIP queue to deliver the data into the CHIP stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = att_wr_data->device_id;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        err                               = PlatformMgr().PostEvent(&event);
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
        if ((gHciUnsupportedRemoteFeature_c == pGenericEvent->eventData.internalError.errorCode) &&
            (gLeSetPhy_c == pGenericEvent->eventData.internalError.errorSource))
        {
            (void) blekw_msg_add_u8(BLE_KW_MSG_ERROR, BLE_KW_MSG_2M_UPGRADE_ERROR);
        }
        else
        {
            (void) blekw_msg_add_u8(BLE_KW_MSG_ERROR, BLE_INTERNAL_ERROR);
        }
        break;

    case gAdvertisingSetupFailed_c:
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED);
        break;

    case gAdvertisingParametersSetupComplete_c:
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_ADV_PAR_SETUP_COMPLETE);
        break;

    case gAdvertisingDataSetupComplete_c:
        OSA_EventSet(event_msg, CHIP_BLE_KW_EVNT_ADV_DAT_SETUP_COMPLETE);
        break;

    case gRandomAddressReady_c:
        Gap_SetRandomAddress(pGenericEvent->eventData.addrReady.aAddress);
        break;

    case gRandomAddressSet_c:
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
#if CHIP_DEVICE_CONFIG_BLE_SET_PHY_2M_REQ
        ChipLogProgress(DeviceLayer, "BLE K32W: Trying to set the PHY to 2M");

        (void) Gap_LeSetPhy(FALSE, deviceId, 0, gConnPhyUpdateReqTxPhySettings_c, gConnPhyUpdateReqRxPhySettings_c,
                            (uint16_t) gConnPhyUpdateReqPhyOptions_c);
#endif

        /* Notify App Task that the BLE is connected now */
        (void) blekw_msg_add_u8(BLE_KW_MSG_CONNECTED, (uint8_t) deviceId);
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
        PWR_AllowDeviceToSleep();
#endif
    }
    else if (pConnectionEvent->eventType == gConnEvtDisconnected_c)
    {
        blekw_stop_connection_timeout();

        /* Notify App Task that the BLE is disconnected now */
        (void) blekw_msg_add_u8(BLE_KW_MSG_DISCONNECTED, (uint8_t) deviceId);

#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
        if (bleAppStopInProgress == TRUE)
        {
            bleAppStopInProgress = FALSE;
            PWR_AllowDeviceToSleep();
        }
#endif
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
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    PWR_DisallowDeviceToSleep();
#endif /* cPWR_UsePowerDownMode */
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

void BLEManagerImpl::BleAdvTimeoutHandler(TimerHandle_t xTimer)
{
    if (sInstance.mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Start slow advertisement");

        sInstance.mFlags.Clear(Flags::kFastAdvertisingEnabled);
        // stop advertiser, change interval and restart it;
        sInstance.StopAdvertising();
        sInstance.StartAdvertising();
    }

    return;
}

void BLEManagerImpl::CancelBleAdvTimeoutTimer(void)
{
    if (xTimerStop(sbleAdvTimeoutTimer, 0) == pdFAIL)
    {
        ChipLogError(DeviceLayer, "Failed to stop BledAdv timeout timer");
    }
}

void BLEManagerImpl::StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs)
{
    if (xTimerIsTimerActive(sbleAdvTimeoutTimer))
    {
        CancelBleAdvTimeoutTimer();
    }

    // timer is not active, change its period to required value (== restart).
    // FreeRTOS- Block for a maximum of 100 ticks if the change period command
    // cannot immediately be sent to the timer command queue.
    if (xTimerChangePeriod(sbleAdvTimeoutTimer, aTimeoutInMs / portTICK_PERIOD_MS, 100) != pdPASS)
    {
        ChipLogError(DeviceLayer, "Failed to start BledAdv timeout timer");
    }
}

bool BLEManagerImpl::blekw_stop_connection_internal(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    if (Gap_Disconnect(conId) != gBleSuccess_c)
    {
        ChipLogProgress(DeviceLayer, "Gap_Disconnect() failed.");
        return false;
    }
#if defined(cPWR_UsePowerDownMode) && (cPWR_UsePowerDownMode)
    else
    {
        bleAppStopInProgress = TRUE;
        PWR_DisallowDeviceToSleep();
    }
#endif

    return true;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
