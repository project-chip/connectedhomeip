
/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
 *    Copyright (c) 2019 Nest Labs, Inc.
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
 * @file
 * Provides an implementation of the BLEManager object for cc13xx_cc26xx
 * platform using the Texas Instruments SDK and the OpenThread stack.
 */

// #define BLEMGR_DBG_LOGGING

// BLE Manager Debug Logs
extern "C" {
#ifdef BLEMGR_DBG_LOGGING
extern void cc13xx_26xxLog(const char * aFormat, ...);
#define BLEMGR_LOG(...) cc13xx_26xxLog(__VA_ARGS__);
#elif defined(ti_log_Log_ENABLE)
#include "ti_drivers_config.h"
#include "ti_log_config.h"
#define BLEMGR_LOG(...) Log_printf(LogModule_App1, Log_DEBUG, __VA_ARGS__);
#else
#define BLEMGR_LOG(...)
#endif
}
#include <string.h>

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#ifdef CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/Ble.h>
#include <platform/internal/BLEManager.h>

#include "FreeRTOS.h"
#include <queue.h>
#include <task.h>

/* Include DMM module */
#include "chipOBleProfile.h"
#include "hal_types.h"
#include "ti_dmm_application_policy.h"
#include <bcomdef.h>
#include <devinfoservice.h>
#include <dmm/apps/common/freertos/util.h>
#include <dmm/dmm_policy.h>
#include <dmm/dmm_priority_ble_thread.h>
#include <dmm/dmm_scheduler.h>
#include <icall.h>
#include <icall_ble_api.h>
#include <util.h>

extern "C" {
#include "ti_ble_config.h"
#include "ti_drivers_config.h"
#include <gap_advertiser.h>
#ifndef ICALL_FEATURE_SEPARATE_IMGINFO
#include <icall_addrs.h>
#endif /* ICALL_FEATURE_SEPARATE_IMGINFO */
}

#ifndef USE_DEFAULT_USER_CFG
#include "ble_user_config.h"
// BLE user defined configuration. Required to be globally accesible for BLE initialization
icall_userCfg_t user0Cfg = BLE_USER_CFG;
#endif // USE_DEFAULT_USER_CFG

using namespace ::chip;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {
#ifdef ti_log_Log_ENABLE
extern "C" {
extern const Log_Module LogMod_LogModule_App1;
}
#endif
/* Static class member initialization */
BLEManagerImpl BLEManagerImpl::sInstance;
TaskHandle_t BLEManagerImpl::sBleTaskHndl;
ICall_EntityID BLEManagerImpl::sSelfEntity;
ICall_SyncHandle BLEManagerImpl::sSyncEvent;
QueueHandle_t BLEManagerImpl::sEventHandlerMsgQueueID;

// GAP Bond Manager Callbacks
gapBondCBs_t BLEManagerImpl::BLEMgr_BondMgrCBs = {
    PasscodeCb, // Passcode callback
    PairStateCb // Pairing/Bonding state Callback
};

const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };

const ChipBleUUID chipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

chipOBleProfileCBs_t BLEManagerImpl::CHIPoBLEProfile_CBs = {
    // Provisioning GATT Characteristic value change callback
    CHIPoBLEProfile_charValueChangeCB
};

// ===== Members that implement the BLEManager internal interface.
CHIP_ERROR BLEManagerImpl::_Init(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    BLEMGR_LOG("BLEMGR: BLE Initialization Start");
    // Initialize the CHIP BleLayer.
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
    if (err != CHIP_NO_ERROR)
    {
        return err;
    }

    /* Register BLE Stack assert handler */
    RegisterAssertCback(AssertHandler);
    matterAdvIndex = ADV_INDEX_INVALID;

    /* Initialize advertising handles */
    for (uint8_t currAdvIndex = 0; currAdvIndex < MAX_NUM_ADV_SETS; currAdvIndex++)
    {
        sInstance.mAdvSetArray[currAdvIndex].advHandle = ADV_INDEX_INVALID;
    }

    err = CreateEventHandler();
    return err;
}

bool BLEManagerImpl::_IsAdvertisingEnabled(void)
{
    return isAdvertisingEnabled(sInstance.matterAdvIndex);
}

bool BLEManagerImpl::isAdvertisingEnabled(uint8_t advIndex)
{

    bool isAdvertisingEn = false;

    if (advIndex != ADV_INDEX_INVALID)
    {
        isAdvertisingEn = sInstance.mAdvSetArray[advIndex].advState.Has(AdvFlags::kAdvertisingEnabled);
    }

    BLEMGR_LOG("BLEMGR: isAdvertisingEnabled Index: %d: State: %d, is actively advertising: %d", advIndex, isAdvertisingEn,
               isAdvertising(advIndex));

    return isAdvertisingEn;
}

uint8_t BLEManagerImpl::setAdvertisingEnabled(bool val, uint8_t advIndex)
{
    uint8_t ret = FAILURE;
    BLEMGR_LOG("BLEMGR: setAdvertisingEnabled Index: %d, enable: %d", advIndex, val);

    if (advIndex != ADV_INDEX_INVALID)
    {
        sInstance.mAdvSetArray[advIndex].advState.Set(AdvFlags::kAdvertisingEnabled, val);
        sInstance.mAdvSetArray[advIndex].advState.Set(AdvFlags::kAdvertisingRefreshNeeded);

        /* Send event to process state change request */
        if (CHIP_NO_ERROR == DriveBLEState())
        {
            ret = SUCCESS;
        }
    }

    return ret;
}

/* Post event to app processing loop to begin CHIP advertising */
CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_ERROR_INVALID_ARGUMENT;

    if (setAdvertisingEnabled(val, sInstance.matterAdvIndex) == SUCCESS)
    {
        err = CHIP_NO_ERROR;
    }

    return err;
}

CHIP_ERROR BLEManagerImpl::setAdvertisingMode(BLEAdvertisingMode mode, uint8_t advIndex)
{
    CHIP_ERROR ret = CHIP_NO_ERROR;
    BLEMGR_LOG("BLEMGR: setAdvertisingMode Index: %d", advIndex);

    if (advIndex != ADV_INDEX_INVALID)
    {
        /* Send event to process state change request */
        switch (mode)
        {
        case BLEAdvertisingMode::kFastAdvertising:
            sInstance.mAdvSetArray[advIndex].advState.Set(AdvFlags::kFastAdvertisingEnabled, true);
            break;
        case BLEAdvertisingMode::kSlowAdvertising:
            sInstance.mAdvSetArray[advIndex].advState.Set(AdvFlags::kFastAdvertisingEnabled, false);
            break;
        default:
            ret = CHIP_ERROR_INVALID_ARGUMENT;
        }

        sInstance.mAdvSetArray[advIndex].advState.Set(AdvFlags::kAdvertisingRefreshNeeded);

        ret = DriveBLEState();
    }
    else
    {
        ret = CHIP_ERROR_INVALID_ARGUMENT;
    }

    return ret;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    return setAdvertisingMode(mode, sInstance.matterAdvIndex);
}

bool BLEManagerImpl::isAdvertising(uint8_t advIndex)
{
    bool isAdvertising = false;

    if (advIndex != ADV_INDEX_INVALID)
    {
        isAdvertising = sInstance.mAdvSetArray[advIndex].advState.Has(AdvFlags::kAdvertising);
    }

    return isAdvertising;
}
bool BLEManagerImpl::_IsAdvertising(void)
{
    return isAdvertising(sInstance.matterAdvIndex);
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    CHIP_ERROR ret = CHIP_NO_ERROR;

    if (SUCCESS == getDeviceName(buf, bufSize))
    {
        ret = CHIP_NO_ERROR;
    }
    else
    {
        ret = CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    return ret;
}

uint8_t BLEManagerImpl::getDeviceName(char * buf, size_t bufSize)
{
    uint8_t ret = FAILURE;
    if (bufSize >= GAP_DEVICE_NAME_LEN)
    {
        Platform::CopyString(buf, bufSize, mDeviceName);
        ret = SUCCESS;
    }

    return ret;
}

uint8_t BLEManagerImpl::setDeviceName(const char * deviceName)
{
    uint8_t ret = FAILURE;
    if (strlen(deviceName) <= GAP_DEVICE_NAME_LEN)
    {
        Platform::CopyString(mDeviceName, deviceName);
        /* Advertisement payload update only relevant for Matter advertisement set */
        mAdvSetArray[sInstance.matterAdvIndex].advState.Set(AdvFlags::kAdvertisingRefreshNeeded);
        if (CHIP_NO_ERROR == DriveBLEState())
        {
            ret = SUCCESS;
        }
    }

    return ret;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    CHIP_ERROR ret = CHIP_NO_ERROR;

    if (setDeviceName(deviceName) == SUCCESS)
    {
        ret = CHIP_NO_ERROR;
    }
    else
    {
        ret = CHIP_ERROR_INTERNAL;
    }

    return ret;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    uint8_t i, numConns = 0;

    // Try to find an available entry
    for (i = 0; i < MAX_NUM_BLE_CONNS; i++)
    {
        if (connList[i].connHandle != LL_CONNHANDLE_INVALID)
        {
            numConns++;
        }
    }

    return numConns;
}

/*
Iterate through all available connections via connIndex.
Connection is valid if handle is not NULL.
*/
BLE_CONNECTION_OBJECT BLEManagerImpl::GetConnection(uint8_t connIndex)
{
    BLE_CONNECTION_OBJECT conId = NULL;
    if (connIndex < MAX_NUM_BLE_CONNS)
    {
        conId = (BLE_CONNECTION_OBJECT) &connList[connIndex].connHandle;
    }
    return (conId);
}

ConnectivityManager::CHIPoBLEServiceMode BLEManagerImpl::getMatteroBLEServiceMode(BLE_CONNECTION_OBJECT conId)
{
    ConnectivityManager::CHIPoBLEServiceMode mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_NotSupported;
    uint8_t index;

    index = GetBLEConnIndex(*((uint32_t *) conId));

    if (index < MAX_NUM_BLE_CONNS)
    {
        mServiceMode = connList[index].mServiceMode;
    }

    return mServiceMode;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe: {
        ChipDeviceEvent connEstEvent;
        uint8_t i;
        BLEMGR_LOG("BLEMGR: OnPlatformEvent, kCHIPoBLESubscribe on connection: %d", *((uint16_t *) event->CHIPoBLESubscribe.ConId));
        for (i = 0; i < MAX_NUM_BLE_CONNS; i++)
        {
            if (sInstance.connList[i].connHandle == *((uint16_t *) event->CHIPoBLESubscribe.ConId))
            {
                sInstance.connList[i].mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
                break;
            }
        }

        connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;

        PlatformMgr().PostEventOrDie(&connEstEvent);
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kCHIPoBLEUnsubscribe: {
        BLEMGR_LOG("BLEMGR: OnPlatformEvent, kCHIPoBLEUnsubscribe on connection: %d",
                   *((uint16_t *) event->CHIPoBLEUnsubscribe.ConId));
        uint8_t i;
        for (i = 0; i < MAX_NUM_BLE_CONNS; i++)
        {
            if (sInstance.connList[i].connHandle == *((uint16_t *) event->CHIPoBLEUnsubscribe.ConId))
            {
                sInstance.connList[i].mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
                break;
            }
        }

        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kCHIPoBLEWriteReceived: {
        BLEMGR_LOG("BLEMGR: OnPlatformEvent, kCHIPoBLEWriteReceived");
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
    }
    break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm:
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        break;

    case DeviceEventType::kCHIPoBLEConnectionError: {
        BLEMGR_LOG("BLEMGR: OnPlatformEvent, kCHIPoBLEConnectionError");
        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
    }
    break;

    default:
        break;
    }
}

// ===== Members that implement virtual methods on BlePlatformDelegate.
CHIP_ERROR BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    void * pMsg = (void *) ICall_malloc(sizeof(void *));
    pMsg        = (void *) conId;

    if (EnqueueBLEMgrMsg(BLEMGR_CHIPOBLE_CONN_CLOSE, (void *) pMsg) == SUCCESS)
    {
        return CHIP_NO_ERROR;
    }
    else
    {
        return CHIP_ERROR_NO_MEMORY;
    }
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    uint8_t index;
    uint16_t mtu = 0;

    index = GetBLEConnIndex(*((uint32_t *) conId));

    if (index != MAX_NUM_BLE_CONNS)
    {
        mtu = sInstance.connList[index].mtu;
        /* Prior to MTU update event, MTU is determined by the below formula */
        if (mtu == 0)
        {
            mtu = MAX_PDU_SIZE - 4;
        }
    }

    return mtu;
}

// ===== Members that implement virtual methods on BleApplicationDelegate.

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // Unused
}

