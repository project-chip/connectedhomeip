/*
 *
 *    Copyright (c) 2018 Nest Labs, Inc.
 *    All rights reserved.
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
 *          for the nRF5 platforms.
 */

#include <Weave/DeviceLayer/internal/WeaveDeviceLayerInternal.h>
#include <Weave/DeviceLayer/internal/BLEManager.h>
#include <BleLayer/WeaveBleServiceData.h>
#include <new>

#if WEAVE_DEVICE_CONFIG_ENABLE_WOBLE

#include "nrf_error.h"
#include "ble.h"
#include "ble_gap.h"
#include "ble_gattc.h"
#include "ble_advdata.h"
#include "ble_srv_common.h"
#include "nrf_ble_gatt.h"

#define NRF_LOG_MODULE_NAME weave
#include "nrf_log.h"

using namespace ::nl;
using namespace ::nl::Ble;

namespace nl {
namespace Weave {
namespace DeviceLayer {
namespace Internal {

namespace {

const uint16_t UUID16_WoBLEService = 0xFEAF;
const ble_uuid_t UUID_WoBLEService = { UUID16_WoBLEService, BLE_UUID_TYPE_BLE };

const ble_uuid128_t UUID128_WoBLEChar_RX = { { 0x11, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 } };
ble_uuid_t UUID_WoBLEChar_RX;
const WeaveBleUUID WeaveUUID_WoBLEChar_RX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D, 0x11 } };

const ble_uuid128_t UUID128_WoBLEChar_TX = { { 0x12, 0x9D, 0x9F, 0x42, 0x9C, 0x4F, 0x9F, 0x95, 0x59, 0x45, 0x3D, 0x26, 0xF5, 0x2E, 0xEE, 0x18 } };
ble_uuid_t UUID_WoBLEChar_TX;
const WeaveBleUUID WeaveUUID_WoBLEChar_TX = { { 0x18, 0xEE, 0x2E, 0xF5, 0x26, 0x3D, 0x45, 0x59, 0x95, 0x9F, 0x4F, 0x9C, 0x42, 0x9F, 0x9D, 0x12 } };

NRF_BLE_GATT_DEF(GATTModule);

WEAVE_ERROR RegisterVendorUUID(ble_uuid_t & uuid, const ble_uuid128_t & vendorUUID)
{
    WEAVE_ERROR err;

    err = sd_ble_uuid_vs_add(&vendorUUID, &uuid.type);
    SuccessOrExit(err);

    uuid.uuid = (((uint16_t)vendorUUID.uuid128[13]) << 8) | vendorUUID.uuid128[12];

exit:
    return err;
}

} // unnamed namespace


BLEManagerImpl BLEManagerImpl::sInstance;

WEAVE_ERROR BLEManagerImpl::_Init()
{
    WEAVE_ERROR err;
    uint16_t svcHandle;
    ble_add_char_params_t addCharParams;

    mServiceMode = ConnectivityManager::kWoBLEServiceMode_Enabled;
    mFlags = kFlag_AdvertisingEnabled;
    mAdvHandle = BLE_GAP_ADV_SET_HANDLE_NOT_SET;
    mNumGAPCons = 0;
    for (int i = 0; i < kMaxConnections; i++)
    {
        mSubscribedConIds[i] = BLE_CONNECTION_UNINITIALIZED;
    }

    // Initialize the Weave BleLayer.
    err = BleLayer::Init(this, this, &SystemLayer);
    SuccessOrExit(err);

    // Register vendor-specific UUIDs with the soft device.
    //     NOTE: An NRF_ERROR_NO_MEM here means the soft device hasn't been configured
    //     with space for enough custom UUIDs.  Typically, this limit is set by overriding
    //     the NRF_SDH_BLE_VS_UUID_COUNT config option.
    err = RegisterVendorUUID(UUID_WoBLEChar_RX, UUID128_WoBLEChar_RX);
    SuccessOrExit(err);
    err = RegisterVendorUUID(UUID_WoBLEChar_TX, UUID128_WoBLEChar_TX);
    SuccessOrExit(err);

    // Add the WoBLE service.
    err = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY, (ble_uuid_t *)&UUID_WoBLEService, &svcHandle);
    SuccessOrExit(err);

    // Add the WoBLEChar_RX characteristic to the WoBLE service.
    memset(&addCharParams, 0, sizeof(addCharParams));
    addCharParams.uuid = UUID_WoBLEChar_RX.uuid;
    addCharParams.uuid_type = UUID_WoBLEChar_RX.type;
    addCharParams.max_len = NRF_SDH_BLE_GATT_MAX_MTU_SIZE;
    addCharParams.init_len = 0;
    addCharParams.is_var_len = true;
    addCharParams.char_props.write_wo_resp = 1;
    addCharParams.char_props.write = 1;
    addCharParams.read_access = SEC_OPEN;
    addCharParams.write_access = SEC_OPEN;
    addCharParams.cccd_write_access = SEC_NO_ACCESS;
    err = characteristic_add(svcHandle, &addCharParams, &mWoBLECharHandle_RX);
    SuccessOrExit(err);

