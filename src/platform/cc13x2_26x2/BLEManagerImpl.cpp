
/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
#include <string.h>

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/CHIPBleServiceData.h>
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
#ifndef ICALL_FEATURE_SEPARATE_IMGINFO
#include <icall_addrs.h>
#endif /* ICALL_FEATURE_SEPARATE_IMGINFO */
}

// BLE Manager Debug Logs
extern "C" {
#ifdef BLEMGR_DBG_LOGGING
extern void cc13x2_26x2Log(const char * aFormat, ...);
#define BLEMGR_LOG(...) cc13x2_26x2Log(__VA_ARGS__);
#else
#define BLEMGR_LOG(...)
#endif
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

/* Static class member initialization */
BLEManagerImpl BLEManagerImpl::sInstance;
TaskHandle_t BLEManagerImpl::sBleTaskHndl;
ICall_EntityID BLEManagerImpl::sSelfEntity;
ICall_SyncHandle BLEManagerImpl::sSyncEvent;
QueueHandle_t BLEManagerImpl::sEventHandlerMsgQueueID;

chipOBleProfileCBs_t BLEManagerImpl::CHIPoBLEProfile_CBs = {
    // Provisioning GATT Characteristic value change callback
    CHIPoBLEProfile_charValueChangeCB
};

// GAP Bond Manager Callbacks
gapBondCBs_t BLEManagerImpl::BLEMgr_BondMgrCBs = {
    PasscodeCb, // Passcode callback
    PairStateCb // Pairing/Bonding state Callback
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

    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);

    mServiceMode             = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    OnChipBleConnectReceived = HandleIncomingBleConnection;

    err = CreateEventHandler();
    return err;
}

bool BLEManagerImpl::_IsAdvertisingEnabled(void)
{
    return mFlags.Has(Flags::kAdvertisingEnabled);
}

/* Post event to app processing loop to begin CHIP advertising */
CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    mFlags.Set(Flags::kAdvertisingEnabled, val);

    /* Send event to process state change request */
    return DriveBLEState();
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    CHIP_ERROR ret = CHIP_NO_ERROR;

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
    ret = DriveBLEState();
    return ret;
}

bool BLEManagerImpl::_IsAdvertising(void)
{
    return mFlags.Has(Flags::kAdvertising);
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    CHIP_ERROR ret = CHIP_NO_ERROR;

    if (bufSize <= GAP_DEVICE_NAME_LEN)
    {
        strncpy(buf, mDeviceName, bufSize);
    }
    else
    {
        ret = CHIP_ERROR_BUFFER_TOO_SMALL;
    }

    return ret;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    CHIP_ERROR ret = CHIP_NO_ERROR;

    if (strlen(deviceName) <= GAP_DEVICE_NAME_LEN)
    {
        strncpy(mDeviceName, deviceName, strlen(deviceName));

        mFlags.Set(Flags::kBLEStackGATTNameUpdate);
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);
        ret = DriveBLEState();
    }
    else
    {
        ret = CHIP_ERROR_BUFFER_TOO_SMALL;
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

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe: {
        ChipDeviceEvent connEstEvent;

        BLEMGR_LOG("BLEMGR: OnPlatformEvent, kCHIPoBLESubscribe");
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);

        connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;

        PlatformMgr().PostEventOrDie(&connEstEvent);
    }
    break;

    case DeviceEventType::kCHIPoBLEUnsubscribe: {
        BLEMGR_LOG("BLEMGR: OnPlatformEvent, kCHIPoBLEUnsubscribe");
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
    }
    break;

    case DeviceEventType::kCHIPoBLEWriteReceived: {
        BLEMGR_LOG("BLEMGR: OnPlatformEvent, kCHIPoBLEWriteReceived");
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_RX,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
    }
    break;

    case DeviceEventType::kCHIPoBLEIndicateConfirm:
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
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
bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    void * pMsg = (void *) ICall_malloc(sizeof(void *));
    pMsg        = (void *) conId;

    EnqueueEvtHdrMsg(BLEManagerIMPL_CHIPOBLE_CLOSE_CONN_EVT, (void *) pMsg);

    return false;
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

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    BLEMGR_LOG("BLEMGR: BLE SendIndication ");

    // Allocate buffers to send to BLE app task
    uint8_t dataLen = static_cast<uint8_t>(data->DataLength());
    CHIPoBLEIndEvt_t * pMsg;
    uint8_t * pBuf;

    pMsg = (CHIPoBLEIndEvt_t *) ICall_malloc(sizeof(CHIPoBLEIndEvt_t));
    if (NULL == pMsg)
    {
        return false;
    }

    pBuf = (uint8_t *) ICall_malloc(dataLen);
    if (NULL == pBuf)
    {
        ICall_free((void *) pMsg);
        return false;
    }

    memset(pBuf, 0x00, dataLen);
    memcpy(pBuf, data->Start(), dataLen);

    pMsg->pData = pBuf;
    pMsg->len   = dataLen;

    EnqueueEvtHdrMsg(BLEManagerIMPL_CHIPOBLE_TX_IND_EVT, (void *) pMsg);

    BLEMGR_LOG("BLEMGR: BLE SendIndication RETURN, Length: %d ", dataLen);
    return true;
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                             const Ble::ChipBleUUID * charId)
{
    /* Unsupported on TI peripheral device implementation */
    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                               const Ble::ChipBleUUID * charId)
{
    /* Unsupported on TI peripheral device implementation */
    return false;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBufferHandle pBuf)
{
    /* Unsupported on TI peripheral device implementation */
    BLEMGR_LOG("BLEMGR: BLE SendWriteRequest");
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     PacketBufferHandle pBuf)
{
    /* Unsupported on TI peripheral device implementation */
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    /* Unsupported on TI peripheral device implementation */
    return false;
}

