/*
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Nest Labs, Inc.
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
 *          for Tizen platforms.
 */
#include <platform/internal/CHIPDeviceLayerInternal.h>

#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include "MainLoop.h"
#include <bluetooth.h>

using namespace ::chip;
using namespace ::chip::Ble;

namespace chip {
namespace DeviceLayer {
namespace Internal {

BLEManagerImpl BLEManagerImpl::sInstance;

struct BLEConnection
{
    char * peerAddr;
    uint16_t mtu;
    bool subscribed;
    bt_gatt_h gattCharC1Handle;
    bt_gatt_h gattCharC2Handle;
    bool isChipDevice;
};

/* CHIPoBLE UUID strings */
const char * chip_ble_service_uuid    = "0000FFF6-0000-1000-8000-00805F9B34FB";
const char * chip_ble_char_c1_tx_uuid = "18EE2EF5-263D-4559-959F-4F9C429F9D11";
const char * chip_ble_char_c2_rx_uuid = "18EE2EF5-263D-4559-959F-4F9C429F9D12";
/* CCCD */
const char * desc_uuid_short = "2902";

const char * chip_ble_service_uuid_short = "FFF6";

/* Tizen Default Scan Timeout */
static constexpr unsigned kNewConnectionScanTimeoutMs = 10000;

/* Tizen Default Connect Timeout */
constexpr System::Clock::Timeout kConnectTimeoutMs = System::Clock::Seconds16(10);

static void __AdapterStateChangedCb(int result, bt_adapter_state_e adapterState, void * userData)
{
    ChipLogProgress(DeviceLayer, "Adapter State Changed: %s", adapterState == BT_ADAPTER_ENABLED ? "Enabled" : "Disabled");
}

void BLEManagerImpl::GattConnectionStateChangedCb(int result, bool connected, const char * remoteAddress, void * userData)
{
    ChipLogProgress(DeviceLayer, "Gatt Connection State Changed: %s result [%d]", connected ? "Connected" : "Disconnected", result);

    if (result != BT_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "%s", connected ? "Connection req failed" : "Disconnection req failed");
        if (connected)
            sInstance.NotifyHandleConnectFailed(CHIP_ERROR_INTERNAL);
    }
    else
    {
        sInstance.HandleConnectionEvent(connected, remoteAddress);
    }
}

gboolean BLEManagerImpl::_BleInitialize(void * userData)
{
    int ret;

    if (sInstance.mFlags.Has(Flags::kTizenBLELayerInitialized))
    {
        ChipLogProgress(DeviceLayer, "BLE Already Initialized");
        return true;
    }

    ret = bt_initialize();
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_initialize() failed. ret: %d", ret));

    ret = bt_adapter_set_state_changed_cb(__AdapterStateChangedCb, nullptr);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_adapter_set_state_changed_cb() failed. ret: %d", ret));

    ret = bt_gatt_server_initialize();
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_server_initialize() failed. ret: %d", ret));

    ret = bt_gatt_set_connection_state_changed_cb(GattConnectionStateChangedCb, nullptr);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_adapter_set_state_changed_cb() failed. ret: %d", ret));

    sInstance.InitConnectionData();

    sInstance.mFlags.Set(Flags::kTizenBLELayerInitialized);
    ChipLogProgress(DeviceLayer, "BLE Initialized");
    sInstance.mMainContext = g_main_context_get_thread_default();
    return true;

exit:
    return false;
}

static int __GetAttInfo(bt_gatt_h gattHandle, char ** uuid, bt_gatt_type_e * type)
{
    int ret = bt_gatt_get_type(gattHandle, type);
    VerifyOrReturnError(ret == BT_ERROR_NONE, ret);
    return bt_gatt_get_uuid(gattHandle, uuid);
}

static constexpr const char * __ConvertAttTypeToStr(bt_gatt_type_e type)
{
    switch (type)
    {
    case BT_GATT_TYPE_SERVICE:
        return "Service";
    case BT_GATT_TYPE_CHARACTERISTIC:
        return "Characteristic";
    case BT_GATT_TYPE_DESCRIPTOR:
        return "Descriptor";
    default:
        return "(unknown)";
    }
}

static void __ReadValueRequestedCb(const char * remoteAddress, int requestId, bt_gatt_server_h server, bt_gatt_h gattHandle,
                                   int offset, void * userData)
{
    int ret, len = 0;
    bt_gatt_type_e type;
    char * uuid  = nullptr;
    char * value = nullptr;

    VerifyOrReturn(__GetAttInfo(gattHandle, &uuid, &type) == BT_ERROR_NONE,
                   ChipLogError(DeviceLayer, "Failed to fetch GATT Attribute from GATT handle"));
    ChipLogProgress(DeviceLayer, "Read Requested on %s: %s", __ConvertAttTypeToStr(type), uuid);
    g_free(uuid);

    ret = bt_gatt_get_value(gattHandle, &value, &len);
    VerifyOrReturn(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_get_value() failed. ret: %d", ret));

    ChipLogProgress(DeviceLayer, "Read Value (len: %d): %.*s", len, len, value);

    ret = bt_gatt_server_send_response(requestId, BT_GATT_REQUEST_TYPE_READ, offset, 0x00, value, len);
    g_free(value);
    VerifyOrReturn(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_server_send_response() failed. ret: %d", ret));
}

void BLEManagerImpl::WriteValueRequestedCb(const char * remoteAddress, int requestId, bt_gatt_server_h server, bt_gatt_h gattHandle,
                                           bool responseNeeded, int offset, const char * value, int len, void * userData)
{
    int ret;
    char * uuid          = nullptr;
    BLEConnection * conn = nullptr;
    bt_gatt_type_e type;

    conn = static_cast<BLEConnection *>(g_hash_table_lookup(sInstance.mConnectionMap, remoteAddress));
    VerifyOrReturn(conn != nullptr, ChipLogError(DeviceLayer, "Failed to find connection info"));

    VerifyOrReturn(__GetAttInfo(gattHandle, &uuid, &type) == BT_ERROR_NONE,
                   ChipLogError(DeviceLayer, "Failed to fetch GATT Attribute from GATT handle"));

    ChipLogProgress(DeviceLayer, "Write Requested on %s: %s", __ConvertAttTypeToStr(type), uuid);
    ChipLogProgress(DeviceLayer, "Write Value (len: %d): %.*s ", len, len, value);
    g_free(uuid);

    ret = bt_gatt_set_value(gattHandle, value, len);
    VerifyOrReturn(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_set_value() failed. ret: %d", ret));

    ret = bt_gatt_server_send_response(requestId, BT_GATT_REQUEST_TYPE_WRITE, offset, 0x00, nullptr, 0);
    VerifyOrReturn(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_server_send_response() failed. ret: %d", ret));

    sInstance.HandleC1CharWriteEvent(conn, reinterpret_cast<const uint8_t *>(value), len);
}

void BLEManagerImpl::NotificationStateChangedCb(bool notify, bt_gatt_server_h server, bt_gatt_h gattHandle, void * userData)
{
    char * uuid          = nullptr;
    BLEConnection * conn = nullptr;
    bt_gatt_type_e type;
    GHashTableIter iter;
    gpointer key, value;

    g_hash_table_iter_init(&iter, sInstance.mConnectionMap);
    while (g_hash_table_iter_next(&iter, &key, &value))
    {
        /* NOTE: Currently Tizen Platform API does not return remote device address, which enables/disables
         * notification/Indication. Therefore, returning first Connection */
        conn = static_cast<BLEConnection *>(value);
        break;
    }

    VerifyOrReturn(conn != nullptr, ChipLogError(DeviceLayer, "Failed to find connection info"));
    VerifyOrReturn(__GetAttInfo(gattHandle, &uuid, &type) == BT_ERROR_NONE,
                   ChipLogError(DeviceLayer, "Failed to fetch GATT Attribute from GATT handle"));

    ChipLogProgress(DeviceLayer, "Notification State Changed %d on %s: %s", notify, __ConvertAttTypeToStr(type), uuid);
    g_free(uuid);
    sInstance.NotifyBLESubscribed(notify ? true : false, conn);
}

void BLEManagerImpl::WriteCompletedCb(int result, bt_gatt_h gattHandle, void * userData)
{
    auto conn = static_cast<BLEConnection *>(userData);

    VerifyOrReturn(result == BT_ERROR_NONE, ChipLogError(DeviceLayer, "Failed to Send Write request"));
    VerifyOrReturn(conn != nullptr, ChipLogError(DeviceLayer, "Connection object is invalid"));
    VerifyOrReturn(conn->gattCharC1Handle == gattHandle, ChipLogError(DeviceLayer, "Gatt characteristic handle did not match"));

    ChipLogProgress(DeviceLayer, "Write Completed to CHIP peripheral [%s]", conn->peerAddr);
    sInstance.NotifyHandleWriteComplete(conn);
}

void BLEManagerImpl::CharacteristicNotificationCb(bt_gatt_h characteristic, char * value, int len, void * userData)
{
    auto conn = static_cast<BLEConnection *>(userData);

    VerifyOrReturn(value != nullptr);
    VerifyOrReturn(conn != nullptr, ChipLogError(DeviceLayer, "Connection object is invalid"));
    VerifyOrReturn(conn->gattCharC2Handle == characteristic, ChipLogError(DeviceLayer, "Gatt characteristic handle did not match"));

    ChipLogProgress(DeviceLayer, "Notification Received from CHIP peripheral [%s]", conn->peerAddr);
    sInstance.HandleRXCharChanged(conn, reinterpret_cast<const uint8_t *>(value), len);
}

void BLEManagerImpl::IndicationConfirmationCb(int result, const char * remoteAddress, bt_gatt_server_h server,
                                              bt_gatt_h characteristic, bool completed, void * userData)
{
    BLEConnection * conn = nullptr;
    VerifyOrReturn(result == BT_ERROR_NONE, ChipLogError(DeviceLayer, "Failed to Get Indication Confirmation"));

    conn = static_cast<BLEConnection *>(g_hash_table_lookup(sInstance.mConnectionMap, remoteAddress));

    VerifyOrReturn(conn != nullptr, ChipLogError(DeviceLayer, "Could not find connection for [%s]", remoteAddress));
    VerifyOrReturn(sInstance.mGattCharC2Handle == characteristic,
                   ChipLogError(DeviceLayer, "Gatt characteristic handle did not match"));

    sInstance.NotifyBLEIndicationConfirmation(conn);
}

void BLEManagerImpl::AdvertisingStateChangedCb(int result, bt_advertiser_h advertiser, bt_adapter_le_advertising_state_e advState,
                                               void * userData)
{
    ChipLogProgress(DeviceLayer, "Advertising %s", advState == BT_ADAPTER_LE_ADVERTISING_STARTED ? "Started" : "Stopped");

    if (advState == BT_ADAPTER_LE_ADVERTISING_STARTED)
    {
        sInstance.mFlags.Set(Flags::kAdvertising);
        sInstance.NotifyBLEPeripheralAdvStartComplete(true, nullptr);
    }
    else
    {
        sInstance.mFlags.Clear(Flags::kAdvertising);
        sInstance.NotifyBLEPeripheralAdvStopComplete(true, nullptr);
    }

    if (sInstance.mFlags.Has(Flags::kAdvertisingRefreshNeeded))
    {
        sInstance.mFlags.Clear(Flags::kAdvertisingRefreshNeeded);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

    sInstance.mAdvReqInProgress = false;
}

// ====== Private Functions.
void BLEManagerImpl::NotifyBLEPeripheralGATTServerRegisterComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type = DeviceEventType::kPlatformTizenBLEPeripheralGATTServerRegisterComplete;
    event.Platform.BLEPeripheralGATTServerRegisterComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralGATTServerRegisterComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvConfiguredComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type = DeviceEventType::kPlatformTizenBLEPeripheralAdvConfiguredComplete;
    event.Platform.BLEPeripheralAdvConfiguredComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvConfiguredComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                              = DeviceEventType::kPlatformTizenBLEPeripheralAdvStartComplete;
    event.Platform.BLEPeripheralAdvStartComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvStartComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(bool aIsSuccess, void * apAppstate)
{
    ChipDeviceEvent event;
    event.Type                                             = DeviceEventType::kPlatformTizenBLEPeripheralAdvStopComplete;
    event.Platform.BLEPeripheralAdvStopComplete.mIsSuccess = aIsSuccess;
    event.Platform.BLEPeripheralAdvStopComplete.mpAppstate = apAppstate;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEWriteReceived(System::PacketBufferHandle & buf, BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event;
    event.Type                        = DeviceEventType::kCHIPoBLEWriteReceived;
    event.CHIPoBLEWriteReceived.ConId = conId;
    event.CHIPoBLEWriteReceived.Data  = std::move(buf).UnsafeRelease();
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLENotificationReceived(System::PacketBufferHandle & buf, BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event;
    event.Type                                       = DeviceEventType::kPlatformTizenBLEIndicationReceived;
    event.Platform.BLEIndicationReceived.mConnection = conId;
    event.Platform.BLEIndicationReceived.mData       = std::move(buf).UnsafeRelease();
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLESubscribed(bool indicationsEnabled, BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event;
    event.Type = (indicationsEnabled) ? DeviceEventType::kCHIPoBLESubscribe : DeviceEventType::kCHIPoBLEUnsubscribe;
    event.CHIPoBLESubscribe.ConId = conId;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEIndicationConfirmation(BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event;
    event.Type                          = DeviceEventType::kCHIPoBLEIndicateConfirm;
    event.CHIPoBLEIndicateConfirm.ConId = conId;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyBLEConnectionEstablished(BLE_CONNECTION_OBJECT conId, CHIP_ERROR error)
{
    ChipDeviceEvent connectionEvent;
    connectionEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
    PlatformMgr().PostEventOrDie(&connectionEvent);
}

void BLEManagerImpl::NotifyBLEDisconnection(BLE_CONNECTION_OBJECT conId, CHIP_ERROR error)
{
    ChipDeviceEvent event;
    event.Type                           = DeviceEventType::kCHIPoBLEConnectionError;
    event.CHIPoBLEConnectionError.ConId  = conId;
    event.CHIPoBLEConnectionError.Reason = error;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifyHandleConnectFailed(CHIP_ERROR error)
{
    if (sInstance.mIsCentral)
    {
        ChipLogProgress(DeviceLayer, "Connection Failed: Post Platform event");
        ChipDeviceEvent event;
        event.Type                                    = DeviceEventType::kPlatformTizenBLECentralConnectFailed;
        event.Platform.BLECentralConnectFailed.mError = error;
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::NotifyHandleNewConnection(BLE_CONNECTION_OBJECT conId)
{
    if (sInstance.mIsCentral)
    {
        ChipDeviceEvent event;
        event.Type                                     = DeviceEventType::kPlatformTizenBLECentralConnected;
        event.Platform.BLECentralConnected.mConnection = conId;
        PlatformMgr().PostEventOrDie(&event);
    }
}

void BLEManagerImpl::NotifyHandleWriteComplete(BLE_CONNECTION_OBJECT conId)
{
    ChipDeviceEvent event;
    event.Type                                  = DeviceEventType::kPlatformTizenBLEWriteComplete;
    event.Platform.BLEWriteComplete.mConnection = conId;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::NotifySubscribeOpComplete(BLE_CONNECTION_OBJECT conId, bool isSubscribed)
{
    ChipDeviceEvent event;
    event.Type                                          = DeviceEventType::kPlatformTizenBLESubscribeOpComplete;
    event.Platform.BLESubscribeOpComplete.mConnection   = conId;
    event.Platform.BLESubscribeOpComplete.mIsSubscribed = isSubscribed;
    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleConnectionTimeout(chip::System::Layer * layer, void * data)
{
    sInstance.NotifyHandleConnectFailed(CHIP_ERROR_TIMEOUT);
}

gboolean BLEManagerImpl::ConnectChipThing(gpointer userData)
{
    int ret = BT_ERROR_NONE;

    char * address = reinterpret_cast<char *>(userData);
    ChipLogProgress(DeviceLayer, "ConnectRequest: Addr [%s]", address);

    ret = bt_gatt_client_create(address, &sInstance.mGattClient);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "Failed to create GATT client. ret [%d]", ret));

    ret = bt_gatt_connect(address, false);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "Failed to issue GATT connect request. ret [%d]", ret));

    ChipLogProgress(DeviceLayer, "GATT Connect Issued");
exit:
    if (ret != BT_ERROR_NONE)
        sInstance.NotifyHandleConnectFailed(CHIP_ERROR_INTERNAL);

    g_free(address);
    return G_SOURCE_REMOVE;
}

void BLEManagerImpl::ConnectHandler(const char * address)
{
    GSource * idleSource;

    idleSource = g_idle_source_new();
    g_source_set_callback(idleSource, ConnectChipThing, g_strdup(address), nullptr);
    g_source_set_priority(idleSource, G_PRIORITY_HIGH_IDLE);
    g_source_attach(idleSource, sInstance.mMainContext);
    g_source_unref(idleSource);
}

void BLEManagerImpl::OnChipDeviceScanned(void * device, const chip::Ble::ChipBLEDeviceIdentificationInfo & info)
{
    auto deviceInfo = reinterpret_cast<bt_adapter_le_device_scan_result_info_s *>(device);
    VerifyOrReturn(deviceInfo != nullptr, ChipLogError(DeviceLayer, "Invalid Device Info"));

    ChipLogProgress(DeviceLayer, "New device scanned: %s", deviceInfo->remote_address);

    if (mBLEScanConfig.mBleScanState == BleScanState::kScanForDiscriminator)
    {
        if (!mBLEScanConfig.mDiscriminator.MatchesLongDiscriminator(info.GetDeviceDiscriminator()))
        {
            return;
        }
        ChipLogProgress(DeviceLayer, "Device discriminator match. Attempting to connect.");
    }
    else if (mBLEScanConfig.mBleScanState == BleScanState::kScanForAddress)
    {
        if (strcmp(deviceInfo->remote_address, mBLEScanConfig.mAddress.c_str()) != 0)
        {
            return;
        }
        ChipLogProgress(DeviceLayer, "Device address match. Attempting to connect.");
    }
    else
    {
        ChipLogError(DeviceLayer, "Unknown discovery type. Ignoring scanned device.");
        return;
    }

    /* Set CHIP Connecting state */
    mBLEScanConfig.mBleScanState = BleScanState::kConnecting;
    chip::DeviceLayer::SystemLayer().StartTimer(kConnectTimeoutMs, HandleConnectionTimeout, nullptr);
    mDeviceScanner->StopChipScan();

    /* Initiate Connect */
    ConnectHandler(deviceInfo->remote_address);
}

void BLEManagerImpl::OnChipScanComplete(void)
{
    if (mBLEScanConfig.mBleScanState != BleScanState::kScanForDiscriminator &&
        mBLEScanConfig.mBleScanState != BleScanState::kScanForAddress)
    {
        ChipLogProgress(DeviceLayer, "Scan complete notification without an active scan.");
        return;
    }

    ChipLogError(DeviceLayer, "Scan Completed with Timeout: Notify Upstream.");
    BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, CHIP_ERROR_TIMEOUT);
    mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
}

int BLEManagerImpl::RegisterGATTServer()
{
    int ret = BT_ERROR_NONE;

    bt_gatt_server_h server = nullptr;
    bt_gatt_h service       = nullptr;
    bt_gatt_h char1 = nullptr, char2 = nullptr;
    bt_gatt_h desc     = nullptr;
    char desc_value[2] = { 0, 0 };

    ChipLogProgress(DeviceLayer, "Start GATT Service Registration");

    // Create Server
    ret = bt_gatt_server_create(&server);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_server_create() failed. ret: %d", ret));

    // Create Service (BTP Service)
    ret = bt_gatt_service_create(chip_ble_service_uuid, BT_GATT_SERVICE_TYPE_PRIMARY, &service);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_service_create() failed. ret: %d", ret));

    // Create 1st Characteristic (Client TX Buffer)
    ret = bt_gatt_characteristic_create(
        chip_ble_char_c1_tx_uuid, BT_GATT_PERMISSION_WRITE,
        BT_GATT_PROPERTY_WRITE, // Write Request is not coming if we use WITHOUT_RESPONSE property. Let's use WRITE property and
                                // consider to use WITHOUT_RESPONSE property in the future according to the CHIP Spec 4.16.3.2. BTP
                                // GATT Service
        "CHIPoBLE_C1", strlen("CHIPoBLE_C1"), &char1);

    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_characteristic_create() failed. ret: %d", ret));
    ret = bt_gatt_server_set_write_value_requested_cb(char1, WriteValueRequestedCb, nullptr);
    VerifyOrExit(ret == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "bt_gatt_server_set_write_value_requested_cb() failed. ret: %d", ret));
    ret = bt_gatt_service_add_characteristic(service, char1);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_service_add_characteristic() failed. ret: %d", ret));

    // Create 2nd Characteristic (Client RX Buffer)
    ret = bt_gatt_characteristic_create(chip_ble_char_c2_rx_uuid, BT_GATT_PERMISSION_READ,
                                        BT_GATT_PROPERTY_READ | BT_GATT_PROPERTY_INDICATE, "CHIPoBLE_C2", strlen("CHIPoBLE_C2"),
                                        &char2);

    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_characteristic_create() failed. ret: %d", ret));
    ret = bt_gatt_server_set_read_value_requested_cb(char2, __ReadValueRequestedCb, nullptr);
    VerifyOrExit(ret == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "bt_gatt_server_set_read_value_requested_cb() failed. ret: %d", ret));
    ret = bt_gatt_server_set_characteristic_notification_state_change_cb(char2, NotificationStateChangedCb, nullptr);
    VerifyOrExit(
        ret == BT_ERROR_NONE,
        ChipLogError(DeviceLayer, "bt_gatt_server_set_characteristic_notification_state_change_cb() failed. ret: %d", ret));

    // Create CCC Descriptor
    ret = bt_gatt_descriptor_create(desc_uuid_short, BT_GATT_PERMISSION_READ | BT_GATT_PERMISSION_WRITE, desc_value,
                                    sizeof(desc_value), &desc);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_descriptor_create() failed. ret: %d", ret));
    ret = bt_gatt_characteristic_add_descriptor(char2, desc);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_characteristic_add_descriptor() failed. ret: %d", ret));
    ret = bt_gatt_service_add_characteristic(service, char2);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_service_add_characteristic() failed. ret: %d", ret));

    // Register Service to Server
    ret = bt_gatt_server_register_service(server, service);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_server_register_service() failed. ret: %d", ret));

    // Start Server
    ret = bt_gatt_server_start();
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_server_start() failed. ret: %d", ret));

    ChipLogDetail(DeviceLayer, "NotifyBLEPeripheralGATTServerRegisterComplete Success");
    BLEManagerImpl::NotifyBLEPeripheralGATTServerRegisterComplete(true, nullptr);

    // Save the Local Peripheral char1 & char2 handles
    mGattCharC1Handle = char1;
    mGattCharC2Handle = char2;

    return ret;

exit:
    ChipLogDetail(DeviceLayer, "NotifyBLEPeripheralGATTServerRegisterComplete Failed");
    BLEManagerImpl::NotifyBLEPeripheralGATTServerRegisterComplete(false, nullptr);
    return ret;
}

int BLEManagerImpl::StartAdvertising()
{
    int ret                                                    = BT_ERROR_NONE;
    CHIP_ERROR err                                             = CHIP_NO_ERROR;
    char service_data[sizeof(ChipBLEDeviceIdentificationInfo)] = {
        0x0,
    }; // need to fill advertising data. 5.2.3.8.6. Advertising Data, CHIP Specification
    ChipBLEDeviceIdentificationInfo deviceIdInfo = {
        0x0,
    };

    if (sInstance.mAdvReqInProgress)
    {
        ChipLogProgress(DeviceLayer, "Advertising Request In Progress");
        return ret;
    }

    ChipLogProgress(DeviceLayer, "Start Advertising");

    if (mAdvertiser == nullptr)
    {
        ret = bt_adapter_le_create_advertiser(&mAdvertiser);
        VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_adapter_le_create_advertiser() failed. ret: %d", ret));
    }
    else
    {
        ret = bt_adapter_le_clear_advertising_data(mAdvertiser, BT_ADAPTER_LE_PACKET_ADVERTISING);
        VerifyOrExit(ret == BT_ERROR_NONE,
                     ChipLogError(DeviceLayer, "bt_adapter_le_clear_advertising_data() failed. ret: %d", ret));

        ret = bt_adapter_le_clear_advertising_data(mAdvertiser, BT_ADAPTER_LE_PACKET_SCAN_RESPONSE);
        VerifyOrExit(ret == BT_ERROR_NONE,
                     ChipLogError(DeviceLayer, "bt_adapter_le_clear_advertising_data() failed. ret: %d", ret));
    }

    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        ret = bt_adapter_le_set_advertising_mode(mAdvertiser, BT_ADAPTER_LE_ADVERTISING_MODE_LOW_LATENCY);
        VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_adapter_le_set_advertising_mode() failed. ret: %d", ret));

        // NOTE: Check specification for recommended Advertising Interval range for Fast Advertising
        // ret = bt_adapter_le_set_advertising_interval(mAdvertiser, BT_LE_ADV_INTERVAL_MIN, BT_LE_ADV_INTERVAL_MIN);
    }
    else
    {
        // NOTE: Check specification for recommended Advertising Interval range for Slow Advertising
        // ret = bt_adapter_le_set_advertising_interval(mAdvertiser, BT_LE_ADV_INTERVAL_MAX, BT_LE_ADV_INTERVAL_MAX);
    }

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    VerifyOrExit(err == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "GetBLEDeviceIdentificationInfo() failed. %s", ErrorStr(err)));

    memcpy(service_data, &deviceIdInfo, sizeof(service_data));
    ret = bt_adapter_le_add_advertising_service_data(mAdvertiser, BT_ADAPTER_LE_PACKET_ADVERTISING, chip_ble_service_uuid_short,
                                                     service_data, sizeof(service_data));
    VerifyOrExit(ret == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "bt_adapter_le_add_advertising_service_data() failed. ret: %d", ret));

    BLEManagerImpl::NotifyBLEPeripheralAdvConfiguredComplete(true, nullptr);

    ret = bt_adapter_le_start_advertising_new(mAdvertiser, AdvertisingStateChangedCb, nullptr);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_adapter_le_start_advertising_new() failed. ret: %d", ret));
    sInstance.mAdvReqInProgress = true;
    return ret;

exit:
    BLEManagerImpl::NotifyBLEPeripheralAdvStartComplete(false, nullptr);
    return ret;
}

int BLEManagerImpl::StopAdvertising()
{
    int ret = BT_ERROR_NONE;

    ChipLogProgress(DeviceLayer, "Stop Advertising");
    ret = bt_adapter_le_stop_advertising(mAdvertiser);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_adapter_le_stop_advertising() failed. ret: %d", ret));
    sInstance.mAdvReqInProgress = true;
    return ret;

exit:
    BLEManagerImpl::NotifyBLEPeripheralAdvStopComplete(false, nullptr);
    return ret;
}

void BLEManagerImpl::InitConnectionData(void)
{
    /* Initialize Hashmap */
    if (!mConnectionMap)
    {
        mConnectionMap = g_hash_table_new(g_str_hash, g_str_equal);
        ChipLogProgress(DeviceLayer, "GATT Connection HashMap created");
    }
}

static bool __GattClientForeachCharCb(int total, int index, bt_gatt_h charHandle, void * data)
{
    bt_gatt_type_e type;
    char * uuid = nullptr;
    auto conn   = static_cast<BLEConnection *>(data);

    VerifyOrExit(__GetAttInfo(charHandle, &uuid, &type) == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "Failed to fetch GATT Attribute from CHAR handle"));

    if (strcasecmp(uuid, chip_ble_char_c1_tx_uuid) == 0)
    {
        ChipLogProgress(DeviceLayer, "CHIP Char C1 TX Found [%s]", uuid);
        conn->gattCharC1Handle = charHandle;
    }
    else if (strcasecmp(uuid, chip_ble_char_c2_rx_uuid) == 0)
    {
        ChipLogProgress(DeviceLayer, "CHIP Char C2 RX Found [%s]", uuid);
        conn->gattCharC2Handle = charHandle;
    }
    g_free(uuid);

exit:
    /* Try next Char UUID */
    return true;
}

static bool __GattClientForeachServiceCb(int total, int index, bt_gatt_h svcHandle, void * data)
{
    bt_gatt_type_e type;
    char * uuid = nullptr;
    auto conn   = static_cast<BLEConnection *>(data);
    ChipLogProgress(DeviceLayer, "__GattClientForeachServiceCb");

    VerifyOrExit(__GetAttInfo(svcHandle, &uuid, &type) == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "Failed to fetch GATT Attribute from SVC handle"));

    if (strcasecmp(uuid, chip_ble_service_uuid) == 0)
    {
        ChipLogProgress(DeviceLayer, "CHIP Service UUID Found [%s]", uuid);

        if (bt_gatt_service_foreach_characteristics(svcHandle, __GattClientForeachCharCb, conn) == BT_ERROR_NONE)
            conn->isChipDevice = true;

        /* Got CHIP Device, no need to process further service */
        g_free(uuid);
        return false;
    }
    g_free(uuid);

exit:
    /* Try next Service UUID */
    return true;
}

bool BLEManagerImpl::IsDeviceChipPeripheral(BLE_CONNECTION_OBJECT conId)
{
    auto conn = static_cast<BLEConnection *>(conId);

    if (bt_gatt_client_foreach_services(sInstance.mGattClient, __GattClientForeachServiceCb, conn) != BT_ERROR_NONE)
        ChipLogError(DeviceLayer, "Error Browsing GATT services");

    return (conn->isChipDevice ? true : false);
}

void BLEManagerImpl::AddConnectionData(const char * remoteAddr)
{
    BLEConnection * conn;
    ChipLogProgress(DeviceLayer, "AddConnectionData for [%s]", remoteAddr);

    if (!g_hash_table_lookup(mConnectionMap, remoteAddr))
    {
        ChipLogProgress(DeviceLayer, "Connection not found in map [%s]", remoteAddr);
        conn           = static_cast<BLEConnection *>(g_malloc0(sizeof(BLEConnection)));
        conn->peerAddr = g_strdup(remoteAddr);

        if (sInstance.mIsCentral)
        {
            /* Local Device is BLE Central Role */
            if (IsDeviceChipPeripheral(conn))
            {
                g_hash_table_insert(mConnectionMap, (gpointer) conn->peerAddr, conn);
                ChipLogProgress(DeviceLayer, "New Connection Added for [%s]", remoteAddr);
                NotifyHandleNewConnection(conn);
            }
            else
            {
                g_free(conn->peerAddr);
                g_free(conn);
            }
        }
        else
        {
            /* Local Device is BLE Peripheral Role, assume remote is CHIP Central */
            conn->isChipDevice = true;

            /* Save own gatt handles */
            conn->gattCharC1Handle = mGattCharC1Handle;
            conn->gattCharC2Handle = mGattCharC2Handle;

            g_hash_table_insert(mConnectionMap, (gpointer) conn->peerAddr, conn);
            ChipLogProgress(DeviceLayer, "New Connection Added for [%s]", remoteAddr);
        }
    }
}

void BLEManagerImpl::RemoveConnectionData(const char * remoteAddr)
{
    BLEConnection * conn = nullptr;
    ChipLogProgress(DeviceLayer, "Connection Remove Request for [%s]", remoteAddr);

    VerifyOrReturn(mConnectionMap != nullptr, ChipLogError(DeviceLayer, "Connection map does not exist"));

    conn = static_cast<BLEConnection *>(g_hash_table_lookup(mConnectionMap, remoteAddr));
    VerifyOrReturn(conn != nullptr, ChipLogError(DeviceLayer, "Connection does not exist for [%s]", remoteAddr));

    g_hash_table_remove(mConnectionMap, conn->peerAddr);

    g_free(conn->peerAddr);
    g_free(conn);

    if (!g_hash_table_size(mConnectionMap))
        mConnectionMap = nullptr;

    ChipLogProgress(DeviceLayer, "Connection Removed");
}

void BLEManagerImpl::HandleC1CharWriteEvent(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;

    ChipLogProgress(DeviceLayer, "Write request received for CHIPoBLE Client TX characteristic (data len %u)",
                    static_cast<unsigned int>(len));
    // Copy the data to a packet buffer.
    buf = System::PacketBufferHandle::NewWithData(value, len);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    NotifyBLEWriteReceived(buf, conId);
    return;
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleC1CharWriteEvent() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleRXCharChanged(BLE_CONNECTION_OBJECT conId, const uint8_t * value, size_t len)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    System::PacketBufferHandle buf;

    ChipLogProgress(DeviceLayer, "Notification received on CHIPoBLE Client RX characteristic (data len %u)",
                    static_cast<unsigned int>(len));
    // Copy the data to a packet buffer.
    buf = System::PacketBufferHandle::NewWithData(value, len);
    VerifyOrExit(!buf.IsNull(), err = CHIP_ERROR_NO_MEMORY);
    NotifyBLENotificationReceived(buf, conId);
    return;
exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "HandleRXCharChanged() failed: %s", ErrorStr(err));
    }
}

void BLEManagerImpl::HandleConnectionEvent(bool connected, const char * remoteAddress)
{
    if (connected)
    {
        ChipLogProgress(DeviceLayer, "Device Connected [%s]", remoteAddress);
        AddConnectionData(remoteAddress);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Device DisConnected [%s]", remoteAddress);
        RemoveConnectionData(remoteAddress);
    }
}

void BLEManagerImpl::DriveBLEState()
{
    int ret = BT_ERROR_NONE;

    ChipLogProgress(DeviceLayer, "Enter DriveBLEState");

    if (!mIsCentral && mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && !mFlags.Has(Flags::kAppRegistered))
    {
        ret = RegisterGATTServer();
        VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "Register GATT Server Failed. ret: %d", ret));

        ChipLogProgress(DeviceLayer, "GATT Service Registered");
        mFlags.Set(Flags::kAppRegistered);
        ExitNow();
    }

    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled && mFlags.Has(Flags::kAdvertisingEnabled))
    {
        if (!mFlags.Has(Flags::kAdvertising))
        {
            ret = StartAdvertising();
            VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "Start Advertising Failed. ret: %d", ret));
        }
        else if (mFlags.Has(Flags::kAdvertisingRefreshNeeded))
        {
            ChipLogProgress(DeviceLayer, "Advertising Refreshed Needed. Stop Advertising");
            ret = StopAdvertising();
            VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "Stop Advertising Failed. ret: %d", ret));
        }
    }
    else if (mFlags.Has(Flags::kAdvertising))
    {
        ChipLogProgress(DeviceLayer, "Stop Advertising");

        ret = StopAdvertising();
        VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "Stop Advertising Failed. ret: %d", ret));

        ret = bt_adapter_le_destroy_advertiser(mAdvertiser);
        VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_adapter_le_destroy_advertiser() failed. ret: %d", ret));
        mAdvertiser = nullptr;
    }