    // Add the WoBLEChar_TX characteristic.
    memset(&addCharParams, 0, sizeof(addCharParams));
    addCharParams.uuid = UUID_WoBLEChar_TX.uuid;
    addCharParams.uuid_type = UUID_WoBLEChar_TX.type;
    addCharParams.max_len = NRF_SDH_BLE_GATT_MAX_MTU_SIZE;
    addCharParams.is_var_len = true;
    addCharParams.char_props.read = 1;
    addCharParams.char_props.write = 1;
    addCharParams.char_props.indicate = 1;
    addCharParams.read_access = SEC_OPEN;
    addCharParams.write_access = SEC_OPEN;
    addCharParams.cccd_write_access = SEC_OPEN;
    err = characteristic_add(svcHandle, &addCharParams, &mWoBLECharHandle_TX);
    SuccessOrExit(err);

    // Initialize the nRF5 GATT module and set the allowable GATT MTU and GAP packet sizes
    // based on compile-time config values.
    err = nrf_ble_gatt_init(&GATTModule, NULL);
    SuccessOrExit(err);
    err = nrf_ble_gatt_att_mtu_periph_set(&GATTModule, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    SuccessOrExit(err);
    err = nrf_ble_gatt_data_length_set(&GATTModule, BLE_CONN_HANDLE_INVALID, NRF_SDH_BLE_GAP_DATA_LENGTH);
    SuccessOrExit(err);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(sBLEObserver, WEAVE_DEVICE_LAYER_BLE_OBSERVER_PRIORITY, SoftDeviceBLEEventCallback, NULL);

    // Set a default device name.
    err = _SetDeviceName(NULL);
    SuccessOrExit(err);

    // TODO: call sd_ble_gap_ppcp_set() to set gap parameters???

    PlatformMgr().ScheduleWork(DriveBLEState, 0);

exit:
    WeaveLogProgress(DeviceLayer, "BLEManagerImpl::Init() complete");
    return err;
}

WEAVE_ERROR BLEManagerImpl::_SetWoBLEServiceMode(WoBLEServiceMode val)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(val != ConnectivityManager::kWoBLEServiceMode_NotSupported, err = WEAVE_ERROR_INVALID_ARGUMENT);
    VerifyOrExit(mServiceMode != ConnectivityManager::kWoBLEServiceMode_NotSupported, err = WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE);

    if (val != mServiceMode)
    {
        mServiceMode = val;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::_SetAdvertisingEnabled(bool val)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(mServiceMode != ConnectivityManager::kWoBLEServiceMode_NotSupported, err = WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE);

    if (GetFlag(mFlags, kFlag_AdvertisingEnabled) != val)
    {
        SetFlag(mFlags, kFlag_AdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::_SetFastAdvertisingEnabled(bool val)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    VerifyOrExit(mServiceMode == ConnectivityManager::kWoBLEServiceMode_NotSupported, err = WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE);

    if (GetFlag(mFlags, kFlag_FastAdvertisingEnabled) != val)
    {
        SetFlag(mFlags, kFlag_FastAdvertisingEnabled, val);
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::_GetDeviceName(char * buf, size_t bufSize)
{
    WEAVE_ERROR err;
    uint16_t len = (uint16_t)(bufSize - 1);

    err = sd_ble_gap_device_name_get((uint8_t *)buf, &len);
    SuccessOrExit(err);

    buf[len] = 0;

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::_SetDeviceName(const char * devName)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    ble_gap_conn_sec_mode_t secMode;
    char devNameBuf[kMaxDeviceNameLength + 1];

    VerifyOrExit(mServiceMode != ConnectivityManager::kWoBLEServiceMode_NotSupported, err = WEAVE_ERROR_UNSUPPORTED_WEAVE_FEATURE);

    if (devName != NULL && devName[0] != 0)
    {
        VerifyOrExit(strlen(devName) <= kMaxDeviceNameLength, err = WEAVE_ERROR_INVALID_ARGUMENT);
        strcpy(devNameBuf, devName);
    }
    else
    {
        snprintf(devNameBuf, sizeof(devNameBuf), "%s%04" PRIX32,
                 WEAVE_DEVICE_CONFIG_BLE_DEVICE_NAME_PREFIX,
                 (uint32_t)FabricState.LocalNodeId);
        devNameBuf[kMaxDeviceNameLength] = 0;
    }

    // Do not allow device name characteristic to be changed
    BLE_GAP_CONN_SEC_MODE_SET_NO_ACCESS(&secMode);

    // Configure the device name within the BLE soft device.
    err = sd_ble_gap_device_name_set(&secMode, (const uint8_t *)devNameBuf, strlen(devNameBuf));
    SuccessOrExit(err);

exit:
    return err;
}

void BLEManagerImpl::_OnPlatformEvent(const WeaveDeviceEvent * event)
{
    switch (event->Type)
    {
    case DeviceEventType::kSoftDeviceBLEEvent:
        HandleSoftDeviceBLEEvent(event);
        break;

    case DeviceEventType::kWoBLERXCharWriteEvent:
        HandleRXCharWrite(event);
        break;

    case DeviceEventType::kWoBLEOutOfBuffersEvent:
        WeaveLogError(DeviceLayer, "BLEManagerImpl: Out of buffers during WoBLE RX");
        break;

    case DeviceEventType::kFabricMembershipChange:
    case DeviceEventType::kServiceProvisioningChange:
    case DeviceEventType::kAccountPairingChange:

        // If WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled, and there is a change to the
        // device's provisioning state, then automatically disable WoBLE advertising if the device
        // is now fully provisioned.
#if WEAVE_DEVICE_CONFIG_WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            WeaveLogProgress(DeviceLayer, "WoBLE advertising disabled because device is fully provisioned");
        }
#endif // WEAVE_DEVICE_CONFIG_WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED

        // Force the advertising state to be refreshed to reflect new provisioning state.
        SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);

        DriveBLEState();

        break;

    default:
        break;
    }
}

bool BLEManagerImpl::SubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    WeaveLogProgress(DeviceLayer, "BLEManagerImpl::SubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::UnsubscribeCharacteristic(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    WeaveLogProgress(DeviceLayer, "BLEManagerImpl::UnsubscribeCharacteristic() not supported");
    return false;
}

bool BLEManagerImpl::CloseConnection(BLE_CONNECTION_OBJECT conId)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    WeaveLogProgress(DeviceLayer, "Closing BLE GATT connection (con %u)", conId);

    // Initiate a GAP disconnect.
    err = sd_ble_gap_disconnect(conId, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "sd_ble_gap_disconnect() failed: %s", ErrorStr(err));
    }

    return (err == WEAVE_NO_ERROR);
}

uint16_t BLEManagerImpl::GetMTU(BLE_CONNECTION_OBJECT conId) const
{
    return nrf_ble_gatt_eff_mtu_get(&GATTModule, conId);
}

bool BLEManagerImpl::SendIndication(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId, PacketBuffer * data)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    ble_gatts_hvx_params_t hvxParams;
    uint16_t dataLen = data->DataLength();

