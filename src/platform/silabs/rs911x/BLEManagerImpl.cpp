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
 *    @file
 *          Provides an implementation of the BLEManager singleton object
 *          for the Silicon Labs EFR32 platforms.
 */

/* this file behaves like a config.h, comes first */
#include <platform/internal/CHIPDeviceLayerInternal.h>
#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "wfx_sl_ble_init.h"
#include <ble/Ble.h>
#include <crypto/RandUtils.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/internal/BLEManager.h>

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#endif

extern "C" {
#if !(SLI_SI91X_MCU_INTERFACE | EXP_BOARD)
#include <rsi_driver.h>
#endif
#include <rsi_utils.h>
}

#define BLE_MIN_CONNECTION_INTERVAL_MS 24
#define BLE_MAX_CONNECTION_INTERVAL_MS 40
#define BLE_SLAVE_LATENCY_MS 0
#define BLE_TIMEOUT_MS 400
#define BLE_SEND_INDICATION_TIMER_PERIOD_MS (5000)

using namespace ::chip;
using namespace ::chip::Ble;
using namespace ::chip::DeviceLayer::Internal;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

#define CHIP_ADV_DATA_TYPE_FLAGS 0x01
#define CHIP_ADV_DATA_TYPE_UUID 0x03
#define CHIP_ADV_DATA_TYPE_NAME 0x09
#define CHIP_ADV_DATA_TYPE_SERVICE_DATA 0x16

#define CHIP_ADV_DATA_FLAGS 0x06

#define CHIP_ADV_DATA 0
#define CHIP_ADV_SCAN_RESPONSE_DATA 1
#define CHIP_ADV_SHORT_UUID_LEN 2

#define MAX_RESPONSE_DATA_LEN 31
#define MAX_ADV_DATA_LEN 31

// Timer Frequency used.
#define TIMER_CLK_FREQ ((uint32_t) 32768)

// Convert msec to timer ticks.
#define TIMER_MS_2_TIMERTICK(ms) ((TIMER_CLK_FREQ * ms) / 1000)
#define TIMER_S_2_TIMERTICK(s) (TIMER_CLK_FREQ * s)

const uint8_t UUID_CHIPoBLEService[]      = { 0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
                                              0x00, 0x10, 0x00, 0x00, 0xF6, 0xFF, 0x00, 0x00 };
const uint8_t ShortUUID_CHIPoBLEService[] = { 0xF6, 0xFF };

// Used to send the Indication Confirmation
uint8_t dev_address[RSI_DEV_ADDR_LEN];
uint16_t rsi_ble_measurement_hndl;
uint16_t rsi_ble_gatt_server_client_config_hndl;

osTimerId_t sbleAdvTimeoutTimer;

osThreadId_t sBleThread;
constexpr uint32_t kBleTaskSize = 2560;
uint8_t bleStack[kBleTaskSize];
osThread_t sBleTaskControlBlock;
constexpr osThreadAttr_t kBleTaskAttr = { .name       = "rsi_ble",
                                          .attr_bits  = osThreadDetached,
                                          .cb_mem     = &sBleTaskControlBlock,
                                          .cb_size    = osThreadCbSize,
                                          .stack_mem  = bleStack,
                                          .stack_size = kBleTaskSize,
                                          .priority   = osPriorityHigh };

