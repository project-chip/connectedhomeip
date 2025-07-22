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
 * Instruments CC13XX_26XX platform.
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
#include <AppoBLE_interface.h>

namespace chip {
namespace DeviceLayer {
namespace Internal {

using namespace chip::Ble;

// Internal Events for RTOS application
#define ICALL_EVT ICALL_MSG_EVENT_ID  // Event_Id_31
#define QUEUE_EVT UTIL_QUEUE_EVENT_ID // Event_Id_30

// 500ms interval
#define BLEMANAGERIMPL_ADV_INT_SLOW (800)
// 100ms interval (Default)
#define BLEMANAGERIMPL_ADV_INT_FAST (160)

#define CHIPOBLE_ADV_SIZE_NO_DEVICE_ID_INFO (4)

#define CHIPOBLE_SCANRES_SIZE_NO_NAME (6)

#define CHIPOBLE_ADV_DATA_MAX_SIZE (GAP_DEVICE_NAME_LEN + CHIPOBLE_SCANRES_SIZE_NO_NAME)

#define CHIPOBLE_DEVICE_DESC_LENGTH (4)

// How often to read current current RPA (in ms)
#define BLEMGR_RPA_CB_UPDATE_PERIOD 3000

// 15 Minute Advertisement CHIP Timeout period
#define ADV_TIMEOUT (900000)

typedef enum BleMgr_events
{
    BLEMGR_ADV_STATE_UPDATE,
    BLEMGR_ADV_CB_UPDATE,
    BLEMGR_PAIR_STATE_CB_UPDATE,
    BLEMGR_PASSCODE_CB_UPDATE,
    BLEMGR_RPA_CB_UPDATE,
    BLEMGR_CONN_PARAM_CB_UPDATE,
    BLEMGR_CHIPOBLE_CONN_CLOSE,
    BLEMGR_CHIPOBLE_CHAR_CHANGE,
    BLEMGR_APPOBLE_ADV_API_ADD,
    BLEMGR_APPOBLE_ADV_API_REMOVE,
    BLEMGR_APPOBLE_CHAR_WRITE_REQ,
    BLEMGR_APPOBLE_CHAR_READ_REQ,
    BLEMGR_APPOBLE_CUSTOM_API_REQ,
    END_OF_BLE_MGR_RESERVE = 200, /* Start index of AppoBLE Optional Message Events */
} BleMgr_events_t;

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
    /*
    MatteroBLE Service Mode states:
    1. Disabled on startup
    2. Enabled once central initiates connection to peripheral AND subscribes to MatteroBLE characteristic
    3. Disabled once MatteroBLE characteristic is unsubscribed from
*/
    ConnectivityManager::CHIPoBLEServiceMode mServiceMode;
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
 * Concrete implementation of the BLEManager singleton object for CC13XX_26XX.
 */
class BLEManagerImpl final : public BLEManager,
                             public AppoBLE_interface,
                             private BleLayer,
                             private BlePlatformDelegate,
                             private BleApplicationDelegate
{
    // Allow the BLEManager interface class to delegate method calls to
    // the implementation methods provided by this class.
    friend BLEManager;
    friend AppoBLE_interface;

public:
/* AppoBLE */
#define MAX_NUM_ADV_SETS 2

    enum class AdvFlags : uint16_t
    {
        kAdvertisingEnabled       = 0x0001, /* App enabled CHIPoBLE advertising */
        kFastAdvertisingEnabled   = 0x0002, /* App enabled Fast CHIPoBLE advertising */
        kAdvertising              = 0x0004, /* TI BLE stack actively advertising */
        kAdvSetInitialized        = 0x0010, /* TI BLE Stack Advertisement Initialization complete */
        kAdvertisingRefreshNeeded = 0x0020, /* Advertising settings changed and it should be restarted */
    };

    typedef struct
    {
        uint8_t advHandle;
        uint16_t advProps;
        BitFlags<AdvFlags> advState;
        uint32_t slowAdvIntMin;
        uint32_t slowAdvIntMax;
        uint32_t fastAdvIntMin;
        uint32_t fastAdvIntMax;
    } AdvSet_t;

    // ===== Platform-specific members that may be accessed directly by the application.

    AdvSet_t mAdvSetArray[MAX_NUM_ADV_SETS];
    uint32_t pairingPasscode = B_APP_DEFAULT_PASSCODE;
// Helpers
#ifdef TI_APPOBLE_ENABLE
    bStatus_t EnqueueAppoBLEMsg(uint32_t event, void * pData);
    uint8_t SendAppoBLEAdvApi(AppoBLE_api_type_t api, AppoBLEAdvApi_msg * msg);
    void SetAppoBLEPairingPassCode(uint32_t passcode);
    void Generic_charValueChangeCB(uint8_t * servUUID, uint8_t * charUUID, uint8_t len, uint8_t connHandle);
    bool SendGenericFxnReq(pfnCallInBleMgrCtx_t FxnPtr, void * arg);
#endif
private:
    uint8_t RemoveAdvSet(uint8_t inputAdvIndex);

    uint8_t AddUpdateAdvSet(uint8_t inputAdvIndex, GapAdv_params_t advParams, const uint8_t * advData, uint8_t advDataLen,
                            const uint8_t * scanRspData, uint8_t scanRspDataLen, GapAdv_eventMaskFlags_t evtMask);
    BLE_CONNECTION_OBJECT GetConnection(uint8_t connIndex);

    uint8_t SetAdvInterval(uint8_t advIndex, uint32_t intervalMax, uint32_t intervalMin);

    CHIP_ERROR AddUpdateMatteroBLEAdv(void);
    void UpdateAdvInterval(uint8_t advIndex);

    uint8_t getAdvIndex(uint8_t advHandle);

    bool isAdvertisingEnabled(uint8_t advIndex);
    uint8_t setAdvertisingEnabled(bool val, uint8_t advIndex);
    bool isAdvertising(uint8_t advIndex);
    CHIP_ERROR setAdvertisingMode(BLEAdvertisingMode mode, uint8_t advIndex);

    uint8_t setAdvertisingInterval(uint8_t advIndex, uint32_t intervalMax, uint32_t intervalMin);
    ConnectivityManager::CHIPoBLEServiceMode getMatteroBLEServiceMode(BLE_CONNECTION_OBJECT conId);

    uint8_t getDeviceName(char * buf, size_t bufSize);
    uint8_t setDeviceName(const char * deviceName);

    bool SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId, uint8_t * srcBuf,
                        uint16_t srcBufLen);