exit:
    if (ret != BT_ERROR_NONE)
    {
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

CHIP_ERROR BLEManagerImpl::_Init(void)
{
    CHIP_ERROR err;
    bool ret;

    err = BleLayer::Init(this, this, this, &DeviceLayer::SystemLayer());
    SuccessOrExit(err);

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;

    ChipLogProgress(DeviceLayer, "Initialize BLE");
    ret = MainLoop::Instance().Init(_BleInitialize);
    VerifyOrExit(ret != false, err = CHIP_ERROR_INTERNAL);

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    mFlags.Set(Flags::kAdvertisingEnabled, val);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);
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
    CHIP_ERROR err = CHIP_NO_ERROR;
    int ret;
    char * deviceName = nullptr;

    VerifyOrExit(buf != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    ret = bt_adapter_get_name(&deviceName);
    if (ret != BT_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "bt_adapter_get_name() failed. ret: %d", ret);
        return CHIP_ERROR_INTERNAL;
    }

    VerifyOrExit(deviceName != nullptr, err = CHIP_ERROR_INTERNAL);
    VerifyOrExit(strlen(deviceName) >= bufSize, err = CHIP_ERROR_BUFFER_TOO_SMALL);

    g_strlcpy(buf, deviceName, bufSize);
    ChipLogProgress(DeviceLayer, "DeviceName: %s", buf);

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    CHIP_ERROR err = CHIP_NO_ERROR;
    int ret;

    VerifyOrExit(deviceName != nullptr, err = CHIP_ERROR_INVALID_ARGUMENT);

    ret = bt_adapter_set_name(deviceName);
    if (ret != BT_ERROR_NONE)
    {
        ChipLogError(DeviceLayer, "bt_adapter_set_name() failed. ret: %d", ret);
        return CHIP_ERROR_INTERNAL;
    }

exit:
    return err;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    return 0;
}