CHIP_ERROR BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                          PacketBufferHandle data)
{
    BLEMGR_LOG("BLEMGR: BLE SendIndication ");

    // Allocate buffers to send to BLE app task
    uint8_t dataLen = static_cast<uint8_t>(data->DataLength());
    BLECharApi_msg * pMsg;

    pMsg = (BLECharApi_msg *) ICall_malloc(sizeof(BLECharApi_msg));
    if (NULL == pMsg)
    {
        return CHIP_ERROR_NO_MEMORY;
    }

    pMsg->pData = (uint8_t *) ICall_malloc(dataLen);
    if (NULL == pMsg->pData)
    {
        ICall_free((void *) pMsg);
        return CHIP_ERROR_NO_MEMORY;
    }

    pMsg->conId = ICall_malloc(sizeof(BLE_CONNECTION_OBJECT));
    if (NULL == pMsg->conId)
    {
        ICall_free((void *) pMsg->pData);
        ICall_free((void *) pMsg);
        return CHIP_ERROR_NO_MEMORY;
    }

    memset(pMsg->pData, 0x00, dataLen);
    memcpy(pMsg->pData, data->Start(), dataLen);

    pMsg->len = dataLen;

    memcpy(&pMsg->svcId, svcId, sizeof(ChipBleUUID));
    memcpy(&pMsg->charId, charId, sizeof(ChipBleUUID));
    memcpy(pMsg->conId, conId, sizeof(BLE_CONNECTION_OBJECT));

    EnqueueBLEMgrMsg(BLEMGR_APPOBLE_CHAR_WRITE_REQ, (void *) pMsg);

    BLEMGR_LOG("BLEMGR: BLE SendIndication RETURN, Length: %d ", dataLen);
    return CHIP_NO_ERROR;
}

#ifdef TI_APPOBLE_ENABLE

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    uint8_t * srcBuf, uint16_t srcBufLen)
{
    return SendWriteRequest(conId, svcId, charId, srcBuf, srcBufLen);
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      uint8_t * srcBuf, uint16_t srcBufLen)
{
    BLEMGR_LOG("BLEMGR: BLE SendWriteRequest");

    BLECharApi_msg * pMsg;

    pMsg = (BLECharApi_msg *) ICall_malloc(sizeof(BLECharApi_msg));
    if (NULL == pMsg)
    {
        return false;
    }

    pMsg->pData = (uint8_t *) ICall_malloc(srcBufLen);
    if (NULL == pMsg->pData)
    {
        ICall_free((void *) pMsg);
        return false;
    }

    pMsg->conId = ICall_malloc(sizeof(BLE_CONNECTION_OBJECT));
    if (NULL == pMsg->conId)
    {
        ICall_free((void *) pMsg->pData);
        ICall_free((void *) pMsg);
        return false;
    }

    memset(pMsg->pData, 0x00, srcBufLen);
    memcpy(pMsg->pData, srcBuf, srcBufLen);

    pMsg->len = srcBufLen;

    memcpy(&pMsg->svcId, svcId, sizeof(ChipBleUUID));
    memcpy(&pMsg->charId, charId, sizeof(ChipBleUUID));
    memcpy(pMsg->conId, conId, sizeof(BLE_CONNECTION_OBJECT));

    EnqueueBLEMgrMsg(BLEMGR_APPOBLE_CHAR_WRITE_REQ, (void *) pMsg);

    BLEMGR_LOG("BLEMGR: BLE SendWriteRequest, Length: %d ", srcBufLen);
    return true;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     uint8_t * const dstBuf, uint16_t dstBufLen)
{
    BLEMGR_LOG("BLEMGR: BLE SendReadRequest");

    BLECharApi_msg * pMsg;

    pMsg = (BLECharApi_msg *) ICall_malloc(sizeof(BLECharApi_msg));
    if (NULL == pMsg)
    {
        return false;
    }

    /* Save user requested destination buffer */
    pMsg->pData = dstBuf;
    pMsg->len   = dstBufLen;

    pMsg->conId = ICall_malloc(sizeof(BLE_CONNECTION_OBJECT));
    if (NULL == pMsg->conId)
    {
        ICall_free((void *) pMsg);
        return false;
    }

    memcpy(&pMsg->svcId, svcId, sizeof(ChipBleUUID));
    memcpy(&pMsg->charId, charId, sizeof(ChipBleUUID));
    memcpy(pMsg->conId, conId, sizeof(BLE_CONNECTION_OBJECT));

    EnqueueBLEMgrMsg(BLEMGR_APPOBLE_CHAR_READ_REQ, (void *) pMsg);

    BLEMGR_LOG("BLEMGR: BLE SendReadRequest, Length: %d ", dstBufLen);

    return true;
}

bool BLEManagerImpl::SendGenericFxnReq(pfnCallInBleMgrCtx_t FxnPtr, void * arg)
{

    AppoBLEGenericApi_msg * pMsg;
    bool status = false;

    pMsg = (AppoBLEGenericApi_msg *) ICall_malloc(sizeof(AppoBLEGenericApi_msg));
    if (NULL == pMsg)
    {
        return status;
    }

    /* Save user requested destination buffer */
    pMsg->FxnPtr = FxnPtr;
    pMsg->arg    = arg;

    if (SUCCESS == EnqueueBLEMgrMsg(BLEMGR_APPOBLE_CUSTOM_API_REQ, (void *) pMsg))
    {
        status = true;
    }

    BLEMGR_LOG("BLEMGR: BLE SendGenericFxnReq: %d", status);

    return status;
}

uint8_t BLEManagerImpl::SendAppoBLEAdvApi(AppoBLE_api_type_t api, AppoBLEAdvApi_msg * msg)
{
    uint8_t ret = bleMemAllocError;

    switch (api)
    {
    case AppoBLE_interface::AppoBLE_ADV_ADD_UPDATE: {
        BLEMGR_LOG("BLEMGR: Send AppoBLE_ADV_ADD_UPDATE");

        if (msg != NULL)
        {
            AppoBLEAdvApi_msg * pValue = (AppoBLEAdvApi_msg *) ICall_malloc(sizeof(AppoBLEAdvApi_msg));
            memcpy(pValue, msg, sizeof(AppoBLEAdvApi_msg));

            if (sInstance.EnqueueBLEMgrMsg(BLEMGR_APPOBLE_ADV_API_ADD, (void *) pValue))
            {
                ICall_free(pValue);
            }
            else
            {
                ret = SUCCESS;
            }
        }
    }
    break;
    case AppoBLE_interface::AppoBLE_api_type_t::AppoBLE_ADV_REMOVE: {
        BLEMGR_LOG("Send AppoBLE_ADV_REMOVE");

        if ((msg != NULL) && (msg->advIndex != ADV_INDEX_INVALID))
        {
            uint8_t * pValue = (uint8_t *) ICall_malloc(sizeof(uint8_t));
            *pValue          = msg->advIndex;
            if (sInstance.EnqueueBLEMgrMsg(BLEMGR_APPOBLE_ADV_API_REMOVE, (void *) pValue))
            {
                ICall_free(pValue);
            }
            else
            {
                ret = SUCCESS;
            }
        }
    }
    break;

    default:
        BLEMGR_LOG("BLEMGR Unknown API");
        break;
    }

    return ret;
}

// ===== Helper Members that implement the Low level BLE Stack behavior.
void BLEManagerImpl::SetAppoBLEPairingPassCode(uint32_t passcode)
{
    pairingPasscode = passcode;
}

#endif