void BLEManagerImpl::HandleIncomingBleConnection(BLEEndPoint * bleEP)
{
    BLEMGR_LOG("BLEMGR: HandleIncomingBleConnection");
}

// ===== Helper Members that implement the Low level BLE Stack behavior.

/*********************************************************************
 * @fn      ConfigureAdvertisements
 *
 * @brief   Initialize CHIPoBLE Advertisements.
 */
void BLEManagerImpl::ConfigureAdvertisements(void)
{
    bStatus_t status = FAILURE;
    uint16_t deviceDiscriminator;
    uint8_t localDeviceNameLen;
    uint8_t scanIndex = 0;
    uint8_t advIndex  = 0;
    uint8_t scanResLength;
    uint8_t advLength;

    GapAdv_params_t advParams = { .eventProps   = GAP_ADV_PROP_CONNECTABLE | GAP_ADV_PROP_LEGACY | GAP_ADV_PROP_SCANNABLE,
                                  .primIntMin   = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN,
                                  .primIntMax   = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN,
                                  .primChanMap  = GAP_ADV_CHAN_ALL,
                                  .peerAddrType = PEER_ADDRTYPE_PUBLIC_OR_PUBLIC_ID,
                                  .peerAddr     = { 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa },
                                  .filterPolicy = GAP_ADV_WL_POLICY_ANY_REQ,
                                  .txPower      = GAP_ADV_TX_POWER_NO_PREFERENCE,
                                  .primPhy      = GAP_ADV_PRIM_PHY_1_MBPS,
                                  .secPhy       = GAP_ADV_SEC_PHY_1_MBPS,
                                  .sid          = 0 };

    BLEMGR_LOG("BLEMGR: ConfigureAdvertisements");

    ChipBLEDeviceIdentificationInfo mDeviceIdInfo;
    ConfigurationMgr().GetBLEDeviceIdentificationInfo(mDeviceIdInfo);

    memset(sInstance.mScanResDatachipOBle, 0, CHIPOBLE_ADV_DATA_MAX_SIZE);
    memset(sInstance.mAdvDatachipOBle, 0, CHIPOBLE_ADV_DATA_MAX_SIZE);

    // Verify device name was not already set
    if (!sInstance.mFlags.Has(Flags::kBLEStackGATTNameSet))
    {
        /* Default device name is CHIP-<DISCRIMINATOR> */
        deviceDiscriminator = mDeviceIdInfo.GetDeviceDiscriminator();

        localDeviceNameLen = strlen(CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX) + CHIPOBLE_DEVICE_DESC_LENGTH;

        memset(sInstance.mDeviceName, 0, GAP_DEVICE_NAME_LEN);
        snprintf(sInstance.mDeviceName, GAP_DEVICE_NAME_LEN, "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX,
                 deviceDiscriminator);

        // Set the Device Name characteristic in the GAP GATT Service
        // For more information, see the section in the User's Guide:
        // http://software-dl.ti.com/lprf/ble5stack-latest/
        GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, sInstance.mDeviceName);

        BLEMGR_LOG("BLEMGR: AdvInit New device name set: %s", sInstance.mDeviceName);
    }
    else
    {
        sInstance.mFlags.Clear(Flags::kBLEStackGATTNameUpdate);

        localDeviceNameLen = strlen(sInstance.mDeviceName);
    }

    scanResLength = localDeviceNameLen + CHIPOBLE_SCANRES_SIZE_NO_NAME;

    /* Verify scan response data length */
    assert(scanResLength < CHIPOBLE_ADV_DATA_MAX_SIZE);

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

    advLength = sizeof(static_cast<uint16_t>(CHIPOBLE_SERV_UUID)) + static_cast<uint8_t>(sizeof(mDeviceIdInfo)) + 1;

    /* Verify advertising data length */
    assert((CHIPOBLE_ADV_SIZE_NO_DEVICE_ID_INFO + advLength) < CHIPOBLE_ADV_DATA_MAX_SIZE);

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

    // Setup and start Advertising
    // For more information, see the GAP section in the User's Guide:
    // http://software-dl.ti.com/lprf/ble5stack-latest/

    if (!sInstance.mFlags.Has(Flags::kBLEStackAdvInitialized))
    {
        // Create Advertisement set #1 and assign handle
        status = (bStatus_t) GapAdv_create(&advCallback, &advParams, &sInstance.advHandleLegacy);
        assert(status == SUCCESS);

        // Set event mask for set #1
        status = (bStatus_t) GapAdv_setEventMask(sInstance.advHandleLegacy,
                                                 GAP_ADV_EVT_MASK_START_AFTER_ENABLE | GAP_ADV_EVT_MASK_END_AFTER_DISABLE |
                                                     GAP_ADV_EVT_MASK_SET_TERMINATED);

        Util_constructClock(&sInstance.clkAdvTimeout, AdvTimeoutHandler, ADV_TIMEOUT, 0, false, (uintptr_t) NULL);
    }
    if (sInstance.mFlags.Has(Flags::kBLEStackAdvInitialized))

    {

        // Don't free anything since we're going to use the same buffer to re-load
        GapAdv_prepareLoadByHandle(sInstance.advHandleLegacy, GAP_ADV_FREE_OPTION_DONT_FREE);
    }
    // Load advertising data for set #1 that is statically allocated by the app
    status = (bStatus_t) GapAdv_loadByHandle(sInstance.advHandleLegacy, GAP_ADV_DATA_TYPE_ADV,
                                             CHIPOBLE_ADV_SIZE_NO_DEVICE_ID_INFO + advLength, sInstance.mAdvDatachipOBle);
    assert(status == SUCCESS);

    if (sInstance.mFlags.Has(Flags::kBLEStackAdvInitialized))
    {

        // Don't free anything since we're going to use the same buffer to re-load
        GapAdv_prepareLoadByHandle(sInstance.advHandleLegacy, GAP_ADV_FREE_OPTION_DONT_FREE);
    }

    // Load scan response data for set #1 that is statically allocated by the app
    status = (bStatus_t) GapAdv_loadByHandle(sInstance.advHandleLegacy, GAP_ADV_DATA_TYPE_SCAN_RSP, scanResLength,
                                             sInstance.mScanResDatachipOBle);
    assert(status == SUCCESS);
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
    BLEManagerImpl::sBleTaskHndl = (TaskHandle_t)(*((TaskHandle_t *) ICall_getRemoteTaskHandle(0)));
    DMMSch_registerClient((TaskHandle_t) BLEManagerImpl::sBleTaskHndl, DMMPolicy_StackRole_BlePeripheral);
    /* set the stacks in default states */
    DMMPolicy_updateStackState(DMMPolicy_StackRole_BlePeripheral, DMMPOLICY_BLE_IDLE);

    vTaskPrioritySet(xTaskGetCurrentTaskHandle(), 3);

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
    VOID GAPBondMgr_Register(BLEMgr_BondMgrCBs);

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
                            ICALL_TASK_PRIORITIES,               /* Keep priority the same as ICALL until init is complete */
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
uint8_t BLEManagerImpl::ProcessStackEvent(ICall_Hdr * pMsg)
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
        case HCI_COMMAND_COMPLETE_EVENT_CODE:
            // Process HCI Command Complete Events here
            {
                // RemoteDisplay_processCmdCompleteEvt((hciEvt_CmdComplete_t *) pMsg);
                break;
            }
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

        break;
    }

    default:
        // do nothing
        break;
    }

    return safeToDealloc;
}

