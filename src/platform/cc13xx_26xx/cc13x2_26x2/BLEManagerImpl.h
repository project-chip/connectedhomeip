/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
 *    Copyright (c) 2020 Texas Instruments Incorporated
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
 * Provides an implementation of the BLEManager object for the Texas
 * Instruments cc13xx_cc26xx platform.
 */

#pragma once

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "FreeRTOS.h"
#include <queue.h>
#include <task.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <icall.h>
#include <icall_ble_api.h>

#include "hal_types.h"

#include "ti_ble_config.h"
#include "ti_drivers_config.h"

#ifdef __cplusplus
}
#endif

#include "chipOBleProfile.h"

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

// Internal Events for RTOS application
#define ICALL_EVT ICALL_MSG_EVENT_ID  // Event_Id_31
#define QUEUE_EVT UTIL_QUEUE_EVENT_ID // Event_Id_30

// Application events
#define BLEManagerIMPL_STATE_UPDATE_EVT (0)

// 500ms interval
#define BLEMANAGERIMPL_ADV_INT_SLOW (800)
// 100ms interval (Default)
#define BLEMANAGERIMPL_ADV_INT_FAST (160)

#define CHIPOBLE_ADV_SIZE_NO_DEVICE_ID_INFO (4)

#define CHIPOBLE_SCANRES_SIZE_NO_NAME (6)

#define CHIPOBLE_ADV_DATA_MAX_SIZE (GAP_DEVICE_NAME_LEN + CHIPOBLE_SCANRES_SIZE_NO_NAME)

#define CHIPOBLE_DEVICE_DESC_LENGTH (4)

// How often to read current current RPA (in ms)
#define READ_RPA_EVT_PERIOD 3000

// 15 Minute Advertisement CHIP Timeout period
#define ADV_TIMEOUT (900000)

#define STATE_CHANGE_EVT 0
#define CHAR_CHANGE_EVT 1
#define CHIPOBLE_CHAR_CHANGE_EVT 2
#define BLEManagerIMPL_CHIPOBLE_TX_IND_EVT 3
#define ADV_EVT 4
#define PAIR_STATE_EVT 5
#define PASSCODE_EVT 6
#define READ_RPA_EVT 7
#define SEND_PARAM_UPDATE_EVT 8
#define BLEManagerIMPL_CHIPOBLE_CLOSE_CONN_EVT 9
#define CONN_EVT 10

// For storing the active connections
#define RSSI_TRACK_CHNLS 1 // Max possible channels can be GAP_BONDINGS_MAX
#define MAX_RSSI_STORE_DEPTH 5
#define INVALID_HANDLE 0xFFFF
#define RSSI_2M_THRSHLD -30
#define RSSI_1M_THRSHLD -40
#define RSSI_S2_THRSHLD -50
#define RSSI_S8_THRSHLD -60
#define PHY_NONE LL_PHY_NONE // No PHY set
#define AUTO_PHY_UPDATE 0xFF
// Set initial values to maximum, RX is set to max. by default(251 octets, 2120us)
// Some brand smartphone is essentially needing 251/2120, so we set them here.
#define BLEMANAGER_SUGGESTED_PDU_SIZE 251 // default is 27 octets(TX)
#define BLEMANAGER_SUGGESTED_TX_TIME 2120 // default is 328us(TX)

typedef struct
{
    uint8_t len;  // data length
    void * pData; // pointer to message
} CHIPoBLEIndEvt_t;

typedef struct
{
    uint8_t paramId;     // Parameter written
    uint16_t len;        //  data length
    uint16_t connHandle; // Active connection which received the write
} CHIPoBLEProfChgEvt_t;

// App event passed from stack modules. This type is defined by the application
// since it can queue events to itself however it wants.
typedef struct
{
    uint8_t event; // event type
    void * pData;  // pointer to message
} QueuedEvt_t;

// Container to store advertising event data when passing from advertising
// callback to app event. See the respective event in GapAdvScan_Event_IDs
// in gap_advertiser.h for the type that pBuf should be cast to.
typedef struct
{
    uint32_t event;
    void * pBuf;
} GapAdvEventData_t;

// Container to store information from clock expiration using a flexible array
// since data is not always needed
typedef struct
{
    uint8_t event;
    uint8_t data[];
} ClockEventData_t;

// List element for parameter update and PHY command status lists
typedef struct
{
    List_Elem elem;
    uint16_t connHandle;
} ConnHandleEntry_t;

// Connected device information
typedef struct
{
    uint16_t connHandle;          // Connection Handle
    ClockP_Struct * pUpdateClock; // pointer to clock struct
    int8_t rssiArr[MAX_RSSI_STORE_DEPTH];
    uint8_t rssiCntr;
    int8_t rssiAvg;
    bool phyCngRq; // Set to true if PHY change request is in progress
    uint8_t currPhy;
    uint8_t rqPhy;
    uint8_t phyRqFailCnt; // PHY change request count
    bool isAutoPHYEnable; // Flag to indicate auto phy change
    uint16_t mtu;
    ClockEventData_t * pParamUpdateEventData;
} ConnRec_t;

// Container to store passcode data when passing from gapbondmgr callback
// to app event. See the pfnPairStateCB_t documentation from the gapbondmgr.h
// header file for more information on each parameter.
typedef struct
{
    uint8_t state;
    uint16_t connHandle;
    uint8_t status;
} PairStateData_t;