uint8_t BLEManagerImpl::RemoveAdvSet(uint8_t inputAdvIndex)
{
    uint8_t ret = FAILURE;

    BLEMGR_LOG("BLEMGR: RemoveAdvSet Input Index: %d", inputAdvIndex);

    if ((inputAdvIndex != ADV_INDEX_INVALID) && (mAdvSetArray[inputAdvIndex].advState.Has(AdvFlags::kAdvSetInitialized)))
    {
        // Create Advertisement set N and assign handle
        ret = (bStatus_t) GapAdv_destroy(sInstance.mAdvSetArray[inputAdvIndex].advHandle, GAP_ADV_FREE_OPTION_DONT_FREE);
    }

    return ret;
}
void BLEManagerImpl::UpdateAdvInterval(uint8_t advIndex)
{
    uint8_t status = SUCCESS;
    (void) status; // Unused in non-appoble usecases
    if ((advIndex != ADV_INDEX_INVALID) && (mAdvSetArray[advIndex].advState.Has(AdvFlags::kAdvSetInitialized)))
    {
        if (mAdvSetArray[advIndex].advState.Has(AdvFlags::kFastAdvertisingEnabled))
        {
            // Update advertising interval
            BLEMGR_LOG("BLEMGR: UpdateAdvInterval: Fast Advertising Enabled");
            status = SetAdvInterval(advIndex, mAdvSetArray[advIndex].fastAdvIntMax, mAdvSetArray[advIndex].fastAdvIntMin);
        }
        else
        {
            // Decrease advertising interval
            BLEMGR_LOG("BLEMGR: UpdateAdvInterval: Slow Advertising Enabled");
            status = SetAdvInterval(advIndex, mAdvSetArray[advIndex].slowAdvIntMax, mAdvSetArray[advIndex].slowAdvIntMin);
        }
    }

#ifdef TI_APPOBLE_ENABLE
    if (advIndex != matterAdvIndex)
    {
        HandleAppoBLEAdvChange(advIndex, status, AppoBLE_ADV_CHANGE_INTERVAL);
    }
#endif
}
CHIP_ERROR BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                                   const Ble::ChipBleUUID * charId)
{
    /* Unsupported on TI peripheral device implementation */
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                                     const Ble::ChipBleUUID * charId)
{
    /* Unsupported on TI peripheral device implementation */
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                            PacketBufferHandle pBuf)
{
    /* Unsupported on TI peripheral device implementation */
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

uint8_t BLEManagerImpl::setAdvertisingInterval(uint8_t advIndex, uint32_t intervalMax, uint32_t intervalMin)
{
    uint8_t ret = FAILURE;
    BLEMGR_LOG("BLEMGR: setAdvertisingInterval Index: %d, interval Max: %d, interval Min: %d", advIndex, intervalMax, intervalMin);

    if (advIndex != ADV_INDEX_INVALID)
    {
        if (sInstance.mAdvSetArray[advIndex].advState.Has(AdvFlags::kFastAdvertisingEnabled))
        {
            sInstance.mAdvSetArray[advIndex].fastAdvIntMin = intervalMin;
            sInstance.mAdvSetArray[advIndex].fastAdvIntMax = intervalMax;
        }
        else
        {
            sInstance.mAdvSetArray[advIndex].slowAdvIntMin = intervalMin;
            sInstance.mAdvSetArray[advIndex].slowAdvIntMax = intervalMax;
        }

        sInstance.mAdvSetArray[advIndex].advState.Set(AdvFlags::kAdvertisingRefreshNeeded);

        /* Send event to process state change request */
        if (CHIP_NO_ERROR == DriveBLEState())
        {
            ret = SUCCESS;
        }
    }

    return ret;
}

/*
 * Advertisement Data and Scan Response Data buffers must persist
 * Returns valid index into BLE Manager advertisement data structure for internal use and issues application callback with
 * advertisement index.
 */
uint8_t BLEManagerImpl::AddUpdateAdvSet(uint8_t inputAdvIndex, GapAdv_params_t advParams, const uint8_t * advData,
                                        uint8_t advDataLen, const uint8_t * scanRspData, uint8_t scanRspDataLen,
                                        GapAdv_eventMaskFlags_t evtMask)
{
    uint8_t status       = SUCCESS;
    uint8_t currAdvIndex = 0;

    /* Events must support Start/End logic for BLE Manager */
    evtMask = ((GapAdv_eventMaskFlags_t) (GAP_ADV_EVT_MASK_START_AFTER_ENABLE | GAP_ADV_EVT_MASK_END_AFTER_DISABLE |
                                          GAP_ADV_EVT_MASK_SET_TERMINATED | evtMask));
    BLEMGR_LOG("BLEMGR: AddUpdateAdvSet Input Evt Mask: %d", evtMask);

    // Find free advertisement slot (New set request) or use provided advertising index (Update)
    if (inputAdvIndex == ADV_INDEX_INVALID)
    {
        for (currAdvIndex = 0; currAdvIndex < MAX_NUM_ADV_SETS; currAdvIndex++)
        {
            if (!mAdvSetArray[currAdvIndex].advState.Has(AdvFlags::kAdvSetInitialized))
            {
                break;
            }
        }

        /* Intialize new element*/
        if (currAdvIndex < MAX_NUM_ADV_SETS)
        {
            mAdvSetArray[currAdvIndex].advState.ClearAll();
            mAdvSetArray[currAdvIndex].slowAdvIntMin = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
            mAdvSetArray[currAdvIndex].slowAdvIntMax = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
            mAdvSetArray[currAdvIndex].fastAdvIntMin = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
            mAdvSetArray[currAdvIndex].fastAdvIntMax = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
        }
        else
        {
            // Issue callback to application indicating failure, no advertismenet slot found
            status = bleNoResources;
        }
    }
    else
    {
        currAdvIndex = inputAdvIndex;
    }

    if ((status == SUCCESS) && (!mAdvSetArray[currAdvIndex].advState.Has(AdvFlags::kAdvSetInitialized)))
    {
        // Create Advertisement set N and assign handle
        status = (bStatus_t) GapAdv_create(&advCallback, &advParams, &sInstance.mAdvSetArray[currAdvIndex].advHandle);

        // Maintain copy of advertising properties
        mAdvSetArray[currAdvIndex].advProps = advParams.eventProps;

        // Set event mask for set N
        if (status == SUCCESS)
        {
            status = (bStatus_t) GapAdv_setEventMask(sInstance.mAdvSetArray[currAdvIndex].advHandle, evtMask);
        }
    }

    if ((status == SUCCESS) && (mAdvSetArray[currAdvIndex].advState.Has(AdvFlags::kAdvSetInitialized)))
    {
        // Don't free anything since we're going to use the same buffer to re-load
        status =
            (bStatus_t) GapAdv_prepareLoadByHandle(sInstance.mAdvSetArray[currAdvIndex].advHandle, GAP_ADV_FREE_OPTION_DONT_FREE);
    }

    if (status == SUCCESS)
    {
        // Load advertising data for set N that is statically allocated by the app
        status = (bStatus_t) GapAdv_loadByHandle(sInstance.mAdvSetArray[currAdvIndex].advHandle, GAP_ADV_DATA_TYPE_ADV, advDataLen,
                                                 advData);
    }

    if ((status == SUCCESS) && (NULL != scanRspData))
    {
        if (mAdvSetArray[currAdvIndex].advState.Has(AdvFlags::kAdvSetInitialized))
        {
            // Don't free anything since we're going to use the same buffer to re-load
            status = (bStatus_t) GapAdv_prepareLoadByHandle(sInstance.mAdvSetArray[currAdvIndex].advHandle,
                                                            GAP_ADV_FREE_OPTION_DONT_FREE);
        }

        if (status == SUCCESS)
        {
            // Load scan response data for set N that is statically allocated by the app
            status = (bStatus_t) GapAdv_loadByHandle(sInstance.mAdvSetArray[currAdvIndex].advHandle, GAP_ADV_DATA_TYPE_SCAN_RSP,
                                                     scanRspDataLen, scanRspData);
        }
    }

    if (status == SUCCESS)
    {
        mAdvSetArray[currAdvIndex].advState.Set(AdvFlags::kAdvSetInitialized);
    }
    else
    {
        // Internal error in BLE stack, abort operation
        GapAdv_abortLoad();
    }
#ifdef TI_APPOBLE_ENABLE
    BLEMGR_LOG("BLEMGR: HandleAppoBLEAdvChange: %d", status);

    if (currAdvIndex != matterAdvIndex)
    {
        HandleAppoBLEAdvChange(currAdvIndex, status, AppoBLE_ADV_ADD_UPDATE);
    }
#endif
    return currAdvIndex;
}

/* The global device name is set through the matter advertisement SetDeviceName API,
 *  AppoBLE advertisement sets must manually update their device name in the
 *  advertisment set to match
 */
CHIP_ERROR BLEManagerImpl::AddUpdateMatteroBLEAdv(void)
{
    static uint8_t matterAdvInit = false;
    CHIP_ERROR err               = CHIP_NO_ERROR;
    uint16_t deviceDiscriminator;
    uint8_t localDeviceNameLen;
    uint8_t scanIndex = 0;
    uint8_t advIndex  = 0;
    uint8_t scanResLength;
    uint8_t advLength;
    ChipBLEDeviceIdentificationInfo mDeviceIdInfo;
    const GapAdv_eventMaskFlags_t evtMask =
        (GapAdv_eventMaskFlags_t) (GAP_ADV_EVT_MASK_START_AFTER_ENABLE | GAP_ADV_EVT_MASK_END_AFTER_DISABLE |
                                   GAP_ADV_EVT_MASK_SET_TERMINATED);

    const GapAdv_params_t advParams = { .eventProps   = GAP_ADV_PROP_CONNECTABLE | GAP_ADV_PROP_LEGACY | GAP_ADV_PROP_SCANNABLE,
                                        .primIntMin   = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN,
                                        .primIntMax   = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN,
                                        .primChanMap  = GAP_ADV_CHAN_ALL,
                                        .peerAddrType = PEER_ADDRTYPE_PUBLIC_OR_PUBLIC_ID,
                                        .peerAddr     = { 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa },
                                        .filterPolicy = GAP_ADV_AL_POLICY_ANY_REQ,
                                        .txPower      = GAP_ADV_TX_POWER_NO_PREFERENCE,
                                        .primPhy      = GAP_ADV_PRIM_PHY_1_MBPS,
                                        .secPhy       = GAP_ADV_SEC_PHY_1_MBPS,
                                        .sid          = 0 };

    ConfigurationMgr().GetBLEDeviceIdentificationInfo(mDeviceIdInfo);
    BLEMGR_LOG("BLEMGR: AddUpdateMatteroBLEAdv");

    memset(sInstance.mScanResDatachipOBle, 0, CHIPOBLE_ADV_DATA_MAX_SIZE);
    memset(sInstance.mAdvDatachipOBle, 0, CHIPOBLE_ADV_DATA_MAX_SIZE);

    // Verify device name was not already set
    if (sInstance.matterAdvIndex == ADV_INDEX_INVALID)
    {
        /* Default device name is MATTER-<DISCRIMINATOR> */
        deviceDiscriminator = mDeviceIdInfo.GetDeviceDiscriminator();

        localDeviceNameLen = strlen(CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX) + CHIPOBLE_DEVICE_DESC_LENGTH;

        memset(sInstance.mDeviceName, 0, GAP_DEVICE_NAME_LEN);
        snprintf(sInstance.mDeviceName, GAP_DEVICE_NAME_LEN, "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX,
                 deviceDiscriminator);
    }
    else
    {
        localDeviceNameLen = strlen(sInstance.mDeviceName);
    }

    // Set the Device Name characteristic in the GAP GATT Service
    // For more information, see the section in the User's Guide:
    // http://software-dl.ti.com/lprf/ble5stack-latest/
    GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, sInstance.mDeviceName);

    /* Verify scan response data length */
    advLength     = sizeof(static_cast<uint16_t>(CHIPOBLE_SERV_UUID)) + static_cast<uint8_t>(sizeof(mDeviceIdInfo)) + 1;
    scanResLength = localDeviceNameLen + CHIPOBLE_SCANRES_SIZE_NO_NAME;

    if (!(((CHIPOBLE_ADV_SIZE_NO_DEVICE_ID_INFO + advLength) < CHIPOBLE_ADV_DATA_MAX_SIZE) &&
          (scanResLength < CHIPOBLE_ADV_DATA_MAX_SIZE)))
    {
        err = CHIP_ERROR_NO_MEMORY;
    }

    if (CHIP_NO_ERROR == err)
    {
        BLEMGR_LOG("BLEMGR: AdvInit: MDeviceIDInfo Size: %d", sizeof(mDeviceIdInfo));
        BLEMGR_LOG("BLEMGR: AdvInit: advlength: %d", advLength);
        BLEMGR_LOG("BLEMGR: AdvInit:Desc : %d", mDeviceIdInfo.GetDeviceDiscriminator());

        sInstance.mAdvDatachipOBle[advIndex++] = 0x02;
        sInstance.mAdvDatachipOBle[advIndex++] = GAP_ADTYPE_FLAGS;
        sInstance.mAdvDatachipOBle[advIndex++] = GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED | GAP_ADTYPE_FLAGS_GENERAL;
        sInstance.mAdvDatachipOBle[advIndex++] = advLength;
        sInstance.mAdvDatachipOBle[advIndex++] = GAP_ADTYPE_SERVICE_DATA;
        sInstance.mAdvDatachipOBle[advIndex++] = static_cast<uint8_t>(LO_UINT16(CHIPOBLE_SERV_UUID));
        sInstance.mAdvDatachipOBle[advIndex++] = static_cast<uint8_t>(HI_UINT16(CHIPOBLE_SERV_UUID));
        memcpy(&sInstance.mAdvDatachipOBle[advIndex], (void *) &mDeviceIdInfo, static_cast<uint8_t>(sizeof(mDeviceIdInfo)));
        advIndex += static_cast<uint8_t>(sizeof(mDeviceIdInfo));

        sInstance.mScanResDatachipOBle[scanIndex++] = localDeviceNameLen + 1;
        sInstance.mScanResDatachipOBle[scanIndex++] = GAP_ADTYPE_LOCAL_NAME_COMPLETE;
        memcpy(&sInstance.mScanResDatachipOBle[scanIndex], sInstance.mDeviceName, localDeviceNameLen);
        scanIndex += localDeviceNameLen;
        sInstance.mScanResDatachipOBle[scanIndex++] = 0x03;
        sInstance.mScanResDatachipOBle[scanIndex++] = GAP_ADTYPE_16BIT_COMPLETE;
        sInstance.mScanResDatachipOBle[scanIndex++] = static_cast<uint8_t>(LO_UINT16(CHIPOBLE_SERV_UUID));
        sInstance.mScanResDatachipOBle[scanIndex++] = static_cast<uint8_t>(HI_UINT16(CHIPOBLE_SERV_UUID));

        for (uint8_t temp = 0; temp < scanIndex; temp++)
        {
            BLEMGR_LOG("BLEMGR: AdvInit Scan Response Data: %x", sInstance.mScanResDatachipOBle[temp]);
        }

        matterAdvIndex = AddUpdateAdvSet(matterAdvIndex, advParams, (const unsigned char *) &sInstance.mAdvDatachipOBle, advIndex,
                                         (const unsigned char *) &sInstance.mScanResDatachipOBle, scanIndex, evtMask);

        if (!matterAdvInit)
        {
            matterAdvInit = true;
            // Initial Setup State
            Util_constructClock(&sInstance.clkAdvTimeout, AdvTimeoutHandler, ADV_TIMEOUT, 0, false, (uintptr_t) NULL);
            mAdvSetArray[matterAdvIndex].advState.Set(AdvFlags::kAdvertisingEnabled,
                                                      CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
            mAdvSetArray[matterAdvIndex].advState.Set(AdvFlags::kFastAdvertisingEnabled, true);

            mAdvSetArray[matterAdvIndex].slowAdvIntMin = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
            mAdvSetArray[matterAdvIndex].slowAdvIntMax = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
            mAdvSetArray[matterAdvIndex].fastAdvIntMin = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
            mAdvSetArray[matterAdvIndex].fastAdvIntMax = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
        }

        BLEMGR_LOG("BLEMGR: Matter Advertisement Set Generated: %d", matterAdvIndex);
    }
    else
    {
        BLEMGR_LOG("BLEMGR: Error Generating Matter Advertisement Set");
    }

    return err;
}

/*********************************************************************
 * @fn      EventHandler_init
 *
 * @brief   Called during initialization and contains application
 *          specific initialization (ie. hardware initialization/setup,
 *          table initialization, power up notification, etc), and
 *          profile initialization/setup.
 */
void BLEManagerImpl::EventHandler_init(void)
{
    BLEMGR_LOG("BLEMGR: EventHandler_init");

    /* Update User Configuration of the stack */
    user0Cfg.appServiceInfo->timerTickPeriod     = ICall_getTickPeriod();
    user0Cfg.appServiceInfo->timerMaxMillisecond = ICall_getMaxMSecs();

    /* Initialize ICall module */
    ICall_init();

    /* Start tasks of external images */
    ICall_createRemoteTasks();
    BLEManagerImpl::sBleTaskHndl = (TaskHandle_t) (*((TaskHandle_t *) ICall_getRemoteTaskHandle(0)));
    DMMSch_registerClient((TaskHandle_t) BLEManagerImpl::sBleTaskHndl, DMMPolicy_StackRole_BlePeripheral);
    /* Set the stacks in default states */
    DMMPolicy_updateStackState(DMMPolicy_StackRole_BlePeripheral, DMMPOLICY_BLE_IDLE);

    vTaskPrioritySet(xTaskGetCurrentTaskHandle(), BLE_MANAGER_TASK_PRIORITY);

    // ******************************************************************
    // N0 STACK API CALLS CAN OCCUR BEFORE THIS CALL TO ICall_registerApp
    // ******************************************************************
    // Register the current thread as an ICall dispatcher application
    // so that the application can send and receive messages.
    ICall_registerApp(&BLEManagerImpl::sSelfEntity, &BLEManagerImpl::sSyncEvent);

#ifdef USE_RCOSC
    RCOSC_enableCalibration();
#endif // USE_RCOSC

    // Create an RTOS queue for message from profile to be sent to app.
    Util_constructQueue(&BLEManagerImpl::sEventHandlerMsgQueueID);

    // Configure GAP
    {
        uint16_t paramUpdateDecision = DEFAULT_PARAM_UPDATE_REQ_DECISION;

        // Pass all parameter update requests to the app for it to decide
        GAP_SetParamValue(GAP_PARAM_LINK_UPDATE_DECISION, paramUpdateDecision);
    }

    // Setup the GAP Bond Manager. For more information see the GAP Bond Manager
    // section in the User's Guide:
    // http://software-dl.ti.com/lprf/ble5stack-latest/
    setBondManagerParameters();

    // Initialize GATT attributes
    GGS_AddService(GATT_ALL_SERVICES);         // GAP GATT Service
    GATTServApp_AddService(GATT_ALL_SERVICES); // GATT Service
    DevInfo_AddService();                      // Device Information Service

    CHIPoBLEProfile_AddService(GATT_ALL_SERVICES);

    // Start Bond Manager and register callback
    VOID GAPBondMgr_Register(&BLEMgr_BondMgrCBs);

    // Register with GAP for HCI/Host messages. This is needed to receive HCI
    // events. For more information, see the HCI section in the User's Guide:
    // http://software-dl.ti.com/lprf/ble5stack-latest/
    GAP_RegisterForMsgs(BLEManagerImpl::sSelfEntity);

    // Register for GATT local events and ATT Responses pending for transmission
    GATT_RegisterForMsgs(BLEManagerImpl::sSelfEntity);

    CHIPoBLEProfile_RegisterAppCBs(&CHIPoBLEProfile_CBs);

    // Set default values for Data Length Extension
    // Extended Data Length Feature is already enabled by default
    {
        // This API is documented in hci.h
        // See the LE Data Length Extension section in the BLE5-Stack User's Guide for information on using this command:
        // http://software-dl.ti.com/lprf/ble5stack-latest/
        HCI_LE_WriteSuggestedDefaultDataLenCmd(BLEMANAGER_SUGGESTED_PDU_SIZE, BLEMANAGER_SUGGESTED_TX_TIME);
    }

    // Initialize GATT Client
    GATT_InitClient("");

    // Initialize Connection List
    ClearBLEConnListEntry(LL_CONNHANDLE_ALL);

    // Initialize GAP layer for Peripheral role and register to receive GAP events
    GAP_DeviceInit(GAP_PROFILE_PERIPHERAL, BLEManagerImpl::sSelfEntity, sInstance.addrMode, &pRandomAddress);

    // Initialize array to store connection handle and RSSI values
    InitPHYRSSIArray();
    BLEMGR_LOG("BLEMGR: EventHandler_init Done");
#ifdef TI_APPOBLE_ENABLE
    /* Call Application initialization function if necessary */
    AppoBLEHook_Init();
#endif
}

/*********************************************************************
 * @fn      InitPHYRSSIArray
 *
 * @brief   Initializes the array of structure/s to store data related
 *          RSSI based auto PHy change
 *
 * @param   connHandle - the connection handle
 *
 * @param   addr - pointer to device address
 *
 * @return  index of connection handle
 */
void BLEManagerImpl::InitPHYRSSIArray(void)
{
    BLEMGR_LOG("BLEMGR: InitPHYRSSIArray");

    // Initialize array to store connection handle and RSSI values
    memset(sInstance.connList, 0, sizeof(sInstance.connList));

    for (uint8_t index = 0; index < MAX_NUM_BLE_CONNS; index++)
    {
        sInstance.connList[index].connHandle = INVALID_HANDLE;
    }
}

/*********************************************************************
 * @fn      CreateEventHandler
 *
 * @brief   Create FreeRTOS Task for BLE Event handling
 *
 */
CHIP_ERROR BLEManagerImpl::CreateEventHandler(void)
{
    BLEMGR_LOG("BLEMGR: CreateEventHandler");

    BaseType_t xReturned;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(EventHandler,                        /* Function that implements the task. */
                            "ble_hndlr",                         /* Text name for the task. */
                            BLEMANAGER_EVENT_HANDLER_STACK_SIZE, /* Stack size in words, not bytes. */
                            this,                                /* Parameter passed into the task. */
                            BLE_STACK_TASK_PRIORITY,             /* Keep priority the same as ICALL until init is complete */
                            NULL);                               /* Used to pass out the created task's handle. */

    if (xReturned == errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY)
    {
        return CHIP_ERROR_NO_MEMORY;
    }
    else
    {
        return CHIP_NO_ERROR;
    }
}

/*********************************************************************
 * @fn      RemoteDisplay_processStackMsg
 *
 * @brief   Process an incoming stack message.
 *
 * @param   pMsg - message to process
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
uint8_t BLEManagerImpl::ProcessBLEStackEvent(ICall_Hdr * pMsg)
{
    // Always dealloc pMsg unless set otherwise
    uint8_t safeToDealloc = TRUE;

    switch (pMsg->event)
    {
    case GAP_MSG_EVENT:
        ProcessGapMessage((gapEventHdr_t *) pMsg);
        break;

    case GATT_MSG_EVENT:
        // Process GATT message
        safeToDealloc = ProcessGATTMsg((gattMsgEvent_t *) pMsg);
        break;
    case HCI_GAP_EVENT_EVENT: {
        // Process HCI message
        switch (pMsg->status)
        {
        case HCI_BLE_HARDWARE_ERROR_EVENT_CODE:
            assert(false);
            break;
        // HCI Commands Events
        case HCI_COMMAND_STATUS_EVENT_CODE: {
            hciEvt_CommandStatus_t * pMyMsg = (hciEvt_CommandStatus_t *) pMsg;
            switch (pMyMsg->cmdOpcode)
            {
            case HCI_LE_SET_PHY: {
                if (pMyMsg->cmdStatus == HCI_ERROR_CODE_UNSUPPORTED_REMOTE_FEATURE)
                {
                    /* PHY Change failure, peer does not support this */
                }
                break;
            }
            default:
                break;
            }
            break;
        }

        case HCI_LE_EVENT_CODE: {
            hciEvt_BLEPhyUpdateComplete_t * pPUC = (hciEvt_BLEPhyUpdateComplete_t *) pMsg;

            // A Phy Update Has Completed or Failed
            if (pPUC->BLEEventCode == HCI_BLE_PHY_UPDATE_COMPLETE_EVENT)
            {
                if (pPUC->status != SUCCESS)
                {
                    /* PHY Change failure */
                }
                else
                {
                    /* PHY Update successful */
                }
            }
            break;
        }
        default:
            break;
        }