/*********************************************************************
 * @fn      ProcessEvtHdrMsg
 *
 * @brief   Process an incoming callback from a profile.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
void BLEManagerImpl::ProcessEvtHdrMsg(QueuedEvt_t * pMsg)
{
    bool dealloc = TRUE;

    switch (pMsg->event)
    {
    /* External CHIPoBLE Event trigger */
    case BLEManagerIMPL_STATE_UPDATE_EVT: {
        bStatus_t status;

        /* Verify BLE service mode is enabled */
        if ((sInstance.mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled) &&
            sInstance.mFlags.Has(Flags::kBLEStackInitialized))
        {
            if (sInstance.mFlags.Has(Flags::kAdvertisingEnabled))
            {
                BLEMGR_LOG("BLEMGR: BLE Process Application Message: kAdvertisingEnabled");

                if (sInstance.mFlags.Has(Flags::kAdvertisingRefreshNeeded))
                {
                    BLEMGR_LOG("BLEMGR: BLE Process Application Message: kAdvertisingRefreshNeeded");

                    // Disable advertisements and proceed with updates
                    sInstance.mFlags.Clear(Flags::kAdvertisingRefreshNeeded);

                    GapAdv_disable(sInstance.advHandleLegacy);
                    sInstance.mFlags.Clear(Flags::kAdvertising);

                    uint32_t newParamMax = 0, newParamMin = 0;
                    if (sInstance.mFlags.Has(Flags::kFastAdvertisingEnabled))
                    {
                        // Update advertising interval
                        BLEMGR_LOG("BLEMGR: ConfigureAdvertisements: Fast Advertising Enabled");
                        newParamMax = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
                        newParamMin = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
                    }
                    else
                    {
                        // Decrease advertising interval
                        BLEMGR_LOG("BLEMGR: ConfigureAdvertisements: Slow Advertising Enabled");
                        newParamMax = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
                        newParamMin = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
                    }

                    // Set a parameter
                    GapAdv_setParam(sInstance.advHandleLegacy, GAP_ADV_PARAM_PRIMARY_INTERVAL_MAX, &newParamMax);
                    GapAdv_setParam(sInstance.advHandleLegacy, GAP_ADV_PARAM_PRIMARY_INTERVAL_MIN, &newParamMin);

                    // Update advertisement parameters
                    ConfigureAdvertisements();
                }
            }

            // Turn on advertisements
            if (sInstance.mFlags.Has(Flags::kAdvertisingEnabled) && !sInstance.mFlags.Has(Flags::kAdvertising))
            {
                // Send notification to thread manager that CHIPoBLE advertising is starting

                // Enable legacy advertising for set #1
                status = (bStatus_t) GapAdv_enable(sInstance.advHandleLegacy, GAP_ADV_ENABLE_OPTIONS_USE_MAX, 0);

                // If adverisement fails, keep flags set
                if (status == SUCCESS)
                {

                    // Start advertisement timeout timer
                    if (sInstance.mFlags.Has(Flags::kFastAdvertisingEnabled))
                    {
                        Util_rescheduleClock(&sInstance.clkAdvTimeout, CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
                    }
                    else
                    {
                        Util_rescheduleClock(&sInstance.clkAdvTimeout,
                                             ADV_TIMEOUT - CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
                    }
                    Util_startClock(&sInstance.clkAdvTimeout);

                    sInstance.mFlags.Set(Flags::kAdvertising);
                }
            }
            // Advertising should be disabled
            if ((!sInstance.mFlags.Has(Flags::kAdvertisingEnabled)) && sInstance.mFlags.Has(Flags::kAdvertising))
            {
                BLEMGR_LOG("BLEMGR: BLE Process Application Message: ADvertisements disabled");

                // Stop advertising
                GapAdv_disable(sInstance.advHandleLegacy);
                sInstance.mFlags.Clear(Flags::kAdvertising);

                Util_stopClock(&sInstance.clkAdvTimeout);

                // reset fast advertising
                sInstance.mFlags.Set(Flags::kFastAdvertisingEnabled);
            }
        }
    }
    break;

    case BLEManagerIMPL_CHIPOBLE_CLOSE_CONN_EVT: {
        uint16_t connHandle = *((uint16_t *) (pMsg->pData));

        // Close active connection
        GAP_TerminateLinkReq(connHandle, HCI_DISCONNECT_REMOTE_USER_TERM);
    }
    break;

    case BLEManagerIMPL_CHIPOBLE_TX_IND_EVT: {
        uint8_t dataLen = ((CHIPoBLEIndEvt_t *) (pMsg->pData))->len;

        CHIPoBLEProfile_SetParameter(CHIPOBLEPROFILE_TX_CHAR, dataLen, (void *) (((CHIPoBLEIndEvt_t *) (pMsg->pData))->pData),
                                     BLEManagerImpl::sSelfEntity);

        BLEMGR_LOG("BLEMGR: BLE Process Application Message: BLEManagerIMPL_CHIPOBLE_TX_IND_EVT: Length: %d", dataLen);

        ICall_free((void *) (((CHIPoBLEIndEvt_t *) (pMsg->pData))->pData));

        dealloc = TRUE;
    }
    break;

    case CHIPOBLE_CHAR_CHANGE_EVT: {
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

        connHandle = (void *) &activeConnObj->connHandle;

        if (paramId == CHIPOBLEPROFILE_RX_CHAR)
        {
            BLEMGR_LOG("BLEMGR: BLE Process Application Message: CHIPOBLE_CHAR_CHANGE_EVT, CHIPOBLEPROFILE_RX_CHAR");
            // Pull written data from CHIPOBLE Profile based on extern server write
            uint8_t * rxBuf = (uint8_t *) ICall_malloc(writeLen);

            if (rxBuf == NULL)
            {
                // alloc error
                return;
            }

            memset(rxBuf, 0x00, writeLen);

            BLEMGR_LOG("BLEMGR: BLE Process Application Message: CHIPOBLE_CHAR_CHANGE_EVT, length: %d", writeLen);
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
            BLEMGR_LOG("BLEMGR: BLE Process Application Message: CHIPOBLE_CHAR_CHANGE_EVT, CHIPOBLEPROFILE_CCCWrite");

            // TODO: Add check to see if subscribing OR unsubscribing from char indications
            uint8_t cccValue;

            CHIPoBLEProfile_GetParameter(CHIPOBLEPROFILE_CCCWrite, &cccValue, 1);

            // Check whether it is a sub/unsub event. 0x1 = Notifications enabled, 0x2 = Indications enabled
            if (cccValue & 0x2)
            {
                // Post event to CHIP
                BLEMGR_LOG("BLEMGR: BLE Process Application Message: CHIPOBLE_CHAR_CHANGE_EVT, Subscrbe");
                event.Type = DeviceEventType::kCHIPoBLESubscribe;
            }
            else
            {
                BLEMGR_LOG("BLEMGR: BLE Process Application Message: CHIPOBLE_CHAR_CHANGE_EVT, unsubscrbe");
                event.Type = DeviceEventType::kCHIPoBLEUnsubscribe;
            }

            // Post event to CHIP
            event.CHIPoBLESubscribe.ConId = (void *) connHandle;
        }
        PlatformMgr().PostEventOrDie(&event);
    }
    break;

    case ADV_EVT:
        ProcessAdvEvent((GapAdvEventData_t *) (pMsg->pData));
        break;

    case PAIR_STATE_EVT: {
        BLEMGR_LOG("BLEMGR: PAIR_STATE_EVT");

        // Send passcode response
        GAPBondMgr_PasscodeRsp(((PasscodeData_t *) (pMsg->pData))->connHandle, SUCCESS, B_APP_DEFAULT_PASSCODE);
    }
    break;

    case PASSCODE_EVT: {
        BLEMGR_LOG("BLEMGR: PASSCODE_EVT");
    }
    break;

    case READ_RPA_EVT:
        UpdateBLERPA();
        break;

    case SEND_PARAM_UPDATE_EVT: {
        // Extract connection handle from data
        uint16_t connHandle = *(uint16_t *) (((ClockEventData_t *) pMsg->pData)->data);

        if (CHIP_NO_ERROR != ProcessParamUpdate(connHandle))
        {
            // error
            return;
        }

        // This data is not dynamically allocated
        dealloc = FALSE;

        /* If we are sending a param update request then the service discovery
         * should have ended. Changed state to connected */
        DMMPolicy_updateStackState(DMMPolicy_StackRole_BlePeripheral, DMMPOLICY_BLE_CONNECTED);

        break;
    }

    case CONN_EVT:
        break;

    default:
        // Do nothing.
        break;
    }

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

            ConfigureAdvertisements();

            sInstance.mFlags.Set(Flags::kBLEStackInitialized);
            sInstance.mFlags.Set(Flags::kBLEStackAdvInitialized);

            /* Trigger post-initialization state update */
            DriveBLEState();

            if (sInstance.addrMode > ADDRMODE_RANDOM)
            {
                UpdateBLERPA();
                // Create one-shot clock for RPA check event.
                Util_constructClock(&sInstance.clkRpaRead, ClockHandler, READ_RPA_EVT_PERIOD, 0, true,
                                    (uintptr_t) &sInstance.argRpaRead);
            }
        }
        break;
    }

    case GAP_LINK_ESTABLISHED_EVENT: {
        gapEstLinkReqEvent_t * pPkt = (gapEstLinkReqEvent_t *) pMsg;
        BLEMGR_LOG("BLEMGR: ProcessGapMessage: GAP_LINK_ESTABLISHED_EVENT");

        // Display the amount of current connections
        uint8_t numActive = (uint8_t) linkDB_NumActive("");

        if (pPkt->hdr.status == SUCCESS)
        {
            // Add connection to list and start RSSI
            AddBLEConn(pPkt->connectionHandle);
        }

        DMMPolicy_updateStackState(DMMPolicy_StackRole_BlePeripheral, DMMPOLICY_BLE_HIGH_BANDWIDTH);

        if (numActive >= MAX_NUM_BLE_CONNS)
        {
            // Stop advertising since there is no room for more connections
            BLEMGR_LOG("BLEMGR: BLE event GAP_LINK_ESTABLISHED_EVENT: MAX connections");
            sInstance.mFlags.Clear(Flags::kAdvertisingEnabled).Clear(Flags::kAdvertising);
        }

        /* Stop advertisement timeout timer */
        Util_stopClock(&sInstance.clkAdvTimeout);
        // reset fast advertising
        sInstance.mFlags.Set(Flags::kFastAdvertisingEnabled);

        DriveBLEState();

        break;
    }

    case GAP_LINK_TERMINATED_EVENT: {
        gapTerminateLinkEvent_t * pPkt = (gapTerminateLinkEvent_t *) pMsg;
        BLEMGR_LOG("BLEMGR: ProcessGapMessage: GAP_LINK_TERMINATED_EVENT, reason: %d", pPkt->reason);

        // Remove the connection from the list and disable RSSI if needed
        RemoveBLEConn(pPkt->connectionHandle);

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
        void * connHandle;
        ChipDeviceEvent event;

        ConnRec_t * activeConnObj = NULL;

        activeConnObj = &sInstance.connList[0];
        connHandle    = (void *) &activeConnObj->connHandle;

        event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
        event.CHIPoBLEIndicateConfirm.ConId = connHandle;
        PlatformMgr().PostEventOrDie(&event);

        BLEMGR_LOG("BLEMGR: ProcessGATTMsg, ATT_HANDLE_VALUE_CFM:");
    }
    // Free message payload. Needed only for ATT Protocol messages
    GATT_bm_free(&pMsg->msg, pMsg->method);

    // It's safe to free the incoming message
    return TRUE;
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
    BLEMGR_LOG("BLEMGR: ProcessAdvEvent");

    switch (pEventData->event)
    {
    case GAP_EVT_ADV_START_AFTER_ENABLE: {
        BLEMGR_LOG("BLEMGR: ProcessAdvEvent: GAP_EVT_ADV_START_AFTER_ENABLE");

        if (linkDB_NumActive("") == 0)
        {
            DMMPolicy_updateStackState(DMMPolicy_StackRole_BlePeripheral, DMMPOLICY_BLE_ADV);
        }
    }
    break;

    case GAP_EVT_ADV_END_AFTER_DISABLE: {
        BLEMGR_LOG("BLEMGR: ProcessAdvEvent: GAP_EVT_ADV_END_AFTER_DISABLE");
    }
    break;

    case GAP_EVT_ADV_START:
        break;

    case GAP_EVT_ADV_END:
        break;

    // BLE Stack has ended advertising due to connection
    case GAP_EVT_ADV_SET_TERMINATED: {
        BLEMGR_LOG("BLEMGR: ProcessAdvEvent: GAP_EVT_ADV_SET_TERMINATED");
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
    req.connLatency      = DEFAULT_DESIRED_SLAVE_LATENCY;
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
 * @fn      EnqueueEvtHdrMsg
 *
 * @brief   Creates a message and puts the message in RTOS queue.
 *
 * @param   event - message event.
 * @param   state - message state.
 */
status_t BLEManagerImpl::EnqueueEvtHdrMsg(uint8_t event, void * pData)
{
    uint8_t success;

    if (sInstance.mFlags.Has(Flags::kBLEStackInitialized))
    {
        QueuedEvt_t * pMsg = (QueuedEvt_t *) ICall_malloc(sizeof(QueuedEvt_t));

        // Create dynamic pointer to message.
        if (pMsg)
        {
            pMsg->event = event;
            pMsg->pData = pData;

            // Enqueue the message.
            success = Util_enqueueMsg(sEventHandlerMsgQueueID, BLEManagerImpl::sSyncEvent, (uint8_t *) pMsg);

            return (success) ? SUCCESS : FAILURE;
        }

        return bleMemAllocError;
    }
    else
    {
        return true;
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
                sInstance.connList[i].pParamUpdateEventData->event                = SEND_PARAM_UPDATE_EVT;
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

            /* Lock CHIP Stack while processing BLE Stack/App events */
            PlatformMgr().LockChipStack();

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
                        safeToDealloc = sInstance.ProcessStackEvent((ICall_Hdr *) hcipMsg);
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
                QueuedEvt_t * pMsg;
                for (;;)
                {
                    pMsg = (QueuedEvt_t *) Util_dequeueMsg(BLEManagerImpl::sEventHandlerMsgQueueID);
                    if (NULL != pMsg)
                    {
                        // Process message.
                        sInstance.ProcessEvtHdrMsg(pMsg);

                        // Free the space from the message.
                        ICall_free(pMsg);
                    }
                    else
                    {
                        break;
                    }
                }
            }

            PlatformMgr().UnlockChipStack();
        }
    }
}