    VerifyOrExit(IsSubscribed(conId), err = WEAVE_ERROR_INVALID_ARGUMENT);

    WeaveLogDetail(DeviceLayer, "Sending indication for WoBLE TX characteristic (con %u, len %u)", conId, dataLen);

    // Send a indication for the WoBLE TX characteristic to the client containing the supplied data.
    // Note that this call copies the data from the buffer.
    memset(&hvxParams, 0, sizeof(hvxParams));
    hvxParams.type = BLE_GATT_HVX_INDICATION;
    hvxParams.handle = mWoBLECharHandle_TX.value_handle;
    hvxParams.offset = 0;
    hvxParams.p_len = &dataLen;
    hvxParams.p_data = data->Start();
    err = sd_ble_gatts_hvx(conId, &hvxParams);
    SuccessOrExit(err);

exit:
    PacketBuffer::Free(data);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "BLEManagerImpl::SendIndication() failed: %s", ErrorStr(err));
        return false;
    }
    return true;
}

bool BLEManagerImpl::SendWriteRequest(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId, PacketBuffer * pBuf)
{
    WeaveLogProgress(DeviceLayer, "BLEManagerImpl::SendWriteRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadRequest(BLE_CONNECTION_OBJECT conId, const WeaveBleUUID * svcId, const WeaveBleUUID * charId, PacketBuffer * pBuf)
{
    WeaveLogProgress(DeviceLayer, "BLEManagerImpl::SendReadRequest() not supported");
    return false;
}

bool BLEManagerImpl::SendReadResponse(BLE_CONNECTION_OBJECT conId, BLE_READ_REQUEST_CONTEXT requestContext, const WeaveBleUUID * svcId, const WeaveBleUUID * charId)
{
    WeaveLogProgress(DeviceLayer, "BLEManagerImpl::SendReadResponse() not supported");
    return false;
}

void BLEManagerImpl::NotifyWeaveConnectionClosed(BLE_CONNECTION_OBJECT conId)
{
    // Nothing to do
}

void BLEManagerImpl::DriveBLEState(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    // Perform any initialization actions that must occur after the Weave task is running.
    if (!GetFlag(mFlags, kFlag_AsyncInitCompleted))
    {
        SetFlag(mFlags, kFlag_AsyncInitCompleted);

        // If WEAVE_DEVICE_CONFIG_WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED is enabled,
        // disable WoBLE advertising if the device is fully provisioned.
#if WEAVE_DEVICE_CONFIG_WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
        if (ConfigurationMgr().IsFullyProvisioned())
        {
            ClearFlag(mFlags, kFlag_AdvertisingEnabled);
            WeaveLogProgress(DeviceLayer, "WoBLE advertising disabled because device is fully provisioned");
        }
#endif // WEAVE_DEVICE_CONFIG_WOBLE_DISABLE_ADVERTISING_WHEN_PROVISIONED
    }

    // If the application has enabled WoBLE and BLE advertising...
    if (mServiceMode == ConnectivityManager::kWoBLEServiceMode_Enabled && GetFlag(mFlags, kFlag_AdvertisingEnabled)
#if WEAVE_DEVICE_CONFIG_WOBLE_SINGLE_CONNECTION
        // and no connections are active...
        && (mNumGAPCons == 0)
#endif
        )
    {
        // Start/re-start SoftDevice advertising if not already advertising, or if the
        // advertising state of the SoftDevice needs to be refreshed.
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
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Disabling WoBLE service due to error: %s", ErrorStr(err));
        mServiceMode = ConnectivityManager::kWoBLEServiceMode_Disabled;
    }
}

WEAVE_ERROR BLEManagerImpl::StartAdvertising(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    ble_gap_adv_data_t gapAdvData;
    ble_gap_adv_params_t gapAdvParams;
    uint16_t numWoBLECons;
    bool connectable;

    // If necessary, inform the ThreadStackManager that WoBLE advertising is about to start.
#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
    if (!GetFlag(mFlags, kFlag_Advertising))
    {
        ThreadStackMgr().OnWoBLEAdvertisingStart();
    }
#endif // WEAVE_DEVICE_CONFIG_ENABLE_THREAD

    // Since we're about to update the SoftDevice, clear the refresh needed flag.
    ClearFlag(mFlags, kFlag_AdvertisingRefreshNeeded);

    if (mAdvHandle != BLE_GAP_ADV_SET_HANDLE_NOT_SET)
    {
        // Instruct the SoftDevice to stop advertising.
        // Ignore any error indicating that advertising is already stopped.  This case is arises
        // when a connection is established, which causes the SoftDevice to immediately cease
        // advertising.
        err = sd_ble_gap_adv_stop(mAdvHandle);
        if (err == NRF_ERROR_INVALID_STATE)
        {
            err = WEAVE_NO_ERROR;
        }
        SuccessOrExit(err);

        // Force the SoftDevice to relinquish its references to the buffers containing the advertising
        // data.  This ensures the SoftDevice is not accessing these buffers while we are encoding
        // new advertising data into them.
        err = sd_ble_gap_adv_set_configure(&mAdvHandle, NULL, NULL);
        SuccessOrExit(err);
    }

    // Encode the data that will be sent in the advertising packet and the scan response packet.
    err = EncodeAdvertisingData(gapAdvData);
    SuccessOrExit(err);

    // Set advertising parameters.
    memset(&gapAdvParams, 0, sizeof(gapAdvParams));
    gapAdvParams.primary_phy     = BLE_GAP_PHY_1MBPS;
    gapAdvParams.duration        = BLE_GAP_ADV_TIMEOUT_GENERAL_UNLIMITED;
    gapAdvParams.filter_policy   = BLE_GAP_ADV_FP_ANY;

    // Advertise connectable if we haven't reached the maximum number of WoBLE connections or the
    // maximum number of GAP connections.  (Note that the SoftDevice will return an error if connectable
    // advertising is requested when the max number of GAP connections exist).
    numWoBLECons = NumConnections();
    connectable = (numWoBLECons < kMaxConnections && mNumGAPCons < NRF_SDH_BLE_PERIPHERAL_LINK_COUNT);
    gapAdvParams.properties.type = connectable
            ? BLE_GAP_ADV_TYPE_CONNECTABLE_SCANNABLE_UNDIRECTED
            : BLE_GAP_ADV_TYPE_NONCONNECTABLE_SCANNABLE_UNDIRECTED;

    // Advertise in fast mode if not fully provisioned and there are no WoBLE connections, or
    // if the application has expressly requested fast advertising.
    gapAdvParams.interval =
        ((numWoBLECons == 0 && !ConfigurationMgr().IsFullyProvisioned()) || GetFlag(mFlags, kFlag_FastAdvertisingEnabled))
        ? WEAVE_DEVICE_CONFIG_BLE_FAST_ADVERTISING_INTERVAL
        : WEAVE_DEVICE_CONFIG_BLE_SLOW_ADVERTISING_INTERVAL;

#if WEAVE_PROGRESS_LOGGING

    {
        char devNameBuf[kMaxDeviceNameLength + 1];
        GetDeviceName(devNameBuf, sizeof(devNameBuf));
        WeaveLogProgress(DeviceLayer, "Configuring WoBLE advertising (interval %" PRIu32 " ms, %sconnectable, device name %s)",
                         (((uint32_t)gapAdvParams.interval) * 10) / 16,
                         (connectable) ? "" : "non-",
                         devNameBuf);
    }

#endif // WEAVE_PROGRESS_LOGGING

    // Configure an "advertising set" in the BLE soft device with the data and parameters for Weave advertising.
    // If the advertising set doesn't exist, this call will create it and return its handle.
    err = sd_ble_gap_adv_set_configure(&mAdvHandle, &gapAdvData, &gapAdvParams);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "sd_ble_gap_adv_set_configure() failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Instruct the BLE soft device to start advertising using the configured advertising set.
    err = sd_ble_gap_adv_start(mAdvHandle, WEAVE_DEVICE_LAYER_BLE_CONN_CFG_TAG);
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "sd_ble_gap_adv_start() failed: %s", ErrorStr(err));
    }
    SuccessOrExit(err);

    // Transition to the Advertising state...
    if (!GetFlag(mFlags, kFlag_Advertising))
    {
        WeaveLogProgress(DeviceLayer, "WoBLE advertising started");

        SetFlag(mFlags, kFlag_Advertising);

        // Post a WoBLEAdvertisingChange(Started) event.
        {
            WeaveDeviceEvent advChange;
            advChange.Type = DeviceEventType::kWoBLEAdvertisingChange;
            advChange.WoBLEAdvertisingChange.Result = kActivity_Started;
            PlatformMgr().PostEvent(&advChange);
        }
    }

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::StopAdvertising(void)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;

    // Since we're about to update the SoftDevice, clear the refresh needed flag.
    ClearFlag(mFlags, kFlag_AdvertisingRefreshNeeded);

    // Instruct the SoftDevice to stop advertising.
    // Ignore any error indicating that advertising is already stopped.  This case is arises
    // when a connection is established, which causes the SoftDevice to immediately cease
    // advertising.
    err = sd_ble_gap_adv_stop(mAdvHandle);
    if (err == NRF_ERROR_INVALID_STATE)
    {
        err = WEAVE_NO_ERROR;
    }
    SuccessOrExit(err);

    // Transition to the not Advertising state...
    if (GetFlag(mFlags, kFlag_Advertising))
    {
        ClearFlag(mFlags, kFlag_Advertising);

        WeaveLogProgress(DeviceLayer, "WoBLE advertising stopped");

        // Directly inform the ThreadStackManager that WoBLE advertising has stopped.
#if WEAVE_DEVICE_CONFIG_ENABLE_THREAD
        ThreadStackMgr().OnWoBLEAdvertisingStop();
#endif // WEAVE_DEVICE_CONFIG_ENABLE_THREAD

        // Post a WoBLEAdvertisingChange(Stopped) event.
        {
            WeaveDeviceEvent advChange;
            advChange.Type = DeviceEventType::kWoBLEAdvertisingChange;
            advChange.WoBLEAdvertisingChange.Result = kActivity_Stopped;
            PlatformMgr().PostEvent(&advChange);
        }
    }

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::EncodeAdvertisingData(ble_gap_adv_data_t & gapAdvData)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    ble_advdata_t advData;
    ble_advdata_service_data_t serviceData;
    WeaveBLEDeviceIdentificationInfo deviceIdInfo;

    // Form the contents of the advertising packet.
    memset(&advData, 0, sizeof(advData));
    advData.name_type = BLE_ADVDATA_FULL_NAME;
    advData.include_appearance = false;
    advData.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;
    advData.uuids_complete.uuid_cnt = 1;
    advData.uuids_complete.p_uuids = (ble_uuid_t *)&UUID_WoBLEService;
    gapAdvData.adv_data.p_data = mAdvDataBuf;
    gapAdvData.adv_data.len = sizeof(mAdvDataBuf);
    err = ble_advdata_encode(&advData, mAdvDataBuf, &gapAdvData.adv_data.len);
    SuccessOrExit(err);

    // Initialize the Weave BLE Device Identification Information block that will be sent as payload
    // within the BLE service advertisement data.
    err = ConfigurationMgr().GetBLEDeviceIdentificationInfo(deviceIdInfo);
    SuccessOrExit(err);

    // Form the contents of the scan response packet.
    memset(&serviceData, 0, sizeof(serviceData));
    serviceData.service_uuid = UUID16_WoBLEService;
    serviceData.data.size = sizeof(deviceIdInfo);
    serviceData.data.p_data = (uint8_t *)&deviceIdInfo;
    memset(&advData, 0, sizeof(advData));
    advData.name_type = BLE_ADVDATA_NO_NAME;
    advData.include_appearance = false;
    advData.p_service_data_array = &serviceData;
    advData.service_data_count = 1;
    gapAdvData.scan_rsp_data.p_data = mScanRespDataBuf;
    gapAdvData.scan_rsp_data.len = sizeof(mScanRespDataBuf);
    err = ble_advdata_encode(&advData, mScanRespDataBuf, &gapAdvData.scan_rsp_data.len);
    SuccessOrExit(err);

exit:
    return err;
}