#ifdef TI_APPOBLE_ENABLE
        AppoBLEHook_ProcessGapHCIEvt((const ICall_Hdr *) pMsg);
#endif
        break;
    }

    default:
        // do nothing
        break;
    }

    return safeToDealloc;
}

uint8_t BLEManagerImpl::SetAdvInterval(uint8_t advIndex, uint32_t intervalMax, uint32_t intervalMin)
{
    uint8_t status = SUCCESS;
    BLEMGR_LOG("BLEMGR: SetAdvInterval");

    if (mAdvSetArray[advIndex].advState.Has(AdvFlags::kAdvSetInitialized))
    {
        GapAdv_disable(mAdvSetArray[advIndex].advHandle);

        //  Set interval Max/Min
        GapAdv_setParam(mAdvSetArray[advIndex].advHandle, GAP_ADV_PARAM_PRIMARY_INTERVAL_MAX, &intervalMax);
        GapAdv_setParam(mAdvSetArray[advIndex].advHandle, GAP_ADV_PARAM_PRIMARY_INTERVAL_MIN, &intervalMin);
    }

    // Re-enable advertisement based on previous state
    if (mAdvSetArray[advIndex].advState.Has(AdvFlags::kAdvertisingEnabled))
    {
        status = (bStatus_t) GapAdv_enable(mAdvSetArray[advIndex].advHandle, GAP_ADV_ENABLE_OPTIONS_USE_MAX, 0);
        BLEMGR_LOG("BLEMGR: SetAdvInterval, re-enable adv set status: %d", status);

        // If adverisement fails, keep flags set
        if (status == SUCCESS)
        {
            if (advIndex == sInstance.matterAdvIndex)
            {
                BLEMGR_LOG("BLEMGR: SetAdvInterval, Set Matter adv timer");

                // Start advertisement timeout timer for Matter Advertisment set
                if (mAdvSetArray[advIndex].advState.Has(AdvFlags::kFastAdvertisingEnabled))
                {
                    Util_rescheduleClock(&sInstance.clkAdvTimeout, CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
                }
                else
                {
                    Util_rescheduleClock(&sInstance.clkAdvTimeout,
                                         ADV_TIMEOUT - CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
                }
                Util_startClock(&sInstance.clkAdvTimeout);
            }
        }
    }

    // Advertising should be disabled
    if ((!mAdvSetArray[advIndex].advState.Has(AdvFlags::kAdvertisingEnabled)) &&
        mAdvSetArray[advIndex].advState.Has(AdvFlags::kAdvertising))
    {
        BLEMGR_LOG("BLEMGR: SetAdvInterval: Advertisements disabled");

        // Stop advertising
        GapAdv_disable(mAdvSetArray[advIndex].advHandle);

        if (advIndex == sInstance.matterAdvIndex)
        {
            Util_stopClock(&sInstance.clkAdvTimeout);
        }

        // reset fast advertising
        mAdvSetArray[advIndex].advState.Set(AdvFlags::kFastAdvertisingEnabled);
    }
    return status;
}
/*********************************************************************
 * @fn      ProcessBleMgrEvt
 *
 * @brief   Process an incoming callback from a profile.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
void BLEManagerImpl::ProcessBleMgrEvt(GenericQueuedEvt_t * pMsg)
{
    bool dealloc = TRUE;

    switch (pMsg->event)
    {
    case BLEMGR_APPOBLE_ADV_API_ADD: {
        BLEMGR_LOG("BLEMGR: BLEMGR_APPOBLE_ADV_API_ADD");

        AppoBLEAdvApi_msg * adv = (AppoBLEAdvApi_msg *) (pMsg->pData);
        AddUpdateAdvSet(adv->advIndex, adv->advParams, (const unsigned char *) adv->advData, adv->advDataLen,
                        (const unsigned char *) adv->scanRspData, adv->scanRspDataLen, adv->evtMask);
    }

    break;
    case BLEMGR_APPOBLE_ADV_API_REMOVE: {
        BLEMGR_LOG("BLEMGR: BLEMGR_APPOBLE_ADV_API_REMOVE");

        uint8_t * index = (uint8_t *) (pMsg->pData);
        RemoveAdvSet(*index);
    }

    break;

    /* External CHIPoBLE Event trigger */
    case BLEMGR_ADV_STATE_UPDATE: {
        BLEMGR_LOG("BLEMGR: BLEMGR_ADV_STATE_UPDATE");

        if (sInstance.mFlags.Has(Flags::kBLEStackInitialized))
        {
            for (uint8_t currAdvIndex = 0; currAdvIndex < MAX_NUM_ADV_SETS; currAdvIndex++)
            {
                BLEMGR_LOG("BLEMGR: BLEMGR_ADV_STATE_UPDATE curr indx: %d refresh: %d", currAdvIndex,
                           mAdvSetArray[currAdvIndex].advState.Has(AdvFlags::kAdvertisingRefreshNeeded));

                if (((mAdvSetArray[currAdvIndex].advHandle != ADV_INDEX_INVALID)) &&
                    (mAdvSetArray[currAdvIndex].advState.Has(AdvFlags::kAdvertisingRefreshNeeded)))
                {
                    BLEMGR_LOG("BLEMGR: BLE Process Application Message: kAdvertisingRefreshNeeded");

                    /* Update Advertising payload for Matter if required */
                    if (currAdvIndex == matterAdvIndex)
                    {
                        AddUpdateMatteroBLEAdv();
                    }
                    mAdvSetArray[currAdvIndex].advState.Clear(AdvFlags::kAdvertisingRefreshNeeded);
                    UpdateAdvInterval(currAdvIndex);
                }
            }
        }
    }
    break;
    case BLEMGR_CHIPOBLE_CONN_CLOSE: {
        uint16_t connHandle = *((uint16_t *) (pMsg->pData));

        // Close active connection
        GAP_TerminateLinkReq(connHandle, HCI_DISCONNECT_REMOTE_USER_TERM);
    }
    break;

    case BLEMGR_APPOBLE_CHAR_WRITE_REQ: {
        BLECharApi_msg * msg = ((BLECharApi_msg *) (pMsg->pData));
        uint8_t dataLen      = msg->len;

        if (UUIDsMatch(&CHIP_BLE_SVC_ID, &msg->svcId))
        {
            /* MatteroBLE Event*/
            BLEMGR_LOG("BLEMGR: MatteroBLE Char write Length: %d", dataLen);

            CHIPoBLEProfile_SetParameter(CHIPOBLEPROFILE_TX_CHAR, dataLen, (void *) msg->pData, BLEManagerImpl::sSelfEntity);
        }
#ifdef TI_APPOBLE_ENABLE
        else
        {
            uint8_t status;

            BLEMGR_LOG("BLEMGR: AppoBLE Char ");

            status = AppoBLE_GenericSetParameter(msg->conId, &msg->svcId, &msg->charId, dataLen, (void *) msg->pData,
                                                 BLEManagerImpl::sSelfEntity);

            HandleAppoBLEWriteConfirmation(msg->conId, &msg->svcId, &msg->charId, status);
        }
#endif
        if (NULL != msg->pData)
        {
            ICall_free((void *) msg->pData);
        }
        if (NULL != msg->conId)
        {
            ICall_free((void *) msg->conId);
        }

        dealloc = TRUE;
    }
    break;
#ifdef TI_APPOBLE_ENABLE
    case BLEMGR_APPOBLE_CHAR_READ_REQ: {
        /* MatteroBLE Event*/
        BLECharApi_msg * msg   = ((BLECharApi_msg *) (pMsg->pData));
        uint8_t status         = FAILURE;
        uint8_t const * dstBuf = NULL;

        if (!UUIDsMatch(&CHIP_BLE_SVC_ID, &msg->svcId))
        {
            BLEMGR_LOG("BLEMGR:  BLEMGR_APPOBLE_CHAR_READ_REQ ");
            /* Internal message read, due to characteristic value change */
            if (NULL == msg->pData)
            {
                BLEMGR_LOG("BLEMGR:  GENERIC_CHAR_CHANGE_EVT ");
                uint8_t * dstBuf = (uint8_t *) ICall_malloc(msg->len);

                status = AppoBLE_GenericGetParameter(msg->conId, &msg->svcId, &msg->charId, msg->len, (void *) dstBuf);
                HandleAppoBLEWriteReceived(msg->conId, &msg->svcId, &msg->charId, dstBuf, msg->len, status);

                /* Message data didn't contain target buffer, free allocated buffer */
                if (dstBuf != NULL)
                {
                    ICall_free((void *) dstBuf);
                }
            }
            else
            {
                dstBuf = msg->pData;

                status = AppoBLE_GenericGetParameter(msg->conId, &msg->svcId, &msg->charId, msg->len, (void *) dstBuf);
                HandleAppoBLEReadConfirmation(msg->conId, &msg->svcId, &msg->charId, (const uint8_t *) dstBuf, msg->len, status);
            }
        }

        if (NULL != msg->conId)
        {
            ICall_free((void *) msg->conId);
        }

        dealloc = TRUE;
    }
    break;
    case BLEMGR_APPOBLE_CUSTOM_API_REQ: {
        AppoBLEGenericApi_msg * msg = ((AppoBLEGenericApi_msg *) (pMsg->pData));
        if (msg->FxnPtr != NULL)
        {
            /* Call requested function */
            msg->FxnPtr(msg->arg);
        }
        BLEMGR_LOG("BLEMGR: BLEMGR_APPOBLE_CUSTOM_API_REQ");

        dealloc = TRUE;
    }
    break;
#endif
    case BLEMGR_CHIPOBLE_CHAR_CHANGE: {
        uint16_t writeLen     = ((CHIPoBLEProfChgEvt_t *) (pMsg->pData))->len;
        uint8_t paramId       = ((CHIPoBLEProfChgEvt_t *) (pMsg->pData))->paramId;
        uint16_t connHandleId = ((CHIPoBLEProfChgEvt_t *) (pMsg->pData))->connHandle;
        void * connHandle;
        ChipDeviceEvent event;

        uint8_t i;
        ConnRec_t * activeConnObj = NULL;

        // Find active connection
        for (i = 0; i < MAX_NUM_BLE_CONNS; i++)
        {
            if (sInstance.connList[i].connHandle == connHandleId)
            {
                activeConnObj = &sInstance.connList[i];
            }
        }
        if (activeConnObj == NULL)
        {
            BLEMGR_LOG("BLEMGR: BLE Process Application Message: CHIPOBLE_CHAR_CHANGE_EVT, No active connection found");
            return;
        }

        connHandle = (void *) &activeConnObj->connHandle;

        if (paramId == CHIPOBLEPROFILE_RX_CHAR)
        {
            BLEMGR_LOG("BLEMGR: BLE Process Application Message: BLEMGR_CHIPOBLE_CHAR_CHANGE, CHIPOBLEPROFILE_RX_CHAR");
            // Pull written data from CHIPOBLE Profile based on extern server write
            uint8_t * rxBuf = (uint8_t *) ICall_malloc(writeLen);

            if (rxBuf == NULL)
            {
                // alloc error
                return;
            }

            memset(rxBuf, 0x00, writeLen);

            BLEMGR_LOG("BLEMGR: BLE Process Application Message: BLEMGR_CHIPOBLE_CHAR_CHANGE, length: %d", writeLen);
            CHIPoBLEProfile_GetParameter(CHIPOBLEPROFILE_RX_CHAR, rxBuf, writeLen);

            System::PacketBufferHandle packetBuf = System::PacketBufferHandle::NewWithData(rxBuf, writeLen, 0, 0);

            ICall_free(rxBuf);

            if (packetBuf.IsNull())
            {
                // alloc error
                return;
            }

            // Arrange to post a CHIPoBLERXWriteEvent event to the CHIP queue.
            event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
            event.CHIPoBLEWriteReceived.ConId = (void *) connHandle;
            event.CHIPoBLEWriteReceived.Data  = std::move(packetBuf).UnsafeRelease();
        }
        else if (paramId == CHIPOBLEPROFILE_CCCWrite)
        {
            BLEMGR_LOG("BLEMGR: BLE Process Application Message: BLEMGR_CHIPOBLE_CHAR_CHANGE, CHIPOBLEPROFILE_CCCWrite");

            // TODO: Add check to see if subscribing OR unsubscribing from char indications
            uint8_t cccValue;

            CHIPoBLEProfile_GetParameter(CHIPOBLEPROFILE_CCCWrite, &cccValue, 1);

            // Check whether it is a sub/unsub event. 0x1 = Notifications enabled, 0x2 = Indications enabled
            if (cccValue & 0x2)
            {
                // Post event to CHIP
                BLEMGR_LOG("BLEMGR: BLE Process Application Message: BLEMGR_CHIPOBLE_CHAR_CHANGE, Subscrbe");
                event.Type = DeviceEventType::kCHIPoBLESubscribe;
            }
            else
            {
                BLEMGR_LOG("BLEMGR: BLE Process Application Message: BLEMGR_CHIPOBLE_CHAR_CHANGE, unsubscrbe");
                event.Type = DeviceEventType::kCHIPoBLEUnsubscribe;
            }

            // Post event to CHIP
            event.CHIPoBLESubscribe.ConId = (void *) connHandle;
        }
        PlatformMgr().PostEventOrDie(&event);
    }
    break;

    case BLEMGR_ADV_CB_UPDATE:
        ProcessAdvEvent((GapAdvEventData_t *) (pMsg->pData));
        break;

    case BLEMGR_PAIR_STATE_CB_UPDATE: {
        BLEMGR_LOG("BLEMGR: BLEMGR_PAIR_STATE_CB_UPDATE: State: %d Status: %d", ((PairStateData_t *) (pMsg->pData))->state,
                   ((PairStateData_t *) (pMsg->pData))->status);
    }
    break;

    case BLEMGR_PASSCODE_CB_UPDATE: {
        BLEMGR_LOG("BLEMGR: BLEMGR_PASSCODE_CB_UPDATE: Connection ID: %d", ((PasscodeData_t *) (pMsg->pData))->connHandle);

        // Send passcode response
        GAPBondMgr_PasscodeRsp(((PasscodeData_t *) (pMsg->pData))->connHandle, SUCCESS, pairingPasscode);
    }
    break;

    case BLEMGR_RPA_CB_UPDATE:
        UpdateBLERPA();
        break;

    case BLEMGR_CONN_PARAM_CB_UPDATE: {
        // Extract connection handle from data
        uint16_t connHandle = *(uint16_t *) (((ClockEventData_t *) pMsg->pData)->data);

        if (CHIP_NO_ERROR != ProcessParamUpdate(connHandle))
        {
            // error
            return;
        }

        // This data is not dynamically allocated
        dealloc = FALSE;
        break;
    }

    default:
        // Do nothing.
        break;
    }
#ifdef TI_APPOBLE_ENABLE
    AppoBLEHook_ProcessBleMgrEvt((const GenericQueuedEvt_t *) pMsg);
#endif
    // Free message data if it exists and we are to dealloc
    if ((dealloc == TRUE) && (pMsg->pData != NULL))
    {
        ICall_free(pMsg->pData);
    }
}

