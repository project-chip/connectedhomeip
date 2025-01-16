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

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <platform/CommissionableDataProvider.h>

#include <crypto/CHIPCryptoPAL.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/Ble.h>

#include "board.h"
#include "gatt_db_app_interface.h"
#include "gatt_db_handles.h"
#include "stdio.h"
#include "timers.h"

#if defined(CPU_JN518X) && defined(nxp_use_low_power) && (nxp_use_low_power == 1)
#include "PWR_Configuration.h"
#endif

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
#include <platform/DeviceInstanceInfoProvider.h>
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#endif

// Temporarily keep backwards compatibility. To be removed
#ifndef CONFIG_CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_TIMEOUT
#define CONFIG_CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_TIMEOUT CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_TIMEOUT
#endif

/*******************************************************************************
 * Local data types
 *******************************************************************************/
extern "C" bool_t Ble_ConfigureHostStackConfig(void);

#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
extern "C" void PWR_DisallowDeviceToSleep(void);
extern "C" void PWR_AllowDeviceToSleep(void);
#endif

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
#define CHIP_BLE_KW_EVNT_TIMEOUT 1000 / portTICK_PERIOD_MS

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
/** TX Power Level Set */
#define CHIP_BLE_KW_EVNT_POWER_LEVEL_SET 0x0200
/** Maximal time of connection without activity */
#define CHIP_BLE_KW_CONN_TIMEOUT 60000
/** Maximum number of pending BLE events */
#define CHIP_BLE_EVENT_QUEUE_MAX_ENTRIES 10

#define LOOP_EV_BLE (0x08)

/* controller task configuration */
#define CONTROLLER_TASK_PRIORITY (6U)
#define CONTROLLER_TASK_STACK_SIZE (gControllerTaskStackSize_c / sizeof(StackType_t))

/* host task configuration */
#define HOST_TASK_PRIORITY (4U)
#define HOST_TASK_STACK_SIZE (gHost_TaskStackSize_c / sizeof(StackType_t))

/* advertising configuration */
#define BLEKW_ADV_MAX_NO (2)
#define BLEKW_SCAN_RSP_MAX_NO (2)
#define BLEKW_MAX_ADV_DATA_LEN (31)
#define CHIP_ADV_SHORT_UUID_LEN (2)

/* FreeRTOS sw timer */
TimerHandle_t sbleAdvTimeoutTimer;

/* Queue used to synchronize asynchronous messages from the KW BLE tasks */
QueueHandle_t sBleEventQueue;

/* Used to manage asynchronous events from BLE Stack: e.g.: GAP setup finished */
EventGroupHandle_t sEventGroup;

TimerHandle_t connectionTimeout;

const uint8_t ShortUUID_CHIPoBLEService[] = { 0xF6, 0xFF };

#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
static bool bleAppStopInProgress;
#endif

BLEManagerCommon * sImplInstance = nullptr;

} // namespace

CHIP_ERROR BLEManagerCommon::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    EventBits_t eventBits;
    mWriteNotificationHandle[mWriteHandleSize++] = (uint16_t) value_chipoble_rx;

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    mReadNotificationHandle[mReadHandleSize++] = (uint16_t) value_chipoble_c3;
#endif

    mServiceMode = kCHIPoBLE_Enabled;

    // Check if BLE stack is initialized
    VerifyOrExit(!mFlags.Has(Flags::kK32WBLEStackInitialized), err = CHIP_ERROR_INCORRECT_STATE);

    // Initialize the Chip BleLayer.
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    /* Initialization of message wait events -
     * used for receiving BLE Stack events */
    sEventGroup = xEventGroupCreate();
    VerifyOrExit(sEventGroup != NULL, err = CHIP_ERROR_INCORRECT_STATE);

    /* Prepare callback input queue.*/
    sBleEventQueue = xQueueCreate(CHIP_BLE_EVENT_QUEUE_MAX_ENTRIES, sizeof(blekw_msg_t *));
    VerifyOrExit(sBleEventQueue != NULL, err = CHIP_ERROR_INCORRECT_STATE);

    /* Create the connection timeout timer. */
    connectionTimeout =
        xTimerCreate("bleTimeoutTmr", pdMS_TO_TICKS(CHIP_BLE_KW_CONN_TIMEOUT), pdFALSE, (void *) 0, blekw_connection_timeout_cb);
    VerifyOrExit(connectionTimeout != NULL, err = CHIP_ERROR_INCORRECT_STATE);

    sImplInstance = GetImplInstance();

    /* BLE platform code initialization */
    SuccessOrExit(err = InitHostController(&blekw_generic_cb));

    /* Register the GATT server callback */
    VerifyOrExit(GattServer_RegisterCallback(blekw_gatt_server_cb) == gBleSuccess_c, err = CHIP_ERROR_INCORRECT_STATE);

    /* Wait until BLE Stack is ready */
    eventBits = xEventGroupWaitBits(sEventGroup, CHIP_BLE_KW_EVNT_INIT_COMPLETE, pdTRUE, pdTRUE, CHIP_BLE_KW_EVNT_TIMEOUT);
    VerifyOrExit(eventBits & CHIP_BLE_KW_EVNT_INIT_COMPLETE, err = CHIP_ERROR_INCORRECT_STATE);

