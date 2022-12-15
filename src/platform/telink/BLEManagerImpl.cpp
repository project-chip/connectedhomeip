/*
 *
 *    Copyright (c) 2020-2022 Project CHIP Authors
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
 *          for the Telink platform.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/BleUUID.h>
#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

/*Includes for ieee802154 switchings */
#define DT_DRV_COMPAT telink_b91_zb
#include <drivers/ieee802154/b91.h>
#include <zephyr/net/ieee802154_radio.h>

/* Telink headers */
#include "drivers.h"
#include "ext_driver/ext_misc.h"
#include "stack/ble/ble.h"
#include "tl_common.h"
#include "types.h"

using namespace ::chip;
using namespace ::chip::Ble;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

typedef enum
{
    ATT_H_START = 0,

    /* GAP service */
    GenericAccess_PS_H,            // UUID: 2800, VALUE: uuid 1800
    GenericAccess_DeviceName_CD_H, // UUID: 2803, VALUE: Prop: Read | Notify
    GenericAccess_DeviceName_DP_H, // UUID: 2A00, VALUE: device name
    GenericAccess_Appearance_CD_H, // UUID: 2803, VALUE: Prop: Read
    GenericAccess_Appearance_DP_H, // UUID: 2A01, VALUE: appearance
    CONN_PARAM_CD_H,               // UUID: 2803, VALUE: Prop: Read
    CONN_PARAM_DP_H,               // UUID: 2A04, VALUE: connParameter

    /* GATT service */
    GenericAttribute_PS_H,                 // UUID: 2800, VALUE: uuid 1801
    GenericAttribute_ServiceChanged_CD_H,  // UUID: 2803, VALUE: Prop: Indicate
    GenericAttribute_ServiceChanged_DP_H,  // UUID: 2A05, VALUE: service change
    GenericAttribute_ServiceChanged_CCB_H, // UUID: 2902, VALUE: serviceChangeCCC

    /* Matter service */
    Matter_PS_H,
    Matter_RX_CD_H,
    Matter_RX_DP_H,
    Matter_TX_CD_H,
    Matter_TX_DP_H,
    Matter_TX_CCC_H,

    ATT_END_H,

} ATT_HANDLE;

typedef struct
{
    /** Minimum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
    u16 intervalMin;
    /** Maximum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
    u16 intervalMax;
    /** Number of LL latency connection events (0x0000 - 0x03e8) */
    u16 latency;
    /** Connection Timeout (0x000A - 0x0C80 * 10 ms) */
    u16 timeout;
} gap_periConnectParams_t;

#define CHIP_MAC_LEN 6
#define CHIP_MTU_SIZE 244
#define CHIP_MAX_ADV_DATA_LEN 31
#define CHIP_MAX_RESPONSE_DATA_LEN 31
#define CHIP_SHORT_UUID_LEN 2

#define CHIP_ADE_DATA_LEN_FLAGS 0x02
#define CHIP_ADV_DATA_TYPE_FLAGS 0x01
#define CHIP_ADV_DATA_FLAGS 0x06
#define CHIP_ADV_DATA_TYPE_UUID 0x03
#define CHIP_ADV_DATA_TYPE_NAME 0x09
#define CHIP_ADV_DATA_TYPE_SERVICE_DATA 0x16
#define CHIP_ADV_SERVICE_DATA_LEN (sizeof(ChipBLEDeviceIdentificationInfo) + CHIP_SHORT_UUID_LEN + 1)

#define CHIP_BLE_TX_FIFO_SIZE 48
#define CHIP_BLE_TX_FIFO_NUM 33
#define CHIP_BLE_RX_FIFO_SIZE 48
#define CHIP_BLE_RX_FIFO_NUM 8

#define CHIP_BLE_THREAD_STACK_SIZE 2048
#define CHIP_BLE_THREAD_PRIORITY 2

#define CHIP_BLE_DISCONNECT_REASON 8

#define CHIP_RF_PACKET_HEADER_SIZE 3

#define STIMER_IRQ_NUM 1
#define RF_IRQ_NUM 15

#define CHIP_RX_CHAR_UUID 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18
#define CHIP_TX_CHAR_UUID 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18

const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };

const ChipBleUUID chipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

static const uint8_t matterServiceUUID[CHIP_SHORT_UUID_LEN] = { 0xF6, 0xFF }; // service UUID

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;

void rf_irq_handler(const void * paramiter)
{
    irq_blt_sdk_handler();
}

void stimer_irq_handler(const void * paramiter)
{
    irq_blt_sdk_handler();
}

void BLEManagerImpl::BleEntry(void *, void *, void *)
{
    while (true)
    {
        blt_sdk_main_loop();

        k_msleep(10);
    }
}

/* Thread for running BLE main loop */
K_THREAD_DEFINE(chipBleThread, CHIP_BLE_THREAD_STACK_SIZE, BLEManagerImpl::BleEntry, NULL, NULL, NULL, CHIP_BLE_THREAD_PRIORITY, 0,
                0);

CHIP_ERROR BLEManagerImpl::_Init()
{
    ThreadConnectivityReady = false;

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);
    mGAPConns = 0;

    memset(mSubscribedConns, 0, sizeof(mSubscribedConns));

    // Initialize the CHIP BleLayer.
    ReturnErrorOnFailure(BleLayer::Init(this, this, &DeviceLayer::SystemLayer()));

    // Suspend BLE Task
    k_thread_suspend(chipBleThread);

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    BLEMgrImpl().DriveBLEState();
}

void BLEManagerImpl::DriveBLEState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Perform any initialization actions that must occur after the CHIP task is running.
    if (!mFlags.Has(Flags::kAsyncInitCompleted))
    {
        mFlags.Set(Flags::kAsyncInitCompleted);
    }

    // If the application has enabled CHIPoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled &&
        mFlags.Has(Flags::kAdvertisingEnabled)
#if CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION
        // and no connections are active...
        && (NumConnections() == 0)
#endif
    )
    {
        // Start/re-start advertising if not already advertising, or if the
        // advertising state needs to be refreshed.
        if (!mFlags.Has(Flags::kAdvertising) || mFlags.Has(Flags::kAdvertisingRefreshNeeded))
        {
            mFlags.Clear(Flags::kAdvertisingRefreshNeeded);
            err = StartAdvertising();
            SuccessOrExit(err);
        }
    }
    else
    {
        if (mFlags.Has(Flags::kAdvertising))
        {
            err = StopAdvertising();
            SuccessOrExit(err);
        }
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %" CHIP_ERROR_FORMAT, err.Format());
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

int BLEManagerImpl::RxWriteCallback(uint16_t connHandle, void * p)
{
    rf_packet_att_t * packet = (rf_packet_att_t *) p;
    size_t dataLen           = packet->l2capLen - CHIP_RF_PACKET_HEADER_SIZE;
    ChipDeviceEvent event;

    PacketBufferHandle packetBuf = PacketBufferHandle::NewWithData(packet->dat, dataLen);

    // If successful...
    if (packetBuf.IsNull())
    {
        ChipLogError(DeviceLayer, "Failed to allocate buffer");

        return 0;
    }

    // Arrange to post a CHIPoBLERXWriteEvent event to the CHIP queue.
    event.Type                                = DeviceEventType::kPlatformTelinkBleRXWrite;
    event.Platform.BleRXWriteEvent.connHandle = connHandle;
    event.Platform.BleRXWriteEvent.Data       = std::move(packetBuf).UnsafeRelease();

    PlatformMgr().PostEventOrDie(&event);

    return 0;
}

void BLEManagerImpl::ConnectCallback(uint8_t bleEvent, uint8_t * data, int len)
{
    ChipDeviceEvent event;
    ble_sts_t status = BLE_SUCCESS;

    event.Type                             = DeviceEventType::kPlatformTelinkBleConnected;
    event.Platform.BleConnEvent.connHandle = BLS_CONN_HANDLE;
    event.Platform.BleConnEvent.HciResult  = BLE_SUCCESS;

    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::DisconnectCallback(uint8_t bleEvent, uint8_t * data, int len)
{
    ChipDeviceEvent event;

    event.Type                             = DeviceEventType::kPlatformTelinkBleDisconnected;
    event.Platform.BleConnEvent.connHandle = BLS_CONN_HANDLE;
    event.Platform.BleConnEvent.HciResult  = *data; // Reason of disconnection stored in first data byte

    PlatformMgr().PostEventOrDie(&event);
}

int BLEManagerImpl::TxCccWriteCallback(uint16_t connHandle, void * p)
{
    ChipDeviceEvent event;
    rf_packet_att_t * packet = (rf_packet_att_t *) p;
    int dataLen              = packet->rf_len - CHIP_RF_PACKET_HEADER_SIZE;
    uint16_t value           = *((uint16_t *) packet->dat);

    event.Type                                 = DeviceEventType::kPlatformTelinkBleCCCWrite;
    event.Platform.BleCCCWriteEvent.connHandle = connHandle;
    event.Platform.BleCCCWriteEvent.Value      = value;

    PlatformMgr().PostEventOrDie(&event);

    return 0;
}

int BLEManagerImpl::GapEventHandler(uint32_t gapEvent, uint8_t * data, int size)
{
    ChipDeviceEvent event;

    if ((gapEvent & 0xFF) == GAP_EVT_GATT_HANDLE_VLAUE_CONFIRM)
    {
        /* Send TX complete event if everything is fine */
        event.Type                                   = DeviceEventType::kPlatformTelinkBleTXComplete;
        event.Platform.BleTXCompleteEvent.connHandle = BLS_CONN_HANDLE;

        PlatformMgr().PostEventOrDie(&event);
    }

    return 0;
}

CHIP_ERROR BLEManagerImpl::_InitStack(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t macPublic[CHIP_MAC_LEN]       = { 0 };
    uint8_t macRandomStatic[CHIP_MAC_LEN] = { 0 };
    int ret                               = 0;

    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Thread Provisioned. Ignore");

        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Reset Radio */
    rf_radio_reset();

    /* Reset DMA */
    rf_reset_dma();

    /* Init Radio driver */
    ble_radio_init();

    /* Generate MAC address if it does not exist or read it from flash if it is exist already */
    blc_initMacAddress(CFG_ADR_MAC_1M_FLASH, macPublic, macRandomStatic);

    /* Init interrupts and DMA for BLE module ??? */
    blc_ll_initBasicMCU();

    /* Setup MAC Address */
    blc_ll_initStandby_module(macPublic);

    /* Init advertisement */
    blc_ll_initAdvertising_module();

    /* Init slave role */
    blc_ll_initSlaveRole_module();

    /* Init connection mode */
    blc_ll_initConnection_module();

    /*set rf power index*/
    rf_set_power_level_index(RF_POWER_INDEX_P9p11dBm);

    /* Init GAP */
    err = _InitGap();
    SuccessOrExit(err);

    /* Resetup stimer interrupt to handle BLE stack */
    ret = irq_connect_dynamic(STIMER_IRQ_NUM + CONFIG_2ND_LVL_ISR_TBL_OFFSET, 2, stimer_irq_handler, NULL, 0);
    ChipLogDetail(DeviceLayer, "Stimer IRQ assigned vector %d", ret);

    /* Resetup rf interrupt to handle BLE stack */
    ret = irq_connect_dynamic(RF_IRQ_NUM + CONFIG_2ND_LVL_ISR_TBL_OFFSET, 2, rf_irq_handler, NULL, 0);
    ChipLogDetail(DeviceLayer, "RF IRQ assigned vector %d", ret);

exit:

    return err;
}

CHIP_ERROR BLEManagerImpl::_InitGap(void)
{
    ble_sts_t status = BLE_SUCCESS;
    /* Fifo buffers */
    static u8 txFifoBuff[CHIP_BLE_TX_FIFO_SIZE * CHIP_BLE_TX_FIFO_NUM] = { 0 };
    static u8 rxFifoBuff[CHIP_BLE_RX_FIFO_SIZE * CHIP_BLE_RX_FIFO_NUM] = { 0 };

    status = blc_ll_initAclConnTxFifo(txFifoBuff, CHIP_BLE_TX_FIFO_SIZE, CHIP_BLE_TX_FIFO_NUM);
    if (status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to init BLE TX FIFO. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    status = blc_ll_initAclConnRxFifo(rxFifoBuff, CHIP_BLE_RX_FIFO_SIZE, CHIP_BLE_RX_FIFO_NUM);
    if (status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to init BLE RX FIFO. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    status = blc_controller_check_appBufferInitialization();
    if (status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Buffer initialization check failed. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Init GAP */
    blc_gap_peripheral_init();

    /* Set up GATT Services */
    _InitGatt();

    /* L2CAP Initialization */
    blc_l2cap_register_handler((void *) blc_l2cap_packet_receive);

    /* Setup connect/terminate callbacks */
    bls_app_registerEventCallback(BLT_EV_FLAG_CONNECT, BLEManagerImpl::ConnectCallback);
    bls_app_registerEventCallback(BLT_EV_FLAG_TERMINATE, BLEManagerImpl::DisconnectCallback);

    /* Add GAP event handler to handle indication send */
    blc_gap_registerHostEventHandler(BLEManagerImpl::GapEventHandler);
    blc_gap_setEventMask(GAP_EVT_MASK_GATT_HANDLE_VLAUE_CONFIRM);

    /* Set MTU */
    status = blc_att_setRxMtuSize(CHIP_MTU_SIZE);
    if (status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to set MTU size. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_InitGatt(void)
{
    /* UUIDs */
    static const u16 primaryServiceUUID     = GATT_UUID_PRIMARY_SERVICE;
    static const u16 gapServiceUUID         = SERVICE_UUID_GENERIC_ACCESS;
    static const u16 characterUUID          = GATT_UUID_CHARACTER;
    static const u16 devNameUUID            = GATT_UUID_DEVICE_NAME;
    static const u16 gattServiceUUID        = SERVICE_UUID_GENERIC_ATTRIBUTE;
    static const u16 serviceChangeUUID      = GATT_UUID_SERVICE_CHANGE;
    static const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;
    static const u16 devServiceUUID         = SERVICE_UUID_DEVICE_INFORMATION;
    static const u16 appearanceUUID         = GATT_UUID_APPEARANCE;
    static const u16 periConnParamUUID      = GATT_UUID_PERI_CONN_PARAM;
    static const u8 MatterRxCharUUID[]      = WRAPPING_BRACES(CHIP_RX_CHAR_UUID);
    static const u8 MatterTxCharUUID[]      = WRAPPING_BRACES(CHIP_TX_CHAR_UUID);

    /* Characteristics */
    static const u8 devNameCharVal[] = { CHAR_PROP_READ | CHAR_PROP_NOTIFY, U16_LO(GenericAccess_DeviceName_DP_H),
                                         U16_HI(GenericAccess_DeviceName_DP_H), U16_LO(GATT_UUID_DEVICE_NAME),
                                         U16_HI(GATT_UUID_DEVICE_NAME) };

    static const u8 appearanceCharVal[] = { CHAR_PROP_READ, U16_LO(GenericAccess_Appearance_DP_H),
                                            U16_HI(GenericAccess_Appearance_DP_H), U16_LO(GATT_UUID_APPEARANCE),
                                            U16_HI(GATT_UUID_APPEARANCE) };

    static const u8 periConnParamCharVal[] = { CHAR_PROP_READ, U16_LO(CONN_PARAM_DP_H), U16_HI(CONN_PARAM_DP_H),
                                               U16_LO(GATT_UUID_PERI_CONN_PARAM), U16_HI(GATT_UUID_PERI_CONN_PARAM) };

    static const u8 serviceChangeCharVal[] = { CHAR_PROP_INDICATE, U16_LO(GenericAttribute_ServiceChanged_DP_H),
                                               U16_HI(GenericAttribute_ServiceChanged_DP_H), U16_LO(GATT_UUID_SERVICE_CHANGE),
                                               U16_HI(GATT_UUID_SERVICE_CHANGE) };

    static const u8 MatterRxCharVal[] = { CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP, U16_LO(Matter_RX_DP_H),
                                          U16_HI(Matter_RX_DP_H), CHIP_RX_CHAR_UUID };

    static const u8 MatterTxCharVal[] = { CHAR_PROP_INDICATE, U16_LO(Matter_TX_DP_H), U16_HI(Matter_TX_DP_H), CHIP_TX_CHAR_UUID };

    /* Values */
    static const u16 appearance                             = GAP_APPEARE_UNKNOWN;
    static const gap_periConnectParams_t periConnParameters = { 8, 11, 0, 1000 };
    static u16 serviceChangeVal[2]                          = { 0 };
    static u8 serviceChangeCCC[2]                           = { 0 };
    static u8 matterTxCCC[2]                                = { 0 };

    static const attribute_t gattTable[] = {
        /* Total number of attributes */
        { ATT_END_H - 1, 0, 0, 0, 0, 0 },

        /* 0001 - 0007  GAP service */
        { 7, ATT_PERMISSIONS_READ, 2, 2, (u8 *) (&primaryServiceUUID), (u8 *) (&gapServiceUUID), 0 },
        { 0, ATT_PERMISSIONS_READ, 2, sizeof(devNameCharVal), (u8 *) (&characterUUID), (u8 *) (devNameCharVal), 0 },
        { 0, ATT_PERMISSIONS_READ, 2, (u32) kMaxDeviceNameLength, (u8 *) (&devNameUUID), (u8 *) (mDeviceName), 0 },
        { 0, ATT_PERMISSIONS_READ, 2, sizeof(appearanceCharVal), (u8 *) (&characterUUID), (u8 *) (appearanceCharVal), 0 },
        { 0, ATT_PERMISSIONS_READ, 2, sizeof(appearance), (u8 *) (&appearanceUUID), (u8 *) (&appearance), 0 },
        { 0, ATT_PERMISSIONS_READ, 2, sizeof(periConnParamCharVal), (u8 *) (&characterUUID), (u8 *) (periConnParamCharVal), 0 },
        { 0, ATT_PERMISSIONS_READ, 2, sizeof(periConnParameters), (u8 *) (&periConnParamUUID), (u8 *) (&periConnParameters), 0 },

        /* 0008 - 000b GATT */
        { 4, ATT_PERMISSIONS_READ, 2, 2, (u8 *) (&primaryServiceUUID), (u8 *) (&gattServiceUUID), 0 },
        { 0, ATT_PERMISSIONS_READ, 2, sizeof(serviceChangeCharVal), (u8 *) (&characterUUID), (u8 *) (serviceChangeCharVal), 0 },
        { 0, ATT_PERMISSIONS_READ, 2, sizeof(serviceChangeVal), (u8 *) (&serviceChangeUUID), (u8 *) (&serviceChangeVal), 0 },
        { 0, ATT_PERMISSIONS_RDWR, 2, sizeof(serviceChangeCCC), (u8 *) (&clientCharacterCfgUUID), (u8 *) (serviceChangeCCC), 0 },

        /* 000c - 0011 Matter service */
        { 6, ATT_PERMISSIONS_READ, 2, 2, (u8 *) (&primaryServiceUUID), (u8 *) (&matterServiceUUID), 0 },
        { 0, ATT_PERMISSIONS_READ, 2, sizeof(MatterRxCharVal), (u8 *) (&characterUUID), (u8 *) (MatterRxCharVal), 0 },
        { 0, ATT_PERMISSIONS_RDWR, 16, sizeof(mRxDataBuff), (u8 *) (&MatterRxCharUUID), mRxDataBuff, RxWriteCallback, NULL },
        { 0, ATT_PERMISSIONS_READ, 2, sizeof(MatterTxCharVal), (u8 *) (&characterUUID), (u8 *) (MatterTxCharVal), 0 },
        { 0, ATT_PERMISSIONS_RDWR, 16, sizeof(mTxDataBuff), (u8 *) (&MatterTxCharUUID), mTxDataBuff, 0 },
        { 0, ATT_PERMISSIONS_RDWR, 2, sizeof(matterTxCCC), (u8 *) (&clientCharacterCfgUUID), (u8 *) (matterTxCCC),
          TxCccWriteCallback, NULL }
    };

    bls_att_setAttributeTable((u8 *) gattTable);
}

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    ble_sts_t status   = BLE_SUCCESS;
    CHIP_ERROR err     = CHIP_NO_ERROR;
    uint8_t index      = 0;
    uint8_t devNameLen = 0;
    ChipBLEDeviceIdentificationInfo deviceIdInfo;
    u8 adv[CHIP_MAX_ADV_DATA_LEN]       = { 0 };
    u8 srsp[CHIP_MAX_RESPONSE_DATA_LEN] = { 0 };

    ChipLogProgress(DeviceLayer, "BLEManagerImpl::ConfigureAdvertisingData");

    /* Get BLE device identification info */
    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    SuccessOrExit(err);

    /* Check device name */
    if (!mFlags.Has(Flags::kDeviceNameSet))
    {
        err = _SetDeviceName("TelinkMatter");
        SuccessOrExit(err);
    }

    /* Fulfill BLE advertisement data */
    /* Set flags */
    adv[index++] = CHIP_ADE_DATA_LEN_FLAGS;
    adv[index++] = CHIP_ADV_DATA_TYPE_FLAGS;
    adv[index++] = CHIP_ADV_DATA_FLAGS;

    /* Set Service Data */
    adv[index++] = CHIP_ADV_SERVICE_DATA_LEN;
    adv[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;
    adv[index++] = matterServiceUUID[0];
    adv[index++] = matterServiceUUID[1];
    memcpy(&adv[index], (void *) &deviceIdInfo, sizeof(deviceIdInfo));
    index += sizeof(deviceIdInfo);

    /* Set device name */
    devNameLen   = strlen(mDeviceName);
    adv[index++] = devNameLen + 1;
    adv[index++] = CHIP_ADV_DATA_TYPE_NAME;
    memcpy(&adv[index], mDeviceName, devNameLen);
    index += devNameLen;

    /* Set advetisment data */
    status = bls_ll_setAdvData(adv, index);
    if (status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to set BLE advertisement data. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    index         = 0;
    srsp[index++] = CHIP_SHORT_UUID_LEN + 1;
    srsp[index++] = CHIP_ADV_DATA_TYPE_UUID;
    srsp[index++] = matterServiceUUID[0];
    srsp[index++] = matterServiceUUID[1];

    /* Set scan response data */
    status = bls_ll_setScanRspData(srsp, sizeof(srsp));
    if (status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to set BLE scan response data. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrReturnError(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported,
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (mFlags.Has(Flags::kAdvertisingEnabled) != val)
    {
        ChipLogDetail(DeviceLayer, "CHIPoBLE advertising set to %s", val ? "on" : "off");

        mFlags.Set(Flags::kAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err   = CHIP_NO_ERROR;
    ble_sts_t status = BLE_SUCCESS;

    /* At first run always select fast advertising, on the next attempt slow down interval. */
    u16 intervalMin = mFlags.Has(Flags::kFastAdvertisingEnabled) ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN
                                                                 : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
    u16 intervalMax = mFlags.Has(Flags::kFastAdvertisingEnabled) ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX
                                                                 : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;

    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Thread provisioned. Start advertisement not possible");

        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Block IEEE802154 */
    /* @todo: move to RadioSwitch module*/
    const struct device * radio_dev = device_get_binding(CONFIG_NET_CONFIG_IEEE802154_DEV_NAME);
    __ASSERT(radio_dev != NULL, "Fail to get radio device");
    b91_deinit(radio_dev);

    /* It is time to init BLE stack */
    err = _InitStack();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Fail to init BLE stack");

        return err;
    }

    /* Configure CHIP BLE advertisement data */
    err = ConfigureAdvertisingData();
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Fail to config BLE advertisement data");

        return err;
    }

    /* Setup advertisement parameters */
    status = bls_ll_setAdvParam(intervalMin, intervalMax, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL,
                                BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
    if (status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to set BLE advertisement parameters. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Enable advertisement */
    status = bls_ll_setAdvEnable(BLC_ADV_ENABLE);
    if (status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to start BLE advertisement. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Transition to the Advertising state...
    if (!mFlags.Has(Flags::kAdvertising))
    {
        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising started");

        mFlags.Set(Flags::kAdvertising);

        // Post a CHIPoBLEAdvertisingChange(Started) event.
        {
            ChipDeviceEvent advChange;
            advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
            advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Started;
            ReturnErrorOnFailure(PlatformMgr().PostEvent(&advChange));
        }

        if (mFlags.Has(Flags::kFastAdvertisingEnabled))
        {
            // Start timer to change advertising interval.
            DeviceLayer::SystemLayer().StartTimer(
                System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME),
                HandleBLEAdvertisementIntervalChange, this);
        }
    }

    /* Start BLE Task */
    k_thread_resume(chipBleThread);

    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    ble_sts_t status = BLE_SUCCESS;

    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Thread provisioned. Advertisement already stopped at this stage");

        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Disable advertisement */
    status = bls_ll_setAdvEnable(BLC_ADV_DISABLE);
    if (status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to stop BLE advertisement. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Transition to the not Advertising state...
    if (mFlags.Has(Flags::kAdvertising))
    {
        mFlags.Clear(Flags::kAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled, true);

        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");

        // Post a CHIPoBLEAdvertisingChange(Stopped) event.
        {
            ChipDeviceEvent advChange;
            advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
            advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
            ReturnErrorOnFailure(PlatformMgr().PostEvent(&advChange));
        }

        // Cancel timer event changing CHIPoBLE advertisement interval
        DeviceLayer::SystemLayer().CancelTimer(HandleBLEAdvertisementIntervalChange, this);
    }

    return CHIP_NO_ERROR;
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

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * devName)
{

    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported)
    {
        ChipLogError(DeviceLayer, "Unsupported");

        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    if (devName == NULL && devName[0] == 0)
    {
        ChipLogError(DeviceLayer, "Invalid name");

        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    if (strlen(devName) >= kMaxDeviceNameLength)
    {
        ChipLogError(DeviceLayer, "BLE device name is to long");

        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    strcpy(mDeviceName, devName);
    mFlags.Set(Flags::kDeviceNameSet);

    ChipLogProgress(DeviceLayer, "Setting device name to : \"%s\"", devName);

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::HandleBLEAdvertisementIntervalChange(System::Layer * layer, void * param)
{
    BLEMgr().SetAdvertisingMode(BLEAdvertisingMode::kSlowAdvertising);
    ChipLogProgress(DeviceLayer, "CHIPoBLE advertising mode changed to slow");
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (event->Type)
    {
    case DeviceEventType::kPlatformTelinkBleConnected:
        err = HandleGAPConnect(event);
        break;

    case DeviceEventType::kPlatformTelinkBleDisconnected:
        err = HandleGAPDisconnect(event);
        break;

    case DeviceEventType::kPlatformTelinkBleDisconnectRequest:
        err = HandleDisconnectRequest(event);
        break;

    case DeviceEventType::kPlatformTelinkBleRXWrite:
        err = HandleRXCharWrite(event);
        break;

    case DeviceEventType::kPlatformTelinkBleCCCWrite:
        err = HandleTXCharCCCDWrite(event);
        break;

    case DeviceEventType::kPlatformTelinkBleTXComplete:
        err = HandleTXCharComplete(event);
        break;

    case DeviceEventType::kThreadStateChange:
        err = HandleThreadStateChange(event);
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        err = HandleBleConnectionClosed(event);
        break;

    case DeviceEventType::kOperationalNetworkEnabled:
        err = HandleOperationalNetworkEnabled(event);
        break;

    default:
        ChipLogDetail(DeviceLayer, "Event: Unknown (0x%04x)", event->Type);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Fail to handle 0x%04x event. Error: %s", event->Type, ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    return false;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return blc_att_getEffectiveMtuSize(conId);
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle pBuf)
{
    ble_sts_t status = BLE_SUCCESS;

    do
    {
        if (status != BLE_SUCCESS)
        {
            k_msleep(1);
        }
        status = blc_gatt_pushHandleValueIndicate(conId, Matter_TX_DP_H, pBuf->Start(), pBuf->DataLength());
    } while (status == GATT_ERR_DATA_PENDING_DUE_TO_SERVICE_DISCOVERY_BUSY);
    if (status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to send indication. Error %d", status);

        return false;
    }

    return true;
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

/* @todo: move to RadioSwitch module */
void BLEManagerImpl::SwitchToIeee802154(void)
{
    int result = 0;

    ChipLogProgress(DeviceLayer, "BLEManagerImpl::Switch to IEEE802154");

    /* Stop BLE */
    _SetAdvertisingEnabled(false);

    /* Stop BLE task */
    k_thread_suspend(chipBleThread);

    /* Reset Radio */
    rf_radio_reset();

    /* Reset DMA */
    rf_reset_dma();

    const struct device * radio_dev = device_get_binding(CONFIG_NET_CONFIG_IEEE802154_DEV_NAME);
    __ASSERT(radio_dev != NULL, "Fail to get radio device");

    /* Init IEEE802154 */
    result = b91_init(radio_dev);
    __ASSERT(result == 0, "Fail to init IEEE802154 radio. Error: %d", result);
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) {}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    return mGAPConns;
}

/// @todo implement multicinnection subscription
CHIP_ERROR BLEManagerImpl::SetSubscribed(uint16_t conId)
{

    mSubscribedConns[0] = true;

    return CHIP_NO_ERROR;
}

/// @todo implement multicinnection subscription
bool BLEManagerImpl::UnsetSubscribed(uint16_t conId)
{

    mSubscribedConns[0] = false;

    return true;
}

/// @todo implement multicinnection subscription
bool BLEManagerImpl::IsSubscribed(uint16_t conId)
{
    return mSubscribedConns[0];
}

CHIP_ERROR BLEManagerImpl::HandleGAPConnect(const ChipDeviceEvent * event)
{
    const BleConnEventType * connEvent = &event->Platform.BleConnEvent;

    ChipLogProgress(DeviceLayer, "BLE connection established (ConnId: 0x%02x)", connEvent->connHandle);
    mGAPConns++;
    ChipLogProgress(DeviceLayer, "Current number of connections: %u/%u", NumConnections(), kMaxConnections);

    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleGAPDisconnect(const ChipDeviceEvent * event)
{
    const BleConnEventType * connEvent = &event->Platform.BleConnEvent;

    ChipLogProgress(DeviceLayer, "BLE GAP connection terminated (reason 0x%02x)", connEvent->HciResult);

    mGAPConns--;

    ChipLogProgress(DeviceLayer, "Current number of connections: %u/%u", NumConnections(), kMaxConnections);

    // Unsubscribe
    if (UnsetSubscribed(connEvent->connHandle))
    {
        HandleUnsubscribeReceived(connEvent->connHandle, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
    }

    ChipDeviceEvent disconnectEvent;
    disconnectEvent.Type = DeviceEventType::kCHIPoBLEConnectionClosed;
    ReturnErrorOnFailure(PlatformMgr().PostEvent(&disconnectEvent));

    // Force a reconfiguration of advertising in case we switched to non-connectable mode when
    // the BLE connection was established.
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleDisconnectRequest(const ChipDeviceEvent * event)
{
    ble_sts_t status = BLE_SUCCESS;
    uint16_t handle  = event->Platform.BleConnEvent.connHandle;
    uint8_t reason   = event->Platform.BleConnEvent.HciResult;

    ChipLogDetail(DeviceLayer, "HandleDisconnectRequest");

    /* Trigger disconnect. DisconnectCallback call occures on completion */
    status = blc_ll_disconnect(handle, reason);
    if (status != BLE_SUCCESS && status != LL_ERR_CONNECTION_NOT_ESTABLISH)
    {
        ChipLogError(DeviceLayer, "Fail to disconnect. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    // Force a reconfiguration of advertising in case we switched to non-connectable mode when
    // the BLE connection was established.
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleOperationalNetworkEnabled(const ChipDeviceEvent * event)
{
    ChipDeviceEvent disconnectEvent;

    ChipLogDetail(DeviceLayer, "HandleOperationalNetworkEnabled");

    disconnectEvent.Type                             = DeviceEventType::kPlatformTelinkBleDisconnectRequest;
    disconnectEvent.Platform.BleConnEvent.connHandle = BLS_CONN_HANDLE;
    disconnectEvent.Platform.BleConnEvent.HciResult  = CHIP_BLE_DISCONNECT_REASON;
    ReturnErrorOnFailure(PlatformMgr().PostEvent(&disconnectEvent));

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleThreadStateChange(const ChipDeviceEvent * event)
{
    CHIP_ERROR error = CHIP_NO_ERROR;

    ChipLogDetail(DeviceLayer, "HandleThreadStateChange");

    if (event->Type == DeviceEventType::kThreadStateChange && event->ThreadStateChange.RoleChanged)
    {
        ChipDeviceEvent attachEvent;
        attachEvent.Type                            = DeviceEventType::kThreadConnectivityChange;
        attachEvent.ThreadConnectivityChange.Result = kConnectivity_Established;

        error = PlatformMgr().PostEvent(&attachEvent);
        VerifyOrExit(error == CHIP_NO_ERROR,
                     ChipLogError(DeviceLayer, "Failed to post Thread connectivity change: %" CHIP_ERROR_FORMAT, error.Format()));

        ChipLogDetail(DeviceLayer, "Thread Connectivity Ready");
        ThreadConnectivityReady = true;
    }

exit:
    return error;
}

CHIP_ERROR BLEManagerImpl::HandleBleConnectionClosed(const ChipDeviceEvent * event)
{
    /* It is time to swich to IEEE802154 radio if it is provisioned */
    if (ThreadConnectivityReady)
    {
        SwitchToIeee802154();
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleRXCharWrite(const ChipDeviceEvent * event)
{
    const BleRXWriteEventType * writeEvent = &event->Platform.BleRXWriteEvent;

    ChipLogDetail(DeviceLayer, "Write request received for CHIPoBLE RX (ConnId 0x%02x)", writeEvent->connHandle);

    HandleWriteReceived(writeEvent->connHandle, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_RX,
                        PacketBufferHandle::Adopt(writeEvent->Data));

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleTXCharComplete(const ChipDeviceEvent * event)
{
    const BleTXCompleteEventType * completeEvent = &event->Platform.BleTXCompleteEvent;

    ChipLogDetail(DeviceLayer, "Notification for CHIPoBLE TX done (ConnId 0x%02x)", completeEvent->connHandle);

    // Signal the BLE Layer that the outstanding notification is complete.
    HandleIndicationConfirmation(completeEvent->connHandle, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleTXCharCCCDWrite(const ChipDeviceEvent * event)
{
    const BleCCCWriteEventType * writeEvent = &event->Platform.BleCCCWriteEvent;

    ChipLogDetail(DeviceLayer, "ConnId: 0x%02x, New CCCD value: 0x%04x", writeEvent->connHandle, writeEvent->Value);

    /* If the client has requested to enable notifications and if it is not yet subscribed */
    if (writeEvent->Value != 0 && SetSubscribed(writeEvent->connHandle) == CHIP_NO_ERROR)
    {
        /* Alert the BLE layer that CHIPoBLE "subscribe" has been received and increment the bt_conn reference counter. */
        HandleSubscribeReceived(writeEvent->connHandle, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);

        ChipLogProgress(DeviceLayer, "CHIPoBLE connection established (ConnId: 0x%02x, GATT MTU: %d)", writeEvent->connHandle,
                        GetMTU(writeEvent->connHandle));

        /* Post a CHIPoBLEConnectionEstablished event to the DeviceLayer and the application. */
        {
            ChipDeviceEvent conEstEvent;
            conEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
            ReturnErrorOnFailure(PlatformMgr().PostEvent(&conEstEvent));
        }
    }
    else
    {
        if (UnsetSubscribed(writeEvent->connHandle))
        {
            HandleUnsubscribeReceived(writeEvent->connHandle, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
        }
    }

    return CHIP_NO_ERROR;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