/*********************************************************************
 * @fn      ProcessGapMessage
 *
 * @brief   Process an incoming GAP event.
 *
 * @param   pMsg - message to process
 */
void BLEManagerImpl::ProcessGapMessage(gapEventHdr_t * pMsg)
{
    BLEMGR_LOG("BLEMGR: ProcessGapMessage");

    switch (pMsg->opcode)
    {
    case GAP_DEVICE_INIT_DONE_EVENT: {
        BLEMGR_LOG("BLEMGR: ProcessGapMessage: GAP_DEVICE_INIT_DONE_EVENT");

        gapDeviceInitDoneEvent_t * pPkt = (gapDeviceInitDoneEvent_t *) pMsg;

        if (pPkt->hdr.status == SUCCESS)
        {
            // Store the system ID
            uint8_t systemId[DEVINFO_SYSTEM_ID_LEN];

            // use 6 bytes of device address for 8 bytes of system ID value
            systemId[0] = pPkt->devAddr[0];
            systemId[1] = pPkt->devAddr[1];
            systemId[2] = pPkt->devAddr[2];

            // set middle bytes to zero
            systemId[4] = 0x00;
            systemId[3] = 0x00;

            // shift three bytes up
            systemId[7] = pPkt->devAddr[5];
            systemId[6] = pPkt->devAddr[4];
            systemId[5] = pPkt->devAddr[3];

            // Set Device Info Service Parameter
            DevInfo_SetParameter(DEVINFO_SYSTEM_ID, DEVINFO_SYSTEM_ID_LEN, systemId);

            // Create Matter advertisement set
            AddUpdateMatteroBLEAdv();
            sInstance.mAdvSetArray[sInstance.matterAdvIndex].advState.Set(AdvFlags::kAdvertisingRefreshNeeded);

            sInstance.mFlags.Set(Flags::kBLEStackInitialized);

            /* Trigger post-initialization state update */
            DriveBLEState();

            if (sInstance.addrMode > ADDRMODE_RANDOM)
            {
                UpdateBLERPA();
                // Create one-shot clock for RPA check event.
                Util_constructClock(&sInstance.clkRpaRead, ClockHandler, BLEMGR_RPA_CB_UPDATE_PERIOD, 0, true,
                                    (uintptr_t) &sInstance.argRpaRead);
            }
        }
        break;
    }

    case GAP_LINK_ESTABLISHED_EVENT: {
        gapEstLinkReqEvent_t * pPkt = (gapEstLinkReqEvent_t *) pMsg;

        // Display the amount of current connections
        uint8_t numActive = (uint8_t) linkDB_NumActive("");
        BLEMGR_LOG("BLEMGR: ProcessGapMessage: GAP_LINK_ESTABLISHED_EVENT: Num Connections: %d", numActive);

        if (pPkt->hdr.status == SUCCESS)
        {
            // Add connection to list and start RSSI
            AddBLEConn(pPkt->connectionHandle);
        }

        if (numActive >= MAX_NUM_BLE_CONNS)
        {
            // Stop advertising since there is no room for more connections
            uint8_t pLen;
            uint16_t pValue;
            BLEMGR_LOG("BLEMGR: BLE event GAP_LINK_ESTABLISHED_EVENT: Max connections reached");

            for (uint8_t currAdvIndex = 0; currAdvIndex < MAX_NUM_ADV_SETS; currAdvIndex++)
            {
                if (mAdvSetArray[currAdvIndex].advHandle != ADV_INDEX_INVALID)
                {
                    GapAdv_disable(mAdvSetArray[currAdvIndex].advHandle);

                    mAdvSetArray[currAdvIndex].advState.Clear(AdvFlags::kAdvertisingEnabled).Set(AdvFlags::kFastAdvertisingEnabled);
                    GapAdv_getParam(mAdvSetArray[currAdvIndex].advHandle, GAP_ADV_PARAM_PROPS, &pValue, &pLen);

                    // Legacy Advertisement, connectable/scannable
                    if (pValue & GAP_ADV_PROP_LEGACY)
                    {
                        pValue &= ~(GAP_ADV_PROP_CONNECTABLE | GAP_ADV_PROP_SCANNABLE);
                    }
                    else // Ext advertisement connectable/scannable
                    {
                        pValue &= ~(GAP_ADV_PROP_CONNECTABLE);
                    }

                    GapAdv_setParam(mAdvSetArray[currAdvIndex].advHandle, GAP_ADV_PARAM_PROPS, (void *) &pValue);
                }
            }

            /* Stop Matter advertisement timeout timer */
            Util_stopClock(&sInstance.clkAdvTimeout);
        }

        DriveBLEState();

        break;
    }

    case GAP_LINK_TERMINATED_EVENT: {
        gapTerminateLinkEvent_t * pPkt = (gapTerminateLinkEvent_t *) pMsg;
        BLEMGR_LOG("BLEMGR: ProcessGapMessage: GAP_LINK_TERMINATED_EVENT, reason: %d", pPkt->reason);

        // Remove the connection from the list and disable RSSI if needed
        RemoveBLEConn(pPkt->connectionHandle);
        for (uint8_t currAdvIndex = 0; currAdvIndex < MAX_NUM_ADV_SETS; currAdvIndex++)
        {
            if (mAdvSetArray[currAdvIndex].advHandle != ADV_INDEX_INVALID)
            {
                GapAdv_disable(mAdvSetArray[currAdvIndex].advHandle);

                // Restore original command properties
                GapAdv_setParam(mAdvSetArray[currAdvIndex].advHandle, GAP_ADV_PARAM_PROPS,
                                (void *) &mAdvSetArray[currAdvIndex].advProps);
            }
        }

        ChipDeviceEvent event;
        event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId  = (void *) &pPkt->connectionHandle;
        event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
        PlatformMgr().PostEventOrDie(&event);

        DriveBLEState();

        break;
    }

    case GAP_UPDATE_LINK_PARAM_REQ_EVENT: {
        BLEMGR_LOG("BLEMGR: ProcessGapMessage: GAP_UPDATE_LINK_PARAM_REQ_EVENT");

        gapUpdateLinkParamReqReply_t rsp;

        gapUpdateLinkParamReqEvent_t * pReq = (gapUpdateLinkParamReqEvent_t *) pMsg;

        rsp.connectionHandle = pReq->req.connectionHandle;
        rsp.signalIdentifier = pReq->req.signalIdentifier;

        // Only accept connection intervals with slave latency of 0
        // This is just an example of how the application can send a response
        if (pReq->req.connLatency == 0)
        {
            rsp.intervalMin = pReq->req.intervalMin;
            rsp.intervalMax = pReq->req.intervalMax;
            rsp.connLatency = pReq->req.connLatency;
            rsp.connTimeout = pReq->req.connTimeout;
            rsp.accepted    = TRUE;
            BLEMGR_LOG("BLEMGR: ProcessGapMessage: GAP_UPDATE_LINK_PARAM_REQ_EVENT Accecpted");
        }
        else
        {
            rsp.accepted = FALSE;
            BLEMGR_LOG("BLEMGR: ProcessGapMessage: GAP_UPDATE_LINK_PARAM_REQ_EVENT Rejected");
        }

        // Send Reply
        VOID GAP_UpdateLinkParamReqReply(&rsp);

        break;
    }

    case GAP_LINK_PARAM_UPDATE_EVENT: {
        BLEMGR_LOG("BLEMGR: ProcessGapMessage: GAP_LINK_PARAM_UPDATE_EVENT");

        gapLinkUpdateEvent_t * pPkt = (gapLinkUpdateEvent_t *) pMsg;

        // Get the address from the connection handle
        linkDBInfo_t linkInfo;
        linkDB_GetInfo(pPkt->connectionHandle, &linkInfo);

        // Check if there are any queued parameter updates
        ConnHandleEntry_t * connHandleEntry = (ConnHandleEntry_t *) List_get(&sInstance.paramUpdateList);
        if (connHandleEntry != NULL)
        {
            // Attempt to send queued update now
            ProcessParamUpdate(connHandleEntry->connHandle);

            // Free list element
            ICall_free(connHandleEntry);
        }

        break;
    }

    default:
        break;
    }
#ifdef TI_APPOBLE_ENABLE
    AppoBLEHook_ProcessGapMessage((const gapEventHdr_t *) pMsg);
#endif
}