#if BLE_HIGH_TX_POWER
    /* Set Adv Power */
    Gap_SetTxPowerLevel(gAdvertisingPowerLeveldBm_c, gTxPowerAdvChannel_c);
    eventBits = xEventGroupWaitBits(sEventGroup, CHIP_BLE_KW_EVNT_POWER_LEVEL_SET, pdTRUE, pdTRUE, CHIP_BLE_KW_EVNT_TIMEOUT);
    VerifyOrExit(eventBits & CHIP_BLE_KW_EVNT_POWER_LEVEL_SET, err = CHIP_ERROR_INCORRECT_STATE);

    /* Set Connect Power */
    Gap_SetTxPowerLevel(gConnectPowerLeveldBm_c, gTxPowerConnChannel_c);
    eventBits = xEventGroupWaitBits(sEventGroup, CHIP_BLE_KW_EVNT_POWER_LEVEL_SET, pdTRUE, pdTRUE, CHIP_BLE_KW_EVNT_TIMEOUT);
    VerifyOrExit(eventBits & CHIP_BLE_KW_EVNT_POWER_LEVEL_SET, err = CHIP_ERROR_INCORRECT_STATE);
#endif

#if defined(CPU_JN518X) && defined(nxp_use_low_power) && (nxp_use_low_power == 1)
    PWR_ChangeDeepSleepMode(cPWR_PowerDown_RamRet);
#endif

    GattServer_RegisterHandlesForWriteNotifications(mWriteHandleSize, mWriteNotificationHandle);
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    VerifyOrExit(GattServer_RegisterHandlesForReadNotifications(mReadHandleSize, mReadNotificationHandle) == gBleSuccess_c,
                 err = CHIP_ERROR_INCORRECT_STATE);
#endif

    mFlags.Set(Flags::kK32WBLEStackInitialized);
    mFlags.Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART ? true : false);
    mFlags.Set(Flags::kFastAdvertisingEnabled);

    // Create FreeRTOS sw timer for BLE timeouts and interval change.
    sbleAdvTimeoutTimer = xTimerCreate("BleAdvTimer",       // Just a text name, not used by the RTOS kernel
                                       pdMS_TO_TICKS(100),  // == default timer period (mS)
                                       false,               // no timer reload (==one-shot)
                                       (void *) this,       // init timer id = ble obj context
                                       BleAdvTimeoutHandler // timer callback handler
    );
    VerifyOrExit(sbleAdvTimeoutTimer != NULL, err = CHIP_ERROR_INCORRECT_STATE);

exit:
    return err;
}

uint16_t BLEManagerCommon::_NumConnections(void)
{
    return mDeviceIds.size();
}

bool BLEManagerCommon::_IsAdvertisingEnabled(void)
{
    return mFlags.Has(Flags::kAdvertisingEnabled);
}

bool BLEManagerCommon::_IsAdvertising(void)
{
    return mFlags.Has(Flags::kAdvertising);
}

CHIP_ERROR BLEManagerCommon::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit((mServiceMode == kCHIPoBLE_Enabled) || (mServiceMode == kMultipleBLE_Enabled),
                 err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (mFlags.Has(Flags::kAdvertisingEnabled) != val)
    {
        mFlags.Set(Flags::kAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerCommon::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    switch (mode)
    {
    case BLEAdvertisingMode::kFastAdvertising:
        mFlags.Set(Flags::kFastAdvertisingEnabled);
        break;
    case BLEAdvertisingMode::kSlowAdvertising: {
        // We are in FreeRTOS timer service context, which is a default daemon task and has
        // the highest priority. Stop advertising should be scheduled to run from Matter task.
        mFlags.Clear(Flags::kFastAdvertisingEnabled);
        PlatformMgr().ScheduleWork(StopAdvertisingPriorToSwitchingMode, 0);
        break;
    }
    default:
        return CHIP_ERROR_INVALID_ARGUMENT;
    }
    mFlags.Set(Flags::kRestartAdvertising);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerCommon::_GetDeviceName(char * buf, size_t bufSize)
{
    if (strlen(mDeviceName) >= bufSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    strcpy(buf, mDeviceName);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerCommon::_SetDeviceName(const char * deviceName)
{
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

void BLEManagerCommon::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe:
        ChipDeviceEvent connEstEvent;

        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
        PlatformMgr().PostEventOrDie(&connEstEvent);
        break;

    case DeviceEventType::kCHIPoBLEUnsubscribe:
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        break;

    case DeviceEventType::kCHIPoBLEWriteReceived:
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
        break;

    case DeviceEventType::kCHIPoBLEConnectionError:
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
        break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm:
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        break;

    default:
        break;
    }
}

CHIP_ERROR BLEManagerCommon::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                     const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerCommon::SubscribeCharacteristic() not supported");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerCommon::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                       const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerCommon::UnsubscribeCharacteristic() not supported");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerCommon::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    return blekw_stop_connection_internal(conId);
}

uint16_t BLEManagerCommon::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    uint16_t tempMtu = 0;
    (void) Gatt_GetMtu(conId, &tempMtu);

    return tempMtu;
}

CHIP_ERROR BLEManagerCommon::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                              PacketBufferHandle pBuf)
{
    ChipLogProgress(DeviceLayer, "BLEManagerCommon::SendWriteRequest() not supported");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void BLEManagerCommon::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    BLEMgrImpl().CloseConnection(conId);
}