// Container to store passcode data when passing from gapbondmgr callback
// to app event. See the pfnPasscodeCB_t documentation from the gapbondmgr.h
// header file for more information on each parameter.
typedef struct
{
    uint8_t deviceAddr[B_ADDR_LEN];
    uint16_t connHandle;
    uint8_t uiInputs;
    uint8_t uiOutputs;
    uint32_t numComparison;
} PasscodeData_t;

/**
 * Concrete implementation of the BLEManager singleton object for cc13x2_cc26x2.
 */
class BLEManagerImpl final : public BLEManager, private BleLayer, private BlePlatformDelegate, private BleApplicationDelegate

{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;

public:
    // ===== Platform-specific members that may be accessed directly by the application.

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

    const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42,
                                                     0x9F, 0x9D, 0x11 } };
    const ChipBleUUID chipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42,
                                                     0x9F, 0x9D, 0x12 } };

    friend BLEManager & BLEMgr(void);
    friend BLEManagerImpl & BLEMgrImpl(void);

    static BLEManagerImpl sInstance;

    /* BLE stack task handle */
    static TaskHandle_t sBleTaskHndl;
    // Entity ID globally used to check for source and/or destination of messages
    static ICall_EntityID sSelfEntity;

    // Event globally used to post local events and pend on system and
    // local events.
    static ICall_SyncHandle sSyncEvent;
    static QueueHandle_t sEventHandlerMsgQueueID;
    static chipOBleProfileCBs_t CHIPoBLEProfile_CBs;
    static gapBondCBs_t BLEMgr_BondMgrCBs;

    enum class Flags : uint16_t
    {
        kAdvertisingEnabled       = 0x0001, /* App enabled CHIPoBLE advertising */
        kFastAdvertisingEnabled   = 0x0002, /* App enabled Fash CHIPoBLE advertising */
        kAdvertising              = 0x0004, /* TI BLE stack actively advertising */
        kBLEStackInitialized      = 0x0008, /* TI BLE Stack GAP/GATT Intilization complete */
        kBLEStackAdvInitialized   = 0x0010, /* TI BLE Stack Advertisement Intilization complete */
        kBLEStackGATTNameUpdate   = 0x0020, /* Trigger TI BLE Stack name update, must be performed prior to adv start */
        kBLEStackGATTNameSet      = 0x0040, /* Device name has been set externally*/
        kAdvertisingRefreshNeeded = 0x0080, /* Advertising settings changed and it should be restarted */

    };

    BitFlags<Flags> mFlags;
    CHIPoBLEServiceMode mServiceMode;
    char mDeviceName[GAP_DEVICE_NAME_LEN];

    ConnRec_t connList[MAX_NUM_BLE_CONNS];
    // List to store connection handles for queued param updates
    List_List paramUpdateList;

    // Advertising handles
    uint8_t advHandleLegacy;
    // Address mode
    GAP_Addr_Modes_t addrMode = DEFAULT_ADDRESS_MODE;
    // Current Random Private Address
    uint8_t rpa[B_ADDR_LEN] = { 0 };

    uint8_t mAdvDatachipOBle[CHIPOBLE_ADV_DATA_MAX_SIZE];
    uint8_t mScanResDatachipOBle[CHIPOBLE_ADV_DATA_MAX_SIZE];

    ClockP_Struct clkRpaRead;
    ClockP_Struct clkAdvTimeout;
    // Memory to pass RPA read event ID to clock handler
    ClockEventData_t argRpaRead = { .event = READ_RPA_EVT };

    // ===== Private BLE Stack Helper functions.
    void ConfigureAdvertisements(void);
    void EventHandler_init(void);
    void InitPHYRSSIArray(void);
    CHIP_ERROR CreateEventHandler(void);
    uint8_t ProcessStackEvent(ICall_Hdr * pMsg);
    void ProcessEvtHdrMsg(QueuedEvt_t * pMsg);
    void ProcessGapMessage(gapEventHdr_t * pMsg);
    uint8_t ProcessGATTMsg(gattMsgEvent_t * pMsg);
    void ProcessAdvEvent(GapAdvEventData_t * pEventData);
    CHIP_ERROR ProcessParamUpdate(uint16_t connHandle);
    status_t EnqueueEvtHdrMsg(uint8_t event, void * pData);
    uint8_t AddBLEConn(uint16_t connHandle);
    uint8_t RemoveBLEConn(uint16_t connHandle);
    uint8_t GetBLEConnIndex(uint16_t connHandle) const;
    uint8_t ClearBLEConnListEntry(uint16_t connHandle);
    void ClearPendingBLEParamUpdate(uint16_t connHandle);
    void UpdateBLERPA(void);

    static void HandleIncomingBleConnection(Ble::BLEEndPoint * bleEP);

    /* Static helper function */
    static void EventHandler(void * arg);
    static CHIP_ERROR DriveBLEState(void);

    /* Declared static to acquire function ptr */
    static void advCallback(uint32_t event, void * pBuf, uintptr_t arg);
    static void ClockHandler(uintptr_t arg);
    static void AdvTimeoutHandler(uintptr_t arg);
    static void FastAdvTimeoutHandler(uintptr_t arg);
    static void CHIPoBLEProfile_charValueChangeCB(uint8_t paramId, uint16_t len, uint16_t connHandle);
    static void PasscodeCb(uint8_t * pDeviceAddr, uint16_t connHandle, uint8_t uiInputs, uint8_t uiOutputs, uint32_t numComparison);
    static void PairStateCb(uint16_t connHandle, uint8_t state, uint8_t status);
    static void AssertHandler(uint8 assertCause, uint8 assertSubcause);
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
 * that are specific to the cc13x2_cc26x2 platforms.
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