/*********************************************************************
 * @fn      ProcessGATTMsg
 *
 * @brief   Process GATT messages and events.
 *
 * @return  TRUE if safe to deallocate incoming message, FALSE otherwise.
 */
uint8_t BLEManagerImpl::ProcessGATTMsg(gattMsgEvent_t * pMsg)
{
    uint8_t index;
    BLEMGR_LOG("BLEMGR: ProcessGATTMsg");

    if (pMsg->method == ATT_FLOW_CTRL_VIOLATED_EVENT)
    {
        // ATT request-response or indication-confirmation flow control is
        // The app is informed in case it wants to drop the connection.
    }
    else if (pMsg->method == ATT_MTU_UPDATED_EVENT)
    {
        index = GetBLEConnIndex(pMsg->connHandle);

        sInstance.connList[index].mtu = pMsg->msg.mtuEvt.MTU;
        BLEMGR_LOG("BLEMGR: ProcessGATTMsg, ATT_MTU_UPDATED_EVENT: %d", pMsg->msg.mtuEvt.MTU);
    }
    else if (pMsg->method == ATT_HANDLE_VALUE_CFM)
    {
        uint8_t i;
        /* Issue Matter specific Event for previous CCC write request */
        for (i = 0; i < MAX_NUM_BLE_CONNS; i++)
        {
            if (sInstance.connList[i].connHandle != LL_CONNHANDLE_INVALID)
            {
                if (sInstance.connList[i].mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled)
                {
                    ChipDeviceEvent event;
                    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
                    event.CHIPoBLEIndicateConfirm.ConId = &sInstance.connList[i].connHandle;
                    PlatformMgr().PostEventOrDie(&event);
                    BLEMGR_LOG("BLEMGR: ProcessGATTMsg, MatteroBLE ATT_HANDLE_VALUE_CFM for connection %d",
                               sInstance.connList[i].connHandle);
                    break;
                }
#ifdef TI_APPOBLE_ENABLE
                /* Do not issue callback when processing Matter Data, or for unestablished connections */
                if (sInstance.connList[i].mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Disabled)
                {
                    BLEMGR_LOG("BLEMGR: ProcessGATTMsg, AppoBLE ATT_HANDLE_VALUE_CFM for connection %d",
                               sInstance.connList[i].connHandle);
                    HandleAppoBLEIndicationConfirmation(&sInstance.connList[i].connHandle);
                }
#endif
            }
        }
    }
#ifdef TI_APPOBLE_ENABLE
    AppoBLEHook_ProcessGattMessage((const gattMsgEvent_t *) pMsg);
#endif
    // Free message payload. Needed only for ATT Protocol messages
    GATT_bm_free(&pMsg->msg, pMsg->method);

    // It's safe to free the incoming message
    return TRUE;
}

uint8_t BLEManagerImpl::getAdvIndex(uint8_t advHandle)
{
    uint8_t index;
    for (index = 0; index < MAX_NUM_ADV_SETS; index++)
    {
        if (mAdvSetArray[index].advHandle == advHandle)
        {
            break;
        }
    }

    if (index > MAX_NUM_ADV_SETS)
    {
        index = ADV_INDEX_INVALID;
    }

    return index;
}

/*********************************************************************
 * @fn      ProcessAdvEvent
 *
 * @brief   Process advertising event in app context
 *
 * @param   pEventData
 */