    bool SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                          uint8_t * srcBuf, uint16_t srcBufLen);
    bool SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId, uint8_t * const dstBuf,
                         uint16_t dstBufLen);
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

    CHIP_ERROR SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                       const Ble::ChipBleUUID * charId) override;
    CHIP_ERROR UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId,
                                         const Ble::ChipBleUUID * charId) override;
    CHIP_ERROR CloseConnection(BLE_CONNECTION_OBJECT conId) override;
    uint16_t GetMTU(BLE_CONNECTION_OBJECT conId) const override;

    CHIP_ERROR SendIndication(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                              System::PacketBufferHandle pBuf) override;
    CHIP_ERROR SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                System::PacketBufferHandle pBuf) override;

    // ===== Members that implement virtual methods on BleApplicationDelegate.

    void NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) override;

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

    char mDeviceName[GAP_DEVICE_NAME_LEN];

    ConnRec_t connList[MAX_NUM_BLE_CONNS];
    // List to store connection handles for queued param updates
    List_List paramUpdateList;

    // Advertising handles
    uint8_t matterAdvIndex;
    // Address mode
    GAP_Addr_Modes_t addrMode = DEFAULT_ADDRESS_MODE;
    // Current Random Private Address
    uint8_t rpa[B_ADDR_LEN] = { 0 };

    uint8_t mAdvDatachipOBle[CHIPOBLE_ADV_DATA_MAX_SIZE];
    uint8_t mScanResDatachipOBle[CHIPOBLE_ADV_DATA_MAX_SIZE];

    ClockP_Struct clkRpaRead;
    ClockP_Struct clkAdvTimeout;
    // Memory to pass RPA read event ID to clock handler
    ClockEventData_t argRpaRead = { .event = BLEMGR_RPA_CB_UPDATE };

    // ===== Private BLE Stack Helper functions.
    void ConfigureAdvertisements(void);
    void EventHandler_init(void);
    void InitPHYRSSIArray(void);
    CHIP_ERROR CreateEventHandler(void);
    uint8_t ProcessBLEStackEvent(ICall_Hdr * pMsg);
    void ProcessBleMgrEvt(GenericQueuedEvt_t * pMsg);
    void ProcessGapMessage(gapEventHdr_t * pMsg);
    uint8_t ProcessGATTMsg(gattMsgEvent_t * pMsg);
    void ProcessAdvEvent(GapAdvEventData_t * pEventData);
    CHIP_ERROR ProcessParamUpdate(uint16_t connHandle);
    status_t EnqueueBLEMgrMsg(uint32_t event, void * pData);
    uint8_t AddBLEConn(uint16_t connHandle);
    uint8_t RemoveBLEConn(uint16_t connHandle);
    uint8_t GetBLEConnIndex(uint16_t connHandle) const;
    uint8_t ClearBLEConnListEntry(uint16_t connHandle);
    void ClearPendingBLEParamUpdate(uint16_t connHandle);
    void UpdateBLERPA(void);

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
 * that are specific to the CC13XX_26XX platforms.
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