void rsi_ble_add_matter_service(void)
{
    constexpr uuid_t custom_service                         = { .size = RSI_BLE_MATTER_CUSTOM_SERVICE_SIZE,
                                                                .val  = { .val16 = RSI_BLE_MATTER_CUSTOM_SERVICE_VALUE_16 } };
    uint8_t data[RSI_BLE_MATTER_CUSTOM_SERVICE_DATA_LENGTH] = { RSI_BLE_MATTER_CUSTOM_SERVICE_DATA };

    constexpr uuid_t custom_characteristic_RX = { .size     = RSI_BLE_CUSTOM_CHARACTERISTIC_RX_SIZE,
                                                  .reserved = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_RESERVED },
                                                  .val      = { .val128 = {
                                                                    .data1 = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_1 },
                                                                    .data2 = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_2 },
                                                                    .data3 = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_3 },
                                                                    .data4 = { RSI_BLE_CUSTOM_CHARACTERISTIC_RX_VALUE_128_DATA_4 } } } };

    rsi_ble_resp_add_serv_t new_serv_resp = { 0 };
    rsi_ble_add_service(custom_service, &new_serv_resp);

    // Adding custom characteristic declaration to the custom service
    SilabsBleWrapper::rsi_ble_add_char_serv_att(
        new_serv_resp.serv_handler, new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_RX_ATTRIBUTE_HANDLE_LOCATION,
        RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ, // Set read, write, write without response
        new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_RX_VALUE_HANDLE_LOCATION, custom_characteristic_RX);

    // Adding characteristic value attribute to the service
    SilabsBleWrapper::rsi_ble_add_char_val_att(
        new_serv_resp.serv_handler, new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_RX_VALUE_HANDLE_LOCATION,
        custom_characteristic_RX,
        RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ, // Set read, write, write without response
        data, sizeof(data), ATT_REC_IN_HOST);

    constexpr uuid_t custom_characteristic_TX = { .size     = RSI_BLE_CUSTOM_CHARACTERISTIC_TX_SIZE,
                                                  .reserved = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_RESERVED },
                                                  .val      = { .val128 = {
                                                                    .data1 = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_1 },
                                                                    .data2 = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_2 },
                                                                    .data3 = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_3 },
                                                                    .data4 = { RSI_BLE_CUSTOM_CHARACTERISTIC_TX_VALUE_128_DATA_4 } } } };

    // Adding custom characteristic declaration to the custom service
    SilabsBleWrapper::rsi_ble_add_char_serv_att(
        new_serv_resp.serv_handler, new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_ATTRIBUTE_HANDLE_LOCATION,
        RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE | RSI_BLE_ATT_PROPERTY_WRITE | RSI_BLE_ATT_PROPERTY_READ |
            RSI_BLE_ATT_PROPERTY_NOTIFY | RSI_BLE_ATT_PROPERTY_INDICATE, // Set read, write, write without response
        new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_MEASUREMENT_HANDLE_LOCATION, custom_characteristic_TX);

    // Adding characteristic value attribute to the service
    rsi_ble_measurement_hndl = new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_MEASUREMENT_HANDLE_LOCATION;

    // Adding characteristic value attribute to the service
    rsi_ble_gatt_server_client_config_hndl =
        new_serv_resp.start_handle + RSI_BLE_CHARACTERISTIC_TX_GATT_SERVER_CLIENT_HANDLE_LOCATION;

    SilabsBleWrapper::rsi_ble_add_char_val_att(new_serv_resp.serv_handler, rsi_ble_measurement_hndl, custom_characteristic_TX,
                                               RSI_BLE_ATT_PROPERTY_WRITE_NO_RESPONSE | RSI_BLE_ATT_PROPERTY_WRITE |
                                                   RSI_BLE_ATT_PROPERTY_READ | RSI_BLE_ATT_PROPERTY_NOTIFY |
                                                   RSI_BLE_ATT_PROPERTY_INDICATE, // Set read, write, write without response
                                               data, sizeof(data), ATT_REC_MAINTAIN_IN_HOST);
}

} // namespace

BLEManagerImpl BLEManagerImpl::sInstance;