/* Post event to app processing loop to begin CHIP advertising */
CHIP_ERROR BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BLEMGR_LOG("BLEMGR: DriveBLEState");

    if (sInstance.EnqueueEvtHdrMsg(BLEManagerIMPL_STATE_UPDATE_EVT, NULL) != SUCCESS)
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
        if (sInstance.EnqueueEvtHdrMsg(ADV_EVT, pData) != SUCCESS)
        {
            ICall_free(pData);
        }
    }
}

void BLEManagerImpl::AdvTimeoutHandler(uintptr_t arg)
{
    BLEMGR_LOG("BLEMGR: AdvTimeoutHandler");

    if (sInstance.mFlags.Has(Flags::kAdvertisingEnabled))
    {
        if (sInstance.mFlags.Has(Flags::kFastAdvertisingEnabled))
        {
            BLEMGR_LOG("BLEMGR: Fast advertising timeout reached");

            sInstance.mFlags.Clear(Flags::kFastAdvertisingEnabled);
            sInstance.mFlags.Set(Flags::kAdvertisingRefreshNeeded);
        }
        else
        {
            BLEMGR_LOG("BLEMGR: Advertising timeout reached");

            sInstance.mFlags.Clear(Flags::kAdvertisingEnabled);
        }
        /* Send event to process state change request */
        DriveBLEState();
    }
}

void BLEManagerImpl::ClockHandler(uintptr_t arg)
{
    ClockEventData_t * pData = (ClockEventData_t *) arg;

    if (pData->event == READ_RPA_EVT)
    {
        // Start the next period
        Util_startClock(&sInstance.clkRpaRead);

        // Post event to read the current RPA
        sInstance.EnqueueEvtHdrMsg(READ_RPA_EVT, NULL);
    }
    else if (pData->event == SEND_PARAM_UPDATE_EVT)
    {
        // Send message to app
        if (sInstance.EnqueueEvtHdrMsg(SEND_PARAM_UPDATE_EVT, pData) != SUCCESS)
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

        if (sInstance.EnqueueEvtHdrMsg(CHIPOBLE_CHAR_CHANGE_EVT, pValue) != SUCCESS)
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
        if (sInstance.EnqueueEvtHdrMsg(PASSCODE_EVT, pData) != SUCCESS)
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
        if (sInstance.EnqueueEvtHdrMsg(PAIR_STATE_EVT, pData) != SUCCESS)
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