uint16_t BLEManagerImpl::_NumConnections(void)
{
    uint16_t numCons = 0;
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mSubscribedConIds[i] != BLE_CONNECTION_UNINITIALIZED)
        {
            numCons++;
        }
    }
    return numCons;
}

void BLEManagerImpl::DriveBLEState(intptr_t arg)
{
    sInstance.DriveBLEState();
}

void BLEManagerImpl::HandleSoftDeviceBLEEvent(const WeaveDeviceEvent * event)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    const ble_evt_t * bleEvent = &event->Platform.SoftDeviceBLEEvent.EventData;

    WeaveLogDetail(DeviceLayer, "BLE SoftDevice event 0x%02x", bleEvent->header.evt_id);

    switch (bleEvent->header.evt_id)
    {
    case BLE_GAP_EVT_CONNECTED:
        err = HandleGAPConnect(event);
        SuccessOrExit(err);
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        err = HandleGAPDisconnect(event);
        SuccessOrExit(err);
        break;

    case BLE_GAP_EVT_SEC_PARAMS_REQUEST:
        // BLE Pairing not supported
        err = sd_ble_gap_sec_params_reply(bleEvent->evt.gap_evt.conn_handle,
                                          BLE_GAP_SEC_STATUS_PAIRING_NOT_SUPP,
                                          NULL,
                                          NULL);
        SuccessOrExit(err);
        break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
    {
        WeaveLogDetail(DeviceLayer, "BLE GAP PHY update request (con %" PRIu16 ")", bleEvent->evt.gap_evt.conn_handle);
        const ble_gap_phys_t phys = { BLE_GAP_PHY_AUTO, BLE_GAP_PHY_AUTO };
        err = sd_ble_gap_phy_update(bleEvent->evt.gap_evt.conn_handle, &phys);
        SuccessOrExit(err);
        break;
    }

    case BLE_GATTS_EVT_SYS_ATTR_MISSING:
        err = sd_ble_gatts_sys_attr_set(bleEvent->evt.gatts_evt.conn_handle, NULL, 0, 0);
        SuccessOrExit(err);
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        WeaveLogProgress(DeviceLayer, "BLE GATT Server timeout (con %" PRIu16 ")", bleEvent->evt.gatts_evt.conn_handle);
        err = sd_ble_gap_disconnect(bleEvent->evt.gatts_evt.conn_handle,
                                    BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        SuccessOrExit(err);
        break;

    case BLE_GATTS_EVT_WRITE:
        // Handle the event if it is a write to the WoBLE TX CCCD attribute.  Otherwise simply
        // ignore it.
        if (bleEvent->evt.gatts_evt.params.write.handle == mWoBLECharHandle_TX.cccd_handle)
        {
            err = HandleTXCharCCCDWrite(event);
            SuccessOrExit(err);
        }
        break;

    case BLE_GATTS_EVT_HVC:
        err = HandleTXComplete(event);
        SuccessOrExit(err);
        break;

    default:
        break;
    }

exit:
    if (err != WEAVE_NO_ERROR)
    {
        WeaveLogError(DeviceLayer, "Disabling WoBLE service due to error: %s", ErrorStr(err));
        sInstance.mServiceMode = ConnectivityManager::kWoBLEServiceMode_Disabled;
        PlatformMgr().ScheduleWork(DriveBLEState, 0);
    }
}