void BLEManagerImpl::ProcessEvent(SilabsBleWrapper::BleEvent_t inEvent)
{
    switch (inEvent.eventType)
    {
    case SilabsBleWrapper::BleEventType::RSI_BLE_CONN_EVENT: {
        BLEMgrImpl().HandleConnectEvent(inEvent.eventData);
        // Requests the connection parameters change with the remote device
        rsi_ble_conn_params_update(inEvent.eventData.resp_enh_conn.dev_addr, BLE_MIN_CONNECTION_INTERVAL_MS,
                                   BLE_MAX_CONNECTION_INTERVAL_MS, BLE_SLAVE_LATENCY_MS, BLE_TIMEOUT_MS);
        rsi_ble_set_data_len(inEvent.eventData.resp_enh_conn.dev_addr, RSI_BLE_TX_OCTETS, RSI_BLE_TX_TIME);

        // Used to send the Indication confirmation
        memcpy(dev_address, inEvent.eventData.resp_enh_conn.dev_addr, RSI_DEV_ADDR_LEN);
    }
    break;
    case SilabsBleWrapper::BleEventType::RSI_BLE_DISCONN_EVENT: {
        // event invokes when disconnection was completed
        BLEMgrImpl().HandleConnectionCloseEvent(inEvent.eventData);
    }
    break;
    case SilabsBleWrapper::BleEventType::RSI_BLE_MTU_EVENT: {
        // event invokes when write/notification events received
        BLEMgrImpl().UpdateMtu(inEvent.eventData);
    }
    break;
    case SilabsBleWrapper::BleEventType::RSI_BLE_EVENT_GATT_RD: {
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
        if (inEvent.eventData.rsi_ble_read_req->type == 0)
        {
            BLEMgrImpl().HandleC3ReadRequest(inEvent.eventData);
        }
#endif // CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    }
    break;
    case SilabsBleWrapper::BleEventType::RSI_BLE_GATT_WRITE_EVENT: {
        // event invokes when write/notification events received
        BLEMgrImpl().HandleWriteEvent(inEvent.eventData);
    }
    break;
    case SilabsBleWrapper::BleEventType::RSI_BLE_GATT_INDICATION_CONFIRMATION: {
        BLEMgrImpl().HandleTxConfirmationEvent(1);
    }
    break;
    default:
        break;
    }
}

void BLEManagerImpl::BlePostEvent(SilabsBleWrapper::BleEvent_t * event)
{
    sl_status_t status = osMessageQueuePut(sInstance.sBleEventQueue, event, 0, 0);
    if (status != osOK)
    {
        ChipLogError(DeviceLayer, "BlePostEvent: failed to post event: 0x%lx", status);
        // TODO: Handle error, requeue event depending on queue size or notify relevant task, Chipdie, etc.
    }
}

void BLEManagerImpl::sl_ble_event_handling_task(void * args)
{
    sl_status_t status;
    SilabsBleWrapper::BleEvent_t bleEvent;

    // This function initialize BLE and start BLE advertisement.
    sInstance.sl_ble_init();

    // Application event map
    while (1)
    {
        status = osMessageQueueGet(sInstance.sBleEventQueue, &bleEvent, NULL, osWaitForever);
        if (status == osOK)
        {
            sInstance.ProcessEvent(bleEvent);
        }
        else
        {
            ChipLogError(DeviceLayer, "sl_ble_event_handling_task: get event failed: 0x%lx", static_cast<uint32_t>(status));
        }
    }
}

void BLEManagerImpl::sl_ble_init()
{
    uint8_t randomAddrBLE[RSI_BLE_ADDR_LENGTH] = { 0 };
    uint64_t randomAddr                        = chip::Crypto::GetRandU64();
    memcpy(randomAddrBLE, &randomAddr, RSI_BLE_ADDR_LENGTH);
    // Set the two least significant bits as the first 2 bits of the address has to be '11' to ensure the address is a random
    // non-resolvable private address
    randomAddrBLE[(RSI_BLE_ADDR_LENGTH - 1)] |= 0xC0;

    // registering the GAP callback functions
    rsi_ble_gap_register_callbacks(NULL, NULL, SilabsBleWrapper::rsi_ble_on_disconnect_event, NULL, NULL, NULL,
                                   SilabsBleWrapper::rsi_ble_on_enhance_conn_status_event, NULL, NULL, NULL);

    // registering the GATT call back functions
    rsi_ble_gatt_register_callbacks(NULL, NULL, NULL, NULL, NULL, NULL, NULL, SilabsBleWrapper::rsi_ble_on_gatt_write_event, NULL,
                                    NULL, SilabsBleWrapper::rsi_ble_on_read_req_event, SilabsBleWrapper::rsi_ble_on_mtu_event, NULL,
                                    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                                    SilabsBleWrapper::rsi_ble_on_event_indication_confirmation, NULL);

    //  Exchange of GATT info with BLE stack
    rsi_ble_add_matter_service();
    rsi_ble_set_random_address_with_value(randomAddrBLE);

    sInstance.sBleEventQueue = osMessageQueueNew(WFX_QUEUE_SIZE, sizeof(SilabsBleWrapper::BleEvent_t), NULL);
    VerifyOrDie(sInstance.sBleEventQueue != nullptr);

    chip::DeviceLayer::Internal::BLEMgrImpl().HandleBootEvent();
}

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    sBleThread = osThreadNew(sInstance.sl_ble_event_handling_task, NULL, &kBleTaskAttr);

    VerifyOrReturnError(sBleThread != nullptr, CHIP_ERROR_INCORRECT_STATE);

    // Initialize the CHIP BleLayer.
    err = BleLayer::Init(this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    memset(mBleConnections, 0, sizeof(mBleConnections));
    memset(mIndConfId, kUnusedIndex, sizeof(mIndConfId));
    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;

    // SW timer for BLE timeouts and interval change.
    sbleAdvTimeoutTimer = osTimerNew(BleAdvTimeoutHandler, osTimerOnce, NULL, NULL);

    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

void BLEManagerImpl::OnSendIndicationTimeout(System::Layer * aLayer, void * appState)
{
    // TODO: change the connection handle with the ble device ID
    uint8_t connHandle = 1;
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::HandleSoftTimerEvent CHIPOBLE_PROTOCOL_ABORT");
    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
    event.CHIPoBLEConnectionError.ConId  = connHandle;
    event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
    PlatformMgr().PostEventOrDie(&event);
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
    ChipLogProgress(DeviceLayer, "_SetDeviceName Started");
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported)
    {
        ChipLogProgress(DeviceLayer, "_SetDeviceName CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE");
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }
    if (deviceName != NULL && deviceName[0] != 0)
    {
        if (strlen(deviceName) >= kMaxDeviceNameLength)
        {
            ChipLogProgress(DeviceLayer, "_SetDeviceName CHIP_ERROR_INVALID_ARGUMENT");
            return CHIP_ERROR_INVALID_ARGUMENT;
        }
        strcpy(mDeviceName, deviceName);
        mFlags.Set(Flags::kDeviceNameSet);
        mFlags.Set(Flags::kRestartAdvertising);
        ChipLogProgress(DeviceLayer, "Setting device name to : \"%s\"", mDeviceName);
    }
    else
    {
        mDeviceName[0] = 0;
    }
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
    ChipLogProgress(DeviceLayer, "_SetDeviceName Ended");
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe: {
        ChipDeviceEvent connEstEvent;

        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLESubscribe");
        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        connEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
        PlatformMgr().PostEventOrDie(&connEstEvent);
    }
    break;

    case DeviceEventType::kCHIPoBLEUnsubscribe: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEUnsubscribe");
        HandleUnsubscribeReceived(event->CHIPoBLEUnsubscribe.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
    }
    break;

    case DeviceEventType::kCHIPoBLEWriteReceived: {
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent kCHIPoBLEWriteReceived");
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID,
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
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
    }
    break;

    default:
        ChipLogProgress(DeviceLayer, "_OnPlatformEvent default:  event->Type = %d", event->Type);
        break;
    }
}

CHIP_ERROR BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                   const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic() not supported");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                     const ChipBleUUID * charId)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic() not supported");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "sl_bt_connection_close() failed: %s", ErrorStr(err));
    }

    return err;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    CHIPoBLEConState * conState = const_cast<BLEManagerImpl *>(this)->GetConnectionState(conId);
    return (conState != NULL) ? conState->mtu : 0;
}

CHIP_ERROR BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                          PacketBufferHandle data)
{
    int32_t status = 0;
    status         = rsi_ble_indicate_value(dev_address, rsi_ble_measurement_hndl, data->DataLength(), data->Start());
    if (status != RSI_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "indication failed with error code %lx ", status);
        return BLE_ERROR_GATT_INDICATE_FAILED;
    }

    // start timer for the indication Confirmation Event
    DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(BLE_SEND_INDICATION_TIMER_PERIOD_MS),
                                          OnSendIndicationTimeout, this);
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                            PacketBufferHandle pBuf)
{
    ChipLogProgress(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // Nothing to do
}

// TODO: Need to add RSI BLE STATUS codes
CHIP_ERROR BLEManagerImpl::MapBLEError(int bleErr)
{
    switch (bleErr)
    {
    case SL_STATUS_OK:
        return CHIP_NO_ERROR;
    case SL_STATUS_BT_ATT_INVALID_ATT_LENGTH:
        return CHIP_ERROR_INVALID_STRING_LENGTH;
    case SL_STATUS_INVALID_PARAMETER:
        return CHIP_ERROR_INVALID_ARGUMENT;
    case SL_STATUS_INVALID_STATE:
        return CHIP_ERROR_INCORRECT_STATE;
    case SL_STATUS_NOT_SUPPORTED:
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    default:
        return CHIP_ERROR(ChipError::Range::kPlatform, bleErr + CHIP_DEVICE_CONFIG_SILABS_BLE_ERROR_MIN);
    }
}

void BLEManagerImpl::DriveBLEState(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    // Check if BLE stack is initialized ( TODO )
    VerifyOrExit(mFlags.Has(Flags::kSiLabsBLEStackInitialize), /* */);

    // Start advertising if needed...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && mFlags.Has(Flags::kAdvertisingEnabled) &&
        NumConnections() < kMaxConnections)
    {
        // Start/re-start advertising if not already started, or if there is a pending change
        // to the advertising configuration.
        if (!mFlags.Has(Flags::kAdvertising) || mFlags.Has(Flags::kRestartAdvertising))
        {
            ChipLogProgress(DeviceLayer, "Start Advertising");
            err = StartAdvertising();
            SuccessOrExit(err);
        }
    }

    // Otherwise, stop advertising if it is enabled.
    else if (mFlags.Has(Flags::kAdvertising))
    {
        ChipLogProgress(DeviceLayer, "Stop Advertising");
        err = StopAdvertising();
        SuccessOrExit(err);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

CHIP_ERROR BLEManagerImpl::ConfigureAdvertisingData(void)
{

    ChipBLEDeviceIdentificationInfo mDeviceIdInfo;
    CHIP_ERROR err;
    int32_t result;
    uint8_t responseData[MAX_RESPONSE_DATA_LEN];
    uint8_t advData[MAX_ADV_DATA_LEN];
    uint32_t index              = 0;
    uint32_t mDeviceNameLength  = 0;
    uint8_t mDeviceIdInfoLength = 0;
    err                         = ConfigurationMgr().GetBLEDeviceIdentificationInfo(mDeviceIdInfo);

    ChipLogProgress(DeviceLayer, "ConfigureAdvertisingData start");

    VerifyOrExit((kMaxDeviceNameLength + 1) < UINT8_MAX, err = CHIP_ERROR_INVALID_ARGUMENT);

    memset(responseData, 0, MAX_RESPONSE_DATA_LEN);
    memset(advData, 0, MAX_ADV_DATA_LEN);

    SuccessOrExit(err);

    if (!mFlags.Has(Flags::kDeviceNameSet))
    {
        uint16_t discriminator;
        SuccessOrExit(err = GetCommissionableDataProvider()->GetSetupDiscriminator(discriminator));

        snprintf(mDeviceName, sizeof(mDeviceName), "%s%04u", CHIP_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX, discriminator);

        mDeviceName[kMaxDeviceNameLength] = 0;
        mDeviceNameLength                 = strlen(mDeviceName);

        VerifyOrExit(mDeviceNameLength < kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);
    }

    mDeviceNameLength = strlen(mDeviceName); // Device Name length + length field
    VerifyOrExit(mDeviceNameLength < kMaxDeviceNameLength, err = CHIP_ERROR_INVALID_ARGUMENT);
    static_assert((kFlagTlvSize + kUUIDTlvSize + kDeviceNameTlvSize) <= MAX_RESPONSE_DATA_LEN, "Scan Response buffer is too small");

    mDeviceIdInfoLength = sizeof(mDeviceIdInfo); // Servicedatalen + length+ UUID (Short)
    static_assert(sizeof(mDeviceIdInfo) + CHIP_ADV_SHORT_UUID_LEN + 1 <= UINT8_MAX, "Our length won't fit in a uint8_t");
    static_assert(2 + CHIP_ADV_SHORT_UUID_LEN + sizeof(mDeviceIdInfo) + 1 <= MAX_ADV_DATA_LEN, "Our buffer is not big enough");

    index            = 0;
    advData[index++] = 0x02;                                                                    // length
    advData[index++] = CHIP_ADV_DATA_TYPE_FLAGS;                                                // AD type : flags
    advData[index++] = CHIP_ADV_DATA_FLAGS;                                                     // AD value
    advData[index++] = static_cast<uint8_t>(mDeviceIdInfoLength + CHIP_ADV_SHORT_UUID_LEN + 1); // AD length
    advData[index++] = CHIP_ADV_DATA_TYPE_SERVICE_DATA;                                         // AD type : Service Data
    advData[index++] = ShortUUID_CHIPoBLEService[0];                                            // AD value
    advData[index++] = ShortUUID_CHIPoBLEService[1];

    memcpy(&advData[index], (void *) &mDeviceIdInfo, mDeviceIdInfoLength); // AD value
    index += mDeviceIdInfoLength;

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    ReturnErrorOnFailure(EncodeAdditionalDataTlv());
#endif

    result = rsi_ble_set_advertise_data(advData, index);
    if (result != SL_STATUS_OK)
    {
        //    err = MapBLEError(result);
        ChipLogError(DeviceLayer, "rsi_ble_set_advertise_data() failed: %ld", result);
        ExitNow();
    }
    else
    {
        ChipLogError(DeviceLayer, "rsi_ble_set_advertise_data() success: %ld", result);
    }
    index                 = 0;
    responseData[index++] = 0x02;                     // length
    responseData[index++] = CHIP_ADV_DATA_TYPE_FLAGS; // AD type : flags
    responseData[index++] = CHIP_ADV_DATA_FLAGS;
    responseData[index++] = CHIP_ADV_SHORT_UUID_LEN + 1;  // AD length
    responseData[index++] = CHIP_ADV_DATA_TYPE_UUID;      // AD type : uuid
    responseData[index++] = ShortUUID_CHIPoBLEService[0]; // AD value
    responseData[index++] = ShortUUID_CHIPoBLEService[1];

    responseData[index++] = static_cast<uint8_t>(mDeviceNameLength + 1); // length
    responseData[index++] = CHIP_ADV_DATA_TYPE_NAME;                     // AD type : name
    memcpy(&responseData[index], mDeviceName, mDeviceNameLength);        // AD value
    index += mDeviceNameLength;

    rsi_ble_set_scan_response_data(responseData, index);

    ChipLogProgress(DeviceLayer, "ConfigureAdvertisingData End");
exit:
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err          = CHIP_NO_ERROR;
    int32_t status          = 0;
    bool postAdvChangeEvent = false;

    ChipLogProgress(DeviceLayer, "StartAdvertising start");

    // If already advertising, stop it, before changing values
    if (mFlags.Has(Flags::kAdvertising))
    {
        status = rsi_ble_stop_advertising();
        if (status != RSI_SUCCESS)
        {
            ChipLogProgress(DeviceLayer, "advertising failed to stop, with status = 0x%lx ", status);
        }
    }
    else
    {
        ChipLogDetail(DeviceLayer, "Start BLE advertisement");
        postAdvChangeEvent = true;
    }

    if (!(mFlags.Has(Flags::kAdvertising)))
    {
        err = ConfigureAdvertisingData();
        SuccessOrExit(err);
    }

    mFlags.Clear(Flags::kRestartAdvertising);

    status = sInstance.SendBLEAdvertisementCommand();

    if (status == RSI_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "rsi_ble_start_advertising Success");

        if (mFlags.Has(Flags::kFastAdvertisingEnabled))
        {
            StartBleAdvTimeoutTimer(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME);
        }
        mFlags.Set(Flags::kAdvertising);

        if (postAdvChangeEvent)
        {
            // Post CHIPoBLEAdvertisingChange event.
            ChipDeviceEvent advChange;
            advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
            advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Started;

            ReturnErrorOnFailure(PlatformMgr().PostEvent(&advChange));
        }
    }
    else
    {
        ChipLogProgress(DeviceLayer, "rsi_ble_start_advertising Failed with status: %lx", status);
    }

exit:
    // TODO: Add MapBLEError to return the correct error code
    ChipLogError(DeviceLayer, "StartAdvertising() End error: %s", ErrorStr(err));
    return err;
}

int32_t BLEManagerImpl::SendBLEAdvertisementCommand(void)
{

    rsi_ble_req_adv_t ble_adv = { 0 };

    ble_adv.status = RSI_BLE_START_ADV;

    ble_adv.adv_type         = RSI_BLE_ADV_TYPE;
    ble_adv.filter_type      = RSI_BLE_ADV_FILTER_TYPE;
    ble_adv.direct_addr_type = RSI_BLE_ADV_DIR_ADDR_TYPE;
    rsi_ascii_dev_address_to_6bytes_rev(ble_adv.direct_addr, (int8_t *) RSI_BLE_ADV_DIR_ADDR);
    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        ble_adv.adv_int_min = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
        ble_adv.adv_int_max = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
    }
    else
    {
        ble_adv.adv_int_min = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        ble_adv.adv_int_max = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
    }
    ble_adv.own_addr_type   = LE_RANDOM_ADDRESS;
    ble_adv.adv_channel_map = RSI_BLE_ADV_CHANNEL_MAP;
    return rsi_ble_start_advertising_with_values(&ble_adv);
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int32_t status = 0;

    if (mFlags.Has(Flags::kAdvertising))
    {
        // Since DriveBLEState is not called the device is still advertising
        status = rsi_ble_stop_advertising();
        if (status != RSI_SUCCESS)
        {
            mFlags.Clear(Flags::kAdvertising).Clear(Flags::kRestartAdvertising);
            mFlags.Set(Flags::kFastAdvertisingEnabled, true);
            advertising_set_handle = 0xff;
            CancelBleAdvTimeoutTimer();

            // Post CHIPoBLEAdvertisingChange event.
            ChipDeviceEvent advChange;
            advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
            advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
            err                                        = PlatformMgr().PostEvent(&advChange);
        }
    }

    // TODO: Add MapBLEError to return the correct error code
    return err;
}

void BLEManagerImpl::UpdateMtu(const SilabsBleWrapper::sl_wfx_msg_t & evt)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(evt.connectionHandle);
    if (bleConnState != NULL)
    {
        // bleConnState->MTU is a 10-bit field inside a uint16_t.  We're
        // assigning to it from a uint16_t, and compilers warn about
        // possibly not fitting.  There's no way to suppress that warning
        // via explicit cast; we have to disable the warning around the
        // assignment.
        //
        // TODO: https://github.com/project-chip/connectedhomeip/issues/2569
        // tracks making this safe with a check or explaining why no check
        // is needed.
        ChipLogProgress(DeviceLayer, "DriveBLEState UpdateMtu %d", evt.rsi_ble_mtu.mtu_size);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
        bleConnState->mtu = evt.rsi_ble_mtu.mtu_size;
#pragma GCC diagnostic pop
        ;
    }
}

void BLEManagerImpl::HandleBootEvent(void)
{
    mFlags.Set(Flags::kSiLabsBLEStackInitialize);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerImpl::HandleConnectEvent(const SilabsBleWrapper::sl_wfx_msg_t & evt)
{
    AddConnection(evt.connectionHandle, evt.bondingHandle);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
}

void BLEManagerImpl::HandleConnectionCloseEvent(const SilabsBleWrapper::sl_wfx_msg_t & evt)
{
    uint8_t connHandle = 1;

    if (RemoveConnection(connHandle))
    {
        ChipDeviceEvent event;
        event.Type                          = DeviceEventType::kCHIPoBLEConnectionError;
        event.CHIPoBLEConnectionError.ConId = connHandle;

        switch (evt.reason)
        {

        case RSI_BT_CTRL_REMOTE_USER_TERMINATED:
        case RSI_BT_CTRL_REMOTE_DEVICE_TERMINATED_CONNECTION_DUE_TO_LOW_RESOURCES:
        case RSI_BT_CTRL_REMOTE_POWERING_OFF:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;
        default:
            event.CHIPoBLEConnectionError.Reason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
        }

        ChipLogProgress(DeviceLayer, "BLE GATT connection closed (con %u, reason %x)", connHandle, evt.reason);

        PlatformMgr().PostEventOrDie(&event);

        // Arrange to re-enable connectable advertising in case it was disabled due to the
        // maximum connection limit being reached.
        mFlags.Set(Flags::kRestartAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

void BLEManagerImpl::HandleWriteEvent(const SilabsBleWrapper::sl_wfx_msg_t & evt)
{
    ChipLogProgress(DeviceLayer, "Char Write Req, packet type %d", evt.rsi_ble_write.pkt_type);

    if (evt.rsi_ble_write.handle[0] == (uint8_t) rsi_ble_gatt_server_client_config_hndl) // TODO:: compare the handle exactly
    {
        HandleTXCharCCCDWrite(evt);
    }
    else
    {
        HandleRXCharWrite(evt);
    }
}

void BLEManagerImpl::HandleTXCharCCCDWrite(const SilabsBleWrapper::sl_wfx_msg_t & evt)
{
    CHIP_ERROR err           = CHIP_NO_ERROR;
    bool isIndicationEnabled = false;
    ChipDeviceEvent event;
    CHIPoBLEConState * bleConnState;

    bleConnState = GetConnectionState(evt.connectionHandle);
    VerifyOrExit(bleConnState != NULL, err = CHIP_ERROR_NO_MEMORY);

    // Determine if the client is enabling or disabling notification/indication.
    if (evt.rsi_ble_write.att_value[0] != 0)
    {
        isIndicationEnabled = true;
    }
    ChipLogProgress(DeviceLayer, "HandleTXcharCCCDWrite - Config Flags value : %d", evt.rsi_ble_write.att_value[0]);
    ChipLogProgress(DeviceLayer, "CHIPoBLE %s received", isIndicationEnabled ? "subscribe" : "unsubscribe");

    if (isIndicationEnabled)
    {
        // If indications are not already enabled for the connection...
        if (!bleConnState->subscribed)
        {
            bleConnState->subscribed = 1;
            // Post an event to the CHIP queue to process either a CHIPoBLE Subscribe or Unsubscribe based on
            // whether the client is enabling or disabling indications.
            event.Type                    = DeviceEventType::kCHIPoBLESubscribe;
            event.CHIPoBLESubscribe.ConId = 1; // TODO:: To be replaced by device mac address
            err                           = PlatformMgr().PostEvent(&event);
        }
    }
    else
    {
        bleConnState->subscribed      = 0;
        event.Type                    = DeviceEventType::kCHIPoBLEUnsubscribe;
        event.CHIPoBLESubscribe.ConId = 1; // TODO:: To be replaced by device mac address
        err                           = PlatformMgr().PostEvent(&event);
    }
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleTXCharCCCDWrite() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleRXCharWrite(const SilabsBleWrapper::sl_wfx_msg_t & evt)
{
    uint8_t conId  = 1;
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;
    uint16_t writeLen = evt.rsi_ble_write.length;
    uint8_t * data    = (uint8_t *) evt.rsi_ble_write.att_value;

    // Copy the data to a packet buffer.
    buf = System::PacketBufferHandle::NewWithData(data, writeLen, 0, 0);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);

    ChipLogDetail(DeviceLayer, "Write request/command received for CHIPoBLE RX characteristic ( len %d)", writeLen);

    // Post an event to the CHIP queue to deliver the data into the CHIP stack.
    {
        ChipDeviceEvent event;
        event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
        event.CHIPoBLEWriteReceived.ConId = conId;
        event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
        err                               = PlatformMgr().PostEvent(&event);
    }

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharWrite() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleTxConfirmationEvent(BLE_CONNECTION_OBJECT conId)
{
    DeviceLayer::PlatformMgr().LockChipStack();
    // stop the indication confirmation timer
    DeviceLayer::SystemLayer().CancelTimer(OnSendIndicationTimeout, this);
    DeviceLayer::PlatformMgr().UnlockChipStack();
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = conId;
    PlatformMgr().PostEventOrDie(&event);
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

void BLEManagerImpl::AddConnection(uint8_t connectionHandle, uint8_t bondingHandle)
{
    CHIPoBLEConState * bleConnState = GetConnectionState(connectionHandle, true);

    if (bleConnState != NULL)
    {
        memset(bleConnState, 0, sizeof(CHIPoBLEConState));
        bleConnState->allocated        = 1;
        bleConnState->connectionHandle = connectionHandle;
        bleConnState->bondingHandle    = bondingHandle;
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

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
CHIP_ERROR BLEManagerImpl::EncodeAdditionalDataTlv()
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    BitFlags<AdditionalDataFields> additionalDataFields;
    AdditionalDataPayloadGeneratorParams additionalDataPayloadParams;

#if CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID)
    uint8_t rotatingDeviceIdUniqueId[ConfigurationManager::kRotatingDeviceIDUniqueIDLength] = {};
    MutableByteSpan rotatingDeviceIdUniqueIdSpan(rotatingDeviceIdUniqueId);

    err = DeviceLayer::GetDeviceInstanceInfoProvider()->GetRotatingDeviceIdUniqueId(rotatingDeviceIdUniqueIdSpan);
    SuccessOrExit(err);
    err = ConfigurationMgr().GetLifetimeCounter(additionalDataPayloadParams.rotatingDeviceIdLifetimeCounter);
    SuccessOrExit(err);
    additionalDataPayloadParams.rotatingDeviceIdUniqueId = rotatingDeviceIdUniqueIdSpan;
    additionalDataFields.Set(AdditionalDataFields::RotatingDeviceId);
#endif /* CHIP_ENABLE_ROTATING_DEVICE_ID && defined(CHIP_DEVICE_CONFIG_ROTATING_DEVICE_ID_UNIQUE_ID) */

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(additionalDataPayloadParams, c3AdditionalDataBufferHandle,
                                                                         additionalDataFields);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to generate TLV encoded Additional Data (%s)", __func__);
    }

    return err;
}

void BLEManagerImpl::HandleC3ReadRequest(const SilabsBleWrapper::sl_wfx_msg_t & evt)
{
    sl_status_t ret = rsi_ble_gatt_read_response(evt.rsi_ble_read_req->dev_addr, GATT_READ_RESP, evt.rsi_ble_read_req->handle,
                                                 GATT_READ_ZERO_OFFSET, sInstance.c3AdditionalDataBufferHandle->DataLength(),
                                                 sInstance.c3AdditionalDataBufferHandle->Start());
    if (ret != SL_STATUS_OK)
    {
        ChipLogDetail(DeviceLayer, "Failed to send read response, err:%ld", ret);
    }
}

#endif // CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING

uint8_t BLEManagerImpl::GetTimerHandle(uint8_t connectionHandle, bool allocate)
{
    uint8_t freeIndex = kMaxConnections;

    for (uint8_t i = 0; i < kMaxConnections; i++)
    {
        if (mIndConfId[i] == connectionHandle)
        {
            return i;
        }
        else if (allocate)
        {
            if (i < freeIndex)
            {
                freeIndex = i;
            }
        }
    }

    if (freeIndex < kMaxConnections)
    {
        mIndConfId[freeIndex] = connectionHandle;
    }
    else
    {
        ChipLogError(DeviceLayer, "Failed to Save Conn Handle for indication");
    }

    return freeIndex;
}

void BLEManagerImpl::BleAdvTimeoutHandler(void * arg)
{
    if (BLEMgrImpl().mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        ChipLogDetail(DeviceLayer, "bleAdv Timeout : Start slow advertisement");
        BLEMgr().SetAdvertisingMode(BLEAdvertisingMode::kSlowAdvertising);
    }
}

void BLEManagerImpl::CancelBleAdvTimeoutTimer(void)
{
    if (osTimerStop(sbleAdvTimeoutTimer) != osOK)
    {
        ChipLogError(DeviceLayer, "Failed to stop BledAdv timeout timer");
    }
}

void BLEManagerImpl::StartBleAdvTimeoutTimer(uint32_t aTimeoutInMs)
{
    if (osTimerStart(sbleAdvTimeoutTimer, pdMS_TO_TICKS(aTimeoutInMs)) != osOK)
    {
        ChipLogError(DeviceLayer, "Failed to start BledAdv timeout timer");
    }
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
