/*
 *
 *    Copyright (c) 2020-2024 Project CHIP Authors
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
 *          for Telink platforms.
 */

#include <platform/internal/CHIPDeviceLayerInternal.h>

#if CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE

#include <platform/telink/BLEManagerImpl.h>

#include <ble/Ble.h>
#include <lib/support/CHIPMemString.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/DeviceInstanceInfoProvider.h>
#include <platform/internal/BLEManager.h>
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
#include <setup_payload/AdditionalDataPayloadGenerator.h>
#endif

#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/drivers/flash.h>
#if defined(CONFIG_ZEPHYR_VERSION_3_3)
#include <zephyr/random/rand32.h>
#else
#include <zephyr/random/random.h>
#endif
#include <zephyr/storage/flash_map.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/sys/util.h>

extern "C" {
#if defined(CONFIG_BT_B9X)
extern __attribute__((noinline)) int b9x_bt_blc_mac_init(uint8_t * bt_mac);
#elif defined(CONFIG_BT_TLX)
extern __attribute__((noinline)) int tlx_bt_blc_mac_init(uint8_t * bt_mac);
#elif defined(CONFIG_BT_W91)
extern __attribute__((noinline)) void telink_bt_blc_mac_init(uint8_t * bt_mac);
#endif
}

#if defined(CONFIG_PM) && !defined(CONFIG_CHIP_ENABLE_PM_DURING_BLE)
#include <zephyr/pm/policy.h>
#endif

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

CHIP_ERROR InitBLEMACAddress()
{
    // By default the BLE public address will be applied. In case if previously generated random address doesn't exist
    // or public address doesn't exist, the new random static address will be generated and written to flash
    int error = 0;
    bt_addr_le_t addr;

#if defined(CONFIG_BT_B9X)
    b9x_bt_blc_mac_init(addr.a.val);
#elif defined(CONFIG_BT_TLX)
    tlx_bt_blc_mac_init(addr.a.val);
#elif defined(CONFIG_BT_W91)
    telink_bt_blc_mac_init(addr.a.val);
#endif

    if (BT_ADDR_IS_STATIC(&addr.a)) // in case of Random static address, create a new id
    {
        addr.type = BT_ADDR_LE_RANDOM;

        error = bt_id_create(&addr, nullptr);

        if (error < 0)
        {
            ChipLogError(DeviceLayer, "Failed to create BLE identity: %d", error);
            return System::MapErrorZephyr(error);
        }
    }

    ChipLogProgress(DeviceLayer, "BLE address: %02X:%02X:%02X:%02X:%02X:%02X", addr.a.val[5], addr.a.val[4], addr.a.val[3],
                    addr.a.val[2], addr.a.val[1], addr.a.val[0]);
    return CHIP_NO_ERROR;
}

} // unnamed namespace

BLEManagerImpl BLEManagerImpl::sInstance;

CHIP_ERROR BLEManagerImpl::_Init(void)
{
    mBLERadioInitialized  = false;
    mconId                = NULL;
    mInternalScanCallback = new InternalScanCallback(this);

    mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Enabled;
    mFlags.ClearAll().Set(Flags::kAdvertisingEnabled, CHIP_DEVICE_CONFIG_CHIPOBLE_ENABLE_ADVERTISING_AUTOSTART);
    mFlags.Set(Flags::kFastAdvertisingEnabled, true);
    mGAPConns = 0;

    memset(mSubscribedConns, 0, sizeof(mSubscribedConns));

    ReturnErrorOnFailure(InitBLEMACAddress());
    // int err = bt_enable(NULL); // Can't init BLE stack here due to abscense of non-cuncurrent mode
    // VerifyOrReturnError(err == 0, MapErrorZephyr(err));

    memset(&mConnCallbacks, 0, sizeof(mConnCallbacks));
    mConnCallbacks.connected    = HandleConnect;
    mConnCallbacks.disconnected = HandleDisconnect;

    bt_conn_cb_register(&mConnCallbacks);

    // Initialize the CHIP BleLayer.
    ReturnErrorOnFailure(BleLayer::Init(this, this, &DeviceLayer::SystemLayer()));

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::_Shutdown()
{
    if (mBLERadioInitialized)
    {
        bt_disable();
        mBLERadioInitialized = false;
    }
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
        ChipLogError(DeviceLayer, "Disabling CHIPoBLE service due to error: %" CHIP_ERROR_FORMAT, err.Format());
        mServiceMode = ConnectivityManager::kCHIPoBLEServiceMode_Disabled;
    }
}

struct BLEManagerImpl::ServiceData
{
    uint8_t uuid[2];
    ChipBLEDeviceIdentificationInfo deviceIdInfo;
} __attribute__((packed));