void BLEManagerImpl::SoftDeviceBLEEventCallback(const ble_evt_t * bleEvent, void * context)
{
    WeaveDeviceEvent event;

    // NB: When NRF_SDH_DISPATCH_MODEL_INTERRUPT is enabled (the typical case), this method runs
    // in interrupt context.

    // If the event is a write event for the WoBLE RX characteristic value...
    if (bleEvent->header.evt_id == BLE_GATTS_EVT_WRITE &&
        bleEvent->evt.gatts_evt.params.write.handle == sInstance.mWoBLECharHandle_RX.value_handle)
    {
        PacketBuffer * buf;
        const uint16_t valLen = bleEvent->evt.gatts_evt.params.write.len;
        const uint16_t minBufSize = WEAVE_SYSTEM_PACKETBUFFER_HEADER_SIZE + valLen;

        // Attempt to allocate a packet buffer with enough space to hold the characteristic value.
        // Note that we must use pbuf_alloc() directly, as PacketBuffer::New() is not interrupt-safe.
        buf = (PacketBuffer *)(pbuf_alloc(PBUF_RAW, minBufSize, PBUF_POOL));

        // If successful...
        if (buf != NULL)
        {
            // Copy the characteristic value into the packet buffer.
            memcpy(buf->Start(), bleEvent->evt.gatts_evt.params.write.data, valLen);
            buf->SetDataLength(valLen);

            // Arrange to post a WoBLERXWriteEvent event to the Weave queue.
            event.Type = DeviceEventType::kWoBLERXCharWriteEvent;
            event.Platform.RXCharWriteEvent.ConId = bleEvent->evt.gatts_evt.conn_handle;
            event.Platform.RXCharWriteEvent.WriteArgs = bleEvent->evt.gatts_evt.params.write;
            event.Platform.RXCharWriteEvent.Data = buf;
        }

        // If we failed to allocate a buffer, post a kWoBLEOutOfBuffersEvent event.
        else
        {
            event.Type = DeviceEventType::kWoBLEOutOfBuffersEvent;
        }
    }

    else
    {
        // Ignore any events that are larger than a particular size.
        //
        // With the exception of GATT write events to the WoBLE RX characteristic, which are handled above,
        // all BLE events that Weave is interested are of a limited size, roughly equal to sizeof(ble_evt_t)
        // plus a couple of bytes of payload data.  Any event that is larger than this size and not handled
        // above must represent an event related to some *other* user of the BLE SoftDevice and therefore
        // can be safely ignored by Weave.
        VerifyOrExit(bleEvent->header.evt_len <= sizeof(event.Platform.SoftDeviceBLEEvent), /**/);

        // Ignore any write event for anything *other* than the WoBLE TX CCCD value.
        VerifyOrExit(bleEvent->header.evt_id != BLE_GATTS_EVT_WRITE ||
                     bleEvent->evt.gatts_evt.params.write.handle == sInstance.mWoBLECharHandle_TX.cccd_handle, /**/);

        // Arrange to post a SoftDeviceBLEEvent containing a copy of the BLE event.
        event.Type = DeviceEventType::kSoftDeviceBLEEvent;
        memcpy(&event.Platform.SoftDeviceBLEEvent.EventData, bleEvent, bleEvent->header.evt_len);
    }

    // Post the event to the Weave queue.
#if (NRF_SDH_DISPATCH_MODEL == NRF_SDH_DISPATCH_MODEL_INTERRUPT)
    BaseType_t yieldRequired;
    PlatformMgrImpl().PostEventFromISR(&event, yieldRequired);
    portYIELD_FROM_ISR(yieldRequired);
#else
    PlatformMgrImpl().PostEvent(&event);
#endif

exit:
    return;
}