void BLEManagerImpl::ProcessAdvEvent(GapAdvEventData_t * pEventData)
{
    BLEMGR_LOG("BLEMGR: ProcessAdvEvent: EVT %d", pEventData->event);
    uint8_t advHandle, advIndex;
    switch (pEventData->event)
    {
    case GAP_EVT_ADV_START_AFTER_ENABLE: {
        advHandle = *((uint8 *) pEventData->pBuf);

        advIndex = getAdvIndex(advHandle);
        mAdvSetArray[advIndex].advState.Set(AdvFlags::kAdvertising);
        BLEMGR_LOG("BLEMGR: ProcessAdvEvent: GAP_EVT_ADV_START_AFTER_ENABLE Handle %d", advIndex);
    }
    break;
    case GAP_EVT_ADV_END_AFTER_DISABLE: {
        advHandle = *((uint8 *) pEventData->pBuf);
        advIndex  = getAdvIndex(advHandle);
        mAdvSetArray[advIndex].advState.Clear(AdvFlags::kAdvertising).Set(AdvFlags::kAdvertisingRefreshNeeded);

        BLEMGR_LOG("BLEMGR: ProcessAdvEvent: GAP_EVT_ADV_END_AFTER_DISABLE Handle %d", advIndex);
    }
    break;

    case GAP_EVT_ADV_START:
        break;

    case GAP_EVT_ADV_END:
        break;

    // BLE Stack has ended advertising due to connection
    case GAP_EVT_ADV_SET_TERMINATED: {
        /* TODO FIX ME, incorrect handle
         advHandle = ((GapAdv_setTerm_t *) pEventData)->handle;
         advIndex  = getAdvIndex(advHandle);
         mAdvSetArray[advIndex].advState.Clear(AdvFlags::kAdvertising).Set(AdvFlags::kAdvertisingRefreshNeeded);
         BLEMGR_LOG("BLEMGR: ProcessAdvEvent: GAP_EVT_ADV_SET_TERMINATED Handle %d", advIndex);
         */
    }
    break;

    case GAP_EVT_SCAN_REQ_RECEIVED:
        break;

    case GAP_EVT_INSUFFICIENT_MEMORY:
        break;

    default:
        break;
    }

    // All events have associated memory to free except the insufficient memory
    // event
    if (pEventData->event != GAP_EVT_INSUFFICIENT_MEMORY)
    {
        ICall_free(pEventData->pBuf);
    }
}

/*********************************************************************
 * @fn      ProcessParamUpdate
 *
 * @brief   Process a parameters update request
 *
 * @return  None
 */
CHIP_ERROR BLEManagerImpl::ProcessParamUpdate(uint16_t connHandle)
{
    gapUpdateLinkParamReq_t req;
    uint8_t connIndex;
    BLEMGR_LOG("BLEMGR: ProcessParamUpdate");

    req.connectionHandle = connHandle;
    req.connLatency      = DEFAULT_DESIRED_PERIPHERAL_LATENCY;
    req.connTimeout      = DEFAULT_DESIRED_CONN_TIMEOUT;
    req.intervalMin      = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    req.intervalMax      = DEFAULT_DESIRED_MAX_CONN_INTERVAL;

    connIndex = GetBLEConnIndex(connHandle);
    if (!(connIndex < MAX_NUM_BLE_CONNS))
    {
        return CHIP_ERROR_TOO_MANY_CONNECTIONS;
    }

    // Deconstruct the clock object
    ClockP_destruct(sInstance.connList[connIndex].pUpdateClock);

    // Free clock struct
    if (sInstance.connList[connIndex].pUpdateClock != NULL)
    {
        ICall_free(sInstance.connList[connIndex].pUpdateClock);
        sInstance.connList[connIndex].pUpdateClock = NULL;
    }
    // Free ParamUpdateEventData
    if (sInstance.connList[connIndex].pParamUpdateEventData != NULL)
    {
        ICall_free(sInstance.connList[connIndex].pParamUpdateEventData);
    }

    BLEMGR_LOG("BLEMGR: ProcessParamUpdate Send Link Param Req");
    // Send parameter update
    bStatus_t status = GAP_UpdateLinkParamReq(&req);

    // If there is an ongoing update, queue this for when the udpate completes
    if (status == bleAlreadyInRequestedMode)
    {

        BLEMGR_LOG("BLEMGR: ProcessParamUpdate pending");
        ConnHandleEntry_t * connHandleEntry = (ConnHandleEntry_t *) (ICall_malloc(sizeof(ConnHandleEntry_t)));
        if (connHandleEntry)
        {
            connHandleEntry->connHandle = connHandle;
            List_put(&sInstance.paramUpdateList, (List_Elem *) connHandleEntry);
        }
    }

    return CHIP_NO_ERROR;
}

/*********************************************************************
 * @fn      EnqueueBLEMgrMsg
 *
 * @brief   Creates a message and puts the message in BLE Manager queue.
 *
 * @param   event - message event.
 * @param   state - message state.
 */
status_t BLEManagerImpl::EnqueueBLEMgrMsg(uint32_t event, void * pData)
{
    uint8_t success;

    if (sInstance.mFlags.Has(Flags::kBLEStackInitialized))
    {
        GenericQueuedEvt_t * pMsg = (GenericQueuedEvt_t *) ICall_malloc(sizeof(GenericQueuedEvt_t));

        // Create dynamic pointer to message.
        if (pMsg)
        {
            pMsg->event = event;
            pMsg->pData = pData;

            // Enqueue the message.
            success = Util_enqueueMsg(sEventHandlerMsgQueueID, BLEManagerImpl::sSyncEvent, (uint8_t *) pMsg);

            return (success) ? SUCCESS : FAILURE;
        }
        BLEMGR_LOG("BLEMGR: BLE EnqueueBLEMgrMsg: NULL");

        return bleMemAllocError;
    }
    else
    {
        BLEMGR_LOG("BLEMGR: BLE EnqueueBLEMgrMsg: fail");

        return false;
    }
}

/*********************************************************************
 * @fn      AddBLEConn
 *
 * @brief   Add a device to the connected device list
 *
 * @return  index of the connected device list entry where the new connection
 *          info is put in.
 *          if there is no room, MAX_NUM_BLE_CONNS will be returned.
 */
uint8_t BLEManagerImpl::AddBLEConn(uint16_t connHandle)
{
    uint8_t i;
    uint8_t status = bleNoResources;
    BLEMGR_LOG("BLEMGR: AddBLEConn");

    // Try to find an available entry
    for (i = 0; i < MAX_NUM_BLE_CONNS; i++)
    {
        if (sInstance.connList[i].connHandle == LL_CONNHANDLE_INVALID)
        {
            // Found available entry to put a new connection info in
            sInstance.connList[i].connHandle = connHandle;

            // Allocate data to send through clock handler
            sInstance.connList[i].pParamUpdateEventData =
                (ClockEventData_t *) ICall_malloc(sizeof(ClockEventData_t) + sizeof(uint16_t));
            if (sInstance.connList[i].pParamUpdateEventData)
            {
                sInstance.connList[i].pParamUpdateEventData->event                = BLEMGR_CONN_PARAM_CB_UPDATE;
                *((uint16_t *) sInstance.connList[i].pParamUpdateEventData->data) = connHandle;

                // Create a clock object and start
                sInstance.connList[i].pUpdateClock = (ClockP_Struct *) ICall_malloc(sizeof(ClockP_Struct));

                if (sInstance.connList[i].pUpdateClock)
                {
                    Util_constructClock(sInstance.connList[i].pUpdateClock, ClockHandler, SEND_PARAM_UPDATE_DELAY, 0, true,
                                        (uintptr_t) sInstance.connList[i].pParamUpdateEventData);
                }
                else
                {
                    ICall_free(sInstance.connList[i].pParamUpdateEventData);
                }
            }
            else
            {
                status = bleMemAllocError;
            }

            break;
        }
    }
    return status;
}

/*********************************************************************
 * @fn      RemoveBLEConn
 *
 * @brief   Remove a device from the connected device list
 *
 * @return  index of the connected device list entry where the new connection
 *          info is removed from.
 *          if connHandle is not found, MAX_NUM_BLE_CONNS will be returned.
 */
uint8_t BLEManagerImpl::RemoveBLEConn(uint16_t connHandle)
{
    uint8_t connIndex = GetBLEConnIndex(connHandle);
    BLEMGR_LOG("BLEMGR: RemoveBLEConn");

    if (connIndex != MAX_NUM_BLE_CONNS)
    {
        ClockP_Struct * pUpdateClock = sInstance.connList[connIndex].pUpdateClock;

        if (pUpdateClock != NULL)
        {
            // Stop and destruct the RTOS clock if it's still alive
            if (Util_isActive(pUpdateClock))
            {
                Util_stopClock(pUpdateClock);
            }

            // Destruct the clock object
            ClockP_destruct(pUpdateClock);
            // Free clock struct
            ICall_free(pUpdateClock);
            // Free ParamUpdateEventData
            ICall_free(sInstance.connList[connIndex].pParamUpdateEventData);
        }
        // Clear pending update requests from paramUpdateList
        ClearPendingBLEParamUpdate(connHandle);

        // Clear Connection List Entry
        ClearBLEConnListEntry(connHandle);
    }
    return connIndex;
}

/*********************************************************************
 * @fn      GetBLEConnIndex
 *
 * @brief   Find index in the connected device list by connHandle
 *
 * @return  the index of the entry that has the given connection handle.
 *          if there is no match, MAX_NUM_BLE_CONNS will be returned.
 */
uint8_t BLEManagerImpl::GetBLEConnIndex(uint16_t connHandle) const
{
    uint8_t i;

    BLEMGR_LOG("BLEMGR: GetBLEConnIndex");

    for (i = 0; i < MAX_NUM_BLE_CONNS; i++)
    {
        if (sInstance.connList[i].connHandle == connHandle)
        {
            return i;
        }
    }

    return MAX_NUM_BLE_CONNS;
}

/*********************************************************************
 * @fn      ClearBLEConnListEntry
 *
 * @brief   Find index in the connected device list by connHandle
 *
 * @return  the index of the entry that has the given connection handle.
 *          if there is no match, MAX_NUM_BLE_CONNS will be returned.
 */
uint8_t BLEManagerImpl::ClearBLEConnListEntry(uint16_t connHandle)
{
    uint8_t i;
    // Set to invalid connection index initially
    uint8_t connIndex = MAX_NUM_BLE_CONNS;

    BLEMGR_LOG("BLEMGR: ClearBLEConnListEntry");
    if (connHandle != LL_CONNHANDLE_ALL)
    {
        // Get connection index from handle
        connIndex = GetBLEConnIndex(connHandle);
        if (connIndex >= MAX_NUM_BLE_CONNS)
        {
            return bleInvalidRange;
        }
    }

    // Clear specific handle or all handles
    for (i = 0; i < MAX_NUM_BLE_CONNS; i++)
    {
        if ((connIndex == i) || (connHandle == LL_CONNHANDLE_ALL))
        {
            sInstance.connList[i].connHandle   = LL_CONNHANDLE_INVALID;
            sInstance.connList[i].currPhy      = 0;
            sInstance.connList[i].phyCngRq     = 0;
            sInstance.connList[i].phyRqFailCnt = 0;
            sInstance.connList[i].rqPhy        = 0;
            memset(sInstance.connList[i].rssiArr, 0, MAX_RSSI_STORE_DEPTH);
            sInstance.connList[i].rssiAvg         = 0;
            sInstance.connList[i].rssiCntr        = 0;
            sInstance.connList[i].isAutoPHYEnable = FALSE;
            sInstance.connList[i].mtu             = 0;
            sInstance.connList[i].mServiceMode    = ConnectivityManager::kCHIPoBLEServiceMode_NotSupported;
        }
    }

    return SUCCESS;
}

/*********************************************************************
 * @fn      ClearPendingBLEParamUpdate
 *
 * @brief   clean pending param update request in the paramUpdateList list
 *
 * @param   connHandle - connection handle to clean
 *
 * @return  none
 */