CHIP_ERROR BLEManagerCommon::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                            PacketBufferHandle data)
{
    VerifyOrReturnError(UUIDsMatch(&Ble::CHIP_BLE_CHAR_2_UUID, charId), BLE_ERROR_GATT_WRITE_FAILED);

    CHIP_ERROR err = CHIP_NO_ERROR;

    if (blekw_send_event(conId, value_chipoble_tx, data->Start(), data->DataLength()) != BLE_OK)
    {
        err = CHIP_ERROR_SENDING_BLOCKED;
    }
    else
    {
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
        event.CHIPoBLEIndicateConfirm.ConId = conId;
        err                                 = PlatformMgr().PostEvent(&event);
    }

    return err;
}

BLEManagerCommon::ble_err_t BLEManagerCommon::blekw_send_event(int8_t connection_handle, uint16_t handle, uint8_t * data,
                                                               uint32_t len)
{
    EventBits_t eventBits;

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
    xEventGroupClearBits(sEventGroup, CHIP_BLE_KW_EVNT_INDICATION_CONFIRMED | CHIP_BLE_KW_EVNT_INDICATION_FAILED);

    if (GattServer_SendInstantValueIndication(connection_handle, handle, len, data) != gBleSuccess_c)
    {
        ChipLogProgress(DeviceLayer, "BLE Event - Can't sent indication");
        return BLE_E_FAIL;
    }

    /* Wait until BLE Stack is ready */
    eventBits = xEventGroupWaitBits(sEventGroup, CHIP_BLE_KW_EVNT_INDICATION_CONFIRMED | CHIP_BLE_KW_EVNT_INDICATION_FAILED, pdTRUE,
                                    pdFALSE, CHIP_BLE_KW_EVNT_TIMEOUT);

    if (eventBits & CHIP_BLE_KW_EVNT_INDICATION_FAILED)
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

BLEManagerCommon::ble_err_t BLEManagerCommon::blekw_start_advertising(gapAdvertisingParameters_t * adv_params,
                                                                      gapAdvertisingData_t * adv, gapScanResponseData_t * scnrsp)
{
    EventBits_t eventBits;

    /************* Set the advertising parameters *************/
    xEventGroupClearBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED | CHIP_BLE_KW_EVNT_ADV_PAR_SETUP_COMPLETE);

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

    eventBits = xEventGroupWaitBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED | CHIP_BLE_KW_EVNT_ADV_PAR_SETUP_COMPLETE,
                                    pdTRUE, pdFALSE, CHIP_BLE_KW_EVNT_TIMEOUT);

    if (!(eventBits & CHIP_BLE_KW_EVNT_ADV_PAR_SETUP_COMPLETE))
    {
        return BLE_E_ADV_PARAMS_FAILED;
    }

    /************* Set the advertising data *************/
    xEventGroupClearBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED | CHIP_BLE_KW_EVNT_ADV_DAT_SETUP_COMPLETE);

    /* Set the advertising data */
    if (Gap_SetAdvertisingData(adv, scnrsp) != gBleSuccess_c)
    {
        return BLE_E_SET_ADV_DATA;
    }

    eventBits = xEventGroupWaitBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED | CHIP_BLE_KW_EVNT_ADV_DAT_SETUP_COMPLETE,
                                    pdTRUE, pdFALSE, CHIP_BLE_KW_EVNT_TIMEOUT);

    if (!(eventBits & CHIP_BLE_KW_EVNT_ADV_DAT_SETUP_COMPLETE))
    {
        return BLE_E_ADV_SETUP_FAILED;
    }

    /************* Start the advertising *************/
    xEventGroupClearBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_CHANGED | CHIP_BLE_KW_EVNT_ADV_FAILED);

    if (gBleSuccess_c != Gap_CreateRandomDeviceAddress(NULL, NULL))
    {
        return BLE_E_SET_ADV_PARAMS;
    }

    eventBits = xEventGroupWaitBits(sEventGroup, CHIP_BLE_KW_EVNT_RND_ADDR_SET, pdTRUE, pdTRUE, CHIP_BLE_KW_EVNT_TIMEOUT);

    if (!(eventBits & CHIP_BLE_KW_EVNT_RND_ADDR_SET))
    {
        return BLE_E_ADV_PARAMS_FAILED;
    }

    /* Start the advertising */
    if (Gap_StartAdvertising(blekw_gap_advertising_cb, blekw_gap_connection_cb) != gBleSuccess_c)
    {
        return BLE_E_START_ADV;
    }