WEAVE_ERROR BLEManagerImpl::HandleGAPConnect(const WeaveDeviceEvent * event)
{
    const ble_gap_evt_t * gapEvent = &event->Platform.SoftDeviceBLEEvent.EventData.evt.gap_evt;

    WeaveLogProgress(DeviceLayer, "BLE GAP connection established (con %" PRIu16 ")", gapEvent->conn_handle);

    // Track the number of active GAP connections.
    mNumGAPCons++;

    // The SoftDevice automatically disables advertising whenever a connection is established.
    // So record that the SoftDevice state needs to be refreshed. If advertising needs to be
    // re-enabled, this will be handled in DriveBLEState() the next time it runs.
    SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);

    // Schedule DriveBLEState() to run.
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return WEAVE_NO_ERROR;
}

WEAVE_ERROR BLEManagerImpl::HandleGAPDisconnect(const WeaveDeviceEvent * event)
{
    const ble_gap_evt_t * gapEvent = &event->Platform.SoftDeviceBLEEvent.EventData.evt.gap_evt;

    WeaveLogProgress(DeviceLayer, "BLE GAP connection terminated (con %" PRIu16 ", reason 0x%02" PRIx8 ")", gapEvent->conn_handle, gapEvent->params.disconnected.reason);

    // Update the number of GAP connections.
    if (mNumGAPCons > 0)
    {
        mNumGAPCons--;
    }

    // If indications were enabled for this connection, record that they are now disabled and
    // notify the BLE Layer of a disconnect.
    if (UnsetSubscribed(gapEvent->conn_handle))
    {
        WEAVE_ERROR disconReason;
        switch (gapEvent->params.disconnected.reason)
        {
        case BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION:
            disconReason = BLE_ERROR_REMOTE_DEVICE_DISCONNECTED;
            break;
        case BLE_HCI_LOCAL_HOST_TERMINATED_CONNECTION:
            disconReason = BLE_ERROR_APP_CLOSED_CONNECTION;
            break;
        default:
            disconReason = BLE_ERROR_WOBLE_PROTOCOL_ABORT;
            break;
        }
        HandleConnectionError(gapEvent->conn_handle, disconReason);
    }

    // Force a reconfiguration of advertising in case we switched to non-connectable mode when
    // the BLE connection was established.
    SetFlag(mFlags, kFlag_AdvertisingRefreshNeeded);
    PlatformMgr().ScheduleWork(DriveBLEState, 0);

    return WEAVE_NO_ERROR;
}

