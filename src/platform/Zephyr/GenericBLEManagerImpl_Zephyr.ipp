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

#ifndef GENERIC_BLE_MANAGER_IMPL_ZEPHYR_IPP
#define GENERIC_BLE_MANAGER_IMPL_ZEPHYR_IPP

#include <platform/Zephyr/GenericBLEManagerImpl_Zephyr.h>

#include <ble/CHIPBleServiceData.h>
#include <platform/internal/BLEManager.h>
#include <support/CodeUtils.h>
#include <support/logging/CHIPLogging.h>

#include <logging/log.h>
#include <sys/byteorder.h>
#include <sys/util.h>

LOG_MODULE_DECLARE(chip, LOG_LEVEL_DBG);

using namespace ::chip;
using namespace ::chip::Ble;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {
namespace Internal {

// Fully instantiate the generic implementation class in whatever compilation unit includes this file.
template class GenericBLEManagerImpl_Zephyr<BLEManagerImpl>;

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

_bt_gatt_ccc CHIPoBLEChar_TX_CCC =
    BT_GATT_CCC_INITIALIZER(nullptr, GenericBLEManagerImpl_Zephyr<BLEManagerImpl>::HandleTXCCCWrite, nullptr);

// clang-format off

BT_GATT_SERVICE_DEFINE(CHIPoBLE_Service,
    BT_GATT_PRIMARY_SERVICE(&UUID16_CHIPoBLEService.uuid),
        BT_GATT_CHARACTERISTIC(&UUID128_CHIPoBLEChar_RX.uuid,
                               BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                               BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                               nullptr, GenericBLEManagerImpl_Zephyr<BLEManagerImpl>::HandleRXWrite, nullptr),
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

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::_Init()
{
    CHIP_ERROR err;

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags       = kFlag_AdvertisingEnabled;
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

template <class ImplClass>
void GenericBLEManagerImpl_Zephyr<ImplClass>::DriveBLEState(intptr_t arg)
{
    BLEMgrImpl().DriveBLEState();
}

template <class ImplClass>
void GenericBLEManagerImpl_Zephyr<ImplClass>::DriveBLEState()
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

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::StartAdvertising(void)
{
    ChipBLEDeviceIdentificationInfo deviceIdInfo;
    const char * deviceName = bt_get_name();
    uint8_t advFlags        = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;
    uint16_t advUUID        = sys_cpu_to_le16(UUID16_CHIPoBLEService.val);
    CHIP_ERROR err          = CHIP_NO_ERROR;

    // Advertising data Initialize with previously set name, flags and service UUIDs.
    bt_data ad[] = { BT_DATA(BT_DATA_NAME_COMPLETE, deviceName, static_cast<uint8_t>(strlen(deviceName))),
                     BT_DATA(BT_DATA_FLAGS, &advFlags, sizeof(advFlags)), BT_DATA(BT_DATA_UUID16_ALL, &advUUID, sizeof(advUUID)) };

    // Scan response data
    bt_data sd[] = { BT_DATA(BT_DATA_UUID16_ALL, &advUUID, sizeof(advUUID)),
                     BT_DATA(BT_DATA_SVC_DATA16, &deviceIdInfo, sizeof(deviceIdInfo)) };

    uint32_t minAdvInterval =
        (NumConnections() == 0 && !ConfigurationMgr().IsFullyProvisioned()) || GetFlag(mFlags, kFlag_FastAdvertisingEnabled)
        ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL
        : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL;

    bt_le_adv_param param = BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_ONE_TIME, minAdvInterval,
                                                 CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL, nullptr);

    // If necessary, inform the ThreadStackManager that CHIPoBLE advertising is about to start.
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (!GetFlag(mFlags, kFlag_Advertising))
    {
        ThreadStackMgr().OnCHIPoBLEAdvertisingStart();
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    SuccessOrExit(err);

    err = bt_le_adv_stop();
    VerifyOrExit(err == CHIP_NO_ERROR, err = MapErrorZephyr(err));

    err = bt_le_adv_start(&param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err == -ENOMEM)
    {
        // No free connection objects for connectable advertiser. Advertise as non-connectable instead.
        param.options &= ~BT_LE_ADV_OPT_CONNECTABLE;
        err = bt_le_adv_start(&param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
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
    }

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::StopAdvertising(void)
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
    }

exit:
    return err;
}

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
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

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::_SetAdvertisingEnabled(bool val)
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

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::_SetFastAdvertisingEnabled(bool val)
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

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::_GetDeviceName(char * buf, size_t bufSize)
{
    size_t len = bufSize - 1;

    strncpy(buf, bt_get_name(), len);
    buf[len] = 0;

    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::_SetDeviceName(const char * deviceName)
{
    if (mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported)
    {
        return CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE;
    }

    ChipLogDetail(DeviceLayer, "Device name set to: %s", deviceName);
    return MapErrorZephyr(bt_set_name(deviceName));
}

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::HandleGAPConnect(const ChipDeviceEvent * event)
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

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::HandleGAPDisconnect(const ChipDeviceEvent * event)
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

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::HandleTXCharCCCDWrite(const ChipDeviceEvent * event)
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

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::HandleRXCharWrite(const ChipDeviceEvent * event)
{
    const BleC1WriteEventType * c1WriteEvent = &event->Platform.BleC1WriteEvent;

    ChipLogDetail(DeviceLayer, "Write request received for CHIPoBLE RX characteristic (ConnId 0x%02" PRIx16 ")",
                  bt_conn_index(c1WriteEvent->BtConn));

    HandleWriteReceived(c1WriteEvent->BtConn, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_RX, c1WriteEvent->Data);
    bt_conn_unref(c1WriteEvent->BtConn);

    return CHIP_NO_ERROR;
}

template <class ImplClass>
CHIP_ERROR GenericBLEManagerImpl_Zephyr<ImplClass>::HandleTXComplete(const ChipDeviceEvent * event)
{
    const BleC2IndDoneEventType * c2IndDoneEvent = &event->Platform.BleC2IndDoneEvent;

    ChipLogDetail(DeviceLayer, "Indication for CHIPoBLE TX characteristic done (ConnId 0x%02" PRIx16 ", result 0x%02" PRIx16 ")",
                  bt_conn_index(c2IndDoneEvent->BtConn), c2IndDoneEvent->Result);

    // Signal the BLE Layer that the outstanding indication is complete.
    HandleIndicationConfirmation(c2IndDoneEvent->BtConn, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
    bt_conn_unref(c2IndDoneEvent->BtConn);

    return CHIP_NO_ERROR;
}

template <class ImplClass>
void GenericBLEManagerImpl_Zephyr<ImplClass>::_OnPlatformEvent(const ChipDeviceEvent * event)
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

template <class ImplClass>
uint16_t GenericBLEManagerImpl_Zephyr<ImplClass>::_NumConnections(void)
{
    return mGAPConns;
}

template <class ImplClass>
bool GenericBLEManagerImpl_Zephyr<ImplClass>::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (ConnId %02" PRIx16 ")", bt_conn_index(conId));
    return bt_conn_disconnect(conId, BT_HCI_ERR_REMOTE_USER_TERM_CONN) == CHIP_NO_ERROR;
}

template <class ImplClass>
uint16_t GenericBLEManagerImpl_Zephyr<ImplClass>::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return bt_gatt_get_mtu(conId);
}

template <class ImplClass>
bool GenericBLEManagerImpl_Zephyr<ImplClass>::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                                      const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

template <class ImplClass>
bool GenericBLEManagerImpl_Zephyr<ImplClass>::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                                        const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

template <class ImplClass>
bool GenericBLEManagerImpl_Zephyr<ImplClass>::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                             const ChipBleUUID * charId, PacketBuffer * pBuf)
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
        ChipLogError(DeviceLayer, "GenericBLEManagerImpl_Zephyr<ImplClass>::SendIndication() failed: %s", ErrorStr(err));
    }

    PacketBuffer::Free(pBuf);

    return err == CHIP_NO_ERROR;
}

template <class ImplClass>
bool GenericBLEManagerImpl_Zephyr<ImplClass>::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                               const ChipBleUUID * charId, PacketBuffer * pBuf)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