#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
    PWR_DisallowDeviceToSleep();
#endif

    eventBits = xEventGroupWaitBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_CHANGED | CHIP_BLE_KW_EVNT_ADV_FAILED, pdTRUE, pdFALSE,
                                    CHIP_BLE_KW_EVNT_TIMEOUT);
    if (!(eventBits & CHIP_BLE_KW_EVNT_ADV_CHANGED))
    {
#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
        PWR_AllowDeviceToSleep();
#endif
        return BLE_E_START_ADV_FAILED;
    }

#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
    PWR_AllowDeviceToSleep();
#endif

    return BLE_OK;
}

BLEManagerCommon::ble_err_t BLEManagerCommon::blekw_stop_advertising(void)
{
    EventBits_t eventBits;
    bleResult_t res;

    xEventGroupClearBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_CHANGED | CHIP_BLE_KW_EVNT_ADV_FAILED);

    /* Stop the advertising data */
    res = Gap_StopAdvertising();
    if (res != gBleSuccess_c)
    {
        ChipLogProgress(DeviceLayer, "Failed to stop advertising %d", res);
        return BLE_E_STOP;
    }

    eventBits = xEventGroupWaitBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_CHANGED | CHIP_BLE_KW_EVNT_ADV_FAILED, pdTRUE, pdFALSE,
                                    CHIP_BLE_KW_EVNT_TIMEOUT);

    if (eventBits & CHIP_BLE_KW_EVNT_ADV_FAILED)
    {
        ChipLogProgress(DeviceLayer, "Stop advertising flat out failed.");
        return BLE_E_ADV_FAILED;
    }
    else if (!(eventBits & CHIP_BLE_KW_EVNT_ADV_CHANGED))
    {
        ChipLogProgress(DeviceLayer, "Stop advertising event timeout.");
        return BLE_E_ADV_CHANGED;
    }

    return BLE_OK;
}

CHIP_ERROR BLEManagerCommon::ConfigureAdvertisingData(void)
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

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    ReturnErrorOnFailure(EncodeAdditionalDataTlv());
#endif

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
    advInterval = (uint16_t) (advInterval * 0.625F);

    adv_params.minInterval = adv_params.maxInterval = advInterval;
    adv_params.advertisingType                      = gAdvConnectableUndirected_c;
    adv_params.ownAddressType                       = gBleAddrTypeRandom_c;
    adv_params.peerAddressType                      = gBleAddrTypePublic_c;
    memset(adv_params.peerAddress, 0, gcBleDeviceAddressSize_c);
    adv_params.channelMap   = (gapAdvertisingChannelMapFlags_t) (gAdvChanMapFlag37_c | gAdvChanMapFlag38_c | gAdvChanMapFlag39_c);
    adv_params.filterPolicy = gProcessAll_c;

    err = blekw_start_advertising(&adv_params, &adv, &scanRsp);
    if (err == BLE_OK)
    {
        ChipLogProgress(DeviceLayer, "Started Advertising at %d ms", advInterval);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Advertising error 0x%x!", err);
        mFlags.Clear(Flags::kAdvertising);
        return CHIP_ERROR_INCORRECT_STATE;
    }

exit:
    return chipErr;
}

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
CHIP_ERROR BLEManagerCommon::EncodeAdditionalDataTlv()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BitFlags<AdditionalDataFields> dataFields;
    AdditionalDataPayloadGeneratorParams params;

#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    uint8_t rotatingDeviceIdUniqueId[ConfigurationManager::kRotatingDeviceIDUniqueIDLength] = {};
    MutableByteSpan rotatingDeviceIdUniqueIdSpan(rotatingDeviceIdUniqueId);

    err = DeviceLayer::GetDeviceInstanceInfoProvider()->GetRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueIdSpan);
    SuccessOrExit(err);
    err = ConfigurationMgr().GetLifetimeCounter(params.rotatingDeviceIdLifetimeCounter);
    SuccessOrExit(err);
    params.rotatingDeviceIdUniqueId = rotatingDeviceIdUniqueIdSpan;
    dataFields.Set(AdditionalDataFields::RotatingDeviceId);
#endif /* CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID) */
    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(params, sImplInstance->c3AdditionalDataBufferHandle,
                                                                         dataFields);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to generate TLV encoded Additional Data (%s)", __func__);
    }

    return err;
}

void BLEManagerCommon::HandleC3ReadRequest(blekw_msg_t * msg)
{
    bleResult_t result;
    blekw_att_read_data_t * att_rd_data = (blekw_att_read_data_t *) msg->data.data;
    deviceId_t deviceId                 = att_rd_data->device_id;
    uint16_t handle                     = att_rd_data->handle;
    uint16_t length                     = sImplInstance->c3AdditionalDataBufferHandle->DataLength();
    const uint8_t * data                = (const uint8_t *) sImplInstance->c3AdditionalDataBufferHandle->Start();

    result = GattDb_WriteAttribute(handle, length, data);
    if (result != gBleSuccess_c)
    {
        ChipLogError(DeviceLayer, "Failed to write C3 characteristic: %d", result);
    }

    result = GattServer_SendAttributeReadStatus(deviceId, handle, gAttErrCodeNoError_c);
    if (result != gBleSuccess_c)
    {
        ChipLogError(DeviceLayer, "Failed to send response to C3 read request: %d", result);
    }
}
#endif /* CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING */

CHIP_ERROR BLEManagerCommon::StartAdvertising(void)
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

CHIP_ERROR BLEManagerCommon::StopAdvertising(void)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    if (mFlags.Has(Flags::kAdvertising))
    {
        mFlags.Clear(Flags::kAdvertising);
        mFlags.Clear(Flags::kRestartAdvertising);

        if (mDeviceIds.size())
        {
            ble_err_t err = blekw_stop_advertising();
            VerifyOrReturnError(err == BLE_OK, CHIP_ERROR_INCORRECT_STATE);
            CancelBleAdvTimeoutTimer();
        }

#if CONFIG_CHIP_NFC_ONBOARDING_PAYLOAD
        /* schedule NFC emulation stop */
        ChipDeviceEvent advChange;
        advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
        advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
        error                                      = PlatformMgr().PostEvent(&advChange);
#endif
    }

    return error;
}

void BLEManagerCommon::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if BLE stack is initialized
    VerifyOrExit(mFlags.Has(Flags::kK32WBLEStackInitialized), err = CHIP_ERROR_INCORRECT_STATE);

    // Start advertising if needed...
    if (((mServiceMode == kCHIPoBLE_Enabled) || (mServiceMode == kMultipleBLE_Enabled)) && mFlags.Has(Flags::kAdvertisingEnabled))
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
        // Reset to fast advertising mode only if SetBLEAdvertisingEnabled(false) was called (usually from app).
        mFlags.Set(Flags::kFastAdvertisingEnabled);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = kCHIPoBLE_Disabled;
    }
}

void BLEManagerCommon::DriveBLEState(intptr_t arg)
{
    sImplInstance->DriveBLEState();
}

void BLEManagerCommon::StopAdvertisingPriorToSwitchingMode(intptr_t arg)
{
    if (CHIP_NO_ERROR != sImplInstance->StopAdvertising())
    {
        ChipLogProgress(DeviceLayer, "Failed to stop advertising");
    }
}

void BLEManagerCommon::DoBleProcessing(void)
{
    blekw_msg_t * msg = NULL;

    while ((xQueueReceive(sBleEventQueue, &msg, 0) == pdTRUE) && msg)
    {
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
            sImplInstance->HandleConnectEvent(msg);
        }
        else if (msg->type == BLE_KW_MSG_DISCONNECTED)
        {
            sImplInstance->HandleConnectionCloseEvent(msg);
        }
        else if (msg->type == BLE_KW_MSG_MTU_CHANGED)
        {
            if (mServiceMode == kCHIPoBLE_Enabled)
                blekw_start_connection_timeout();

            ChipLogProgress(DeviceLayer, "BLE MTU size has been changed to %d.", msg->data.u16);
        }
        else if (msg->type == BLE_KW_MSG_ATT_WRITTEN || msg->type == BLE_KW_MSG_ATT_LONG_WRITTEN ||
                 msg->type == BLE_KW_MSG_ATT_CCCD_WRITTEN)
        {
            sImplInstance->HandleWriteEvent(msg);
        }
        else if (msg->type == BLE_KW_MSG_ATT_READ)
        {
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
            blekw_att_read_data_t * att_rd_data = (blekw_att_read_data_t *) msg->data.data;
            if (value_chipoble_c3 == att_rd_data->handle)
                sImplInstance->HandleC3ReadRequest(msg);
#endif
        }
        else if (msg->type == BLE_KW_MSG_FORCE_DISCONNECT)
        {
            sImplInstance->HandleForceDisconnect();
        }

        /* Free the message from the queue */
        free(msg);
        msg = NULL;
    }
}

void BLEManagerCommon::RegisterAppCallbacks(BLECallbackDelegate::GapGenericCallback gapCallback,
                                            BLECallbackDelegate::GattServerCallback gattCallback)
{
    callbackDelegate.gapCallback  = gapCallback;
    callbackDelegate.gattCallback = gattCallback;
}

CHIP_ERROR BLEManagerCommon::AddWriteNotificationHandle(uint16_t name)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // This function should be called before calling  BLEManagerCommon::_Init
    VerifyOrExit(!mFlags.Has(Flags::kK32WBLEStackInitialized), err = CHIP_ERROR_INCORRECT_STATE);

    mWriteNotificationHandle[mWriteHandleSize++] = name;

exit:
    return err;
}