WEAVE_ERROR BLEManagerImpl::HandleRXCharWrite(const WeaveDeviceEvent * event)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    PacketBuffer * buf = event->Platform.RXCharWriteEvent.Data;

    // Only allow write requests or write commands.
    VerifyOrExit((event->Platform.RXCharWriteEvent.WriteArgs.op == BLE_GATTS_OP_WRITE_REQ ||
                  event->Platform.RXCharWriteEvent.WriteArgs.op == BLE_GATTS_OP_WRITE_CMD),
                 err = WEAVE_ERROR_INVALID_ARGUMENT);

    // NB: The initial write to the RX characteristic happens *before* the client enables
    // indications on the TX characteristic.

    WeaveLogDetail(DeviceLayer, "Write request/command received for WoBLE RX characteristic (con %" PRIu16 ", len %" PRIu16 ")",
                   event->Platform.RXCharWriteEvent.ConId, buf->DataLength());

    // Pass the data to the BLEEndPoint
    HandleWriteReceived(event->Platform.RXCharWriteEvent.ConId, &WEAVE_BLE_SVC_ID, &WeaveUUID_WoBLEChar_RX, buf);
    buf = NULL;

exit:
    PacketBuffer::Free(buf);
    return err;
}

WEAVE_ERROR BLEManagerImpl::HandleTXCharCCCDWrite(const WeaveDeviceEvent * event)
{
    WEAVE_ERROR err = WEAVE_NO_ERROR;
    const ble_gatts_evt_t * gattsEvent = &event->Platform.SoftDeviceBLEEvent.EventData.evt.gatts_evt;
    bool indicationsEnabled;
    enum
    {
        // Per the BLE GATT spec...
        kCCCDBit_NotificationsEnabled       = 0x0001,
        kCCCDBit_IndicationsEnabled         = 0x0002,
    };

    // Only allow write requests or write commands.
    VerifyOrExit((gattsEvent->params.write.op == BLE_GATTS_OP_WRITE_REQ || gattsEvent->params.write.op == BLE_GATTS_OP_WRITE_CMD),
                 err = WEAVE_ERROR_INVALID_ARGUMENT);

    WeaveLogDetail(DeviceLayer, "Write request/command received for WoBLE TX CCCD characteristic (con %" PRIu16 ", len %" PRIu16 ")",
                   gattsEvent->conn_handle, gattsEvent->params.write.len);

    // Ignore the write request if the value is 0 length.
    VerifyOrExit(gattsEvent->params.write.len > 0, /**/);

    WeaveLogDetail(DeviceLayer, "CCCD value: 0x%04" PRIx16, (((uint16_t)gattsEvent->params.write.data[1]) << 8) | gattsEvent->params.write.data[0]);

    // Determine if the client is enabling or disabling indications.
    indicationsEnabled = ((gattsEvent->params.write.data[0] & kCCCDBit_IndicationsEnabled) != 0);

    // If the client has requested to enabled indications...
    if (indicationsEnabled)
    {
        // If indications are not already enabled for the connection...
        if (!IsSubscribed(gattsEvent->conn_handle))
        {
            // Record that indications have been enabled for this connection.  If this fails because
            // there are too many WoBLE connections, simply ignore client's attempt to subscribe.
            err = SetSubscribed(gattsEvent->conn_handle);
            VerifyOrExit(err != WEAVE_ERROR_NO_MEMORY, err = WEAVE_NO_ERROR);
            SuccessOrExit(err);

            // Alert the BLE layer that WoBLE "subscribe" has been received.
            HandleSubscribeReceived(gattsEvent->conn_handle, &WEAVE_BLE_SVC_ID, &WeaveUUID_WoBLEChar_TX);

#if WEAVE_PROGRESS_LOGGING
            {
                uint16_t gattMTU;
                uint8_t packetDataLen;

                gattMTU = nrf_ble_gatt_eff_mtu_get(&GATTModule, gattsEvent->conn_handle);
                nrf_ble_gatt_data_length_get(&GATTModule, gattsEvent->conn_handle, &packetDataLen);
                WeaveLogProgress(DeviceLayer, "WoBLE connection established (con %" PRIu16 ", packet data len %" PRIu8 ", GATT MTU %" PRIu16 ")",
                        gattsEvent->conn_handle, packetDataLen, gattMTU);
            }
#endif // WEAVE_PROGRESS_LOGGING

            // Post a WoBLEConnectionEstablished event to the DeviceLayer and the application.
            {
                WeaveDeviceEvent conEstEvent;
                conEstEvent.Type = DeviceEventType::kWoBLEConnectionEstablished;
                PlatformMgr().PostEvent(&conEstEvent);
            }
        }
    }

    else
    {
        // If indications had previously been enabled for this connection, record that they are no longer
        // enabled and inform the BLE layer that the client has "unsubscribed" the connection.
        if (UnsetSubscribed(gattsEvent->conn_handle))
        {
            HandleUnsubscribeReceived(gattsEvent->conn_handle, &WEAVE_BLE_SVC_ID, &WeaveUUID_WoBLEChar_TX);
        }
    }

exit:
    return err;
}