template <class ImplClass>
bool GenericBLEManagerImpl_Zephyr<ImplClass>::SendReadRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                              const ChipBleUUID * charId, PacketBuffer * pBuf)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

template <class ImplClass>
bool GenericBLEManagerImpl_Zephyr<ImplClass>::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext,
                                                               const ChipBleUUID * svcId, const ChipBleUUID * charId)
{
    ChipLogError(DeviceLayer, "%s: NOT IMPLEMENTED", __PRETTY_FUNCTION__);
    return true;
}

template <class ImplClass>
void GenericBLEManagerImpl_Zephyr<ImplClass>::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // Intentionally empty.
}

template <class ImplClass>
bool GenericBLEManagerImpl_Zephyr<ImplClass>::IsSubscribed(bt_conn * conn)
{
    return mSubscribedConns[bt_conn_index(conn)];
}

template <class ImplClass>
bool GenericBLEManagerImpl_Zephyr<ImplClass>::SetSubscribed(bt_conn * conn)
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

template <class ImplClass>
bool GenericBLEManagerImpl_Zephyr<ImplClass>::UnsetSubscribed(bt_conn * conn)
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

template <class ImplClass>
ssize_t GenericBLEManagerImpl_Zephyr<ImplClass>::HandleRXWrite(struct bt_conn * conId, const struct bt_gatt_attr * attr,
                                                               const void * buf, uint16_t len, uint16_t offset, uint8_t flags)
{
    ChipDeviceEvent event;
    PacketBuffer * packetBuf = PacketBuffer::NewWithAvailableSize(len);

    LOG_HEXDUMP_DBG(buf, len, "Rx char write");

    // If successful...
    if (packetBuf != NULL)
    {
        // Copy the characteristic value into the packet buffer.
        memcpy(packetBuf->Start(), buf, len);
        packetBuf->SetDataLength(len);

        // Arrange to post a CHIPoBLERXWriteEvent event to the CHIP queue.
        event.Type                            = DeviceEventType::kPlatformZephyrBleC1WriteEvent;
        event.Platform.BleC1WriteEvent.BtConn = bt_conn_ref(conId);
        event.Platform.BleC1WriteEvent.Data   = packetBuf;
    }

    // If we failed to allocate a buffer, post a kPlatformZephyrBleOutOfBuffersEvent event.
    else
    {
        event.Type = DeviceEventType::kPlatformZephyrBleOutOfBuffersEvent;
    }

    PlatformMgr().PostEvent(&event);

    return len;
}

