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
 *          for Zephyr platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <platform/Zephyr/BLEManagerImpl.h>

#include <ble/CHIPBleServiceData.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/internal/BLEManager.h>

#include <bluetooth/addr.h>
#include <bluetooth/gatt.h>
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
bt_uuid_16 UUID16_CHIPoBLEService = BT_UUID_INIT_16(0xFFF6);

const ChipBleUUID chipUUID_CHIPoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x11 } };

const ChipBleUUID chipUUID_CHIPoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F,
                                                 0x9D, 0x12 } };

_bt_gatt_ccc CHIPoBLEChar_TX_CCC = BT_GATT_CCC_INITIALIZER(nullptr, BLEManagerImpl::HandleTXCCCWrite, nullptr);

// clang-format off

struct bt_gatt_attr sChipoBleAttributes[] = {    BT_GATT_PRIMARY_SERVICE(&UUID16_CHIPoBLEService.uuid),
        BT_GATT_CHARACTERISTIC(&UUID128_CHIPoBLEChar_RX.uuid,
                               BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                               BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                               nullptr, BLEManagerImpl::HandleRXWrite, nullptr),
        BT_GATT_CHARACTERISTIC(&UUID128_CHIPoBLEChar_TX.uuid,
                               BT_GATT_CHRC_NOTIFY,
                               BT_GATT_PERM_NONE,
                               nullptr, nullptr, nullptr),
        BT_GATT_CCC_MANAGED(&CHIPoBLEChar_TX_CCC, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)};

struct bt_gatt_service sChipoBleService = BT_GATT_SERVICE(sChipoBleAttributes);

// clang-format on

// Index of the CCC descriptor in the CHIPoBLE_Service array of attributes.
// This value should be adjusted accordingly if the service declaration changes.
constexpr int kCHIPoBLE_CCC_AttributeIndex = 3;

void InitRandomStaticAddress()
{
#if !CONFIG_BT_PRIVACY
    // When the BT privacy feature is disabled, generate a random static address once per boot.
    // This must be done before bt_enable() has been called.
    bt_addr_le_t addr;

    int error = bt_addr_le_create_static(&addr);
    VerifyOrReturn(error == 0, ChipLogError(DeviceLayer, "Failed to create BLE address: %d", error));

    error = bt_id_create(&addr, nullptr);
    VerifyOrReturn(error == 0, ChipLogError(DeviceLayer, "Failed to create BLE identity: %d", error));

    ChipLogProgress(DeviceLayer, "BLE address was set to %02X:%02X:%02X:%02X:%02X:%02X", addr.a.val[5], addr.a.val[4],
                    addr.a.val[3], addr.a.val[2], addr.a.val[1], addr.a.val[0]);
#endif
}

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);
    mGAPConns = 0;

    memset(mSubscribedConns, 0, sizeof(mSubscribedConns));

    InitRandomStaticAddress();
    int err = bt_enable(NULL);
    VerifyOrReturnError(err == 0, MapErrorZephyr(err));

    memset(&mConnCallbacks, 0, sizeof(mConnCallbacks));
    mConnCallbacks.connected    = HandleConnect;
    mConnCallbacks.disconnected = HandleDisconnect;

    bt_conn_cb_register(&mConnCallbacks);

    // Initialize the CHIP BleLayer.
    ReturnErrorOnFailure(BleLayer::Init(this, this, &SystemLayer));

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

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

        // If CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled,
        // disable CHIPoBLE advertising if the device is fully provisioned.