inline CHIP_ERROR BLEManagerImpl::PrepareAdvertisingRequest(void)
{
    static ServiceData serviceData;
    static std::array<bt_data, 2> advertisingData;
    static std::array<bt_data, 1> scanResponseData;
    static_assert(sizeof(serviceData) == 10, "Unexpected size of BLE advertising data!");

    const char * name      = bt_get_name();
    const uint8_t nameSize = static_cast<uint8_t>(strlen(name));

    Encoding::LittleEndian::Put16(serviceData.uuid, UUID16_CHIPoBLEService.val);
    ReturnErrorOnFailure(ConfigurationMgr().GetBLEDeviceIdentificationInfo(serviceData.deviceIdInfo));

    advertisingData[0]  = BT_DATA(BT_DATA_FLAGS, &kAdvertisingFlags, sizeof(kAdvertisingFlags));
    advertisingData[1]  = BT_DATA(BT_DATA_SVC_DATA16, &serviceData, sizeof(serviceData));
    scanResponseData[0] = BT_DATA(BT_DATA_NAME_COMPLETE, name, nameSize);

    mAdvertisingRequest.priority         = CHIP_DEVICE_BLE_ADVERTISING_PRIORITY;
    mAdvertisingRequest.options          = kAdvertisingOptions;
    mAdvertisingRequest.minInterval      = mFlags.Has(Flags::kFastAdvertisingEnabled)
             ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MIN
             : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MIN;
    mAdvertisingRequest.maxInterval      = mFlags.Has(Flags::kFastAdvertisingEnabled)
             ? CHIP_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL_MAX
             : CHIP_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL_MAX;
    mAdvertisingRequest.advertisingData  = Span<bt_data>(advertisingData);
    mAdvertisingRequest.scanResponseData = nameSize ? Span<bt_data>(scanResponseData) : Span<bt_data>{};

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::StartAdvertising(void)
{
    CHIP_ERROR err = CHIP_NO_ERROR;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Device provisioned, can't StartAdvertising");

        err = CHIP_ERROR_INCORRECT_STATE;
    }
    else if (!mBLERadioInitialized)
    {
        ThreadStackMgrImpl().StartThreadScan(mInternalScanCallback);
    }
    else
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD
    {
        err = StartAdvertisingProcess();
    }

    return err;
}

CHIP_ERROR BLEManagerImpl::StartAdvertisingProcess(void)
{
    int err;

    if (!mBLERadioInitialized)
    {
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
        // Deinit Thread
        ThreadStackMgrImpl().SetThreadEnabled(false);
        ThreadStackMgrImpl().SetRadioBlocked(true);
#endif

        // Init BLE
        err = bt_enable(NULL);
        VerifyOrReturnError(err == 0, MapErrorZephyr(err));

        mBLERadioInitialized = true;
#if defined(CONFIG_PM) && !defined(CONFIG_CHIP_ENABLE_PM_DURING_BLE)
        pm_policy_state_lock_get(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#endif
    }

    // Prepare advertising request
    ReturnErrorOnFailure(PrepareAdvertisingRequest());

    // Register dynamically CHIPoBLE GATT service
    if (!mFlags.Has(Flags::kChipoBleGattServiceRegister))
    {
        err = bt_gatt_service_register(&sChipoBleService);

        if (err != 0)
            ChipLogError(DeviceLayer, "Failed to register CHIPoBLE GATT service");

        VerifyOrReturnError(err == 0, MapErrorZephyr(err));

        mFlags.Set(Flags::kChipoBleGattServiceRegister);
    }

    // Initialize C3 characteristic data
#if CHIP_ENABLE_ADDITIONAL_DATA_ADVERTISING
    ReturnErrorOnFailure(PrepareC3CharData());
#endif

    // Request advertising
    ReturnErrorOnFailure(System::MapErrorZephyr(bt_le_adv_stop()));
    const bt_le_adv_param params = BT_LE_ADV_PARAM_INIT(mAdvertisingRequest.options, mAdvertisingRequest.minInterval,
                                                        mAdvertisingRequest.maxInterval, nullptr);
    ReturnErrorOnFailure(System::MapErrorZephyr(
        bt_le_adv_start(&params, mAdvertisingRequest.advertisingData.data(), mAdvertisingRequest.advertisingData.size(),
                        mAdvertisingRequest.scanResponseData.data(), mAdvertisingRequest.scanResponseData.size())));
    ChipLogProgress(DeviceLayer, "CHIPoBLE advertising started");

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
            // Start timer to change advertising interval.
            DeviceLayer::SystemLayer().StartTimer(
                System::Clock::Milliseconds32(CHIP_DEVICE_CONFIG_BLE_ADVERTISING_INTERVAL_CHANGE_TIME),
                HandleBLEAdvertisementIntervalChange, this);
        }
    }

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::StopAdvertising(void)
{
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    if (ConnectivityMgr().IsThreadProvisioned())
    {
        ChipLogProgress(DeviceLayer, "Device provisioned, StopAdvertising done");

        return CHIP_ERROR_INCORRECT_STATE;
    }
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

    ReturnErrorOnFailure(System::MapErrorZephyr(bt_le_adv_stop()));

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

CHIP_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
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
        mGAPConns++;
    }
    else
    {
        ChipLogError(DeviceLayer, "BLE connection failed (reason: 0x%02x)", connEvent->HciResult);
    }

    ChipLogProgress(DeviceLayer, "Current number of connections: %u/%u", NumConnections(), CONFIG_BT_MAX_CONN);

    mFlags.Set(Flags::kAdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    mconId = connEvent->BtConn;
    bt_conn_unref(connEvent->BtConn);

    return CHIP_NO_ERROR;
}