template <class ImplClass>
ssize_t GenericBLEManagerImpl_Zephyr<ImplClass>::HandleTXCCCWrite(struct bt_conn * conId, const struct bt_gatt_attr * attr,
                                                                  uint16_t value)
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

template <class ImplClass>
void GenericBLEManagerImpl_Zephyr<ImplClass>::HandleTXIndicated(struct bt_conn * conId, const struct bt_gatt_attr * attr,
                                                                uint8_t err)
{
    ChipDeviceEvent event;

    event.Type                              = DeviceEventType::kPlatformZephyrBleC2IndDoneEvent;
    event.Platform.BleC2IndDoneEvent.BtConn = bt_conn_ref(conId);
    event.Platform.BleC2IndDoneEvent.Result = err;

    PlatformMgr().PostEvent(&event);
}

template <class ImplClass>
void GenericBLEManagerImpl_Zephyr<ImplClass>::HandleConnect(struct bt_conn * conId, uint8_t err)
{
    ChipDeviceEvent event;

    event.Type                            = DeviceEventType::kPlatformZephyrBleConnected;
    event.Platform.BleConnEvent.BtConn    = bt_conn_ref(conId);
    event.Platform.BleConnEvent.HciResult = err;

    PlatformMgr().PostEvent(&event);
}

template <class ImplClass>
void GenericBLEManagerImpl_Zephyr<ImplClass>::HandleDisconnect(struct bt_conn * conId, uint8_t reason)
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

#endif // GENERIC_BLE_MANAGER_IMPL_ZEPHYR_IPP
