/*
 *
 *    Copyright (c) 2020 Project CHIP Authors
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
 *          for Zephyr platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <platform/Zephyr/BLEManagerImpl.h>

#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <logging/log.h>
#include <sys/byteorder.h>
#include <sys/util.h>

LOG_MODULE_DECLARE(chip);

using namespace ::chip;
using namespace ::chip::Ble;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

const bt_uuid_128 UUID128_CHIPoBLEChar_RX =
    BT_UUID_INIT_128(0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18);
const bt_uuid_128 UUID128_CHIPoBLEChar_TX =
    BT_UUID_INIT_128(0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18);
bt_uuid_16 UUID16_CHIPoBLEService = BT_UUID_INIT_16(0xFEAF);

const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };

const ChipBleUUID chipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

_bt_gatt_ccc CHIPoBLEChar_TX_CCC = BT_GATT_CCC_INITIALIZER(nullptr, BLEManagerImpl::HandleTXCCCWrite, nullptr);

// clang-format off

BT_GATT_SERVICE_DEFINE(CHIPoBLE_Service,
    BT_GATT_PRIMARY_SERVICE(&UUID16_CHIPoBLEService.uuid),
        BT_GATT_CHARACTERISTIC(&UUID128_CHIPoBLEChar_RX.uuid,
                               BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                               BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                               nullptr, BLEManagerImpl::HandleRXWrite, nullptr),
        BT_GATT_CHARACTERISTIC(&UUID128_CHIPoBLEChar_TX.uuid,
                               BT_GATT_CHRC_INDICATE,
                               BT_GATT_PERM_NONE,
                               nullptr, nullptr, nullptr),
        BT_GATT_CCC_MANAGED(&CHIPoBLEChar_TX_CCC, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)
);

// clang-format on

// Index of the CCC descriptor in the CHIPoBLE_Service array of attributes.
// This value should be adjusted accordingly if the service declaration changes.
constexpr int kCHIPoBLE_CCC_AttributeIndex = 3;

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    CHIP_ERROR err;

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags       = CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART ? kFlag_AdvertisingEnabled : 0;
    mGAPConns    = 0;

    memset(mSubscribedConns, 0, sizeof(mSubscribedConns));

    err = bt_enable(NULL);
    VerifyOrExit(err == CHIP_NO_ERROR, err = MapErrorZephyr(err));

    memset(&mConnCallbacks, 0, sizeof(mConnCallbacks));
    mConnCallbacks.connected    = HandleConnect;
    mConnCallbacks.disconnected = HandleDisconnect;

    bt_conn_cb_register(&mConnCallbacks);

    // Initialize the CHIP BleLayer.
    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    return err;
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    BLEMgrImpl().DriveBLEState();
}

void BLEManagerImpl::DriveBLEState()
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    ChipLogDetail(DeviceLayer, "In DriveBLEState");

    // Perform any initialization actions that must occur after the CHIP task is running.
    if (!GetFlag(mFlags, kFlag_AsyncInitCompleted))
    {
        SetFlag(mFlags, kFlag_AsyncInitCompleted);

        // If CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled,
        // disable CHIPoBLE advertising if the device is fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    }

    // If the application has enabled CHIPoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_Enabled &&
        GetFlag(mFlags, kFlag_AdvertisingEnabled)
#if CHIP_DEVICE_CONFIG_CHIPOBLE_SINGLE_CONNECTION
        // and no connections are active...
        && (NumConnections() == 0)
#endif
    )
    {
        // Start/re-start advertising if not already advertising, or if the
        // advertising state needs to be refreshed.
        if (!GetFlag(mFlags, kFlag_Advertising) || GetFlag(mFlags, kFlag_AdvertisingRefreshNeeded))
        {
            err = StartAdvertising();
            SuccessOrExit(err);
        }
    }
    // Otherwise, stop advertising if currently active.
    else
    {
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

struct BLEManagerImpl::ServiceData
{
    uint8_t uuid[2];
    ChipBLEDeviceIdentificationInfo deviceIdInfo;
} __attribute__((packed));

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err;

    const char * deviceName   = bt_get_name();
    const uint8_t advFlags    = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;
    bt_le_adv_param advParams = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_ONE_TIME, GetAdvertisingInterval(),
                                                     CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL, nullptr);

    // Define advertising data
    ServiceData serviceData;
    bt_data ad[] = { BT_DATA(BT_DATA_FLAGS, &advFlags, sizeof(advFlags)),
                     BT_DATA(BT_DATA_SVC_DATA16, &serviceData, sizeof(serviceData)),
                     BT_DATA(BT_DATA_NAME_COMPLETE, deviceName, static_cast<uint8_t>(strlen(deviceName))) };

    // Initialize service data
    static_assert(sizeof(serviceData) == 9, "Size of BLE advertisement data changed! Was that intentional?");
    chip::Encoding::LittleEndian::Put16(serviceData.uuid, UUID16_CHIPoBLEService.val);
    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(serviceData.deviceIdInfo);
    SuccessOrExit(err);

    // If necessary, inform the ThreadStackManager that CHIPoBLE advertising is about to start.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (!GetFlag(mFlags, kFlag_Advertising))
    {
        ThreadStackMgr().OnCHIPoBLEAdvertisingStart();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    // Restart advertising
    err = bt_le_adv_stop();
    VerifyOrExit(err == CHIP_NO_ERROR, err = MapErrorZephyr(err));

    err = bt_le_adv_start(&advParams, ad, ARRAY_SIZE(ad), nullptr, 0u);
    if (err == -ENOMEM)
    {
        // No free connection objects for connectable advertiser. Advertise as non-connectable instead.
        advParams.options &= ~BT_LE_ADV_OPT_CONNECTABLE;
        err = bt_le_adv_start(&advParams, ad, ARRAY_SIZE(ad), nullptr, 0u);
    }

    VerifyOrExit(err == CHIP_NO_ERROR, err = MapErrorZephyr(err));

    // Transition to the Advertising state...
    if (!GetFlag(mFlags, kFlag_Advertising))
    {
        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising started");

        SetFlag(mFlags, kFlag_Advertising);

        // Post a CHIPoBLEAdvertisingChange(Started) event.
        {
            ChipDeviceEvent advChange;
            advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
            advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Started;
            PlatformMgr().PostEvent(&advChange);
        }

        // Start timer to disable CHIPoBLE advertisement after timeout expiration
        SystemLayer.StartTimer(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_TIMEOUT, HandleBLEAdvertisementTimeout, this);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    err = bt_le_adv_stop();
    VerifyOrExit(err == CHIP_NO_ERROR, err = MapErrorZephyr(err));

    // Transition to the not Advertising state...
    if (GetFlag(mFlags, kFlag_Advertising))
    {
        ClearFlag(mFlags, kFlag_Advertising);

        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        // Directly inform the ThreadStackManager that CHIPoBLE advertising has stopped.
        ThreadStackMgr().OnCHIPoBLEAdvertisingStop();
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

        // Post a CHIPoBLEAdvertisingChange(Stopped) event.
        {
            ChipDeviceEvent advChange;
            advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
            advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
            PlatformMgr().PostEvent(&advChange);
        }

        // Cancel timer event disabling CHIPoBLE advertisement after timeout expiration
        SystemLayer.CancelTimer(HandleBLEAdvertisementTimeout, this);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (val != mServiceMode)
    {
        mServiceMode = val;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (GetFlag(mFlags, kFlag_AdvertisingEnabled) != val)
    {
        ChipLogDetail(DeviceLayer, "SetAdvertisingEnabled(%s)", val ? "true" : "false");

        SetFlag(mFlags, kFlag_AdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_SetFastAdvertisingEnabled(bool val)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    VerifyOrExit(mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, err = CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (GetFlag(mFlags, kFlag_FastAdvertisingEnabled) != val)
    {
        ChipLogDetail(DeviceLayer, "SetFastAdvertisingEnabled(%s)", val ? "true" : "false");

        SetFlag(mFlags, kFlag_FastAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    size_t len = bufSize - 1;

    strncpy(buf, bt_get_name(), len);
    buf[len] = 0;

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    if (mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    ChipLogDetail(DeviceLayer, "Device name set to: %s", deviceName);
    return MapErrorZephyr(bt_set_name(deviceName));
}

CHIP_ERROR BLEManagerImpl::HandleGAPConnect(const ChipDeviceEvent * event)
{
    const BleConnEventType * connEvent = &event->Platform.BleConnEvent;

    if (connEvent->HciResult == BT_HCI_ERR_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "BLE connection established (ConnId: 0x%02" PRIx16 ")", bt_conn_index(connEvent->BtConn));
        mGAPConns++;
    }
    else
    {
        ChipLogProgress(DeviceLayer, "BLE connection failed (reason: 0x%02" PRIx16 ")", connEvent->HciResult);
    }

    ChipLogProgress(DeviceLayer, "Current number of connections: %" PRIu16 "/%" PRIu16, NumConnections(), CONFIG_BT_MAX_CONN);

    SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    bt_conn_unref(connEvent->BtConn);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleGAPDisconnect(const ChipDeviceEvent * event)
{
    const BleConnEventType * connEvent = &event->Platform.BleConnEvent;

    ChipLogProgress(DeviceLayer, "BLE GAP connection terminated (reason 0x%02" PRIx16 ")", connEvent->HciResult);

    mGAPConns--;

    // If indications were enabled for this connection, record that they are now disabled and
    // notify the BLE Layer of a disconnect.
    if (UnsetSubscribed(connEvent->BtConn))
    {
        CHIP_ERROR disconReason;
        switch (connEvent->HciResult)
        {
        case BT_HCI_ERR_REMOTE_USER_TERM_CONN:
            disconReason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;
        case BT_HCI_ERR_LOCALHOST_TERM_CONN:
            disconReason = BLE_ERROR_APP_CLOSED_CONNECTION;
            break;
        default:
            disconReason = BLE_ERROR_CHIPOBLE_PROTOCOL_ABORT;
            break;
        }
        HandleConnectionError(connEvent->BtConn, disconReason);
    }

    // Unref bt_conn before scheduling DriveBLEState.
    bt_conn_unref(connEvent->BtConn);

    ChipLogProgress(DeviceLayer, "Current number of connections: %" PRIu16 "/%" PRIu16, NumConnections(), CONFIG_BT_MAX_CONN);

    // Force a reconfiguration of advertising in case we switched to non-connectable mode when
    // the BLE connection was established.
    SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleTXCharCCCDWrite(const ChipDeviceEvent * event)
{
    const BleCCCWriteEventType * writeEvent = &event->Platform.BleCCCWriteEvent;

    ChipLogDetail(DeviceLayer, "ConnId: 0x%02" PRIx16 ", New CCCD value: 0x%04" PRIx16, bt_conn_index(writeEvent->BtConn),
                  writeEvent->Value);

    // If the client has requested to enabled indications and if it is not yet subscribed
    if (writeEvent->Value == BT_GATT_CCC_INDICATE && SetSubscribed(writeEvent->BtConn))
    {
        // Alert the BLE layer that CHIPoBLE "subscribe" has been received and increment the bt_conn reference counter.
        HandleSubscribeReceived(writeEvent->BtConn, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);

        ChipLogProgress(DeviceLayer, "CHIPoBLE connection established (ConnId: 0x%02" PRIx16 ", GATT MTU: %" PRIu16 ")",
                        bt_conn_index(writeEvent->BtConn), GetMTU(writeEvent->BtConn));

        // Post a CHIPoBLEConnectionEstablished event to the DeviceLayer and the application.
        {
            ChipDeviceEvent conEstEvent;
            conEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
            PlatformMgr().PostEvent(&conEstEvent);
        }
    }
    else
    {
        if (UnsetSubscribed(writeEvent->BtConn))
        {
            HandleUnsubscribeReceived(writeEvent->BtConn, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
        }
    }

    bt_conn_unref(writeEvent->BtConn);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleRXCharWrite(const ChipDeviceEvent * event)
{
    const BleC1WriteEventType * c1WriteEvent = &event->Platform.BleC1WriteEvent;

    ChipLogDetail(DeviceLayer, "Write request received for CHIPoBLE RX characteristic (ConnId 0x%02" PRIx16 ")",
                  bt_conn_index(c1WriteEvent->BtConn));

    HandleWriteReceived(c1WriteEvent->BtConn, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_RX,
                        PacketBufferHandle::Adopt(c1WriteEvent->Data));
    bt_conn_unref(c1WriteEvent->BtConn);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleTXComplete(const ChipDeviceEvent * event)
{
    const BleC2IndDoneEventType * c2IndDoneEvent = &event->Platform.BleC2IndDoneEvent;

    ChipLogDetail(DeviceLayer, "Indication for CHIPoBLE TX characteristic done (ConnId 0x%02" PRIx16 ", result 0x%02" PRIx16 ")",
                  bt_conn_index(c2IndDoneEvent->BtConn), c2IndDoneEvent->Result);

    // Signal the BLE Layer that the outstanding indication is complete.
    HandleIndicationConfirmation(c2IndDoneEvent->BtConn, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
    bt_conn_unref(c2IndDoneEvent->BtConn);

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::HandleBLEAdvertisementTimeout(System::Layer * layer, void * param, System::Error error)
{
    BLEMgr().SetAdvertisingEnabled(false);
    ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because of timeout expired");
}

void BLEManagerImpl::_OnPlatformEvent(const ChipDeviceEvent * event)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

    switch (event->Type)
    {
    case DeviceEventType::kPlatformZephyrBleConnected:
        err = HandleGAPConnect(event);
        break;

    case DeviceEventType::kPlatformZephyrBleDisconnected:
        err = HandleGAPDisconnect(event);
        break;

    case DeviceEventType::kPlatformZephyrBleCCCWrite:
        err = HandleTXCharCCCDWrite(event);
        break;

    case DeviceEventType::kPlatformZephyrBleC1WriteEvent:
        err = HandleRXCharWrite(event);
        break;

    case DeviceEventType::kPlatformZephyrBleC2IndDoneEvent:
        err = HandleTXComplete(event);
        break;

    case DeviceEventType::kFabricMembershipChange:
    case DeviceEventType::kServiceProvisioningChange:
    case DeviceEventType::kAccountPairingChange:

        // If CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled, and there is a change to the
        // device's provisioning state, then automatically disable CHIPoBLE advertising if the device
        // is now fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED

        // Force the advertising state to be refreshed to reflect new provisioning state.
        SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);

        DriveBLEState();

        break;

    default:
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    return mGAPConns;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (ConnId %02" PRIx16 ")", bt_conn_index(conId));
    return bt_conn_disconnect(conId, BT_HCI_ERR_REMOTE_USER_TERM_CONN) == CHIP_NO_ERROR;
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return bt_gatt_get_mtu(conId);
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                    PacketBufferHandle pBuf)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    uint8_t index                    = bt_conn_index(conId);
    bt_gatt_indicate_params * params = &mIndicateParams[index];

    VerifyOrExit(IsSubscribed(conId) == true, err = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(DeviceLayer, "Sending indication for CHIPoBLE TX characteristic (ConnId %u, len %u)", index, pBuf->DataLength());

    params->uuid = nullptr;
    params->attr = &CHIPoBLE_Service.attrs[kCHIPoBLE_CCC_AttributeIndex];
    params->func = HandleTXIndicated;
    params->data = pBuf->Start();
    params->len  = pBuf->DataLength();

    err = bt_gatt_indicate(conId, params);
    VerifyOrExit(err == CHIP_NO_ERROR, err = MapErrorZephyr(err));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "BLEManagerImpl::SendIndication() failed: %s", ErrorStr(err));
    }

    return err == CHIP_NO_ERROR;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                      PacketBufferHandle pBuf)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                     PacketBufferHandle pBuf)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                      const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // Intentionally empty.
}

bool BLEManagerImpl::IsSubscribed(bt_conn * conn)
{
    return mSubscribedConns[bt_conn_index(conn)];
}

bool BLEManagerImpl::SetSubscribed(bt_conn * conn)
{
    uint8_t index           = bt_conn_index(conn);
    bool isSubscribed       = mSubscribedConns[index];
    mSubscribedConns[index] = true;

    // If we were not subscribed previously, increment the reference counter of the connection.
    if (!isSubscribed)
    {
        bt_conn_ref(conn);
    }

    return !isSubscribed;
}

bool BLEManagerImpl::UnsetSubscribed(bt_conn * conn)
{
    uint8_t index           = bt_conn_index(conn);
    bool isSubscribed       = mSubscribedConns[index];
    mSubscribedConns[index] = false;

    // If we were subscribed previously, decrement the reference counter of the connection.
    if (isSubscribed)
    {
        bt_conn_unref(conn);
    }

    return isSubscribed;
}

uint32_t BLEManagerImpl::GetAdvertisingInterval()
{
    return (NumConnections() == 0 && !ConfigurationMgr().IsFullyProvisioned()) || GetFlag(mFlags, kFlag_FastAdvertisingEnabled)
        ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL
        : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL;
}

ssize_t BLEManagerImpl::HandleRXWrite(struct bt_conn * conId, const struct bt_gatt_attr * attr, const void * buf, uint16_t len,
                                      uint16_t offset, uint8_t flags)
{
    ChipDeviceEvent event;
    PacketBufferHandle packetBuf = PacketBuffer::NewWithAvailableSize(len);

    // Unfortunately the Zephyr logging macros end up assigning uint16_t
    // variables to uint16_t:10 fields, which triggers integer conversion
    // warnings.  And treating the Zephyr headers as system headers does not
    // help, apparently.  Just turn off that warning around this log call.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
    LOG_HEXDUMP_DBG(buf, len, "Rx char write");
#pragma GCC diagnostic pop

    // If successful...
    if (!packetBuf.IsNull())
    {
        // Copy the characteristic value into the packet buffer.
        memcpy(packetBuf->Start(), buf, len);
        packetBuf->SetDataLength(len);

        // Arrange to post a CHIPoBLERXWriteEvent event to the CHIP queue.
        event.Type                            = DeviceEventType::kPlatformZephyrBleC1WriteEvent;
        event.Platform.BleC1WriteEvent.BtConn = bt_conn_ref(conId);
        event.Platform.BleC1WriteEvent.Data   = std::move(packetBuf).UnsafeRelease();
    }

    // If we failed to allocate a buffer, post a kPlatformZephyrBleOutOfBuffersEvent event.
    else
    {
        event.Type = DeviceEventType::kPlatformZephyrBleOutOfBuffersEvent;
    }

    PlatformMgr().PostEvent(&event);

    return len;
}

ssize_t BLEManagerImpl::HandleTXCCCWrite(struct bt_conn * conId, const struct bt_gatt_attr * attr, uint16_t value)
{
    ChipDeviceEvent event;

    if (value != BT_GATT_CCC_INDICATE && value != 0)
    {
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }

    event.Type                             = DeviceEventType::kPlatformZephyrBleCCCWrite;
    event.Platform.BleCCCWriteEvent.BtConn = bt_conn_ref(conId);
    event.Platform.BleCCCWriteEvent.Value  = value;

    PlatformMgr().PostEvent(&event);

    return sizeof(value);
}

void BLEManagerImpl::HandleTXIndicated(struct bt_conn * conId, IndicationAttrType, uint8_t err)
{
    ChipDeviceEvent event;

    event.Type                              = DeviceEventType::kPlatformZephyrBleC2IndDoneEvent;
    event.Platform.BleC2IndDoneEvent.BtConn = bt_conn_ref(conId);
    event.Platform.BleC2IndDoneEvent.Result = err;

    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::HandleConnect(struct bt_conn * conId, uint8_t err)
{
    ChipDeviceEvent event;

    event.Type                            = DeviceEventType::kPlatformZephyrBleConnected;
    event.Platform.BleConnEvent.BtConn    = bt_conn_ref(conId);
    event.Platform.BleConnEvent.HciResult = err;

    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::HandleDisconnect(struct bt_conn * conId, uint8_t reason)
{
    ChipDeviceEvent event;

    event.Type                            = DeviceEventType::kPlatformZephyrBleDisconnected;
    event.Platform.BleConnEvent.BtConn    = bt_conn_ref(conId);
    event.Platform.BleConnEvent.HciResult = reason;

    PlatformMgr().PostEvent(&event);
}

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