CHIP_ERROR BLEManagerImpl::HandleGAPDisconnect(const ChipDeviceEvent * event)
{
    const BleConnEventType * connEvent = &event->Platform.BleConnEvent;

    ChipLogProgress(DeviceLayer, "BLE GAP connection terminated (reason 0x%02x)", connEvent->HciResult);

    mGAPConns--;

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

    ChipLogProgress(DeviceLayer, "Current number of connections: %u/%u", NumConnections(), CONFIG_BT_MAX_CONN);

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
CHIP_ERROR BLEManagerImpl::PrepareC3CharData(void)
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

    case DeviceEventType::kPlatformZephyrBleC1WriteEvent:
        err = HandleRXCharWrite(event);
        break;

    case DeviceEventType::kPlatformZephyrBleC2IndDoneEvent:
        err = HandleTXCharComplete(event);
        break;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    case DeviceEventType::kThreadStateChange:
        err = HandleThreadStateChange(event);
        break;

    case DeviceEventType::kCHIPoBLEConnectionClosed:
        err = HandleBleConnectionClosed(event);
        break;

    case DeviceEventType::kOperationalNetworkEnabled:
        err = HandleOperationalNetworkEnabled(event);
        break;
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

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
    return mGAPConns;
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
    params->len  = pBuf->DataLength();

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

    PlatformMgr().LockChipStack();

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

    PlatformMgr().LockChipStack();

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

    return bt_gatt_attr_read(conId, attr, buf, len, offset, sInstance.c3CharDataBufferHandle->Start(),
                             sInstance.c3CharDataBufferHandle->DataLength());
}
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
CHIP_ERROR BLEManagerImpl::HandleOperationalNetworkEnabled(const ChipDeviceEvent * event)
{
    ChipLogDetail(DeviceLayer, "HandleOperationalNetworkEnabled");

    int error = bt_conn_disconnect(BLEMgrImpl().mconId, BT_HCI_ERR_LOCALHOST_TERM_CONN);
    if (error)
    {
        ChipLogError(DeviceLayer, "Close BLEConn err: %d", error);
    }

    return MapErrorZephyr(error);
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
        VerifyOrExit(error == CHIP_NO_ERROR, ChipLogError(DeviceLayer, "PostEvent err: %" CHIP_ERROR_FORMAT, error.Format()));
    }

exit:
    return error;
}

CHIP_ERROR BLEManagerImpl::HandleBleConnectionClosed(const ChipDeviceEvent * event)
{
    if (ThreadStackMgrImpl().IsReadyToAttach())
    {
        SwitchToIeee802154();
    }

    return CHIP_NO_ERROR;
}

void BLEManagerImpl::SwitchToIeee802154(void)
{
    ChipLogProgress(DeviceLayer, "Switch context from BLE to Thread");

    // Deinit BLE
    bt_disable();
    mBLERadioInitialized = false;

#if defined(CONFIG_PM) && !defined(CONFIG_CHIP_ENABLE_PM_DURING_BLE)
    pm_policy_state_lock_put(PM_STATE_SUSPEND_TO_IDLE, PM_ALL_SUBSTATES);
#endif

    // Init Thread
    ThreadStackMgrImpl().SetRadioBlocked(false);
    ThreadStackMgrImpl().SetThreadEnabled(true);
}
#endif // CHIP_DEVICE_CONFIG_ENABLE_THREAD

} // namespace Internal
} // namespace DeviceLayer
} // namespace chip

#endif // CHIP_DEVICE_CONFIG_ENABLE_CHIPOBLE