CHIP_ERROR BLEManagerImpl::ConfigureBle(uint32_t aAdapterId, bool aIsCentral)
{
    mAdapterId = aAdapterId;
    mIsCentral = aIsCentral;
    return CHIP_NO_ERROR;
}

void BLEManagerImpl::CleanScanConfig(void)
{
    if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        chip::DeviceLayer::SystemLayer().CancelTimer(HandleConnectionTimeout, nullptr);

    mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
}

void BLEManagerImpl::HandlePlatformSpecificBLEEvent(const ChipDeviceEvent * apEvent)
{
    ChipLogDetail(DeviceLayer, "HandlePlatformSpecificBLEEvent %d", apEvent->Type);

    switch (apEvent->Type)
    {
    case DeviceEventType::kPlatformTizenBLECentralConnected:
        if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        {
            BleConnectionDelegate::OnConnectionComplete(mBLEScanConfig.mAppState,
                                                        apEvent->Platform.BLECentralConnected.mConnection);
            CleanScanConfig();
        }
        break;
    case DeviceEventType::kPlatformTizenBLECentralConnectFailed:
        if (mBLEScanConfig.mBleScanState == BleScanState::kConnecting)
        {
            BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, apEvent->Platform.BLECentralConnectFailed.mError);
            CleanScanConfig();
        }
        break;
    case DeviceEventType::kPlatformTizenBLEWriteComplete: {
        ChipBleUUID service_uuid;
        ChipBleUUID char_write_uuid;

        StringToUUID(chip_ble_service_uuid, service_uuid);
        StringToUUID(chip_ble_char_c1_tx_uuid, char_write_uuid);

        HandleWriteConfirmation(apEvent->Platform.BLEWriteComplete.mConnection, &service_uuid, &char_write_uuid);
        break;
    }
    case DeviceEventType::kPlatformTizenBLESubscribeOpComplete: {
        ChipBleUUID service_uuid;
        ChipBleUUID char_notif_uuid;

        StringToUUID(chip_ble_service_uuid, service_uuid);
        StringToUUID(chip_ble_char_c2_rx_uuid, char_notif_uuid);

        if (apEvent->Platform.BLESubscribeOpComplete.mIsSubscribed)
            HandleSubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &service_uuid, &char_notif_uuid);
        else
            HandleUnsubscribeComplete(apEvent->Platform.BLESubscribeOpComplete.mConnection, &service_uuid, &char_notif_uuid);
        break;
    }
    case DeviceEventType::kPlatformTizenBLEIndicationReceived: {
        ChipBleUUID service_uuid;
        ChipBleUUID char_notif_uuid;

        StringToUUID(chip_ble_service_uuid, service_uuid);
        StringToUUID(chip_ble_char_c2_rx_uuid, char_notif_uuid);

        HandleIndicationReceived(apEvent->Platform.BLEIndicationReceived.mConnection, &service_uuid, &char_notif_uuid,
                                 PacketBufferHandle::Adopt(apEvent->Platform.BLEIndicationReceived.mData));
        break;
    }
    default:
        break;
    }
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    ChipBleUUID service_uuid;
    ChipBleUUID char_notification_uuid;
    ChipBleUUID char_write_uuid;

    switch (event->Type)
    {
    case DeviceEventType::kCHIPoBLESubscribe:
        ChipLogProgress(DeviceLayer, "CHIPoBLESubscribe");

        StringToUUID(chip_ble_service_uuid, service_uuid);
        StringToUUID(chip_ble_char_c2_rx_uuid, char_notification_uuid);

        HandleSubscribeReceived(event->CHIPoBLESubscribe.ConId, &service_uuid, &char_notification_uuid);
        NotifyBLEConnectionEstablished(event->CHIPoBLESubscribe.ConId, CHIP_NO_ERROR);
        break;
    case DeviceEventType::kCHIPoBLEUnsubscribe:
        ChipLogProgress(DeviceLayer, "CHIPoBLEUnsubscribe");

        StringToUUID(chip_ble_service_uuid, service_uuid);
        StringToUUID(chip_ble_char_c2_rx_uuid, char_notification_uuid);

        HandleUnsubscribeReceived(event->CHIPoBLESubscribe.ConId, &service_uuid, &char_notification_uuid);
        break;
    case DeviceEventType::kCHIPoBLEWriteReceived:
        ChipLogProgress(DeviceLayer, "CHIPoBLEWriteReceived");

        StringToUUID(chip_ble_service_uuid, service_uuid);
        StringToUUID(chip_ble_char_c1_tx_uuid, char_write_uuid);
        HandleWriteReceived(event->CHIPoBLEWriteReceived.ConId, &service_uuid, &char_write_uuid,
                            PacketBufferHandle::Adopt(event->CHIPoBLEWriteReceived.Data));
        break;
    case DeviceEventType::kCHIPoBLEIndicateConfirm:
        ChipLogProgress(DeviceLayer, "CHIPoBLEIndicateConfirm");

        StringToUUID(chip_ble_service_uuid, service_uuid);
        StringToUUID(chip_ble_char_c2_rx_uuid, char_notification_uuid);
        HandleIndicationConfirmation(event->CHIPoBLEIndicateConfirm.ConId, &service_uuid, &char_notification_uuid);
        break;
    case DeviceEventType::kCHIPoBLEConnectionError:
        ChipLogProgress(DeviceLayer, "CHIPoBLEConnectionError");

        HandleConnectionError(event->CHIPoBLEConnectionError.ConId, event->CHIPoBLEConnectionError.Reason);
        break;
    case DeviceEventType::kServiceProvisioningChange:
        break;
    default:
        HandlePlatformSpecificBLEEvent(event);
        break;
    }
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    auto conn = static_cast<BLEConnection *>(conId);
    return (conn != nullptr) ? conn->mtu : 0;
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipBleUUID service_uuid;
    ChipBleUUID char_notif_uuid;
    auto conn = static_cast<BLEConnection *>(conId);
    int ret   = BT_ERROR_NONE;

    ChipLogProgress(DeviceLayer, "SubscribeCharacteristic");

    StringToUUID(chip_ble_service_uuid, service_uuid);
    StringToUUID(chip_ble_char_c2_rx_uuid, char_notif_uuid);

    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "Invalid Connection"));
    VerifyOrExit(Ble::UUIDsMatch(svcId, &service_uuid),
                 ChipLogError(DeviceLayer, "SubscribeCharacteristic() called with invalid service ID"));
    VerifyOrExit(Ble::UUIDsMatch(charId, &char_notif_uuid),
                 ChipLogError(DeviceLayer, "SubscribeCharacteristic() called with invalid characteristic ID"));
    VerifyOrExit(conn->gattCharC2Handle != nullptr, ChipLogError(DeviceLayer, "Char C2 is null"));

    ChipLogProgress(DeviceLayer, "Sending Notification Enable Request to CHIP Peripheral(con %s)", conn->peerAddr);

    ret = bt_gatt_client_set_characteristic_value_changed_cb(conn->gattCharC2Handle, CharacteristicNotificationCb, conn);
    VerifyOrExit(ret == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "bt_gatt_client_set_characteristic_value_changed_cb() failed. ret: %d", ret));
    sInstance.NotifySubscribeOpComplete(conn, true);
    return true;
exit:
    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipBleUUID service_uuid;
    ChipBleUUID char_notif_uuid;
    auto conn = static_cast<BLEConnection *>(conId);
    int ret   = BT_ERROR_NONE;

    ChipLogProgress(DeviceLayer, "UnSubscribeCharacteristic");

    StringToUUID(chip_ble_service_uuid, service_uuid);
    StringToUUID(chip_ble_char_c2_rx_uuid, char_notif_uuid);

    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "Invalid Connection"));
    VerifyOrExit(Ble::UUIDsMatch(svcId, &service_uuid),
                 ChipLogError(DeviceLayer, "UnSubscribeCharacteristic() called with invalid service ID"));
    VerifyOrExit(Ble::UUIDsMatch(charId, &char_notif_uuid),
                 ChipLogError(DeviceLayer, "UnSubscribeCharacteristic() called with invalid characteristic ID"));
    VerifyOrExit(conn->gattCharC2Handle != nullptr, ChipLogError(DeviceLayer, "Char C2 is null"));

    ChipLogProgress(DeviceLayer, "Disable Notification Request to CHIP Peripheral(con %s)", conn->peerAddr);

    ret = bt_gatt_client_unset_characteristic_value_changed_cb(conn->gattCharC2Handle);
    VerifyOrExit(ret == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "bt_gatt_client_unset_characteristic_value_changed_cb() failed. ret: %d", ret));
    sInstance.NotifySubscribeOpComplete(conn, false);
    return true;
exit:
    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    auto conn = static_cast<BLEConnection *>(conId);
    int ret   = BT_ERROR_NONE;

    ChipLogProgress(DeviceLayer, "Close BLE Connection");

    conn = static_cast<BLEConnection *>(g_hash_table_lookup(sInstance.mConnectionMap, conn->peerAddr));
    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "Failed to find connection info"));

    ChipLogProgress(DeviceLayer, "Send GATT disconnect to [%s]", conn->peerAddr);
    ret = bt_gatt_disconnect(conn->peerAddr);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_disconnect() failed. ret: %d", ret));

    RemoveConnectionData(conn->peerAddr);
    return true;

exit:
    return false;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                    chip::System::PacketBufferHandle pBuf)
{
    auto conn = static_cast<BLEConnection *>(conId);
    int ret   = BT_ERROR_NONE;

    ChipLogProgress(DeviceLayer, "SendIndication");

    conn = static_cast<BLEConnection *>(g_hash_table_lookup(sInstance.mConnectionMap, conn->peerAddr));
    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "Failed to find connection info"));

    ret = bt_gatt_set_value(mGattCharC2Handle, reinterpret_cast<const char *>(pBuf->Start()), pBuf->DataLength());
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_set_value() failed. ret: %d", ret));

    ChipLogProgress(DeviceLayer, "Sending indication for CHIPoBLE RX characteristic (con %s, len %u)", conn->peerAddr,
                    pBuf->DataLength());

    ret = bt_gatt_server_notify_characteristic_changed_value(mGattCharC2Handle, IndicationConfirmationCb, conn->peerAddr, nullptr);
    VerifyOrExit(ret == BT_ERROR_NONE,
                 ChipLogError(DeviceLayer, "bt_gatt_server_notify_characteristic_changed_value() failed. ret: %d", ret));
    return true;

exit:
    return false;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                      chip::System::PacketBufferHandle pBuf)
{
    ChipBleUUID service_uuid;
    ChipBleUUID char_write_uuid;
    auto conn = static_cast<BLEConnection *>(conId);
    int ret   = BT_ERROR_NONE;

    ChipLogProgress(DeviceLayer, "SendWriteRequest");

    StringToUUID(chip_ble_service_uuid, service_uuid);
    StringToUUID(chip_ble_char_c1_tx_uuid, char_write_uuid);

    VerifyOrExit(conn != nullptr, ChipLogError(DeviceLayer, "Invalid Connection"));
    VerifyOrExit(Ble::UUIDsMatch(svcId, &service_uuid),
                 ChipLogError(DeviceLayer, "SendWriteRequest() called with invalid service ID"));
    VerifyOrExit(Ble::UUIDsMatch(charId, &char_write_uuid),
                 ChipLogError(DeviceLayer, "SendWriteRequest() called with invalid characteristic ID"));
    VerifyOrExit(conn->gattCharC1Handle != nullptr, ChipLogError(DeviceLayer, "Char C1 is null"));

    ret = bt_gatt_set_value(conn->gattCharC1Handle, reinterpret_cast<const char *>(pBuf->Start()), pBuf->DataLength());
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_set_value() failed. ret: %d", ret));

    ChipLogProgress(DeviceLayer, "Sending Write Request for CHIPoBLE TX characteristic (con %s, len %u)", conn->peerAddr,
                    pBuf->DataLength());

    ret = bt_gatt_client_write_value(conn->gattCharC1Handle, WriteCompletedCb, conn);
    VerifyOrExit(ret == BT_ERROR_NONE, ChipLogError(DeviceLayer, "bt_gatt_client_write_value() failed. ret: %d", ret));
    return true;
exit:
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId,
                                     chip::System::PacketBufferHandle pBuf)
{
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const Ble::ChipBleUUID * svcId, const Ble::ChipBleUUID * charId)
{
    return false;
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId) {}

void BLEManagerImpl::NewConnection(BleLayer * bleLayer, void * appState, const SetupDiscriminator & connDiscriminator)
{
    mBLEScanConfig.mDiscriminator = connDiscriminator;
    mBLEScanConfig.mAppState      = appState;
    if (connDiscriminator.IsShortDiscriminator())
    {
        ChipLogProgress(DeviceLayer, "NewConnection: short discriminator value [%u]", connDiscriminator.GetShortValue());
    }
    else
    {
        ChipLogProgress(DeviceLayer, "NewConnection: long discriminator value [%u]", connDiscriminator.GetLongValue());
    }

    // Initiate Scan.
    PlatformMgr().ScheduleWork(InitiateScan, static_cast<intptr_t>(BleScanState::kScanForDiscriminator));
}

void BLEManagerImpl::InitiateScan(BleScanState scanType)
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    ScanFilterData data = {
        { 0x0 },
    };

    ChipLogProgress(DeviceLayer, "Initiate Scan");

    /* Check Scanning state */
    if (scanType == BleScanState::kNotScanning)
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(DeviceLayer, "Invalid scan type requested");
        goto exit;
    }
    /* Check Tizen BLE layer is initialized or not */
    if (!mFlags.Has(Flags::kTizenBLELayerInitialized))
    {
        err = CHIP_ERROR_INCORRECT_STATE;
        ChipLogError(DeviceLayer, "Tizen BLE Layer is not yet initialized");
        goto exit;
    }
    /* Setup ScanFilter */
    memset(&data.service_data, 0x00, sizeof(data.service_data));
    data.service_data_len = 0;
    strcpy(data.service_uuid, chip_ble_service_uuid_short);

    /* Acquire Chip Device Scanner */
    if (!mDeviceScanner)
        mDeviceScanner = Internal::ChipDeviceScanner::Create(this);

    if (!mDeviceScanner)
    {
        err = CHIP_ERROR_INTERNAL;
        ChipLogError(DeviceLayer, "Failed to create a BLE device scanner");
        goto exit;
    }

    /* Send StartChipScan Request to Scanner Class */
    err = mDeviceScanner->StartChipScan(kNewConnectionScanTimeoutMs, ScanFilterType::kServiceData, data);

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to start a BLE Scan: %s", chip::ErrorStr(err));
        goto exit;
    }

    ChipLogError(DeviceLayer, "BLE Scan Initiation Successful");
    mBLEScanConfig.mBleScanState = scanType;
    return;

exit:
    ChipLogError(DeviceLayer, "Scan Initiation Failed!");
    mBLEScanConfig.mBleScanState = BleScanState::kNotScanning;
    BleConnectionDelegate::OnConnectionError(mBLEScanConfig.mAppState, err);
}

void BLEManagerImpl::InitiateScan(intptr_t arg)
{
    sInstance.InitiateScan(static_cast<BleScanState>(arg));
}

CHIP_ERROR BLEManagerImpl::CancelConnection()
{
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