void BLEManagerImpl::ClearPendingBLEParamUpdate(uint16_t connHandle)
{
    List_Elem * curr;

    BLEMGR_LOG("BLEMGR: ClearPendingBLEParamUpdate");

    for (curr = List_head(&sInstance.paramUpdateList); curr != NULL; curr = List_next(curr))
    {
        if (((ConnHandleEntry_t *) curr)->connHandle == connHandle)
        {
            List_remove(&sInstance.paramUpdateList, curr);
        }
    }
}

/*********************************************************************
 * @fn      UpdateBLERPA
 *
 * @brief   Read the current RPA from the stack and update display
 *          if the RPA has changed.
 *
 * @param   None.
 *
 * @return  None.
 */
void BLEManagerImpl::UpdateBLERPA(void)
{
    uint8_t * pRpaNew;

    // Read the current RPA.
    pRpaNew = GAP_GetDevAddress(FALSE);

    if (memcmp(pRpaNew, sInstance.rpa, B_ADDR_LEN))
    {
        memcpy(sInstance.rpa, pRpaNew, B_ADDR_LEN);
        BLEMGR_LOG("BLE RP Address: %x:%x:%x:%x:%x:%x", pRpaNew[5], pRpaNew[4], pRpaNew[3], pRpaNew[2], pRpaNew[1], pRpaNew[0]);
    }
}

void BLEManagerImpl::EventHandler(void * arg)
{
    PlatformMgr().LockChipStack();
    sInstance.EventHandler_init();
    PlatformMgr().UnlockChipStack();

    for (;;)
    {
        uint32_t events;

        // Waits for an event to be posted associated with the calling thread.
        // Note that an event associated with a thread is posted when a
        // message is queued to the message receive queue of the thread
        xQueueReceive((QueueHandle_t) BLEManagerImpl::sSyncEvent, (char *) &events, portMAX_DELAY);

        if (events)
        {
            ICall_EntityID dest;
            ICall_ServiceEnum src;
            ICall_HciExtEvt * hcipMsg = NULL;

            // Fetch any available messages that might have been sent from the stack
            if (ICall_fetchServiceMsg(&src, &dest, (void **) &hcipMsg) == ICALL_ERRNO_SUCCESS)
            {
                uint8 safeToDealloc = TRUE;

                if ((src == ICALL_SERVICE_CLASS_BLE) && (dest == BLEManagerImpl::sSelfEntity))
                {
                    ICall_Stack_Event * pEvt = (ICall_Stack_Event *) hcipMsg;

                    // Check for non-BLE stack events
                    if (pEvt->signature != 0xffff)
                    {
                        // Process inter-task message
                        safeToDealloc = sInstance.ProcessBLEStackEvent((ICall_Hdr *) hcipMsg);
                    }
                }

                if (hcipMsg && safeToDealloc)
                {
                    ICall_freeMsg(hcipMsg);
                }
            }

            // If RTOS queue is not empty, process CHIP messages.
            if (events & QUEUE_EVT)
            {
                GenericQueuedEvt_t * pMsg;
                for (;;)
                {
                    pMsg = (GenericQueuedEvt_t *) Util_dequeueMsg(BLEManagerImpl::sEventHandlerMsgQueueID);
                    if (NULL != pMsg)
                    {
                        // Process message.
                        sInstance.ProcessBleMgrEvt(pMsg);

                        // Free the space from the message.
                        ICall_free(pMsg);
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }
    }
}

#ifdef TI_APPOBLE_ENABLE
bStatus_t BLEManagerImpl::EnqueueAppoBLEMsg(uint32_t event, void * pData)
{
    bStatus_t err = SUCCESS;
    BLEMGR_LOG("BLEMGR: EnqueueAppoBLEMsg");

    if (sInstance.EnqueueBLEMgrMsg(event, pData) != SUCCESS)
    {
        err = FAILURE;
    }
    return err;
}
#endif
/* Post event to app processing loop to begin CHIP advertising */
CHIP_ERROR BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    if (sInstance.EnqueueBLEMgrMsg(BLEMGR_ADV_STATE_UPDATE, NULL) != SUCCESS)
    {
        err = CHIP_ERROR_NO_MEMORY;
    }
    return err;
}

/*********************************************************************
 * @fn      advCallback
 *
 * @brief   GapAdv module callback
 *
 * @param   pMsg - message to process
 */
void BLEManagerImpl::advCallback(uint32_t event, void * pBuf, uintptr_t arg)
{
    BLEMGR_LOG("BLEMGR: advCallback");

    GapAdvEventData_t * pData = (GapAdvEventData_t *) ICall_malloc(sizeof(GapAdvEventData_t));

    if (pData)
    {
        pData->event = event;
        pData->pBuf  = pBuf;
        if (sInstance.EnqueueBLEMgrMsg(BLEMGR_ADV_CB_UPDATE, pData) != SUCCESS)
        {
            ICall_free(pData);
        }
    }
}

void BLEManagerImpl::AdvTimeoutHandler(uintptr_t arg)
{
    BLEMGR_LOG("BLEMGR: AdvTimeoutHandler");
    if ((sInstance.matterAdvIndex != ADV_INDEX_INVALID) &&
        (sInstance.mAdvSetArray[sInstance.matterAdvIndex].advState.Has(AdvFlags::kAdvertisingEnabled)))
    {
        if (sInstance.mAdvSetArray[sInstance.matterAdvIndex].advState.Has(AdvFlags::kFastAdvertisingEnabled))
        {
            BLEMGR_LOG("BLEMGR: Fast advertising timeout reached");
            sInstance.mAdvSetArray[sInstance.matterAdvIndex].advState.Clear(AdvFlags::kFastAdvertisingEnabled);
            sInstance.mAdvSetArray[sInstance.matterAdvIndex].advState.Set(AdvFlags::kAdvertisingRefreshNeeded);
        }
        else
        {
            BLEMGR_LOG("BLEMGR: Advertising timeout reached");
            sInstance.mAdvSetArray[sInstance.matterAdvIndex].advState.Clear(AdvFlags::kAdvertisingEnabled);
        }
        /* Send event to process state change request */
        DriveBLEState();
    }
}

void BLEManagerImpl::ClockHandler(uintptr_t arg)
{
    ClockEventData_t * pData = (ClockEventData_t *) arg;

    if (pData->event == BLEMGR_RPA_CB_UPDATE)
    {
        // Start the next period
        Util_startClock(&sInstance.clkRpaRead);

        // Post event to read the current RPA
        sInstance.EnqueueBLEMgrMsg(BLEMGR_RPA_CB_UPDATE, NULL);
    }
    else if (pData->event == BLEMGR_CONN_PARAM_CB_UPDATE)
    {
        // Send message to app
        if (sInstance.EnqueueBLEMgrMsg(BLEMGR_CONN_PARAM_CB_UPDATE, pData) != SUCCESS)
        {
            ICall_free(pData);
        }
    }
}

/*********************************************************************
 * @fn      CHIPoBLEProfile_charValueChangeCB
 *
 * @brief   Callback from CHIPoBLE Profile indicating a characteristic
 *          value change.
 *          Calling context (BLE Stack Task)
 *
 * @param   paramId - parameter Id of the value that was changed.
 *
 * @return  None.
 */
void BLEManagerImpl::CHIPoBLEProfile_charValueChangeCB(uint8_t paramId, uint16_t len, uint16_t connHandle)
{
    CHIPoBLEProfChgEvt_t * pValue = (CHIPoBLEProfChgEvt_t *) ICall_malloc(sizeof(CHIPoBLEProfChgEvt_t));
    BLEMGR_LOG("BLEMGR: CHIPoBLEProfile_charValueChangeCB");

    if (pValue)
    {
        pValue->paramId    = paramId;
        pValue->len        = len;
        pValue->connHandle = connHandle;

        if (sInstance.EnqueueBLEMgrMsg(BLEMGR_CHIPOBLE_CHAR_CHANGE, pValue) != SUCCESS)
        {
            ICall_free(pValue);
        }
    }
}

/*********************************************************************
 * @fn      RemoteDisplay_passcodeCb
 *
 * @brief   Passcode callback.
 *
 * @return  none
 */
void BLEManagerImpl::PasscodeCb(uint8_t * pDeviceAddr, uint16_t connHandle, uint8_t uiInputs, uint8_t uiOutputs,
                                uint32_t numComparison)
{
    PasscodeData_t * pData = (PasscodeData_t *) ICall_malloc(sizeof(PasscodeData_t));

    // Allocate space for the passcode event.
    if (pData)
    {
        pData->connHandle = connHandle;
        memcpy(pData->deviceAddr, pDeviceAddr, B_ADDR_LEN);
        pData->uiInputs      = uiInputs;
        pData->uiOutputs     = uiOutputs;
        pData->numComparison = numComparison;

        // Enqueue the event.
        if (sInstance.EnqueueBLEMgrMsg(BLEMGR_PASSCODE_CB_UPDATE, pData) != SUCCESS)
        {
            ICall_free(pData);
        }
    }
}

/*********************************************************************
 * @fn      PairStateCb
 *
 * @brief   Pairing state callback.
 *
 * @return  none
 */
void BLEManagerImpl::PairStateCb(uint16_t connHandle, uint8_t state, uint8_t status)
{
    PairStateData_t * pData = (PairStateData_t *) ICall_malloc(sizeof(PairStateData_t));

    // Allocate space for the event data.
    if (pData)
    {
        pData->state      = state;
        pData->connHandle = connHandle;
        pData->status     = status;

        // Queue the event.
        if (sInstance.EnqueueBLEMgrMsg(BLEMGR_PAIR_STATE_CB_UPDATE, pData) != SUCCESS)
        {
            ICall_free(pData);
        }
    }
}

/*******************************************************************************
 * @fn          AssertHandler
 *
 * @brief       This is the Application's callback handler for asserts raised
 *              in the stack.  When EXT_HAL_ASSERT is defined in the Stack
 *              project this function will be called when an assert is raised,
 *              and can be used to observe or trap a violation from expected
 *              behavior.
 *
 *              As an example, for Heap allocation failures the Stack will raise
 *              HAL_ASSERT_CAUSE_OUT_OF_MEMORY as the assertCause and
 *              HAL_ASSERT_SUBCAUSE_NONE as the assertSubcause.  An application
 *              developer could trap any malloc failure on the stack by calling
 *              HAL_ASSERT_SPINLOCK under the matching case.
 *
 *              An application developer is encouraged to extend this function
 *              for use by their own application.  To do this, add hal_assert.c
 *              to your project workspace, the path to hal_assert.h (this can
 *              be found on the stack side). Asserts are raised by including
 *              hal_assert.h and using macro HAL_ASSERT(cause) to raise an
 *              assert with argument assertCause.  the assertSubcause may be
 *              optionally set by macro HAL_ASSERT_SET_SUBCAUSE(subCause) prior
 *              to asserting the cause it describes. More information is
 *              available in hal_assert.h.
 *
 * input parameters
 *
 * @param       assertCause    - Assert cause as defined in hal_assert.h.
 * @param       assertSubcause - Optional assert subcause (see hal_assert.h).
 *
 * output parameters
 *
 * @param       None.
 *
 * @return      None.
 */
void BLEManagerImpl::AssertHandler(uint8 assertCause, uint8 assertSubcause)
{
    // check the assert cause
    switch (assertCause)
    {
    case HAL_ASSERT_CAUSE_OUT_OF_MEMORY:
        assert(false);
        break;

    case HAL_ASSERT_CAUSE_INTERNAL_ERROR:
        // check the subcause
        if (assertSubcause == HAL_ASSERT_SUBCAUSE_FW_INERNAL_ERROR)
        {
            assert(false);
        }
        else
        {
            assert(false);
        }
        break;
    case HAL_ASSERT_CAUSE_ICALL_ABORT:
        assert(false);

    case HAL_ASSERT_CAUSE_ICALL_TIMEOUT:
        assert(false);
        break;
    case HAL_ASSERT_CAUSE_WRONG_API_CALL:
        assert(false);
        break;
    default:
        assert(false);
        break;
    }
    return;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif /* CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE */
