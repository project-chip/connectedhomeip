/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
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
 *          for the Telink platforms.
 */
/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <ble/BleUUID.h>
#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>

#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

/* Telink headers */ 
#include "types.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

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
    GenericAccess_PS_H,                     //UUID: 2800, VALUE: uuid 1800
    GenericAccess_DeviceName_CD_H,          //UUID: 2803, VALUE: Prop: Read | Notify
    GenericAccess_DeviceName_DP_H,          //UUID: 2A00, VALUE: device name
    GenericAccess_Appearance_CD_H,          //UUID: 2803, VALUE: Prop: Read
    GenericAccess_Appearance_DP_H,          //UUID: 2A01, VALUE: appearance
    CONN_PARAM_CD_H,                        //UUID: 2803, VALUE: Prop: Read
    CONN_PARAM_DP_H,                        //UUID: 2A04, VALUE: connParameter

    /* GATT service */
    GenericAttribute_PS_H,                  //UUID: 2800, VALUE: uuid 1801
    GenericAttribute_ServiceChanged_CD_H,   //UUID: 2803, VALUE: Prop: Indicate
    GenericAttribute_ServiceChanged_DP_H,   //UUID: 2A05, VALUE: service change
    GenericAttribute_ServiceChanged_CCB_H,  //UUID: 2902, VALUE: serviceChangeCCC

    /* Matter service */
    Matter_PS_H,
    Matter_RX_CD_H,
    Matter_RX_DP_H,
    Matter_TX_CD_H,
    Matter_TX_DP_H,
    Matter_TX_CCC_H,

    ATT_END_H,

}ATT_HANDLE;

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
#define CHIP_BLE_TX_FIFO_NUM 17
#define CHIP_BLE_RX_FIFO_SIZE 48
#define CHIP_BLE_RX_FIFO_NUM 8

#define CHIP_BLE_THREAD_STACK_SIZE 8196
#define CHIP_BLE_THREAD_PRIORITY 2

#define CHIP_RX_BUFF_SIZE 20
#define CHIP_RF_PACKET_HEADER_SIZE 7

#define WHITE_LED GPIO_PB6
#define GREEN_LED GPIO_PB5
#define BLUE_LED GPIO_PB4

#define STIMER_IRQ_NUM 1
#define RF_IRQ_NUM     15

#define CHIP_RX_CHAR_UUID 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18
#define CHIP_TX_CHAR_UUID 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18

const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };

const ChipBleUUID chipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

static const uint8_t matterServiceUUID[CHIP_SHORT_UUID_LEN] = { 0xF6, 0xFF };     // service UUID

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;

void rf_irq_handler(const void *paramiter)
{
    // gpio_set_high_level(WHITE_LED);

    irq_blt_sdk_handler();

    // gpio_set_low_level(WHITE_LED);
}

void stimer_irq_handler(const void *paramiter)
{
    // gpio_set_high_level(GREEN_LED);

    irq_blt_sdk_handler();

    // gpio_set_low_level(GREEN_LED);
}

void chip_ble_thread_entry_point(void *, void *, void *)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::BLEThreadEntry");

    while(true) {
        // gpio_set_high_level(BLUE_LED);

        blt_sdk_main_loop();

        // gpio_set_low_level(BLUE_LED);

        k_msleep(10);

    }
}

void connectCallback(u8 e, u8 *p, int n)
{
    ChipLogProgress(DeviceLayer, "BLE connect");
}

void terminateCallback(u8 e, u8 *p, int n)
{
    ChipLogProgress(DeviceLayer, "BLE terminate");
}

/* Thread for runing BLE main loop */
K_THREAD_DEFINE(chip_ble_thread, 
                CHIP_BLE_THREAD_STACK_SIZE, 
                chip_ble_thread_entry_point, 
                NULL, NULL, NULL,
                CHIP_BLE_THREAD_PRIORITY, 0, 0);

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    uint8_t mac_public[6] = {0};
    uint8_t mac_random_static[6] = {0};
    int ret = 0;

    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_Init");

    /* Init Radio driver */
    ble_radio_init();

    /* Generate MAC address if it does not exist or read it from flash if it is exist already */
    blc_initMacAddress(CFG_ADR_MAC_1M_FLASH, mac_public, mac_random_static);

    /* Init interrupts and DMA for BLE module ??? */
    blc_ll_initBasicMCU();

    /* Setup MAC Address */
    blc_ll_initStandby_module(mac_public);

    /* Init advertisement */
    blc_ll_initAdvertising_module();

    /* Init slave role */
    blc_ll_initSlaveRole_module();

    /* Init connection mode */
    blc_ll_initConnection_module();

    /* Init GATT */
    err = _InitGatt();
    SuccessOrExit(err);

    /* Resetup stimer interrupt to handle BLE stack */
    ret = irq_connect_dynamic(STIMER_IRQ_NUM, 2, stimer_irq_handler, NULL, 0);
    ChipLogDetail(DeviceLayer, "Stimer IRQ assigned vector %d", ret);

    /* Resetup rf interrupt to handle BLE stack */ 
    ret = irq_connect_dynamic(RF_IRQ_NUM, 2, rf_irq_handler, NULL, 0);
    ChipLogDetail(DeviceLayer, "RF IRQ assigned vector %d", ret);

    /* Enable White LED for debug purposes */
    gpio_function_en(WHITE_LED);
    gpio_output_en(WHITE_LED);

    /* Switch off the LED on the beginning */
    gpio_set_low_level(WHITE_LED);

    /* Enable Green LED for debug purposes */
    gpio_function_en(GREEN_LED);
    gpio_output_en(GREEN_LED);

    /* Switch off the LED on the beginning */
    gpio_set_low_level(GREEN_LED);

    /* Enable Blue LED for debug purposes */
    gpio_function_en(BLUE_LED);
    gpio_output_en(BLUE_LED);

    /* Switch off the LED on the beginning */
    gpio_set_low_level(BLUE_LED);

    /* Initialize the CHIP BleLayer. */
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    /* Enable CHIP over BLE service */
    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;

exit:
    return err;
}

int BLEManagerImpl::RxWriteCallback(uint16_t connHandle, void *p)
{
    rf_packet_att_t *packet = (rf_packet_att_t*)p;
    int dataLen = packet->rf_len - CHIP_RF_PACKET_HEADER_SIZE;
    ChipDeviceEvent event;
    
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::RxWriteCallback");
    ChipLogDetail(DeviceLayer, "Data len: %d", dataLen);

    PacketBufferHandle packetBuf = PacketBufferHandle::NewWithData(packet->dat, dataLen);

    ChipLogDetail(DeviceLayer,"Data: %02x %02x %02x %02x %02x %02x %02x %02x",  packet->dat[0],
                                                                                packet->dat[1],
                                                                                packet->dat[2],
                                                                                packet->dat[3],
                                                                                packet->dat[4],
                                                                                packet->dat[5],
                                                                                packet->dat[6],
                                                                                packet->dat[7]);

    ChipLogDetail(DeviceLayer,"Data: %02x %02x %02x %02x %02x %02x %02x %02x",  packet->dat[8],
                                                                                packet->dat[9],
                                                                                packet->dat[10],
                                                                                packet->dat[11],
                                                                                packet->dat[12],
                                                                                packet->dat[13],
                                                                                packet->dat[14],
                                                                                packet->dat[15]);


    // If successful...
    if (!packetBuf.IsNull())
    {
        // Arrange to post a CHIPoBLERXWriteEvent event to the CHIP queue.
        event.Type                                = DeviceEventType::kPlatformTelinkBleRXWrite;
        event.Platform.BleRXWriteEvent.connHandle = connHandle;
        event.Platform.BleRXWriteEvent.Data       = std::move(packetBuf).UnsafeRelease();
        
        ChipLogDetail(DeviceLayer, "Packet is ok :)");
    }

    // If we failed to allocate a buffer, post a kPlatformTelinkBleOutOfBuffersEvent event.
    else
    {
        // event.Type = DeviceEventType::kPlatformTelinkBleOutOfBuffersEvent;
        ChipLogDetail(DeviceLayer, "Packet is bad :(");
    }

    PlatformMgr().PostEventOrDie(&event);

    return 0;
}

CHIP_ERROR BLEManagerImpl::_InitGatt(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    ble_sts_t status = BLE_SUCCESS;

    /* Fifo buffers */
    static u8 txFifoBuff[CHIP_BLE_TX_FIFO_SIZE * CHIP_BLE_TX_FIFO_NUM] = {0};
    static u8 rxFufoBuff[CHIP_BLE_RX_FIFO_SIZE * CHIP_BLE_RX_FIFO_NUM] = {0};

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
    static const u8  MatterRxCharUUID[]     = WRAPPING_BRACES(CHIP_RX_CHAR_UUID);
    static const u8  MatterTxCharUUID[]     = WRAPPING_BRACES(CHIP_TX_CHAR_UUID);
 

    /* Characteristics */
    static const u8 devNameCharVal[] = 
    {
        CHAR_PROP_READ | CHAR_PROP_NOTIFY,
        U16_LO(GenericAccess_DeviceName_DP_H), U16_HI(GenericAccess_DeviceName_DP_H),
        U16_LO(GATT_UUID_DEVICE_NAME), U16_HI(GATT_UUID_DEVICE_NAME)
    };

    static const u8 appearanceCharVal[] = 
    {
        CHAR_PROP_READ,
        U16_LO(GenericAccess_Appearance_DP_H), U16_HI(GenericAccess_Appearance_DP_H),
        U16_LO(GATT_UUID_APPEARANCE), U16_HI(GATT_UUID_APPEARANCE)
    };

    static const u8 periConnParamCharVal[] = 
    {
        CHAR_PROP_READ,
        U16_LO(CONN_PARAM_DP_H), U16_HI(CONN_PARAM_DP_H),
        U16_LO(GATT_UUID_PERI_CONN_PARAM), U16_HI(GATT_UUID_PERI_CONN_PARAM)
    };

    static const u8 serviceChangeCharVal[] = 
    {
        CHAR_PROP_INDICATE,
        U16_LO(GenericAttribute_ServiceChanged_DP_H), U16_HI(GenericAttribute_ServiceChanged_DP_H),
        U16_LO(GATT_UUID_SERVICE_CHANGE), U16_HI(GATT_UUID_SERVICE_CHANGE)
    };

    static const u8 MatterRxCharVal[] = 
    {
        CHAR_PROP_WRITE | CHAR_PROP_WRITE_WITHOUT_RSP,
        U16_LO(Matter_RX_DP_H), U16_HI(Matter_RX_DP_H),
        CHIP_RX_CHAR_UUID
    };

    static const u8 MatterTxCharVal[] = 
    {
        CHAR_PROP_NOTIFY,
        U16_LO(Matter_TX_DP_H), U16_HI(Matter_TX_DP_H),
        CHIP_TX_CHAR_UUID
    };

    /* Values */
    static const u16 appearance = GAP_APPEARE_UNKNOWN;
    static const gap_periConnectParams_t periConnParameters = {8, 11, 0, 1000};
    static u16 serviceChangeVal[2] = {0};
    static u8 serviceChangeCCC[2] = {0};
    static u8 matterTxCCC[2] = {0,0};

    static const attribute_t gattTable[] = 
    {
        /* Total number of attributes */
        {ATT_END_H - 1, 0,0,0,0,0},

        /* 0001 - 0007  GAP service */
        {7, ATT_PERMISSIONS_READ, 2,  2,                            (u8*)(&primaryServiceUUID),     (u8*)(&gapServiceUUID),         0},
        {0, ATT_PERMISSIONS_READ, 2,  sizeof(devNameCharVal),       (u8*)(&characterUUID),          (u8*)(devNameCharVal),          0},
        {0, ATT_PERMISSIONS_READ, 2,  (u32)kMaxDeviceNameLength,    (u8*)(&devNameUUID),            (u8*)(mDeviceName),             0},
        {0, ATT_PERMISSIONS_READ, 2,  sizeof(appearanceCharVal),    (u8*)(&characterUUID),          (u8*)(appearanceCharVal),       0},
        {0, ATT_PERMISSIONS_READ, 2,  sizeof(appearance),           (u8*)(&appearanceUUID),         (u8*)(&appearance),             0},
        {0, ATT_PERMISSIONS_READ, 2,  sizeof(periConnParamCharVal), (u8*)(&characterUUID),          (u8*)(periConnParamCharVal),    0},
        {0, ATT_PERMISSIONS_READ, 2,  sizeof(periConnParameters),   (u8*)(&periConnParamUUID),      (u8*)(&periConnParameters),     0},

        /* 0008 - 000b GATT */
        {4, ATT_PERMISSIONS_READ, 2,  2,                            (u8*)(&primaryServiceUUID),     (u8*)(&gattServiceUUID),         0},
        {0, ATT_PERMISSIONS_READ, 2,  sizeof(serviceChangeCharVal), (u8*)(&characterUUID),          (u8*)(serviceChangeCharVal),     0},
        {0, ATT_PERMISSIONS_READ, 2,  sizeof(serviceChangeVal),     (u8*)(&serviceChangeUUID),      (u8*)(&serviceChangeVal),        0},
        {0, ATT_PERMISSIONS_RDWR, 2,  sizeof(serviceChangeCCC),     (u8*)(&clientCharacterCfgUUID), (u8*)(serviceChangeCCC),         0},

        /* 000c - 0011 Matter service */
        {6, ATT_PERMISSIONS_READ, 2,  2,                            (u8*)(&primaryServiceUUID),     (u8*)(&matterServiceUUID),       0},
        {0, ATT_PERMISSIONS_READ, 2,  sizeof(MatterRxCharVal),      (u8*)(&characterUUID),          (u8*)(MatterRxCharVal),          0},
        {0, ATT_PERMISSIONS_RDWR, 16, sizeof(mRxDataBuff),          (u8*)(&MatterRxCharUUID),       mRxDataBuff, RxWriteCallback, NULL},
        {0, ATT_PERMISSIONS_READ, 2,  sizeof(MatterTxCharVal),      (u8*)(&characterUUID),          (u8*)(MatterTxCharVal),          0},
        {0, ATT_PERMISSIONS_RDWR, 16, sizeof(mTxDataBuff),          (u8*)(&MatterTxCharUUID),       mTxDataBuff,                     0},
        {0, ATT_PERMISSIONS_RDWR, 2,  sizeof(matterTxCCC),          (u8*)(&clientCharacterCfgUUID), (u8*)(matterTxCCC),              0}
    };

    status = blc_ll_initAclConnTxFifo(txFifoBuff, CHIP_BLE_TX_FIFO_SIZE, CHIP_BLE_TX_FIFO_NUM);
    if(status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to init BLE TX FIFO. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    status = blc_ll_initAclConnRxFifo(rxFufoBuff, CHIP_BLE_RX_FIFO_SIZE, CHIP_BLE_RX_FIFO_NUM);
    if(status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to init BLE RX FIFO. Error %d", status);
        
        return CHIP_ERROR_INCORRECT_STATE;
    }

    status = blc_controller_check_appBufferInitialization();
    if(status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Buffer initialization check failed. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    /* Init GAP */
    blc_gap_peripheral_init();

    /* Set up GATT table */
    bls_att_setAttributeTable((u8 *)gattTable);

    /* L2CAP Initialization */
    blc_l2cap_register_handler((void *)blc_l2cap_packet_receive);

    /* Setup connect/terminate callbacks */
    bls_app_registerEventCallback(BLT_EV_FLAG_CONNECT, connectCallback);
    bls_app_registerEventCallback(BLT_EV_FLAG_TERMINATE, terminateCallback);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetCHIPoBLEServiceMode");

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{
    ble_sts_t status = BLE_SUCCESS;
    CHIP_ERROR err = CHIP_NO_ERROR;
    uint8_t index = 0;
    uint8_t devNameLen = 0;
    ChipBLEDeviceIdentificationInfo deviceIdInfo;
    u8 adv[CHIP_MAX_ADV_DATA_LEN] = {0};
    u8 srsp[CHIP_MAX_RESPONSE_DATA_LEN] = {0};

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
    devNameLen = strlen(mDeviceName);
    adv[index++] = devNameLen + 1;
    adv[index++] = CHIP_ADV_DATA_TYPE_NAME;
    memcpy(&adv[index], mDeviceName, devNameLen);
    index += devNameLen;

    /* Set advetisment data */
    status = bls_ll_setAdvData(adv, index);
    if(status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to set BLE advertisement data. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

    index = 0;
    srsp[index++] = CHIP_SHORT_UUID_LEN + 1;
    srsp[index++] = CHIP_ADV_DATA_TYPE_UUID;
    srsp[index++] = matterServiceUUID[0];
    srsp[index++] = matterServiceUUID[1];

    /* Set scan responce data */
    status = bls_ll_setScanRspData(srsp, sizeof(srsp));
    if(status != BLE_SUCCESS)
    {
        ChipLogError(DeviceLayer, "Fail to set BLE scan responce data. Error %d", status);

        return CHIP_ERROR_INCORRECT_STATE;
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    ble_sts_t status = BLE_SUCCESS;
    CHIP_ERROR err = CHIP_NO_ERROR;
    
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetAdvertisingEnabled");

    if(val == true)
    {
        ChipLogProgress(DeviceLayer, "Enable BLE");

        /* At first run always select fast advertising, on the next attemp slow down interval. */
        u16 intervalMin = mFlags.Has(Flags::kFastAdvertisingEnabled) ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN
                                                                     : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        u16 intervalMax = mFlags.Has(Flags::kFastAdvertisingEnabled) ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX
                                                                     : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;

        /* Configure CHIP BLE advertisement data */
        err = ConfigureAdvertisingData();
        if(err != CHIP_NO_ERROR)
        {
            ChipLogError(DeviceLayer, "Fail to config BLE advertisement data");

            return err;
        }

        /* Setup advertisement paramiters */
        status = bls_ll_setAdvParam(intervalMin, 
                                    intervalMax,
                                    ADV_TYPE_CONNECTABLE_UNDIRECTED, 
                                    OWN_ADDRESS_PUBLIC,
                                    0,  NULL,
                                    BLT_ENABLE_ADV_ALL,
                                    ADV_FP_NONE);
        if(status != BLE_SUCCESS)
        {
            ChipLogError(DeviceLayer, "Fail to set BLE advertisement paramiters. Error %d", status);

            return CHIP_ERROR_INCORRECT_STATE;
        }

        /* Enable advertisement */
        status = bls_ll_setAdvEnable(BLC_ADV_ENABLE);
        if(status != BLE_SUCCESS)
        {
            ChipLogError(DeviceLayer, "Fail to start BLE advertisement. Error %d", status);

            return CHIP_ERROR_INCORRECT_STATE;
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Disable BLE");
    }

    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::StartAdvertising");

    return _SetAdvertisingEnabled(true);
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::StopAdvertising");

    return _SetAdvertisingEnabled(false);
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetAdvertisingMode");

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_GetDeviceName");

    if (strlen(mDeviceName) >= bufSize)
    {
        return CHIP_ERROR_BUFFER_TOO_SMALL;
    }
    strcpy(buf, mDeviceName);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * devName)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_SetDeviceName");

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

    if(strlen(devName) >= kMaxDeviceNameLength)
    {
        ChipLogError(DeviceLayer, "BLE device name is to long");

        return CHIP_ERROR_INVALID_ARGUMENT;
    }

    strcpy(mDeviceName, devName);
    mFlags.Set(Flags::kDeviceNameSet);
    
    ChipLogProgress(DeviceLayer, "Setting device name to : \"%s\"", devName);

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(DeviceLayer, "BLEManagerImpl::_OnPlatformEvent");

    switch (event->Type)
    {
        case DeviceEventType::kPlatformTelinkBleRXWrite:
            ChipLogDetail(DeviceLayer, "Event: kPlatformTelinkBleRXWrite");

            err = HandleRXCharWrite(event);
        break;

        default:
            ChipLogDetail(DeviceLayer, "Event: Unknown (%d)", event->Type);
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Error: %s", ErrorStr(err));
    }
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic");

    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic");

    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::CloseConnection");

    return false;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::GetMTU");

    return 0;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle data)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendIndication()");

    return false;
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
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::NotifyChipConnectionClosed");

}

void BLEManagerImpl::DriveBLEState(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::DriveBLEState");

}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    return 0;
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::DriveBLEState");

}

CHIP_ERROR BLEManagerImpl::SetSubscribed(uint16_t conId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SetSubscribed");

    return CHIP_NO_ERROR;
}

bool BLEManagerImpl::UnsetSubscribed(uint16_t conId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::UnsetSubscribed");

    return false;
}

bool BLEManagerImpl::IsSubscribed(uint16_t conId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::IsSubscribed");

    return false;
}

void BLEManagerImpl::CancelBleAdvTimeoutTimer(void)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::CancelBleAdvTimeoutTimer");

}

void BLEManagerImpl::StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::StartBleAdvTimeoutTimer");

}

CHIP_ERROR BLEManagerImpl::HandleRXCharWrite(const ChipDeviceEvent * event)
{
    const BleRXWriteEventType * writeEvent = &event->Platform.BleRXWriteEvent;

    ChipLogDetail(DeviceLayer, "Write request received for CHIPoBLE RX (connHandle 0x%02" PRIx16 ")", 
                  writeEvent->connHandle);

    HandleWriteReceived(writeEvent->connHandle, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_RX,
                        PacketBufferHandle::Adopt(writeEvent->Data));

    return CHIP_NO_ERROR;
}


} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
