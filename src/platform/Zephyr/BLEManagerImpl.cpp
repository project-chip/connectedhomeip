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
 *          for Zephyr platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <platform/Zephyr/BLEManagerImpl.h>

#include <ble/Ble.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/SafeInt.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/internal/BLEManager.h>
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#endif

#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

#ifdef CONFIG_BT_BONDABLE
#include <zephyr/settings/settings.h>
#endif // CONFIG_BT_BONDABLE

#include <array>

using namespace ::chip;
using namespace ::chip::Ble;
using namespace ::chip::System;

namespace chip {
namespace DeviceLayer {
namespace Internal {

namespace {

constexpr uint32_t kAdvertisingOptions = BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_ONE_TIME;
constexpr uint8_t kAdvertisingFlags    = BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR;

const bt_uuid_128 UUID128_CHIPoBLEChar_RX =
    BT_UUID_INIT_128(0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18);
const bt_uuid_128 UUID128_CHIPoBLEChar_TX =
    BT_UUID_INIT_128(0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18);
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
const bt_uuid_128 UUID128_CHIPoBLEChar_C3 =
    BT_UUID_INIT_128(0x04, 0x8F, 0x21, 0x83, 0x8A, 0x74, 0x7D, 0xB8, 0xF2, 0x45, 0x72, 0x87, 0x38, 0x02, 0x63, 0x64);
#endif

bt_uuid_16 UUID16_CHIPoBLEService = BT_UUID_INIT_16(0xFFF6);

_bt_gatt_ccc CHIPoBLEChar_TX_CCC = BT_GATT_CCC_INITIALIZER(nullptr, BLEManagerImpl::HandleTXCCCWrite, nullptr);

// clang-format off

bt_gatt_attr sChipoBleAttributes[] = {
    BT_GATT_PRIMARY_SERVICE(&UUID16_CHIPoBLEService.uuid),
        BT_GATT_CHARACTERISTIC(&UUID128_CHIPoBLEChar_RX.uuid,
                               BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
                               BT_GATT_PERM_READ | BT_GATT_PERM_WRITE,
                               nullptr, BLEManagerImpl::HandleRXWrite, nullptr),
        BT_GATT_CHARACTERISTIC(&UUID128_CHIPoBLEChar_TX.uuid,
                               BT_GATT_CHRC_INDICATE,
                               BT_GATT_PERM_NONE,
                               nullptr, nullptr, nullptr),
        BT_GATT_CCC_MANAGED(&CHIPoBLEChar_TX_CCC, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
        BT_GATT_CHARACTERISTIC(&UUID128_CHIPoBLEChar_C3.uuid,
                               BT_GATT_CHRC_READ,
                               BT_GATT_PERM_READ,
                               BLEManagerImpl::HandleC3Read, nullptr, nullptr),
#endif
};

bt_gatt_service sChipoBleService = BT_GATT_SERVICE(sChipoBleAttributes);

// clang-format on

// Index of the CCC descriptor in the CHIPoBLE_Service array of attributes.
// This value should be adjusted accordingly if the service declaration changes.
constexpr int kCHIPoBLE_CCC_AttributeIndex = 3;

#ifdef CONFIG_BT_BONDABLE
constexpr uint8_t kMatterBleIdentity = 1;
#else
constexpr uint8_t kMatterBleIdentity = 0;
#endif // CONFIG_BT_BONDABLE

int InitRandomStaticAddress(bool idPresent, int & id)
{
    // Generate a random static address for the default identity.
    // This must be done before bt_enable() as after that updating the default identity is not possible.
    int error = 0;
    bt_addr_le_t addr;

    // generating the address
    addr.type = BT_ADDR_LE_RANDOM;
    error     = sys_csrand_get(addr.a.val, sizeof(addr.a.val));

    if (error)
    {
        ChipLogError(DeviceLayer, "Failed to create BLE address: %d", error);
        return error;
    }

    BT_ADDR_SET_STATIC(&addr.a);

    if (!idPresent)
    {
        id = bt_id_create(&addr, nullptr);
    }
#if CONFIG_BT_ID_MAX == 2
    else
    {
        id = bt_id_reset(1, &addr, nullptr);
    }
#endif // CONFIG_BT_BONDABLE

    if (id < 0)
    {
        ChipLogError(DeviceLayer, "Failed to create BLE identity: %d", error);
        return id;
    }

    ChipLogProgress(DeviceLayer, "BLE address: %02X:%02X:%02X:%02X:%02X:%02X", addr.a.val[5], addr.a.val[4], addr.a.val[3],
                    addr.a.val[2], addr.a.val[1], addr.a.val[0]);
    return 0;
}

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init()
{
    int err = 0;
    int id  = 0;

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);
    mMatterConnNum = 0;
    mTotalConnNum  = 0;

    memset(mSubscribedConns, 0, sizeof(mSubscribedConns));

#ifdef CONFIG_BT_BONDABLE
    bt_addr_le_t idsAddr[CONFIG_BT_ID_MAX];
    size_t idsCount = CONFIG_BT_ID_MAX;

    err = bt_enable(nullptr);

    VerifyOrReturnError(err == 0, MapErrorZephyr(err));

    settings_load();

    bt_id_get(idsAddr, &idsCount);

    err = InitRandomStaticAddress(idsCount > 1, id);

    VerifyOrReturnError(err == 0 && id == kMatterBleIdentity, MapErrorZephyr(err));

#else
    err = InitRandomStaticAddress(false, id);
    VerifyOrReturnError(err == 0 && id == kMatterBleIdentity, MapErrorZephyr(err));
    err = bt_enable(nullptr);
    VerifyOrReturnError(err == 0, MapErrorZephyr(err));
#endif // CONFIG_BT_BONDABLE

    BLEAdvertisingArbiter::Init(static_cast<uint8_t>(id));

    memset(&mConnCallbacks, 0, sizeof(mConnCallbacks));
    mConnCallbacks.connected    = HandleConnect;
    mConnCallbacks.disconnected = HandleDisconnect;

    bt_conn_cb_register(&mConnCallbacks);

    // Initialize the CHIP BleLayer.
    ReturnErrorOnFailure(BleLayer::Init(this, this, &DeviceLayer::SystemLayer()));

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
            if (err != CHIP_NO_ERROR)
            {
                // Return prematurely but keep the CHIPoBLE service mode enabled to allow advertising retries
                mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
                ChipLogError(DeviceLayer, "Could not start CHIPoBLE service due to error: %" CHIP_ERROR_FORMAT, err.Format());
                return;
            }
        }
    }
    else
    {
        if (mFlags.Has(Flags::kAdvertising))
        {
            err = StopAdvertising();
            if (err != CHIP_NO_ERROR)
            {
                ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %" CHIP_ERROR_FORMAT, err.Format());
                mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
                return;
            }
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
}

struct BLEManagerImpl::ServiceData
{
    uint8_t uuid[2];
    ChipBLEDeviceIdentificationInfo deviceIdInfo;
} __attribute__((packed));

inline CHIP_ERROR BLEManagerImpl::PrepareAdvertisingRequest()
{
#ifdef CONFIG_CHIP_CUSTOM_BLE_ADV_DATA
    if (mCustomAdvertising.empty())
    {
        ChipLogError(DeviceLayer, "mCustomAdvertising should be set when CONFIG_CHIP_CUSTOM_BLE_ADV_DATA is define");
        return CHIP_ERROR_INTERNAL;
    }
#else
    static ServiceData serviceData;
    static std::array<bt_data, 2> advertisingData;
    static std::array<bt_data, 1> scanResponseData;
    static_assert(sizeof(serviceData) == 10, "Unexpected size of BLE advertising data!");

    const char * name      = bt_get_name();
    const uint8_t nameSize = static_cast<uint8_t>(strlen(name));

    Encoding::LittleEndian::Put16(serviceData.uuid, UUID16_CHIPoBLEService.val);
    ReturnErrorOnFailure(ConfigurationMgr().GetBLEDeviceIdentificationInfo(serviceData.deviceIdInfo));

#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
    if (mFlags.Has(Flags::kExtendedAdvertisingEnabled))
    {
        serviceData.deviceIdInfo.SetVendorId(DEVICE_HANDLE_NULL);
        serviceData.deviceIdInfo.SetProductId(DEVICE_HANDLE_NULL);
        serviceData.deviceIdInfo.SetExtendedAnnouncementFlag(true);
    }
#endif

    advertisingData[0]                   = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
    advertisingData[1]                   = BT_DATA(BT_DATA_SVC_DATA16, &serviceData, sizeof(serviceData));
    scanResponseData[0]                  = BT_DATA(BT_DATA_NAME_COMPLETE, name, nameSize);
#endif // CONFIG_CHIP_CUSTOM_BLE_ADV_DATA

    mAdvertisingRequest.priority = CHIP_DEVICE_BLE_ADVERTISING_PRIORITY;
    mAdvertisingRequest.options  = kAdvertisingOptions;

    if (mFlags.Has(Flags::kFastAdvertisingEnabled))
    {
        mAdvertisingRequest.minInterval = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN;
        mAdvertisingRequest.maxInterval = CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX;
    }
#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
    else if (mFlags.Has(Flags::kExtendedAdvertisingEnabled))
    {
        mAdvertisingRequest.minInterval = CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MIN;
        mAdvertisingRequest.maxInterval = CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_MAX;
    }
#endif
    else
    {
        mAdvertisingRequest.minInterval = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
        mAdvertisingRequest.maxInterval = CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
    }
#ifdef CONFIG_CHIP_CUSTOM_BLE_ADV_DATA
    mAdvertisingRequest.advertisingData  = mCustomAdvertising;
    mAdvertisingRequest.scanResponseData = mCustomScanResponse;
#else
    mAdvertisingRequest.advertisingData  = Span<bt_data>(advertisingData);
    mAdvertisingRequest.scanResponseData = nameSize ? Span<bt_data>(scanResponseData) : Span<bt_data>{};
#endif
    mAdvertisingRequest.onStarted = [](int rc) {
        if (rc == 0)
        {
            ChipLogProgress(DeviceLayer, "CHIPoBLE advertising started");
        }
        else
        {
            ChipLogError(DeviceLayer, "Failed to start CHIPoBLE advertising: %d", rc);
            BLEManagerImpl().StopAdvertising();
        }
    };

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::RegisterGattService()
{
    // Register CHIPoBLE GATT service
    if (!mFlags.Has(Flags::kChipoBleGattServiceRegister))
    {
        int err = bt_gatt_service_register(&sChipoBleService);
        if (err != 0)
        {
            ChipLogError(DeviceLayer, "Failed to register CHIPoBLE GATT service: %d", err);
        }

        VerifyOrReturnError(err == 0, MapErrorZephyr(err));
        mFlags.Set(Flags::kChipoBleGattServiceRegister);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::UnregisterGattService()
{
    // Unregister CHIPoBLE GATT service
    if (mFlags.Has(Flags::kChipoBleGattServiceRegister))
    {
        int err = bt_gatt_service_unregister(&sChipoBleService);
        if (err != 0)
        {
            ChipLogError(DeviceLayer, "Failed to unregister CHIPoBLE GATT service: %d", err);
        }

        VerifyOrReturnError(err == 0, MapErrorZephyr(err));
        mFlags.Clear(Flags::kChipoBleGattServiceRegister);
    }
    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising()
{
    // Prepare advertising request
    ReturnErrorOnFailure(PrepareAdvertisingRequest());
    // We need to register GATT service before issuing the advertising to start
    ReturnErrorOnFailure(RegisterGattService());

    // Initialize C3 characteristic data
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    ReturnErrorOnFailure(PrepareC3CharData());
#endif

    // Request advertising
    CHIP_ERROR err = BLEAdvertisingArbiter::InsertRequest(mAdvertisingRequest);
    if (CHIP_NO_ERROR != err)
    {
        // It makes not sense to keep GATT services registered after the advertising request failed
        (void) UnregisterGattService();
        return err;
    }

    // Transition to the Advertising state...
    if (!mFlags.Has(Flags::kAdvertising))
    {
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
            // Start timer to change advertising interval from fast to slow.
            DeviceLayer::SystemLayer().StartTimer(
                System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME),
                HandleSlowBLEAdvertisementInterval, this);

#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
            // Start timer to schedule start of the extended advertising
            DeviceLayer::SystemLayer().StartTimer(
                System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_BLE_EXT_ADVERTISING_INTERVAL_CHANGE_TIME_MS),
                HandleExtendedBLEAdvertisementInterval, this);
#endif
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising()
{
    BLEAdvertisingArbiter::CancelRequest(mAdvertisingRequest);

    // Transition to the not Advertising state...
    if (mFlags.Has(Flags::kAdvertising))
    {
        mFlags.Clear(Flags::kAdvertising);
        mFlags.Set(Flags::kFastAdvertisingEnabled, true);

#if CHIP_DEVICE_CONFIG_EXT_ADVERTISING
        mFlags.Clear(Flags::kExtendedAdvertisingEnabled);
#endif

        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising stopped");

        // Post a CHIPoBLEAdvertisingChange(Stopped) event.
        {
            ChipDeviceEvent advChange;
            advChange.Type                             = DeviceEventType::kCHIPoBLEAdvertisingChange;
            advChange.CHIPoBLEAdvertisingChange.Result = kActivity_Stopped;
            ReturnErrorOnFailure(PlatformMgr().PostEvent(&advChange));
        }

        // Cancel timer event changing CHIPoBLE advertisement interval
        DeviceLayer::SystemLayer().CancelTimer(HandleSlowBLEAdvertisementInterval, this);
        DeviceLayer::SystemLayer().CancelTimer(HandleExtendedBLEAdvertisementInterval, this);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "CHIPoBLE advertising already stopped");
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    ChipLogDetail(DeviceLayer, "CHIPoBLE advertising set to %s", val ? "on" : "off");

    mFlags.Set(Flags::kAdvertisingEnabled, val);
    // Ensure that each enabling/disabling of the general advertising clears
    // the extended mode, to make sure we always start fresh in the regular mode
    mFlags.Set(Flags::kExtendedAdvertisingEnabled, false);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetAdvertisingMode(BLEAdvertisingMode mode)
{
    switch (mode)
    {
    case BLEAdvertisingMode::kFastAdvertising:
        mFlags.Set(Flags::kFastAdvertisingEnabled, true);
        mFlags.Set(Flags::kExtendedAdvertisingEnabled, false);
        break;
    case BLEAdvertisingMode::kSlowAdvertising:
        mFlags.Set(Flags::kFastAdvertisingEnabled, false);
        mFlags.Set(Flags::kExtendedAdvertisingEnabled, false);
        break;
    case BLEAdvertisingMode::kExtendedAdvertising:
        mFlags.Set(Flags::kExtendedAdvertisingEnabled, true);
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
    Platform::CopyString(buf, bufSize, bt_get_name());

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::_SetDeviceName(const char * deviceName)
{
    ChipLogDetail(DeviceLayer, "Device name set to: %s", deviceName);
    return MapErrorZephyr(bt_set_name(deviceName));
}

CHIP_ERROR BLEManagerImpl::HandleGAPConnect(const ChipDeviceEvent * event)
{
    const BleConnEventType * connEvent = &event->Platform.BleConnEvent;

    if (connEvent->HciResult == BT_HCI_ERR_SUCCESS)
    {
        ChipLogProgress(DeviceLayer, "BLE connection established (ConnId: 0x%02x)", bt_conn_index(connEvent->BtConn));
        mMatterConnNum++;
    }
    else
    {
        ChipLogError(DeviceLayer, "BLE connection failed (reason: 0x%02x)", connEvent->HciResult);
    }

    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    bt_conn_unref(connEvent->BtConn);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleGAPDisconnect(const ChipDeviceEvent * event)
{
    const BleConnEventType * connEvent = &event->Platform.BleConnEvent;

    ChipLogProgress(DeviceLayer, "BLE GAP connection terminated (reason 0x%02x)", connEvent->HciResult);

    if (mMatterConnNum > 0)
    {
        mMatterConnNum--;
    }

    // If indications were enabled for this connection, record that they are now disabled and
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

    ChipDeviceEvent disconnectEvent;
    disconnectEvent.Type = DeviceEventType::kCHIPoBLEConnectionClosed;
    ReturnErrorOnFailure(PlatformMgr().PostEvent(&disconnectEvent));

    // Force a reconfiguration of advertising in case we switched to non-connectable mode when
    // the BLE connection was established.
    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleTXCharCCCDWrite(const ChipDeviceEvent * event)
{
    const BleCCCWriteEventType * writeEvent = &event->Platform.BleCCCWriteEvent;

    ChipLogDetail(DeviceLayer, "ConnId: 0x%02x, New CCCD value: 0x%04x", bt_conn_index(writeEvent->BtConn), writeEvent->Value);

    // If the client has requested to enable indications and if it is not yet subscribed
    if (writeEvent->Value == BT_GATT_CCC_INDICATE && SetSubscribed(writeEvent->BtConn))
    {
        // Alert the BLE layer that CHIPoBLE "subscribe" has been received and increment the bt_conn reference counter.
        HandleSubscribeReceived(writeEvent->BtConn, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);

        ChipLogProgress(DeviceLayer, "CHIPoBLE connection established (ConnId: 0x%02x, GATT MTU: %u)",
                        bt_conn_index(writeEvent->BtConn), GetMTU(writeEvent->BtConn));

        // Post a CHIPoBLEConnectionEstablished event to the DeviceLayer and the application.
        {
            ChipDeviceEvent conEstEvent;
            conEstEvent.Type = DeviceEventType::kCHIPoBLEConnectionEstablished;
            ReturnErrorOnFailure(PlatformMgr().PostEvent(&conEstEvent));
        }
    }
    else
    {
        if (UnsetSubscribed(writeEvent->BtConn))
        {
            HandleUnsubscribeReceived(writeEvent->BtConn, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
        }
    }

    bt_conn_unref(writeEvent->BtConn);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleRXCharWrite(const ChipDeviceEvent * event)
{
    const BleC1WriteEventType * c1WriteEvent = &event->Platform.BleC1WriteEvent;

    ChipLogDetail(DeviceLayer, "Write request received for CHIPoBLE RX characteristic (ConnId 0x%02x)",
                  bt_conn_index(c1WriteEvent->BtConn));

    HandleWriteReceived(c1WriteEvent->BtConn, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_1_UUID,
                        PacketBufferHandle::Adopt(c1WriteEvent->Data));
    bt_conn_unref(c1WriteEvent->BtConn);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleTXCharComplete(const ChipDeviceEvent * event)
{
    const BleC2IndDoneEventType * c2IndDoneEvent = &event->Platform.BleC2IndDoneEvent;

    ChipLogDetail(DeviceLayer, "Indication for CHIPoBLE TX characteristic done (ConnId 0x%02x, result 0x%02x)",
                  bt_conn_index(c2IndDoneEvent->BtConn), c2IndDoneEvent->Result);

    // Signal the BLE Layer that the outstanding indication is complete.
    HandleIndicationConfirmation(c2IndDoneEvent->BtConn, &CHIP_BLE_SVC_ID, &Ble::CHIP_BLE_CHAR_2_UUID);
    bt_conn_unref(c2IndDoneEvent->BtConn);

    return CHIP_NO_ERROR;
}

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
CHIP_ERROR BLEManagerImpl::PrepareC3CharData()
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

    err = AdditionalDataPayloadGenerator().generateAdditionalDataPayload(additionalDataPayloadParams, c3CharDataBufferHandle,
                                                                         additionalDataFields);

exit:
    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Failed to generate TLV encoded Additional Data (%s)", __func__);
    }

    return err;
}
#endif

void BLEManagerImpl::HandleSlowBLEAdvertisementInterval(System::Layer * layer, void * param)
{
    BLEMgr().SetAdvertisingMode(BLEAdvertisingMode::kSlowAdvertising);
    ChipLogProgress(DeviceLayer, "CHIPoBLE advertising mode changed to slow");
}

void BLEManagerImpl::HandleExtendedBLEAdvertisementInterval(System::Layer * layer, void * param)
{
    BLEMgr().SetAdvertisingMode(BLEAdvertisingMode::kExtendedAdvertising);
    ChipLogProgress(DeviceLayer, "CHIPoBLE advertising mode changed to extended");
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
        err = HandleTXCharComplete(event);
        break;

    default:
        break;
    }

    if (err != CHIP_NO_ERROR)
    {
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %" CHIP_ERROR_FORMAT, err.Format());
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    return mMatterConnNum;
}

CHIP_ERROR BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    ChipLogProgress(DeviceLayer, "Closing BLE GATT connection (ConnId %02x)", bt_conn_index(conId));
    return MapErrorZephyr(bt_conn_disconnect(conId, BT_HCI_ERR_REMOTE_USER_TERM_CONN));
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return bt_gatt_get_mtu(conId);
}

CHIP_ERROR BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                   const ChipBleUUID * charId)
{
    ChipLogDetail(DeviceLayer, "BLE central not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId,
                                                     const ChipBleUUID * charId)
{
    ChipLogDetail(DeviceLayer, "BLE central not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

CHIP_ERROR BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                          PacketBufferHandle pBuf)
{
    CHIP_ERROR err                   = CHIP_NO_ERROR;
    int status                       = 0;
    uint8_t index                    = bt_conn_index(conId);
    bt_gatt_indicate_params * params = &mIndicateParams[index];

    VerifyOrExit(IsSubscribed(conId) == true, err = CHIP_ERROR_INVALID_ARGUMENT);

    ChipLogDetail(DeviceLayer, "Sending indication for CHIPoBLE TX characteristic (ConnId %02x, len %u)", index,
                  pBuf->DataLength());

    params->uuid = nullptr;
    params->attr = &sChipoBleAttributes[kCHIPoBLE_CCC_AttributeIndex];
    params->func = HandleTXIndicated;
    params->data = pBuf->Start();
    VerifyOrExit(CanCastTo<uint16_t>(pBuf->DataLength()), err = CHIP_ERROR_MESSAGE_TOO_LONG);
    params->len = static_cast<uint16_t>(pBuf->DataLength());

    status = bt_gatt_indicate(conId, params);
    VerifyOrExit(status == 0, err = MapErrorZephyr(status));

exit:
    return err;
}

CHIP_ERROR BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const ChipBleUUID * svcId, const ChipBleUUID * charId,
                                            PacketBufferHandle pBuf)
{
    ChipLogDetail(DeviceLayer, "BLE central not implemented");
    return CHIP_ERROR_NOT_IMPLEMENTED;
}

void BLEManagerImpl::NotifyChipConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    CloseConnection(conId);
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

    if (!packetBuf.IsNull())
    {
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

    PlatformMgr().PostEventOrDie(&event);

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

    PlatformMgr().PostEventOrDie(&event);

    return sizeof(value);
}

void BLEManagerImpl::HandleTXIndicated(struct bt_conn * conId, bt_gatt_indicate_params *, uint8_t err)
{
    ChipDeviceEvent event;

    event.Type                              = DeviceEventType::kPlatformZephyrBleC2IndDoneEvent;
    event.Platform.BleC2IndDoneEvent.BtConn = bt_conn_ref(conId);
    event.Platform.BleC2IndDoneEvent.Result = err;

    PlatformMgr().PostEventOrDie(&event);
}

void BLEManagerImpl::HandleConnect(struct bt_conn * conId, uint8_t err)
{
    ChipDeviceEvent event;
    bt_conn_info bt_info;

    PlatformMgr().LockChipStack();

    sInstance.mTotalConnNum++;
    ChipLogProgress(DeviceLayer, "Current number of connections: %u/%u", sInstance.mTotalConnNum, CONFIG_BT_MAX_CONN);

    VerifyOrExit(bt_conn_get_info(conId, &bt_info) == 0, );
    // Drop all callbacks incoming for the role other than peripheral, required by the Matter accessory
    VerifyOrExit(bt_info.role == BT_CONN_ROLE_PERIPHERAL, );
    // Don't handle BLE connecting events when it is not related to CHIPoBLE
    VerifyOrExit(sInstance.mFlags.Has(Flags::kChipoBleGattServiceRegister), );

    event.Type                            = DeviceEventType::kPlatformZephyrBleConnected;
    event.Platform.BleConnEvent.BtConn    = bt_conn_ref(conId);
    event.Platform.BleConnEvent.HciResult = err;

    PlatformMgr().PostEventOrDie(&event);

exit:
    PlatformMgr().UnlockChipStack();
}

void BLEManagerImpl::HandleDisconnect(struct bt_conn * conId, uint8_t reason)
{
    ChipDeviceEvent event;
    bt_conn_info bt_info;

    PlatformMgr().LockChipStack();

    if (sInstance.mTotalConnNum > 0)
    {
        sInstance.mTotalConnNum--;
    }

    ChipLogProgress(DeviceLayer, "Current number of connections: %u/%u", sInstance.mTotalConnNum, CONFIG_BT_MAX_CONN);

    VerifyOrExit(bt_conn_get_info(conId, &bt_info) == 0, );
    // Drop all callbacks incoming for the role other than peripheral, required by the Matter accessory
    VerifyOrExit(bt_info.role == BT_CONN_ROLE_PERIPHERAL, );
    // Don't handle BLE disconnecting events when it is not related to CHIPoBLE
    VerifyOrExit(sInstance.mFlags.Has(Flags::kChipoBleGattServiceRegister), );

    event.Type                            = DeviceEventType::kPlatformZephyrBleDisconnected;
    event.Platform.BleConnEvent.BtConn    = bt_conn_ref(conId);
    event.Platform.BleConnEvent.HciResult = reason;

    PlatformMgr().PostEventOrDie(&event);

exit:
    PlatformMgr().UnlockChipStack();
}

#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
ssize_t BLEManagerImpl::HandleC3Read(struct bt_conn * conId, const struct bt_gatt_attr * attr, void * buf, uint16_t len,
                                     uint16_t offset)
{
    ChipLogDetail(DeviceLayer, "Read request received for CHIPoBLE C3 (ConnId 0x%02x)", bt_conn_index(conId));

    if (sInstance.c3CharDataBufferHandle.IsNull())
    {
        return 0;
    }

    // For BLE, the max payload size is limited to UINT16_MAX since the length
    // field is 2 bytes long. So, the cast to uint16_t should be fine.
    return bt_gatt_attr_read(conId, attr, buf, len, offset, sInstance.c3CharDataBufferHandle->Start(),
                             static_cast<uint16_t>(sInstance.c3CharDataBufferHandle->DataLength()));
}
#endif

#ifdef CONFIG_CHIP_CUSTOM_BLE_ADV_DATA
void BLEManagerImpl::SetCustomAdvertising(Span<bt_data> CustomAdvertising)
{
    mCustomAdvertising = CustomAdvertising;
}
void BLEManagerImpl::SetCustomScanResponse(Span<bt_data> CustomScanResponse)
{
    mCustomScanResponse = CustomScanResponse;
}
#endif

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