#if CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            mFlags.Clear(Flags::kAdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
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

        // If no connections are active unregister also CHIPoBLE GATT service
        if (NumConnections() == 0 && mFlags.Has(Flags::kChipoBleGattServiceRegister))
        {
            // Unregister CHIPoBLE service to not allow discovering it when pairing is disabled.
            if (bt_gatt_service_unregister(&sChipoBleService) != 0)
            {
                ChipLogError(DeviceLayer, "Failed to unregister CHIPoBLE GATT service");
            }
            else
            {
                mFlags.Clear(Flags::kChipoBleGattServiceRegister);
            }
        }
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
    int err                       = 0;
    const char * deviceName       = bt_get_name();
    const uint8_t advFlags        = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;
    const bool isAdvertisingRerun = mFlags.Has(Flags::kAdvertising);

    // At first run always select fast advertising, on the next attemp slow down interval.
    const uint32_t intervalMin = mFlags.Has(Flags::kFastAdvertisingEnabled) ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN
                                                                            : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
    const uint32_t intervalMax = mFlags.Has(Flags::kFastAdvertisingEnabled) ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX
                                                                            : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;

    bt_le_adv_param advParams =
        BT_LE_ADV_PARAM_INIT(BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_ONE_TIME, intervalMin, intervalMax, nullptr);

    // Define advertising data
    ServiceData serviceData;
    bt_data ad[] = { BT_DATA(BT_DATA_FLAGS, &advFlags, sizeof(advFlags)),
                     BT_DATA(BT_DATA_SVC_DATA16, &serviceData, sizeof(serviceData)),
                     BT_DATA(BT_DATA_NAME_COMPLETE, deviceName, static_cast<uint8_t>(strlen(deviceName))) };

    // Register dynamically CHIPoBLE GATT service
    if (!mFlags.Has(Flags::kChipoBleGattServiceRegister))
    {
        err = bt_gatt_service_register(&sChipoBleService);

        if (err != 0)
            ChipLogError(DeviceLayer, "Failed to register CHIPoBLE GATT service");

        VerifyOrReturnError(err == 0, MapErrorZephyr(err));

        mFlags.Set(Flags::kChipoBleGattServiceRegister);
    }

    // Initialize service data
    static_assert(sizeof(serviceData) == 9, "Size of BLE advertisement data changed! Was that intentional?");
    chip::Encoding::LittleEndian::Put16(serviceData.uuid, UUID16_CHIPoBLEService.val);
    ReturnErrorOnFailure(ConfigurationMgr().GetBLEDeviceIdentificationInfo(serviceData.deviceIdInfo));

    if (!isAdvertisingRerun)
    {
#if CONFIG_BT_PRIVACY
        static_assert((CHIP_DEVICE_CONFIG_BLE_ADVERTISING_TIMEOUT / 1000) <= CONFIG_BT_RPA_TIMEOUT,
                      "BLE advertising timeout is too long relative to RPA timeout");
        // Generate new private BLE address
        bt_le_oob bleOobInfo;
        err = bt_le_oob_get_local(advParams.id, &bleOobInfo);
        VerifyOrReturnError(err == 0, MapErrorZephyr(err));
#endif // CONFIG_BT_PRIVACY
    }

    // Restart advertising
    err = bt_le_adv_stop();
    VerifyOrReturnError(err == 0, MapErrorZephyr(err));

    err = bt_le_adv_start(&advParams, ad, ARRAY_SIZE(ad), nullptr, 0u);
    VerifyOrReturnError(err == 0, MapErrorZephyr(err));

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
            PlatformMgr().PostEvent(&advChange);
        }

        if (mFlags.Has(Flags::kFastAdvertisingEnabled))
        {
            // Start timer to change advertising interval.
            SystemLayer.StartTimer(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME, HandleBLEAdvertisementIntervalChange,
                                   this);
        }

        // Start timer to disable CHIPoBLE advertisement after timeout expiration only if it isn't advertising rerun (in that case
        // timer is already running).
        if (!isAdvertisingRerun)
        {
            SystemLayer.StartTimer(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_TIMEOUT, HandleBLEAdvertisementTimeout, this);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
    int err = bt_le_adv_stop();
    VerifyOrReturnError(err == 0, MapErrorZephyr(err));

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
            PlatformMgr().PostEvent(&advChange);
        }

        // Cancel timer event disabling CHIPoBLE advertisement after timeout expiration
        SystemLayer.CancelTimer(HandleBLEAdvertisementTimeout, this);

        // Cancel timer event changing CHIPoBLE advertisement interval
        SystemLayer.CancelTimer(HandleBLEAdvertisementIntervalChange, this);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetCHIPoBLEServiceMode(CHIPoBLEServiceMode val)
{
    VerifyOrReturnError(val != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported, CHIP_ERROR_INVALID_ARGUMENT);
    VerifyOrReturnError(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported,
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (val != mServiceMode)
    {
        mServiceMode = val;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    VerifyOrReturnError(mServiceMode != ConnectivityManager::kCHIPoBLEServiceMode_NotSupported,
                        CHIP_ERROR_UNSUPPORTED_CHIP_FEATURE);

    if (mFlags.Has(Flags::kAdvertisingEnabled) != val)
    {
        ChipLogDetail(DeviceLayer, "CHIPoBLE advertising set to %s", val ? "on" : "off");

        mFlags.Set(Flags::kAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
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
    size_t len = bufSize - 1;

    strncpy(buf, bt_get_name(), len);
    buf[len] = 0;

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    if (mServiceMode == ConnectivityManager::kCHIPoBLEServiceMode_NotSupported)
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

    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    bt_conn_unref(connEvent->BtConn);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleGAPDisconnect(const ChipDeviceEvent * event)
{
    const BleConnEventType * connEvent = &event->Platform.BleConnEvent;

    ChipLogProgress(DeviceLayer, "BLE GAP connection terminated (reason 0x%02" PRIx16 ")", connEvent->HciResult);

    mGAPConns--;

    // If notifications were enabled for this connection, record that they are now disabled and
    // notify the BLE Layer of a disconnect.
    if (UnsetSubscribed(connEvent->BtConn))
    {
        CHIP_ERROR disconReason;
        switch (connEvent->HciResult)
        {
        case BT_HCI_ERR_REMOTE_USER_TERM_CONN:
            // Do not treat proper connection termination as an error and exit.
            VerifyOrExit(!ConfigurationMgr().IsFullyProvisioned(), );
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

exit:
    // Unref bt_conn before scheduling DriveBLEState.
    bt_conn_unref(connEvent->BtConn);

    ChipLogProgress(DeviceLayer, "Current number of connections: %" PRIu16 "/%" PRIu16, NumConnections(), CONFIG_BT_MAX_CONN);

    ChipDeviceEvent disconnectEvent;
    disconnectEvent.Type = DeviceEventType::kCHIPoBLEConnectionClosed;
    PlatformMgr().PostEvent(&disconnectEvent);

    // Force a reconfiguration of advertising in case we switched to non-connectable mode when
    // the BLE connection was established.
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleTXCharCCCDWrite(const ChipDeviceEvent * event)
{
    const BleCCCWriteEventType * writeEvent = &event->Platform.BleCCCWriteEvent;

    ChipLogDetail(DeviceLayer, "ConnId: 0x%02" PRIx16 ", New CCCD value: 0x%04" PRIx16, bt_conn_index(writeEvent->BtConn),
                  writeEvent->Value);

    // If the client has requested to enable notifications and if it is not yet subscribed
    if (writeEvent->Value != 0 && SetSubscribed(writeEvent->BtConn))
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
    const BleRXWriteEventType * writeEvent = &event->Platform.BleRXWriteEvent;

    ChipLogDetail(DeviceLayer, "Write request received for CHIPoBLE RX (ConnId 0x%02" PRIx16 ")",
                  bt_conn_index(writeEvent->BtConn));

    HandleWriteReceived(writeEvent->BtConn, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_RX,
                        PacketBufferHandle::Adopt(writeEvent->Data));
    bt_conn_unref(writeEvent->BtConn);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleTXCharComplete(const ChipDeviceEvent * event)
{
    const BleTXCompleteEventType * completeEvent = &event->Platform.BleTXCompleteEvent;

    ChipLogDetail(DeviceLayer, "Notification for CHIPoBLE TX done (ConnId 0x%02" PRIx16 ")", bt_conn_index(completeEvent->BtConn));

    // Signal the BLE Layer that the outstanding notification is complete.
    HandleIndicationConfirmation(completeEvent->BtConn, &CHIP_BLE_SVC_ID, &chipUUID_CHIPoBLEChar_TX);
    bt_conn_unref(completeEvent->BtConn);

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::HandleBLEAdvertisementTimeout(System::Layer * layer, void * param)
{
    BLEMgr().SetAdvertisingEnabled(false);
    ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because of timeout expired");
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
    case DeviceEventType::kPlatformZephyrBleConnected:
        err = HandleGAPConnect(event);
        break;

    case DeviceEventType::kPlatformZephyrBleDisconnected:
        err = HandleGAPDisconnect(event);
        break;

    case DeviceEventType::kPlatformZephyrBleCCCWrite:
        err = HandleTXCharCCCDWrite(event);
        break;

    case DeviceEventType::kPlatformZephyrBleRXWrite:
        err = HandleRXCharWrite(event);
        break;

    case DeviceEventType::kPlatformZephyrBleTXComplete:
        err = HandleTXCharComplete(event);
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
            mFlags.Clear(Flags::kAdvertisingEnabled);
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising disabled because device is fully provisioned");
        }
#endif // CHIP_DEVICE_CONFIG_CHIPOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED

        // Force the advertising state to be refreshed to reflect new provisioning state.
        mFlags.Set(Flags::kAdvertisingRefreshNeeded);

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
    return bt_conn_disconnect(conId, BT_HCI_ERR_REMOTE_USER_TERM_CONN) == 0;
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
    CHIP_ERROR err                 = CHIP_NO_ERROR;
    int status                     = 0;
    uint8_t index                  = bt_conn_index(conId);
    bt_gatt_notify_params * params = &mNotifyParams[index];

    VerifyOrExit(IsSubscribed(conId) == true, err = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(DeviceLayer, "Sending notification for CHIPoBLE TX (ConnId %02" PRIx16 ", len %u)", index, pBuf->DataLength());

    params->uuid      = nullptr;
    params->attr      = &sChipoBleAttributes[kCHIPoBLE_CCC_AttributeIndex];
    params->data      = pBuf->Start();
    params->len       = pBuf->DataLength();
    params->func      = HandleTXCompleted;
    params->user_data = nullptr;

    status = bt_gatt_notify_cb(conId, params);
    VerifyOrExit(status == 0, err = MapErrorZephyr(status));

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Sending notification for CHIPoBLE TX failed: %s", ErrorStr(err));
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

ssize_t BLEManagerImpl::HandleRXWrite(struct bt_conn * conId, const struct bt_gatt_attr * attr, const void * buf, uint16_t len,
                                      uint16_t offset, uint8_t flags)
{
    ChipDeviceEvent event;
    PacketBufferHandle packetBuf = PacketBufferHandle::NewWithData(buf, len);

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
        // Arrange to post a CHIPoBLERXWriteEvent event to the CHIP queue.
        event.Type                            = DeviceEventType::kPlatformZephyrBleRXWrite;
        event.Platform.BleRXWriteEvent.BtConn = bt_conn_ref(conId);
        event.Platform.BleRXWriteEvent.Data   = std::move(packetBuf).UnsafeRelease();
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

    if (value != BT_GATT_CCC_NOTIFY && value != 0)
    {
        return BT_GATT_ERR(BT_ATT_ERR_VALUE_NOT_ALLOWED);
    }

    event.Type                             = DeviceEventType::kPlatformZephyrBleCCCWrite;
    event.Platform.BleCCCWriteEvent.BtConn = bt_conn_ref(conId);
    event.Platform.BleCCCWriteEvent.Value  = value;

    PlatformMgr().PostEvent(&event);

    return sizeof(value);
}

void BLEManagerImpl::HandleTXCompleted(struct bt_conn * conId, void * /* param */)
{
    ChipDeviceEvent event;

    event.Type                               = DeviceEventType::kPlatformZephyrBleTXComplete;
    event.Platform.BleTXCompleteEvent.BtConn = bt_conn_ref(conId);

    PlatformMgr().PostEvent(&event);
}

void BLEManagerImpl::HandleConnect(struct bt_conn * conId, uint8_t err)
{
    ChipDeviceEvent event;

    PlatformMgr().LockChipStack();

    // Don't handle BLE connecting events when it is not related to CHIPoBLE
    VerifyOrExit(sInstance.mFlags.Has(Flags::kChipoBleGattServiceRegister), );

    event.Type                            = DeviceEventType::kPlatformZephyrBleConnected;
    event.Platform.BleConnEvent.BtConn    = bt_conn_ref(conId);
    event.Platform.BleConnEvent.HciResult = err;

    PlatformMgr().PostEvent(&event);

exit:
    PlatformMgr().UnlockChipStack();
}

void BLEManagerImpl::HandleDisconnect(struct bt_conn * conId, uint8_t reason)
{
    ChipDeviceEvent event;

    PlatformMgr().LockChipStack();

    // Don't handle BLE disconnecting events when it is not related to CHIPoBLE
    VerifyOrExit(sInstance.mFlags.Has(Flags::kChipoBleGattServiceRegister), );

    event.Type                            = DeviceEventType::kPlatformZephyrBleDisconnected;
    event.Platform.BleConnEvent.BtConn    = bt_conn_ref(conId);
    event.Platform.BleConnEvent.HciResult = reason;

    PlatformMgr().PostEvent(&event);

exit:
    PlatformMgr().UnlockChipStack();
}
} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