WEAVE_ERROR BLEManagerImpl::HandleTXComplete(const WeaveDeviceEvent * event)
{
    const ble_gatts_evt_t * gattsEvent = &event->Platform.SoftDeviceBLEEvent.EventData.evt.gatts_evt;

    WeaveLogDetail(DeviceLayer, "Confirm received for WoBLE TX characteristic indication (con %" PRIu16 ")", gattsEvent->conn_handle);

    // Signal the BLE Layer that the outstanding indication is complete.
    HandleIndicationConfirmation(gattsEvent->conn_handle, &WEAVE_BLE_SVC_ID, &WeaveUUID_WoBLEChar_TX);

    return WEAVE_NO_ERROR;
}

WEAVE_ERROR BLEManagerImpl::SetSubscribed(uint16_t conId)
{
    uint16_t freeIndex = kMaxConnections;

    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mSubscribedConIds[i] == conId)
        {
            return WEAVE_NO_ERROR;
        }
        else if (mSubscribedConIds[i] == BLE_CONNECTION_UNINITIALIZED && i < freeIndex)
        {
            freeIndex = i;
        }
    }

    if (freeIndex < kMaxConnections)
    {
        mSubscribedConIds[freeIndex] = conId;
        return WEAVE_NO_ERROR;
    }
    else
    {
        return WEAVE_ERROR_NO_MEMORY;
    }
}

bool BLEManagerImpl::UnsetSubscribed(uint16_t conId)
{
    for (uint16_t i = 0; i < kMaxConnections; i++)
    {
        if (mSubscribedConIds[i] == conId)
        {
            mSubscribedConIds[i] = BLE_CONNECTION_UNINITIALIZED;
            return true;
        }
    }
    return false;
}

bool BLEManagerImpl::IsSubscribed(uint16_t conId)
{
    if (conId != BLE_CONNECTION_UNINITIALIZED)
    {
        for (uint16_t i = 0; i < kMaxConnections; i++)
        {
            if (mSubscribedConIds[i] == conId)
            {
                return true;
            }
        }
    }
    return false;
}


} // namespace Internal
} // namespace DeviceLayer
} // namespace Weave
} // namespace nl

#endif // WEAVE_DEVICE_CONFIG_ENABLE_WOBLE