CHIP_ERROR BLEManagerCommon::AddReadNotificationHandle(uint16_t name)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // This function should be called before calling  BLEManagerCommon::_Init
    VerifyOrExit(!mFlags.Has(Flags::kK32WBLEStackInitialized), err = CHIP_ERROR_INCORRECT_STATE);

    mReadNotificationHandle[mReadHandleSize++] = name;

exit:
    return err;
}

void BLEManagerCommon::HandleConnectEvent(blekw_msg_t * msg)
{
    uint8_t deviceId = msg->data.u8;
    ChipLogProgress(DeviceLayer, "BLE is connected with device: %d.\n", deviceId);

#if gClkUseFro32K && defined(nxp_use_low_power) && (nxp_use_low_power == 1)
    PWR_DisallowDeviceToSleep();
#endif

    mDeviceIds.insert(deviceId);

    if (mServiceMode == kCHIPoBLE_Enabled)
        blekw_start_connection_timeout();

    if (mServiceMode == kMultipleBLE_Enabled)
    {
        _SetAdvertisingEnabled(false);
        mServiceMode = kMultipleBLE_Disabled;
    }

    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerCommon::HandleConnectionCloseEvent(blekw_msg_t * msg)
{
    uint8_t deviceId = msg->data.u8;
    ChipLogProgress(DeviceLayer, "BLE is disconnected with device: %d.\n", deviceId);

#if gClkUseFro32K && defined(nxp_use_low_power) && (nxp_use_low_power == 1)
    PWR_AllowDeviceToSleep();
#endif

    mDeviceIds.erase(deviceId);

    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kCHIPoBLEConnectionClosed;
    event.CHIPoBLEConnectionError.ConId  = deviceId;
    event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;

    CancelBleAdvTimeoutTimer();

    PlatformMgr().PostEventOrDie(&event);
    mFlags.Set(Flags::kRestartAdvertising);
    mFlags.Set(Flags::kFastAdvertisingEnabled);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerCommon::HandleWriteEvent(blekw_msg_t * msg)
{
    blekw_att_written_data_t * att_wr_data = (blekw_att_written_data_t *) msg->data.data;
    attErrorCode_t status                  = gAttErrCodeNoError_c;

#if CHIP_DEVICE_CHIP0BLE_DEBUG
    ChipLogProgress(DeviceLayer, "Attribute write request(device: %d,handle: %d).", att_wr_data->device_id, att_wr_data->handle);
#endif

    if (mServiceMode == kCHIPoBLE_Enabled)
        blekw_start_connection_timeout();

    if (value_chipoble_rx == att_wr_data->handle)
    {
        sImplInstance->HandleRXCharWrite(msg);
    }
    else if (cccd_chipoble_tx == att_wr_data->handle)
    {
        sImplInstance->HandleTXCharCCCDWrite(msg);
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

void BLEManagerCommon::HandleTXCharCCCDWrite(blekw_msg_t * msg)
{
    CHIP_ERROR err                         = CHIP_NO_ERROR;
    blekw_att_written_data_t * att_wr_data = (blekw_att_written_data_t *) msg->data.data;
    ChipDeviceEvent event;

    VerifyOrExit(att_wr_data->length != 0, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(att_wr_data->data != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

#if CHIP_DEVICE_CHIP0BLE_DEBUG
    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", *att_wr_data->data ? "subscribe" : "unsubscribe");
#endif

    if (*att_wr_data->data)
    {
        if (!mDeviceSubscribed)
        {
            mDeviceSubscribed             = true;
            event.Type                    = DeviceEventType::kCHIPoBLESubscribe;
            event.CHIPoBLESubscribe.ConId = att_wr_data->device_id;
            err                           = PlatformMgr().PostEvent(&event);
        }
    }
    else
    {
        mDeviceSubscribed             = false;
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

void BLEManagerCommon::HandleRXCharWrite(blekw_msg_t * msg)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;
    blekw_att_written_data_t * att_wr_data = (blekw_att_written_data_t *) msg->data.data;

    VerifyOrExit(att_wr_data->length != 0, err = CHIP_ERROR_INCORRECT_STATE);
    VerifyOrExit(att_wr_data->data != nullptr, err = CHIP_ERROR_INCORRECT_STATE);

    // Copy the data to a PacketBuffer.
    buf = System::PacketBufferHandle::NewWithData(att_wr_data->data, att_wr_data->length);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

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

void BLEManagerCommon::HandleForceDisconnect()
{
    ChipLogProgress(DeviceLayer, "BLE connection timeout: Forcing disconnection.");

    /* Set the advertising parameters */
    for (auto & id : mDeviceIds)
    {
        if (Gap_Disconnect(id) != gBleSuccess_c)
        {
            ChipLogProgress(DeviceLayer, "Gap_Disconnect() failed.");
        }
    }

#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
    PWR_AllowDeviceToSleep();
#endif
}

/*******************************************************************************
 * BLE stack callbacks
 *******************************************************************************/
void BLEManagerCommon::blekw_generic_cb(gapGenericEvent_t * pGenericEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GenericEvent(pGenericEvent);

    if (sImplInstance && sImplInstance->callbackDelegate.gapCallback)
    {
        sImplInstance->callbackDelegate.gapCallback(pGenericEvent);
    }

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
        xEventGroupSetBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_SETUP_FAILED);
        break;

    case gAdvertisingParametersSetupComplete_c:
        xEventGroupSetBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_PAR_SETUP_COMPLETE);
        break;

    case gAdvertisingDataSetupComplete_c:
        xEventGroupSetBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_DAT_SETUP_COMPLETE);
        break;

    case gRandomAddressReady_c:
        Gap_SetRandomAddress(pGenericEvent->eventData.addrReady.aAddress);
        break;

    case gRandomAddressSet_c:
        xEventGroupSetBits(sEventGroup, CHIP_BLE_KW_EVNT_RND_ADDR_SET);
        break;

#if BLE_HIGH_TX_POWER
    case gTxPowerLevelSetComplete_c:
        if (gBleSuccess_c == pGenericEvent->eventData.txPowerLevelSetStatus)
        {
            xEventGroupSetBits(sEventGroup, CHIP_BLE_KW_EVNT_POWER_LEVEL_SET);
        }
        break;
#endif

    case gInitializationComplete_c:
        /* Common GAP configuration */
        BleConnManager_GapCommonConfig();

        /* Set the local synchronization event */
        xEventGroupSetBits(sEventGroup, CHIP_BLE_KW_EVNT_INIT_COMPLETE);
        break;
    default:
        break;
    }
}

void BLEManagerCommon::blekw_gap_advertising_cb(gapAdvertisingEvent_t * pAdvertisingEvent)
{
    if (pAdvertisingEvent->eventType == gAdvertisingStateChanged_c)
    {
        /* Set the local synchronization event */
        xEventGroupSetBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_CHANGED);
    }
    else
    {
        /* The advertisement start failed */
        ChipLogProgress(DeviceLayer, "Advertising failed: event=%d reason=0x%04X\n", pAdvertisingEvent->eventType,
                        pAdvertisingEvent->eventData.failReason);

        /* Set the local synchronization event */
        xEventGroupSetBits(sEventGroup, CHIP_BLE_KW_EVNT_ADV_FAILED);
    }
}

void BLEManagerCommon::blekw_gap_connection_cb(deviceId_t deviceId, gapConnectionEvent_t * pConnectionEvent)
{
    /* Call BLE Conn Manager */
    BleConnManager_GapPeripheralEvent(deviceId, pConnectionEvent);

    if (pConnectionEvent->eventType == gConnEvtConnected_c)
    {
#if NXP_DEVICE_K32W1_MCXW7X
#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
        /* Disallow must be called here for K32W1, otherwise an assert will be reached.
         * Disclaimer: this is a workaround until a better cross platform solution is found. */
        PWR_DisallowDeviceToSleep();
#endif
#endif

#if CHIP_DEVICE_CONFIG_BLE_SET_PHY_2M_REQ
        ChipLogProgress(DeviceLayer, "BLE K32W: Trying to set the PHY to 2M");

        (void) Gap_LeSetPhy(FALSE, deviceId, 0, gConnPhyUpdateReqTxPhySettings_c, gConnPhyUpdateReqRxPhySettings_c,
                            (uint16_t) gConnPhyUpdateReqPhyOptions_c);
#endif

        /* Notify App Task that the BLE is connected now */
        (void) blekw_msg_add_u8(BLE_KW_MSG_CONNECTED, (uint8_t) deviceId);
#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
        PWR_AllowDeviceToSleep();
#endif
    }
    else if (pConnectionEvent->eventType == gConnEvtDisconnected_c)
    {
        blekw_stop_connection_timeout();

        /* Notify App Task that the BLE is disconnected now */
        (void) blekw_msg_add_u8(BLE_KW_MSG_DISCONNECTED, (uint8_t) deviceId);

#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
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

void BLEManagerCommon::blekw_connection_timeout_cb(TimerHandle_t timer)
{
    (void) blekw_msg_add_u8(BLE_KW_MSG_FORCE_DISCONNECT, 0);
}

void BLEManagerCommon::blekw_start_connection_timeout(void)
{
    xTimerReset(connectionTimeout, 0);
}

void BLEManagerCommon::blekw_stop_connection_timeout(void)
{
    ChipLogProgress(DeviceLayer, "Stopped connectionTimeout timer.");
    xTimerStop(connectionTimeout, 0);
}

void BLEManagerCommon::blekw_gatt_server_cb(deviceId_t deviceId, gattServerEvent_t * pServerEvent)
{
    if (sImplInstance && sImplInstance->callbackDelegate.gattCallback)
    {
        sImplInstance->callbackDelegate.gattCallback(deviceId, pServerEvent);
    }

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
        xEventGroupSetBits(sEventGroup, CHIP_BLE_KW_EVNT_INDICATION_CONFIRMED);
        break;

    case gEvtError_c:
        if (pServerEvent->eventData.procedureError.procedureType == gSendIndication_c)
        {
            /* Set the local synchronization event */
            xEventGroupSetBits(sEventGroup, CHIP_BLE_KW_EVNT_INDICATION_FAILED);
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
CHIP_ERROR BLEManagerCommon::blekw_msg_add_att_written(blekw_msg_type_t type, uint8_t device_id, uint16_t handle, uint8_t * data,
                                                       uint16_t length)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    blekw_msg_t * msg = NULL;
    blekw_att_written_data_t * att_wr_data;

    /* Allocate a buffer with enough space to store the packet */
    msg = (blekw_msg_t *) malloc(sizeof(blekw_msg_t) + sizeof(blekw_att_written_data_t) + length);
    VerifyOrExit(msg, err = CHIP_ERROR_NO_MEMORY);

    msg->type              = type;
    msg->length            = sizeof(blekw_att_written_data_t) + length;
    att_wr_data            = (blekw_att_written_data_t *) msg->data.data;
    att_wr_data->device_id = device_id;
    att_wr_data->handle    = handle;
    att_wr_data->length    = length;
    FLib_MemCpy(att_wr_data->data, data, length);

    VerifyOrExit(xQueueSend(sBleEventQueue, &msg, 0) == pdTRUE, err = CHIP_ERROR_NO_MEMORY);
    otTaskletsSignalPending(NULL);

exit:
    return err;
}

CHIP_ERROR BLEManagerCommon::blekw_msg_add_att_read(blekw_msg_type_t type, uint8_t device_id, uint16_t handle)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    blekw_msg_t * msg = NULL;
    blekw_att_read_data_t * att_rd_data;

    /* Allocate a buffer with enough space to store the packet */
    msg = (blekw_msg_t *) malloc(sizeof(blekw_msg_t) + sizeof(blekw_att_read_data_t));
    VerifyOrExit(msg, err = CHIP_ERROR_NO_MEMORY);

    msg->type              = type;
    msg->length            = sizeof(blekw_att_read_data_t);
    att_rd_data            = (blekw_att_read_data_t *) msg->data.data;
    att_rd_data->device_id = device_id;
    att_rd_data->handle    = handle;

    VerifyOrExit(xQueueSend(sBleEventQueue, &msg, 0) == pdTRUE, err = CHIP_ERROR_NO_MEMORY);
    otTaskletsSignalPending(NULL);

exit:
    return err;
}

CHIP_ERROR BLEManagerCommon::blekw_msg_add_u8(blekw_msg_type_t type, uint8_t data)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    blekw_msg_t * msg = NULL;

    /* Allocate a buffer with enough space to store the packet */
    msg = (blekw_msg_t *) malloc(sizeof(blekw_msg_t));
    VerifyOrExit(msg, err = CHIP_ERROR_NO_MEMORY);

    msg->type    = type;
    msg->length  = 0;
    msg->data.u8 = data;

    VerifyOrExit(xQueueSend(sBleEventQueue, &msg, 0) == pdTRUE, err = CHIP_ERROR_NO_MEMORY);
    otTaskletsSignalPending(NULL);

exit:
    return err;
}

CHIP_ERROR BLEManagerCommon::blekw_msg_add_u16(blekw_msg_type_t type, uint16_t data)
{
    CHIP_ERROR err    = CHIP_NO_ERROR;
    blekw_msg_t * msg = NULL;

    /* Allocate a buffer with enough space to store the packet */
    msg = (blekw_msg_t *) malloc(sizeof(blekw_msg_t));
    VerifyOrExit(msg, err = CHIP_ERROR_NO_MEMORY);

    msg->type     = type;
    msg->length   = 0;
    msg->data.u16 = data;

    VerifyOrExit(xQueueSend(sBleEventQueue, &msg, 0) == pdTRUE, err = CHIP_ERROR_NO_MEMORY);
    otTaskletsSignalPending(NULL);

exit:
    return err;
}

void BLEManagerCommon::BleAdvTimeoutHandler(TimerHandle_t xTimer)
{
    if (BLEMgrImpl().mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        ChipLogDetail(DeviceLayer, "Start slow advertisement");
        BLEMgr().SetAdvertisingMode(BLEAdvertisingMode::kSlowAdvertising);
    }
}

void BLEManagerCommon::CancelBleAdvTimeoutTimer(void)
{
    if (xTimerStop(sbleAdvTimeoutTimer, 0) == pdFAIL)
    {
        ChipLogError(DeviceLayer, "Failed to stop BledAdv timeout timer");
    }
}

void BLEManagerCommon::StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs)
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

CHIP_ERROR BLEManagerCommon::blekw_stop_connection_internal(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    if (Gap_Disconnect(conId) != gBleSuccess_c)
    {
        ChipLogProgress(DeviceLayer, "Gap_Disconnect() failed.");
        return CHIP_ERROR_INTERNAL;
    }
#if defined(nxp_use_low_power) && (nxp_use_low_power == 1)
    else
    {
        bleAppStopInProgress = TRUE;
        PWR_DisallowDeviceToSleep();
    }
#endif

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip
#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
